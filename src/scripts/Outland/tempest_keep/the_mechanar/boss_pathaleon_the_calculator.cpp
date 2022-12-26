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
SDName: Boss Pathaleon the Calculator
SD%Complete: 95
SDComment:
SDCategory: Tempest Keep, The Mechanar
EndScriptData */

#include "scriptPCH.h"
#include "def_mechanar.h"

#define SAY_AGGRO                       -1554020
#define SAY_DOMINATION_1                -1554021
#define SAY_DOMINATION_2                -1554022
#define SAY_SUMMON                      -1554023
#define SAY_ENRAGE                      -1554024
#define SAY_SLAY_1                      -1554025
#define SAY_SLAY_2                      -1554026
#define SAY_DEATH                       -1554027

// Spells to be cast
#define SPELL_MANA_TAP                36021
#define SPELL_ARCANE_TORRENT          36022
#define SPELL_DOMINATION              35280
#define H_SPELL_ARCANE_EXPLOSION        15453
#define SPELL_FRENZY                    36992

#define SPELL_SUMMON_NETHER_WRAITH_1    35285               //Spells work, but not implemented
#define SPELL_SUMMON_NETHER_WRAITH_2    35286
#define SPELL_SUMMON_NETHER_WRAITH_3    35287
#define SPELL_SUMMON_NETHER_WRAITH_4    35288

// Add Spells
#define SPELL_DETONATION              35058
#define SPELL_ARCANE_MISSILES         35034

struct boss_pathaleon_the_calculatorAI : public ScriptedAI
{
    boss_pathaleon_the_calculatorAI(Creature *c) : ScriptedAI(c), summons(me)
    {
        pInstance = c->GetInstanceData();
        me->GetPosition(wLoc);
    }

    ScriptedInstance *pInstance;

    Timer Summon_Timer;
    SummonList summons;
    Timer ManaTap_Timer;
    Timer ArcaneTorrent_Timer;
    Timer Domination_Timer;
    Timer ArcaneExplosion_Timer;
    Timer Check_Timer;
    WorldLocation wLoc;
    bool Enraged;

    uint32 Counter;

    void Reset()
    {
        Summon_Timer.Reset(30000);
        ManaTap_Timer.Reset(12000 + rand() % 8000);
        ArcaneTorrent_Timer.Reset(16000 + rand() % 9000);
        Domination_Timer.Reset(25000 + rand() % 15000);
        ArcaneExplosion_Timer.Reset(8000 + rand() % 5000);

        Enraged = false;

        Counter = 0;
        summons.DespawnAll();

        if (pInstance && pInstance->GetData(DATA_BRIDGE_EVENT) != DONE)
        {
            me->SetVisibility(VISIBILITY_OFF);
            me->SetReactState(REACT_PASSIVE);
        }
    }
    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, me);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), me);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, me);

        summons.DespawnAll();
    }

    void JustSummoned(Creature *summon) { summons.Summon(summon); }
    void SummonedCreatureDespawn(Creature *summon) { summons.Despawn(summon); }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (Summon_Timer.Expired(diff))
        {
            for(int i = 0; i < 3;i++)
            {
                Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0);
                Creature* Wraith = me->SummonCreature(21062,me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(),0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                if (target && Wraith)
                    Wraith->AI()->AttackStart(target);
            }

            DoScriptText(SAY_SUMMON, me);
            Summon_Timer = 30000 + rand()%15000;
        }

        if (Check_Timer.Expired(diff))
        {
            if(!me->IsWithinDistInMap(&wLoc, 30.0f))
                EnterEvadeMode();
            else
                DoZoneInCombat();
                Check_Timer = 3000;
        }

        if (ManaTap_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_MANA_TAP);
            ManaTap_Timer = 14000 + rand()%8000;
        }

        if (ArcaneTorrent_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_ARCANE_TORRENT);
            ArcaneTorrent_Timer = 12000 + rand()%6000;
        }

        if (Domination_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 200, true, me->getVictimGUID()))
            {
                DoScriptText(RAND(SAY_DOMINATION_1, SAY_DOMINATION_2), me);

                DoCast(target,SPELL_DOMINATION);
            }
            Domination_Timer = 25000 + rand()%5000;
        }

        //Only casting if Heroic Mode is used
        if (HeroicMode)
        {
            if(ArcaneExplosion_Timer.Expired(diff))
            {
                DoCast(me->GetVictim(),H_SPELL_ARCANE_EXPLOSION);
                ArcaneExplosion_Timer = 10000 + rand()%4000;
            }
        }

        if (!Enraged && me->GetHealth()*100 / me->GetMaxHealth() < 21)
        {
            DoCast(me, SPELL_FRENZY);
            DoScriptText(SAY_ENRAGE, me);
            Enraged = true;

        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_pathaleon_the_calculator(Creature *_Creature)
{
    return new boss_pathaleon_the_calculatorAI (_Creature);
}

struct mob_nether_wraithAI : public ScriptedAI
{
    mob_nether_wraithAI(Creature *c) : ScriptedAI(c) {}

    ScriptedInstance *pInstance;

    Timer ArcaneMissiles_Timer;
    Timer Detonation_Timer;
    Timer Die_Timer;
    bool Detonation;

    void Reset()
    {
        ArcaneMissiles_Timer.Reset(1000 + rand() % 3000);
        Detonation_Timer.Reset(20000);
        Die_Timer.Reset(2200);
        Detonation = false;

    }

    void EnterCombat(Unit* who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(ArcaneMissiles_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 200, true, me->getVictimGUID()))
                DoCast(target,SPELL_ARCANE_MISSILES);
            else
                DoCast(me->GetVictim(),SPELL_ARCANE_MISSILES);

            ArcaneMissiles_Timer = 5000 + rand()%5000;
        }

        if (!Detonation && Detonation_Timer.Expired(diff))
        {
            DoCast(me,SPELL_DETONATION);
            Detonation = true;
        }

        if (Detonation && Die_Timer.Expired(diff))
        {
            me->setDeathState(JUST_DIED);
            me->RemoveCorpse();
        }

        DoMeleeAttackIfReady();
    }

};
CreatureAI* GetAI_mob_nether_wraith(Creature *_Creature)
{
    return new mob_nether_wraithAI (_Creature);
}

void AddSC_boss_pathaleon_the_calculator()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_pathaleon_the_calculator";
    newscript->GetAI = &GetAI_boss_pathaleon_the_calculator;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_nether_wraith";
    newscript->GetAI = &GetAI_mob_nether_wraith;
    newscript->RegisterSelf();
}

