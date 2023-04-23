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
SDName: Boss_Scorn
SD%Complete: 100
SDComment:
SDCategory: Scarlet Monastery
EndScriptData */

#include "scriptPCH.h"

#define SPELL_LICHSLAP                  28873
#define SPELL_FROSTBOLTVOLLEY           8398
#define SPELL_MINDFLAY                  17313
#define SPELL_FROSTNOVA                 15531

struct boss_scornAI : public ScriptedAI
{
    boss_scornAI(Creature *c) : ScriptedAI(c) {}

    int32 LichSlap_Timer;
    int32 FrostboltVolley_Timer;
    int32 MindFlay_Timer;
    int32 FrostNova_Timer;

    void Reset()
    {
        LichSlap_Timer = 45000;
        FrostboltVolley_Timer = 30000;
        MindFlay_Timer = 30000;
        FrostNova_Timer = 30000;
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        LichSlap_Timer -= diff;
        if (LichSlap_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_LICHSLAP);
            LichSlap_Timer += 45000;
        }

        FrostboltVolley_Timer -= diff;
        if (FrostboltVolley_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_FROSTBOLTVOLLEY);
            FrostboltVolley_Timer += 20000;
        }

        MindFlay_Timer -= diff;
        if (MindFlay_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_MINDFLAY);
            MindFlay_Timer += 20000;
        }

        FrostNova_Timer -= diff;
        if (FrostNova_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_FROSTNOVA);
            FrostNova_Timer += 15000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_scorn(Creature *_Creature)
{
    return new boss_scornAI (_Creature);
}

void AddSC_boss_scorn()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_scorn";
    newscript->GetAI = &GetAI_boss_scorn;
    newscript->RegisterSelf();
}

