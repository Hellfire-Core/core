/* 
 * Copyright (C) 2008-2015 Hellground <http://hellground.net/>
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
SDName: instance_zul_farrak
SD%Complete: 0!
SDComment: basicaly pyramid event, TODO: make captives fight together against mobs and players; proper implementation of end door
SDCategory: Zul'Farrak
EndScriptData 

BLAH BLAH. CODE TO REWRITE. WTF IS GOING ON WITH wavecounter ??

*/

#include "precompiled.h"
#include "def_zul_farrak.h"

#define ENCOUNTERS 2

static float ZFWPs[10][3] = {
{1887.35f,1263.67f,41.48f},
{1890.87f,1263.86f,41.41f},
{1883.12f,1263.76f,41.59f},
{1890.72f,1268.39f,41.47f},
{1882.84f,1267.99f,41.73f},
{1885.85f,1202.20f,8.88f},
{1889.46f,1204.23f,8.88f},
{1887.41f,1208.92f,8.88f},
{1895.49f,1204.23f,8.88f},
{1876.23f,1207.52f,8.88f}};

static float spawns[26][4] ={
{1902.83f,    1223.41f,    8.96f,    3.95f},
{1894.64f,    1206.29f,    8.87f,    2.22f},
{1874.45f,    1204.44f,    8.87f,    0.88f},
{1874.18f,    1221.24f,    9.21f,    0.84f},
{1879.02f,    1223.06f,    9.12f,    5.91f},
{1882.07f,    1225.70f,    9.32f,    5.69f},
{1886.97f,    1225.86f,    9.85f,    5.79f},
{1892.28f,    1225.49f,    9.57f,    5.63f},
{1894.72f,    1221.91f,    8.87f,    2.34f},
{1890.08f,    1218.68f,    8.87f,    1.59f},
{1883.50f,    1218.25f,    8.90f,    0.67f},
{1886.93f,    1221.40f,    8.94f,    1.60f},
{1883.76f,    1222.30f,    9.11f,    6.26f},
{1889.82f,    1222.51f,    9.03f,    0.97f},
{1898.23f,    1217.97f,    8.87f,    3.42f},
{1877.40f,    1216.41f,    8.97f,    0.37f},
{1893.07f,    1215.26f,    8.87f,    3.08f},
{1878.57f,    1214.16f,    8.87f,    3.12f},
{1889.94f,    1212.21f,    8.87f,    1.59f},
{1883.74f,    1212.35f,    8.87f,    1.59f},
{1877.00f,    1207.27f,    8.87f,    3.80f},
{1873.63f,    1204.65f,    8.87f,    0.61f},
{1896.46f,    1205.62f,    8.87f,    5.72f},
{1899.63f,    1202.52f,    8.87f,    2.46f},
{1889.23f,    1207.72f,    8.87f,    1.47f},
{1879.77f,    1207.96f,    8.87f,    1.55f}
};

static uint32 spawnentries[8] = {7789,7787,7787,8876,7788,8877,7275,7796};

struct instance_zul_farrak : public ScriptedInstance
{
    instance_zul_farrak(Map *map) : ScriptedInstance(map) {Initialize();};

    uint32 Encounter[ENCOUNTERS];
    uint8 waves;
    uint32 wavecounter;
    uint64 captives[5];
    uint64 doorsGUID;

    void Initialize()
    {
        for(uint8 i = 0; i < ENCOUNTERS; i++)
            Encounter[i] = NOT_STARTED;
        waves = 0;
        wavecounter = 0;
        for(uint8 i = 0; i < 5; i++)
            captives[i] = 0;
        doorsGUID = 0;
    }

    void OnGameObjectCreate(GameObject *go, bool)
    {
        if (go->GetEntry() >= CAGES_BEGIN && go->GetEntry() <= CAGES_END )
            if (Encounter[0] > IN_PROGRESS)
                go->SetGoState(GO_STATE_ACTIVE);
        if (go->GetEntry() == DOOR_ENTRY && Encounter[1] == DONE)
        {
            go->SetGoState(GO_STATE_ACTIVE);
            doorsGUID = go->GetGUID();
        }
    }

    void OnCreatureCreate(Creature *c, bool)
    {
        if (c->GetEntry() >= CAPTIVES_BEGIN && c->GetEntry() <= CAPTIVES_END )
        {
            if (Encounter[0] != NOT_STARTED)
                c->ForcedDespawn();
            captives[c->GetEntry() - CAPTIVES_BEGIN] = c->GetGUID();
        }
    }

    std::string GetSaveData()
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream stream;
        stream << Encounter[0] << " " << Encounter[1];

        OUT_SAVE_INST_DATA_COMPLETE;
        return stream.str();
    }

    void Load(const char* in)
    {
        if (!in)
        {
            OUT_LOAD_INST_DATA_FAIL;
            return;
        }

        OUT_LOAD_INST_DATA(in);

        std::istringstream loadStream(in);
        loadStream >> Encounter[0] >> Encounter[1];

        if (Encounter[0] == IN_PROGRESS)
            Encounter[0] = NOT_STARTED;

        OUT_LOAD_INST_DATA_COMPLETE;
    }

    void SetData(uint32 Data, uint32 Value)
    {
        if (Data == DATA_PYRAMID_BATTLE)
        {
            if (Value == IN_PROGRESS && Encounter[0] == NOT_STARTED)
            Encounter[0] = Value;
            else if (Value == SPECIAL)
            --wavecounter;
        }
        else if (Data == DATA_DOOR_EVENT && Encounter[1] != DONE)
            Encounter[1] = Value;
        if (Data == DATA_DOOR_EVENT && Value == DONE)
            if (GameObject * go = instance->GetGameObject(doorsGUID))
                go->SetGoState(GO_STATE_ACTIVE);
        if (Value >= FAIL)
            SaveToDB();

    }

    uint32 GetData(uint32 Data)
    {
        if (Data == DATA_PYRAMID_BATTLE)
            return Encounter[0];
        if (Data == DATA_DOOR_EVENT)
            return Encounter[1];
        return 0;
    }

    void Update(uint32 diff)
    {
        if (Encounter[0] == IN_PROGRESS && wavecounter == 0)
        {
            if (waves == 0)
                for (uint8 i = 0; i < 5; i++)
                    if (Creature *c = GetCreature(captives[i]))
                        {
                            c->SetWalk(true);
                            c->GetMotionMaster()->MovePoint(0,ZFWPs[i][0],ZFWPs[i][1],ZFWPs[i][2]);
                        }
            if (waves == 0 || waves == 1 || waves == 2)
                for (uint8 j = 0; j < 4; j++)
                    for (uint8 i = 0; i < 6; i++)
                        if (Player * p = instance->GetPlayers().begin()->getSource())
                            if (Creature* c = p->SummonCreature(spawnentries[i],spawns[j*6 + i][0],spawns[j*6 + i][1],spawns[j*6 + i][2],spawns[j*6 + i][3],TEMPSUMMON_CORPSE_TIMED_DESPAWN,60000))
                                wavecounter++;
            if (waves == 3)
                wavecounter = 15000;
            if (waves >= 4)
                Encounter[0] = DONE;

            if (waves == 2)
            {
                for (uint8 i = 0; i < 5; i++)
                    if (Creature *c = GetCreature(captives[i]))
                    {
                        c->GetMotionMaster()->MovePoint(1,ZFWPs[i+5][0],ZFWPs[i+5][1],ZFWPs[i+5][2]);
                        c->SetHomePosition(ZFWPs[i+5][0],ZFWPs[i+5][1],ZFWPs[i+5][2],0);
                    }
                if (Player * p = instance->GetPlayers().begin()->getSource())
                {
                    if (Creature* c = p->SummonCreature(spawnentries[6],spawns[24][0],spawns[24][1],spawns[24][2],spawns[24][3],TEMPSUMMON_CORPSE_TIMED_DESPAWN,60000))
                        wavecounter++;
                    if (Creature* c = p->SummonCreature(spawnentries[7],spawns[25][0],spawns[25][1],spawns[25][2],spawns[25][3],TEMPSUMMON_CORPSE_TIMED_DESPAWN,60000))
                        wavecounter++;
                }
            }
            
            waves++;
        }
        
        if (waves == 4)
        {
            if (wavecounter <= diff)
                wavecounter = 0;
            else
                wavecounter -= diff;
        }
    }
};

InstanceData* GetInstanceData_instance_zul_farrak(Map* map)
{
    return new instance_zul_farrak(map);
}

void AddSC_instance_zul_farrak()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_zul_farrak";
    newscript->GetInstanceData = &GetInstanceData_instance_zul_farrak;
    newscript->RegisterSelf();
}