/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Instance_Serpent_Shrine
SD%Complete: 100
SDComment: Instance Data Scripts and functions to acquire mobs and set encounter status for use in various Serpent Shrine Scripts
SDCategory: Coilfang Resevoir, Serpent Shrine Cavern
EndScriptData */

#include "scriptPCH.h"
#include "def_serpent_shrine.h"

#define ENCOUNTERS 6

#define SPELL_SCALDINGWATER 37284
#define MOB_COILFANG_FRENZY 21508
#define TRASHMOB_GRAYHEART_TECHICIAN 21263


/* Serpentshrine cavern encounters:
0 - Hydross The Unstable event
1 - Leotheras The Blind Event
2 - The Lurker Below Event
3 - Fathom-Lord Karathress Event
4 - Morogrim Tidewalker Event
5 - Lady Vashj Event
*/

bool GOUse_go_bridge_console(Player *player, GameObject* go)
{
    ScriptedInstance* pInstance = (ScriptedInstance*)go->GetInstanceData();
    player->SendCombatStats(1 << COMBAT_STATS_TEST, "using console object", NULL);
    if(!pInstance)
        return false;
    player->SendCombatStats(1 << COMBAT_STATS_TEST, "instance found %u %u", NULL, pInstance->GetData(DATA_PREVIOUS_BOSS_DEAD), pInstance->GetData(DATA_CONTROL_CONSOLE));
    if (pInstance->GetData(DATA_PREVIOUS_BOSS_DEAD) != DONE)
        return true;

    if (pInstance->GetData(DATA_CONTROL_CONSOLE) != DONE)
    {
        pInstance->SetData(DATA_CONTROL_CONSOLE, DONE);
        return false;
    }

    return true;
}

struct instance_serpentshrine_cavern : public ScriptedInstance
{
    instance_serpentshrine_cavern(Map *map) : ScriptedInstance(map),m_gbk(map) {Initialize();};

    uint64 LurkerBelow;
    uint64 Sharkkis;
    uint64 SharkkisPet;
    uint64 Tidalvess;
    uint64 Caribdis;
    uint64 LadyVashj;
    uint64 Karathress;
    uint64 KarathressEvent_Starter;
    uint64 LeotherasTheBlind;
    uint64 LeotherasEventStarter;

    uint64 ControlConsole;
    uint64 BridgePart[3];
    uint32 StrangePool;
    Timer FishingTimer;
    uint32 LurkerSubEvent;
    Timer WaterCheckTimer;
    Timer FrenzySpawnTimer;
    Timer PlayerInWaterTimer;
    uint32 Water;
    Timer trashCheckTimer;
    Timer ScaldingWaterDelayer;

    bool ShieldGeneratorDeactivated[4];
    uint32 Encounters[ENCOUNTERS];
    bool DoSpawnFrenzy;
    GBK_handler m_gbk;

    void Initialize()
    {
        LurkerBelow = 0;
        Sharkkis = 0;
        SharkkisPet = 0;
        Tidalvess = 0;
        Caribdis = 0;
        LadyVashj = 0;
        Karathress = 0;
        KarathressEvent_Starter = 0;
        LeotherasTheBlind = 0;
        LeotherasEventStarter = 0;

        ControlConsole = 0;
        BridgePart[0] = 0;
        BridgePart[1] = 0;
        BridgePart[2] = 0;
        StrangePool = 0;
        Water = WATERSTATE_FRENZY;

        ShieldGeneratorDeactivated[0] = false;
        ShieldGeneratorDeactivated[1] = false;
        ShieldGeneratorDeactivated[2] = false;
        ShieldGeneratorDeactivated[3] = false;
        FishingTimer = 1000;
        LurkerSubEvent = 0;
        WaterCheckTimer = 500;
        FrenzySpawnTimer = 2000;
        PlayerInWaterTimer = 0;
        DoSpawnFrenzy = false;
        trashCheckTimer = 10000;
        ScaldingWaterDelayer = 500;

        for(uint8 i = 0; i < ENCOUNTERS; i++)
            Encounters[i] = NOT_STARTED;
    }

    bool IsEncounterInProgress() const
    {
        for(uint8 i = 0; i < ENCOUNTERS; i++)
            if (Encounters[i] == IN_PROGRESS)
                return true;

        return false;
    }

    void OnObjectCreate(GameObject *go)
    {
        switch(go->GetEntry())
        {
            case 184568:
                ControlConsole = go->GetGUID();
                go->setActive(true);
            break;

            case 184203:
                BridgePart[0] = go->GetGUID();
                go->setActive(true);
            break;

            case 184204:
                BridgePart[1] = go->GetGUID();
                go->setActive(true);
            break;

            case 184205:
                BridgePart[2] = go->GetGUID();
                go->setActive(true);
            break;
            case 184956:
                StrangePool = go->GetGUID();
                if(go->isActiveObject())
                    SetData(DATA_STRANGE_POOL, DONE);
                break;
            case GAMEOBJECT_FISHINGNODE_ENTRY:
            {
                if (!go->GetOwner())
                    return;

                Player* player = go->GetOwner()->ToPlayer();

                if (!player || player->GetSkillValue(SKILL_FISHING) < 300 || rand() % 3)
                    return;

                if (LurkerSubEvent == LURKER_NOT_STARTED)
                {
                    if (Unit *pTemp = instance->GetCreature(LurkerBelow))
                    {
                        if (go->GetDistance2d(pTemp) > 16.0f)
                            return;

                        FishingTimer = 3000;//10000+rand()%30000;//random time before lurker emerges
                        LurkerSubEvent = LURKER_FISHING;
                    }
                }

                break;
            }
        }
    }

    void OpenDoor(uint64 DoorGUID, bool open)
    {
        if(GameObject *Door = instance->GetGameObject(DoorGUID))
            Door->SetUInt32Value(GAMEOBJECT_STATE, open ? 0 : 1);
    }

    uint32 GetEncounterForEntry(uint32 entry)
    {
        switch(entry)
        {
            case 21212:
                return DATA_LADYVASHJEVENT;
            case 21214:
                return DATA_KARATHRESSEVENT;
            case 21217:
                return DATA_THELURKERBELOWEVENT;
            case 21215:
                return DATA_LEOTHERASTHEBLINDEVENT;
            case 21213:
                return DATA_MOROGRIMTIDEWALKEREVENT;
            case 21216:
                return DATA_HYDROSSTHEUNSTABLEEVENT;
            default:
                return 0;
        }
    }

    void OnCreatureCreate(Creature *creature, uint32 creature_entry)
    {
        switch(creature_entry)
        {
            case 21212:
                LadyVashj = creature->GetGUID();
                break;
            case 21214:
                Karathress = creature->GetGUID();
                break;
            case 21966:
                Sharkkis = creature->GetGUID();
                break;
            case 21217:
                LurkerBelow = creature->GetGUID();
                break;
            case 21965:
                Tidalvess = creature->GetGUID();
                break;
            case 21964:
                Caribdis = creature->GetGUID();
                break;
            case 21215:
                LeotherasTheBlind = creature->GetGUID();
                break;
        }

        HandleInitCreatureState(creature);
    }

    void SetData64(uint32 type, uint64 data)
    {
        if(type == DATA_KARATHRESSEVENT_STARTER)
            KarathressEvent_Starter = data;
        if(type == DATA_LEOTHERAS_EVENT_STARTER)
            LeotherasEventStarter = data;
        if(type == DATA_SHARKKIS_PET)
            SharkkisPet = data;
    }

    uint64 GetData64(uint32 identifier)
    {
        switch(identifier)
        {
            case DATA_THELURKERBELOW:           return LurkerBelow;
            case DATA_SHARKKIS:                 return Sharkkis;
            case DATA_SHARKKIS_PET:             return SharkkisPet;
            case DATA_TIDALVESS:                return Tidalvess;
            case DATA_CARIBDIS:                 return Caribdis;
            case DATA_LADYVASHJ:                return LadyVashj;
            case DATA_KARATHRESS:               return Karathress;
            case DATA_KARATHRESSEVENT_STARTER:  return KarathressEvent_Starter;
            case DATA_LEOTHERAS:                return LeotherasTheBlind;
            case DATA_LEOTHERAS_EVENT_STARTER:  return LeotherasEventStarter;
        }
        return 0;
    }

    GBK_Encounters EncounterForGBK(uint32 enc)
    {
        switch (enc)
        {
        case DATA_HYDROSSTHEUNSTABLEEVENT:  return GBK_HYDROSS_THE_UNSTABLE;
        case DATA_LEOTHERASTHEBLINDEVENT:   return GBK_LEOTHERAS_THE_BLIND;
        case DATA_THELURKERBELOWEVENT:      return GBK_LURKER_BELOW;
        case DATA_KARATHRESSEVENT:          return GBK_FATHOMLORD_KARATHRESS;
        case DATA_MOROGRIMTIDEWALKEREVENT:  return GBK_MOROGRIM_TIDEWALKER;
        case DATA_LADYVASHJEVENT:           return GBK_LADY_VASHJ;
        }
        return GBK_NONE;
    }

    void SetData(uint32 type, uint32 data)
    {
        switch(type)
        {
            case DATA_STRANGE_POOL:
                {
                    StrangePool = data;
                    if(data == NOT_STARTED)
                        LurkerSubEvent = LURKER_NOT_STARTED;
                }
                break;
            case DATA_WATER : Water = data; break;
            case DATA_CONTROL_CONSOLE:
                if(data == DONE)
                {
                    OpenDoor(BridgePart[0], true);
                    OpenDoor(BridgePart[1], true);
                    OpenDoor(BridgePart[2], true);
                }
                break;
            case DATA_HYDROSSTHEUNSTABLEEVENT:
                if(Encounters[0] != DONE)
                    Encounters[0] = data;
                break;
            case DATA_LEOTHERASTHEBLINDEVENT:
                if(Encounters[1] != DONE)
                    Encounters[1] = data;
                break;
            case DATA_THELURKERBELOWEVENT:
                if(Encounters[2] != DONE)
                    Encounters[2] = data;
                break;
            case DATA_KARATHRESSEVENT:
                if(Encounters[3] != DONE)
                    Encounters[3] = data;
                break;
            case DATA_MOROGRIMTIDEWALKEREVENT:
                if(Encounters[4] != DONE)
                    Encounters[4] = data;
                break;
                //Lady Vashj
            case DATA_LADYVASHJEVENT:
                if(data == NOT_STARTED)
                {
                    ShieldGeneratorDeactivated[0] = false;
                    ShieldGeneratorDeactivated[1] = false;
                    ShieldGeneratorDeactivated[2] = false;
                    ShieldGeneratorDeactivated[3] = false;
                }
                if (Encounters[5] != DONE)
                    Encounters[5] = data;
                break;
            case DATA_SHIELDGENERATOR1:ShieldGeneratorDeactivated[0] = (data) ? true : false;   break;
            case DATA_SHIELDGENERATOR2:ShieldGeneratorDeactivated[1] = (data) ? true : false;   break;
            case DATA_SHIELDGENERATOR3:ShieldGeneratorDeactivated[2] = (data) ? true : false;   break;
            case DATA_SHIELDGENERATOR4:ShieldGeneratorDeactivated[3] = (data) ? true : false;   break;
        }

        GBK_Encounters gbkEnc = EncounterForGBK(type);
        if (gbkEnc != GBK_NONE)
        {
            if (data == DONE)
                m_gbk.StopCombat(gbkEnc, true);
            else if (data == NOT_STARTED)
                m_gbk.StopCombat(gbkEnc, false);
            else if (data == IN_PROGRESS)
                m_gbk.StartCombat(gbkEnc);
        }
        
        if (data == DONE)
            SaveToDB();
    }

    uint32 GetData(uint32 type)
    {
        switch(type)
        {
            case DATA_HYDROSSTHEUNSTABLEEVENT:  return Encounters[0];
            case DATA_LEOTHERASTHEBLINDEVENT:   return Encounters[1];
            case DATA_THELURKERBELOWEVENT:      return Encounters[2];
            case DATA_KARATHRESSEVENT:          return Encounters[3];
            case DATA_MOROGRIMTIDEWALKEREVENT:  return Encounters[4];
                //Lady Vashj
            case DATA_LADYVASHJEVENT:           return Encounters[5];
            case DATA_SHIELDGENERATOR1:         return ShieldGeneratorDeactivated[0];
            case DATA_SHIELDGENERATOR2:         return ShieldGeneratorDeactivated[1];
            case DATA_SHIELDGENERATOR3:         return ShieldGeneratorDeactivated[2];
            case DATA_SHIELDGENERATOR4:         return ShieldGeneratorDeactivated[3];
            case DATA_CANSTARTPHASE3:
                if (ShieldGeneratorDeactivated[0] && ShieldGeneratorDeactivated[1] && ShieldGeneratorDeactivated[2] && ShieldGeneratorDeactivated[3])
                    return 1;
                break;
            case DATA_STRANGE_POOL:             return StrangePool;
            case DATA_WATER:                    return Water;
            case DATA_PREVIOUS_BOSS_DEAD:
                return (Encounters[0] == DONE && Encounters[1] == DONE && Encounters[2] == DONE && Encounters[3] == DONE &&
                    Encounters[4] == DONE) ? DONE : NOT_STARTED;
        }
        return 0;
    }

    std::string GetSaveData()
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream stream;
        stream << Encounters[0] << " ";
        stream << Encounters[1] << " ";
        stream << Encounters[2] << " ";
        stream << Encounters[3] << " ";
        stream << Encounters[4] << " ";
        stream << Encounters[5];

        OUT_SAVE_INST_DATA_COMPLETE;

        return stream.str();
    }

    void Load(const char* in)
    {
        if(!in)
        {
            OUT_LOAD_INST_DATA_FAIL;
            return;
        }
        OUT_LOAD_INST_DATA(in);
        std::istringstream stream(in);
        stream >> Encounters[0] >> Encounters[1] >> Encounters[2] >> Encounters[3]
        >> Encounters[4] >> Encounters[5];
        for(uint8 i = 0; i < ENCOUNTERS; ++i)
            if(Encounters[i] == IN_PROGRESS)                // Do not load an encounter as "In Progress" - reset it instead.
                Encounters[i] = NOT_STARTED;
        OUT_LOAD_INST_DATA_COMPLETE;
    }

    void Update (uint32 diff)
    {
        //Lurker Fishing event
        if (LurkerSubEvent == LURKER_FISHING)
        {
            if (FishingTimer.Expired(diff))
            {
                LurkerSubEvent = LURKER_HOOKED;
                SetData(DATA_STRANGE_POOL, IN_PROGRESS);//just fished, signal Lurker script to emerge and start fight, we use IN_PROGRESS so it won't get saved and lurker will be alway invis at start if server restarted
            }
        }

        if (trashCheckTimer.Expired(diff))
        {
            if (Encounters[2] == NOT_STARTED)   // check and change water state only if lurker event is not started
            {
                uint64 tmpTechnicianGuid = instance->GetCreatureGUID(TRASHMOB_GRAYHEART_TECHICIAN, GET_ALIVE_CREATURE_GUID);
                if (!tmpTechnicianGuid)
                    Water = WATERSTATE_SCALDING;
                else
                    Water = WATERSTATE_FRENZY;
            }
            else if (Encounters[2] == DONE)
            {
                Water = WATERSTATE_NONE;
            }
            
            trashCheckTimer = 5000;
        }
        

        //Water checks
        if (WaterCheckTimer.Expired(diff))
        {
            Map::PlayerList const &PlayerList = instance->GetPlayers();
            if (PlayerList.isEmpty())
                return;

            bool PlayerInWater = false;
            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            {
                if(Player* pPlayer = i->getSource())
                {
                    if (pPlayer->IsAlive() && pPlayer->IsInWater())
                    {
                        PlayerInWater = true;
                        if (Water == WATERSTATE_SCALDING)
                        {
                            if (!pPlayer->HasAura(SPELL_SCALDINGWATER))
                            {
                                if (ScaldingWaterDelayer.Expired(diff)) // this timer (delayer) prevents multiple application of this buff when player jumps in water (sometimes >3k damage)
                               {
                                  pPlayer->CastSpell(pPlayer, SPELL_SCALDINGWATER, true);
                                  ScaldingWaterDelayer = 500;
                                  break;
                               }
                               break;
                            }
                        }
                        else if (Water == WATERSTATE_FRENZY)
                        {
                            //spawn frenzy
                            if (DoSpawnFrenzy && !pPlayer->IsGameMaster())
                            {
                               if (Creature* frenzy = pPlayer->SummonCreature(MOB_COILFANG_FRENZY,pPlayer->GetPositionX(),pPlayer->GetPositionY(),pPlayer->GetPositionZ(),pPlayer->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,5000))
                                  frenzy->AI()->AttackStart(pPlayer);

                                DoSpawnFrenzy = false;
                            }
                        }
                        else if (Water == WATERSTATE_NONE)
                        {
                            break;
                        }
                    }
                }
            }

            if (PlayerInWater)
                PlayerInWaterTimer = 5000;
            else
            {
                if (PlayerInWaterTimer.Expired(diff))
                    PlayerInWaterTimer = 0;
            }

            if (PlayerInWaterTimer.GetInterval())  //FIXME: after changing to imp.PCAT not sure if that will work
                WaterCheckTimer = 1;
            else
                WaterCheckTimer = 1000; //remove stress from core
        }
        

        if (FrenzySpawnTimer.Expired(diff))
        {
            DoSpawnFrenzy = true;
            FrenzySpawnTimer = 500;
        }
    }

    void OnPlayerDealDamage(Player* plr, uint32 amount)
    {
        m_gbk.DamageDone(plr->GetGUIDLow(), amount);
    }

    void OnPlayerHealDamage(Player* plr, uint32 amount)
    {
        m_gbk.HealingDone(plr->GetGUIDLow(), amount);
    }

    void OnPlayerDeath(Player* plr)
    {
        m_gbk.PlayerDied(plr->GetGUIDLow());
    }
};

InstanceData* GetInstanceData_instance_serpentshrine_cavern(Map* map)
{
    return new instance_serpentshrine_cavern(map);
}

void AddSC_instance_serpentshrine_cavern()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "instance_serpent_shrine";
    newscript->GetInstanceData = &GetInstanceData_instance_serpentshrine_cavern;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_bridge_console";
    newscript->pGOUse = &GOUse_go_bridge_console;
    newscript->RegisterSelf();
}
