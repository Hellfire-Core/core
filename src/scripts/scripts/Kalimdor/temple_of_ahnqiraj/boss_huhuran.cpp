/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * Copyright (C) 2008-2015 Hellground <http://hellground.net/>
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
SDName: Boss_Huhuran
SD%Complete: 100
SDComment:
SDCategory: Temple of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "def_temple_of_ahnqiraj.h"

#define SPELL_FRENZY 26051
#define SPELL_BERSERK 26068
#define SPELL_POISONBOLT 26052
#define SPELL_NOXIOUSPOISON 26053
#define SPELL_WYVERNSTING 26180
#define SPELL_ACIDSPIT 26050

struct boss_huhuranAI : public ScriptedAI
{
    boss_huhuranAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;
    Timer Frenzy_Timer;
    Timer Wyvern_Timer;
    Timer Spit_Timer;
    Timer PoisonBolt_Timer;
    Timer NoxiousPoison_Timer;
    Timer FrenzyBack_Timer;

    bool Frenzy;
    bool Berserk;

    void Reset()
    {
        Frenzy_Timer.Reset(25000 + rand() % 10000);
        Wyvern_Timer.Reset(18000 + rand() % 10000);
        Spit_Timer.Reset(8000);
        PoisonBolt_Timer.Reset(4000);
        NoxiousPoison_Timer.Reset(10000 + rand() % 10000);
        FrenzyBack_Timer.Reset(15000);

        Frenzy = false;
        Berserk = false;

        if (pInstance)
            pInstance->SetData(DATA_PRINCESS_HUHURAN, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(DATA_PRINCESS_HUHURAN, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_PRINCESS_HUHURAN, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        //Frenzy_Timer
        if (!Frenzy && Frenzy_Timer.Expired(diff))
        {
            DoCast(m_creature, SPELL_FRENZY);
            Frenzy = true;
            PoisonBolt_Timer = 3000;
            Frenzy_Timer = 25000 + rand()%10000;
        }

        // Wyvern Timer
        if (Wyvern_Timer.Expired(diff))
        {
            if( Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0) )
                DoCast(target,SPELL_WYVERNSTING);
            Wyvern_Timer = 15000 + rand()%17000;
        }

        //Spit Timer
        if (Spit_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_ACIDSPIT);
            Spit_Timer = 5000 + rand()%5000;
        }

        //NoxiousPoison_Timer
        if (NoxiousPoison_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_NOXIOUSPOISON);
            NoxiousPoison_Timer = 12000 + rand()%12000;
        }

        //PoisonBolt only if frenzy or berserk
        if (Frenzy || Berserk)
        {
            if (PoisonBolt_Timer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(),SPELL_POISONBOLT);
                PoisonBolt_Timer = 3000;
            }
        }

        //FrenzyBack_Timer
        if (Frenzy && FrenzyBack_Timer.Expired(diff))
        {
            m_creature->InterruptNonMeleeSpells(false);
            Frenzy = false;
            FrenzyBack_Timer = 15000;
        }

        if ( !Berserk && m_creature->GetHealth()*100 / m_creature->GetMaxHealth() < 31 )
        {
            m_creature->InterruptNonMeleeSpells(false);
            DoTextEmote("is going berserk", NULL);
            DoCast(m_creature, SPELL_BERSERK);
            Berserk = true;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_huhuran(Creature *_Creature)
{
    return new boss_huhuranAI (_Creature);
}

void AddSC_boss_huhuran()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_huhuran";
    newscript->GetAI = &GetAI_boss_huhuran;
    newscript->RegisterSelf();
}

