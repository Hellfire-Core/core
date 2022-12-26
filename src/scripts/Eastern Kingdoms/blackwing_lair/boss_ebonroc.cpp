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
SDName: Boss_Ebonroc
SD%Complete: 50
SDComment: Shadow of Ebonroc needs core support
SDCategory: Blackwing Lair
EndScriptData */

#include "scriptPCH.h"
#include "def_blackwing_lair.h"

#define SPELL_SHADOWFLAME           22539
#define SPELL_WINGBUFFET            18500
#define SPELL_SHADOWOFEBONROC       23340
#define SPELL_HEAL                  41386                   //Thea Heal spell of his Shadow

struct boss_ebonrocAI : public ScriptedAI
{
    boss_ebonrocAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    ScriptedInstance * pInstance;
    int32 ShadowFlame_Timer;
    int32 WingBuffet_Timer;
    int32 ShadowOfEbonroc_Timer;
    int32 Heal_Timer;

    void Reset()
    {
        ShadowFlame_Timer = 15000;                          //These times are probably wrong
        WingBuffet_Timer = 30000;
        ShadowOfEbonroc_Timer = 45000;
        Heal_Timer = 1000;

        if (pInstance && pInstance->GetData(DATA_EBONROC_EVENT) != DONE)
            pInstance->SetData(DATA_EBONROC_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoZoneInCombat();

        if (pInstance)
            pInstance->SetData(DATA_EBONROC_EVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_EBONROC_EVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim() )
            return;

        ShadowFlame_Timer -= diff;
        if (ShadowFlame_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_SHADOWFLAME);
            ShadowFlame_Timer += 12000 + rand()%3000;
        }

        WingBuffet_Timer -= diff;
        if (WingBuffet_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_WINGBUFFET);
            WingBuffet_Timer += 25000;
        }

        ShadowOfEbonroc_Timer -= diff;
        if (ShadowOfEbonroc_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_SHADOWOFEBONROC);
            ShadowOfEbonroc_Timer += 25000 + rand()%10000;
        }

        if (m_creature->GetVictim()->HasAura(SPELL_SHADOWOFEBONROC,0))
        {
            Heal_Timer -= diff;
            if (Heal_Timer <= diff)
            {
                DoCast(m_creature, SPELL_HEAL);
                Heal_Timer += 1000 + rand()%2000;
            }
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_ebonroc(Creature *_Creature)
{
    return new boss_ebonrocAI (_Creature);
}

void AddSC_boss_ebonroc()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_ebonroc";
    newscript->GetAI = &GetAI_boss_ebonroc;
    newscript->RegisterSelf();
}

