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
SDName: Boss_Lady_Vashj
SD%Complete: 99
SDComment: Missing blizzlike Shield Generators coords
SDCategory: Coilfang Resevoir, Serpent Shrine Cavern
EndScriptData */

#include "scriptPCH.h"
#include "def_serpent_shrine.h"
#include "../../../special/simple_ai.h"
#include "Spell.h"

#define SAY_INTRO                   -1548042
#define SAY_AGGRO1                  -1548043
#define SAY_AGGRO2                  -1548044
#define SAY_AGGRO3                  -1548045
#define SAY_AGGRO4                  -1548046
#define SAY_PHASE1                  -1548047
#define SAY_PHASE2                  -1548048
#define SAY_PHASE3                  -1548049
#define SAY_BOWSHOT1                -1548050
#define SAY_BOWSHOT2                -1548051
#define SAY_SLAY1                   -1548052
#define SAY_SLAY2                   -1548053
#define SAY_SLAY3                   -1548054
#define SAY_DEATH                   -1548055

#define SPELL_SURGE                 38044
#define SPELL_MIND_BLAST            38259
#define SPELL_MULTI_SHOT            38310
#define SPELL_SHOCK_BLAST           38509
#define SPELL_ENTANGLE              38316
#define SPELL_STATIC_CHARGE_TRIGGER 38280
#define SPELL_FORKED_LIGHTNING      40088
#define SPELL_SHOOT                 40873
#define SPELL_POISON_BOLT           40095
#define SPELL_TOXIC_SPORES          38575
#define SPELL_MAGIC_BARRIER         38112
#define SPELL_PARALYZE              38132
#define SPELL_PERSUASION            38511

#define MIDDLE_X                    30.134
#define MIDDLE_Y                    -923.65
#define MIDDLE_Z                    42.9

#define SPOREBAT_X                  30.977156
#define SPOREBAT_Y                  -925.297761
#define SPOREBAT_Z                  77.176567
#define SPOREBAT_O                  5.223932

#define SHIED_GENERATOR_CHANNEL       19870
#define ENCHANTED_ELEMENTAL           21958
#define TAINTED_ELEMENTAL             22009
#define COILFANG_STRIDER              22056
#define COILFANG_ELITE                22055
#define TOXIC_SPOREBAT                22140
#define TOXIC_SPORES_TRIGGER          22207

#define ITEM_TAINTED_CORE             31088

static float StriderNagaWP[15][3] =
{
    // {-82, -899, 17.0}, // [0]Woda
    {-60, -903, 20.7f}, // [1]Woda
    {-44, -906, 31.2f}, // [2]Schody: srodek
    {-27, -910, 41.5f}, // [3]Schody: koniec
    {  2, -916, 42.0f}, // [4]Koniec pierwsze mozliwej trasy
    // {60, -1034, 17.0}, // [5]Woda
    {57, -1013, 20.7f}, // [6]Woda
    { 49, -990, 34.4f}, // [7]Schody: srodek
    { 46, -978, 41.5f}, // [8]Schody: koniec
    { 38, -950, 42.0f}, // [9]Koniec drugiej mozliwej trasy
    // { 51, -814, 17.0}, // [10]Woda
    { 48, -833, 20.7f}, // [11]Woda
    { 43, -850, 32.4f}, // [12]Schody: srodek
    { 40, -867, 41.6f}, // [13]Schody: koniec
    { 35, -895, 42.1f}  // [14]Koniec trzeciej mozliwej trasy
};

static float ElementPos[8][4] =
{
    {8.3f,   -835.3f,  21.9f, 5},
    {53.4f,  -835.3f,  21.9f, 4.5f},
    {96,     -861.9f,  21.8f, 4},
    {96,     -986.4f,  21.4f, 2.5f},
    {54.4f,  -1010.6f, 22,    1.8f},
    {9.8f,   -1012,    21.7f, 1.4f},
    {-35,    -987.6f,  21.5f, 0.8f},
    {-58.9f, -901.6f,  21.5f, 6}
};

static float ElementWPPos[8][3] =
{
    {71.700752f, -883.905884f, 41.097168f},
    {45.039848f, -868.022827f, 41.097015f},
    {14.585141f, -867.894470f, 41.097061f},
    {-25.415508f, -906.737732f, 41.097061f},
    {-11.801594f, -963.405884f, 41.097067f},
    {14.556657f, -979.051514f, 41.097137f},
    {43.466549f, -979.406677f, 41.097027f},
    {69.945908f, -964.663940f, 41.097054f}
};

static float SporebatWPPos[8][3] =
{
    {31.6f, -896.3f, 74.1f},
    {9.1f,  -913.9f, 72},
    {5.2f,  -934.4f, 74.4f},
    {20.7f, -946.9f, 70.7f},
    {41,    -941.9f, 73},
    {47.7f, -927.3f, 77},
    {42.2f, -912.4f, 72.7f},
    {27,    -905.9f, 72}
};

static float ShieldGeneratorChannelPos[4][4] =
{
    {49.6262f, -902.181f, 43.0975f, 3.95683f},
    {10.988f,  -901.616f, 42.5371f, 5.4373f},
    {10.3859f, -944.036f, 42.5446f, 0.779888f},
    {49.3126f, -943.398f, 42.5501f, 2.40174f}
};

//Lady Vashj AI
struct boss_lady_vashjAI : public ScriptedAI
{
    boss_lady_vashjAI (Creature *c) : ScriptedAI(c)
    {
        instance = c->GetInstanceData();
    }

    ScriptedInstance *instance;

    uint64 ShieldGeneratorChannel[4];

    Timer AggroTimer;
    Timer ShockBlast_Timer;
    Timer Entangle_Timer;
    Timer StaticCharge_Timer;
    Timer ForkedLightning_Timer;
    Timer Check_Timer;
    Timer ParalyzeCheck_Timer;
    Timer EnchantedElemental_Timer;
    Timer TaintedElemental_Timer;
    Timer CoilfangElite_Timer;
    Timer CoilfangStrider_Timer;
    Timer SummonSporebat_Timer;
    Timer SummonSporebat_StaticTimer;
    Timer Possession_Timer;
    uint8 EnchantedElemental_Pos;
    uint8 Phase;
    uint8 path_nr;
    Timer InviCheckTimer;

    bool Entangle;
    bool InCombat;
    bool Intro;
    bool CanAttack;

    void Reset()
    {
        AggroTimer.Reset(19000);
        ShockBlast_Timer.Reset(urand(1, 60001));
        Entangle_Timer.Reset(30000);
        StaticCharge_Timer.Reset(urand(10000, 25000));
        ForkedLightning_Timer.Reset(2000);
        Check_Timer.Reset(15000);
        ParalyzeCheck_Timer.Reset(1000);
        EnchantedElemental_Timer.Reset(5000);
        TaintedElemental_Timer.Reset(50000);
        CoilfangElite_Timer.Reset(45000);
        CoilfangStrider_Timer.Reset(urand(60000, 70000));
        SummonSporebat_Timer.Reset(10000);
        SummonSporebat_StaticTimer.Reset(25000);
        Possession_Timer.Reset(10000);
        EnchantedElemental_Pos = 0;
        Phase = 0;
        Intro = false;

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);

        if (instance->GetData(DATA_PREVIOUS_BOSS_DEAD) != DONE)
        {
            me->SetVisibility(VISIBILITY_OFF);    
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->setFaction(35);
        }
        else
        {
            me->SetVisibility(VISIBILITY_ON);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->setFaction(14);
        }
        InviCheckTimer.Reset(5000);

        Entangle = false;
        CanAttack = false;

        for (uint8 i = 0; i < 4; i++)
        {
            if (Unit* remo = me->GetMap()->GetUnit(ShieldGeneratorChannel[i]))
                remo->setDeathState(JUST_DIED);

            ShieldGeneratorChannel[i] = 0;
        }

        instance->SetData(DATA_LADYVASHJEVENT, NOT_STARTED);

        me->SetCorpseDelay(1000*60*60);
    }

    void Paralyze(bool apply)
    {
        Map *map = me->GetMap();
        Map::PlayerList const &PlayerList = map->GetPlayers();

        for(Map::PlayerList::const_iterator i = PlayerList.begin();i != PlayerList.end(); ++i)
        {
            Player* i_pl = i->getSource();
            if(i_pl){
                bool check = ( apply ? i_pl->HasAura(SPELL_PARALYZE,0) && !i_pl->HasItemCount(ITEM_TAINTED_CORE,1,false) : i_pl->HasAura(SPELL_PARALYZE,0));

                if(apply && i_pl->HasItemCount(ITEM_TAINTED_CORE,1,false) && !i_pl->HasAura(SPELL_PARALYZE,0) )
                    i_pl->AddAura(SPELL_PARALYZE,i_pl);
                else if(check )
                    i_pl->RemoveAurasDueToSpell(SPELL_PARALYZE,0);
            }
        }
    }

    //Called when a tainted elemental dies
    void EventTaintedElementalDeath()
    {
        //the next will spawn 50 seconds after the previous one's death
        if (TaintedElemental_Timer.GetTimeLeft() > 50000)
            TaintedElemental_Timer.Reset(50000);
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2, SAY_SLAY3), me);
    }

    void JustDied(Unit *Killer)
    {
        Paralyze(false);
        DoScriptText(SAY_DEATH, me);

        instance->SetData(DATA_LADYVASHJEVENT, DONE);
    }

    void StartEvent()
    {
        DoScriptText(RAND(SAY_AGGRO1, SAY_AGGRO2, SAY_AGGRO3, SAY_AGGRO4), me);

        InCombat = true;
        Phase = 1;

        instance->SetData(DATA_LADYVASHJEVENT, IN_PROGRESS);
    }

    void EnterCombat(Unit *who)
    {
        if (instance)
        {
            //remove old tainted cores to prevent cheating in phase 2
            Map *map = me->GetMap();
            Map::PlayerList const &PlayerList = map->GetPlayers();
            for(Map::PlayerList::const_iterator i = PlayerList.begin();i != PlayerList.end(); ++i)
            {
                if(Player* i_pl = i->getSource())
                {
                    i_pl->DestroyItemCount(ITEM_TAINTED_CORE, 1, true);
                }
            }
        }

        if(Phase != 2)
            AttackStart(who);

        StartEvent();
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!Intro)
        {
            Intro = true;
            DoScriptText(SAY_INTRO, me);
        }
        if (!CanAttack || me->IsDead())
            return;

        if (!who || me->GetVictim())
            return;

        if (who->isTargetableForAttack() && who->isInAccessiblePlacefor(me) && me->IsHostileTo(who))
        {
            float attackRadius = me->GetAttackDistance(who);
            if (me->IsWithinDistInMap(who, attackRadius) && me->GetDistanceZ(who) <= CREATURE_Z_ATTACK_RANGE && me->IsWithinLOSInMap(who))
            {
                //if(who->HasStealthAura())
                //    who->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);

                if(Phase != 2)
                    AttackStart(who);

                if(!me->IsInCombat())
                    StartEvent();
            }
        }
    }

    void CastShootOrMultishot()
    {
        //Shoot
        //Used in Phases 1 and 3 after Entangle or while having nobody in melee range. A shot that hits her target for 4097-5543 Physical damage.
        //Multishot
        //Used in Phases 1 and 3 after Entangle or while having nobody in melee range. A shot that hits 1 person and 4 people around him for 6475-7525 physical damage.
        DoCast(me->GetVictim(), RAND(SPELL_SHOOT, SPELL_MULTI_SHOT));

        if(rand()%3)
            DoScriptText(RAND(SAY_BOWSHOT1, SAY_BOWSHOT2), me);
    }

    void UpdateAI(const uint32 diff)
    {
        if (me->getFaction() == 35 && InviCheckTimer.Expired(diff))
        {
            if (instance->GetData(DATA_PREVIOUS_BOSS_DEAD) == DONE)
            {
                me->SetVisibility(VISIBILITY_ON);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->setFaction(14);
            }
            InviCheckTimer = 5000;
            return;
        }

        if(!CanAttack && Intro)
        {
            if(AggroTimer.Expired(diff))
            {
                CanAttack = true;
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
                AggroTimer = 19000;
            }
            else
                return;
        }
        //to prevent abuses during phase 2
        Unit *nearTarget = SelectUnit(SELECT_TARGET_NEAREST,0,100.0f,true);
        if(Phase == 2 && !nearTarget && InCombat)
        {
            EnterEvadeMode();
            return;
        }
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        DoSpecialThings(diff, DO_PULSE_COMBAT, 200.0f);


        // Paralyze effect
        if (Phase == 2)
        {
            if (ParalyzeCheck_Timer.Expired(diff))
            {
                Paralyze(true);
                if (me->HasUnitState(UNIT_STAT_CHASE))
                {
                    me->GetMotionMaster()->Clear();
                    DoTeleportTo(MIDDLE_X, MIDDLE_Y, MIDDLE_Z);
                }
                ParalyzeCheck_Timer = 1000;
            }
        }

        if(Phase == 1 || Phase == 3)
        {
            //ShockBlast_Timer
            if (ShockBlast_Timer.Expired(diff))
            {
                //Shock Burst
                //Randomly used in Phases 1 and 3 on Vashj's target, it's a Shock spell doing 8325-9675 nature damage and stunning the target for 5 seconds, during which she will not attack her target but switch to the next person on the aggro list.
                //if(me->GetVictim()->HasAura(23920,0)) anti-reflect not needed anymore
                //    me->GetVictim()->RemoveAurasDueToSpell(23920);

                DoCast(me->GetVictim(), SPELL_SHOCK_BLAST);
                ShockBlast_Timer = 8000+rand()%12000;       //random cooldown
            }


            //StaticCharge_Timer
            if (StaticCharge_Timer.Expired(diff))
            {
                //Static Charge
                //Used on random people (only 1 person at any given time) in Phases 1 and 3, it's a debuff doing 2775 to 3225 Nature damage to the target and everybody in about 5 yards around it, every 1 seconds for 30 seconds. It can be removed by Cloak of Shadows, Iceblock, Divine Shield, etc, but not by Cleanse or Dispel Magic.
                Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_STATIC_CHARGE_TRIGGER), true);

                if(target && !target->HasAura(SPELL_STATIC_CHARGE_TRIGGER, 0))
                    DoCast(target, SPELL_STATIC_CHARGE_TRIGGER);

                StaticCharge_Timer = 10000+rand()%20000;    //blizzlike
            }


            //Entangle_Timer
            if (Entangle_Timer.Expired(diff))
            {
                if(!Entangle)
                {
                    //Entangle
                    //Used in Phases 1 and 3, it casts Entangling Roots on everybody in a 15 yard radius of Vashj, immobilzing them for 10 seconds and dealing 500 damage every 2 seconds. It's not a magic effect so it cannot be dispelled, but is removed by various buffs such as Cloak of Shadows or Blessing of Freedom.
                    DoCast(me->GetVictim(), SPELL_ENTANGLE);
                    Entangle = true;
                    Entangle_Timer = 10000;
                }
                else
                {
                    CastShootOrMultishot();
                    Entangle = false;
                    Entangle_Timer = 20000+rand()%5000;
                }
            }


            //Phase 1
            if(Phase == 1)
            {
                //Start phase 2
                if ((me->GetHealth()*100 / me->GetMaxHealth()) < 70)
                {
                    //Phase 2 begins when Vashj hits 70%. She will run to the middle of her platform and surround herself in a shield making her invulerable.
                    Phase = 2;

                    me->GetMotionMaster()->MovementExpired();
                    DoTeleportTo(MIDDLE_X, MIDDLE_Y, MIDDLE_Z);

                    Creature *pCreature;
                    for(uint8 i = 0; i < 4; i++)
                    {
                        pCreature = me->SummonCreature(SHIED_GENERATOR_CHANNEL, ShieldGeneratorChannelPos[i][0],  ShieldGeneratorChannelPos[i][1],  ShieldGeneratorChannelPos[i][2],  ShieldGeneratorChannelPos[i][3], TEMPSUMMON_CORPSE_DESPAWN, 0);
                        if (pCreature)
                            ShieldGeneratorChannel[i] = pCreature->GetGUID();
                    }

                    me->CastSpell(me, SPELL_MAGIC_BARRIER,true);
                    DoScriptText(SAY_PHASE2, me);
                }
            }
            //Phase 3
            else
            {
                //SummonSporebat_Timer
                if (SummonSporebat_Timer.Expired(diff))
                {
                    Creature *Sporebat = NULL;
                    Sporebat = me->SummonCreature(TOXIC_SPOREBAT, SPOREBAT_X, SPOREBAT_Y, SPOREBAT_Z, SPOREBAT_O, TEMPSUMMON_CORPSE_DESPAWN, 0);

                    if(Sporebat)
                    {
                        if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                            Sporebat->AI()->AttackStart(target);
                    }

                    //summon sporebats faster and faster
                    if(SummonSporebat_StaticTimer.GetInterval() > 1000)
                        SummonSporebat_StaticTimer.SetInterval(SummonSporebat_StaticTimer.GetInterval() - 1000);

                    SummonSporebat_Timer = SummonSporebat_StaticTimer;

                    if(SummonSporebat_Timer.GetTimeLeft() < 5000)
                        SummonSporebat_Timer.Reset(5000);

                }

                if (Possession_Timer.Expired(diff))
                {
                    // HACK, not working now (diferent effects get diferent targets)
                    //DoCast(NULL, SPELL_PERSUASION);
                    std::list<Unit*> targets;
                    SelectUnitList(targets, 3, SELECT_TARGET_RANDOM, 0, true);
                    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); i++)
                    {
                        DoCast(*i, SPELL_PERSUASION, true);
                    }
                    Possession_Timer = urand(20000, 30000);
                }
            }

            //Melee attack
            DoMeleeAttackIfReady();

            //Check_Timer - used to check if somebody is in melee range
            if (Check_Timer.Expired(diff))
            {
                bool InMeleeRange = false;
                Unit *target;
                std::list<HostileReference *> t_list = me->getThreatManager().getThreatList();
                for(std::list<HostileReference *>::iterator itr = t_list.begin(); itr!= t_list.end(); ++itr)
                {
                    target = Unit::GetUnit(*me, (*itr)->getUnitGuid());
                                                            //if in melee range
                    if(target && target->IsWithinDistInMap(me, 5))
                    {
                        InMeleeRange = true;
                        break;
                    }
                }

                //if nobody is in melee range
                if(!InMeleeRange)
                    CastShootOrMultishot();

                Check_Timer = 2000;
            }
        }
        //Phase 2
        else
        {
            //ForkedLightning_Timer
            if (ForkedLightning_Timer.Expired(diff))
            {
                //Forked Lightning
                //Used constantly in Phase 2, it shoots out completely randomly targeted bolts of lightning which hit everybody in a roughtly 60 degree cone in front of Vashj for 2313-2687 nature damage.
                Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_FORKED_LIGHTNING), true);

                if(!target)
                    target = me->GetVictim();

                DoCast(target, SPELL_FORKED_LIGHTNING);

                ForkedLightning_Timer = 2000+rand()%6000;   //blizzlike
            }


            //EnchantedElemental_Timer
            if (EnchantedElemental_Timer.Expired(diff))
            {
                Creature *Elemental;

                for (int i = 0; i < 4; i++)
                {
                    EnchantedElemental_Pos = i * 2 + (rand()%2);
                    Elemental = me->SummonCreature(ENCHANTED_ELEMENTAL, ElementPos[EnchantedElemental_Pos][0], ElementPos[EnchantedElemental_Pos][1], ElementPos[EnchantedElemental_Pos][2], ElementPos[EnchantedElemental_Pos][3], TEMPSUMMON_CORPSE_DESPAWN, 0);
                }

                if (Elemental)
                    EnchantedElemental_Timer = 20000+rand()%5000;

            }


            //TaintedElemental_Timer
            if (TaintedElemental_Timer.Expired(diff))
            {
                Creature *Tain_Elemental;
                uint32 pos = rand()%8;
                Tain_Elemental = me->SummonCreature(TAINTED_ELEMENTAL, ElementPos[pos][0], ElementPos[pos][1], ElementPos[pos][2], ElementPos[pos][3], TEMPSUMMON_DEAD_DESPAWN, 0);

                TaintedElemental_Timer = 120000;
            }


            //CoilfangElite_Timer
            if (CoilfangElite_Timer.Expired(diff))
            {
                path_nr = urand(0,3);
                me->SummonCreature(COILFANG_ELITE, StriderNagaWP[path_nr*4][0],StriderNagaWP[path_nr*4][1],StriderNagaWP[path_nr*4][2],0, TEMPSUMMON_DEAD_DESPAWN, 0);
                CoilfangElite_Timer = 50000+rand()%5000;
            }


            //CoilfangStrider_Timer
            if (CoilfangStrider_Timer.Expired(diff))
            {
                uint32 pos = rand()%3;
                me->SummonCreature(COILFANG_STRIDER, StriderNagaWP[pos*4][0],StriderNagaWP[pos*4][1],StriderNagaWP[pos*4][2],0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);

                CoilfangStrider_Timer = 60000+rand()%10000;
            }


            //Check_Timer
            if (Check_Timer.Expired(diff))
            {
                //Start Phase 3
                if(instance && instance->GetData(DATA_CANSTARTPHASE3))
                {
                    //set life 50%
                    me->SetHealth(me->GetMaxHealth()/2);
                    me->RemoveAurasDueToSpell(SPELL_MAGIC_BARRIER);

                    DoResetThreat();
                    DoScriptText(SAY_PHASE3, me);

                    Phase = 3;
                    Paralyze(false);
                    //return to the tank
                    me->GetMotionMaster()->MoveChase(me->GetVictim());
                }
                Check_Timer = 1000;
            }
        }
    }
};
class VashjSurgeAura : public Aura
{
    public:
        VashjSurgeAura(SpellEntry *spell, uint32 eff, int32 *bp, Unit *target, Unit *caster) : Aura(spell, eff, bp, target, caster, NULL)
            {}
};
//Enchanted Elemental
//If one of them reaches Vashj he will increase her damage done by 5%.
struct mob_enchanted_elementalAI : public ScriptedAI
{
    mob_enchanted_elementalAI(Creature *c) : ScriptedAI(c)
    {
        instance = (c->GetInstanceData());
    }

    ScriptedInstance *instance;
    Timer move;
    uint32 phase;
    float x, y, z;
    Unit *Vashj;

    void Reset()
    {
        me->SetSpeed(MOVE_WALK,0.6);//walk
        me->SetSpeed(MOVE_RUN,0.6);//run
        move = 1000;
        phase = 1;
        Vashj = NULL;

        for (int i = 0;i<8;i++)//search for nearest waypoint (up on stairs)
        {
            if (!x || !y || !z)
            {
                x = ElementWPPos[i][0];
                y = ElementWPPos[i][1];
                z = ElementWPPos[i][2];
            }
            else
            {
                if (me->GetDistance(ElementWPPos[i][0],ElementWPPos[i][1],ElementWPPos[i][2]) < me->GetDistance(x,y,z))
                {
                    x = ElementWPPos[i][0];
                    y = ElementWPPos[i][1];
                    z = ElementWPPos[i][2];
                }
            }
        }
        if (instance)
            Vashj = Unit::GetUnit((*me), instance->GetData64(DATA_LADYVASHJ));
    }

    void EnterCombat(Unit *who) { return; }

    void MoveInLineOfSight(Unit *who){return;}

    void UpdateAI(const uint32 diff)
    {
        if(!instance)
            return;

        if (!Vashj)
        {
            return;
        }

        if (move.Expired(diff))
        {
            me->SetWalk(true);
            me->SetSpeed(MOVE_WALK, 0.6, true);
            if (phase == 1)
            {
                me->GetMotionMaster()->MovePoint(0, x, y, z);
            }
            if (phase == 1 && me->GetDistance(x,y,z) < 0.1)
            {
                phase = 2;
            }
            if (phase == 2)
            {
                me->GetMotionMaster()->MovePoint(0, MIDDLE_X, MIDDLE_Y, MIDDLE_Z);
                phase = 3;
            }
            if (phase == 3)
            {
                me->GetMotionMaster()->MovePoint(0, MIDDLE_X, MIDDLE_Y, MIDDLE_Z);
                if(me->GetDistance(MIDDLE_X, MIDDLE_Y, MIDDLE_Z) < 3)
                {
                    SpellEntry *spell = (SpellEntry *)GetSpellStore()->LookupEntry(SPELL_SURGE);
                    if( spell )
                    {
                        for(uint32 i = 0;i<3;i++)
                        {
                            if (!spell->Effect[i])
                                continue;

                            Vashj->AddAura(new VashjSurgeAura(spell, i, NULL, Vashj, Vashj));
                        }
                    }
                    me->DealDamage(me, me->GetMaxHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                }
            }
            if(((boss_lady_vashjAI*)((Creature*)Vashj)->AI())->InCombat == false || ((boss_lady_vashjAI*)((Creature*)Vashj)->AI())->Phase != 2 || Vashj->IsDead())
            {
                //call Unsummon()
                me->DealDamage(me, me->GetMaxHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            }
            move = 1000;
        }
    }
};

//Tainted Elemental
//This mob has 7,900 life, doesn't move, and shoots Poison Bolts at one person anywhere in the area, doing 3,000 nature damage and placing a posion doing 2,000 damage every 2 seconds. He will switch targets often, or sometimes just hang on a single player, but there is nothing you can do about it except heal the damage and kill the Tainted Elemental
struct mob_tainted_elementalAI : public Scripted_NoMovementAI
{
    mob_tainted_elementalAI(Creature *c) : Scripted_NoMovementAI(c)
    {
        instance = (c->GetInstanceData());
    }

    ScriptedInstance *instance;

    Timer PoisonBolt_Timer;
    Timer Despawn_Timer;

    void Reset()
    {
        PoisonBolt_Timer.Reset(5000 + rand() % 5000);
        Despawn_Timer.Reset(16000);
    }

    void JustDied(Unit *killer)
    {
        if(instance)
        {
            Creature *Vashj = Unit::GetCreature((*me), instance->GetData64(DATA_LADYVASHJ));

            if(Vashj)
                ((boss_lady_vashjAI*)Vashj->AI())->EventTaintedElementalDeath();
        }
    }

    void EnterCombat(Unit *who)
    {
        me->AddThreat(who, 0.1f);
    }

    void UpdateAI(const uint32 diff)
    {
        //PoisonBolt_Timer
        if (PoisonBolt_Timer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_POISON_BOLT), true))
                DoCast(target, SPELL_POISON_BOLT);

            PoisonBolt_Timer = 5000+rand()%5000;
        }

        //Despawn_Timer
        if (Despawn_Timer.Expired(diff))
        {
            //call Unsummon()
            me->setDeathState(DEAD);
            me->AI()->JustDied(me);

            //to prevent crashes
            Despawn_Timer = 5000;
        }
    }
};

//Toxic Sporebat
//Toxic Spores: Used in Phase 3 by the Spore Bats, it creates a contaminated green patch of ground, dealing about 2775-3225 nature damage every second to anyone who stands in it.
struct mob_toxic_sporebatAI : public ScriptedAI
{
    mob_toxic_sporebatAI(Creature *c) : ScriptedAI(c)
    {
        instance = (c->GetInstanceData());
        EnterEvadeMode();
        SpellEntry *TempSpell = (SpellEntry*)GetSpellStore()->LookupEntry(SPELL_TOXIC_SPORES);
        if(TempSpell)
        {
            TempSpell->EffectBasePoints[0] = 1500;
        }
    }

    ScriptedInstance *instance;

    bool moving;
    Timer ToxicSpore_Timer;
    Timer bolt_timer;
    Timer Check_Timer;

    void Reset()
    {
        me->SetLevitate(true);
        me->setFaction(14);
        moving = false;
        ToxicSpore_Timer.Reset(5000);
        bolt_timer.Reset(5500);
        Check_Timer.Reset(1000);
    }

    void EnterCombat(Unit *who)
    {

    }

    void MoveInLineOfSight(Unit *who)
    {

    }

    void MovementInform(uint32 type, uint32 id)
    {
        if(type != POINT_MOTION_TYPE)
            return;

        if(id == 1)
            moving = false;
    }

    void UpdateAI (const uint32 diff)
    {
        DoSpecialThings(diff, DO_PULSE_COMBAT, 300.0f);

        //Random movement
        if (!moving)
        {
            uint32 rndpos = rand()%8;
            me->GetMotionMaster()->MovePoint(1,SporebatWPPos[rndpos][0], SporebatWPPos[rndpos][1], SporebatWPPos[rndpos][2]);
            moving = true;
        }

        //toxic spores
        if (bolt_timer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 300, true))
            {
                if(Creature* trig = me->SummonCreature(TOXIC_SPORES_TRIGGER,target->GetPositionX(),target->GetPositionY(),target->GetPositionZ(),0,TEMPSUMMON_TIMED_DESPAWN,30000))
                {
                    trig->setFaction(14);
                    trig->CastSpell(trig, SPELL_TOXIC_SPORES,true);
                }
            }
            bolt_timer = 10000+rand()%5000;
        }


        //Check_Timer
        if (Check_Timer.Expired(diff))
        {
            if(instance)
            {
                //check if vashj is death
                Unit *Vashj = Unit::GetUnit((*me), instance->GetData64(DATA_LADYVASHJ));
                if(!Vashj || (Vashj && !Vashj->IsAlive()) || (Vashj && ((boss_lady_vashjAI*)((Creature*)Vashj)->AI())->Phase != 3))
                {
                    //remove
                    me->setDeathState(DEAD);
                    me->RemoveCorpse();
                    me->setFaction(35);
                }
            }

            Check_Timer = 1000;
        }
    }
};

struct mob_coilfang_eliteAI : public ScriptedAI
{
    mob_coilfang_eliteAI(Creature *c) : ScriptedAI(c)
    {
        instance = (c->GetInstanceData());
    }

    ScriptedInstance *instance;

    uint8 MoveWP, path_nr;
    Timer Cleave_Timer, Check_Timer;

    bool Move, OnPath;

    void Reset()
    {
        me->SetSpeed(MOVE_WALK, 3);
        me->SetSpeed(MOVE_RUN, 3);

        for(int i = 0; i < 3; ++i)
        {
            if(me->GetDistance(StriderNagaWP[i*4][0],StriderNagaWP[i*4][1],StriderNagaWP[i*4][2]) < 5)
            {
                path_nr = i;
                break;
            }
        }

        Cleave_Timer.Reset(10000 + rand() % 5000);
        Check_Timer.Reset(2000);

        MoveWP = 1;
        Move = false;
        OnPath = true;

        me->GetMotionMaster()->MovePoint(MoveWP,StriderNagaWP[4*path_nr + 1][0],StriderNagaWP[4*path_nr + 1][1],StriderNagaWP[4*path_nr + 1][2]);
    }

    void MoveInLineOfSight(Unit* who)
    {
        if(OnPath)
            return;

        ScriptedAI::MoveInLineOfSight(who);
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        if(OnPath)
        {
            me->SetSpeed(MOVE_WALK,1.5);
            me->SetSpeed(MOVE_RUN,1.5);

            me->GetMotionMaster()->Clear(false);
            DoStartMovement(done_by);
            OnPath = false;
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
       if(type != POINT_MOTION_TYPE)
            return;

       Move = true;
       ++MoveWP;
    }

    void UpdateAI (const uint32 diff)
    {
        if(Move)
        {
            if(MoveWP >= 4)
            {
                me->GetMotionMaster()->Clear(false);

                me->SetSpeed(MOVE_WALK,1.5);
                me->SetSpeed(MOVE_RUN,1.5);

                OnPath = false;

                if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0,60,true))
                    DoStartMovement(target);

            }else
                me->GetMotionMaster()->MovePoint(MoveWP,StriderNagaWP[4*path_nr + MoveWP][0],StriderNagaWP[4*path_nr + MoveWP][1],StriderNagaWP[4*path_nr + MoveWP][2]);

            Move = false;
        }

        if(Check_Timer.Expired(diff))
        {
            DoZoneInCombat();

            if(instance && instance->GetData(DATA_LADYVASHJEVENT) != IN_PROGRESS)
                me->Kill(me,false);

              Check_Timer = 2000;
        }

        if(OnPath)
            return;

        if(!UpdateVictim())
            return;

        if (Cleave_Timer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(),31345,false);
            Cleave_Timer = 10000+rand()%5000;
        }

        DoMeleeAttackIfReady();
    }
};
//Coilfang Strider
struct mob_coilfang_striderAI : public ScriptedAI
{
    mob_coilfang_striderAI(Creature *c) : ScriptedAI(c)
    {
        instance = (c->GetInstanceData());
    }

    ScriptedInstance *instance;

    uint8 MoveWP, path_nr;
    Timer MindBlast_Timer, Check_Timer;

    bool Move, OnPath;

    void Reset()
    {
        me->SetSpeed(MOVE_RUN, 2);

        me->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 2);
        me->SetFloatValue(UNIT_FIELD_COMBATREACH, 2);

        for(int i = 0; i < 3; ++i)
        {
            if(me->GetDistance(StriderNagaWP[i*4][0],StriderNagaWP[i*4][1],StriderNagaWP[i*4][2]) < 5)
            {
                path_nr = i;
                break;
            }
        }

        MindBlast_Timer.Reset(3000 + rand() % 10000);
        Check_Timer.Reset(2000);

        MoveWP = 1;
        Move = false;
        OnPath = true;

        me->CastSpell(me,38257,true);
        me->GetMotionMaster()->MovePoint(MoveWP,StriderNagaWP[4*path_nr + 1][0],StriderNagaWP[4*path_nr + 1][1],StriderNagaWP[4*path_nr + 1][2]);
    }

    void MoveInLineOfSight(Unit* who)
    {
        if(OnPath)
            return;

        ScriptedAI::MoveInLineOfSight(who);
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        if(OnPath)
        {
            me->SetSpeed(MOVE_WALK, 1.1);
            me->SetSpeed(MOVE_RUN, 1.1);

            me->GetMotionMaster()->Clear(false);
            DoStartMovement(done_by);
            OnPath = false;
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
       if(type != POINT_MOTION_TYPE)
            return;

       Move = true;
       ++MoveWP;
    }

    void UpdateAI (const uint32 diff)
    {
        if(Move)
        {
            if(MoveWP >= 4)
            {
                me->SetSpeed(MOVE_WALK, 1.1);
                me->SetSpeed(MOVE_RUN, 1.1);

                me->GetMotionMaster()->Clear(false);
                OnPath = false;

                if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0,60,true))
                    DoStartMovement(target);
            }
            else
                me->GetMotionMaster()->MovePoint(MoveWP,StriderNagaWP[4*path_nr + MoveWP][0],StriderNagaWP[4*path_nr + MoveWP][1],StriderNagaWP[4*path_nr + MoveWP][2]);

            Move = false;
        }

        if (Check_Timer.Expired(diff))
        {
            DoZoneInCombat();

            if (instance && instance->GetData(DATA_LADYVASHJEVENT) != IN_PROGRESS)
                me->Kill(me, false);

            Check_Timer = 2000;
        }

        if(OnPath)
            return;

        if(!UpdateVictim())
            return;

        if (MindBlast_Timer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(),SPELL_MIND_BLAST,true);
            MindBlast_Timer = 3000+rand()%1000;
        }

        DoMeleeAttackIfReady();
    }
};
struct mob_shield_generator_channelAI : public ScriptedAI
{
    mob_shield_generator_channelAI(Creature *c) : ScriptedAI(c)
    {
        instance = (c->GetInstanceData());
    }

    ScriptedInstance *instance;
    Timer Check_Timer;
    bool Cast;
    void Reset()
    {
        Check_Timer = 1000;
        Cast = false;
        me->SetUInt32Value(UNIT_FIELD_DISPLAYID , 11686);  //invisible

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void EnterCombat(Unit *who) { return; }

    void MoveInLineOfSight(Unit *who) { return; }

    void UpdateAI (const uint32 diff)
    {
        if(!instance)
            return;

        if (Check_Timer.Expired(diff))
        {
            Unit *Vashj = NULL;
            Vashj = Unit::GetUnit((*me), instance->GetData64(DATA_LADYVASHJ));

            if(Vashj && Vashj->IsAlive())
            {
                //start visual channel
                if (!Cast || !Vashj->HasAura(SPELL_MAGIC_BARRIER,0))
                {
                    me->CastSpell(Vashj,SPELL_MAGIC_BARRIER,true);
                    Cast = true;
                }
            }
            Check_Timer = 1000;
        }
    }
};

bool ItemUse_item_tainted_core(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    if(targets.getUnitTarget() && targets.getUnitTarget()->GetTypeId() == TYPEID_PLAYER)
    {
            player->DestroyItemCount(ITEM_TAINTED_CORE, 1, true);
            player->CastSpell(targets.getUnitTarget(), 38134, true);
            return true;
    }
    else if(targets.getUnitTarget() && targets.getUnitTarget()->GetTypeId() == TYPEID_UNIT )
            return false;

    ScriptedInstance *instance = (player->GetInstanceData()) ? (player->GetInstanceData()) : NULL;

    if(!instance)
    {
        player->GetSession()->SendNotification("Instance script not initialized");
        return true;
    }

    Creature *Vashj = Unit::GetCreature((*player), instance->GetData64(DATA_LADYVASHJ));
    if(Vashj && ((boss_lady_vashjAI*)Vashj->AI())->Phase == 2)
    {
        if(targets.getGOTarget() && targets.getGOTarget()->GetTypeId()==TYPEID_GAMEOBJECT)
        {
            uint32 identifier;
            uint8 channel_identifier;
            switch(targets.getGOTarget()->GetEntry())
            {
                case 185052:
                    identifier = DATA_SHIELDGENERATOR1;
                    channel_identifier = 0;
                    break;
                case 185053:
                    identifier = DATA_SHIELDGENERATOR2;
                    channel_identifier = 1;
                    break;
                case 185051:
                    identifier = DATA_SHIELDGENERATOR3;
                    channel_identifier = 2;
                    break;
                case 185054:
                    identifier = DATA_SHIELDGENERATOR4;
                    channel_identifier = 3;
                    break;
                default:
                    return true;
            }

            if(instance->GetData(identifier))
            {
                player->GetSession()->SendNotification("Already deactivated");
                return true;
            }

            //get and remove channel
            Unit *Channel = Unit::GetUnit((*Vashj), ((boss_lady_vashjAI*)Vashj->AI())->ShieldGeneratorChannel[channel_identifier]);
            if(Channel)
            {
                //call Unsummon()
                Channel->setDeathState(JUST_DIED);
            }

            instance->SetData(identifier, 1);

            //remove this item
            player->DestroyItemCount(ITEM_TAINTED_CORE, 1, true);
            return true;
        }
    }
    return true;
}

CreatureAI* GetAI_boss_lady_vashj(Creature *_Creature)
{
    return new boss_lady_vashjAI (_Creature);
}

CreatureAI* GetAI_mob_enchanted_elemental(Creature *_Creature)
{
    return new mob_enchanted_elementalAI (_Creature);
}

CreatureAI* GetAI_mob_tainted_elemental(Creature *_Creature)
{
    return new mob_tainted_elementalAI (_Creature);
}

CreatureAI* GetAI_mob_toxic_sporebat(Creature *_Creature)
{
    return new mob_toxic_sporebatAI (_Creature);
}

CreatureAI* GetAI_mob_shield_generator_channel(Creature *_Creature)
{
    return new mob_shield_generator_channelAI (_Creature);
}

CreatureAI* GetAI_mob_coilfang_elite(Creature *_Creature)
{
    return new mob_coilfang_eliteAI (_Creature);
}

CreatureAI* GetAI_mob_coilfang_strider(Creature *_Creature)
{
    return new mob_coilfang_striderAI (_Creature);
}

void AddSC_boss_lady_vashj()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_lady_vashj";
    newscript->GetAI = &GetAI_boss_lady_vashj;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_enchanted_elemental";
    newscript->GetAI = &GetAI_mob_enchanted_elemental;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_tainted_elemental";
    newscript->GetAI = &GetAI_mob_tainted_elemental;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_toxic_sporebat";
    newscript->GetAI = &GetAI_mob_toxic_sporebat;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_coilfang_elite";
    newscript->GetAI = &GetAI_mob_coilfang_elite;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_coilfang_strider";
    newscript->GetAI = &GetAI_mob_coilfang_strider;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_shield_generator_channel";
    newscript->GetAI = &GetAI_mob_shield_generator_channel;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="item_tainted_core";
    newscript->pItemUse = &ItemUse_item_tainted_core;
    newscript->RegisterSelf();
}

