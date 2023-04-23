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
SDName: Arcatraz
SD%Complete: 90
SDComment:
SDCategory: Tempest Keep, The Arcatraz
EndScriptData */

/* ContentData
npc_millhouse_manastorm
npc_warden_mellichar
mob_zerekethvoidzone
EndContentData */

#include "scriptPCH.h"
#include "def_arcatraz.h"

/*#####
# npc_millhouse_manastorm
#####*/

#define SAY_INTRO_1                 -1552010
#define SAY_INTRO_2                 -1552011
#define SAY_WATER                   -1552012
#define SAY_BUFFS                   -1552013
#define SAY_DRINK                   -1552014
#define SAY_READY                   -1552015
#define SAY_KILL_1                  -1552016
#define SAY_KILL_2                  -1552017
#define SAY_PYRO                    -1552018
#define SAY_ICEBLOCK                -1552019
#define SAY_LOWHP                   -1552020
#define SAY_DEATH                   -1552021
#define SAY_COMPLETE                -1552022

#define SPELL_CONJURE_WATER         36879
#define SPELL_ARCANE_INTELLECT      36880
#define SPELL_ICE_ARMOR             36881

#define SPELL_ARCANE_MISSILES       33833
#define SPELL_CONE_OF_COLD          12611
#define SPELL_FIRE_BLAST            13341
#define SPELL_FIREBALL              14034
#define SPELL_FROSTBOLT             15497
#define SPELL_PYROBLAST             33975

struct npc_millhouse_manastormAI : public ScriptedAI
{
    npc_millhouse_manastormAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    Timer EventProgress_Timer;
    uint32 Phase;
    bool Init;
    bool LowHp;

    Timer Pyroblast_Timer;
    Timer Fireball_Timer;

    void Reset()
    {
        EventProgress_Timer.Reset(2000);
        LowHp = false;
        Init = false;
        Phase = 1;

        Pyroblast_Timer.Reset(1000);
        Fireball_Timer.Reset(2500);

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);

        if( pInstance )
        {
            if( pInstance->GetData(TYPE_WARDEN_2) == DONE )
                Init = true;

            if( pInstance->GetData(TYPE_HARBINGERSKYRISS) == DONE )
            {
                DoScriptText(SAY_COMPLETE, me);
                me->ForcedDespawn(20000);
            }
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type != POINT_MOTION_TYPE)
            return;

        me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 4.8f);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP); // enable healing
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(RAND(SAY_KILL_1, SAY_KILL_2), me);
    }

    void JustDied(Unit *victim)
    {
        DoScriptText(SAY_DEATH, me);

        /*for questId 10886 (heroic mode only)
        if( pInstance && pInstance->GetData(TYPE_HARBINGERSKYRISS) != DONE )
            ->FailQuest();*/
    }

    void UpdateAI(const uint32 diff)
    {
        if( !Init )
        {
            if (EventProgress_Timer.Expired(diff))
            {
                if( Phase < 8 )
                {
                    switch( Phase )
                    {
                        case 1:
                            DoScriptText(SAY_INTRO_1, me);
                            EventProgress_Timer = 18000;
                            break;
                        case 2:
                            DoScriptText(SAY_INTRO_2, me);
                            EventProgress_Timer = 18000;
                            break;
                        case 3:
                            if( pInstance )
                                pInstance->SetData(TYPE_WARDEN_2,DONE);
                            EventProgress_Timer = 7000;
                            break;
                        case 4:
                            DoScriptText(SAY_WATER, me);
                            DoCast(me,SPELL_CONJURE_WATER);
                            EventProgress_Timer = 10000;
                            break;
                        case 5:
                            DoScriptText(SAY_BUFFS, me);
                            DoCast(me,SPELL_ICE_ARMOR);
                            EventProgress_Timer = 10000;
                            break;
                        case 6:
                             DoScriptText(SAY_DRINK, me);
                            DoCast(me,SPELL_ARCANE_INTELLECT);
                            EventProgress_Timer = 7000;
                            break;
                        case 7:
                            DoScriptText(SAY_READY, me);
                            Init = true;
                            break;
                    }
                    ++Phase;
                }
            }
        }

        if( !UpdateVictim() )
            return;

        if( !LowHp && ((me->GetHealth()*100 / me->GetMaxHealth()) < 20) )
        {
            DoScriptText(SAY_LOWHP, me);
            LowHp = true;
        }

        if (Pyroblast_Timer.Expired(diff))
        {
            if( me->IsNonMeleeSpellCast(false) )
                return;

             DoScriptText(SAY_PYRO, me);

            DoCast(me->GetVictim(),SPELL_PYROBLAST);
            Pyroblast_Timer = 40000;
        }

        if (Fireball_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_FIREBALL);
            Fireball_Timer = 4000;
        }

        //DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_millhouse_manastorm(Creature *_Creature)
{
    return new npc_millhouse_manastormAI (_Creature);
}

/*#####
# npc_warden_mellichar
#####*/

#define YELL_INTRO1         -1552023
#define YELL_INTRO2         -1552024
#define YELL_RELEASE1       -1552025
#define YELL_RELEASE2A      -1552026
#define YELL_RELEASE2B      -1552027
#define YELL_RELEASE3       -1552028
#define YELL_RELEASE4       -1552029
#define YELL_WELCOME        -1552030

//phase 2(acid mobs)
#define ENTRY_TRICKSTER     20905
#define ENTRY_PH_HUNTER     20906
//phase 3
#define ENTRY_MILLHOUSE     20977
//phase 4(acid mobs)
#define ENTRY_AKKIRIS       20908
#define ENTRY_SULFURON      20909
//phase 5(acid mobs)
#define ENTRY_TW_DRAK       20910
#define ENTRY_BL_DRAK       20911
//phase 6
#define ENTRY_SKYRISS       20912

//TARGET_SCRIPT
#define SPELL_TARGET_ALPHA  36856
#define SPELL_TARGET_BETA   36854
#define SPELL_TARGET_DELTA  36857
#define SPELL_TARGET_GAMMA  36858
#define SPELL_TARGET_OMEGA  36852
#define SPELL_BUBBLE_VISUAL 36849

#define SPELL_ETHEREAL_TELEPORT 34427

struct npc_warden_mellicharAI : public ScriptedAI
{
    npc_warden_mellicharAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    bool IsRunning;
    bool CanSpawn;

    Timer EventProgress_Timer;
    uint32 Phase;

    void Reset()
    {
        IsRunning = false;
        CanSpawn = false;

        EventProgress_Timer.Reset(22000);
        Phase = 1;

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
        DoCast(me,SPELL_TARGET_OMEGA);

        if( pInstance && !(pInstance->GetData(TYPE_HARBINGERSKYRISS) == DONE))
            pInstance->SetData(TYPE_HARBINGERSKYRISS,NOT_STARTED);
        if(Unit* millhouse = (Unit*)FindCreature(ENTRY_MILLHOUSE, 100, me))
            millhouse->ToCreature()->ForcedDespawn(0);
    }

    void AttackStart(Unit* who) {}

    void MoveInLineOfSight(Unit *who)
    {
        if( IsRunning )
            return;

        if( !me->GetVictim() && who->isTargetableForAttack() && ( me->IsHostileTo( who )) && who->isInAccessiblePlacefor(me) )
        {
            if (!me->CanFly() && me->GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
                return;
            if (who->GetTypeId() != TYPEID_PLAYER)
                return;

            float attackRadius = me->GetAttackDistance(who)/10;
            if( me->IsWithinDistInMap(who, attackRadius) && me->IsWithinLOSInMap(who) )
                EnterCombat(who);
        }
    }

    void EnterCombat(Unit *who)
    {
        if (!IsRunning)
        {
            DoScriptText(YELL_INTRO1, me);
            DoCast(me,SPELL_BUBBLE_VISUAL);
        }

        if( pInstance )
        {
            pInstance->HandleGameObject(pInstance->GetData64(DATA_SPHERE_SHIELD),false);
            IsRunning = true;
        }
    }

    bool CanProgress()
    {
        if( pInstance )
        {
            if( Phase == 7 && pInstance->GetData(TYPE_WARDEN_4) == DONE )
                return true;
            if( Phase == 6 && pInstance->GetData(TYPE_WARDEN_3) == DONE )
                return true;
            if( Phase == 5 && pInstance->GetData(TYPE_WARDEN_2) == DONE )
                return true;
            if( Phase == 4 || Phase == 1 || Phase == 2)
                return true;
            if( Phase == 3 && pInstance->GetData(TYPE_WARDEN_1) == DONE )
                return true;
            return false;
        }
        return false;
    }

    void DoPrepareForPhase()
    {
        if( pInstance )
        {
            me->InterruptNonMeleeSpells(true);
            me->RemoveSpellsCausingAura(SPELL_AURA_DUMMY);

            switch( Phase )
            {
                case 2:
                    DoCast(me,SPELL_TARGET_ALPHA);
                    pInstance->HandleGameObject(pInstance->GetData64(DATA_POD_A),true);
                    break;
                case 3:
                    DoCast(me,SPELL_TARGET_BETA);
                    pInstance->HandleGameObject(pInstance->GetData64(DATA_POD_B),true);
                    break;
                case 5:
                    DoCast(me,SPELL_TARGET_DELTA);
                    pInstance->HandleGameObject(pInstance->GetData64(DATA_POD_D),true);
                    break;
                case 6:
                    DoCast(me,SPELL_TARGET_GAMMA);
                    pInstance->HandleGameObject(pInstance->GetData64(DATA_POD_G),true);
                    break;
                case 7:
                    pInstance->HandleGameObject(pInstance->GetData64(DATA_POD_O),true);
                    pInstance->SetData(TYPE_HARBINGERSKYRISS,IN_PROGRESS);
                    break;
            }
            CanSpawn = true;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if( !IsRunning )
            return;

        if (EventProgress_Timer.Expired(diff))
        {
            if( pInstance )
            {
                if( pInstance->GetData(TYPE_HARBINGERSKYRISS) == FAIL )
                    Reset();
            }

            if( CanSpawn )
            {
                //continue beam omega pod, unless we are about to summon skyriss
                if( Phase != 7 )
                    DoCast(me,SPELL_TARGET_OMEGA);

                switch( Phase )
                {
                    case 2:
                        if (Creature* temp = me->SummonCreature(RAND(ENTRY_TRICKSTER, ENTRY_PH_HUNTER), 478.326, -148.505, 42.56, 3.19, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 600000))
                            temp->CastSpell(temp, SPELL_ETHEREAL_TELEPORT, false);
                        break;
                    case 3:
                        if (Creature* temp = me->SummonCreature(ENTRY_MILLHOUSE,413.292,-148.378,42.56,6.27,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,600000))
                        {
                            temp->CastSpell(temp, SPELL_ETHEREAL_TELEPORT, false);
                            temp->SetReactState(REACT_PASSIVE);
                        }
                        break;
                    case 4:
                        DoScriptText(YELL_RELEASE2B, me);
                        break;
                    case 5:
                        if (Creature* temp = me->SummonCreature(RAND(ENTRY_AKKIRIS, ENTRY_SULFURON), 420.179, -174.396, 42.58, 0.02, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 600000))
                           temp->CastSpell(temp, SPELL_ETHEREAL_TELEPORT, false);
                        break;
                    case 6:
                        if (Creature* temp = me->SummonCreature(RAND(ENTRY_TW_DRAK,ENTRY_BL_DRAK), 471.795, -174.58, 42.58, 3.06, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 600000))
                            temp->CastSpell(temp, SPELL_ETHEREAL_TELEPORT, false);

                        if (Creature* millhouse = GetClosestCreatureWithEntry(me, ENTRY_MILLHOUSE, 100))
                        {
                            millhouse->SetWalk(true);
                            millhouse->GetMotionMaster()->MovePoint(0, 445.55f, -157.658f, 43.06f);
                        }
                        break;
                    case 7:
                        if (Creature* temp = me->SummonCreature(ENTRY_SKYRISS,445.763,-191.639,44.64,1.60,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,600000))
                            temp->CastSpell(temp, SPELL_ETHEREAL_TELEPORT, false);
                        DoScriptText(YELL_WELCOME, me);
                        break;
                }
                CanSpawn = false;
                ++Phase;
            }
            if( CanProgress() )
            {
                switch( Phase )
                {
                    case 1:
                        DoScriptText(YELL_INTRO2, me);
                        EventProgress_Timer = 10000;
                        ++Phase;
                        break;
                    case 2:
                        DoScriptText(YELL_RELEASE1, me);
                        DoPrepareForPhase();
                        EventProgress_Timer = 7000;
                        break;
                    case 3:
                        DoScriptText(YELL_RELEASE2A, me);
                        DoPrepareForPhase();
                        EventProgress_Timer = 10000;
                        break;
                    case 4:
                        DoPrepareForPhase();
                        EventProgress_Timer = 15000;
                        break;
                    case 5:
                        DoScriptText(YELL_RELEASE3, me);
                        DoPrepareForPhase();
                        EventProgress_Timer = 15000;
                        break;
                    case 6:
                        DoScriptText(YELL_RELEASE4, me);
                        DoPrepareForPhase();
                        EventProgress_Timer = 15000;
                        break;
                    case 7:
                        DoPrepareForPhase();
                        EventProgress_Timer = 15000;
                        break;
                }
            }
        }
    }
};

CreatureAI* GetAI_npc_warden_mellichar(Creature *_Creature)
{
    return new npc_warden_mellicharAI (_Creature);
}

/*######
## npc_felfire_wave
######*/

#define SPELL_FELFIRE          35769

struct npc_felfire_waveAI : public ScriptedAI
{
    npc_felfire_waveAI(Creature* c) : ScriptedAI(c) {}

    Timer BurnTimer;

    void IsSummonedBy(Unit *summoner)
    {
        BurnTimer = 1;
        float x, y, z;
        me->SetSpeed(MOVE_RUN, 1.1);
        me->GetNearPoint(x, y, z, 0, 20, summoner->GetAngle(me));
        me->UpdateAllowedPositionZ(x, y, z);
        me->GetMotionMaster()->MovePoint(1, x, y, z);
    }

    void AttackStart(Unit* who) {}

    void MovementInform(uint32 type, uint32 id)
    {
        if (type != POINT_MOTION_TYPE || id != 1)
            return;

        me->ForcedDespawn(5000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (BurnTimer.Expired(diff))
        {
            DoCast(me, SPELL_FELFIRE, true);
            BurnTimer = 450;
        }
    }
};

CreatureAI* GetAI_npc_felfire_wave(Creature* _Creature)
{
    return new npc_felfire_waveAI(_Creature);
}

#define SPELL_AURA      (HeroicMode ? 38828 : 36716)
#define SPELL_EXPLODE   (HeroicMode ? 38830 : 36719)

struct npc_arcatraz_sentinelAI : public ScriptedAI
{
    npc_arcatraz_sentinelAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    Timer ThreatWipe_Timer;
    Timer Suicide_Timer;

    ScriptedInstance *pInstance;

    void Reset()
    {
        me->SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
        me->SetUInt32Value(UNIT_FIELD_BYTES_1, PLAYER_STATE_DEAD);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
        me->CastSpell(me, SPELL_AURA, true);

        ThreatWipe_Timer.Reset(urand(5000, 10000));
        Suicide_Timer = 0;
    }

    void EnterCombat(Unit *who)
    {
        me->SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_NONE);
        me->SetUInt32Value(UNIT_FIELD_BYTES_1, PLAYER_STATE_NONE);
        me->SetHealth(me->GetMaxHealth() * 40 / 100);
        me->ResetPlayerDamageReq();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (!Suicide_Timer.GetInterval())
        {
            if (ThreatWipe_Timer.Expired(diff))
            {
                DoResetThreat();
                ThreatWipe_Timer = urand(10000, 15000);
            }

            if (me->GetHealth()*100/me->GetMaxHealth() <= 12)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
                me->RemoveAllAuras();
                me->CastSpell(me, SPELL_EXPLODE, true);

                Suicide_Timer = 10000;
            }
        }
        else if (Suicide_Timer.Expired(diff))
            me->GetVictim()->Kill(me, false);

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_arcatraz_sentinel(Creature *_Creature)
{
    return new npc_arcatraz_sentinelAI (_Creature);
}

#define SPELL_SUMMON   36593

struct npc_warder_corpseAI : public ScriptedAI
{
    npc_warder_corpseAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    bool summon;

    void Reset()
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
        me->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 32);
        me->SetUInt32Value(UNIT_FIELD_BYTES_1, 7);
        summon = false;
    }

    void EnterCombat(Unit *who) { EnterEvadeMode(); }
    void AttackStart(Unit* who) {}

    void MoveInLineOfSight(Unit* who)
    {
        if (who->GetTypeId() == TYPEID_PLAYER && me->IsWithinMeleeRange(who) && !summon) //no GM support they fail last days :P
        {
            DoCast(me, SPELL_SUMMON, true);
            summon = true;
        }
    }
};

CreatureAI* GetAI_npc_warder_corpse(Creature *_Creature)
{
    return new npc_warder_corpseAI (_Creature);
}

struct npc_negaton_screamerAI : public ScriptedAI
{
    enum spells {
        // common
        SPELL_PSYCHIC_SCREAM        = 13704,

        SPELL_REDUCTION_ARCANE      = 34331,
        SPELL_REDUCTION_FIRE        = 34333,
        SPELL_REDUCTION_FROST       = 34334,
        SPELL_REDUCTION_NATURE      = 34335,
        SPELL_REDUCTION_HOLY        = 34336,
        SPELL_REDUCTION_SHADOW      = 34338,

        // normal
        SPELL_SHADOW_BOLT_VOLLEY    = 36736,
        SPELL_ARCANE_VOLLEY         = 36738,
        SPELL_LIGHTNING_BOLT_VOLLEY = 36740,
        SPELL_FROSTBOLT_VOLLEY      = 36741,
        SPELL_FIREBALL_VOLLEY       = 36742,
        SPELL_HOLY_BOLT_VOLLEY      = 36743,

        // heroic
        SPELL_SHADOW_BOLT_VOLLEY_HERO    = 38840,
        SPELL_ARCANE_VOLLEY_HERO         = 38835,
        SPELL_LIGHTNING_BOLT_VOLLEY_HERO = 36740,
        SPELL_FROSTBOLT_VOLLEY_HERO      = 38837,
        SPELL_FIREBALL_VOLLEY_HERO       = 38836,
        SPELL_HOLY_BOLT_VOLLEY_HERO      = 38838,
    };

    npc_negaton_screamerAI(Creature* c) : ScriptedAI(c) {}

    Timer fearTimer;
    Timer volleyTimer;
    SpellSchools school;

    void Reset()
    {
        fearTimer.Reset(urand(1000, 7000));
        volleyTimer = 0;
        school = SPELL_SCHOOL_NORMAL;
        me->RemoveSpellsCausingAura(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
    }

    uint32 GetVolleySpell()
    {
        if (me->GetMap()->IsHeroic())
        {
            switch (school)
            {
                case SPELL_SCHOOL_HOLY:     return SPELL_HOLY_BOLT_VOLLEY_HERO;
                case SPELL_SCHOOL_FIRE:     return SPELL_FIREBALL_VOLLEY_HERO;
                case SPELL_SCHOOL_NATURE:   return SPELL_LIGHTNING_BOLT_VOLLEY_HERO;
                case SPELL_SCHOOL_FROST:    return SPELL_FROSTBOLT_VOLLEY_HERO;
                case SPELL_SCHOOL_SHADOW:   return SPELL_SHADOW_BOLT_VOLLEY_HERO;
                default:
                case SPELL_SCHOOL_ARCANE:   return SPELL_ARCANE_VOLLEY_HERO;
            }
        }
        else
        {
            switch (school)
            {
                case SPELL_SCHOOL_HOLY:     return SPELL_HOLY_BOLT_VOLLEY;
                case SPELL_SCHOOL_FIRE:     return SPELL_FIREBALL_VOLLEY;
                case SPELL_SCHOOL_NATURE:   return SPELL_LIGHTNING_BOLT_VOLLEY;
                case SPELL_SCHOOL_FROST:    return SPELL_FROSTBOLT_VOLLEY;
                case SPELL_SCHOOL_SHADOW:   return SPELL_SHADOW_BOLT_VOLLEY;
                default:
                case SPELL_SCHOOL_ARCANE:   return SPELL_ARCANE_VOLLEY;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (volleyTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), GetVolleySpell());
            volleyTimer = urand(6000, 9000); // will be reset in SpellHitTarget
        }

        if (fearTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_PSYCHIC_SCREAM);
            fearTimer = urand(15000, 30000);
        }

        DoMeleeAttackIfReady();
    }

    void SpellHit(Unit*, const SpellEntry* spell)
    {
        if (school != SPELL_SCHOOL_NORMAL ||
            spell->SchoolMask & SPELL_SCHOOL_MASK_NORMAL)
            return;

        if (spell->SchoolMask & SPELL_SCHOOL_MASK_HOLY)
        {
            me->CastSpell(me, SPELL_REDUCTION_HOLY, true);
            school = SPELL_SCHOOL_HOLY;
        }
        else if (spell->SchoolMask & SPELL_SCHOOL_MASK_FIRE)
        {
            me->CastSpell(me, SPELL_REDUCTION_FIRE, true);
            school = SPELL_SCHOOL_FIRE;
        }
        else if (spell->SchoolMask & SPELL_SCHOOL_MASK_NATURE)
        {
            me->CastSpell(me, SPELL_REDUCTION_NATURE, true);
            school = SPELL_SCHOOL_NATURE;
        }
        else if (spell->SchoolMask & SPELL_SCHOOL_MASK_FROST)
        {
            me->CastSpell(me, SPELL_REDUCTION_FROST, true);
            school = SPELL_SCHOOL_FROST;
        }
        else if (spell->SchoolMask & SPELL_SCHOOL_MASK_SHADOW)
        {
            me->CastSpell(me, SPELL_REDUCTION_SHADOW, true);
            school = SPELL_SCHOOL_SHADOW;
        }
        else if (spell->SchoolMask & SPELL_SCHOOL_MASK_ARCANE)
        {
            me->CastSpell(me, SPELL_REDUCTION_ARCANE, true);
            school = SPELL_SCHOOL_ARCANE;
        }

        volleyTimer = urand(3000, 5000);
    }

    void SpellHitTarget(Unit*, const SpellEntry* spell)
    {
        if (school != SPELL_SCHOOL_NORMAL)
        {
            me->RemoveSpellsCausingAura(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
            school = SPELL_SCHOOL_NORMAL;
            volleyTimer = 0; // disable volleys
        }
    }
};

CreatureAI* GetAI_npc_negaton_screamer(Creature* _Creature)
{
    return new npc_negaton_screamerAI(_Creature);
}

void AddSC_arcatraz()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_millhouse_manastorm";
    newscript->GetAI = &GetAI_npc_millhouse_manastorm;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_warden_mellichar";
    newscript->GetAI = &GetAI_npc_warden_mellichar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_felfire_wave";
    newscript->GetAI = &GetAI_npc_felfire_wave;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="arcatraz_sentinel";
    newscript->GetAI = &GetAI_npc_arcatraz_sentinel;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="warder_corpse";
    newscript->GetAI = &GetAI_npc_warder_corpse;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_negaton_screamer";
    newscript->GetAI = &GetAI_npc_negaton_screamer;
    newscript->RegisterSelf();
}
