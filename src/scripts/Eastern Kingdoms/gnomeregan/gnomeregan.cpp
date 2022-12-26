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
#include "ScriptedFollowerAI.h"

enum eKernobee
{
    QUEST_A_FINE_MESS = 2904,
    LOC_X = -339,
    LOC_Y = 1,
    LOC_Z = -153
};

struct npc_kernobee : public FollowerAI
{
    npc_kernobee(Creature* c) : FollowerAI(c) {}

    void Reset() {}

    void UpdateFollowerAI(const uint32 uiDiff)
    {
        if (!UpdateVictim())
        {
            if (me->GetDistance(LOC_X, LOC_Y, LOC_Z) < 20)
            {
                if (Player* pPlayer = GetLeaderForFollower())
                {
                    if (pPlayer->GetQuestStatus(QUEST_A_FINE_MESS) == QUEST_STATUS_INCOMPLETE)
                        pPlayer->GroupEventHappens(QUEST_A_FINE_MESS, me);
                }
                SetFollowComplete();
            }
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_kernobee(Creature *_Creature)
{
    return new npc_kernobee(_Creature);
}

bool QuestAccept_npc_kernobee(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_A_FINE_MESS)
    {
        if (npc_kernobee* pkernobee = CAST_AI(npc_kernobee, pCreature->AI()))
        {
            pkernobee->StartFollow(pPlayer, 0, pQuest);
        }
        pCreature->SetStandState(UNIT_STAND_STATE_STAND);
    }

    return true;
}

void AddSC_gnomeregan()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_kernobee";
    newscript->GetAI = &GetAI_npc_kernobee;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_kernobee;
    newscript->RegisterSelf();
}