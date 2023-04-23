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
SDName: Boss_Vectus
SD%Complete: 100
SDComment:
SDCategory: Scholomance
EndScriptData */

#include "scriptPCH.h"

#define SPELL_FIRESHIELD        19626
#define SPELL_BLASTWAVE         13021
#define SPELL_FRENZY            28371

struct boss_vectusAI : public ScriptedAI
{
    boss_vectusAI(Creature *c) : ScriptedAI(c) {}

    int32 FireShield_Timer;
    int32 BlastWave_Timer;
    int32 Frenzy_Timer;

    void Reset()
    {
        FireShield_Timer = 2000;
        BlastWave_Timer = 14000;
        Frenzy_Timer = 0;
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        FireShield_Timer -= diff;
        if (FireShield_Timer <= diff)
        {
            DoCast(m_creature, SPELL_FIRESHIELD);
            FireShield_Timer += 90000;
        }

        BlastWave_Timer -= diff;
        if (BlastWave_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_BLASTWAVE);
            BlastWave_Timer += 12000;
        }

        //Frenzy_Timer
        if ( m_creature->GetHealth()*100 / m_creature->GetMaxHealth() < 25 )
        {
            Frenzy_Timer -= diff;
            if (Frenzy_Timer <= diff)
            {
                DoCast(m_creature,SPELL_FRENZY);
                DoTextEmote("goes into a killing frenzy!",NULL);

                Frenzy_Timer += 24000;
            }
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_vectus(Creature *_Creature)
{
    return new boss_vectusAI (_Creature);
}

void AddSC_boss_vectus()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_vectus";
    newscript->GetAI = &GetAI_boss_vectus;
    newscript->RegisterSelf();
}

