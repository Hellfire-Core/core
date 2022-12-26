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
SDName: Instance_Blackwing_Lair
SD%Complete: 0
SDComment:
SDCategory: Blackwing Lair
EndScriptData */

#include "scriptPCH.h"
#include "def_blackwing_lair.h"

struct instance_blackwing_lair: public ScriptedInstance
{
    instance_blackwing_lair(Map *map) : ScriptedInstance(map), VictorNefariusGuid(0)
    {
        Initialize();
    };

    uint32 Encounters[NUM_ENCOUNTERS];
    uint64 VictorNefariusGuid;

    void Initialize()
    {
        for (uint32& state : Encounters)
            state = NOT_STARTED;
    }

    bool IsEncounterInProgress() const
    {
        for (const uint32& state : Encounters)
            if (state != NOT_STARTED && state != DONE)
                return true;
        return false;
    }

    void OnObjectCreate(GameObject *go)
    {
    }

    uint32 GetEncounterForEntry(uint32 entry)
    {
        switch (entry)
        {
            case NPC_RAZORGORE:
                return DATA_RAZORGORE_THE_UNTAMED_EVENT;
            case NPC_VAELASTRASZ:
                return DATA_VAELASTRASZ_THE_CORRUPT_EVENT;
            case NPC_BROODLORD_LASHLAYER:
                return DATA_BROODLORD_LASHLAYER_EVENT;
            case NPC_FIREMAW:
                return DATA_FIREMAW_EVENT;
            case NPC_EBONROC:
                return DATA_EBONROC_EVENT;
            case NPC_FLAMEGOR:
                return DATA_FLAMEGOR_EVENT;
            case NPC_CHROMAGGUS:
                return DATA_CHROMAGGUS_EVENT;
            case NPC_NEFARIAN:
            case NPC_VICTOR_NEFARIUS:
                return DATA_NEFARIAN_EVENT;
            default:
                return 0;
        }
    }

    void OnCreatureCreate(Creature *creature, uint32 creature_entry)
    {
        if (creature->GetEntry() == NPC_VICTOR_NEFARIUS)
            SetData64(NPC_VICTOR_NEFARIUS, creature->GetGUID());

        const CreatureData *tmp = creature->GetLinkedRespawnCreatureData();
        if (!tmp)
            return;

        if (GetEncounterForEntry(tmp->id) && creature->IsAlive() && GetData(GetEncounterForEntry(tmp->id)) == DONE)
        {
            creature->setDeathState(JUST_DIED);
            creature->RemoveCorpse();
            return;
        }
    }

    void SetData64(uint32 type, uint64 data)
    {
        switch (type)
        {
            case NPC_VICTOR_NEFARIUS:
                VictorNefariusGuid = data;
                break;
        }
    }

    uint64 GetData64(uint32 identifier)
    {
        switch (identifier)
        {
            case NPC_VICTOR_NEFARIUS:
                return VictorNefariusGuid;
            default:
                return 0;
        }
    }

    void SetData(uint32 type, uint32 data)
    {
        switch (type)
        {
            case DATA_RAZORGORE_THE_UNTAMED_EVENT:
                if (Encounters[0] != DONE)
                    Encounters[0] = data;
                break;
            case DATA_VAELASTRASZ_THE_CORRUPT_EVENT:
                if (Encounters[1] != DONE)
                    Encounters[1] = data;
                break;
            case DATA_BROODLORD_LASHLAYER_EVENT:
                if (Encounters[2] != DONE)
                    Encounters[2] = data;
                break;
            case DATA_FIREMAW_EVENT:
                if (Encounters[3] != DONE)
                    Encounters[3] = data;
                break;
            case DATA_EBONROC_EVENT:
                if (Encounters[4] != DONE)
                    Encounters[4] = data;
                break;
            case DATA_FLAMEGOR_EVENT:
                if (Encounters[5] != DONE)
                    Encounters[5] = data;
                break;
            case DATA_CHROMAGGUS_EVENT:
                if (Encounters[6] != DONE)
                    Encounters[6] = data;
                break;
            case DATA_NEFARIAN_EVENT:
                if (Encounters[7] != DONE)
                    Encounters[7] = data;
                break;
        }

        if (data == DONE)
            SaveToDB();
    }

    uint32 GetData(uint32 type)
    {
        switch (type)
        {
            case DATA_RAZORGORE_THE_UNTAMED_EVENT:
                return Encounters[0];
            case DATA_VAELASTRASZ_THE_CORRUPT_EVENT:
                return Encounters[1];
            case DATA_BROODLORD_LASHLAYER_EVENT:
                return Encounters[2];
            case DATA_FIREMAW_EVENT:
                return Encounters[3];
            case DATA_EBONROC_EVENT:
                return Encounters[4];
            case DATA_FLAMEGOR_EVENT:
                return Encounters[5];
            case DATA_CHROMAGGUS_EVENT:
                return Encounters[6];
            case DATA_NEFARIAN_EVENT:
                return Encounters[7];
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
        stream << Encounters[5] << " ";
        stream << Encounters[6] << " ";
        stream << Encounters[7];

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
        stream >> Encounters[0];
        stream >> Encounters[1];
        stream >> Encounters[2];
        stream >> Encounters[3];
        stream >> Encounters[4];
        stream >> Encounters[5];
        stream >> Encounters[6];
        stream >> Encounters[7];

        for (uint32& state : Encounters)
            if (state == IN_PROGRESS)                // Do not load an encounter as "In Progress" - reset it instead.
                state = NOT_STARTED;

        OUT_LOAD_INST_DATA_COMPLETE;
    }

    void Update(uint32 diff)
    {
    }
};

InstanceData* GetInstanceData_instance_blackwing_lair(Map* map)
{
    return new instance_blackwing_lair(map);
}

void AddSC_instance_blackwing_lair()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "instance_blackwing_lair";
    newscript->GetInstanceData = &GetInstanceData_instance_blackwing_lair;
    newscript->RegisterSelf();
}
