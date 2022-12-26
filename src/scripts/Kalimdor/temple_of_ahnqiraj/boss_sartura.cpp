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
SDName: Boss_Sartura
SD%Complete: 95
SDComment:
SDCategory: Temple of Ahn'Qiraj
EndScriptData */

#include "scriptPCH.h"
#include "def_temple_of_ahnqiraj.h"

#define SAY_AGGRO                   -1531008
#define SAY_SLAY                    -1531009
#define SAY_DEATH                   -1531010

#define SPELL_WHIRLWIND                              26083
#define SPELL_ENRAGE                                 28747            //Not sure if right ID.
#define SPELL_ENRAGEHARD                             28798

//Guard Spell
#define SPELL_WHIRLWINDADD                           26038
#define SPELL_KNOCKBACK                              26027


struct boss_sarturaAI : public ScriptedAI
{
    boss_sarturaAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    Timer WhirlWind_Timer;
    Timer WhirlWindRandom_Timer;
    Timer WhirlWindEnd_Timer;
    Timer AggroReset_Timer;
    Timer AggroResetEnd_Timer;
    Timer EnrageHard_Timer;

    bool Enraged;
    bool EnragedHard;
    bool WhirlWind;
    bool AggroReset;

    void Reset()
    {
        WhirlWind_Timer.Reset(30000);
        WhirlWindRandom_Timer.Reset(3000 + rand() % 4000);
        WhirlWindEnd_Timer.Reset(15000);
        AggroReset_Timer.Reset(45000 + rand() % 10000);
        AggroResetEnd_Timer.Reset(5000);
        EnrageHard_Timer.Reset(10 * 60000);

        WhirlWind = false;
        AggroReset = false;
        Enraged = false;
        EnragedHard = false;

        if (pInstance)
            pInstance->SetData(DATA_BATTLEGUARD_SARTURA, NOT_STARTED);

    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);
        if (pInstance)
            pInstance->SetData(DATA_BATTLEGUARD_SARTURA, IN_PROGRESS);
    }

     void JustDied(Unit* Killer)
     {
         DoScriptText(SAY_DEATH, m_creature);
         if (pInstance)
            pInstance->SetData(DATA_BATTLEGUARD_SARTURA, DONE);
     }

     void KilledUnit(Unit* victim)
     {
         DoScriptText(SAY_SLAY, m_creature);
     }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (WhirlWind)
        {
            if (WhirlWindRandom_Timer.Expired(diff))
            {
                //Attack random Gamers
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 1, 200, true, m_creature->getVictimGUID()))
                    AttackStart(target);

                WhirlWindRandom_Timer = 3000 + rand() % 4000;
            }

            if (WhirlWindEnd_Timer.Expired(diff))
            {
                WhirlWind = false;
                WhirlWind_Timer = 25000 + rand()%15000;
            }
        }

        if (!WhirlWind)
        {
            if (WhirlWind_Timer.Expired(diff))
            {
                DoCast(m_creature, SPELL_WHIRLWIND);
                WhirlWind = true;
                WhirlWindEnd_Timer = 15000;
            }

            if (AggroReset_Timer.Expired(diff))
            {
                //Attack random Gamers
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,1, 200, true, m_creature->getVictimGUID()))
                    m_creature->TauntApply(target);

                    AggroReset = true;
                    AggroReset_Timer = 2000 + rand()%3000;
            }

            if (AggroReset)
            {
                if (AggroResetEnd_Timer.Expired(diff))
                {
                    AggroReset = false;
                    AggroResetEnd_Timer = 5000;
                    AggroReset_Timer = 35000 + rand()%10000;
                } 
            }

            //If she is 20% enrage
            if (!Enraged)
            {
                if (m_creature->GetHealth()*100 / m_creature->GetMaxHealth() <= 20 && !m_creature->IsNonMeleeSpellCast(false))
                {
                    DoCast(m_creature, SPELL_ENRAGE);
                    Enraged = true;
                }
            }

            //After 10 minutes hard enrage
            if (!EnragedHard)
            {
                if (EnrageHard_Timer.Expired(diff))
                {
                    DoCast(m_creature, SPELL_ENRAGEHARD);
                    EnragedHard = true;
                } 
            }

            DoMeleeAttackIfReady();
        }
    }
};

struct mob_sartura_royal_guardAI : public ScriptedAI
{
    mob_sartura_royal_guardAI(Creature *c) : ScriptedAI(c) {}

    Timer WhirlWind_Timer;
    Timer WhirlWindRandom_Timer;
    Timer WhirlWindEnd_Timer;
    Timer AggroReset_Timer;
    Timer AggroResetEnd_Timer;
    Timer KnockBack_Timer;

    bool WhirlWind;
    bool AggroReset;

    void Reset()
    {
        WhirlWind_Timer.Reset(30000);
        WhirlWindRandom_Timer.Reset(3000 + rand() % 4000);
        WhirlWindEnd_Timer.Reset(15000);
        AggroReset_Timer.Reset(45000 + rand() % 10000);
        AggroResetEnd_Timer.Reset(5000);
        KnockBack_Timer.Reset(10000);

        WhirlWind = false;
        AggroReset = false;
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (!WhirlWind && WhirlWind_Timer.Expired(diff))
        {
            DoCast(m_creature, SPELL_WHIRLWINDADD);
            WhirlWind = true;
            WhirlWind_Timer = 25000 + rand()%15000;
            WhirlWindEnd_Timer = 15000;
        }

        if (WhirlWind)
        {
            if (WhirlWindRandom_Timer.Expired(diff))
            {
                //Attack random Gamers
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,1, 200, true, m_creature->getVictimGUID()))
                    m_creature->TauntApply(target);

                WhirlWindRandom_Timer = 3000 + rand()%4000;
            }

            if (WhirlWindEnd_Timer.Expired(diff))
            {
                WhirlWind = false;
            }
        }

        if (!WhirlWind)
        {
            if (AggroReset_Timer.Expired(diff))
            {
                //Attack random Gamers
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,1, 200, true, m_creature->getVictimGUID()))
                    AttackStart(target);

                AggroReset = true;
                AggroReset_Timer = 2000 + rand()%3000;
            }

            if (KnockBack_Timer.Expired(diff))
            {
                DoCast(m_creature, SPELL_WHIRLWINDADD);
                KnockBack_Timer = 10000 + rand()%10000;
            }
        }

        if (AggroReset)
        {
            if (AggroResetEnd_Timer.Expired(diff))
            {
                AggroReset = false;
                AggroResetEnd_Timer = 5000;
                AggroReset_Timer = 30000 + rand()%10000;
            } 
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_sartura(Creature *_Creature)
{
    return new boss_sarturaAI (_Creature);
}

CreatureAI* GetAI_mob_sartura_royal_guard(Creature *_Creature)
{
    return new mob_sartura_royal_guardAI (_Creature);
}

void AddSC_boss_sartura()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_sartura";
    newscript->GetAI = &GetAI_boss_sartura;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_sartura_royal_guard";
    newscript->GetAI = &GetAI_mob_sartura_royal_guard;
    newscript->RegisterSelf();
}

