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

/* ScriptData
SDName: Dire_Maul
SD%Complete: ?
SDComment: Quest support: 7481, 7482
SDCategory: Dire_Maul
EndScriptData */

/* ContentData
go_kariel_remains
EndContentData */

#include "scriptPCH.h"

#define QUEST_ELVEN_LEGENDS_HORDE 7481
#define QUEST_ELVEN_LEGENDS_ALLY 7482
bool GOUse_go_kariel_remains(Player *player, GameObject* _GO)
{
    player->AreaExploredOrEventHappens(QUEST_ELVEN_LEGENDS_ALLY);
    player->AreaExploredOrEventHappens(QUEST_ELVEN_LEGENDS_HORDE);
    return true;
}

#define NPC_STOMPER_KREEG 14322
struct instance_dire_maul : public ScriptedInstance
{
    instance_dire_maul(Map *map) : ScriptedInstance(map) { kingdead = false; };

    bool kingdead;
    uint64 stomperGuid;

    void OnCreatureCreate(Creature *creature, uint32 creature_entry)
    {
        if (creature_entry == NPC_STOMPER_KREEG)
        {
            stomperGuid = creature->GetGUID();
            if (kingdead)
                creature->setFaction(35);
        }
    }


    void SetData(uint32 type, uint32 data)
    {
        if (type == 1)
        {
            kingdead = (data > 0);
            if (Creature* stomper = GetCreature(stomperGuid))
                stomper->setFaction(kingdead ? 35 : 1374);
            SaveToDB();
        }
    }

    uint32 GetData(uint32 data)
    {
        if (data == 1)
            return kingdead;
        return 0;
    }


    std::string GetSaveData()
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream stream;
        stream << kingdead;

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
        stream >> kingdead;

        OUT_LOAD_INST_DATA_COMPLETE;
    }
};


InstanceData* GetInstanceData_instance_dire_maul(Map* map)
{
    return new instance_dire_maul(map);
}

void AddSC_dire_maul()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name="go_kariel_remains";
    newscript->pGOUse =  &GOUse_go_kariel_remains;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "instance_dire_maul";
    newscript->GetInstanceData = &GetInstanceData_instance_dire_maul;
    newscript->RegisterSelf();
}