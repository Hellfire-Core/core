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
SDName: Boss_Brutallus
SD%Complete: 98
SDComment: Check Felmyst spawning event, final debugging
EndScriptData */

#include "precompiled.h"
#include "def_sunwell_plateau.h"

enum Quotes
{
    YELL_INTRO                 =   -1580017,
    YELL_INTRO_BREAK_ICE       =   -1580018,
    YELL_INTRO_CHARGE          =   -1580019,
    YELL_INTRO_KILL_MADRIGOSA  =   -1580020,
    YELL_INTRO_TAUNT           =   -1580021,

    YELL_MADR_ICE_BARRIER      =   -1580031,
    YELL_MADR_INTRO            =   -1580032,
    YELL_MADR_ICE_BLOCK        =   -1580033,
    YELL_MADR_TRAP             =   -1580034,
    YELL_MADR_DEATH            =   -1580035,

    YELL_AGGRO                 =   -1580022,
    YELL_KILL1                 =   -1580023,
    YELL_KILL2                 =   -1580024,
    YELL_KILL3                 =   -1580025,
    YELL_LOVE1                 =   -1580026,
    YELL_LOVE2                 =   -1580027,
    YELL_LOVE3                 =   -1580028,
    YELL_BERSERK               =   -1580029,
    YELL_DEATH                 =   -1580030
};

enum Spells
{
    // Brutallus fight spells
    SPELL_METEOR_SLASH                 =   45150,
    SPELL_BURN                         =   45141,
    SPELL_STOMP                        =   45185,
    SPELL_BERSERK                      =   26662,
    SPELL_DUAL_WIELD                   =   42459,
    SPELL_SUMMON_DEATH_CLOUD           =   45884,
    SPELL_DEATH_CLOUD                  =   45212,

    // Brutallus intro spells
    SPELL_INTRO_CHARGE                 =   44884,
    SPELL_INTRO_FROST_BREATH           =   45065,
    SPELL_INTRO_FROST_BLAST            =   45203,
    SPELL_INTRO_FROSTBOLT              =   44843,
    SPELL_INTRO_ENCAPSULATE            =   44883,
    SPELL_INTRO_BREAK_ICE              =   46637,
    SPELL_INTRO_BREAK_ICE_KNOCKBACK    =   47030,

    // Felmyst intro spells
    SPELL_FELMYST_PRE_VISUAL           =   44885,
    SPELL_TRANSFORM_FELMYST            =   45068,
    SPELL_FELMYST_SUMMON               =   45069
};

#define MADRI_FLY_X        1476.3
#define MADRI_FLY_Y           649               
#define MADRI_FLY_Z          21.5                  
                                                   
#define MOB_DEATH_CLOUD 25703
#define FELMYST 25038

struct boss_brutallusAI : public ScriptedAI
{
    boss_brutallusAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    Timer SlashTimer;
    Timer BurnTimer;
    Timer StompTimer;
    Timer BerserkTimer;
    Timer CheckTimer;
    Timer CheckGroundTimer;

    uint32 IntroPhase;
    Timer IntroPhaseTimer;
    Timer IntroFrostBoltTimer;
    bool Enraged;

    void Reset()
    {
        SlashTimer.Reset(11000);
        StompTimer.Reset(30000);
        BurnTimer.Reset(20000);
        BerserkTimer.Reset(360000);
        CheckTimer.Reset(1000);
        CheckGroundTimer.Reset(500);

        IntroPhase = 0;
        IntroPhaseTimer.Reset(1);
        IntroFrostBoltTimer=0;
        Enraged = false;

        ForceSpellCast(me, SPELL_DUAL_WIELD, INTERRUPT_AND_CAST_INSTANTLY);
        pInstance->SetData(DATA_BRUTALLUS_EVENT, NOT_STARTED);
        me->CombatStop();
    }

    void EnterCombat(Unit* /*pWho*/)
    {
        if (pInstance->GetData(DATA_KALECGOS_EVENT) == IN_PROGRESS)
        {
            EnterEvadeMode();
            return;
        }

        if (pInstance->GetData(DATA_BRUTALLUS_INTRO_EVENT) == DONE)
        {
            DoScriptText(YELL_AGGRO, me);
            BurnTimer = 20000;  // just in case?
            pInstance->SetData(DATA_BRUTALLUS_EVENT, IN_PROGRESS);
        }
    }

    void KilledUnit(Unit* /*victim*/)
    {
        if (pInstance->GetData(DATA_BRUTALLUS_INTRO_EVENT) == DONE && me->IsAlive())
        {
            if (roll_chance_f(40.0))
                DoScriptText(RAND(YELL_KILL1, YELL_KILL2, YELL_KILL3), me);
        }
    }

    void JustDied(Unit* Killer)
    {
        ForceSpellCastWithScriptText(me, SPELL_SUMMON_DEATH_CLOUD, YELL_DEATH, INTERRUPT_AND_CAST_INSTANTLY);
        pInstance->SetData(DATA_BRUTALLUS_EVENT, DONE);
    }

    void DamageMade(Unit* target, uint32 &damage, bool /*direct_damage*/, uint8 school_mask)
    {
        if (target->GetTypeId() == TYPEID_UNIT && target->GetEntry() == 24895)
            damage *= 40;
    }

    void DoIntro()
    {
        Creature* pMadrigosa = me->GetCreature(pInstance->GetData64(DATA_MADRIGOSA));
        if (!pMadrigosa)
            return;

        switch (IntroPhase)
        {
            float x, y, z;
            case 0:
                DoScriptText(YELL_MADR_ICE_BARRIER, pMadrigosa);
                pMadrigosa->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
                pMadrigosa->SetReactState(REACT_PASSIVE);
                for (uint8 i = 0; i < 8; ++i)
                    pMadrigosa->SetSpeed(UnitMoveType(i), 2.5);

                pMadrigosa->GetMotionMaster()->MovePoint(1, MADRI_FLY_X, MADRI_FLY_Y, MADRI_FLY_Z);
                me->SetInFront(pMadrigosa);
                IntroPhaseTimer = 6500;
                pMadrigosa->SetIgnoreVictimSelection(false);
                ++IntroPhase;
                break;
            case 1:
                pInstance->SetData(DATA_BRUTALLUS_INTRO_EVENT, IN_PROGRESS);
                pMadrigosa->SetLevitate(false);
                pMadrigosa->setHover(false);
                pMadrigosa->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
                IntroPhaseTimer = 2500;
                ++IntroPhase;
                break;
            case 2:
                DoScriptText(YELL_MADR_INTRO, pMadrigosa);
                IntroPhaseTimer = 5000;
                ++IntroPhase;
                break;
            case 3:
                DoScriptText(YELL_INTRO, me);
                IntroPhaseTimer = 6000;
                ++IntroPhase;
                break;
            case 4:
                pMadrigosa->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
                DoStartMovement(pMadrigosa);
                me->Attack(pMadrigosa, true);
                pMadrigosa->Attack(me, true);
                IntroPhaseTimer = 7000;
                ++IntroPhase;
                break;
            case 5:
                pMadrigosa->CastSpell(me, SPELL_INTRO_FROST_BREATH, false, 0, 0, me->GetGUID());
                IntroPhaseTimer = 2500;
                ++IntroPhase;
                break;
            case 6:
                me->AttackStop();
                me->GetMotionMaster()->MoveIdle();

                pMadrigosa->GetMotionMaster()->MoveIdle();
                pMadrigosa->AttackStop();
                pMadrigosa->SetLevitate(true);
                pMadrigosa->setHover(true);

                IntroPhaseTimer = 4000;
                ++IntroPhase;
                break;
            case 7:
                pMadrigosa->CastSpell(me, SPELL_INTRO_FROST_BLAST, false);
                DoScriptText(YELL_MADR_ICE_BLOCK, pMadrigosa);
                IntroFrostBoltTimer = 500;
                IntroPhaseTimer = 11000;
                ++IntroPhase;
                break;
            case 8:
                DoScriptText(YELL_INTRO_BREAK_ICE, me);

                IntroPhaseTimer = 2000;
                ++IntroPhase;
                break;
            case 9:
                pMadrigosa->setHover(false);
                pMadrigosa->SetLevitate(false);
                IntroPhaseTimer = 6500;
                ++IntroPhase;
                break;
            case 10:
                me->GetMotionMaster()->MoveIdle();
                pMadrigosa->CastSpell(me, SPELL_INTRO_ENCAPSULATE, false);
                DoScriptText(YELL_MADR_TRAP, pMadrigosa);
                IntroPhaseTimer = 1000;
                ++IntroPhase;
                break;
            case 11:
                me->GetPosition(x, y, z);
                me->GetMotionMaster()->MovePoint(1, x - 6, y - 15, z + 10);
                me->SetInFront(pMadrigosa);
                IntroPhaseTimer = 8000;
                ++IntroPhase;
                break;
            case 12:
                me->RemoveAurasDueToSpell(44883);
                pMadrigosa->InterruptNonMeleeSpells(false);
                DoScriptText(YELL_INTRO_CHARGE, me);
                me->GetMotionMaster()->MoveFall();
                IntroPhaseTimer = 3500;
                ++IntroPhase;
                break;
            case 13:
                me->CastSpell(pMadrigosa, SPELL_INTRO_CHARGE, false);
                IntroPhaseTimer = 1000;
                ++IntroPhase;
                break;
            case 14:
                me->GetMotionMaster()->MoveIdle();
                me->InterruptNonMeleeSpells(true);
                me->UpdateVisibilityAndView();
                me->SendHeartBeat();
                DoScriptText(YELL_MADR_DEATH, pMadrigosa);
                pMadrigosa->setHover(false);
                pMadrigosa->SetFlying(false);
                pMadrigosa->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                pMadrigosa->SetFlag(UNIT_DYNAMIC_FLAGS, (UNIT_DYNFLAG_DEAD | UNIT_FLAG_SPAWNING));
                pMadrigosa->CombatStop();
                pMadrigosa->DeleteThreatList();
                pMadrigosa->setFaction(35);
                me->CombatStop();
                IntroPhaseTimer = 4000;
                ++IntroPhase;
                break;
            case 15:
                if (pInstance)
                    me->SetInFront(me->GetUnit(pInstance->GetData64(DATA_BRUTALLUS_INTRO_EVENT)));
                me->SendHeartBeat();
                IntroPhaseTimer = 2000;
                ++IntroPhase;
                break;
            case 16:
                DoScriptText(YELL_INTRO_KILL_MADRIGOSA, me);
                IntroPhaseTimer = 8000;
                ++IntroPhase;
                break;
            case 17:
                DoScriptText(YELL_INTRO_TAUNT, me);
                me->CastSpell(me, SPELL_INTRO_BREAK_ICE, false);
                if (pInstance)
                    pInstance->SetData(DATA_BRUTALLUS_INTRO_EVENT, DONE);
                else
                    return;
                if (Unit *pTrigger = me->GetUnit(pInstance->GetData64(DATA_BRUTALLUS_TRIGGER)))
                    pTrigger->CastSpell(pTrigger, SPELL_INTRO_BREAK_ICE_KNOCKBACK, false);
                IntroPhaseTimer = 2000;
                ++IntroPhase;
                break;
            case 18:
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
                EnterEvadeMode();
                IntroPhaseTimer = 0;
                ++IntroPhase;
                break;
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (pInstance->GetData(DATA_BRUTALLUS_INTRO_EVENT) == DONE)
            ScriptedAI::MoveInLineOfSight(who);
    }

    void UpdateAI(const uint32 diff)
    {
        if (pInstance->GetData(DATA_BRUTALLUS_INTRO_EVENT) == SPECIAL || pInstance->GetData(DATA_BRUTALLUS_INTRO_EVENT) == IN_PROGRESS || IntroPhase == 18)
        {
            Creature* pMadrigosa = me->GetCreature(pInstance->GetData64(DATA_MADRIGOSA));
            if (!pMadrigosa)
                return;

            if (IntroPhase < 14 && pMadrigosa->GetSelection() == 0)       //ye, we spam it every tick, this bitch has special needs... :)
            {                                                             //still, with this, we have 2-3 moments where she sets her sel on null, but it's enough for now...
                pMadrigosa->SetSelection(me->GetGUID());
            }
            if (IntroPhase < 10 && IntroPhase > 12)
            {
                if (CheckGroundTimer.Expired(diff))
                {
                    float x, y, z;
                    me->GetPosition(x, y, z);
                    float ground_z = me->GetTerrain()->GetHeight(x, y, MAX_HEIGHT, true);
                    if(z > ground_z)
                        me->GetMap()->CreatureRelocation(me, x, y, z, me->GetOrientation());
                    CheckGroundTimer = 500;
                }
            }

            if (IntroPhaseTimer.Expired(diff))
                DoIntro();

            if (IntroPhase >= 4 && IntroPhase <= 9)
            {
                if (IntroFrostBoltTimer.Expired(diff))
                {
                    pMadrigosa->CastSpell(me, SPELL_INTRO_FROSTBOLT, false);
                    IntroFrostBoltTimer = 2000;
                }
                else
                    pMadrigosa->AI()->DoMeleeAttackIfReady();
            }

            DoMeleeAttackIfReady();
            CastNextSpellIfAnyAndReady();
            return;
        }

        if (!UpdateVictim())
            return;

        if (CheckTimer.Expired(diff))
        {
            DoZoneInCombat();

            me->SetSpeed(MOVE_RUN, 2.0f);
            CheckTimer = 1000;
        }

        if (SlashTimer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_METEOR_SLASH);
            SlashTimer = 11000;
        }

        if (StompTimer.Expired(diff))
        {
            AddSpellToCastWithScriptText(me->GetVictim(), SPELL_STOMP, RAND(YELL_LOVE1, YELL_LOVE2, YELL_LOVE3));
            StompTimer = 30000;
        }

        if (BurnTimer.Expired(diff))
        {
            if(Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 300.0f, true))
                AddSpellToCast(pTarget, SPELL_BURN);
            BurnTimer = 20000;
        }

        if (!Enraged && BerserkTimer.Expired(diff))
        {
            AddSpellToCastWithScriptText(me, SPELL_BERSERK, YELL_BERSERK);
            Enraged = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

struct npc_death_cloudAI : public ScriptedAI
{
    npc_death_cloudAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;
    uint8 Phase;
    Timer _Timer;
    Timer SummonTimer;
    Timer DespawnTimer;    //Madrigosa in Felmyst's model despawn timer
    bool summon;

    void Reset()
    {
        Phase = 0;
        DespawnTimer = 0;
        SummonTimer = 0;
        _Timer = 0;
        summon = false;
    }

    uint32 CalculateSummonTimer()
    {
        if (Unit* pMadrigosa= me->GetUnit(pInstance->GetData64(DATA_MADRIGOSA)))
        {
            float dist = me->GetDistance2d(pMadrigosa);
            uint32 steps = (int)(dist/10);
            return (int)60000/(steps+1);
        }
        return 5000;
    }

    void JustRespawned()
    {
        ForceSpellCast(me, SPELL_DEATH_CLOUD, INTERRUPT_AND_CAST_INSTANTLY);
        Phase = 1;
        SummonTimer = _Timer = CalculateSummonTimer();
    }

    void UpdateAI(const uint32 diff)
    {
        if (Phase == 1)
        {
            if (_Timer.Expired(diff))
            {
                float x, y, z;
                if (Unit* pMadrigosa = me->GetUnit(pInstance->GetData64(DATA_MADRIGOSA)))
                {
                    if (summon)
                    {
                        pMadrigosa->CastSpell(pMadrigosa, SPELL_TRANSFORM_FELMYST, false);
                        Phase = 2;
                        DespawnTimer = 1000;
                        return;
                    }
                    else if (!me->IsWithinDist(pMadrigosa, 10.0f))
                    {
                        me->GetNearPoint(x, y, z, 0.0f, 10.0f, me->GetAngle(pMadrigosa));
                        z = me->GetPositionZ();
                        me->UpdateAllowedPositionZ(x, y, z, true);
                        me->GetMap()->CreatureRelocation(me, x, y, z, 0);
                        Creature* Trigger = me->SummonTrigger(x, y, z, 0, SummonTimer.GetTimeLeft()*4);
                        if(Trigger)
                            Trigger->CastSpell(Trigger, SPELL_DEATH_CLOUD, true);
                        _Timer = SummonTimer.GetInterval();
                    }
                    else
                    {
                        pMadrigosa->GetPosition(x, y, z);
                        Creature* Trigger = me->SummonTrigger(x, y, z, 0, SummonTimer.GetTimeLeft());
                        if(Trigger)
                            Trigger->CastSpell(Trigger, SPELL_DEATH_CLOUD, true);
                        pMadrigosa->CastSpell(pMadrigosa, SPELL_FELMYST_PRE_VISUAL, true);
                        _Timer = 10000;
                        summon = true;
                    }
                }
            }
        }
        else if (Phase == 2)
        {
            if (DespawnTimer.Expired(diff))
            {
                if (Unit* pMadrigosa= me->GetUnit(pInstance->GetData64(DATA_MADRIGOSA)))
                {
                    pMadrigosa->RemoveAllAuras();
                    pMadrigosa->CastSpell(pMadrigosa, SPELL_FELMYST_SUMMON, false);
                    pMadrigosa->SetVisibility(VISIBILITY_OFF);
                    // kill madrigosa, not needed at this point
                    pMadrigosa->Kill(pMadrigosa, false);
                    ((Creature*)pMadrigosa)->RemoveCorpse();
                    Phase++;
                }
            }
        }
    }
};

struct brutallus_intro_triggerAI : public Scripted_NoMovementAI
{
    brutallus_intro_triggerAI(Creature *c) : Scripted_NoMovementAI(c)
    {
        pInstance = c->GetInstanceData();
        if(pInstance)
        {
            if(Unit *pMadrigosa = me->GetUnit(pInstance->GetData64(DATA_MADRIGOSA)))
                ((Creature*)pMadrigosa)->SetLevitate(true);
        }
    }

    ScriptedInstance* pInstance;

    void EnterCombat(Unit* /*pWho*/)
    {
        EnterEvadeMode();
    }

    void MoveInLineOfSight(Unit *who)
    {
        // temporary
        if(!pInstance)
            return;
        if(who->GetTypeId() == TYPEID_PLAYER && me->IsWithinDist(who, 40) && pInstance && pInstance->GetData(DATA_BRUTALLUS_INTRO_EVENT) == NOT_STARTED)
        {
            if(Unit *pBrutallus = me->GetUnit(pInstance->GetData64(DATA_BRUTALLUS)))
                pInstance->SetData(DATA_BRUTALLUS_INTRO_EVENT, SPECIAL);
        }
    }
};

CreatureAI* GetAI_boss_brutallus(Creature *_Creature)
{
    return new boss_brutallusAI (_Creature);
}

CreatureAI* GetAI_npc_death_cloud(Creature *_Creature)
{
    return new npc_death_cloudAI (_Creature);
}

CreatureAI* GetAI_brutallus_intro_trigger(Creature *_Creature)
{
    return new brutallus_intro_triggerAI (_Creature);
}

void AddSC_boss_brutallus()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_death_cloud";
    newscript->GetAI = &GetAI_npc_death_cloud;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_brutallus";
    newscript->GetAI = &GetAI_boss_brutallus;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="brutallus_intro_trigger";
    newscript->GetAI = &GetAI_brutallus_intro_trigger;
    newscript->RegisterSelf();
}
