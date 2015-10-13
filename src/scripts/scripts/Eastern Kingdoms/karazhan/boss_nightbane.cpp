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
SDName: Boss_Nightbane
SD%Complete: 80
SDComment: SDComment: Timers may incorrect
SDCategory: Karazhan
EndScriptData */

#include "precompiled.h"
#include "def_karazhan.h"

//phase 1
#define SPELL_BELLOWING_ROAR        39427
#define SPELL_CHARRED_EARTH         30129
#define SPELL_DISTRACTING_ASH       30130
#define SPELL_SMOLDERING_BREATH     30210
#define SPELL_TAIL_SWEEP            25653
#define SPELL_CLEAVE                30131
//phase 2
#define SPELL_RAIN_OF_BONES         37098
#define SPELL_RAIN_OF_BONES_EFFECT  37091
#define SPELL_SMOKING_BLAST         37057
#define SPELL_FIREBALL_BARRAGE      30282
#define SPELL_SEARING_CINDERS       30127
#define SPELL_SUMMON_SKELETON       30170

#define EMOTE_SUMMON                "An ancient being awakens in the distance..."
#define YELL_AGGRO                  "What fools! I shall bring a quick end to your suffering!"
#define YELL_FLY_PHASE              "Miserable vermin. I shall exterminate you from the air!"
#define YELL_LAND_PHASE_1           "Enough! I shall land and crush you myself!"
#define YELL_LAND_PHASE_2           "Insects! Let me show you my strength up close!"
#define EMOTE_BREATH                "takes a deep breath."

float IntroWay[8][3] =
{
    { -11053.37, -1794.48, 149 },
    { -11141.07, -1841.40, 125 },
    { -11187.28, -1890.23, 125 },
    { -11189.20, -1931.25, 125 },
    { -11153.76, -1948.93, 125 },
    { -11128.73, -1929.75, 125 },
    { -11140,    -1915,    122 },
    { -11163,    -1903,    91.473 }
};

struct boss_nightbaneAI : public ScriptedAI
{
    boss_nightbaneAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        c->setActive(true);
        Intro = true;
        Summoned = true;
    }

    ScriptedInstance* pInstance;

    enum {
        PHASE_GROUND,
        PHASE_FLIGHT
    } Phase;

    bool Summoned;
    bool RainBones;
    bool Skeletons;

    Timer BellowingRoarTimer;
    Timer CharredEarthTimer;
    Timer DistractingAshTimer;
    Timer SmolderingBreathTimer;
    Timer TailSweepTimer;
    Timer RainofBonesTimer;
    Timer SmokingBlastTimer;
    Timer FireballBarrageTimer;
    Timer SearingCindersTimer;
    Timer Cleave_Timer;

    uint32 FlyCount;
    Timer FlyTimer;

    bool Intro;
    bool Flying;
    bool Movement;

    Timer WaitTimer;
    int32 MovePhase;

    void Reset()
    {
        if (Summoned)
        {
            if (pInstance->GetData64(DATA_NIGHTBANE) || pInstance->GetData(DATA_NIGHTBANE_EVENT) == DONE)
            {
                m_creature->setDeathState(JUST_DIED);
                m_creature->RemoveCorpse();
            }
            else
                pInstance->SetData64(DATA_NIGHTBANE, m_creature->GetGUID());
        }

        BellowingRoarTimer.Reset(30000);
        CharredEarthTimer.Reset(15000);
        DistractingAshTimer.Reset(20000);
        SmolderingBreathTimer.Reset(10000);
        TailSweepTimer.Reset(12000);
        RainofBonesTimer.Reset(5000);
        SmokingBlastTimer.Reset(20000);
        FireballBarrageTimer.Reset(13000);
        SearingCindersTimer.Reset(14000);
        WaitTimer.Reset(1000);
        Cleave_Timer.Reset(6000);

        Phase = PHASE_GROUND;
        FlyCount = 0;
        MovePhase = 0;

        me->SetReactState(REACT_AGGRESSIVE);

        m_creature->SetSpeed(MOVE_RUN, 2.0f);
        m_creature->SetLevitate(true);
        m_creature->SetWalk(false);

        if (pInstance && pInstance->GetData(DATA_NIGHTBANE_EVENT) != DONE)
            pInstance->SetData(DATA_NIGHTBANE_EVENT, NOT_STARTED);

        HandleTerraceDoors(true);

        Flying = false;
        Movement = false;

        me->addUnitState(UNIT_STAT_IGNORE_PATHFINDING);

        if (!Intro)
        {
            m_creature->SetHomePosition(IntroWay[7][0], IntroWay[7][1], IntroWay[7][2], 0);
            m_creature->GetMotionMaster()->MoveTargetedHome();
        }

        Summoned = false;
    }

    void HandleTerraceDoors(bool open)
    {
        if (GameObject *Door = GameObject::GetGameObject((*m_creature), pInstance->GetData64(DATA_MASTERS_TERRACE_DOOR_1)))
        {
            Door->SetUInt32Value(GAMEOBJECT_STATE, open ? 0 : 1);
            Door->SetFlag(GO_FLAG_NOTSELECTABLE, 1);
        }
        if (GameObject *Door = GameObject::GetGameObject((*m_creature), pInstance->GetData64(DATA_MASTERS_TERRACE_DOOR_2)))
        {
            Door->SetUInt32Value(GAMEOBJECT_STATE, open ? 0 : 1);
            Door->SetFlag(GO_FLAG_NOTSELECTABLE, 1);
        }
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(DATA_NIGHTBANE_EVENT, IN_PROGRESS);

        HandleTerraceDoors(false);
        DoYell(YELL_AGGRO, LANG_UNIVERSAL, NULL);
    }

    void DamageTaken(Unit*, uint32 &damage)
    {
        if (Phase == PHASE_FLIGHT && (damage >= me->GetHealth() || me->GetHealth() <= 1))
            damage = 0;
    }

    void AttackStart(Unit* who)
    {
        if (!Intro && !Flying)
            ScriptedAI::AttackStart(who);
    }

    void JustDied(Unit* killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_NIGHTBANE_EVENT, DONE);

        HandleTerraceDoors(true);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!Intro && !Flying)
            if (!m_creature->getVictim() && m_creature->canStartAttack(who))
                ScriptedAI::AttackStart(who);
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type != POINT_MOTION_TYPE)
        {
            if (Flying)
                DoStartNoMovement(me->getVictim()); // TODO: remove after fixing weird stuff with movement and targets while casting spellz
            return;
        }

        if (Intro)
        {
            if (id >= 8)
            {
                Intro = false;
                m_creature->SetHomePosition(IntroWay[7][0], IntroWay[7][1], IntroWay[7][2], 0);
                return;
            }

            WaitTimer = 1;
        }

        if (Flying)
        {
            if (id == 0)
            {
                DoTextEmote(EMOTE_BREATH, NULL, true);
                Flying = true;
                Phase = PHASE_FLIGHT;
                return;
            }

            if (id == 3)
            {
                MovePhase = 4;
                WaitTimer = 1;
                return;
            }

            if (id == 8)
            {
                Flying = false;
                Phase = PHASE_GROUND;
                Movement = true;
                return;
            }

            WaitTimer = 1;
        }
    }

    void JustSummoned(Creature *summoned)
    {
        if (summoned->GetPositionZ() < 85.0f)
        {
            if (Unit* victim = me->getVictim())
                summoned->Relocate(victim->GetPositionX(), victim->GetPositionY(), victim->GetPositionZ());
            else
            {
                summoned->DisappearAndDie();
                return;
            }
        }

        summoned->AI()->AttackStart(m_creature->getVictim());
    }
    
    void JustRespawned()
    {
        DoZoneInCombat(1000.0f);
    }

    void TakeOff()
    {
        DoYell(YELL_FLY_PHASE, LANG_UNIVERSAL, NULL);

        DoResetThreat();
        m_creature->InterruptSpell(CURRENT_GENERIC_SPELL);
        m_creature->HandleEmoteCommand(EMOTE_ONESHOT_LIFTOFF);
        m_creature->SetLevitate(true);
        m_creature->GetMotionMaster()->Clear(false);
        m_creature->GetMotionMaster()->MovePoint(0, IntroWay[2][0], IntroWay[2][1], IntroWay[2][2]);
        DistractingAshTimer.Delay(10000); // wrong, just to not cast while flying up

        Flying = true;

        FlyTimer.Reset(45000 + rand() % 15000); //timer wrong between 45 and 60 seconds
        ++FlyCount;

        RainofBonesTimer.Reset(5000); //timer wrong (maybe)
        RainBones = false;
        Skeletons = false;
    }

    void UpdateAI(const uint32 diff)
    {
        if (WaitTimer.Expired(diff))
        {
            if (Intro)
            {
                if (MovePhase >= 7)
                {
                    m_creature->SetLevitate(false);
                    m_creature->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
                    m_creature->GetMotionMaster()->MovePoint(8, IntroWay[7][0], IntroWay[7][1], IntroWay[7][2]);
                }
                else
                {
                    m_creature->GetMotionMaster()->MovePoint(MovePhase, IntroWay[MovePhase][0], IntroWay[MovePhase][1], IntroWay[MovePhase][2]);
                    ++MovePhase;
                }
            }

            if (Flying)
            {
                if (MovePhase >= 7)
                {
                    m_creature->SetLevitate(false);
                    m_creature->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
                    m_creature->GetMotionMaster()->MovePoint(8, IntroWay[7][0], IntroWay[7][1], IntroWay[7][2]);
                    DoZoneInCombat(1000.0f);
                    UpdateVictim();
                }
                else
                {
                    m_creature->GetMotionMaster()->MovePoint(MovePhase, IntroWay[MovePhase][0], IntroWay[MovePhase][1], IntroWay[MovePhase][2]);
                    ++MovePhase;
                }
            }

            WaitTimer = 0;
        }

        if (!Flying || !me->isInCombat())
            if (!UpdateVictim())
                return;

        DoSpecialThings(diff, DO_PULSE_COMBAT);

        if (Phase == PHASE_GROUND)
        {
            if (Movement)
            {
                DoStartMovement(m_creature->getVictim());
                Movement = false;
            }

            if (BellowingRoarTimer.Expired(diff))
            {
                DoCast(m_creature->getVictim(), SPELL_BELLOWING_ROAR);
                BellowingRoarTimer = 30000 + rand() % 10000; //Timer
            }

            if (SmolderingBreathTimer.Expired(diff))
            {
                DoCast(m_creature->getVictim(), SPELL_SMOLDERING_BREATH);
                SmolderingBreathTimer = 20000;//timer
            }

            if (Cleave_Timer.Expired(diff))
            {
                DoCast(m_creature->getVictim(), SPELL_CLEAVE);
                Cleave_Timer = 6000 + rand() % 6000;
            }

            if (CharredEarthTimer.Expired(diff))
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_CHARRED_EARTH), true))
                    DoCast(target, SPELL_CHARRED_EARTH);
                CharredEarthTimer = 20000; //timer
            }

            if (TailSweepTimer.Expired(diff))
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_TAIL_SWEEP), true))
                    if (!m_creature->HasInArc(M_PI, target))
                        DoCast(target, SPELL_TAIL_SWEEP);
                TailSweepTimer = 15000;//timer
            }

            if (SearingCindersTimer.Expired(diff))
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_SEARING_CINDERS), true))
                    DoCast(target, SPELL_SEARING_CINDERS);
                SearingCindersTimer = 10000; //timer
            }

            if (DistractingAshTimer.Expired(diff))
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_DISTRACTING_ASH), true))
                    m_creature->AddAura(SPELL_DISTRACTING_ASH, target);

                DistractingAshTimer = 2000;//timer wrong
            }

            if (HealthBelowPct((3 - FlyCount) * 25))
                TakeOff();

            DoMeleeAttackIfReady();
        }

        else if (Phase == PHASE_FLIGHT)
        {
            if (!RainBones)
            {
                if (RainofBonesTimer.Expired(diff)) // only once at the beginning of phase 2
                {
                    if (Unit* target = SelectCastTarget(SPELL_RAIN_OF_BONES, CAST_RANDOM))
                    {
                        m_creature->CastSpell(target, SPELL_RAIN_OF_BONES, false);

                        for (int i = 0; i < 5; ++i)
                            DoCast(target, SPELL_SUMMON_SKELETON, true);
                    }

                    RainBones = true;
                    RainofBonesTimer = 0;
                    SmokingBlastTimer.Reset(15000);
                }
            }
            else
            {
                if (SmokingBlastTimer.Expired(diff))
                {
                    DoCast(m_creature->getVictim(), SPELL_SMOKING_BLAST);
                    SmokingBlastTimer = 1000;
                }
            }

            if (FireballBarrageTimer.Expired(diff))
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_FARTHEST, 0, GetSpellMaxRange(SPELL_FIREBALL_BARRAGE), true, uint64(0), 40.0f))
                    DoCast(target, SPELL_FIREBALL_BARRAGE);
                FireballBarrageTimer = 5000; //Timer
            }

            if (FlyTimer.Expired(diff)) //landing
            {
                if (rand() % 2)
                    DoYell(YELL_LAND_PHASE_1, LANG_UNIVERSAL, NULL);
                else
                    DoYell(YELL_LAND_PHASE_2, LANG_UNIVERSAL, NULL);

                DoResetThreat();
                m_creature->GetMotionMaster()->Clear(false);
                m_creature->GetMotionMaster()->MovePoint(3, IntroWay[3][0], IntroWay[3][1], IntroWay[3][2]);

                FlyTimer = 0;
            }
        }
    }
};

CreatureAI* GetAI_boss_nightbane(Creature *_Creature)
{
    return new boss_nightbaneAI(_Creature);
}

void AddSC_boss_nightbane()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_nightbane";
    newscript->GetAI = &GetAI_boss_nightbane;
    newscript->RegisterSelf();
}

