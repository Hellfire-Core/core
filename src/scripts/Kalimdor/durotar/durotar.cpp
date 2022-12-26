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

/* ScriptData
SDName: Durotar
SD%Complete: 100
SDComment: Quest support: 5441.
SDCategory: Durotar
EndScriptData */

/* ContentData
npc_lazy_peon
EndContentData */

#include "scriptPCH.h"

/*######
## npc_lazy_peon
######*/

enum LazyPeon
{
    SAY_SPELL_HIT             = -1000622,

    MIN_TIME_TO_GO_ASLEEP     = 30000,        // 30 to 60 sec, buff has 120 sec max duration
    MAX_TIME_TO_GO_ASLEEP     = 60000,

    QUEST_LAZY_PEONS          = 5441,
    GO_LUMBERPILE             = 175784,
    SPELL_BUFF_SLEEP          = 17743,
    SPELL_AWAKEN_PEON         = 19938
};

struct npc_lazy_peonAI : public ScriptedAI
{
    npc_lazy_peonAI(Creature *c) : ScriptedAI(c) {}

    Timer m_uiRebuffTimer;
    bool work;

    void Reset ()
    {
        m_uiRebuffTimer.Reset(urand(MIN_TIME_TO_GO_ASLEEP, MAX_TIME_TO_GO_ASLEEP));
        work = false;
    }

    void MovementInform(uint32 /*type*/, uint32 id)
    {
        if (id == 1)
        {
            work = true;
            if (GameObject* Lumberpile = FindGameObject(GO_LUMBERPILE, 20, me))
                me->SetFacingToObject(Lumberpile);
        }
    }

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if (spell->Id == SPELL_AWAKEN_PEON && caster->GetTypeId() == TYPEID_PLAYER)
        {
            DoScriptText(SAY_SPELL_HIT, me, caster);
            me->RemoveAllAuras();
            m_uiRebuffTimer.Reset(urand(MIN_TIME_TO_GO_ASLEEP, MAX_TIME_TO_GO_ASLEEP)); // refresh the buff
            if (GameObject* Lumberpile = FindGameObject(GO_LUMBERPILE, 20, me))
                me->GetMotionMaster()->MovePoint(1,Lumberpile->GetPositionX()-1,Lumberpile->GetPositionY(),Lumberpile->GetPositionZ());
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (work == true)
            me->HandleEmoteCommand(466);

        if (m_uiRebuffTimer.Expired(uiDiff))
        {
            DoCast(me, SPELL_BUFF_SLEEP);
            m_uiRebuffTimer = MAX_TIME_TO_GO_ASLEEP; // refresh the buff
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_lazy_peon(Creature* pCreature)
{
    return new npc_lazy_peonAI(pCreature);
}

void AddSC_durotar()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_lazy_peon";
    newscript->GetAI = &GetAI_npc_lazy_peon;
    newscript->RegisterSelf();
}
