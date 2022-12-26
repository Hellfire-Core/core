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
#include "ScriptedEscortAI.h"

#define QUEST_MISSING_IN_ACTION 219

struct npc_corporal_keeshanAI : public npc_escortAI
{
    npc_corporal_keeshanAI(Creature* pCreature) : npc_escortAI(pCreature) { }

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();
        if (pPlayer && i == 43)
            pPlayer->GroupEventHappens(QUEST_MISSING_IN_ACTION, m_creature);
    }

};

CreatureAI* GetAI_npc_corporal_keeshan(Creature* pCreature)
{
    return new npc_corporal_keeshanAI(pCreature);
}

bool QuestAccept_npc_corporal_keeshan(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_MISSING_IN_ACTION)
    {
        if (npc_escortAI* pEscortAI = CAST_AI(npc_corporal_keeshanAI, pCreature->AI()))
        {
            pEscortAI->Start(true, true, pPlayer->GetGUID(), quest, true);
            pCreature->setFaction(113);
        }
    }
    return false;
}

void AddSC_redridge()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_corporal_keeshan";
    newscript->GetAI = &GetAI_npc_corporal_keeshan;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_corporal_keeshan;
    newscript->RegisterSelf();
}
