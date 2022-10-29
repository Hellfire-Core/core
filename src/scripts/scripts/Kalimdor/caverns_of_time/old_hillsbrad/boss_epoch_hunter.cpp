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
SDName: Boss_Epoch_Hunter
SD%Complete: 95
SDComment:
SDCategory: Caverns of Time, Old Hillsbrad Foothills
EndScriptData */

#include "precompiled.h"
#include "def_old_hillsbrad.h"

#define SAY_AGGRO1                    -1560016
#define SAY_AGGRO2                    -1560017
#define SAY_SLAY1                     -1560018
#define SAY_SLAY2                     -1560019
#define SAY_BREATH1                   -1560020
#define SAY_BREATH2                   -1560021
#define SAY_DEATH                     -1560022
#define SAY_INFINITE_AGGRO_1          -1560069
#define SAY_INFINITE_AGGRO_2          -1560070
#define SAY_INFINITE_AGGRO_3          -1560071

#define NPC_TARREN_MILL_GUARDSMAN      18092
#define NPC_TARREN_MILL_PROTECTOR      18093
#define NPC_TARREN_MILL_LOOKOUT        18094
#define NPC_INFINITE_SLAYER            18170
#define NPC_INFINITE_DEFILER           18171
#define NPC_INFINITE_SABOTEUR          18172
#define NPC_THRALL                     17876

#define SPELL_SAND_BREATH              31914
#define SPELL_IMPENDING_DEATH          31916
#define SPELL_MAGIC_DISRUPTION_AURA    33834
#define SPELL_WING_BUFFET              HeroicMode ? 31475:38593
#define SPELL_TRANSFORM                33133 // transform infinite defilers

struct boss_epoch_hunterAI : public ScriptedAI
{
    boss_epoch_hunterAI(Creature *creature) : ScriptedAI(creature)
    {
        pInstance = (creature->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    bool Intro;
    bool Next;

    std::list<uint64> attackers;

    uint8 Wave;
    Timer IntroTimer;
    Timer NextTimer;
    Timer SandBreath_Timer;
    Timer ImpendingDeath_Timer;
    Timer WingBuffet_Timer;
    Timer Mda_Timer;
    uint64 ThrallGUID;

    void Reset()
    {
        Intro = true;
        Next = true;
        Wave = 0;
        IntroTimer.Reset(45000);
        NextTimer.Reset(51000);
        SandBreath_Timer.Reset(25000);
        ImpendingDeath_Timer.Reset(30000);
        WingBuffet_Timer.Reset(35000);
        Mda_Timer.Reset(40000);
        attackers.clear();
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);

        Map* tmpMap = me->GetMap();

        if (!tmpMap)
            return;

        if (Creature* Thrall = tmpMap->GetCreature(pInstance->GetData64(DATA_THRALL)))
            ThrallGUID = Thrall->GetGUID();
    }

    void EnterCombat(Unit *who)
    {
        pInstance->SetData(DATA_EPOCH_DEATH, IN_PROGRESS);
        DoScriptText(RAND(SAY_AGGRO1, SAY_AGGRO2), me);
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2), me);
    }

    void EnterEvadeMode()
    {
        me->InterruptNonMeleeSpells(true);
        me->RemoveAllAuras();
        me->DeleteThreatList();
        me->CombatStop(true);
        me->ForcedDespawn();
        pInstance->SetData(DATA_EPOCH_DEATH, NOT_STARTED);
    }

    void JustDied(Unit *victim)
    {
        DoScriptText(SAY_DEATH, me);

        if (pInstance->GetData(TYPE_THRALL_EVENT) == IN_PROGRESS)
        {
            if (Creature* Thrall = me->GetMap()->GetCreature(ThrallGUID))
                Thrall->AI()->DoAction();

            pInstance->SetData(TYPE_THRALL_PART4, DONE);
        }

        if (pInstance->GetData(DATA_EPOCH_DEATH) == DONE)
            me->SetLootRecipient(NULL);
        else
        {
            pInstance->SetData(DATA_EPOCH_DEATH, DONE);
            pInstance->SetData(TYPE_THRALL_PART4, DONE);
        }

        attackers.clear();
    }

    void JustSummoned(Creature* summoned)
    {
        attackers.push_back(summoned->GetGUID());
        summoned->SetReactState(REACT_PASSIVE);
        summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
    }

    void UpdateEntry()
    {
        Map* tmpMap = me->GetMap();

        if (!tmpMap)
            return;

        if (!attackers.empty())
        {
            for (std::list<uint64>::iterator itr = attackers.begin(); itr != attackers.end(); ++itr)
            {
                if (Creature* attacker = tmpMap->GetCreature((*itr)))
                {
                    attacker->CastSpell(attacker, SPELL_TRANSFORM, true);

                    switch (attacker->GetEntry())
                    {
                        case NPC_TARREN_MILL_GUARDSMAN:
                            attacker->UpdateEntry(NPC_INFINITE_SLAYER);
                            break;
                        case NPC_TARREN_MILL_PROTECTOR:
                            attacker->UpdateEntry(NPC_INFINITE_DEFILER);

                            switch (Wave)
                            {
                                case 2:
                                    DoScriptText(SAY_INFINITE_AGGRO_3, attacker);
                                    break;
                                case 4:
                                    DoScriptText(SAY_INFINITE_AGGRO_1, attacker);
                                    break;
                                case 6:
                                    DoScriptText(SAY_INFINITE_AGGRO_2, attacker);
                                break;
                            }
                            break;
                        case NPC_TARREN_MILL_LOOKOUT:
                            attacker->UpdateEntry(NPC_INFINITE_SABOTEUR);
                            break;
                    }
                            
                    attacker->SetReactState(REACT_AGGRESSIVE);
                    attacker->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);

                    if (Creature* Thrall = me->GetMap()->GetCreature(ThrallGUID))
                        attacker->AI()->AttackStart(Thrall);
                }
            }
        }
    }

    void NextWave()
    {
        ++Wave;

        switch (Wave)
        {
            case 1:
                for (int i = 0; i < 3; i++)
                {
                    switch (i)
                    {
                        case 0:
                            me->SummonCreature(NPC_TARREN_MILL_GUARDSMAN, 2639.87f, 696.65f, 55.803f, 4.51f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                            break;
                        case 1:
                            me->SummonCreature(NPC_TARREN_MILL_LOOKOUT, 2635.31f, 697.35f, 56.024f, 4.51f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                            break;
                        case 2:
                            me->SummonCreature(NPC_TARREN_MILL_PROTECTOR, 2644.64f, 697.09f, 55.860f, 4.51f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                            break;
                    }
                }
                break;
            case 2:
                UpdateEntry();
                Next = false;
                break;
            case 3:
                for (int i = 0; i < 3; i++)
                {
                    switch (i)
                    {
                        case 0:
                            me->SummonCreature(NPC_TARREN_MILL_GUARDSMAN, 2594.84f, 683.77f, 55.826f, 5.92f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                            break;
                        case 1:
                            me->SummonCreature(NPC_TARREN_MILL_LOOKOUT, 2599.79f, 686.86f, 55.774f, 5.92f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                            break;
                        case 2:
                            me->SummonCreature(NPC_TARREN_MILL_PROTECTOR, 2596.47f, 679.29f, 56.177f, 5.92f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                            break;
                    }
                }
                Next = true;
                break;
             case 4:
                UpdateEntry();
                Next = false;
                break;
             case 5:
                for (int i = 0; i < 4; i++)
                {
                    switch (i)
                    {
                        case 0:
                            me->SummonCreature(NPC_TARREN_MILL_GUARDSMAN, 2639.87f, 696.65f, 55.803f, 4.51f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                            break;
                        case 1:
                            me->SummonCreature(NPC_TARREN_MILL_LOOKOUT, 2635.31f, 697.35f, 56.024f, 4.51f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                            break;
                        case 2:
                            me->SummonCreature(NPC_TARREN_MILL_PROTECTOR, 2644.64f, 697.09f, 55.860f, 4.51f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                            break;
                        case 3:
                            me->SummonCreature(NPC_TARREN_MILL_GUARDSMAN, 2640.52f, 700.75f, 55.962f, 4.51f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                            break;
                    }
                }
                Next = true;
                break;
            case 6:
                UpdateEntry();
                Next = false;
                break;
            case 7:
                me->SetReactState(REACT_AGGRESSIVE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
                me->GetMotionMaster()->MovePoint(0, 2638.60f, 686.948f, 58.245f);
                if (Creature* Thrall = me->GetMap()->GetCreature(ThrallGUID))
                    Thrall->AI()->AttackStart(me);
                me->SetLevitate(false);
                Intro = false;
                break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (Intro)
        {
            if (IntroTimer.Expired(diff))
            {
                if (attackers.empty())
                    NextWave();

                Map* tmpMap = me->GetMap();

                if (!tmpMap)
                    return;

                if (!attackers.empty())
                {
                    bool alive = false;
                    for (std::list<uint64>::iterator itr = attackers.begin(); itr != attackers.end(); ++itr)
                    {
                        if (Creature* attacker = tmpMap->GetCreature((*itr)))
                        {
                            if (attacker->IsAlive())
                            {
                                alive = true;
                                break;
                            }
                        }
                    }
                    
                    if (!alive)
                    {
                        NextWave();
                        NextTimer = 5000;
                    }
                }

                if (Creature* Thrall = tmpMap->GetCreature(ThrallGUID))
                {
                    if (!Thrall->IsAlive())
                        me->ForcedDespawn();
                }

                IntroTimer = 5000;
            }
            

            if (Next)
            {
                if (NextTimer.Expired(diff))
                {
                    NextWave();
                }
            }
        }

        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (SandBreath_Timer.Expired(diff))
        {
            if (me->IsNonMeleeSpellCast(false))
                me->InterruptNonMeleeSpells(false);

            DoCast(me->GetVictim(),SPELL_SAND_BREATH);

            DoScriptText(RAND(SAY_BREATH1, SAY_BREATH2), me);

            SandBreath_Timer = 25000+rand()%5000;
        }
        
        if (ImpendingDeath_Timer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0 , GetSpellMaxRange(SPELL_IMPENDING_DEATH), true))
                DoCast(target,SPELL_IMPENDING_DEATH);
            ImpendingDeath_Timer = 30000+rand()%5000;
        }

        if (WingBuffet_Timer.Expired(diff))
        {
            DoCast(me,SPELL_WING_BUFFET);
            WingBuffet_Timer = 25000+rand()%10000;
        }
        


        if (Mda_Timer.Expired(diff))
        {
            DoCast(me,SPELL_MAGIC_DISRUPTION_AURA);
            Mda_Timer = 15000;
        }
        

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_epoch_hunter(Creature *creature)
{
    return new boss_epoch_hunterAI (creature);
}

void AddSC_boss_epoch_hunter()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_epoch_hunter";
    newscript->GetAI = &GetAI_boss_epoch_hunter;
    newscript->RegisterSelf();
}

