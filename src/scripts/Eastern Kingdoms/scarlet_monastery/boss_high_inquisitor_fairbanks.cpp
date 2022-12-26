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
SDName: Boss_High_Inquisitor_Faribanks
SD%Complete: 100
SDComment:
SDCategory: Scarlet Monastery
EndScriptData */

#include "scriptPCH.h"

#define SPELL_SLEEP2                    1090
#define SPELL_CURSEOFBLOOD              16098
#define SPELL_SMITE                     6060
#define SPELL_SHADOWWORDPAIN            2767
#define SPELL_FLASHHEAL4                9474
#define SPELL_RENEW6                    6078
#define SPELL_DEVOURINGPLAGUE3          19277
#define SPELL_MINDBLAST5                8105

struct boss_high_inquisitor_fairbanksAI : public ScriptedAI
{
    boss_high_inquisitor_fairbanksAI(Creature *c) : ScriptedAI(c) {}

    int32 Healing_Timer;
    int32 Sleep2_Timer;
    int32 Smite_Timer;
    int32 ShadowWordPain_Timer;
    int32 CurseOfBlood_Timer;
    int32 DevouringPlague3_Timer;
    int32 MindBlast5_Timer;

    void Reset()
    {
        Healing_Timer = 300;
        Sleep2_Timer = 45000;
        Smite_Timer = 30000;
        ShadowWordPain_Timer = 30000;
        CurseOfBlood_Timer = 45000;
        DevouringPlague3_Timer = 60000;
        MindBlast5_Timer = 20000;
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        Healing_Timer -= diff;
        //If we are <45% hp cast Renew rank 6 or Flash heal rank 4
        if ( m_creature->GetHealth()*100 / m_creature->GetMaxHealth() <= 45 && !m_creature->IsNonMeleeSpellCast(false) 
            && Healing_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_RENEW6 || SPELL_FLASHHEAL4);
            Healing_Timer += 30000;
        }

        Sleep2_Timer -= diff;
        if (Sleep2_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_SLEEP2);
            Sleep2_Timer += 45000;
        }

        Smite_Timer -= diff;
        if (Smite_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_SMITE);
            Smite_Timer += 20000;
        }

        ShadowWordPain_Timer -= diff;
        if (ShadowWordPain_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_SHADOWWORDPAIN);
            ShadowWordPain_Timer += 30000;
        }

        CurseOfBlood_Timer -= diff;
        if (CurseOfBlood_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_CURSEOFBLOOD);
            CurseOfBlood_Timer += 25000;
        }

        DevouringPlague3_Timer -= diff;
        if (DevouringPlague3_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_DEVOURINGPLAGUE3);
            DevouringPlague3_Timer += 35000;
        }

        MindBlast5_Timer -= diff;
        if (MindBlast5_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_MINDBLAST5);
            MindBlast5_Timer += 30000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_high_inquisitor_fairbanks(Creature *_Creature)
{
    return new boss_high_inquisitor_fairbanksAI (_Creature);
}

void AddSC_boss_high_inquisitor_fairbanks()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_high_inquisitor_fairbanks";
    newscript->GetAI = &GetAI_boss_high_inquisitor_fairbanks;
    newscript->RegisterSelf();
}

