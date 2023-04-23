/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2017 MaNGOSOne <https://github.com/mangos/one>
 * Copyright (C) 2017 Hellfire <https://hellfire-core.github.io/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "MapManager.h"
#include "Player.h"
#include "TemporarySummon.h"
#include "GridNotifiers.h"
#include "WorldSession.h"
#include "Log.h"
#include "GridStates.h"
#include "CellImpl.h"
#include "InstanceData.h"
#include "Map.h"
#include "GridNotifiersImpl.h"
#include "Transports.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "World.h"
#include "ScriptMgr.h"
#include "Group.h"
#include "MapRefManager.h"
#include "WaypointMgr.h"
#include "BattleGround.h"
#include "GridMap.h"
#include "PathFinder.h"
#include "InstanceSaveMgr.h"
#include "VMapFactory.h"
#include "MoveMap.h"

#define DEFAULT_GRID_EXPIRY     300
#define MAX_GRID_LOAD_TIME      50
#define MAX_CREATURE_ATTACK_RADIUS  (45.0f * sWorld.getConfig(RATE_CREATURE_AGGRO))

struct ScriptAction
{
    uint64 sourceGUID;
    uint64 targetGUID;
    uint64 ownerGUID;                                       // owner of source if source is item
    ScriptInfo const* script;                               // pointer to static script data
};

GridState* si_GridStates[MAX_GRID_STATE];

Map::~Map()
{
    UnloadAll();

    if (!m_scriptSchedule.empty())
        sWorld.DecreaseScheduledScriptCount(m_scriptSchedule.size());

     MMAP::MMapFactory::createOrGetMMapManager()->unloadMapInstance(m_TerrainData->GetMapId(), GetInstanceId());

    //release reference count
    if (m_TerrainData->Release())
        sTerrainMgr.UnloadTerrain(m_TerrainData->GetMapId());
}

void Map::InitStateMachine()
{
    si_GridStates[GRID_STATE_INVALID] = new InvalidState;
    si_GridStates[GRID_STATE_ACTIVE] = new ActiveState;
    si_GridStates[GRID_STATE_IDLE] = new IdleState;
    si_GridStates[GRID_STATE_REMOVAL] = new RemovalState;
}

void Map::DeleteStateMachine()
{
    delete si_GridStates[GRID_STATE_INVALID];
    delete si_GridStates[GRID_STATE_ACTIVE];
    delete si_GridStates[GRID_STATE_IDLE];
    delete si_GridStates[GRID_STATE_REMOVAL];
}

Map::Map(uint32 id, time_t expiry, uint32 InstanceId, uint8 SpawnMode)
   : i_mapEntry (sMapStore.LookupEntry(id)), i_spawnMode(SpawnMode),
     i_id(id), i_InstanceId(InstanceId), m_unloadTimer(0), i_gridExpiry(expiry), m_TerrainData(sTerrainMgr.LoadTerrain(id)),
     m_activeNonPlayersIter(m_activeNonPlayers.end()), i_scriptLock(true)
{
    for (unsigned int j=0; j < MAX_NUMBER_OF_GRIDS; ++j)
    {
        for (unsigned int idx=0; idx < MAX_NUMBER_OF_GRIDS; ++idx)
        {
            setNGrid(NULL, idx, j);
        }
    }

    //lets initialize visibility distance for map
    Map::InitVisibilityDistance();

    //add reference for TerrainData object
    m_TerrainData->AddRef();

    SetBroken(false);

    if (IsBattleArena())
        m_wanted_delay = sWorld.getConfig(CONFIG_MAPUPDATE_ARENAS);
    else if (IsBattleGround())
        m_wanted_delay = sWorld.getConfig(CONFIG_MAPUPDATE_BATTLEGROUNDS);
    else if (IsDungeon())
        m_wanted_delay = sWorld.getConfig(CONFIG_MAPUPDATE_INSTANCES);
    else
        m_wanted_delay = sWorld.getConfig(CONFIG_MAPUPDATE_CONTINENTS);
}

void Map::InitVisibilityDistance()
{
    //init visibility for continents
    m_ActiveObjectUpdateDistance = sWorld.GetActiveObjectUpdateDistanceOnContinents();
}

// Template specialization of utility methods
template<class T>
void Map::AddToGrid(T* obj, NGridType *grid, Cell const& cell)
{
    (*grid)(cell.CellX(), cell.CellY()).template AddGridObject<T>(obj);
}

template<>
void Map::AddToGrid(Player* obj, NGridType *grid, Cell const& cell)
{
    (*grid)(cell.CellX(), cell.CellY()).AddWorldObject(obj);
}

template<>
void Map::AddToGrid(Corpse *obj, NGridType *grid, Cell const& cell)
{
    // add to world object registry in grid
    if (obj->GetType()!=CORPSE_BONES)
    {
        (*grid)(cell.CellX(), cell.CellY()).AddWorldObject(obj);
    }
    // add to grid object store
    else
    {
        (*grid)(cell.CellX(), cell.CellY()).AddGridObject(obj);
    }
}

template<>
void Map::AddToGrid(Creature* obj, NGridType *grid, Cell const& cell)
{
    // add to world object registry in grid
    if (obj->isPet() || obj->IsTempWorldObject)
    {
        (*grid)(cell.CellX(), cell.CellY()).AddWorldObject<Creature>(obj);
    }
    // add to grid object store
    else
    {
        (*grid)(cell.CellX(), cell.CellY()).AddGridObject<Creature>(obj);
    }
    obj->SetCurrentCell(cell);
}

template<>
void Map::AddToGrid(DynamicObject* obj, NGridType *grid, Cell const& cell)
{
    if (obj->isActiveObject()) // only farsight
        (*grid)(cell.CellX(), cell.CellY()).AddWorldObject<DynamicObject>(obj);
    else
        (*grid)(cell.CellX(), cell.CellY()).AddGridObject<DynamicObject>(obj);
}

template<class T>
void Map::RemoveFromGrid(T* obj, NGridType *grid, Cell const& cell)
{
    (*grid)(cell.CellX(), cell.CellY()).template RemoveGridObject<T>(obj);
}

template<>
void Map::RemoveFromGrid(Player* obj, NGridType *grid, Cell const& cell)
{
    (*grid)(cell.CellX(), cell.CellY()).RemoveWorldObject(obj);
}

template<>
void Map::RemoveFromGrid(Corpse *obj, NGridType *grid, Cell const& cell)
{
    // remove from world object registry in grid
    if (obj->GetType()!=CORPSE_BONES)
    {
        (*grid)(cell.CellX(), cell.CellY()).RemoveWorldObject(obj);
    }
    // remove from grid object store
    else
    {
        (*grid)(cell.CellX(), cell.CellY()).RemoveGridObject(obj);
    }
}

template<>
void Map::RemoveFromGrid(Creature* obj, NGridType *grid, Cell const& cell)
{
    // remove from world object registry in grid
    if (obj->isPet() || obj->IsTempWorldObject)
    {
        (*grid)(cell.CellX(), cell.CellY()).RemoveWorldObject<Creature>(obj);
    }
    // remove from grid object store
    else
    {
        (*grid)(cell.CellX(), cell.CellY()).RemoveGridObject<Creature>(obj);
    }
}

template<>
void Map::RemoveFromGrid(DynamicObject* obj, NGridType *grid, Cell const& cell)
{
    if (obj->isActiveObject()) // only farsight
        (*grid)(cell.CellX(), cell.CellY()).RemoveWorldObject<DynamicObject>(obj);
    else
        (*grid)(cell.CellX(), cell.CellY()).RemoveGridObject<DynamicObject>(obj);
}

template<class T>
void Map::SwitchGridContainers(T* obj, bool on)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    if (p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Map::SwitchGridContainers: Object %lu have invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell(p);
    if (!loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)))
        return;

    DEBUG_LOG("Switch object " I64FMT " from grid[%u,%u] %u", obj->GetGUID(), cell.data.Part.grid_x, cell.data.Part.grid_y, on);
    NGridType *ngrid = getNGrid(cell.GridX(), cell.GridY());
    ASSERT(ngrid != NULL);

    GridType &grid = (*ngrid)(cell.CellX(), cell.CellY());

    if (on)
    {
        grid.RemoveGridObject<T>(obj);
        grid.AddWorldObject<T>(obj);
    }
    else
    {
        grid.RemoveWorldObject<T>(obj);
        grid.AddGridObject<T>(obj);
    }
    obj->IsTempWorldObject = on;
}

template void Map::SwitchGridContainers(Creature *, bool);
template void Map::SwitchGridContainers(DynamicObject *, bool);

template<class T>
void Map::DeleteFromWorld(T* obj)
{
    // Note: In case resurrectable corpse and pet its removed from global lists in own destructor
    delete obj;
}

void Map::EnsureGridCreated(const GridPair &p)
{
    if (!getNGrid(p.x_coord, p.y_coord))
    {
        ACE_GUARD(ACE_Thread_Mutex, Guard, Lock);
        if (!getNGrid(p.x_coord, p.y_coord))
        {
            sLog.outDebug("Loading grid[%u,%u] for map %u", p.x_coord, p.y_coord, i_id);

            setNGrid(new NGridType(p.x_coord*MAX_NUMBER_OF_GRIDS + p.y_coord, p.x_coord, p.y_coord, i_gridExpiry, sWorld.getConfig(CONFIG_GRID_UNLOAD)),
                p.x_coord, p.y_coord);

            // build a linkage between this map and NGridType
            buildNGridLinkage(getNGrid(p.x_coord, p.y_coord));

            getNGrid(p.x_coord, p.y_coord)->SetGridState(GRID_STATE_IDLE);

            //z coord
            int gx = (MAX_NUMBER_OF_GRIDS - 1) - p.x_coord;
            int gy = (MAX_NUMBER_OF_GRIDS - 1) - p.y_coord;

            m_TerrainData->Load(gx, gy); // ensure vmap/mmap loaded
        }
    }
}

void Map::EnsureGridLoaded(const Cell &cell)
{
    EnsureGridCreated(GridPair(cell.GridX(), cell.GridY()));
    NGridType *grid = getNGrid(cell.GridX(), cell.GridY());

    ASSERT(grid != NULL);
    if (!isGridObjectDataLoaded(cell.GridX(), cell.GridY()))
    {
        sLog.outDebug("Loading grid[%u,%u] for map %u instance %u", cell.GridX(), cell.GridY(), GetId(), i_InstanceId);

        ObjectGridLoader loader(*grid, this, cell);
        loader.LoadN();

        // Add resurrectable corpses to world object list in grid
        sObjectAccessor.AddCorpsesToGrid(GridPair(cell.GridX(), cell.GridY()), (*grid)(cell.CellX(), cell.CellY()), this);

        setGridObjectDataLoaded(true, cell.GridX(), cell.GridY());
    }
    
    // refresh grid state & timer
    if (grid->GetGridState() != GRID_STATE_ACTIVE)
    {
        ResetGridExpiry(*grid, 0.1f);;
        grid->SetGridState(GRID_STATE_ACTIVE);
    }
}

bool Map::Add(Player *player)
{
    player->SetInstanceId(GetInstanceId());

    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Map::Add: Player (GUID: %u) have invalid coordinates X:%f Y:%f grid cell [%u:%u]", player->GetGUIDLow(), player->GetPositionX(), player->GetPositionY(), p.x_coord, p.y_coord);
        return false;
    }

    player->GetMapRef().link(this, player);

    Cell cell(p);
    EnsureGridLoaded(cell);
    NGridType *grid = getNGrid(cell.GridX(), cell.GridY());
    ASSERT(grid != NULL);
    AddToGrid(player, grid, cell);

    player->AddToWorld();

    SendInitSelf(player);
    SendInitTransports(player);

    if (!player->IsAlive())
        player->SetMovement(MOVE_WATER_WALK);
    player->GetViewPoint().Event_AddedToWorld(&(*grid)(cell.CellX(), cell.CellY()));
    //player->UpdateObjectVisibility();
    player->m_clientGUIDs.clear();
    return true;
}

template<class T>
void Map::Add(T *obj)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());

    if (p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Map::Add: Object " UI64FMTD " have invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell(p);
    if (obj->IsInWorld()) // need some clean up later
    {
        obj->UpdateObjectVisibility();
        return;
    }

    if (obj->isActiveObject())
        EnsureGridLoaded(cell);
    else
        EnsureGridCreated(GridPair(cell.GridX(), cell.GridY()));

    NGridType *grid = getNGrid(cell.GridX(), cell.GridY());
    ASSERT(grid != NULL);

    AddToGrid(obj,grid,cell);
    obj->AddToWorld();

    if (obj->isActiveObject())
        AddToActive(obj);

    DEBUG_LOG("Object %u enters grid[%u,%u]", GUID_LOPART(obj->GetGUID()), cell.GridX(), cell.GridY());

    obj->GetViewPoint().Event_AddedToWorld(&(*grid)(cell.CellX(), cell.CellY()));
    obj->UpdateObjectVisibility();
}

void Map::BroadcastPacket(WorldObject* sender, WorldPacket *msg, bool toSelf)
{
    MaNGOS::PacketBroadcaster post_man(*sender, msg, toSelf ? NULL : sender->ToPlayer());
    Cell::VisitWorldObjects(sender, post_man, GetVisibilityDistance());
}

void Map::BroadcastPacketInRange(WorldObject* sender, WorldPacket *msg, float dist, bool toSelf, bool ownTeam)
{
    MaNGOS::PacketBroadcaster post_man(*sender, msg, toSelf ? NULL : sender->ToPlayer(), dist, ownTeam);
    Cell::VisitWorldObjects(sender, post_man, GetVisibilityDistance());
}

void Map::BroadcastPacketExcept(WorldObject* sender, WorldPacket* msg, Player* except)
{
    MaNGOS::PacketBroadcaster post_man(*sender, msg, except);
    Cell::VisitWorldObjects(sender, post_man, GetVisibilityDistance());
}

bool Map::loaded(const GridPair &p) const
{
    // sometimes when removing old corpse (converting to bones) map id goes to incredible values then... BANG CRASH!
    // possible cause: map pointer becomes invalid somewhere between ObjectAccessor::ConvertCorpseForPlayer and here.

    if (NGridType* grid_type = getNGrid(p.x_coord, p.y_coord))
    {
        return grid_type->isGridObjectDataLoaded();
    }
    return false;
}

void Map::Update(const uint32 &t_diff)
{
    volatile uint32 debug_map_id = GetId();
    uint32 startTime = WorldTimer::getMSTime();
    _dynamicTree.update(t_diff);

    /// update worldsessions for existing players
    for (m_mapRefIter = m_mapRefManager.begin(); m_mapRefIter != m_mapRefManager.end(); ++m_mapRefIter)
    {
        Player* plr = m_mapRefIter->getSource();
        if (plr && plr->IsInWorld())
        {
            WorldSession * pSession = plr->GetSession();
            MapSessionFilter updater(pSession);
            pSession->Update(t_diff, updater);
        }
    }

    if (WorldTimer::getMSTimeDiffToNow(startTime) > 90)
        sLog.outLog(LOG_DIFF, "Map::Update sessions (%u ms) map %u", WorldTimer::getMSTimeDiffToNow(startTime), GetId());
    startTime = WorldTimer::getMSTime();
    /// update players at tick
    for (m_mapRefIter = m_mapRefManager.begin(); m_mapRefIter != m_mapRefManager.end(); ++m_mapRefIter)
    {
        Player* plr = m_mapRefIter->getSource();
        if (plr && plr->IsInWorld())
        {
            WorldObject::UpdateHelper helper(plr);
            helper.Update(t_diff);
        }
    }

    if (WorldTimer::getMSTimeDiffToNow(startTime) > 50)
        sLog.outLog(LOG_DIFF, "Map::Update players (%u ms) map %u", WorldTimer::getMSTimeDiffToNow(startTime), GetId());

    uint32 alloweddiff = sWorld.getConfig(CONFIG_MIN_LOG_CELL);

    resetMarkedCells();

    MaNGOS::ObjectUpdater updater(t_diff);
    // for creature
    TypeContainerVisitor<MaNGOS::ObjectUpdater, GridTypeMapContainer> grid_object_update(updater);
    // for pets
    TypeContainerVisitor<MaNGOS::ObjectUpdater, WorldTypeMapContainer> world_object_update(updater);


    // the player iterator is stored in the map object
    // to make sure calls to Map::Remove don't invalidate it
    for (m_mapRefIter = m_mapRefManager.begin(); m_mapRefIter != m_mapRefManager.end(); ++m_mapRefIter)
    {
        Player* plr = m_mapRefIter->getSource();

        if (!plr->IsInWorld() || !plr->IsPositionValid())
            continue;

        CheckHostileRefFor(plr);

        CellArea area = Cell::CalculateCellArea(plr->GetPositionX(), plr->GetPositionY(), GetVisibilityDistance() + World::GetVisibleObjectGreyDistance());

        for (uint32 x = area.low_bound.x_coord; x <= area.high_bound.x_coord; ++x)
        {
            for (uint32 y = area.low_bound.y_coord; y <= area.high_bound.y_coord; ++y)
            {
                // marked cells are those that have been visited
                // don't visit the same cell twice
                uint32 cell_id = (y * TOTAL_NUMBER_OF_CELLS_PER_MAP) + x;
                if (!isCellMarked(cell_id))
                {
                    startTime = WorldTimer::getMSTime();
                    markCell(cell_id);
                    CellPair pair(x,y);
                    Cell cell(pair);
                    cell.SetNoCreate();
                    Visit(cell, grid_object_update);
                    Visit(cell, world_object_update);
                    if (WorldTimer::getMSTimeDiffToNow(startTime) > alloweddiff)
                        sLog.outLog(LOG_DIFF, "Map::Update cell %u %u (%u ms) map %u", x, y, WorldTimer::getMSTimeDiffToNow(startTime), GetId());
                }
            }
        }
    }

    float updatedistance = GetActiveObjectUpdateDistance();
    alloweddiff = sWorld.getConfig(CONFIG_MIN_LOG_ACTIVE_CELL);
    // non-player active objects
    if (!m_activeNonPlayers.empty() && updatedistance>=0)
    {
        for (m_activeNonPlayersIter = m_activeNonPlayers.begin(); m_activeNonPlayersIter != m_activeNonPlayers.end();)
        {
            // skip not in world
            WorldObject* obj = *m_activeNonPlayersIter;

            // step before processing, in this case if Map::Remove remove next object we correctly
            // step to next-next, and if we step to end() then newly added objects can wait next update.
            ++m_activeNonPlayersIter;

            if (!obj->IsInWorld() || !obj->IsPositionValid())
                continue;

            CellArea area = Cell::CalculateCellArea(obj->GetPositionX(), obj->GetPositionY(), updatedistance);

            for (uint32 x = area.low_bound.x_coord; x <= area.high_bound.x_coord; ++x)
            {
                for (uint32 y = area.low_bound.y_coord; y <= area.high_bound.y_coord; ++y)
                {
                    // marked cells are those that have been visited
                    // don't visit the same cell twice
                    uint32 cell_id = (y * TOTAL_NUMBER_OF_CELLS_PER_MAP) + x;
                    if (!isCellMarked(cell_id))
                    {
                        startTime = WorldTimer::getMSTime();
                        if (updatedistance > 0)
                        {
                            markCell(cell_id);
                            CellPair pair(x, y);
                            Cell cell(pair);
                            cell.SetNoCreate();
                            Visit(cell, grid_object_update);
                            Visit(cell, world_object_update);
                        }
                        else // updateddistance == 0
                        {
                            WorldObject::UpdateHelper helper(obj);
                            helper.Update(t_diff);
                        }

                        if (WorldTimer::getMSTimeDiffToNow(startTime) > alloweddiff)
                            sLog.outLog(LOG_DIFF, "Map::Update active cell %u %u (%u ms) map %u, %s %u",x, y,
                                WorldTimer::getMSTimeDiffToNow(startTime), GetId(), obj->GetName(), obj->GetGUIDLow());
                    }
                }
            }
        }
    }
    startTime = WorldTimer::getMSTime();
    // Send world objects and item update field changes
    SendObjectUpdates();

    ///- Process necessary scripts
    if (!m_scriptSchedule.empty())
    {
        i_scriptLock = true;
        ScriptsProcess();
        i_scriptLock = false;
    }

    MoveAllCreaturesInMoveList();

    if (WorldTimer::getMSTimeDiffToNow(startTime) > 100)
        sLog.outLog(LOG_DIFF,"Map::Update all thats left (%u ms) map %u", WorldTimer::getMSTimeDiffToNow(startTime), GetId());
}

void Map::CheckHostileRefFor(Player* plr)
{
    if (IsDungeon())
        return;

}

void Map::SendObjectUpdates()
{
    UpdateDataMapType update_players;
    for (ObjectSet::const_iterator it = i_objectsToClientUpdate.begin(); it != i_objectsToClientUpdate.end(); ++it)
    {
        if ((*it)->IsInWorld())
            (*it)->BuildUpdate(update_players);
    }

    i_objectsToClientUpdate.clear();

    WorldPacket packet;                                     // here we allocate a std::vector with a size of 0x10000
    for (UpdateDataMapType::iterator iter = update_players.begin(); iter != update_players.end(); ++iter)
    {
        if (iter->second.BuildPacket(&packet))
            iter->first->SendPacketToSelf(&packet);

        packet.clear();                                     // clean the string
    }
}

void Map::Remove(Player *player, bool remove)
{
    // this may be called during Map::Update
    // after decrement+unlink, ++m_mapRefIter will continue correctly
    // when the first element of the list is being removed
    // nocheck_prev will return the padding element of the RefManager
    // instead of NULL in the case of prev
    if (m_mapRefIter == player->GetMapRef())
        m_mapRefIter = m_mapRefIter->nocheck_prev();

    player->GetMapRef().unlink();
    CellPair p = MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
    if (p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
    {
        // invalid coordinates
        player->RemoveFromWorld();

        if (remove)
            DeleteFromWorld(player);
        else
            player->TeleportToHomebind();

        return;
    }

    Cell cell(p);

    if (!getNGrid(cell.data.Part.grid_x, cell.data.Part.grid_y))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Map::Remove() i_grids was NULL x:%d, y:%d",cell.data.Part.grid_x,cell.data.Part.grid_y);
        return;
    }

    DEBUG_LOG("Remove player %s from grid[%u,%u]", player->GetName(), cell.GridX(), cell.GridY());
    NGridType *grid = getNGrid(cell.GridX(), cell.GridY());
    ASSERT(grid != NULL);

    player->DestroyForNearbyPlayers();

    player->RemoveFromWorld();
    //player->UpdateObjectVisibility();

    RemoveFromGrid(player,grid,cell);
    SendRemoveTransports(player);

    if (remove)
        DeleteFromWorld(player);
}

template<class T>
void Map::Remove(T *obj, bool remove)
{
    CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    if (p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Map::Remove: Object %lu have invalid coordinates X:%f Y:%f grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
        return;
    }

    Cell cell(p);
    if (!loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)))
        return;

    DEBUG_LOG("Remove object " I64FMT " from grid[%u,%u]", obj->GetGUID(), cell.data.Part.grid_x, cell.data.Part.grid_y);
    NGridType *grid = getNGrid(cell.GridX(), cell.GridY());
    ASSERT(grid != NULL);

    obj->RemoveFromWorld();
    if (obj->isActiveObject())
        RemoveFromActive(obj);

    if (obj->GetMap() != this)
    {
        sLog.outLog(LOG_DEFAULT, "Map::Remove, object not where it should be! %u %p %p %p | %u %u %u %u",
            obj->GetTypeId(), obj->GetMap(), this, sMapMgr.FindMap(obj->GetMapId(), obj->GetInstanceId()),
            GetId(), GetInstanceId(), obj->GetMapId(), obj->GetInstanceId());
        return; // this still needs to be fixed, it will leave some useless object in memory, but at least shouldnt let them crash
    }
    obj->UpdateObjectVisibility();

    RemoveFromGrid(obj,grid,cell);

    if (remove)
    {
        // if option set then object already saved at this moment
        if (!sWorld.getConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY))
            obj->SaveRespawnTime();

        DeleteFromWorld(obj);
    }
}

void Map::PlayerRelocation(Player* player, float x, float y, float z, float orientation)
{
    Cell old_cell(MaNGOS::ComputeCellPair(player->GetPositionX(), player->GetPositionY()));
    Cell new_cell(MaNGOS::ComputeCellPair(x, y));

    player->Relocate(x, y, z, orientation);

    if (old_cell.DiffGrid(new_cell) || old_cell.DiffCell(new_cell))
    {
        // update player position for group at taxi flight
        if (player->GetGroup() && player->IsTaxiFlying())
            player->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_POSITION);

        NGridType* oldGrid = getNGrid(old_cell.GridX(), old_cell.GridY());
        RemoveFromGrid(player, oldGrid,old_cell);

        if (old_cell.DiffGrid(new_cell))
            EnsureGridLoaded(new_cell);

        NGridType* newGrid = getNGrid(new_cell.GridX(), new_cell.GridY());
        AddToGrid(player, newGrid, new_cell);
        player->GetViewPoint().Event_GridChanged(&(*newGrid)(new_cell.CellX(),new_cell.CellY()));
    }

    player->OnRelocated();
}

void Map::CreatureRelocation(Creature *creature, float x, float y, float z, float ang)
{
    uint32 mstime = WorldTimer::getMSTime();
    ASSERT(CheckGridIntegrity(creature,false));

    Cell old_cell = creature->GetCurrentCell();

    CellPair new_val = MaNGOS::ComputeCellPair(x, y);
    Cell new_cell(new_val);

    // delay creature move for grid/cell to grid/cell moves
    if (old_cell.DiffCell(new_cell) || old_cell.DiffGrid(new_cell))
        AddCreatureToMoveList(creature,x,y,z,ang);
    else
    {
        if (WorldTimer::getMSTimeDiffToNow(mstime) > 80)
            sLog.outLog(LOG_SESSION_DIFF, "creature relocation too long (check 1)");
        creature->Relocate(x, y, z, ang);
        creature->OnRelocated();
        if (WorldTimer::getMSTimeDiffToNow(mstime) > 80)
            sLog.outLog(LOG_SESSION_DIFF, "creature relocation too long (check 2)");
    }
    if (WorldTimer::getMSTimeDiffToNow(mstime) > 80)
        sLog.outLog(LOG_SESSION_DIFF, "creature relocation too long (check 3)");
}

void Map::AddCreatureToMoveList(Creature *c, float x, float y, float z, float ang)
{
    if (!c)
        return;

    i_creaturesToMove[c] = CreatureMover(x,y,z,ang);
}

void Map::MoveAllCreaturesInMoveList()
{
    while (!i_creaturesToMove.empty())
    {
        // get data and remove element;
        CreatureMoveList::iterator iter = i_creaturesToMove.begin();

        Creature* c = iter->first;
        CreatureMover cm = iter->second;

        i_creaturesToMove.erase(iter);

        // calculate cells
        CellPair new_val = MaNGOS::ComputeCellPair(cm.x, cm.y);
        Cell new_cell(new_val);

        // do move or do move to respawn or remove creature if previous all fail
        if (CreatureCellRelocation(c,new_cell))
        {
            // update pos
            c->Relocate(cm.x, cm.y, cm.z, cm.ang);
            c->OnRelocated();
        }
        else
        {
            // if creature can't be moved to new cell/grid (not loaded) move it to respawn cell/grid
            // creature coordinates will be updated and notifiers send
            if (!CreatureRespawnRelocation(c))
                AddObjectToRemoveList(c);
        }
    }
}

bool Map::CreatureCellRelocation(Creature *c, Cell new_cell)
{
    Cell const& old_cell = c->GetCurrentCell();
    // same grid
    if (!old_cell.DiffGrid(new_cell))
    {
        // different cell
        if (old_cell.DiffCell(new_cell))
        {
            RemoveFromGrid(c,getNGrid(old_cell.GridX(), old_cell.GridY()),old_cell);
            AddToGrid(c,getNGrid(new_cell.GridX(), new_cell.GridY()),new_cell);
        }
        return true;
    }

    if (c->isActiveObject())
        EnsureGridLoaded(new_cell);
    else if (loaded(GridPair(new_cell.GridX(), new_cell.GridY())))
        EnsureGridCreated(GridPair(new_cell.GridX(), new_cell.GridY()));
    else
        return false;

    NGridType* oldGrid = getNGrid(old_cell.GridX(), old_cell.GridY());
    NGridType* newGrid = getNGrid(new_cell.GridX(), new_cell.GridY());

    RemoveFromGrid(c, oldGrid, old_cell);
    AddToGrid(c, newGrid, new_cell);

    c->GetViewPoint().Event_GridChanged(&(*newGrid)(new_cell.CellX(),new_cell.CellY()));
    return true;
}

bool Map::CreatureRespawnRelocation(Creature *c)
{
    float resp_x, resp_y, resp_z, resp_o;
    c->GetRespawnCoord(resp_x, resp_y, resp_z, &resp_o);

    CellPair resp_val = MaNGOS::ComputeCellPair(resp_x, resp_y);
    Cell resp_cell(resp_val);

    c->CombatStop();

    // teleport it to respawn point (like normal respawn if player see)
    if (CreatureCellRelocation(c,resp_cell))
    {
        c->Relocate(resp_x, resp_y, resp_z, resp_o);
        c->GetUnitStateMgr().InitDefaults(true);
        c->GetMotionMaster()->Initialize();

        c->OnRelocated();
        return true;
    }
    else
        return false;
}

bool Map::UnloadGrid(const uint32 &x, const uint32 &y, bool unloadAll)
{
    NGridType *grid = getNGrid(x, y);
    ASSERT(grid != NULL);

    {
        if (!unloadAll && ActiveObjectsNearGrid(x, y))
             return false;

        sLog.outDebug("Unloading grid[%u,%u] for map %u", x,y, i_id);

        ObjectGridUnloader unloader(*grid);

        if (!unloadAll)
        {
            // Finish creature moves, remove and delete all creatures with delayed remove before moving to respawn grids
            // Must know real mob position before move
            MoveAllCreaturesInMoveList();

            // move creatures to respawn grids if this is diff.grid or to remove list
            unloader.MoveToRespawnN();

            // Finish creature moves, remove and delete all creatures with delayed remove before unload
            MoveAllCreaturesInMoveList();
        }

        ObjectGridCleaner cleaner(*grid);
        cleaner.CleanN();

        RemoveAllObjectsInRemoveList();

        unloader.UnloadN();

        ASSERT(i_objectsToRemove.empty());

        delete grid;
        setNGrid(NULL, x, y);
    }

    DEBUG_LOG("Unloading grid[%u,%u] for map %u finished", x,y, i_id);
    return true;
}

void Map::UnloadAll()
{
    // clear all delayed moves, useless anyway do this moves before map unload.
    i_creaturesToMove.clear();

    for (GridRefManager<NGridType>::iterator i = GridRefManager<NGridType>::begin(); i != GridRefManager<NGridType>::end();)
    {
        NGridType &grid(*i->getSource());
        ++i;
        UnloadGrid(grid.getX(), grid.getY(), true);       // deletes the grid and removes it from the GridRefManager
    }
}

bool Map::CheckGridIntegrity(Creature* c, bool moved) const
{
    Cell const& cur_cell = c->GetCurrentCell();

    CellPair xy_val = MaNGOS::ComputeCellPair(c->GetPositionX(), c->GetPositionY());
    Cell xy_cell(xy_val);
    if (xy_cell != cur_cell)
    {
        sLog.outDebug("Creature (GUIDLow: %u) X: %f Y: %f (%s) in grid[%u,%u]cell[%u,%u] instead grid[%u,%u]cell[%u,%u]",
            c->GetGUIDLow(),
            c->GetPositionX(),c->GetPositionY(),(moved ? "final" : "original"),
            cur_cell.GridX(), cur_cell.GridY(), cur_cell.CellX(), cur_cell.CellY(),
            xy_cell.GridX(),  xy_cell.GridY(),  xy_cell.CellX(),  xy_cell.CellY());
        return true;                                        // not crash at error, just output error in debug mode
    }

    return true;
}

const char* Map::GetMapName() const
{
    return i_mapEntry ? i_mapEntry->name[sWorld.GetDefaultDbcLocale()] : "UNNAMEDMAP\x0";
}

void Map::SendInitSelf(Player * player)
{
    sLog.outDetail("Creating player data for himself %u", player->GetGUIDLow());

    UpdateData data;

    bool hasTransport = false;

    // attach to player data current transport data
    if (Transport* transport = player->GetTransport())
    {
        hasTransport = true;
        transport->BuildCreateUpdateBlockForPlayer(&data, player);
    }

    // build data for self presence in world at own client (one time for map)
    player->BuildCreateUpdateBlockForPlayer(&data, player);

    // build other passengers at transport also (they always visible and marked as visible and will not send at visibility update at add to map
    if (Transport* transport = player->GetTransport())
    {
        for (Transport::PlayerSet::const_iterator itr = transport->GetPassengers().begin();itr!=transport->GetPassengers().end();++itr)
        {
            if (player!=(*itr) && player->HaveAtClient(*itr))
            {
                hasTransport = true;
                (*itr)->BuildCreateUpdateBlockForPlayer(&data, player);
            }
        }
    }

    WorldPacket packet;
    data.BuildPacket(&packet, hasTransport);
    player->SendPacketToSelf(&packet);
}

void Map::SendInitTransports(Player * player)
{
    // Hack to send out transports
    MapManager::TransportMap& tmap = sMapMgr.m_TransportsByMap;

    // no transports at map
    if (tmap.find(player->GetMapId()) == tmap.end())
        return;

    UpdateData transData;

    MapManager::TransportSet& tset = tmap[player->GetMapId()];

    bool hasTransport = false;

    for (MapManager::TransportSet::iterator i = tset.begin(); i != tset.end(); ++i)
    {
        // send data for current transport in other place
        if ((*i) != player->GetTransport()  && (*i)->GetMapId() == GetId())
        {
            hasTransport = true;
            (*i)->BuildCreateUpdateBlockForPlayer(&transData, player);
        }
    }

    WorldPacket packet;
    transData.BuildPacket(&packet, hasTransport);
    player->SendPacketToSelf(&packet);
}

void Map::SendRemoveTransports(Player * player)
{
    // Hack to send out transports
    MapManager::TransportMap& tmap = sMapMgr.m_TransportsByMap;

    // no transports at map
    if (tmap.find(player->GetMapId()) == tmap.end())
        return;

    UpdateData transData;

    MapManager::TransportSet& tset = tmap[player->GetMapId()];

    // except used transport
    for (MapManager::TransportSet::iterator i = tset.begin(); i != tset.end(); ++i)
        if ((*i) != player->GetTransport() && (*i)->GetMapId() != GetId())
            (*i)->BuildOutOfRangeUpdateBlock(&transData);

    WorldPacket packet;
    transData.BuildPacket(&packet);
    player->SendPacketToSelf(&packet);
}

inline void Map::setNGrid(NGridType *grid, uint32 x, uint32 y)
{
    if (x >= MAX_NUMBER_OF_GRIDS || y >= MAX_NUMBER_OF_GRIDS)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: map::setNGrid() Invalid grid coordinates found: %d, %d!",x,y);
        ASSERT(false);
    }
    i_grids[x][y] = grid;
}

void Map::DelayedUpdate(const uint32 t_diff)
{
    RemoveAllObjectsInRemoveList();

    // Don't unload grids if it's battleground, since we may have manually added GOs,creatures, those doesn't load from DB at grid re-load !
    // This isn't really bother us, since as soon as we have instanced BG-s, the whole map unloads as the BG gets ended
    if (!IsBattleGroundOrArena())
    {
        for (GridRefManager<NGridType>::iterator i = GridRefManager<NGridType>::begin(); i != GridRefManager<NGridType>::end();)
        {
            NGridType *grid = i->getSource();
            GridInfo *info = i->getSource()->getGridInfoRef();
            ++i;                                                // The update might delete the map and we need the next map before the iterator gets invalid
            ASSERT(grid->GetGridState() >= 0 && grid->GetGridState() < MAX_GRID_STATE);
            si_GridStates[grid->GetGridState()]->Update(*this, *grid, *info, grid->getX(), grid->getY(), t_diff);
        }
    }
}

void Map::AddObjectToRemoveList(WorldObject *obj)
{
    ASSERT(obj->GetMapId()==GetId() && obj->GetInstanceId()==GetInstanceId());

    obj->CleanupsBeforeDelete();                    // remove or simplify at least cross referenced links

    i_objectsToRemove.insert(obj);
    //sLog.outDebug("Object (GUID: %u TypeId: %u) added to removing list.",obj->GetGUIDLow(),obj->GetTypeId());
}

void Map::AddObjectToSwitchList(WorldObject *obj, bool on)
{
    ASSERT(obj->GetMapId()==GetId() && obj->GetInstanceId()==GetInstanceId());

    std::map<WorldObject*, bool>::iterator itr = i_objectsToSwitch.find(obj);
    if (itr == i_objectsToSwitch.end())
        i_objectsToSwitch.insert(itr, std::make_pair(obj, on));
    else if (itr->second != on)
        i_objectsToSwitch.erase(itr);
    else
        ASSERT(false);
}

void Map::RemoveAllObjectsInRemoveList()
{
    while (!i_objectsToSwitch.empty())
    {
        std::map<WorldObject*, bool>::iterator itr = i_objectsToSwitch.begin();
        WorldObject *obj = itr->first;
        bool on = itr->second;
        i_objectsToSwitch.erase(itr);

        switch (obj->GetTypeId())
        {
            case TYPEID_UNIT:
                if (!((Creature*)obj)->isPet())
                    SwitchGridContainers((Creature*)obj, on);
                break;
            default:
                break;
        }
    }

    //sLog.outDebug("Object remover 1 check.");
    while (!i_objectsToRemove.empty())
    {
        std::set<WorldObject*>::iterator itr = i_objectsToRemove.begin();
        WorldObject* obj = *itr;

        switch (obj->GetTypeId())
        {
            case TYPEID_CORPSE:
            {
                Corpse* corpse = sObjectAccessor.GetCorpse(obj->GetGUID());
                if (!corpse)
                    sLog.outLog(LOG_DEFAULT, "ERROR: Try delete corpse/bones %u that not in map", obj->GetGUIDLow());
                else
                    Remove(corpse,true);
                break;
            }
            case TYPEID_DYNAMICOBJECT:
                Remove((DynamicObject*)obj,true);
                break;
            case TYPEID_GAMEOBJECT:
                Remove((GameObject*)obj,true);
                break;
            case TYPEID_UNIT:
                // in case triggered sequence some spell can continue casting after prev CleanupsBeforeDelete call
                // make sure that like sources auras/etc removed before destructor start
                Remove((Creature*)obj,true);
                break;
            default:
                sLog.outLog(LOG_DEFAULT, "ERROR: Non-grid object (TypeId: %u) in grid object removing list, ignored.",obj->GetTypeId());
                break;
        }

        i_objectsToRemove.erase(itr);
    }
    //sLog.outDebug("Object remover 2 check.");
}

uint32 Map::GetPlayersCountExceptGMs() const
{
    uint32 count = 0;
    for (MapRefManager::const_iterator itr = m_mapRefManager.begin(); itr != m_mapRefManager.end(); ++itr)
        if (!itr->getSource()->IsGameMaster())
            ++count;
    return count;
}

uint32 Map::GetAlivePlayersCountExceptGMs() const
{
    uint32 count = 0;
    for (MapRefManager::const_iterator itr = m_mapRefManager.begin(); itr != m_mapRefManager.end(); ++itr)
    {
        if (!itr->getSource()->IsGameMaster() && itr->getSource()->IsAlive())
            ++count;
    }
    return count;
}

bool Map::ActiveObjectsNearGrid(uint32 x, uint32 y) const
{
    CellPair cell_min(x*MAX_NUMBER_OF_CELLS, y*MAX_NUMBER_OF_CELLS);
    CellPair cell_max(cell_min.x_coord + MAX_NUMBER_OF_CELLS, cell_min.y_coord+MAX_NUMBER_OF_CELLS);

    //we must find visible range in cells so we unload only non-visible cells...
    float viewDist = GetVisibilityDistance();
    int cell_range = (int)ceilf(viewDist / SIZE_OF_GRID_CELL) + 2;

    cell_min << cell_range;
    cell_min -= cell_range;
    cell_max >> cell_range;
    cell_max += cell_range;

    for (MapRefManager::const_iterator iter = m_mapRefManager.begin(); iter != m_mapRefManager.end(); ++iter)
    {
        Player* plr = iter->getSource();

        CellPair p = MaNGOS::ComputeCellPair(plr->GetPositionX(), plr->GetPositionY());
        if ((cell_min.x_coord <= p.x_coord && p.x_coord <= cell_max.x_coord) &&
            (cell_min.y_coord <= p.y_coord && p.y_coord <= cell_max.y_coord))
            return true;
    }

    for (ActiveNonPlayers::const_iterator iter = m_activeNonPlayers.begin(); iter != m_activeNonPlayers.end(); ++iter)
    {
        WorldObject* obj = *iter;

        CellPair p = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
        if ((cell_min.x_coord <= p.x_coord && p.x_coord <= cell_max.x_coord) &&
            (cell_min.y_coord <= p.y_coord && p.y_coord <= cell_max.y_coord))
            return true;
    }

    return false;
}

void Map::AddToActive(WorldObject* obj)
{
    m_activeNonPlayers.insert(obj);

    // also not allow unloading spawn grid to prevent creating creature clone at load
    if (Creature* c = obj->ToCreature())
    {
        if (!c->isPet() && c->GetDBTableGUIDLow())
        {
            float x,y,z;
            c->GetRespawnCoord(x,y,z);
            GridPair p = MaNGOS::ComputeGridPair(x, y);
            if (getNGrid(p.x_coord, p.y_coord))
                getNGrid(p.x_coord, p.y_coord)->incUnloadActiveLock();
            else
            {
                GridPair p2 = MaNGOS::ComputeGridPair(c->GetPositionX(), c->GetPositionY());
                sLog.outLog(LOG_DEFAULT, "ERROR: Active creature (GUID: %u Entry: %u) added to grid[%u,%u] but spawn grid[%u,%u] not loaded.",
                              c->GetGUIDLow(), c->GetEntry(), p.x_coord, p.y_coord, p2.x_coord, p2.y_coord);
            }
        }
    }
}

void Map::RemoveFromActive(WorldObject* obj)
{
    // Map::Update for active object in proccess
    if (m_activeNonPlayersIter != m_activeNonPlayers.end())
    {
        ActiveNonPlayers::iterator itr = m_activeNonPlayers.find(obj);
        if (itr == m_activeNonPlayersIter)
            ++m_activeNonPlayersIter;

        if(itr != m_activeNonPlayers.end())
            m_activeNonPlayers.erase(itr);
    }
    else
        m_activeNonPlayers.erase(obj);

    if (Creature* c = obj->ToCreature())
    {
        // also allow unloading spawn grid
        if (!c->isPet() && c->GetDBTableGUIDLow())
        {
            float x,y,z;
            c->GetRespawnCoord(x,y,z);
            GridPair p = MaNGOS::ComputeGridPair(x, y);
            if (getNGrid(p.x_coord, p.y_coord))
                getNGrid(p.x_coord, p.y_coord)->decUnloadActiveLock();
            else
            {
                GridPair p2 = MaNGOS::ComputeGridPair(c->GetPositionX(), c->GetPositionY());
                sLog.outLog(LOG_DEFAULT, "ERROR: Active creature (GUID: %u Entry: %u) removed from grid[%u,%u] but spawn grid[%u,%u] not loaded.",
                              c->GetGUIDLow(), c->GetEntry(), p.x_coord, p.y_coord, p2.x_coord, p2.y_coord);
            }
        }
    }
}

void Map::ScriptsStart(ScriptMapMap const& scripts, uint32 id, Object* source, Object* target)
{
    ///- Find the script map
    ScriptMapMap::const_iterator s = scripts.find(id);
    if (s == scripts.end())
        return;

    // prepare static data
    uint64 sourceGUID = source ? source->GetGUID() : (uint64)0; //some script commands doesn't have source
    uint64 targetGUID = target ? target->GetGUID() : (uint64)0;
    uint64 ownerGUID  = (source->GetTypeId()==TYPEID_ITEM) ? ((Item*)source)->GetOwnerGUID() : (uint64)0;

    ///- Schedule script execution for all scripts in the script map
    ScriptMap const *s2 = &(s->second);
    bool immedScript = false;
    for (ScriptMap::const_iterator iter = s2->begin(); iter != s2->end(); ++iter)
    {
        ScriptAction sa;
        sa.sourceGUID = sourceGUID;
        sa.targetGUID = targetGUID;
        sa.ownerGUID  = ownerGUID;

        sa.script = &iter->second;
        m_scriptSchedule.insert(std::pair<time_t, ScriptAction>(time_t(sWorld.GetGameTime() + iter->first), sa));
        if (iter->first == 0)
            immedScript = true;

        sWorld.IncreaseScheduledScriptsCount();
    }
    ///- If one of the effects should be immediate, launch the script execution
    if (/*start &&*/ immedScript && !i_scriptLock)
    {
        i_scriptLock = true;
        ScriptsProcess();
        i_scriptLock = false;
    }
}

void Map::ScriptCommandStart(ScriptInfo const& script, uint32 delay, Object* source, Object* target)
{
    // NOTE: script record _must_ exist until command executed

    // prepare static data
    uint64 sourceGUID = source ? source->GetGUID() : (uint64)0;
    uint64 targetGUID = target ? target->GetGUID() : (uint64)0;
    uint64 ownerGUID  = (source->GetTypeId()==TYPEID_ITEM) ? ((Item*)source)->GetOwnerGUID() : (uint64)0;

    ScriptAction sa;
    sa.sourceGUID = sourceGUID;
    sa.targetGUID = targetGUID;
    sa.ownerGUID  = ownerGUID;

    sa.script = &script;
    m_scriptSchedule.insert(std::pair<time_t, ScriptAction>(time_t(sWorld.GetGameTime() + delay), sa));

    sWorld.IncreaseScheduledScriptsCount();

    ///- If effects should be immediate, launch the script execution
    if (delay == 0 && !i_scriptLock)
    {
        i_scriptLock = true;
        ScriptsProcess();
        i_scriptLock = false;
    }
}

/// Process queued scripts
void Map::ScriptsProcess()
{
    if (m_scriptSchedule.empty())
        return;

    ///- Process overdue queued scripts
    std::multimap<time_t, ScriptAction>::iterator iter = m_scriptSchedule.begin();
                                                            // ok as multimap is a *sorted* associative container
    while (!m_scriptSchedule.empty() && (iter->first <= sWorld.GetGameTime()))
    {
        ScriptAction const& step = iter->second;

        Object* source = NULL;

        if (step.sourceGUID)
        {
            switch (GUID_HIPART(step.sourceGUID))
            {
                case HIGHGUID_ITEM:
                    // case HIGHGUID_CONTAINER: ==HIGHGUID_ITEM
                    {
                        Player* player = HashMapHolder<Player>::Find(step.ownerGUID);
                        if (player)
                            source = player->GetItemByGuid(step.sourceGUID);
                        break;
                    }
                case HIGHGUID_UNIT:
                    source = GetCreature(step.sourceGUID);
                    break;
                case HIGHGUID_PET:
                    source = HashMapHolder<Pet>::Find(step.sourceGUID);
                    break;
                case HIGHGUID_PLAYER:
                    source = HashMapHolder<Player>::Find(step.sourceGUID);
                    break;
                case HIGHGUID_GAMEOBJECT:
                    source = GetGameObject(step.sourceGUID);
                    break;
                case HIGHGUID_CORPSE:
                    source = HashMapHolder<Corpse>::Find(step.sourceGUID);
                    break;
                case HIGHGUID_MO_TRANSPORT:
                    for (MapManager::TransportSet::iterator iter = sMapMgr.m_Transports.begin(); iter != sMapMgr.m_Transports.end(); ++iter)
                    {
                        if ((*iter)->GetGUID() == step.sourceGUID)
                        {
                            source = reinterpret_cast<Object*>(*iter);
                            break;
                        }
                    }
                    break;
                default:
                    sLog.outLog(LOG_DEFAULT, "ERROR: *_script source with unsupported high guid value %u",GUID_HIPART(step.sourceGUID));
                    break;
            }
        }

        //if(source && !source->IsInWorld()) source = NULL;

        Object* target = NULL;

        if (step.targetGUID)
        {
            switch (GUID_HIPART(step.targetGUID))
            {
                case HIGHGUID_UNIT:
                    target = GetCreature(step.targetGUID);
                    break;
                case HIGHGUID_PET:
                    target = HashMapHolder<Pet>::Find(step.targetGUID);
                    break;
                case HIGHGUID_PLAYER:                       // empty GUID case also
                    target = HashMapHolder<Player>::Find(step.targetGUID);
                    break;
                case HIGHGUID_GAMEOBJECT:
                    target = GetGameObject(step.targetGUID);
                    break;
                case HIGHGUID_CORPSE:
                    target = HashMapHolder<Corpse>::Find(step.targetGUID);
                    break;
                default:
                    sLog.outLog(LOG_DEFAULT, "ERROR: *_script source with unsupported high guid value %u",GUID_HIPART(step.targetGUID));
                    break;
            }
        }

        //if(target && !target->IsInWorld()) target = NULL;

        switch (step.script->command)
        {
            case SCRIPT_COMMAND_TALK:
            {
                if (!source)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_TALK call for NULL creature.");
                    break;
                }

                if (source->GetTypeId()!=TYPEID_UNIT)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_TALK call for non-creature (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }
                if (step.script->datalong > 3)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_TALK invalid chat type (%u), skipping.",step.script->datalong);
                    break;
                }

                uint64 unit_target = target ? target->GetGUID() : 0;

                //datalong 0=normal say, 1=whisper, 2=yell, 3=emote text
                switch (step.script->datalong)
                {
                    case 0:                                 // Say
                        ((Creature *)source)->Say(step.script->dataint, LANG_UNIVERSAL, unit_target);
                        break;
                    case 1:                                 // Whisper
                        if (!unit_target)
                        {
                            sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_TALK attempt to whisper (%u) NULL, skipping.",step.script->datalong);
                            break;
                        }
                        ((Creature *)source)->Whisper(step.script->dataint,unit_target);
                        break;
                    case 2:                                 // Yell
                        ((Creature *)source)->Yell(step.script->dataint, LANG_UNIVERSAL, unit_target);
                        break;
                    case 3:                                 // Emote text
                        ((Creature *)source)->TextEmote(step.script->dataint, unit_target);
                        break;
                    default:
                        break;                              // must be already checked at load
                }
                break;
            }

            case SCRIPT_COMMAND_EMOTE:
                if (!source)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_EMOTE call for NULL creature.");
                    break;
                }

                if (source->GetTypeId()!=TYPEID_UNIT)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_EMOTE call for non-creature (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                ((Creature *)source)->HandleEmoteCommand(step.script->datalong);
                break;
            case SCRIPT_COMMAND_FIELD_SET:
                if (!source)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_FIELD_SET call for NULL object.");
                    break;
                }
                if (step.script->datalong <= OBJECT_FIELD_ENTRY || step.script->datalong >= source->GetValuesCount())
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_FIELD_SET call for wrong field %u (max count: %u) in object (TypeId: %u).",
                        step.script->datalong,source->GetValuesCount(),source->GetTypeId());
                    break;
                }

                source->SetUInt32Value(step.script->datalong, step.script->datalong2);
                break;
            case SCRIPT_COMMAND_MOVE_TO:
                if (!source)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_MOVE_TO call for NULL creature.");
                    break;
                }

                if (source->GetTypeId()!=TYPEID_UNIT)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_MOVE_TO call for non-creature (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                if (step.script->datalong2 != 0)
                {
                    float speed = ((Unit*)source)->GetDistance(step.script->x, step.script->y, step.script->z) / ((float)step.script->datalong2 * 0.001f);
                    ((Unit*)source)->MonsterMoveWithSpeed(step.script->x, step.script->y, step.script->z, speed);
                }
                else
                    ((Unit*)source)->NearTeleportTo(step.script->x, step.script->y, step.script->z, ((Unit*)source)->GetOrientation());

                break;
            case SCRIPT_COMMAND_FLAG_SET:
                if (!source)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_FLAG_SET call for NULL object.");
                    break;
                }
                if (step.script->datalong <= OBJECT_FIELD_ENTRY || step.script->datalong >= source->GetValuesCount())
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_FLAG_SET call for wrong field %u (max count: %u) in object (TypeId: %u).",
                        step.script->datalong,source->GetValuesCount(),source->GetTypeId());
                    break;
                }

                source->SetFlag(step.script->datalong, step.script->datalong2);

                if (source->GetTypeId() == TYPEID_UNIT && step.script->datalong == UNIT_NPC_FLAGS)
                    ((Creature *)source)->ResetGossipOptions();

                break;
            case SCRIPT_COMMAND_FLAG_REMOVE:
                if (!source)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_FLAG_REMOVE call for NULL object.");
                    break;
                }
                if (step.script->datalong <= OBJECT_FIELD_ENTRY || step.script->datalong >= source->GetValuesCount())
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_FLAG_REMOVE call for wrong field %u (max count: %u) in object (TypeId: %u).",
                        step.script->datalong,source->GetValuesCount(),source->GetTypeId());
                    break;
                }

                source->RemoveFlag(step.script->datalong, step.script->datalong2);

                if (source->GetTypeId() == TYPEID_UNIT && step.script->datalong == UNIT_NPC_FLAGS)
                    ((Creature *)source)->ResetGossipOptions();

                break;

            case SCRIPT_COMMAND_TELEPORT_TO:
            {
                // accept player in any one from target/source arg
                if (!target && !source)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_TELEPORT_TO call for NULL object.");
                    break;
                }

                                                            // must be only Player
                if ((!target || target->GetTypeId() != TYPEID_PLAYER) && (!source || source->GetTypeId() != TYPEID_PLAYER))
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_TELEPORT_TO call for non-player (TypeIdSource: %u)(TypeIdTarget: %u), skipping.", source ? source->GetTypeId() : 0, target ? target->GetTypeId() : 0);
                    break;
                }

                Player* pSource = target && target->GetTypeId() == TYPEID_PLAYER ? (Player*)target : (Player*)source;

                pSource->TeleportTo(step.script->datalong, step.script->x, step.script->y, step.script->z, step.script->o);
                break;
            }

            case SCRIPT_COMMAND_TEMP_SUMMON_CREATURE:
            {
                if (!step.script->datalong)                  // creature not specified
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_TEMP_SUMMON_CREATURE call for NULL creature.");
                    break;
                }

                if (!source)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_TEMP_SUMMON_CREATURE call for NULL world object.");
                    break;
                }

                WorldObject* summoner = dynamic_cast<WorldObject*>(source);

                if (!summoner)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_TEMP_SUMMON_CREATURE call for non-WorldObject (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                float x = step.script->x;
                float y = step.script->y;
                float z = step.script->z;
                float o = step.script->o;

                Creature* pCreature = summoner->SummonCreature(step.script->datalong, x, y, z, o,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,step.script->datalong2);
                if (!pCreature)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_TEMP_SUMMON failed for creature (entry: %u).",step.script->datalong);
                    break;
                }

                break;
            }

            case SCRIPT_COMMAND_RESPAWN_GAMEOBJECT:
            {
                if (!step.script->datalong)                  // gameobject not specified
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_RESPAWN_GAMEOBJECT call for NULL gameobject.");
                    break;
                }

                if (!source)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_RESPAWN_GAMEOBJECT call for NULL world object.");
                    break;
                }

                WorldObject* summoner = dynamic_cast<WorldObject*>(source);

                if (!summoner)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_RESPAWN_GAMEOBJECT call for non-WorldObject (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                GameObject *go = NULL;
                int32 time_to_despawn = step.script->datalong2<5 ? 5 : (int32)step.script->datalong2;

                MaNGOS::GameObjectWithDbGUIDCheck go_check(*summoner,step.script->datalong);
                MaNGOS::ObjectSearcher<GameObject, MaNGOS::GameObjectWithDbGUIDCheck> checker(go,go_check);

                Cell::VisitGridObjects(summoner, checker, GetVisibilityDistance());

                if (!go)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_RESPAWN_GAMEOBJECT failed for gameobject(guid: %u).", step.script->datalong);
                    break;
                }

                if (go->GetGoType()==GAMEOBJECT_TYPE_FISHINGNODE ||
                    go->GetGoType()==GAMEOBJECT_TYPE_DOOR        ||
                    go->GetGoType()==GAMEOBJECT_TYPE_BUTTON      ||
                    go->GetGoType()==GAMEOBJECT_TYPE_TRAP)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_RESPAWN_GAMEOBJECT can not be used with gameobject of type %u (guid: %u).", uint32(go->GetGoType()), step.script->datalong);
                    break;
                }

                if (go->isSpawned())
                    break;                                  //gameobject already spawned

                go->SetLootState(GO_READY);
                go->SetRespawnTime(time_to_despawn);        //despawn object in ? seconds

                go->GetMap()->Add(go);
                break;
            }
            case SCRIPT_COMMAND_OPEN_DOOR:
            {
                if (!step.script->datalong)                  // door not specified
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_OPEN_DOOR call for NULL door.");
                    break;
                }

                if (!source)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_OPEN_DOOR call for NULL unit.");
                    break;
                }

                if (!source->isType(TYPEMASK_UNIT))          // must be any Unit (creature or player)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_OPEN_DOOR call for non-unit (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                Unit* caster = (Unit*)source;

                GameObject *door = NULL;
                int32 time_to_close = step.script->datalong2 < 15 ? 15 : (int32)step.script->datalong2;

                MaNGOS::GameObjectWithDbGUIDCheck go_check(*caster,step.script->datalong);
                MaNGOS::ObjectSearcher<GameObject, MaNGOS::GameObjectWithDbGUIDCheck> checker(door,go_check);

                Cell::VisitGridObjects(caster, checker, GetVisibilityDistance());

                if (!door)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_OPEN_DOOR failed for gameobject(guid: %u).", step.script->datalong);
                    break;
                }
                if (door->GetGoType() != GAMEOBJECT_TYPE_DOOR)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_OPEN_DOOR failed for non-door(GoType: %u).", door->GetGoType());
                    break;
                }

                if (door->GetGoState() != GO_STATE_READY)
                    break;                                  //door already  open

                door->UseDoorOrButton(time_to_close);

                if (target && target->isType(TYPEMASK_GAMEOBJECT) && ((GameObject*)target)->GetGoType()==GAMEOBJECT_TYPE_BUTTON)
                    ((GameObject*)target)->UseDoorOrButton(time_to_close);
                break;
            }
            case SCRIPT_COMMAND_CLOSE_DOOR:
            {
                if (!step.script->datalong)                  // guid for door not specified
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_CLOSE_DOOR call for NULL door.");
                    break;
                }

                if (!source)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_CLOSE_DOOR call for NULL unit.");
                    break;
                }

                if (!source->isType(TYPEMASK_UNIT))          // must be any Unit (creature or player)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_CLOSE_DOOR call for non-unit (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                Unit* caster = (Unit*)source;

                GameObject *door = NULL;
                int32 time_to_open = step.script->datalong2 < 15 ? 15 : (int32)step.script->datalong2;

                MaNGOS::GameObjectWithDbGUIDCheck go_check(*caster,step.script->datalong);
                MaNGOS::ObjectSearcher<GameObject, MaNGOS::GameObjectWithDbGUIDCheck> checker(door,go_check);

                Cell::VisitGridObjects(caster, checker, GetVisibilityDistance());

                if (!door)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_CLOSE_DOOR failed for gameobject(guid: %u).", step.script->datalong);
                    break;
                }

                if (door->GetGoType() != GAMEOBJECT_TYPE_DOOR)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_CLOSE_DOOR failed for non-door(GoType: %u).", door->GetGoType());
                    break;
                }

                if (door->GetGoState() == GO_STATE_READY)
                    break;                                  //door already closed

                door->UseDoorOrButton(time_to_open);

                if (target && target->isType(TYPEMASK_GAMEOBJECT) && ((GameObject*)target)->GetGoType()==GAMEOBJECT_TYPE_BUTTON)
                    ((GameObject*)target)->UseDoorOrButton(time_to_open);

                break;
            }
            case SCRIPT_COMMAND_QUEST_EXPLORED:
            {
                if (!source)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_QUEST_EXPLORED call for NULL source.");
                    break;
                }

                if (!target)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_QUEST_EXPLORED call for NULL target.");
                    break;
                }

                // when script called for item spell casting then target == (unit or GO) and source is player
                WorldObject* worldObject;
                Player* player;

                if (target->GetTypeId()==TYPEID_PLAYER)
                {
                    if (source->GetTypeId()!=TYPEID_UNIT && source->GetTypeId()!=TYPEID_GAMEOBJECT)
                    {
                        sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_QUEST_EXPLORED call for non-creature and non-gameobject (TypeId: %u), skipping.",source->GetTypeId());
                        break;
                    }

                    worldObject = (WorldObject*)source;
                    player = (Player*)target;
                }
                else
                {
                    if (target->GetTypeId()!=TYPEID_UNIT && target->GetTypeId()!=TYPEID_GAMEOBJECT)
                    {
                        sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_QUEST_EXPLORED call for non-creature and non-gameobject (TypeId: %u), skipping.",target->GetTypeId());
                        break;
                    }

                    if (source->GetTypeId()!=TYPEID_PLAYER)
                    {
                        sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_QUEST_EXPLORED call for non-player(TypeId: %u), skipping.",source->GetTypeId());
                        break;
                    }

                    worldObject = (WorldObject*)target;
                    player = (Player*)source;
                }

                // quest id and flags checked at script loading
                if ((worldObject->GetTypeId()!=TYPEID_UNIT || ((Unit*)worldObject)->IsAlive()) &&
                    (step.script->datalong2==0 || worldObject->IsWithinDistInMap(player,float(step.script->datalong2))))
                    player->AreaExploredOrEventHappens(step.script->datalong);
                else
                    player->FailQuest(step.script->datalong);

                break;
            }

            case SCRIPT_COMMAND_ACTIVATE_OBJECT:
            {
                if (!source)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_ACTIVATE_OBJECT must have source caster.");
                    break;
                }

                if (!source->isType(TYPEMASK_UNIT))
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_ACTIVATE_OBJECT source caster isn't unit (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                if (!target)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_ACTIVATE_OBJECT call for NULL gameobject.");
                    break;
                }

                if (target->GetTypeId()!=TYPEID_GAMEOBJECT)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_ACTIVATE_OBJECT call for non-gameobject (TypeId: %u), skipping.",target->GetTypeId());
                    break;
                }

                Unit* caster = (Unit*)source;

                GameObject *go = (GameObject*)target;

                go->Use(caster);
                break;
            }

            case SCRIPT_COMMAND_REMOVE_AURA:
            {
                Object* cmdTarget = step.script->datalong2 ? source : target;

                if (!cmdTarget)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_REMOVE_AURA call for NULL %s.",step.script->datalong2 ? "source" : "target");
                    break;
                }

                if (!cmdTarget->isType(TYPEMASK_UNIT))
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_REMOVE_AURA %s isn't unit (TypeId: %u), skipping.",step.script->datalong2 ? "source" : "target",cmdTarget->GetTypeId());
                    break;
                }

                ((Unit*)cmdTarget)->RemoveAurasDueToSpell(step.script->datalong);
                break;
            }

            case SCRIPT_COMMAND_CAST_SPELL:
            {
                Object* cmdTarget = NULL;
                Object* cmdSource = NULL;
                switch (step.script->datalong2)
                {
                case 0:
                    cmdTarget = target; cmdSource = source; break;
                case 1:
                    cmdTarget = source; cmdSource = source; break;
                case 2:
                    cmdTarget = target; cmdSource = target; break;
                case 3:
                    cmdTarget = source; cmdSource = target; break;
                }

                if (!cmdSource)
                {
                    sLog.outDebug("SCRIPT_COMMAND_CAST_SPELL must have source caster.");
                    break;
                }
                if (!cmdSource->isType(TYPEMASK_UNIT))
                {
                    sLog.outDebug("SCRIPT_COMMAND_CAST_SPELL source caster isn't unit (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }
                if (!cmdTarget)
                {
                    sLog.outDebug("SCRIPT_COMMAND_CAST_SPELL call for NULL %s.",step.script->datalong2 ? "source" : "target");
                    break;
                }
                if (!cmdTarget->isType(TYPEMASK_UNIT))
                {
                    sLog.outDebug("SCRIPT_COMMAND_CAST_SPELL %s isn't unit (TypeId: %u), skipping.",step.script->datalong2 ? "source" : "target",cmdTarget->GetTypeId());
                    break;
                }

                ((Unit*)cmdSource)->CastSpell(((Unit*)cmdTarget), step.script->datalong, step.script->dataint ? true : false);

                break;
            }

            case SCRIPT_COMMAND_LOAD_PATH:
            {
                if (!source)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_START_MOVE is tried to apply to NON-existing unit.");
                    break;
                }

                if (!source->isType(TYPEMASK_UNIT))
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_START_MOVE source mover isn't unit (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                if (!sWaypointMgr.GetPath(step.script->datalong))
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_START_MOVE source mover has an invallid path, skipping.");
                    break;
                }

                dynamic_cast<Unit*>(source)->GetMotionMaster()->MovePath(step.script->datalong, step.script->datalong2);
                break;
            }

            case SCRIPT_COMMAND_CALLSCRIPT_TO_UNIT:
            {
                if (!step.script->datalong || !step.script->datalong2)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_CALLSCRIPT calls invallid db_script_id or lowguid not present: skipping.");
                    break;
                }
                //our target
                Creature* target = NULL;

                if (source) //using grid searcher
                {
                    //sLog.outDebug("Attempting to find Creature: Db GUID: %i", step.script->datalong);
                    MaNGOS::CreatureWithDbGUIDCheck target_check(((Unit*)source), step.script->datalong);
                    MaNGOS::ObjectSearcher<Creature, MaNGOS::CreatureWithDbGUIDCheck> checker(target,target_check);

                    Cell::VisitGridObjects((Unit*)source, checker, GetVisibilityDistance());
                }
                else //check hashmap holders
                {
                    if (CreatureData const* data = sObjectMgr.GetCreatureData(step.script->datalong))
                        target = GetCreature(MAKE_NEW_GUID(step.script->datalong, data->id, HIGHGUID_UNIT), data->posX, data->posY);
                }
                //sLog.outDebug("attempting to pass target...");
                if (!target)
                    break;
                //sLog.outDebug("target passed");
                //Lets choose our ScriptMap map
                ScriptMapMap *datamap = NULL;
                switch (step.script->dataint)
                {
                    case 1://QUEST END SCRIPTMAP
                        datamap = &sQuestEndScripts;
                        break;
                    case 2://QUEST START SCRIPTMAP
                        datamap = &sQuestStartScripts;
                        break;
                    case 3://SPELLS SCRIPTMAP
                        datamap = &sSpellScripts;
                        break;
                    case 4://GAMEOBJECTS SCRIPTMAP
                        datamap = &sGameObjectScripts;
                        break;
                    case 5://EVENTS SCRIPTMAP
                        datamap = &sEventScripts;
                        break;
                    case 6://WAYPOINTS SCRIPTMAP
                        datamap = &sWaypointScripts;
                        break;
                    default:
                        sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_CALLSCRIPT ERROR: no scriptmap present... ignoring");
                        break;
                }
                //if no scriptmap present...
                if (!datamap)
                    break;

                uint32 script_id = step.script->datalong2;
                //insert script into schedule but do not start it
                ScriptsStart(*datamap, script_id, target, NULL/*, false*/);
                break;
            }

            case SCRIPT_COMMAND_PLAY_SOUND:
            {
                if (!source)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_PLAY_SOUND call for NULL creature.");
                    break;
                }

                WorldObject* pSource = dynamic_cast<WorldObject*>(source);
                if (!pSource)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_PLAY_SOUND call for non-world object (TypeId: %u), skipping.",source->GetTypeId());
                    break;
                }

                // bitmask: 0/1=anyone/target, 0/2=with distance dependent
                Player* pTarget = NULL;
                if (step.script->datalong2 & 1)
                {
                    if (!target)
                    {
                        sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_PLAY_SOUND in targeted mode call for NULL target.");
                        break;
                    }

                    if (target->GetTypeId()!=TYPEID_PLAYER)
                    {
                        sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_PLAY_SOUND in targeted mode call for non-player (TypeId: %u), skipping.",target->GetTypeId());
                        break;
                    }

                    pTarget = (Player*)target;
                }

                // bitmask: 0/1=anyone/target, 0/2=with distance dependent
                if (step.script->datalong2 & 2)
                    pSource->PlayDistanceSound(step.script->datalong, pTarget);
                else
                    pSource->PlayDirectSound(step.script->datalong, pTarget);
                break;
            }

            case SCRIPT_COMMAND_KILL:
            {
                if (!source || ((Creature*)source)->IsDead())
                    break;

                switch (step.script->datalong)
                {
                    default: // backward compatibility (defaults to 0)
                    case 0: // source kills source
                        ((Creature*)source)->DealDamage(((Creature*)source), ((Creature*)source)->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                        break;
                    case 1: // target kills source
                        ((Creature*)target)->DealDamage(((Creature*)source), ((Creature*)source)->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                        break;
                    case 2: // source kills target
                        if (target)
                            ((Creature*)source)->DealDamage(((Creature*)target), ((Creature*)target)->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                        break;
                }


                switch (step.script->dataint)
                {
                case 0: break; //return false not remove corpse
                case 1: ((Creature*)source)->RemoveCorpse(); break;
                }
                break;
            }
            case SCRIPT_COMMAND_SET_INST_DATA:
            {
                if (!source)
                    break;

                InstanceData* pInst = (InstanceData*)((WorldObject*)source)->GetInstanceData();
                if (!pInst)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: SCRIPT_COMMAND_SET_INST_DATA %d attempt to set instance data without instance script.", step.script->id);
                    return;
                }

                pInst->SetData(step.script->datalong, step.script->datalong2);
                break;
            }

            default:
                sLog.outLog(LOG_DEFAULT, "ERROR: Unknown script command %u called.",step.script->command);
                break;
        }

        m_scriptSchedule.erase(iter);
        sWorld.DecreaseScheduledScriptCount();

        iter = m_scriptSchedule.begin();
    }
}


template void Map::Add(Corpse *);
template void Map::Add(Creature *);
template void Map::Add(GameObject *);
template void Map::Add(DynamicObject *);

template void Map::Remove(Corpse *,bool);
template void Map::Remove(Creature *,bool);
template void Map::Remove(GameObject *, bool);
template void Map::Remove(DynamicObject *, bool);

/* ******* Dungeon Instance Maps ******* */

InstanceMap::InstanceMap(uint32 id, time_t expiry, uint32 InstanceId, uint8 SpawnMode)
  : Map(id, expiry, InstanceId, SpawnMode), i_data(NULL),
    m_resetAfterUnload(false), m_unloadWhenEmpty(false), m_unlootedCreaturesSummoned(false)
{

    InstanceMap::InitVisibilityDistance();

    // the timer is started by default, and stopped when the first player joins
    // this make sure it gets unloaded if for some reason no player joins
    m_unloadTimer.Reset(std::max(sWorld.getConfig(CONFIG_INSTANCE_UNLOAD_DELAY), (uint32)MIN_UNLOAD_DELAY));
}

InstanceMap::~InstanceMap()
{
    if (i_data)
    {
        delete i_data;
        i_data = NULL;
    }
}

void InstanceMap::InitVisibilityDistance()
{
    m_ActiveObjectUpdateDistance = sWorld.GetActiveObjectUpdateDistanceInInstances();
}

/*
    Do map specific checks to see if the player can enter
*/
bool InstanceMap::EncounterInProgress(Player *player)
{
    bool inProgress = GetInstanceData() && GetInstanceData()->IsEncounterInProgress();
    if (player && inProgress)
    {
        if (player->IsGameMaster())
            return false;

        sLog.outDebug("InstanceMap: Player '%s' can't enter instance '%s' while an encounter is in progress.", player->GetName(),GetMapName());
        player->SendTransferAborted(GetId(),TRANSFER_ABORT_ZONE_IN_COMBAT);
    }
    return inProgress;
}

bool InstanceMap::CanEnter(Player *player)
{
    if (player->GetMapRef().getTarget() == this)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: InstanceMap::CanEnter - player %s(%u) already in map %d,%d,%d!", player->GetName(), player->GetGUIDLow(), GetId(), GetInstanceId(), GetSpawnMode());
        ASSERT(false);
        return false;
    }

    // cannot enter if the instance is full (player cap), GMs don't count
    uint32 maxPlayers = GetMaxPlayers();
    if (!player->IsGameMaster() && GetPlayersCountExceptGMs() >= maxPlayers)
    {
        sLog.outDetail("MAP: Instance '%u' of map '%s' cannot have more than '%u' players. Player '%s' rejected", GetInstanceId(), GetMapName(), maxPlayers, player->GetName());
        player->SendTransferAborted(GetId(), TRANSFER_ABORT_MAX_PLAYERS);
        return false;
    }

    if (EncounterInProgress(player))
        return false;

    return Map::CanEnter(player);
}

/*
    Do map specific checks and add the player to the map if successful.
*/
bool InstanceMap::Add(Player *player)
{
    // TODO: Not sure about checking player level: already done in HandleAreaTriggerOpcode
    // GMs still can teleport player in instance.
    // Is it needed?

    {
        ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, Lock, false);
        if (!CanEnter(player))
            return false;

        // Dungeon only code
        if(IsDungeon())
        {
            // get or create an instance save for the map
            InstanceSave *mapSave = sInstanceSaveManager.GetInstanceSave(GetInstanceId());
            if(!mapSave)
            {
                sLog.outDetail("InstanceMap::Add: creating instance save for map %d spawnmode %d with instance id %d", GetId(), GetSpawnMode(), GetInstanceId());
                mapSave = sInstanceSaveManager.AddInstanceSave(GetId(), GetInstanceId(), GetSpawnMode(), 0, true);
            }
            // check for existing instance binds
            InstancePlayerBind *playerBind = player->GetBoundInstance(GetId(), GetSpawnMode());
            if(playerBind && playerBind->perm)
            {
                // cannot enter other instances if bound permanently
                if(playerBind->save != mapSave)
                {
                    sLog.outDebug("ERROR: InstanceMap::Add: player %s(%d) is permanently bound to instance %d,%d,%d,%d,%d,%d but he is being put in instance %d,%d,%d,%d,%d,%d", player->GetName(), player->GetGUIDLow(), playerBind->save->GetMapId(), playerBind->save->GetInstanceId(), playerBind->save->GetDifficulty(), playerBind->save->GetPlayerCount(), playerBind->save->GetGroupCount(), playerBind->save->CanReset(), mapSave->GetMapId(), mapSave->GetInstanceId(), mapSave->GetDifficulty(), mapSave->GetPlayerCount(), mapSave->GetGroupCount(), mapSave->CanReset());
                    return false;
                    //ASSERT(false);
                }
            }
            else
            {
                Group *pGroup = player->GetGroup();
                if (pGroup)
                {
                    // solo saves should be reset when entering a group
                    InstanceGroupBind *groupBind = pGroup->GetBoundInstance(GetId(), GetSpawnMode());
                    if (playerBind)
                    {
                        sLog.outDebug("ERROR: InstanceMap::Add: player %s(%d) is being put in instance %d,%d,%d,%d,%d,%d but he is in group %d and is bound to instance %d,%d,%d,%d,%d,%d!", player->GetName(), player->GetGUIDLow(), mapSave->GetMapId(), mapSave->GetInstanceId(), mapSave->GetDifficulty(), mapSave->GetPlayerCount(), mapSave->GetGroupCount(), mapSave->CanReset(), GUID_LOPART(pGroup->GetLeaderGUID()), playerBind->save->GetMapId(), playerBind->save->GetInstanceId(), playerBind->save->GetDifficulty(), playerBind->save->GetPlayerCount(), playerBind->save->GetGroupCount(), playerBind->save->CanReset());
                        if (groupBind)
                            sLog.outDebug("ERROR: InstanceMap::Add: the group is bound to instance %d,%d,%d,%d,%d,%d", groupBind->save->GetMapId(), groupBind->save->GetInstanceId(), groupBind->save->GetDifficulty(), groupBind->save->GetPlayerCount(), groupBind->save->GetGroupCount(), groupBind->save->CanReset());
                        sLog.outDebug("ERROR: InstanceMap::Add: do not let player %s enter instance otherwise crash will happen", player->GetName());
                        return false;
                        // sadly some crashes here, need to check those logs
                        // player->UnbindInstance(GetId(), GetSpawnMode());
                    }
                    // bind to the group or keep using the group save
                    if (!groupBind)
                        pGroup->BindToInstance(mapSave, false);
                    else
                    {
                        // cannot jump to a different instance without resetting it
                        if(groupBind->save != mapSave)
                        {
                            sLog.outLog(LOG_DEFAULT, "ERROR: InstanceMap::Add: player %s(%d) is being put in instance %d,%d,%d but he is in group %d which is bound to instance %d,%d,%d!", player->GetName(), player->GetGUIDLow(), mapSave->GetMapId(), mapSave->GetInstanceId(), mapSave->GetDifficulty(), GUID_LOPART(pGroup->GetLeaderGUID()), groupBind->save->GetMapId(), groupBind->save->GetInstanceId(), groupBind->save->GetDifficulty());
                            if(mapSave)
                                sLog.outLog(LOG_DEFAULT, "ERROR: MapSave players: %d, group count: %d", mapSave->GetPlayerCount(), mapSave->GetGroupCount());
                            else
                                sLog.outLog(LOG_DEFAULT, "ERROR: MapSave NULL");
                            if(groupBind->save)
                                sLog.outLog(LOG_DEFAULT, "ERROR: GroupBind save players: %d, group count: %d", groupBind->save->GetPlayerCount(), groupBind->save->GetGroupCount());
                            else
                                sLog.outLog(LOG_DEFAULT, "ERROR: GroupBind save NULL");
                            ASSERT(false);
                        }
                        // if the group/leader is permanently bound to the instance
                        // players also become permanently bound when they enter
                        if (groupBind->perm)
                        {
                            Player* leader = player->GetPlayer(player->GetGroup()->GetLeaderGUID());
                            //accept binding only when leader is in the instance to prevent joining
                            //wrong instance ids
                            if (leader && leader->GetMapId() == groupBind->save->GetMapId())
                            {
                                WorldPacket data(SMSG_INSTANCE_SAVE_CREATED, 4);
                                data << uint32(0);
                                player->SendPacketToSelf(&data);
                                player->BindToInstance(mapSave, true);
                                if (IsRaid())
                                    sLog.outLog(LOG_RAID_BINDS,"Player %s (%u) recived bind %u:%u from %s (%u)",
                                    player->GetName(),player->GetGUIDLow(),GetId(),GetInstanceId(),leader->GetName(), leader->GetGUIDLow());
                            }
                            else
                            {
                                return false;
                            }
                        }
                    }
                }
                else
                {
                    // set up a solo bind or continue using it
                    if (!playerBind)
                        player->BindToInstance(mapSave, false);
                    else
                        // cannot jump to a different instance without resetting it
                        ASSERT(playerBind->save == mapSave);
                }
            }

            if (i_data) i_data->OnPlayerEnter(player);
            // for normal instances cancel the reset schedule when the
            // first player enters (no players yet)
            SetResetSchedule(false);

            player->SendInitWorldStates();
            sLog.outDetail("MAP: Player '%s' entered the instance '%u' of map '%s'", player->GetName(), GetInstanceId(), GetMapName());
            // initialize unload state
            m_unloadTimer = 0;
            m_resetAfterUnload = false;
            m_unloadWhenEmpty = false;
        }


        // get or create an instance save for the map
        InstanceSave *mapSave = sInstanceSaveManager.GetInstanceSave(GetInstanceId());
        if (!mapSave)
        {
            sLog.outDetail("InstanceMap::Add: creating instance save for map %d spawnmode %d with instance id %d", GetId(), GetSpawnMode(), GetInstanceId());
            mapSave = sInstanceSaveManager.AddInstanceSave(GetId(), GetInstanceId(), GetSpawnMode(), 0, true);
        }
    }

    // this will acquire the same mutex so it cannot be in the previous block
    Map::Add(player);
    return true;
}

void InstanceMap::Update(const uint32& t_diff)
{
    Map::Update(t_diff);

    if (i_data)
        i_data->Update(t_diff);

    if (!m_unlootedCreaturesSummoned)
        SummonUnlootedCreatures();
}

void InstanceMap::Remove(Player *player, bool remove)
{
    sLog.outDetail("MAP: Removing player '%s' from instance '%u' of map '%s' before relocating to other map", player->GetName(), GetInstanceId(), GetMapName());
    //if last player set unload timer
    if (!m_unloadTimer.GetTimeLeft() && m_mapRefManager.getSize() == 1)
        m_unloadTimer.Reset(m_unloadWhenEmpty ? MIN_UNLOAD_DELAY : std::max(sWorld.getConfig(CONFIG_INSTANCE_UNLOAD_DELAY), (uint32)MIN_UNLOAD_DELAY));
    Map::Remove(player, remove);
    // for normal instances schedule the reset after all players have left
    SetResetSchedule(true);
}

void InstanceMap::CreateInstanceData(bool load)
{
    if (i_data != NULL)
        return;

    InstanceTemplate const* mInstance = ObjectMgr::GetInstanceTemplate(GetId());
    if (mInstance)
    {
        i_script_id = mInstance->script_id;
        i_data = sScriptMgr.CreateInstanceData(this);
    }

    if (!i_data)
        return;

    i_data->Initialize();

    if (load)
    {
        // TODO: make a global storage for this
        QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT data FROM instance WHERE map = '%u' AND id = '%u'", GetId(), i_InstanceId);
        if (result)
        {
            Field* fields = result->Fetch();
            const char* data = fields[0].GetString();
            if (data && data != "")
            {
                sLog.outDebug("Loading instance data for `%s` with id %u", sScriptMgr.GetScriptName(i_script_id), i_InstanceId);
                i_data->Load(data);
            }
        }
    }
}

/*
    Returns true if there are no players in the instance
*/
bool InstanceMap::Reset(uint8 method)
{
    // note: since the map may not be loaded when the instance needs to be reset
    // the instance must be deleted from the DB by InstanceSaveManager

    if (HavePlayers())
    {
        if (method == INSTANCE_RESET_ALL || method == INSTANCE_RESET_CHANGE_DIFFICULTY)
        {
            // notify the players to leave the instance so it can be reset
            for (MapRefManager::iterator itr = m_mapRefManager.begin(); itr != m_mapRefManager.end(); ++itr)
                itr->getSource()->SendResetFailedNotify(GetId());
        }
        else
        {
            if (method == INSTANCE_RESET_GLOBAL)
            {
                // set the homebind timer for players inside (1 minute)
                for (MapRefManager::iterator itr = m_mapRefManager.begin(); itr != m_mapRefManager.end(); ++itr)
                    itr->getSource()->m_InstanceValid = false;
            }

            // the unload timer is not started
            // instead the map will unload immediately after the players have left
            m_unloadWhenEmpty = true;
            m_resetAfterUnload = true;
        }
    }
    else
    {
        // unloaded at next update
        m_unloadTimer.Reset(MIN_UNLOAD_DELAY);
        m_resetAfterUnload = true;
    }

    return m_mapRefManager.isEmpty();
}

void InstanceMap::PermBindAllPlayers(Player *player)
{
    if (!IsDungeon())
        return;

    InstanceSave *save = sInstanceSaveManager.GetInstanceSave(GetInstanceId());
    if (!save)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Cannot bind players, no instance save available for map!\n");
        return;
    }

    Group *group = player->GetGroup();
    std::ostringstream str;
    str << "Creating bind to " << GetId() << ":" << GetInstanceId() << " ";
    if (group)
        str << "Leader " << group->GetLeaderName() << " (" << group->GetLeaderGUID() << ")";
    // group members outside the instance group don't get bound
    for (MapRefManager::iterator itr = m_mapRefManager.begin(); itr != m_mapRefManager.end(); ++itr)
    {
        Player* plr = itr->getSource();
        // players inside an instance cannot be bound to other instances
        // some players may already be permanently bound, in this case nothing happens
        InstancePlayerBind *bind = plr->GetBoundInstance(save->GetMapId(), save->GetDifficulty());
        if (!bind || !bind->perm)
        {
            str << plr->GetGUIDLow() << "; ";
            plr->BindToInstance(save, true);
            WorldPacket data(SMSG_INSTANCE_SAVE_CREATED, 4);
            data << uint32(0);
            plr->SendPacketToSelf(&data);
        }

        // if the leader is not in the instance the group will not get a perm bind
        if (group && group->GetLeaderGUID() == plr->GetGUID())
            group->BindToInstance(save, true);
    }
    if (IsRaid())
        sLog.outLog(LOG_RAID_BINDS, "%s", str.str().c_str());
}

void InstanceMap::UnloadAll()
{
    if (HavePlayers())
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: InstanceMap::UnloadAll: there are still players in the instance at unload, should not happen!");
        for (MapRefManager::iterator itr = m_mapRefManager.begin(); itr != m_mapRefManager.end(); ++itr)
        {
            Player* plr = itr->getSource();
            plr->TeleportToHomebind();
        }
    }

    if (m_resetAfterUnload == true)
        sObjectMgr.DeleteRespawnTimeForInstance(GetInstanceId());

    Map::UnloadAll();
}

void InstanceMap::SendResetWarnings(uint32 timeLeft) const
{
    for (MapRefManager::const_iterator itr = m_mapRefManager.begin(); itr != m_mapRefManager.end(); ++itr)
        itr->getSource()->SendInstanceResetWarning(GetId(), timeLeft);
}

void InstanceMap::SetResetSchedule(bool on)
{
    // only for normal instances
    // the reset time is only scheduled when there are no players inside
    // it is assumed that the reset time will rarely (if ever) change while the reset is scheduled
    if (IsDungeon() && !HavePlayers() && !IsRaid() && !IsHeroic())
    {
        InstanceSave *save = sInstanceSaveManager.GetInstanceSave(GetInstanceId());
        if (!save)
            sLog.outLog(LOG_DEFAULT, "ERROR: InstanceMap::SetResetSchedule: cannot turn schedule %s, no save available for instance %d (mapid: %d)", on ? "on" : "off", GetInstanceId(), GetId());
        else
            sInstanceSaveManager.ScheduleReset(on, save->GetResetTime(), InstanceSaveManager::InstResetEvent(0, GetId(), GetInstanceId()));
    }
}

void InstanceMap::SummonUnlootedCreatures()
{
    m_unlootedCreaturesSummoned = true;
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT DISTINCT creatureId, position_x, position_y, position_z FROM group_saved_loot WHERE instanceId='%u' AND summoned = TRUE", GetInstanceId());
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 creatureId = fields[0].GetUInt32();
            float pos_x = fields[1].GetFloat();
            float pos_y = fields[2].GetFloat();
            float pos_z = fields[3].GetFloat();

            TemporarySummon* pCreature = new TemporarySummon();
            if (!pCreature->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_UNIT), this, creatureId, 0, pos_x, pos_y, pos_z, 0))
            {
                delete pCreature;
                continue;
            }
            pCreature->Summon(TEMPSUMMON_MANUAL_DESPAWN, 0);
            pCreature->loot.FillLootFromDB(pCreature, NULL);
        }
        while (result->NextRow());
    }
}

uint32 InstanceMap::GetMaxPlayers() const
{
    InstanceTemplate const* iTemplate = sObjectMgr.GetInstanceTemplate(GetId());
    if(!iTemplate)
        return 0;
    return iTemplate->maxPlayers;
}

/* ******* Battleground Instance Maps ******* */

BattleGroundMap::BattleGroundMap(uint32 id, time_t expiry, uint32 InstanceId, BattleGround *bg)
    : Map(id, expiry, InstanceId, DIFFICULTY_NORMAL)
{
    m_bg = bg;
    BattleGroundMap::InitVisibilityDistance();
}

BattleGroundMap::~BattleGroundMap()
{
}

void BattleGroundMap::InitVisibilityDistance()
{
    m_ActiveObjectUpdateDistance = sWorld.GetActiveObjectUpdateDistanceInInstances();
}

bool BattleGroundMap::CanEnter(Player * player)
{
    if (player->GetMapRef().getTarget() == this)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: BGMap::CanEnter - player %u already in map!", player->GetGUIDLow());
        ASSERT(false);
        return false;
    }

    if (player->GetBattleGroundId() != GetInstanceId())
        return false;

    // player number limit is checked in bgmgr, no need to do it here

    return Map::CanEnter(player);
}

void BattleGroundMap::Update(const uint32& t_diff)
{
    Map::Update(t_diff);

    if (m_bg)
        m_bg->Update(time_t(t_diff));
}

bool BattleGroundMap::Add(Player * player)
{
    {
        ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, Lock, false);
        if (!CanEnter(player))
            return false;
        // reset instance validity, battleground maps do not homebind
        player->m_InstanceValid = true;
    }
    return Map::Add(player);
}

void BattleGroundMap::Remove(Player *player, bool remove)
{
    sLog.outDetail("MAP: Removing player '%s' from bg '%u' of map '%s' before relocating to other map", player->GetName(), GetInstanceId(), GetMapName());
    Map::Remove(player, remove);
}

void BattleGroundMap::SetUnload()
{
    m_unloadTimer.Reset(MIN_UNLOAD_DELAY);
}

void BattleGroundMap::UnloadAll()
{
    while (HavePlayers())
    {
        if (Player * plr = m_mapRefManager.getFirst()->getSource())
        {
            plr->TeleportToHomebind();
            // TeleportTo removes the player from this map (if the map exists) -> calls BattleGroundMap::Remove -> invalidates the iterator.
            // just in case, remove the player from the list explicitly here as well to prevent a possible infinite loop
            // note that this remove is not needed if the code works well in other places
            plr->GetMapRef().unlink();
        }
    }

    Map::UnloadAll();
}

Creature * Map::GetCreature(uint64 guid)
{
    CreaturesMapType::const_iterator a = creaturesMap.find(guid);

    if (a != creaturesMap.cend())
    {
        if (a->second->GetInstanceId() != GetInstanceId())
            return NULL;
        else
            return a->second;
    }

    return NULL;
}

Creature * Map::GetCreature(uint64 guid, float x, float y)
{
    CreaturesMapType::const_iterator a = creaturesMap.find(guid);

    if (a != creaturesMap.cend())
    {
        CellPair p = MaNGOS::ComputeCellPair(x,y);
        if (p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Map::GetCorpse: invalid coordinates supplied X:%f Y:%f grid cell [%u:%u]", x, y, p.x_coord, p.y_coord);
            return NULL;
        }

        CellPair q = MaNGOS::ComputeCellPair(a->second->GetPositionX(), a->second->GetPositionY());
        if (q.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || q.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Map::GetCorpse: object " UI64FMTD " has invalid coordinates X:%f Y:%f grid cell [%u:%u]", a->second->GetGUID(), a->second->GetPositionX(), a->second->GetPositionY(), q.x_coord, q.y_coord);
            return NULL;
        }

        int32 dx = int32(p.x_coord) - int32(q.x_coord);
        int32 dy = int32(p.y_coord) - int32(q.y_coord);

        if (dx > -2 && dx < 2 && dy > -2 && dy < 2)
            return a->second;
        else
            return NULL;
    }

    return NULL;
}


Creature * Map::GetCreatureById(uint32 id, GetCreatureGuidType type)
{
    return GetCreature(GetCreatureGUID(id, type));
}

Creature * Map::GetCreatureOrPet(uint64 guid)
{
    if (IS_PLAYER_GUID(guid))
        return NULL;

    if (IS_PET_GUID(guid))
        return ObjectAccessor::GetPet(guid);

    return GetCreature(guid);
}

GameObject * Map::GetGameObject(uint64 guid)
{
    GObjectMapType::const_iterator a = gameObjectsMap.find(guid);

    if (a != gameObjectsMap.cend())
    {
        if (a->second->GetInstanceId() != GetInstanceId())
            return NULL;
        else
            return a->second;
    }

    return NULL;
}

DynamicObject * Map::GetDynamicObject(uint64 guid)
{
    DObjectMapType::const_iterator a = dynamicObjectsMap.find(guid);

    if (a != dynamicObjectsMap.cend())
    {
        if (a->second->GetInstanceId() != GetInstanceId())
            return NULL;
        else
            return a->second;
    }

    return NULL;
}

Unit * Map::GetUnit(uint64 guid)
{
    if (!guid)
        return NULL;

    if (IS_PLAYER_GUID(guid))
        return ObjectAccessor::FindPlayer(guid);

    return GetCreatureOrPet(guid);
}

Object* Map::GetObjectByTypeMask(Player const &p, uint64 guid, uint32 typemask)
{
    Object *obj = NULL;

    if (typemask & TYPEMASK_PLAYER)
    {
        obj = ObjectAccessor::FindPlayer(guid);
        if (obj) return obj;
    }

    if (typemask & TYPEMASK_UNIT)
    {
        obj = GetCreatureOrPet(guid);
        if (obj) return obj;
    }

    if (typemask & TYPEMASK_GAMEOBJECT)
    {
        obj = GetGameObject(guid);
        if (obj) return obj;
    }

    if (typemask & TYPEMASK_DYNAMICOBJECT)
    {
        obj = GetDynamicObject(guid);
        if (obj) return obj;
    }

    if (typemask & TYPEMASK_ITEM)
    {
        obj = p.GetItemByGuid(guid);
        if (obj) return obj;
    }

    return NULL;
}

void Map::VisibilityOfCreatureEntry(uint32 entry, bool hide)
{
    CreatureIdToGuidListMapType::const_iterator a = creatureIdToGuidMap.find(entry);
    if (a == creatureIdToGuidMap.cend())
        return;

    std::list<uint64> tmpList = a->second;
    for (std::list<uint64>::iterator itr = tmpList.begin(); itr != tmpList.end(); itr++)
    {
        Creature* mob = GetCreature(*itr);
        if (!mob)
            continue;
        mob->CombatStop();
        mob->Respawn();
        if (hide)
        {
            mob->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            mob->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
            mob->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
            mob->SetVisibility(VISIBILITY_OFF);
        }
        else
        {
            mob->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            mob->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
            mob->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
            mob->SetVisibility(VISIBILITY_ON);
        }
    }

}


std::list<uint64> Map::GetCreaturesGUIDList(uint32 id, GetCreatureGuidType type , uint32 max)
{
    std::list<uint64> returnList;
    CreatureIdToGuidListMapType::const_iterator a = creatureIdToGuidMap.find(id);
    if (a != creatureIdToGuidMap.cend())
    {
        std::list<uint64> tmpList = a->second;

        if (!max || max > tmpList.size())
        {
            max = tmpList.size();
            if (type == GET_RANDOM_CREATURE_GUID)
                type = GET_FIRST_CREATURE_GUID;
        }
        uint64 count = 0;
        switch (type)
        {
            case GET_FIRST_CREATURE_GUID:
                for (std::list<uint64>::iterator itr = tmpList.begin(); count != max; ++itr, ++count)
                    returnList.push_back(*itr);
                break;
            case GET_LAST_CREATURE_GUID:
                for (std::list<uint64>::reverse_iterator itr = tmpList.rbegin(); count != max; ++itr, ++count)
                    returnList.push_back(*itr);
                break;
            case GET_RANDOM_CREATURE_GUID:
                for (count = 0; count != max; ++count)
                {
                    std::list<uint64>::iterator itr = tmpList.begin();
                    std::advance(itr, rand()%(tmpList.size()-1));
                    returnList.push_back(*itr);
                    tmpList.erase(itr);
                }
                break;
            case GET_ALIVE_CREATURE_GUID:
            {
                for (std::list<uint64>::iterator itr = tmpList.begin(); itr != tmpList.end(); ++itr)
                {
                    Creature * tmpC = GetCreature(*itr);
                    if (tmpC && tmpC->IsAlive())
                    {
                        returnList.push_back(*itr);
                        ++count;
                    }

                    if (count == max)
                        break;
                }
                break;
            }
            default:
                break;
        }
    }

    return returnList;
}

uint64 Map::GetCreatureGUID(uint32 id, GetCreatureGuidType type)
{
    uint64 returnGUID = 0;

    CreatureIdToGuidListMapType::const_iterator a = creatureIdToGuidMap.find(id);
    if (a != creatureIdToGuidMap.cend())
    {
        switch (type)
        {
            case GET_FIRST_CREATURE_GUID:
                returnGUID = a->second.front();
                break;
            case GET_LAST_CREATURE_GUID:
                returnGUID = a->second.back();
                break;
            case GET_RANDOM_CREATURE_GUID:
            {
                std::list<uint64>::const_iterator itr= a->second.begin();
                std::advance(itr, urand(0, a->second.size()-1));
                returnGUID = *itr;
                break;
            }
            case GET_ALIVE_CREATURE_GUID:
            {
                for (std::list<uint64>::const_iterator itr = a->second.begin(); itr != a->second.end(); ++itr)
                {
                    Creature * tmpC = GetCreature(*itr);
                    if (tmpC && tmpC->IsAlive())
                    {
                        returnGUID = *itr;
                        break;
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    return returnGUID;
}

void Map::InsertIntoCreatureGUIDList(Creature * obj)
{
    auto a = creatureIdToGuidMap.insert(std::make_pair(obj->GetEntry(), std::list<uint64>()));
    if (a.second)
    {
        std::list<uint64> tmp;
        tmp.push_back(obj->GetGUID());
        a.first->second = tmp;
    }
    else
    {
        auto b = creatureIdToGuidMap.find(obj->GetEntry());
        if (b != creatureIdToGuidMap.end())
            b->second.push_back(obj->GetGUID());
    }
}

void Map::RemoveFromCreatureGUIDList(Creature * obj)
{
    CreatureIdToGuidListMapType::iterator a = creatureIdToGuidMap.find(obj->GetEntry());
    if (a != creatureIdToGuidMap.end())
        a->second.remove(obj->GetGUID());
}


void Map::InsertIntoObjMap(Object * obj)
{
    ObjectGuid guid(obj->GetGUID());

    switch (guid.GetHigh())
    {
        case HIGHGUID_UNIT:
            {
                auto a = creaturesMap.insert(std::make_pair(guid.GetRawValue(), (Creature*)obj));

                if (a.second)
                {
                    InsertIntoCreatureGUIDList(a.first->second);
                }
                else
                    error_log("Map::InsertIntoCreatureMap: GUID %lu already in map", guid.GetRawValue());

                break;
            }
        case HIGHGUID_GAMEOBJECT:
            {
                auto a = gameObjectsMap.insert(std::make_pair(guid.GetRawValue(), (GameObject*)obj));

                if (!a.second)
                    error_log("Map::InsertIntoGameObjectMap: GUID %lu already in map", guid.GetRawValue());

                break;
            }
        case HIGHGUID_DYNAMICOBJECT:
            {
                auto a = dynamicObjectsMap.insert(std::make_pair(guid.GetRawValue(), (DynamicObject*)obj));

                if (!a.second)
                    error_log("Map::InsertIntoDynamicObjectMap: GUID %lu already in map", guid.GetRawValue());

                break;
            }
        case HIGHGUID_PET:
            sObjectAccessor.AddPet((Pet*)obj);
            break;

        case HIGHGUID_PLAYER:
            sObjectAccessor.AddPlayer((Player*)obj);
            break;

        case HIGHGUID_CORPSE:
            sObjectAccessor.AddCorpse((Corpse*)obj);
            break;
        default:
            break;
    }
}

void Map::RemoveFromObjMap(uint64 guid)
{
    ObjectGuid objGuid(guid);

    switch (objGuid.GetHigh())
    {
        case HIGHGUID_UNIT:
            if (!creaturesMap.erase(guid))
                error_log("Map::RemoveFromCreatureMap: Creature GUID %lu not in map", guid);
            break;

        case HIGHGUID_GAMEOBJECT:
            if (!gameObjectsMap.erase(guid))
                error_log("Map::RemoveFromGameObjectMap: Game Object GUID %lu not in map", guid);
            break;

        case HIGHGUID_DYNAMICOBJECT:
            if (!dynamicObjectsMap.erase(guid))
                error_log("Map::RemoveFromDynamicObjectMap: Dynamic Object GUID %lu not in map", guid);
            break;

        case HIGHGUID_PET:
            sObjectAccessor.RemovePet(guid);
            break;

        case HIGHGUID_CORPSE:
            HashMapHolder<Corpse>::Remove(guid);
            break;
        default:
            break;
    }
}

void Map::RemoveFromObjMap(Object * obj)
{
    ObjectGuid objGuid(obj->GetGUID());

    switch (objGuid.GetHigh())
    {
        case HIGHGUID_UNIT:
        {
            CreaturesMapType::iterator a = creaturesMap.find(objGuid.GetRawValue());
            RemoveFromCreatureGUIDList((Creature*)obj);
            if (a != creaturesMap.end())
                creaturesMap.erase(a);
            else
                sLog.outLog(LOG_DEFAULT, "ERROR: Map::RemoveFromCreatureMap: Creature GUID Low %u not in map", objGuid.GetCounter());
            break;
        }
        case HIGHGUID_GAMEOBJECT:
        {
            GObjectMapType::iterator a = gameObjectsMap.find(objGuid.GetRawValue());
            if (a != gameObjectsMap.end())
                gameObjectsMap.erase(a);
            else
                sLog.outLog(LOG_DEFAULT, "ERROR: Map::RemoveFromGameObjectMap: Game Object GUID Low %u not in map", objGuid.GetCounter());
            break;
        }
        case HIGHGUID_DYNAMICOBJECT:
        {
            DObjectMapType::iterator a = dynamicObjectsMap.find(objGuid.GetRawValue());
            if (a != dynamicObjectsMap.end())
                dynamicObjectsMap.erase(a);
            else
                sLog.outLog(LOG_DEFAULT, "ERROR: Map::RemoveFromDynamicObjectMap: Dynamic Object GUID Low %u not in map", objGuid.GetCounter());
            break;
        }
        case HIGHGUID_PET:
        {
            sObjectAccessor.RemovePet(objGuid.GetRawValue());
            break;
        }
        case HIGHGUID_CORPSE:
        {
            HashMapHolder<Corpse>::Remove(objGuid.GetRawValue());
            break;
        }
        default:
            break;
    }
}

void Map::ForcedUnload()
{
    sLog.outLog(LOG_DEFAULT, "ERROR: Map::ForcedUnload called for map %u instance %u. Map crushed. Cleaning up...", GetId(), GetInstanceId());
    sLog.outLog(LOG_CRASH, "Map::ForcedUnload called for map %u instance %u. Map crushed. Cleaning up...", GetId(), GetInstanceId());

    // Immediately cleanup update sets/queues
    i_objectsToClientUpdate.clear();

    Map::PlayerList const pList = GetPlayers();

    for (PlayerList::const_iterator itr = pList.begin(); itr != pList.end(); ++itr)
    {
        Player* player = itr->getSource();
        if (!player || !player->GetSession())
            continue;

        if (player->IsBeingTeleported())
        {
            WorldLocation old_loc;
            player->GetPosition(old_loc);
            if (!player->TeleportTo(old_loc))
            {
                sLog.outDetail("Map::ForcedUnload: %u is in teleport state, cannot be ported to his previous place, teleporting him to his homebind place...",
                    player->GetGUIDLow());
                player->TeleportToHomebind();
            }
            player->SetSemaphoreTeleport(false);
        }

        switch (sWorld.getConfig(CONFIG_VMSS_MAPFREEMETHOD))
        {
            case 0:
            {
                player->RemoveAllAurasOnDeath();
                if (Pet* pet = player->GetPet())
                    pet->RemoveAllAurasOnDeath();
                player->GetSession()->LogoutPlayer(true);
                break;
            }
            case 1:
            {
                player->GetSession()->KickPlayer();
                break;
            }
            case 2:
            {
                player->GetSession()->LogoutPlayer(false);
                break;
            }
            default:
                break;
        }
    }

    switch (sWorld.getConfig(CONFIG_VMSS_MAPFREEMETHOD))
    {
        case 0:
            if (InstanceMap *instance = dynamic_cast<InstanceMap*>(this))
                if (InstanceData* iData = instance->GetInstanceData())
                    iData->SaveToDB();
            break;
        default:
            break;
    }

    UnloadAll();

    SetBroken(false);
}

float Map::GetVisibilityDistance(WorldObject* obj, Player* invoker) const
{
    if (invoker && invoker->getWatchingCinematic() != 0)
        return MAX_VISIBILITY_DISTANCE;

    if (m_TerrainData == nullptr)
        return DEFAULT_VISIBILITY_DISTANCE;

    float dist = m_TerrainData->GetVisibilityDistance();
    if (obj != nullptr)
    {
        if (obj->GetObjectGuid().IsGameObject())
            return (dist + obj->ToGameObject()->GetDeterminativeSize());    // or maybe should be GetMaxVisibleDistanceForObject instead m_VisibleDistance ?
        else if(obj->GetObjectGuid().IsCreature())
            return (dist + obj->ToCreature()->GetDeterminativeSize());
    }

    return dist;
}

bool Map::WaypointMovementAutoActive() const
{
    if(Instanceable())
        return sWorld.getConfig(CONFIG_WAYPOINT_MOVEMENT_ACTIVE_IN_INSTANCES);
    else
        return sWorld.getConfig(CONFIG_WAYPOINT_MOVEMENT_ACTIVE_ON_CONTINENTS);
}

bool Map::WaypointMovementPathfinding() const
{
    if(Instanceable())
        return sWorld.getConfig(CONFIG_WAYPOINT_MOVEMENT_ACTIVE_IN_INSTANCES);
    else
        return sWorld.getConfig(CONFIG_WAYPOINT_MOVEMENT_ACTIVE_ON_CONTINENTS);
}

bool Map::CanUnload(uint32 diff)
{
 //  if (!m_unloadTimer)   //FIXME: look, here we return if timer = 0
 //      return false;
 //
 //  if (m_unloadTimer <= diff)    // here we return in every other situation
 //      return true;
 //
 //  m_unloadTimer -= diff;         // does not happen 
 //  return false;

    if (m_unloadTimer.Expired(diff)) // this does the same what the "up" but allows timer to be counted
        return true;
    
    return false;
}

bool Map::IsRemovalGrid(float x, float y) const
{
    GridPair p = MaNGOS::ComputeGridPair(x, y);
    return(!getNGrid(p.x_coord, p.y_coord) || getNGrid(p.x_coord, p.y_coord)->GetGridState() == GRID_STATE_REMOVAL);
}

bool Map::IsLoaded(float x, float y) const
{
    GridPair p = MaNGOS::ComputeGridPair(x, y);
    return loaded(p);
}

void Map::ResetGridExpiry(NGridType &grid, float factor /*= 1*/) const
{
    grid.ResetTimeTracker((time_t)((float)i_gridExpiry*factor));
}

bool Map::GetEntrancePos( int32 &mapid, float &x, float &y )
{
    if (!i_mapEntry)
        return false;
    if (i_mapEntry->entrance_map < 0)
        return false;
    mapid = i_mapEntry->entrance_map;
    x = i_mapEntry->entrance_x;
    y = i_mapEntry->entrance_y;
    return true;
}

void Map::UpdateHelper::Update( DelayedMapList& delayedUpdate )
{
    if (m_map->m_wanted_delay > GetTimeElapsed())
        return;
    sMapMgr.GetMapUpdater()->schedule_update(*m_map, GetTimeElapsed());
    delayedUpdate.push_back(std::make_pair(m_map, uint32(GetTimeElapsed())));

    m_map->m_updateTracker.Reset();
}

time_t Map::UpdateHelper::GetTimeElapsed() const
{
    return m_map->m_updateTracker.timeElapsed();
}

std::string Map::getDebugData()
{
    std::ostringstream str;
    str << "Map: " << GetId() << " Id: " << GetInstanceId() << " Active objects: \n";
    for (ActiveNonPlayers::iterator itr = m_activeNonPlayers.begin(); itr != m_activeNonPlayers.end(); itr++)
    {
        WorldObject* obj = *itr;
        if (!obj || !obj->IsInWorld() || !obj->GetName())
            continue;
        str << obj->GetName() << " " << obj->GetPositionX() << " " << obj->GetPositionY() << "\n";
    }
    return str.str();
}

bool Map::IsInLineOfSight(float x1, float y1, float z1, float x2, float y2, float z2, bool checkDynLos, bool ignoreM2Model) const
{
    ASSERT(MaNGOS::IsValidMapCoord(x1, y1, z1));
    ASSERT(MaNGOS::IsValidMapCoord(x2, y2, z2));

    return VMAP::VMapFactory::createOrGetVMapManager()->isInLineOfSight(GetId(), x1, y1, z1, x2, y2, z2, ignoreM2Model)
        && (!checkDynLos || CheckDynamicTreeLoS(x1, y1, z1, x2, y2, z2, ignoreM2Model));
}

/**
* get the hit position and return true if we hit something (in this case the dest position will hold the hit-position)
* otherwise the result pos will be the dest pos
*/
bool Map::GetLosHitPosition(float srcX, float srcY, float srcZ, float& destX, float& destY, float& destZ, float modifyDist) const
{
    // check all static objects
    float tempX, tempY, tempZ = 0.0f;
    bool result0 = VMAP::VMapFactory::createOrGetVMapManager()->getObjectHitPos(GetId(), srcX, srcY, srcZ, destX, destY, destZ, tempX, tempY, tempZ, modifyDist);
    if (result0)
    {
        //DEBUG_LOG("Map::GetLosHitPosition vmaps corrects gained with static objects! new dest coords are X:%f Y:%f Z:%f", destX, destY, destZ);
        destX = tempX;
        destY = tempY;
        destZ = tempZ;
    }
    // at second all dynamic objects, if static check has an hit, then we can calculate only to this closer point
    G3D::Vector3 startPos = G3D::Vector3(srcX, srcY, srcZ);
    G3D::Vector3 dstPos = G3D::Vector3(destX, destY, destZ);
    G3D::Vector3 resultPos;
    bool result1 = GetDynamicObjectHitPos(startPos, dstPos, resultPos, modifyDist);
    if (result1)
    {
        sLog.outDebug("Map::GetLosHitPosition vmaps corrects gained with dynamic objects! new dest coords are X:%f Y:%f Z:%f", destX, destY, destZ);
        destX = resultPos.x;
        destY = resultPos.y;
        destZ = resultPos.z;
    }

    return result0 || result1;
}

bool Map::GetWalkHitPosition(Transport* transport, float srcX, float srcY, float srcZ, float& destX, float& destY, float& destZ, uint32 moveAllowedFlags, float zSearchDist, bool locatedOnSteepSlope) const
{
    if (transport)
        return false;

    if (!MaNGOS::IsValidMapCoord(srcX, srcY, srcZ))
    {
        sLog.outDebug("Map::GetWalkHitPosition invalid source coordinates,"
            "x1: %f y1: %f z1: %f, x2: %f, y2: %f, z2: %f on map %d",
            srcX, srcY, srcZ, destX, destY, destZ, GetId());
        return false;
    }

    if (!MaNGOS::IsValidMapCoord(destX, destY, destZ))
    {
        sLog.outDebug("Map::GetWalkHitPosition invalid destination coordinates,"
            "x1: %f y1: %f z1: %f, x2: %f, y2: %f, z2: %f on map %u",
            srcX, srcY, srcZ, destX, destY, destZ, GetId());
        return false;
    }

    MMAP::MMapManager* mmap = MMAP::MMapFactory::createOrGetMMapManager();
    dtNavMeshQuery const* m_navMeshQuery = mmap->GetNavMeshQuery(GetId(), GetInstanceId());
    if (!m_navMeshQuery)
    {
        sLog.outDebug("WalkHitPos: No nav mesh loaded !");
        return false;
    }

    /// Find navmesh position near source
    float point[3] = { srcY, srcZ, srcX };
    // Warning : Coord order is Y,Z,X
    float closestPoint[3] = { 0.0f, 0.0f, 0.0f };
    float endPosition[3] = { destY, destZ, destX };

    dtQueryFilter filter;
    filter.setIncludeFlags(moveAllowedFlags);

    dtPolyRef startRef = PathFinder::FindWalkPoly(m_navMeshQuery, point, filter, closestPoint, zSearchDist);
    if (!startRef)
    {
        sLog.outDebug("WalkHitPos: Start poly not found");
        return false;
    }

    /// Walk on the surface found
    dtPolyRef visited[50] = { 0 };
    int visitedCount = 0;
    float t = 0.0f;
    float hitNormal[3] = { 0 }; // Normal of wall hit. Not always defined by raycast (if no wall hit)
    dtStatus result = m_navMeshQuery->raycast(startRef, closestPoint, endPosition, &filter, &t, hitNormal, visited, &visitedCount, 50);
    if (DT_SUCCESS != (result) || !visitedCount)
    {
        sLog.outDebug("WalkHitPos: Navmesh raycast failed");
        return false;
    }
    for (int i = 0; i < 3; ++i)
        endPosition[i] += hitNormal[i] * 0.5f;
    if (DT_SUCCESS != (m_navMeshQuery->closestPointOnPoly(visited[visitedCount - 1], endPosition, endPosition)))
        return false;

    /// Compute complete path, and at each path step, check for dynamic LoS collision
    // Rq: This is non-sense on Transports, since we are using position offsets ...
    float pathPoints[MAX_POINT_PATH_LENGTH * VERTEX_SIZE];
    int pointCount = 0;
    result = m_navMeshQuery->findStraightPath(
        closestPoint,         // start position
        endPosition,          // end position
        visited,              // current path
        visitedCount,         // length of current path
        pathPoints,           // [out] path corner points
        nullptr,
        nullptr,
        (int*)&pointCount,
        20);                  // maximum number of points/polygons to use
    if (DT_SUCCESS != (result))
        return false;
    // Add 1y height, because navmesh height is not very precise.
    Vector3 dstPos = Vector3(srcX, srcY, srcZ + 1.0f);
    for (int i = 0; i < pointCount; ++i)
    {
        Vector3 startPos = dstPos;
        dstPos = Vector3(pathPoints[i * VERTEX_SIZE + 2], pathPoints[i * VERTEX_SIZE], pathPoints[i * VERTEX_SIZE + 1]);
        dstPos.z += 1.0f;
        if (GetDynamicObjectHitPos(startPos, dstPos, dstPos, -1.0f))
            break;
    }

    destX = dstPos.x;
    destY = dstPos.y;
    destZ = dstPos.z;
    if (!MaNGOS::IsValidMapCoord(destX, destY, destZ))
        return false;
    else
        destZ = GetHeight(destX, destY, destZ);
    return true;
}

VMAP::ModelInstance* Map::FindCollisionModel(float x1, float y1, float z1, float x2, float y2, float z2)
{
    ASSERT(MaNGOS::IsValidMapCoord(x1, y1, z1));
    ASSERT(MaNGOS::IsValidMapCoord(x2, y2, z2));
    return VMAP::VMapFactory::createOrGetVMapManager()->FindCollisionModel(GetId(), x1, y1, z1, x2, y2, z2);
}

float Map::GetHeight(float x, float y, float z, bool vmap/*=true*/, float maxSearchDist/*=DEFAULT_HEIGHT_SEARCH*/) const
{
    ASSERT(MaNGOS::IsValidMapCoord(x, y, z));
    return std::max<float>(GetTerrain()->GetHeight(x, y, z, vmap, maxSearchDist), GetDynamicTreeHeight(x, y, z, maxSearchDist));
}

// Find an height within a reasonable range of provided Z. This method may fail so we have to handle that case.
bool Map::GetHeightInRange(float x, float y, float& z, float maxSearchDist /*= 4.0f*/) const
{
    float height;
    float mapHeight = INVALID_HEIGHT_VALUE;
    float vmapHeight = VMAP_INVALID_HEIGHT_VALUE;

    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();

    if (vmgr)
    {
        // pure vmap search
        vmapHeight = vmgr->getHeight(i_id, x, y, z + 2.0f, maxSearchDist + 2.0f);
    }

    // find raw height from .map file on X,Y coordinates
    if (GridMap* gmap = const_cast<TerrainInfo*>(m_TerrainData)->GetGrid(x, y)) // TODO:: find a way to remove that const_cast
        mapHeight = gmap->getHeight(x, y);

    float diffMaps = fabs(fabs(z) - fabs(mapHeight));
    float diffVmaps = fabs(fabs(z) - fabs(vmapHeight));
    if (diffVmaps < maxSearchDist)
    {
        if (diffMaps < maxSearchDist)
        {
            // well we simply have to take the highest as normally there we cannot be on top of cavern is maxSearchDist is not too big
            if (vmapHeight > mapHeight || std::fabs(mapHeight - z) > std::fabs(vmapHeight - z))
                height = vmapHeight;
            else
                height = mapHeight;

            //sLog.outString("vmap %5.4f, map %5.4f, height %5.4f", vmapHeight, mapHeight, height);
        }
        else
        {
            //sLog.outString("vmap %5.4f", vmapHeight);
            height = vmapHeight;
        }
    }
    else
    {
        if (diffMaps < maxSearchDist)
        {
            //sLog.outString("map %5.4f", mapHeight);
            height = mapHeight;
        }
        else
            return false;
    }

    z = height;
    return true;
}

// This will generate a random point to all directions in water for the provided point in radius range.
bool Map::GetRandomPointUnderWater(float& x, float& y, float& z, float radius, GridMapLiquidData& liquid_status, bool randomRange/* = true*/) const
{
    const float angle = rand_norm_f() * (M_PI_F * 2.0f);
    const float range = (randomRange ? rand_norm_f() : 1.f) * radius;

    float i_x = x + range * cos(angle);
    float i_y = y + range * sin(angle);

    // get real ground of new point
    // the code consider cylinder instead of sphere for possible z
    float ground = GetTerrain()->GetHeight(i_x, i_y, z);
    if (ground > INVALID_HEIGHT) // GetHeight can fail
    {
        float min_z = z - 0.7f * radius; // 0.7 to have a bit a "flat" cylinder, TODO which value looks nicest
        if (min_z < ground)
            min_z = ground + 0.5f; // Get some space to prevent under map

        float liquidLevel = liquid_status.level - 2.0f; // just to make the generated point is in water and not on surface or a bit above

                                                        // if not enough space to fit the creature better is to return from here
        if (min_z > liquidLevel)
            return false;

        // Mobs underwater do not move along Z axis
        //float max_z = std::max(z + 0.7f * radius, min_z);
        //max_z = std::min(max_z, liquidLevel);
        x = i_x;
        y = i_y;
        if (min_z > z)
            z = min_z;
        return true;
    }
    return false;
}

// This will generate a random point to all directions in air for the provided point in radius range.
bool Map::GetRandomPointInTheAir(float& x, float& y, float& z, float radius, bool randomRange/* = true*/) const
{
    const float angle = rand_norm_f() * (M_PI_F * 2.0f);
    const float range = (randomRange ? rand_norm_f() : 1.f) * radius;

    float i_x = x + range * cos(angle);
    float i_y = y + range * sin(angle);

    // get real ground of new point
    // the code consider cylinder instead of sphere for possible z
    float ground = GetTerrain()->GetHeight(i_x, i_y, z);
    if (ground > INVALID_HEIGHT) // GetHeight can fail
    {
        float min_z = z - 0.7f * radius; // 0.7 to have a bit a "flat" cylinder, TODO which value looks nicest
        if (min_z < ground)
            min_z = ground + 2.5f; // Get some space to prevent landing
        float max_z = std::max(z + 0.7f * radius, min_z);
        x = i_x;
        y = i_y;
        if (min_z > z)
            z = min_z;
        return true;
    }
    return false;
}

// supposed to be used for not big radius, usually less than 20.0f
bool Map::GetReachableRandomPointOnGround(float& x, float& y, float& z, float radius, bool randomRange/* = true*/) const
{
    // Generate a random range and direction for the new point
    const float angle = rand_norm_f() * (M_PI_F * 2.0f);
    const float range = (randomRange ? rand_norm_f() : 1.f) * radius;

    float i_x = x + range * cos(angle);
    float i_y = y + range * sin(angle);
    float i_z = z + 1.0f;

    GetLosHitPosition(x, y, z + 1.0f, i_x, i_y, i_z, -0.5f);
    i_z = z; // reset i_z to z value to avoid too much difference from original point before GetHeightInRange
    if (!GetHeightInRange(i_x, i_y, i_z)) // GetHeight can fail
        return false;

    // here we have a valid position but the point can have a big Z in some case
    // next code will check angle from 2 points
    //        c
    //       /|
    //      / |
    //    b/__|a

    // project vector to get only positive value
    float ab = fabs(x - i_x);
    float ac = fabs(z - i_z);

    // slope represented by c angle (in radian)
    const float MAX_SLOPE_IN_RADIAN = 50.0f / 180.0f * M_PI_F;  // 50(degree) max seem best value for walkable slope

                                                                // check ab vector to avoid divide by 0
    if (ab > 0.0f)
    {
        // compute c angle and convert it from radian to degree
        float slope = atan(ac / ab);
        if (slope < MAX_SLOPE_IN_RADIAN)
        {
            x = i_x;
            y = i_y;
            z = i_z;
            return true;
        }
    }

    return false;
}

// Get random point by handling different situation depending of if the unit is flying/swimming/walking
bool Map::GetReachableRandomPosition(Unit* unit, float& x, float& y, float& z, float radius, bool randomRange/* = true*/) const
{
    float i_x = x;
    float i_y = y;
    float i_z = z;

    bool isFlying;
    bool isSwimming = true;
    switch (unit->GetTypeId())
    {
    case TYPEID_PLAYER:
        isFlying = static_cast<Player*>(unit)->IsFlying();
        break;
    case TYPEID_UNIT:
        isFlying = static_cast<Creature*>(unit)->CanFly();
        isSwimming = static_cast<Creature*>(unit)->CanSwim();
        break;
    default:
        sLog.outDebug("Map::GetReachableRandomPosition> Unsupported unit type is passed!");
        return false;
    }

    // define 1 as minimum radius for random position
    if (radius < 1.0f)
        radius = 1.0f;

    bool newDestAssigned;   // used to check if new random destination is found
    if (isFlying)
    {
        newDestAssigned = GetRandomPointInTheAir(i_x, i_y, i_z, radius, randomRange);
        /*if (newDestAssigned)
        sLog.outString("Generating air random point for %s", GetGuidStr().c_str());*/
    }
    else
    {
        GridMapLiquidData liquid_status;
        GridMapLiquidStatus res = m_TerrainData->getLiquidStatus(i_x, i_y, i_z, MAP_ALL_LIQUIDS, &liquid_status);
        if (isSwimming && (res & (LIQUID_MAP_UNDER_WATER | LIQUID_MAP_IN_WATER)))
        {
            newDestAssigned = GetRandomPointUnderWater(i_x, i_y, i_z, radius, liquid_status, randomRange);
            /*if (newDestAssigned)
            sLog.outString("Generating swim random point for %s", GetGuidStr().c_str());*/
        }
        else
        {
            newDestAssigned = GetReachableRandomPointOnGround(i_x, i_y, i_z, radius, randomRange);
            /*if (newDestAssigned)
            sLog.outString("Generating ground random point for %s", GetGuidStr().c_str());*/
        }
    }

    if (newDestAssigned)
    {
        x = i_x;
        y = i_y;
        z = i_z;
        return true;
    }

    return false;
}