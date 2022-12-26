/*
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

#ifndef _OUTDOOR_PVP_NA_H
#define _OUTDOOR_PVP_NA_H

// TODO: "sometimes" set to neutral

#include "OutdoorPvPImpl.h"
enum OutdoorPvPNASpells
{
    NA_KILL_TOKEN_ALLIANCE = 33005,
    NA_KILL_TOKEN_HORDE = 33004,
    NA_CAPTURE_BUFF = 33795  // strength of the halaani
};
// kill credit for pks
const uint32 NA_CREDIT_MARKER = 24867;
const uint32 NA_GUARDS_MAX = 15;

const uint32 NA_BUFF_ZONE = 3518;

const uint32 NA_HALAA_GRAVEYARD = 993;

const uint32 NA_HALAA_GRAVEYARD_ZONE = 3518; // need to add zone id, not area id

const uint32 NA_RESPAWN_TIME = 3600000; // one hour to capture after defeating all guards

const uint32 NA_GUARD_CHECK_TIME = 500; // every half second

enum OutdoorPvPNAWorldStates{
    NA_UI_HORDE_GUARDS_SHOW = 2503,
    NA_UI_ALLIANCE_GUARDS_SHOW = 2502,
    NA_UI_GUARDS_MAX = 2493,
    NA_UI_GUARDS_LEFT = 2491,

    NA_UI_TOWER_SLIDER_DISPLAY = 2495,
    NA_UI_TOWER_SLIDER_POS = 2494,
    NA_UI_TOWER_SLIDER_N = 2497,

    NA_MAP_WYVERN_NORTH_NEU_H = 2762,
    NA_MAP_WYVERN_NORTH_NEU_A = 2662,
    NA_MAP_WYVERN_NORTH_H = 2663,
    NA_MAP_WYVERN_NORTH_A = 2664,

    NA_MAP_WYVERN_SOUTH_NEU_H = 2760,
    NA_MAP_WYVERN_SOUTH_NEU_A = 2670,
    NA_MAP_WYVERN_SOUTH_H = 2668,
    NA_MAP_WYVERN_SOUTH_A = 2669,

    NA_MAP_WYVERN_WEST_NEU_H = 2761,
    NA_MAP_WYVERN_WEST_NEU_A = 2667,
    NA_MAP_WYVERN_WEST_H = 2665,
    NA_MAP_WYVERN_WEST_A = 2666,

    NA_MAP_WYVERN_EAST_NEU_H = 2763,
    NA_MAP_WYVERN_EAST_NEU_A = 2659,
    NA_MAP_WYVERN_EAST_H = 2660,
    NA_MAP_WYVERN_EAST_A = 2661,

    NA_MAP_HALAA_NEUTRAL = 2671,
    NA_MAP_HALAA_NEU_A = 2676,
    NA_MAP_HALAA_NEU_H = 2677,
    NA_MAP_HALAA_HORDE = 2672,
    NA_MAP_HALAA_ALLIANCE = 2673
};

const uint32 FLIGHT_NODES_NUM = 4;

// used to access the elements of Horde/AllyControlGOs
enum ControlGOTypes{
    NA_ROOST_S = 0,
    NA_ROOST_W = 1,
    NA_ROOST_N = 2,
    NA_ROOST_E = 3,

    NA_BOMB_WAGON_S = 4,
    NA_BOMB_WAGON_W = 5,
    NA_BOMB_WAGON_N = 6,
    NA_BOMB_WAGON_E = 7,

    NA_DESTROYED_ROOST_S = 8,
    NA_DESTROYED_ROOST_W = 9,
    NA_DESTROYED_ROOST_N = 10,
    NA_DESTROYED_ROOST_E = 11,

    NA_CONTROL_GO_NUM = 12
};

const uint32 FlightPathStartNodes[FLIGHT_NODES_NUM] = {103,105,107,109};
const uint32 FlightPathEndNodes[FLIGHT_NODES_NUM] = {104,106,108,110};

enum FlightSpellsNA{
    NA_SPELL_FLY_SOUTH = 32059,
    NA_SPELL_FLY_WEST = 32068,
    NA_SPELL_FLY_NORTH = 32075,
    NA_SPELL_FLY_EAST = 32081
};

// spawned when the alliance is attacking, horde is in control
const go_type HordeControlGOs[NA_CONTROL_GO_NUM] = {
    {182267,530,-1815.8f,8036.51f,-26.2354f,-2.89725f,0,0,0.992546f,-0.121869f}, //ALLY_ROOST_SOUTH
    {182280,530,-1507.95f,8132.1f,-19.5585f,-1.3439f,0,0,0.622515f,-0.782608f}, //ALLY_ROOST_WEST
    {182281,530,-1384.52f,7779.33f,-11.1663f,-0.575959f,0,0,0.284015f,-0.95882f}, //ALLY_ROOST_NORTH
    {182282,530,-1650.11f,7732.56f,-15.4505f,-2.80998f,0,0,0.986286f,-0.165048f}, //ALLY_ROOST_EAST

    {182222,530,-1825.4022f,8039.2602f,-26.08f,-2.89725f,0,0,0.992546f,-0.121869f}, //HORDE_BOMB_WAGON_SOUTH
    {182272,530,-1515.37f,8136.91f,-20.42f,-1.3439f,0,0,0.622515f,-0.782608f}, //HORDE_BOMB_WAGON_WEST
    {182273,530,-1377.95f,7773.44f,-10.31f,-0.575959f,0,0,0.284015f,-0.95882f}, //HORDE_BOMB_WAGON_NORTH
    {182274,530,-1659.87f,7733.15f,-15.75f,-2.80998f,0,0,0.986286f,-0.165048f}, //HORDE_BOMB_WAGON_EAST

    {182266,530,-1815.8f,8036.51f,-26.2354f,-2.89725f,0,0,0.992546f,-0.121869f}, //DESTROYED_ALLY_ROOST_SOUTH
    {182275,530,-1507.95f,8132.1f,-19.5585f,-1.3439f,0,0,0.622515f,-0.782608f}, //DESTROYED_ALLY_ROOST_WEST
    {182276,530,-1384.52f,7779.33f,-11.1663f,-0.575959f,0,0,0.284015f,-0.95882f}, //DESTROYED_ALLY_ROOST_NORTH
    {182277,530,-1650.11f,7732.56f,-15.4505f,-2.80998f,0,0,0.986286f,-0.165048f}  //DESTROYED_ALLY_ROOST_EAST
};

// spawned when the horde is attacking, alliance is in control
const go_type AllianceControlGOs[NA_CONTROL_GO_NUM] = {
    {182301,530,-1815.8f,8036.51f,-26.2354f,-2.89725f,0,0,0.992546f,-0.121869f}, //HORDE_ROOST_SOUTH
    {182302,530,-1507.95f,8132.1f,-19.5585f,-1.3439f,0,0,0.622515f,-0.782608f}, //HORDE_ROOST_WEST
    {182303,530,-1384.52f,7779.33f,-11.1663f,-0.575959f,0,0,0.284015f,-0.95882f}, //HORDE_ROOST_NORTH
    {182304,530,-1650.11f,7732.56f,-15.4505f,-2.80998f,0,0,0.986286f,-0.165048f}, //HORDE_ROOST_EAST

    {182305,530,-1825.4022f,8039.2602f,-26.08f,-2.89725f,0,0,0.992546f,-0.121869f}, //ALLY_BOMB_WAGON_SOUTH
    {182306,530,-1515.37f,8136.91f,-20.42f,-1.3439f,0,0,0.622515f,-0.782608f}, //ALLY_BOMB_WAGON_WEST
    {182307,530,-1377.95f,7773.44f,-10.31f,-0.575959f,0,0,0.284015f,-0.95882f}, //ALLY_BOMB_WAGON_NORTH
    {182308,530,-1659.87f,7733.15f,-15.75f,-2.80998f,0,0,0.986286f,-0.165048f}, //ALLY_BOMB_WAGON_EAST

    {182297,530,-1815.8f,8036.51f,-26.2354f,-2.89725f,0,0,0.992546f,-0.121869f}, //DESTROYED_HORDE_ROOST_SOUTH
    {182298,530,-1507.95f,8132.1f,-19.5585f,-1.3439f,0,0,0.622515f,-0.782608f}, //DESTROYED_HORDE_ROOST_WEST
    {182299,530,-1384.52f,7779.33f,-11.1663f,-0.575959f,0,0,0.284015f,-0.95882f}, //DESTROYED_HORDE_ROOST_NORTH
    {182300,530,-1650.11f,7732.56f,-15.4505f,-2.80998f,0,0,0.986286f,-0.165048f}  //DESTROYED_HORDE_ROOST_EAST
};

const uint32 HordeCreatureEntries[] = { 18816, 18821, 21474, 21484, 21483, 18192 };
const uint32 AllyCreatureEntries[] = { 18817, 18822, 21485, 21487, 21488, 18256 };

enum WyvernStates{
    WYVERN_NEU_HORDE = 1,
    WYVERN_NEU_ALLIANCE = 2,
    WYVERN_HORDE = 4,
    WYVERN_ALLIANCE = 8
};

enum HalaaStates{
    HALAA_N = 1,
    HALAA_N_A = 2,
    HALAA_A = 4,
    HALAA_N_H = 8,
    HALAA_H = 16
};

class Unit;
class Creature;
class OutdoorPvPNA;
class OPvPCapturePointNA : public OPvPCapturePoint
{
friend class OutdoorPvPNA;
public:
    OPvPCapturePointNA(OutdoorPvP * pvp);
    bool Update(uint32 diff);
    void ChangeState();
    void SendChangePhase();
    void FillInitialWorldStates(WorldPacket & data);
    // used when player is activated/inactivated in the area
    bool HandlePlayerEnter(Player * plr);
    void HandlePlayerLeave(Player * plr);
    bool HandleCustomSpell(Player *plr, uint32 spellId, GameObject * go);
    int32 HandleOpenGo(Player *plr, uint64 guid);
    uint32 GetAliveGuardsCount();
protected:
    // called when a faction takes control
    void FactionTakeOver(uint32 team);

    void DeSpawnGOs();
    void SpawnNPCsForTeam(uint32 team);
    void SpawnGOsForTeam(uint32 team);

    void UpdateWyvernRoostWorldState(uint32 roost);
    void UpdateHalaaWorldState();

private:
    bool m_capturable;
    uint32 m_GuardsAlive;
    uint32 m_ControllingFaction;
    uint32 m_WyvernStateNorth;
    uint32 m_WyvernStateSouth;
    uint32 m_WyvernStateEast;
    uint32 m_WyvernStateWest;
    uint32 m_HalaaState;
    Timer m_RespawnTimer;
    Timer m_GuardCheckTimer;
};

class OutdoorPvPNA : public OutdoorPvP
{
friend class OPvPCapturePointNA;
public:
    OutdoorPvPNA();
    bool SetupOutdoorPvP();
    void HandlePlayerEnterZone(Player *plr, uint32 zone);
    void HandlePlayerLeaveZone(Player *plr, uint32 zone);
    bool Update(uint32 diff);
    void FillInitialWorldStates(WorldPacket &data);
    void SendRemoveWorldStates(Player * plr);
    void HandleKillImpl(Player * plr, Unit * killed);
private:
    OPvPCapturePointNA * m_obj;
};

#endif

