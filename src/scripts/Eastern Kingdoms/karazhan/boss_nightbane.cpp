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
SDName: Boss_Nightbane
SD%Complete: 80
SDComment: SDComment: Timers may incorrect
SDCategory: Karazhan
EndScriptData */

#include "scriptPCH.h"
#include "def_karazhan.h"

#define EMOTE_SUMMON                "An ancient being awakens in the distance..."
#define YELL_AGGRO                  "What fools! I shall bring a quick end to your suffering!"
#define YELL_FLY_PHASE              "Miserable vermin. I shall exterminate you from the air!"
#define YELL_LAND_PHASE_1           "Enough! I shall land and crush you myself!"
#define YELL_LAND_PHASE_2           "Insects! Let me show you my strength up close!"
#define EMOTE_BREATH                "takes a deep breath."

static float IntroWay[8][3] =
{
    { -11053.37f, -1794.48f, 149.0f },
    { -11141.07f, -1841.40f, 125.0f },
    { -11187.28f, -1890.23f, 125.0f },
    { -11189.20f, -1931.25f, 125.0f },
    { -11153.76f, -1948.93f, 125.0f },
    { -11128.73f, -1929.75f, 125.0f },
    { -11140.0f,  -1915.0f,  122.0f },
    { -11163.0f,  -1903.0f,  91.473f }
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

    enum Spells
    {
        //phase 1
        SPELL_BELLOWING_ROAR        = 39427,
        SPELL_CHARRED_EARTH         = 30129,
        SPELL_DISTRACTING_ASH       = 30130,
        SPELL_SMOLDERING_BREATH     = 30210,
        SPELL_TAIL_SWEEP            = 25653,
        SPELL_CLEAVE                = 30131,

        //phase 2
        SPELL_RAIN_OF_BONES         = 37098,
        SPELL_RAIN_OF_BONES_EFFECT  = 37091,
        SPELL_SMOKING_BLAST         = 37057,
        SPELL_IMMOLATION            = 37059,
        SPELL_FIREBALL_BARRAGE      = 30282,
        SPELL_SEARING_CINDERS       = 30127,
        SPELL_SUMMON_SKELETON       = 30170,

        NPC_SKELETON                = 17261
    };

    ScriptedInstance* pInstance;

    enum {
        PHASE_GROUND,
        PHASE_FLIGHT,
        PHASE_LANDING,
    } Phase;

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
    Timer SummonSkeleton_Timer;

    uint8 SkeletonCount;
    uint32 FlyCount;
    Timer FlyCheckTimer;

    int32 MovePhase;
    Timer WaitTimer;

    // all this thing should be rewritten to use phases only (add intro phase and should be ccool)
    bool Summoned;
    bool Intro;
    bool Flying;
    bool Movement;

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

        // ground phase timers
        BellowingRoarTimer.Reset(urand(20000, 30000));
        CharredEarthTimer.Reset(urand(10000, 15000));
        SmolderingBreathTimer.Reset(urand(9000, 13000));
        TailSweepTimer.Reset(urand(12000, 15000));
        Cleave_Timer.Reset(urand(4000, 8000));

        // flight phase timers
        DistractingAshTimer.Reset(urand(10000, 12000));
        RainofBonesTimer = 0;
        SmokingBlastTimer = 0;
        FireballBarrageTimer.Reset(10000);
        SearingCindersTimer.Reset(14000);
        WaitTimer.Reset(1000);
        SummonSkeleton_Timer = 0;
        SkeletonCount = 0;

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
            if (!m_creature->GetVictim() && m_creature->canStartAttack(who))
                ScriptedAI::AttackStart(who);
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type != POINT_MOTION_TYPE)
        {
            if (Flying)
                DoStartNoMovement(me->GetVictim()); // TODO: remove after fixing weird stuff with movement and targets while casting spellz
            return;
        }

        if (Flying && id == 0)
            DoTextEmote(EMOTE_BREATH, NULL, true);

        if (Phase == PHASE_LANDING)
            WaitTimer = 1;

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

           
    }

    void JustSummoned(Creature *summoned)
    {
        if (summoned->GetPositionZ() < 85.0f)
        {
            if (Unit* victim = me->GetVictim())
                summoned->Relocate(victim->GetPositionX(), victim->GetPositionY(), victim->GetPositionZ());
            else
            {
                summoned->DisappearAndDie();
                return;
            }
        }

        summoned->AI()->AttackStart(m_creature->GetVictim());
        summoned->AddAura(SPELL_IMMOLATION,summoned);
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

        Phase = PHASE_FLIGHT;
        Flying = true;
        ++FlyCount;

        FlyCheckTimer.Reset(20000);
        DistractingAshTimer = 0;
        RainofBonesTimer = 5000;
        SmokingBlastTimer = 0;
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

            if (Phase == PHASE_LANDING)
            {
                if (MovePhase >= 7)
                {
                    m_creature->SetLevitate(false);
                    m_creature->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
                    m_creature->GetMotionMaster()->MovePoint(8, IntroWay[7][0], IntroWay[7][1], IntroWay[7][2]);
                    DoZoneInCombat(1000.0f);
                    UpdateVictim();
                    Flying = false;
                    Phase = PHASE_GROUND;
                }
                else
                {
                    m_creature->GetMotionMaster()->MovePoint(MovePhase, IntroWay[MovePhase][0], IntroWay[MovePhase][1], IntroWay[MovePhase][2]);
                    ++MovePhase;
                }
            }

            WaitTimer = 0;
        }

        if (!Flying || !me->IsInCombat())
            if (!UpdateVictim())
                return;

        DoSpecialThings(diff, DO_PULSE_COMBAT);

        if (Phase == PHASE_GROUND)
        {
            if (Movement)
            {
                DoStartMovement(m_creature->GetVictim());
                Movement = false;
            }

            if (BellowingRoarTimer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(), SPELL_BELLOWING_ROAR);
                BellowingRoarTimer = urand(20000, 30000);
            }

            if (SmolderingBreathTimer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(), SPELL_SMOLDERING_BREATH);
                SmolderingBreathTimer = urand(14000, 20000);
            }

            if (Cleave_Timer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(), SPELL_CLEAVE);
                Cleave_Timer = urand(6000, 12000);
            }

            if (CharredEarthTimer.Expired(diff))
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_CHARRED_EARTH), true))
                    DoCast(target, SPELL_CHARRED_EARTH);
                CharredEarthTimer = urand(25000, 35000);
            }

            if (TailSweepTimer.Expired(diff))
            {
                DoCast(NULL, SPELL_TAIL_SWEEP);
                TailSweepTimer = urand(14000, 20000);
            }

            if (HealthBelowPct((3 - FlyCount) * 25))
                TakeOff();

            DoMeleeAttackIfReady();
        }

        else if (Phase == PHASE_FLIGHT)
        {
            if (RainofBonesTimer.Expired(diff)) // only once at the beginning of phase 2
            {

                if (Unit* target = SelectCastTarget(SPELL_RAIN_OF_BONES, CAST_RANDOM))
                {
                    m_creature->CastSpell(target, SPELL_RAIN_OF_BONES, false);
                }

                SummonSkeleton_Timer.Reset(urand(2000,3000));
                SkeletonCount = 0;

                RainofBonesTimer = 0;
                SmokingBlastTimer = urand(10000, 12000);
                DistractingAshTimer.Reset(urand(10000, 12000));
            }

            if (SummonSkeleton_Timer.Expired(diff))
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                    DoCast(target, SPELL_SUMMON_SKELETON, true);
                SkeletonCount++;
                SummonSkeleton_Timer = ((SkeletonCount == 5) ? 0 : urand(2000, 3000));
            }

            if (DistractingAshTimer.Expired(diff))
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_DISTRACTING_ASH), true))
                    m_creature->AddAura(SPELL_DISTRACTING_ASH, target);

                DistractingAshTimer = urand(7000, 13000);
            }

            if (SmokingBlastTimer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(), SPELL_SMOKING_BLAST);
                SmokingBlastTimer = urand(1000, 3000);
            }

            if (SearingCindersTimer.Expired(diff))
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_SEARING_CINDERS), true))
                    DoCast(target, SPELL_SEARING_CINDERS);
                SearingCindersTimer = urand(10000, 13000);
            }

            if (FireballBarrageTimer.Expired(diff))
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_FARTHEST, 0, 1000.0f, true))
                    if (!target->IsWithinLOSInMap(me))
                        DoCast(m_creature->GetVictim(), SPELL_FIREBALL_BARRAGE);
                FireballBarrageTimer = urand(3000, 6000);
            }

            if (FlyCheckTimer.Expired(diff)) //landing
            {
                if (Creature* skeleton = m_creature->GetMap()->GetCreatureById(NPC_SKELETON, GET_ALIVE_CREATURE_GUID))
                {
                    FlyCheckTimer = 1000;
                    if (skeleton->IsAlive() && !skeleton->IsInCombat())
                        skeleton->ForcedDespawn();
                }
                else
                {
                    DoYell((urand(0,1) ? YELL_LAND_PHASE_1 : YELL_LAND_PHASE_2), LANG_UNIVERSAL, NULL);
                    // all skeletons dead, fly around (we re in point 3) and land
                    m_creature->GetMotionMaster()->MovePoint(3, IntroWay[3][0], IntroWay[3][1], IntroWay[3][2]);
                    MovePhase = 4; 
                    FlyCheckTimer = 0;
                    Phase = PHASE_LANDING;
                }
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

