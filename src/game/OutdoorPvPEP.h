/*
 * Copyright (C) 2008 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2008-2017 Hellground <http://wow-hellground.com/>
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

#ifndef HELLGROUND_OUTDOOR_PVP_EP_H
#define HELLGROUND_OUTDOOR_PVP_EP_H

#include "OutdoorPvPImpl.h"

#define OutdoorPvPEPBuffZonesNum 3
#define OutdoorPvPEPTeamsBuffNum 4
                                                         //  Eastern Plaguelands, Stratholme and Scholomance
const uint32 OutdoorPvPEPBuffZones[OutdoorPvPEPBuffZonesNum] = {139, 2017, 2057};

const int EP_AllianceBuffs[OutdoorPvPEPTeamsBuffNum] = {11413, 11414, 11415, 1386};
const int EP_HordeBuffs[OutdoorPvPEPTeamsBuffNum] = {30880, 30683, 30682, 29520};

const uint32 EP_GraveYardZone = 139;
const uint32 EP_GraveYardId = 927;

enum OutdoorPvPEPTowerType{
    EP_TOWER_CROWNGUARD = 0,
    EP_TOWER_EASTWALL,
    EP_TOWER_NORTHPASS,
    EP_TOWER_PLAGUEWOOD,
    EP_TOWER_NUM
};

const uint32 EP_CREDITMARKER[EP_TOWER_NUM] = {17689,17690,17696,17698};

const uint32 EP_MAP_N[EP_TOWER_NUM] = {2355,2361,2352,2353};
const uint32 EP_MAP_A[EP_TOWER_NUM] = {2378,2354,2372,2370};
const uint32 EP_MAP_H[EP_TOWER_NUM] = {2379,2356,2373,2371};
const uint32 EP_MAP_N_A[EP_TOWER_NUM] = {2374,2359,2362,2366};
const uint32 EP_MAP_N_H[EP_TOWER_NUM] = {2375,2360,2363,2353}; // plaguewood tower has incorrect display value (2353 as well as logical 2367 is wrong)

const uint32 EP_TAXI_NODE[EP_TOWER_NUM] = {84,85,86,87};

enum OutdoorPvPEPWorldStates{
    EP_UI_TOWER_SLIDER_DISPLAY = 2426,
    EP_UI_TOWER_SLIDER_POS = 2427,
    EP_UI_TOWER_SLIDER_N = 2428,

    EP_UI_TOWER_COUNT_A = 2327,
    EP_UI_TOWER_COUNT_H = 2328
};

enum EP_GoSummons {
    EP_NPT_SHRINE = EP_TOWER_NUM,
    EP_NPT_SHRINE_AURA,
};

const go_type EPCapturePoints[EP_TOWER_NUM] = {
    { 182096,0,1860.85f,-3731.23f,196.716f,-2.53214f,0.033967f,-0.131914f,0.944741f,-0.298177f },
    { 182097,0,2574.51f,-4794.89f,144.704f,-1.45003f,-0.097056f,0.095578f,-0.656229f,0.742165f },
    { 181899,0,3181.08f,-4379.36f,174.123f,-2.03472f,-0.065392f,0.119494f,-0.842275f,0.521553f },
    { 182098,0,2962.71f,-3042.31f,154.789f,2.08426f,-0.074807f,-0.113837f,0.855928f,0.49883f }
};

const go_type EPTowerFlags[EP_TOWER_NUM*2] = {
    {182106,0,1877.6f,-3716.76f,167.188f,1.74533f,0,0,0.766044f,0.642788f}, // flags left side
    {182106,0,2569.6f,-4772.93f,115.399f,2.72271f,0,0,0.978148f,0.207912f},
    {182106,0,3188.76f,-4358.5f,144.555f,1.97222f,0,0,0.833886f,0.551937f},
    {182106,0,2975.5f,-3060.36f,125.108f,-1.0472f,0,0,0.5f,-0.866025f},
    {182106,0,1838.42f,-3703.56f,167.713f,0.890118f,0,0,0.430511f,0.902585f}, // flags right side
    {182106,0,2539.61f,-4801.55f,115.766f,2.00713f,0,0,0.843391f,0.5373f},
    {182106,0,3148.17f,-4365.51f,145.029f,1.53589f,0,0,0.694658f,0.71934f},
    {182106,0,2992.63f,-3022.95f,125.593f,3.03687f,0,0,0.99863f,0.052336f}
};

const uint32 EPTowerPlayerEnterEvents[EP_TOWER_NUM] = {10705,10691,10699,10701};
const uint32 EPTowerPlayerLeaveEvents[EP_TOWER_NUM] = {10704,10692,10698,10700};

enum EP_TowerStates {
    EP_TS_N = 1,
    EP_TS_A = 32,
    EP_TS_H = 64
};

#define PWT_FLIGHT_MASTER 17209
const uint32 EP_PWT_FlightMasterAura = 36725;

const go_type EP_NPT_LordaeronShrine =     {181682,0,3167.72f,-4355.91f,138.785f,1.69297f,0,0,0.748956f,0.66262f};
const go_type EP_NPT_LordaeronShrineAura = {180100,0,3167.72f,-4355.91f,138.785f,1.69297f,0,0,0.748956f,0.66262f};

class OPvPCapturePointEP : public OPvPCapturePoint
{
public:
    OPvPCapturePointEP(OutdoorPvP * pvp, OutdoorPvPEPTowerType type);
    void ChangeState();
    void SendChangePhase();
    void FillInitialWorldStates(WorldPacket & data);
    // used when player is activated/inactivated in the area
    bool HandlePlayerEnter(Player * plr);
    void HandlePlayerLeave(Player * plr);
protected:
    void UpdateTowerEvents();
    void LinkGraveYard(uint32 team);
    void SummonShrine(uint32 team);
    void SummonSupportUnits(uint32 team);
    void SummonFlightMaster(uint32 team);
    bool HandleGossipOption(Player *plr, uint64 guid, uint32 gossipid);
    bool CanTalkTo(Player * plr, Creature * c, GossipOption &gso);
    void UnlinkGraveYard();
    void UnsummonShrine();
    void UnsummonSupportUnits();
    void UnsummonFlightMaster();

    uint32 EP_TOWER_EVENT_TEAM[EP_TOWER_NUM];
private:
    OutdoorPvPEPTowerType m_TowerType;
};

class OutdoorPvPEP : public OutdoorPvP
{
friend class OPvPCapturePointEP;
public:
    OutdoorPvPEP();
    bool SetupOutdoorPvP();
    void HandlePlayerEnterZone(Player *plr, uint32 zone);
    void HandlePlayerLeaveZone(Player *plr, uint32 zone);
    bool Update(uint32 diff);
    void FillInitialWorldStates(WorldPacket &data);
    void SendRemoveWorldStates(Player * plr);
private:
    // how many towers are controlled
    uint32 m_AllianceTowersControlled;
    uint32 m_HordeTowersControlled;
};

#endif
