/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
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

#include "PoolManager.h"
#include "ObjectMgr.h"
#include "ObjectGuid.h"
#include "ProgressBar.h"
#include "Log.h"
#include "MapManager.h"
#include "World.h"

////////////////////////////////////////////////////////////
// template class SpawnedPoolData

// Method that tell amount spawned objects/subpools
uint32 SpawnedPoolData::GetSpawnedObjects(uint32 pool_id) const
{
    SpawnedPoolPools::const_iterator itr = mSpawnedPools.find(pool_id);
    return itr != mSpawnedPools.end() ? itr->second : 0;
}

// Method that tell if a gameobject is spawned currently
bool SpawnedPoolData::IsSpawnedObject(uint32 db_guid) const
{
    return mSpawnedGameobjects.find(db_guid) != mSpawnedGameobjects.end();
}

void SpawnedPoolData::AddSpawn(uint32 db_guid, uint32 pool_id)
{
    mSpawnedGameobjects.insert(db_guid);
    ++mSpawnedPools[pool_id];
}

void SpawnedPoolData::RemoveSpawn(uint32 db_guid, uint32 pool_id)
{
    mSpawnedGameobjects.erase(db_guid);
    uint32& val = mSpawnedPools[pool_id];
    if (val > 0)
        --val;
}

////////////////////////////////////////////////////////////
// Methods of template class PoolGroup

// Method to add a gameobject/creature guid to the proper list depending on pool type and chance value
void PoolGroup::AddEntry(PoolObject& poolitem)
{
    if (poolitem.chance != 0 && maxLimit == 1)
        ExplicitlyChanced.push_back(poolitem);
    else
        EqualChanced.push_back(poolitem);
}

// Method to check the chances are proper in this object pool
bool PoolGroup::CheckPool() const
{
    if (EqualChanced.empty())
        return (ExplicitlyChanced.empty());
    float chance = 0.0f;
    for (uint32 i =0;i<ExplicitlyChanced.size(); i++)
        chance += ExplicitlyChanced[i].chance;
    return (chance <= 100.0f);
}

PoolObject* PoolGroup::RollOne(SpawnedPoolData& spawns)
{
    if (!ExplicitlyChanced.empty())
    {
        float roll = (float)rand_chance();

        for (uint32 i = 0; i < ExplicitlyChanced.size(); ++i)
        {
            if (spawns.IsSpawnedObject(ExplicitlyChanced[i].guid))
                continue;
            roll -= ExplicitlyChanced[i].chance;
            if (roll < 0)
                return &ExplicitlyChanced[i];
        }
    }

    if (!EqualChanced.empty())
    {
        int32 index = irand(0, EqualChanced.size()-1);
        if (!spawns.IsSpawnedObject(EqualChanced[index].guid))
            return &EqualChanced[index];
    }

    return NULL;
}

// Main method to despawn a creature or gameobject in a pool
// If no guid is passed, the pool is just removed (event end case)
// If guid is filled, cache will be used and no removal will occur, it just fill the cache
void PoolGroup::DespawnObject(SpawnedPoolData& spawns, uint32 guid)
{
    for (size_t i = 0; i < EqualChanced.size(); ++i)
    {
        // if spawned
        if (spawns.IsSpawnedObject(EqualChanced[i].guid))
        {
            // any or specially requested
            if (!guid || EqualChanced[i].guid == guid)
            {
                Despawn1Object(EqualChanced[i].guid);
                spawns.RemoveSpawn(EqualChanced[i].guid, poolId);
            }
        }
    }

    for (size_t i = 0; i < ExplicitlyChanced.size(); ++i)
    {
        // spawned
        if (spawns.IsSpawnedObject(ExplicitlyChanced[i].guid))
        {
            // any or specially requested
            if (!guid || ExplicitlyChanced[i].guid == guid)
            {
                Despawn1Object(ExplicitlyChanced[i].guid);
                spawns.RemoveSpawn(ExplicitlyChanced[i].guid, poolId);
            }
        }
    }
}

// Same on one gameobject
void PoolGroup::Despawn1Object(uint32 guid)
{
    if (GameObjectData const* data = sObjectMgr.GetGOData(guid))
    {
        sObjectMgr.RemoveGameobjectFromGrid(guid, data);

        // FIXME: pool system must have local state for each instanced map copy
        // Current code preserve existed single state for all instanced map copies way
        // specially because pool system not spawn object in instanceable maps
        MapEntry const* mapEntry = sMapStore.LookupEntry(data->mapid);

        // temporary limit pool system full power work to continents
        if (mapEntry && !mapEntry->Instanceable())
        {
            if (Map* map = const_cast<Map*>(sMapMgr.FindMap(data->mapid)))
            {
                if (GameObject* pGameobject = map->GetGameObject(ObjectGuid(HIGHGUID_GAMEOBJECT, data->id, guid)))
                    pGameobject->AddObjectToRemoveList();
            }
        }
    }
}

void PoolGroup::SpawnObject(SpawnedPoolData& spawns, bool instantly)
{
    uint32 lastDespawned = 0;
    int count = maxLimit - spawns.GetSpawnedObjects(poolId);

    for (int i = 0; i < count; ++i)
    {
        PoolObject* obj = RollOne(spawns);
        if (!obj)
            continue;
        spawns.AddSpawn(obj->guid, poolId);
        Spawn1Object(obj, instantly);
    }
}

// Same for 1 gameobject
void PoolGroup::Spawn1Object(PoolObject* obj, bool instantly)
{
    if (GameObjectData const* data = sObjectMgr.GetGOData(obj->guid))
    {
        sObjectMgr.AddGameobjectToGrid(obj->guid, data);

        MapEntry const* mapEntry = sMapStore.LookupEntry(data->mapid);

        // FIXME: pool system must have local state for each instanced map copy
        // Current code preserve existed single state for all instanced map copies way
        if (mapEntry && !mapEntry->Instanceable())
        {
            // Spawn if necessary (loaded grids only)
            Map* map = const_cast<Map*>(sMapMgr.FindMap(data->mapid));

            // We use spawn coords to spawn
            if (map && map->IsLoaded(data->posX, data->posY))
            {
                GameObject* pGameobject = new GameObject;
                //DEBUG_LOG("Spawning gameobject %u", obj->guid);
                if (!pGameobject->LoadFromDB(obj->guid, map))
                {
                    delete pGameobject;
                    return;
                }
                else
                {
                    if (pGameobject->isSpawnedByDefault())
                    {
                        // if new spawn replaces a just despawned object, not instantly spawn but set respawn timer
                        if(!instantly)
                        {
                            pGameobject->SetRespawnTime(pGameobject->GetRespawnDelay());
                            if (sWorld.getConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY))
                                pGameobject->SaveRespawnTime();
                        }
                        map->Add(pGameobject);
                    }
                }
            }
            // for not loaded grid just update respawn time (avoid work for instances until implemented support)
            else if(!instantly)
            {
                // for spawned by default object only
                if (data->spawntimesecs >= 0)
                    sObjectMgr.SaveGORespawnTime(obj->guid, 0, time(NULL) + data->spawntimesecs);
            }
        }
    }
}

////////////////////////////////////////////////////////////
// Methods of class PoolManager

PoolManager::PoolManager()
{
}

// Check listing all pool spawns in single instanceable map or only in non-instanceable maps
// This applied to all pools have common mother pool
struct PoolMapChecker
{
    typedef std::map<uint32,MapEntry const*> Pool2Maps;
    Pool2Maps m_pool2maps;

    bool CheckAndRemember(uint32 mapid, uint32 pool_id, char const* tableName, char const* elementName)
    {
        MapEntry const* mapEntry = sMapStore.LookupEntry(mapid);
        if (!mapEntry)
            return false;

        MapEntry const* poolMapEntry = GetPoolMapEntry(pool_id);

        // if not listed then just remember
        if (!poolMapEntry)
        {
            m_pool2maps[pool_id] = mapEntry;
            return true;
        }

        // if at same map, then all ok
        if (poolMapEntry == mapEntry)
            return true;

        // pool spawns must be at single instanceable map
        if (mapEntry->Instanceable())
        {
            sLog.outLog(LOG_DB_ERR, "`%s` has %s spawned at instanceable map %u when one or several other spawned at different map %u in pool id %i, skipped.",
                tableName, elementName, mapid, poolMapEntry->MapID, pool_id);
            return false;
        }

        // pool spawns must be at single instanceable map
        if (poolMapEntry->Instanceable())
        {
            sLog.outLog(LOG_DB_ERR, "`%s` has %s spawned at map %u when one or several other spawned at different instanceable map %u in pool id %i, skipped.",
                tableName, elementName, mapid, poolMapEntry->MapID, pool_id);
            return false;
        }

        // pool spawns can be at different non-instanceable maps
        return true;
    }

    MapEntry const* GetPoolMapEntry(uint32 pool_id) const
    {
        Pool2Maps::const_iterator p2m_itr = m_pool2maps.find(pool_id);
        return p2m_itr != m_pool2maps.end() ? p2m_itr->second : NULL;
    }
};

void PoolManager::LoadFromDB()
{
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT MAX(entry) FROM pool_template");
    if (!result)
    {
        sLog.outString(">> Table pool_template is empty.");
        sLog.outString();
        return;
    }
    else
    {
        Field *fields = result->Fetch();
        max_pool_id = fields[0].GetUInt16();
    }

    uint32 count = 0;
    PoolMapChecker mapChecker;
    mPoolGameobjectGroups.resize(max_pool_id + 1);

    result = GameDataDatabase.Query("SELECT entry,max_limit FROM pool_template");
    BarGoLink bar((int)result->GetRowCount());
    do
    {
        ++count;
        Field *fields = result->Fetch();
        bar.step();
        uint16 pool_id = fields[0].GetUInt16();

        mPoolGameobjectGroups[pool_id].SetLimit(fields[1].GetUInt32());
        if (fields[1].GetUInt32() == 0)
            sLog.outLog(LOG_DB_ERR, "pool id %u has max limit set to 0", pool_id);

    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u objects pools", count);

    // Gameobjects
    mGameobjectSearchMap.clear();
    //                                   1     2           3
    result = GameDataDatabase.Query("SELECT guid, pool_entry, chance FROM pool_gameobject");
    count = 0;
    if (!result)
    {
        BarGoLink bar2(1);
        bar2.step();

        sLog.outString();
        sLog.outString(">> Loaded %u gameobject in pools", count );
    }
    else
    {

        BarGoLink bar2((int)result->GetRowCount());
        do
        {
            Field *fields = result->Fetch();

            bar2.step();

            uint32 guid    = fields[0].GetUInt32();
            uint16 pool_id = fields[1].GetUInt16();
            float chance   = fields[2].GetFloat();

            GameObjectData const* data = sObjectMgr.GetGOData(guid);
            if (!data)
            {
                sLog.outLog(LOG_DB_ERR, "`pool_gameobject` has a non existing gameobject spawn (GUID: %u) defined for pool id (%u), skipped.", guid, pool_id );
                continue;
            }
            GameObjectInfo const* goinfo = ObjectMgr::GetGameObjectInfo(data->id);
            if (goinfo->type != GAMEOBJECT_TYPE_CHEST &&
                goinfo->type != GAMEOBJECT_TYPE_GOOBER &&
                goinfo->type != GAMEOBJECT_TYPE_FISHINGHOLE)
            {
                sLog.outLog(LOG_DB_ERR, "`pool_gameobject` has a not lootable gameobject spawn (GUID: %u, type: %u) defined for pool id (%u), skipped.", guid, goinfo->type, pool_id );
                continue;
            }
            if (pool_id > max_pool_id)
            {
                sLog.outLog(LOG_DB_ERR, "`pool_gameobject` pool id (%i) is out of range compared to max pool id in `pool_template`, skipped.",pool_id);
                continue;
            }
            if (chance < 0 || chance > 100)
            {
                sLog.outLog(LOG_DB_ERR, "`pool_gameobject` has an invalid chance (%f) for gameobject guid (%u) in pool id (%i), skipped.", chance, guid, pool_id);
                continue;
            }

            if (!mapChecker.CheckAndRemember(data->mapid, pool_id, "pool_gameobject", "gameobject guid"))
                continue;

            ++count;

            PoolObject plObject = PoolObject(guid, chance);
            PoolGroup& gogroup = mPoolGameobjectGroups[pool_id];
            gogroup.SetPoolId(pool_id);
            gogroup.AddEntry(plObject);
            SearchPair p(guid, pool_id);
            mGameobjectSearchMap.insert(p);

        } while( result->NextRow() );
        sLog.outString();
        sLog.outString( ">> Loaded %u gameobject in pools", count );
    }
}

// The initialize method will spawn all pools not in an event and not in another pool
void PoolManager::Initialize()
{
    uint32 count = 0;

    for(uint16 pool_entry = 0; pool_entry < max_pool_id; ++pool_entry)
    {
        if (!CheckPool(pool_entry))
        {
            sLog.outLog(LOG_DB_ERR, "Pool Id (%u) has invalid chance sum, cannot spawn", pool_entry);
            continue;
        }
        SpawnPool(pool_entry, true);
        count++;
    }

    sLog.outBasic("Pool handling system initialized, %u pools spawned.", count);
}

/*!
    \param instantly defines if (leaf-)objects are spawned instantly or with fresh respawn timer */
void PoolManager::SpawnPool(uint16 pool_id, bool instantly)
{
    if (!mPoolGameobjectGroups[pool_id].isEmpty())
        mPoolGameobjectGroups[pool_id].SpawnObject(mSpawnedData, instantly);
}

// Call to despawn a pool, all gameobjects/creatures in this pool are removed
void PoolManager::DespawnPool(uint16 pool_id)
{
    if (!mPoolGameobjectGroups[pool_id].isEmpty())
        mPoolGameobjectGroups[pool_id].DespawnObject(mSpawnedData);
}

// Method that check chance integrity of the creatures and gameobjects in this pool
bool PoolManager::CheckPool(uint16 pool_id) const
{
    return pool_id <= max_pool_id && mPoolGameobjectGroups[pool_id].CheckPool();
}

// Call to update the pool when a gameobject/creature part of pool [pool_id] is ready to respawn
// Here we cache only the creature/gameobject whose guid is passed as parameter
// Then the spawn pool call will use this cache to decide
void PoolManager::UpdatePool(uint16 pool_id)
{
    if (!mPoolGameobjectGroups[pool_id].isEmpty())
        mPoolGameobjectGroups[pool_id].SpawnObject(mSpawnedData, false);
}

bool PoolManager::IsSpawnedOrNotInPoolGameobject(uint32 db_guid) const
{
    return IsPartOfAPool(db_guid) ?
        mSpawnedData.IsSpawnedObject(db_guid) : true;
}