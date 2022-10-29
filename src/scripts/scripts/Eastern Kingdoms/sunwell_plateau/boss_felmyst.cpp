/*
 * Copyright (C) 2009 TrinityCore <http://www.trinitycore.org/>
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
SDName: Boss_Felmyst
SD%Complete: 95
SDComment: Make Kalecgos outro cut-scene. Final debugging
EndScriptData */

#include "precompiled.h"
#include "def_sunwell_plateau.h"

enum Quotes
{
    YELL_BIRTH      =       -1580036,
    YELL_KILL1      =       -1580037,
    YELL_KILL2      =       -1580038,
    YELL_BREATH     =       -1580039,
    YELL_TAKEOFF    =       -1580040,
    YELL_BERSERK    =       -1580041,
    YELL_DEATH      =       -1580042,
    EMOTE_BREATH    =       -1811004
};

enum Spells
{
    //Aura
    AURA_NOXIOUS_FUMES          =   47002,

    //Land phase
    SPELL_CLEAVE                =   19983,
    SPELL_CORROSION             =   45866,
    SPELL_GAS_NOVA              =   45855,
    SPELL_ENCAPSULATE_CHANNEL   =   45661,
    SPELL_ENCAPSULATE_EFFECT    =   45665, // linked in DB

    //Flight phase
    SPELL_VAPOR_SELECT          =   45391,   // fel to player, force cast 45392, 50000y selete target
    SPELL_VAPOR_SUMMON          =   45392,   // player summon vapor, radius around caster, 5y,
    SPELL_VAPOR_FORCE           =   45388,   // vapor to fel, force cast 45389
    SPELL_VAPOR_CHANNEL         =   45389,   // fel to vapor, green beam channel
    SPELL_VAPOR_TRIGGER         =   45411,   // linked to 45389, vapor to self, trigger 45410 and 46931
    SPELL_VAPOR_DAMAGE          =   46931,   // vapor damage, 4000
    SPELL_TRAIL_SUMMON          =   45410,   // vapor summon trail
    SPELL_TRAIL_TRIGGER         =   45399,   // trail to self, trigger 45402
    SPELL_TRAIL_DAMAGE          =   45402,   // trail damage, 2000 + 2000 dot
    SPELL_DEAD_SUMMON           =   45400,   // summon blazing dead, 5min
    SPELL_DEAD_PASSIVE          =   45415,   // aura in creature_template_addon
    SPELL_FOG_BREATH            =   45495,   // fel to self, speed burst
    SPELL_FOG_TRIGGER           =   45582,   // fog to self, trigger 45782
    SPELL_FOG_FORCE             =   45782,   // fog to player, force cast 45714
    SPELL_FOG_INFORM            =   45714,   // player let fel cast 45717, script effect
    SPELL_FOG_CHARM             =   45717,   // fel to player
    SPELL_FOG_CHARM2            =   45726,   // link to 45717

    //Other
    SPELL_BERSERK               =   46587,
};

enum Creatures
{
    MOB_FLY_TRIGGER          =   22515,
    MOB_FELMYST              =   25038,
    MOB_UNYIELDING_DEAD      =   25268,
    MOB_MADRIGOSA            =   25160,
    MOB_FELMYST_VISUAL       =   25041,
    MOB_FLIGHT_LEFT          =   25357,
    MOB_FLIGHT_RIGHT         =   25358,
    MOB_FOG_OF_CORRUPTION    =   25266,
    MOB_VAPOR                =   25265,
    MOB_VAPOR_TRAIL          =   25267,
    MOB_KALECGOS             =   25319  // it is the same Kalecgos dragon that used id KJ script
};

enum PhaseFelmyst
{
    PHASE_NULL      = 0,
    PHASE_GROUND    = 1,
    PHASE_FLIGHT    = 2,
    PHASE_RESPAWNING   = 3
};

enum EventFelmyst
{
    EVENT_NULL          =   0,
    EVENT_BERSERK       =   1,
    EVENT_CHECK         =   2,

    EVENT_CLEAVE        =   3,
    EVENT_CORROSION     =   4,
    EVENT_GAS_NOVA      =   5,
    EVENT_ENCAPSULATE   =   6,
    EVENT_FLIGHT        =   7,

    EVENT_FLIGHT_SEQUENCE   =   8,
    EVENT_SUMMON_FOG        =   9
};

#define FELMYST_OOC_PATH    2500

enum Side
{
    LEFT_SIDE = 0,
    RIGHT_SIDE = 1
};

static float FlightMarker[3][2][3] =
{
   //left                     //right
    {{1446.56f, 702.57f, 50.08f}, {1441.64f, 502.52f, 50.08f}},
    {{1469.94f, 704.24f, 50.08f}, {1467.22f, 516.32f, 50.08f}},
    {{1494.76f, 705.00f, 50.08f}, {1492.82f, 515.67f, 50.08f}}
};

static float FlightSide[2][3] =
{
    {1468.38f, 730.27f, 60.08f},   // left
    {1458.17f, 501.3f, 60.08f}     // right
};

static float FallCoord[2][3] =
{
    {1476.30f, 649, 21.5f},     // left
    {1472.55f, 580, 22.5f}     // right
};

static float FogCoords[25][3][3] =
{
     //left side
               //[0]                     [1]                       [2]
    {{1451.98f, 660.00f, 23.05f}, {1472.56f, 680.00f, 22.36f}, {1507.31f, 693.96f, 28.85f}}, //0
    {{1451.98f, 640.00f, 23.06f}, {1472.54f, 660.00f, 20.81f}, {1495.10f, 680.00f, 21.22f}}, //1
    {{1451.98f, 620.00f, 23.06f}, {1472.56f, 640.00f, 21.33f}, {1513.84f, 672.90f, 25.21f}}, //2
    {{1451.98f, 600.00f, 23.06f}, {1472.57f, 620.00f, 22.27f}, {1531.06f, 666.55f, 27.41f}}, //3
    {{1451.98f, 580.00f, 23.06f}, {1472.57f, 600.00f, 23.27f}, {1495.10f, 659.94f, 22.84f}}, //4
    {{1451.98f, 560.00f, 23.06f}, {1472.56f, 580.02f, 22.45f}, {1530.79f, 650.80f, 35.89f}}, //5
    {{1451.98f, 540.00f, 23.06f}, {1472.58f, 560.00f, 22.87f}, {1495.29f, 639.84f, 23.64f}}, //6
    {{                      },    {1451.98f, 560.00f, 23.06f}, {1516.20f, 642.40f, 27.16f}}, //7
    {{                      },    {                      },    {1526.31f, 636.49f, 36.26f}}, //8
    {{                      },    {                      },    {1495.13f, 620.01f, 25.37f}}, //9
    {{                      },    {                      },    {1525.77f, 622.45f, 35.69f}}, //10
    {{                      },    {                      },    {1517.40f, 614.23f, 29.78f}}, //11
    {{                      },    {                      },    {1530.97f, 609.00f, 35.89f}}, //12
    {{                      },    {                      },    {1494.96f, 599.91f, 25.30f}}, //13
    {{                      },    {                      },    {1545.40f, 594.88f, 35.95f}}, //14
    {{                      },    {                      },    {1523.57f, 587.71f, 31.00f}}, //15
    {{                      },    {                      },    {1495.94f, 580.00f, 23.84f}}, //16
    {{                      },    {                      },    {1513.26f, 571.21f, 28.79f}}, //17
    {{                      },    {                      },    {1494.96f, 560.01f, 25.24f}}, //18
    {{                      },    {                      },    {1532.74f, 556.12f, 33.09f}}, //19
    {{                      },    {                      },    {1514.72f, 549.31f, 30.14f}}, //20
    {{                      },    {                      },    {1494.97f, 540.02f, 25.81f}}, //21
    {{                      },    {                      },    {1527.60f, 536.79f, 30.63f}}, //22
    {{                      },    {                      },    {1550.99f, 537.98f, 33.60f}}, //23
    {{                      },    {                      },    {1510.45f, 524.35f, 26.63f}}  //24
     //right side
};

struct boss_felmystAI : public ScriptedAI
{
    boss_felmystAI(Creature *c) : ScriptedAI(c), summons(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;
    PhaseFelmyst Phase;
    EventFelmyst Event;
    Timer _Timer[10];

    SummonList summons;

    uint8 side;
    uint8 path;
    uint8 counter;
    uint32 FlightCount;
    uint32 BreathCount;
    uint32 IntroPhase;
    Timer IntroTimer;

    void Reset()
    {
        Phase = PHASE_NULL;
        Event = EVENT_NULL;
        _Timer[EVENT_BERSERK].Reset(600000);
        _Timer[EVENT_CHECK].Reset(1000);
        FlightCount = 0;
        IntroPhase = 0;
        IntroTimer = 0;
        side = 0;
        path = 0;
        counter = 0;

        me->addUnitState(UNIT_STAT_IGNORE_PATHFINDING);

        me->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 10);
        me->SetFloatValue(UNIT_FIELD_COMBATREACH, 10);
        me->setActive(true);
        me->SetWalk(false);

        if(pInstance)
            pInstance->SetData(DATA_FELMYST_EVENT, NOT_STARTED);

        summons.DespawnAll();   // for any other summons? (should not be needed?)

         me->setActive(false);
    }

    void EnterCombat(Unit *who)
    {
        me->setActive(true);

        DoZoneInCombat();
        RemoveMCAuraIfExist();  // just in case
        EnterPhase(PHASE_GROUND);
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveIdle();
        me->SetSpeed(MOVE_FLIGHT, 2);

        if (!me->HasAura(AURA_NOXIOUS_FUMES))
            me->CastSpell(me, AURA_NOXIOUS_FUMES, true);


        if(Unit* target = SelectUnit(SELECT_TARGET_TOPAGGRO, 0))
        {
            float x, y, z;
            target->GetNearPoint(x, y, z, 0, 2, me->GetAngle(target));
            me->UpdateAllowedPositionZ(x, y, z);
            me->GetMotionMaster()->MovePoint(0, x, y, z);
        }
        else
        {
            EnterEvadeMode();
            return;
        }
        if(pInstance)
        {
            pInstance->SetData(DATA_FELMYST_EVENT, IN_PROGRESS);
            if(Creature* pBrutallus = me->GetCreature(pInstance->GetData64(DATA_BRUTALLUS)))
                pBrutallus->RemoveCorpse();
        }
    }

    void AttackStart(Unit *who)
    {
        if (Phase == PHASE_NULL)
            return;

        if (Phase != PHASE_FLIGHT && Phase != PHASE_RESPAWNING)
            ScriptedAI::AttackStart(who);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (Phase == PHASE_NULL)
            return;

        if(Phase != PHASE_FLIGHT && Phase != PHASE_RESPAWNING)
            ScriptedAI::MoveInLineOfSight(who);
    }

    void RemoveMCAuraIfExist()
    {
        Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers();
        for(Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
        {
            Player* i_pl = i->getSource();
            i_pl->RemoveAurasDueToSpell(SPELL_FOG_CHARM);
            i_pl->RemoveAurasDueToSpell(SPELL_FOG_CHARM2);
        }
    }

    void KilledUnit(Unit* victim)
    {
        if(roll_chance_i(15))
            DoScriptText(RAND(YELL_KILL1, YELL_KILL2), me);
    }

    void JustRespawned()
    {
        Phase = PHASE_RESPAWNING;
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
        me->SetStandState(PLAYER_STATE_SLEEP);
        IntroTimer = 4000;
    }

    void JustDied(Unit* Killer)
    {
        me->SetLevitate(false);
        me->setHover(false);
        DoScriptText(YELL_DEATH, me);
        pInstance->SetData(DATA_FELMYST_EVENT, DONE);
        me->SummonCreature(MOB_KALECGOS, 1555, 737, 88, 0, TEMPSUMMON_TIMED_DESPAWN, 300000);
    }

    void EnterEvadeMode()
    {
        CreatureAI::EnterEvadeMode();
        me->HandleEmoteCommand(EMOTE_ONESHOT_LIFTOFF);
        me->SetSpeed(MOVE_FLIGHT, 1.7, false);
        me->GetMotionMaster()->MovePath(FELMYST_OOC_PATH, true);
        IntroPhase = 6; // to make proper landing on next EnterCombat

        Map::PlayerList const &players = me->GetMap()->GetPlayers();
        for(Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
            if(Player *p = i->getSource())
            {
                if(p->IsAlive() && p->HasAura(SPELL_FOG_CHARM, 0))
                    me->Kill(p, false);
            }
        summons.DespawnAll();
    }

    void JustSummoned(Creature *summon)
    {
        if(summon->GetEntry() == MOB_KALECGOS)
        {
            summon->setActive(true);
            summon->SetLevitate(true);
            summon->setHover(true);
            summon->SetSpeed(MOVE_FLIGHT, 1.2);
            summon->GetMotionMaster()->MovePoint(50, 1471, 632, 37);
        }

        summons.Summon(summon);
    }

    void DamageTaken(Unit*, uint32 &damage)
    {
        if (Phase != PHASE_GROUND && (damage >= me->GetHealth() || me->GetHealth() <= 1))
            damage = 0;
    }

    void EnterPhase(PhaseFelmyst NextPhase)
    {
        switch(NextPhase)
        {
            case PHASE_GROUND:
                _Timer[EVENT_CLEAVE].Reset(urand(5000, 10000));
                _Timer[EVENT_CORROSION].Reset(urand(12000, 20000));
                _Timer[EVENT_GAS_NOVA].Reset(urand(20000, 25000));
                _Timer[EVENT_ENCAPSULATE].Reset(30000);
                _Timer[EVENT_FLIGHT].Reset(60000);
                _Timer[EVENT_CHECK].Reset(1000);
                //DoResetThreat();
                break;
            case PHASE_FLIGHT:
                side = RAND(LEFT_SIDE, RIGHT_SIDE);
                _Timer[EVENT_FLIGHT_SEQUENCE].Reset(1000);
                _Timer[EVENT_SUMMON_FOG].Reset(0);
                _Timer[EVENT_CHECK].Reset(1000);
                FlightCount = 0;
                BreathCount = 0;
                break;
            default:
                break;
        }
        Phase = NextPhase;
    }

    void DoIntro()
    {
        switch(IntroPhase)
        {
            case 0:
                DoScriptText(YELL_BIRTH, me);
                IntroTimer = 1000;
                break;
            case 1:
                me->SetStandState(PLAYER_STATE_NONE);
                IntroTimer = 2000;
                break;
            case 2:
                me->HandleEmoteCommand(EMOTE_ONESHOT_LIFTOFF);
                IntroTimer = 2500;
                break;
            case 3:
                me->SetLevitate(true);
                me->GetMotionMaster()->MovePoint(6, me->GetPositionX()-0.5, me->GetPositionY()-0.5, me->GetPositionZ()+20);
                IntroTimer = 0;
                break;
            case 4:
                me->SetSpeed(MOVE_FLIGHT, 1.7, false);
                me->GetMotionMaster()->MovePath(FELMYST_OOC_PATH, true);
                IntroTimer = 10000;
                break;
            case 5:
                Phase = PHASE_NULL;
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
                IntroTimer = 0;
                break;
        }
        IntroPhase++;     
    }

    void JustReachedHome()
    {
        me->SetLevitate(true);
    }

    void MovementInform(uint32 Type, uint32 Id)
    {
        if(Type == POINT_MOTION_TYPE)
        {
            // stop moving on each waypoint
            me->GetMotionMaster()->MoveIdle();
            if(me->GetVictim()) // cosmetics: to be tested if working
                me->SetInFront(me->GetVictim());
            switch(Id)
            {
                case 0: // on landing after aggroing
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
                    me->SetLevitate(false);
                    me->setHover(false);
                    me->SetSpeed(MOVE_RUN, 2.0,false);
                    IntroTimer = 3000;
                    break;
                case 1: // on starting phase 2
                    me->setHover(true);
                    _Timer[EVENT_FLIGHT_SEQUENCE] = 1000;
                    break;
                case 2: // on left/right side marker
                    me->setHover(true);
                    _Timer[EVENT_FLIGHT_SEQUENCE] = 3000;
                    break;
                case 3: // on path start node
                    me->setHover(true);
                    me->SetSpeed(MOVE_FLIGHT, 3.5, false);
                    _Timer[EVENT_FLIGHT_SEQUENCE] = urand(3000, 4000);
                    break;
                case 4: // on path stop node
                    me->setHover(true);
                    me->SetSpeed(MOVE_FLIGHT, 1.7, false);
                    me->RemoveAurasDueToSpell(SPELL_FOG_BREATH);
                    side = side?LEFT_SIDE:RIGHT_SIDE;
                    BreathCount++;
                    if(BreathCount < 3)
                        FlightCount = 4;
                    else
                        FlightCount = 7;
                    _Timer[EVENT_FLIGHT_SEQUENCE] = 3000;
                    break;
                case 5: // on landing after phase 2
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
                    _Timer[EVENT_FLIGHT_SEQUENCE] = 1500;
                    break;
                case 6:
                    me->setHover(true);
                    IntroTimer = 8000;
                default:
                    break;
            }
        }
    }

    void HandleFlightSequence()
    {
        switch(FlightCount)
        {
        case 0: // fly up
            BreathCount = 0;
            me->HandleEmoteCommand(EMOTE_ONESHOT_LIFTOFF);
            me->SetLevitate(true);
            me->setHover(true);
            DoScriptText(YELL_TAKEOFF, me);
            _Timer[EVENT_FLIGHT_SEQUENCE] = 2000;
            break;
        case 1:
            me->GetMotionMaster()->MovePoint(1, me->GetPositionX()+10, me->GetPositionY(), me->GetPositionZ()+20);
            _Timer[EVENT_FLIGHT_SEQUENCE] = 0;
            break;
        case 2: // summon vapor on player 2 times
        case 3:
            {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 200, true);
            if(target)
                target->CastSpell((Unit*)NULL, SPELL_VAPOR_SUMMON, true);
            else
            {
                EnterEvadeMode();
                return;
            }
            _Timer[EVENT_FLIGHT_SEQUENCE] = 11000;
            break;
            }
        case 4: // go to side left/right marker
            me->SetSpeed(MOVE_FLIGHT, 1.7, false);
            me->GetMotionMaster()->MovePoint(2, FlightSide[side][0], FlightSide[side][1], FlightSide[side][2]);
            _Timer[EVENT_FLIGHT_SEQUENCE] = 0;
            break;
        case 5: // decide path to go breathing
            {
            path = urand(0,2);
            float *pos = FlightMarker[path][side];
            counter = side ? (path ? (path%2 ? 7 : 24) : 6) : 0;
            me->GetMotionMaster()->MovePoint(3, pos[0], pos[1], pos[2]);
            _Timer[EVENT_FLIGHT_SEQUENCE] = 0;
            break;
            }
        case 6: // start fog breath
            {
            float *pos = FlightMarker[path][side?LEFT_SIDE:RIGHT_SIDE];
            me->GetMotionMaster()->MovePoint(4, pos[0], pos[1], pos[2]);
            DoScriptText(EMOTE_BREATH, me);
            AddSpellToCast(me, SPELL_FOG_BREATH);
            _Timer[EVENT_SUMMON_FOG] = 50;
            _Timer[EVENT_FLIGHT_SEQUENCE] = 0;
            break;
            }
        case 7: // start landing..
            if(Unit* target = SelectUnit(SELECT_TARGET_TOPAGGRO, 0))
            {
                float x, y, z;
                target->GetPosition(x, y, z);
                me->GetMotionMaster()->MovePoint(5, x, y, z);
            }
            else
            {
                EnterEvadeMode();
                return;
            }
            _Timer[EVENT_FLIGHT_SEQUENCE] = 0;
            break;
        case 9: // ..and go for phase 1
            me->SetSpeed(MOVE_RUN, 2.0, false);
            me->SetLevitate(false);
            me->SetWalk(false);
            me->setHover(false);
            me->SendHeartBeat();
            EnterPhase(PHASE_GROUND);
            AttackStart(me->GetVictim());
            DoStartMovement(me->GetVictim());
            break;
        default:
            break;
        }
        FlightCount++;
    }

    void ProcessEvent(EventFelmyst Event)
    {
        switch(Event)
        {
            case EVENT_BERSERK:
                DoScriptText(YELL_BERSERK, me);
                ForceSpellCast(me, SPELL_BERSERK, INTERRUPT_AND_CAST_INSTANTLY);
                _Timer[EVENT_BERSERK] = 300000;   // 5 min just in case :)
                break;
            case EVENT_CHECK:
                DoZoneInCombat();
                me->SetSpeed(MOVE_RUN, 2.0, false);
                _Timer[EVENT_CHECK]=1000;
                break;
            case EVENT_CLEAVE:
                AddSpellToCast(me->GetVictim(), SPELL_CLEAVE);
                _Timer[EVENT_CLEAVE] = urand(5000, 10000);
                break;
            case EVENT_CORROSION:
                AddSpellToCast(me->GetVictim(), SPELL_CORROSION);
                _Timer[EVENT_CORROSION] = urand(20000, 30000);
                break;
            case EVENT_GAS_NOVA:
            {
                AddSpellToCastWithScriptText(me, SPELL_GAS_NOVA, YELL_BREATH);
                // gas nova should only be used 2 times in phase 1
                uint32 intval = (_Timer[EVENT_FLIGHT].GetTimeLeft() <= 20000) ? 40000 : urand(20000, 25000);
                _Timer[EVENT_GAS_NOVA].Reset(intval);
                break;
            }
            case EVENT_ENCAPSULATE:
                if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))
                {
                    ClearCastQueue();
                    me->SetSelection(target->GetGUID());
                    AddSpellToCast(target, SPELL_ENCAPSULATE_CHANNEL, false, true);
                    _Timer[EVENT_ENCAPSULATE] = urand(22000, 35000);
                    if(_Timer[EVENT_FLIGHT].GetTimeLeft() < 7000)
                        _Timer[EVENT_FLIGHT].Reset(7000);
                }
                break;
            case EVENT_FLIGHT:
                EnterPhase(PHASE_FLIGHT);
                break;
            case EVENT_FLIGHT_SEQUENCE:
                HandleFlightSequence();
                break;
            case EVENT_SUMMON_FOG:
                float *posFog = FogCoords[counter][path];
                if(Creature *Fog = me->SummonCreature(MOB_FOG_OF_CORRUPTION, posFog[0], posFog[1], posFog[2], 0, TEMPSUMMON_TIMED_DESPAWN, 15000))
                    Fog->CastSpell(Fog, SPELL_FOG_TRIGGER, true);
                if((side && !counter) || (!side && counter == (path ? (path%2 ? 7 : 24) : 6)))
                    _Timer[EVENT_SUMMON_FOG] = 0;
                else
                {
                    side ? counter-- : counter++;
                    _Timer[EVENT_SUMMON_FOG].Reset(6000/(path ? (path%2 ? 8 : 25) : 7));  // check this timer
                }
                break;
        }
    }

    bool UpdateVictim()
    {
        switch (Phase)
        {
            case PHASE_FLIGHT:
            {
                if (me->GetMap()->GetAlivePlayersCountExceptGMs() == 0)
                {
                    EnterEvadeMode();
                    return false;
                }

                return true;
            }
            default:
                 return ScriptedAI::UpdateVictim();
        }
    }

    void UpdateAI(const uint32 diff)
    {

        if (IntroTimer.Expired(diff))
            DoIntro();


        if (!UpdateVictim())
            return;

        DoSpecialThings(diff, DO_PULSE_COMBAT, 400.0f);


        // use enrage timer both phases
        if (_Timer[EVENT_BERSERK].Expired(diff))
            ProcessEvent(EVENT_BERSERK);


        if(Phase == PHASE_GROUND || Phase == PHASE_NULL)
        {
            for(uint32 i = 2; i <= 7; i++)
                if (_Timer[i].Expired(diff))
                    ProcessEvent((EventFelmyst)i);

            CastNextSpellIfAnyAndReady();
            if(Phase == PHASE_GROUND)
                DoMeleeAttackIfReady();
        }

        if(Phase == PHASE_FLIGHT)
        {
            for(uint32 i = 8; i < 10; i++)
                if (_Timer[i].Expired(diff))
                    ProcessEvent((EventFelmyst)i);

            CastNextSpellIfAnyAndReady();
        }
    }
};


// AI for invisible mob that is following player while Felmyst is casting Demonic Vapor on him
struct mob_felmyst_vaporAI : public ScriptedAI
{
    mob_felmyst_vaporAI(Creature *c) : ScriptedAI(c)
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetSpeed(MOVE_RUN, 1.0);
        me->SetFloatValue(UNIT_FIELD_COMBATREACH, 0.01);
        me->SetReactState(REACT_PASSIVE);
    }
    void Reset() {}
    void MoveInLineOfSight(Unit*) {}
    void JustRespawned()
    {
        DoZoneInCombat();
        me->CastSpell(me, SPELL_VAPOR_TRIGGER, false);  // summons 9 trail triggers, so..
        me->CastSpell(me, SPELL_TRAIL_SUMMON, true);    // ..summon one more trail trigger just on spawn
        me->CastSpell((Unit*)NULL, SPELL_VAPOR_FORCE, false);
        me->setFaction(1771);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!me->GetVictim() || !me->GetVictim()->isTargetableForAttack() || me->GetVictim()->GetTypeId() != TYPEID_PLAYER)
            AttackStart(SelectUnit(SELECT_TARGET_NEAREST, 0, 100.0, true));
    }
};

// AI for invisible mob leaving on felmyst vapor trail (summoned by SPELL_VAPOR_FORCE)
struct mob_felmyst_trailAI : public Scripted_NoMovementAI
{
    mob_felmyst_trailAI(Creature *c) : Scripted_NoMovementAI(c)
    {
        pInstance = (c->GetInstanceData());
        me->CastSpell(me, SPELL_TRAIL_TRIGGER, true);
        me->setFaction(1771);
        Delay.Reset(6000);
        Despawn.Reset(20000);
    }

    ScriptedInstance* pInstance;
    Timer Delay;   // timer for Unyielding Dead summoning
    Timer Despawn; // for despawning

    void SpellHitTarget(Unit* target, const SpellEntry *entry)
    {
        if(entry->Id == SPELL_TRAIL_DAMAGE && target->isTargetableForAttack())
            DoCast(me, SPELL_DEAD_SUMMON);
    }

    void JustSummoned(Creature* summon)
    {
        if(summon->AI())
            summon->AI()->DoZoneInCombat();
        if(Unit* Felmyst = me->GetUnit(pInstance->GetData64(DATA_FELMYST)))
            Felmyst->ToCreature()->AI()->JustSummoned(summon);
    }

    void UpdateAI(const uint32 diff)
    {

        if (Delay.Expired(diff))
        {
            DoCast(me, SPELL_DEAD_SUMMON);
            Delay = 30000;  // will despawn sooner
        }



        if (Despawn.Expired(diff))
            me->ForcedDespawn();

    }
};

CreatureAI* GetAI_boss_felmyst(Creature *_Creature)
{
    return new boss_felmystAI(_Creature);
}

CreatureAI* GetAI_mob_felmyst_vapor(Creature *_Creature)
{
    return new mob_felmyst_vaporAI(_Creature);
}

CreatureAI* GetAI_mob_felmyst_trail(Creature *_Creature)
{
    return new mob_felmyst_trailAI(_Creature);
}

void AddSC_boss_felmyst()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_felmyst";
    newscript->GetAI = &GetAI_boss_felmyst;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_felmyst_vapor";
    newscript->GetAI = &GetAI_mob_felmyst_vapor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_felmyst_trail";
    newscript->GetAI = &GetAI_mob_felmyst_trail;
    newscript->RegisterSelf();
}
