/*
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

#include "scriptPCH.h"

enum
{
    GOB_STATUE_START = 148830,//to 148835, should be used in same order as their ids
    GOB_ALTAR = 148836,
    GOB_IDOL = 148838,
    CREATURE_ATALALARION = 8580,

    DATA_STATUES = 1,
};

float spawnpos[3] = { -480,96,-189 };

struct instance_sunken_temple : public ScriptedInstance
{
    instance_sunken_temple(Map *map) : ScriptedInstance(map) { Statues = 0; };

    uint8 Statues; // 0-5 clicking, 6-all clicked boss and obj spawned,7-boss dead
    uint64 altarguid;

    void OnGameObjectCreate(GameObject *go, bool add)
    {
        if (go->GetEntry() == GOB_ALTAR && add)
        {
            altarguid = go->GetGUID();
            if (Statues > 5)
                go->SummonGameObject(GOB_IDOL, spawnpos[0], spawnpos[1], spawnpos[2], 0, 0, 0, 0, 0, 5000);
            if (Statues == 6)
                go->SummonCreature(CREATURE_ATALALARION, spawnpos[0], spawnpos[1], spawnpos[2], 0, TEMPSUMMON_DEAD_DESPAWN, 60000);
        }
    }


    void SetData(uint32 type, uint32 data)
    {
        if (type != DATA_STATUES)
            return;

        if (data == IN_PROGRESS)
        {
            Statues++;
            if (Statues == 6)
            {
                if (GameObject* go = instance->GetGameObject(altarguid))
                {
                    go->SummonCreature(CREATURE_ATALALARION, spawnpos[0], spawnpos[1], spawnpos[2], 0, TEMPSUMMON_DEAD_DESPAWN, 60000);
                    go->SummonGameObject(GOB_IDOL, spawnpos[0], spawnpos[1], spawnpos[2], 0, 0, 0, 0, 0, 5000);
                }
            }
        }
        else if (data == FAIL) // wrong statue used
        {
            Statues = 0;
        }
        else if (data == DONE) // atalalalala killed
        {
            Statues = 7;
        }
        SaveToDB();
    }

    uint32 GetData(uint32 data)
    {
        if (data == DATA_STATUES)
            return Statues;
        return 0;
    }


    std::string GetSaveData()
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream stream;
        stream << Statues;

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

        std::istringstream stream(in);
        stream >> Statues;

        OUT_LOAD_INST_DATA_COMPLETE;
    }
};


InstanceData* GetInstanceData_instance_sunken_temple(Map* map)
{
    return new instance_sunken_temple(map);
}

bool GO_use_atalai_statue(Player* plr, GameObject* gob)
{
    InstanceData* inst = gob->GetInstanceData();
    if (!inst)
        return true;

    if (inst->GetData(DATA_STATUES) == (gob->GetEntry() - GOB_STATUE_START))
        inst->SetData(DATA_STATUES, IN_PROGRESS);
    else
        inst->SetData(DATA_STATUES, FAIL);

    return true;
}

void AddSC_sunken_temple()
{
    Script* newscript;
    newscript = new Script;
    newscript->Name = "instance_sunken_temple";
    newscript->GetInstanceData = &GetInstanceData_instance_sunken_temple;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "gob_atalai_statue";
    newscript->pGOUse = &GO_use_atalai_statue;
    newscript->RegisterSelf();
}