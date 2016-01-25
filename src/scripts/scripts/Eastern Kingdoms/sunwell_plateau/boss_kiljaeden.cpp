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
SDName: Boss_Kiljaeden
SD%Complete: 70
SDComment: Phase4, Phase5, Shadow Spike, Armageddon
SDCategory: Sunwell_Plateau
EndScriptData */

//TODO rewrite Amagedon
//TODO Remove blue visual from Orbs on reset and if it is used

#include "precompiled.h"
#include "def_sunwell_plateau.h"
#include <math.h>

/*** Speech and sounds***/
enum Speeches
{
    // Felmyst outro
    YELL_KALECGOS       = -1580043, //after felmyst's death spawned and say this

    // These are used throughout Sunwell and Magisters(?). Players can hear this while running through the instances.
    SAY_KJ_OFFCOMBAT1   = -1580066,
    SAY_KJ_OFFCOMBAT2   = -1580067,
    SAY_KJ_OFFCOMBAT3   = -1580068,
    SAY_KJ_OFFCOMBAT4   = -1580069,
    SAY_KJ_OFFCOMBAT5   = -1580070,

    // Encounter speech and sounds
    SAY_KJ_EMERGE       = -1580071,
    SAY_KJ_SLAY1        = -1580072,
    SAY_KJ_SLAY2        = -1580073,
    SAY_KJ_REFLECTION1  = -1580074,
    SAY_KJ_REFLECTION2  = -1580075,
    SAY_KJ_DARKNESS1    = -1580076,
    SAY_KJ_DARKNESS2    = -1580077,
    SAY_KJ_DARKNESS3    = -1580078,
    SAY_KJ_PHASE3       = -1580079,
    SAY_KJ_PHASE4       = -1580080,
    SAY_KJ_PHASE5       = -1580081,
    SAY_KJ_DEATH        = -1580093,
    EMOTE_KJ_DARKNESS   = -1580094,

    /*** Kalecgos - Anveena speech at the beginning of Phase 5; Anveena's sacrifice ***/
    SAY_KALECGOS_AWAKEN     = -1580082,
    SAY_ANVEENA_IMPRISONED  = -1580083,
    SAY_KALECGOS_LETGO      = -1580084,
    SAY_ANVEENA_LOST        = -1580085,
    SAY_KALECGOS_FOCUS      = -1580086,
    SAY_ANVEENA_KALEC       = -1580087,
    SAY_KALECGOS_FATE       = -1580088,
    SAY_ANVEENA_GOODBYE     = -1580089,
    SAY_KALECGOS_GOODBYE    = -1580090,
    SAY_KALECGOS_ENCOURAGE  = -1580091,

    /*** Kalecgos says throughout the fight ***/
    SAY_KALECGOS_JOIN       = -1580092,
    SAY_KALEC_ORB_READY1    = -1580095,
    SAY_KALEC_ORB_READY2    = -1580096,
    SAY_KALEC_ORB_READY3    = -1580097,
    SAY_KALEC_ORB_READY4    = -1580098
};

/*** Spells used during the encounter ***/
enum SpellIds
{
    /* Hand of the Deceiver's spells and cosmetics */
    SPELL_SHADOW_BOLT_VOLLEY                            = 45770, // aoe + increases shadow damage taken by 750 for 6s
    SPELL_SHADOW_INFUSION                               = 45772, // They gain this at 20% - Immunity to Stun/Silence and makes them look angry!
    SPELL_FELFIRE_PORTAL                                = 46875, // Creates a portal that spawns Felfire Fiends (LIVE FOR THE SWARM!1 FOR THE OVERMIND!)
    SPELL_SHADOW_CHANNELING                             = 46757, // Channeling animation out of combat

    /* Volatile Felfire Fiend's spells */
    SPELL_FELFIRE_FISSION                               = 45779, // Felfire Fiends explode when they die or get close to target.

    /* Kil'Jaeden's spells and cosmetics */
    SPELL_TRANS                                         = 23188, // Surprisingly, this seems to be the right spell.. (Where is it used?)
    SPELL_REBIRTH                                       = 44200, // Emerge from the Sunwell
    SPELL_SOUL_FLAY                                     = 45442, // 9k Shadow damage over 3 seconds. Spammed throughout all the fight.
    SPELL_SOUL_FLAY_SLOW                                = 47106,
    SPELL_LEGION_LIGHTNING                              = 45664, // Chain Lightning, 4 targets, ~3k Shadow damage, 1.5k mana burn
    SPELL_FIRE_BLOOM                                    = 45641, // Places a debuff on 5 raid members, which causes them to deal 2k Fire damage to nearby allies and selves. MIGHT NOT WORK
    SPELL_SUNWELL_KNOCKBACK                             = 40191, // 10 yd aoe knockback, no damage

    SPELL_SINISTER_REFLECTION                           = 45785, // Summon shadow copies of 5 raid members that fight against KJ's enemies
    SPELL_SINISTER_REFLECTION_ENLARGE                   = 45893, // increases size by 50%

    SPELL_SHADOW_SPIKE                                  = 46680, // Bombard random raid members with Shadow Spikes (Very similar to Void Reaver orbs)
    SPELL_FLAME_DART                                    = 45737, // Bombards the raid with flames every 3(?) seconds
    SPELL_DARKNESS_OF_A_THOUSAND_SOULS                  = 46605, // Begins a 8-second channeling, after which he will deal 50'000 damage to the raid
    SPELL_DARKNESS_OF_A_THOUSAND_SOULS_DAMAGE           = 45657,

    /* Armageddon spells wrong visual */
    SPELL_ARMAGEDDON_TRIGGER                            = 45909, // Meteor spell trigger missile should cast creature on himself
    SPELL_ARMAGEDDON_VISUAL                             = 45911, // Does the hellfire visual to indicate where the meteor missle lands
    SPELL_ARMAGEDDON_VISUAL2                            = 45914, // Does the light visual to indicate where the meteor missle lands
    SPELL_ARMAGEDDON_VISUAL3                            = 24207, // This shouldn't correct but same as seen on the movie
    SPELL_ARMAGEDDON_SUMMON_TRIGGER                     = 45921, // Summons the triggers that cast the spells on himself need random target select
    SPELL_ARMAGEDDON_DAMAGE                             = 45915, // This does the area damage

    /* Shield Orb Spells*/
    SPELL_SHADOW_BOLT                                   = 45680, //45679 would be correct but triggers to often //TODO fix console error


    /* Anveena's spells and cosmetics (Or, generally, everything that has "Anveena" in name) */
    SPELL_ANVEENA_PRISON                                = 46367, // She hovers locked within a bubble
    SPELL_ANVEENA_ENERGY_DRAIN                          = 46410, // Sunwell energy glow animation (Control mob uses this)
    SPELL_SACRIFICE_OF_ANVEENA                          = 46474, // This is cast on Kil'Jaeden when Anveena sacrifices herself into the Sunwell

    /* Sinister Reflection Spells */
    SPELL_SR_CURSE_OF_AGONY                             = 46190,
    SPELL_SR_SHADOW_BOLT                                = 47076,

    SPELL_SR_EARTH_SHOCK                                = 47071,

    SPELL_SR_FIREBALL                                   = 47074,

    SPELL_SR_HEMORRHAGE                                 = 45897,

    SPELL_SR_HOLY_SHOCK                                 = 38921,
    SPELL_SR_HAMMER_OF_JUSTICE                          = 37369,

    SPELL_SR_HOLY_SMITE                                 = 47077,
    SPELL_SR_RENEW                                      = 47079,

    SPELL_SR_SHOOT                                      = 16496,
    SPELL_SR_MULTI_SHOT                                 = 48098,
    SPELL_SR_WING_CLIP                                  = 40652,

    SPELL_SR_WHIRLWIND                                  = 17207,

    SPELL_SR_MOONFIRE                                   = 47072,
    //SPELL_SR_PLAGU STRIKE                             = 58843, Dk Spell!

    /*** Other Spells (used by players, etc) ***/
    SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT                  = 45839, // Possess the blue dragon from the orb to help the raid.
    SPELL_ENTROPIUS_BODY                                = 46819, // Visual for Entropius at the Epilogue
    SPELL_RING_OF_BLUE_FLAMES                           = 45825  //Cast this spell when the go is activated
};

enum CreatureIds
{
    CREATURE_ANVEENA                        = 26046, // Embodiment of the Sunwell
    CREATURE_KALECGOS                       = 25319, // Helps the raid throughout the fight
    CREATURE_PROPHET                        = 26246, // Outro
    CREATURE_KILJAEDEN                      = 25315, // Give it to 'em KJ!
    CREATURE_HAND_OF_THE_DECEIVER           = 25588, // Adds found before KJ emerges
    CREATURE_FELFIRE_PORTAL                 = 25603, // Portal spawned be Hand of the Deceivers
    CREATURE_VOLATILE_FELFIRE_FIEND         = 25598, // Fiends spawned by the above portal
    CREATURE_ARMAGEDDON_TARGET              = 25735, // This mob casts meteor on itself.. I think
    CREATURE_SHIELD_ORB                     = 25502, // Shield orbs circle the room raining shadow bolts on raid
    CREATURE_THE_CORE_OF_ENTROPIUS          = 26262, // Used in the ending cinematic?
    CREATURE_POWER_OF_THE_BLUE_DRAGONFLIGHT = 25653, // NPC that players possess when using the Orb of the Blue Dragonflight
    CREATURE_SPIKE_TARGET1                  = 30598, // Should summon these under Shadow Spike Channel on targets place
    CREATURE_SPIKE_TARGET2                  = 30614,
    CREATURE_SINISTER_REFLECTION            = 25708, // Sinister Relection spawnd on Phase swichtes
    CREATURE_KILJAEDEN_CONTROLLER           = 25608  // controller mob
};

/*** GameObjects ***/
#define GAMEOBJECT_ORB_OF_THE_BLUE_DRAGONFLIGHT 188415

/*** Error messages ***/
#define ERROR_KJ_NOT_SUMMONED "TSCR ERROR: Unable to summon Kil'Jaeden for some reason"

/*** Others ***/
#define FLOOR_Z 28.050388
#define SHIELD_ORB_Z 45.000

enum Phase
{
    PHASE_DECEIVERS     = 1, // Fight 3 adds
    PHASE_NORMAL        = 2, // Kil'Jaeden emerges from the sunwell
    PHASE_DARKNESS      = 3, // At 85%, he gains few abilities; Kalecgos joins the fight
    PHASE_ARMAGEDDON    = 4, // At 55%, he gains even more abilities
    PHASE_SACRIFICE     = 5, // At 25%, Anveena sacrifices herself into the Sunwell; at this point he becomes enraged and has *significally* shorter cooldowns.
};

//Timers
enum KilJaedenTimers
{
    TIMER_KALEC_JOIN = 0,

    //Phase 2 Timer
    TIMER_SOUL_FLAY        = 1,
    TIMER_LEGION_LIGHTNING = 2,
    TIMER_FIRE_BLOOM       = 3,
    TIMER_SUMMON_SHILEDORB = 4,

    //Phase 3 Timer
    TIMER_SHADOW_SPIKE     = 5,
    TIMER_FLAME_DART       = 6,
    TIMER_DARKNESS         = 7,
    TIMER_ORBS_EMPOWER     = 8,

    //Phase 4 Timer
    TIMER_ARMAGEDDON       = 9
};


// Locations of the Hand of Deceiver adds
float DeceiverLocations[3][3] =
{
    {1682.045, 631.299, 5.936},
    {1684.099, 618.848, 0.589},
    {1694.170, 612.272, 1.416},
};

enum Actions
{
    DECEIVER_ENTER_COMBAT = 0,
    DECEIVER_DIED         = 1,
    DECEIVER_RESET        = 2,
    KALEC_RESET_ORBS      = 3
};

// Locations, where Shield Orbs will spawn
float ShieldOrbLocations[4][2] =
{
    {1698.900, 627.870},  //middle pont of Sunwell
    {3.14 * 0.75, 17 }, 
    {3.14 * 1.75, 17 },
    {3.14 * 1.25, 17 }
};

float OrbLocations[4][5] = {
    (1694.48, 674.29,  28.0502, 4.86985),
    (1745.68, 621.823, 28.0505, 2.93777),
    (1704.14, 583.591, 28.1696, 1.59003),
    (1653.12, 635.41,  28.0932, 0.0977725),
};

struct Speech
{
    int32 textid;
    uint32 creature, timer;
};
// TODO: Timers
static Speech Sacrifice[] =
{
    {SAY_KALECGOS_AWAKEN,       CREATURE_KALECGOS,  5000},
    {SAY_ANVEENA_IMPRISONED,    CREATURE_ANVEENA,   5000},
    {SAY_KALECGOS_LETGO,        CREATURE_KALECGOS,  8000},
    {SAY_ANVEENA_LOST,          CREATURE_ANVEENA,   5000},
    {SAY_KALECGOS_FOCUS,        CREATURE_KALECGOS,  7000},
    {SAY_ANVEENA_KALEC,         CREATURE_ANVEENA,   2000},
    {SAY_KALECGOS_FATE,         CREATURE_KALECGOS,  3000},
    {SAY_ANVEENA_GOODBYE,       CREATURE_ANVEENA,   6000},
    {SAY_KALECGOS_GOODBYE,      CREATURE_KALECGOS,  12000},
    {SAY_KJ_PHASE5,             CREATURE_KILJAEDEN, 8000},
    {SAY_KALECGOS_ENCOURAGE,    CREATURE_KALECGOS,  5000}
};

class AllOrbsInGrid
{
public:
    AllOrbsInGrid() {}
    bool operator() (GameObject* go)
    {
        if (go->GetEntry() == GAMEOBJECT_ORB_OF_THE_BLUE_DRAGONFLIGHT)
            return true;
        return false;
    }
};

bool GOUse_go_orb_of_the_blue_flight(Player *plr, GameObject* go)
{
    if (go->GetUInt32Value(GAMEOBJECT_FACTION) == 35)
    {
        ScriptedInstance* pInstance = (go->GetInstanceData());
        float x, y, z, dx, dy, dz;
        Unit* dragon = go->SummonCreature(CREATURE_POWER_OF_THE_BLUE_DRAGONFLIGHT, plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 121000);
        if (dragon) plr->CastSpell(dragon, SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT, true);
        go->SetUInt32Value(GAMEOBJECT_FACTION, 0);
        Unit* Kalec = ((Creature*)Unit::GetUnit(*plr, pInstance->GetData64(DATA_KALECGOS_KJ)));
        go->GetPosition(x, y, z);
        for (uint8 i = 0; i < 4; ++i)
        {
            DynamicObject* Dyn = Kalec->GetDynObject(SPELL_RING_OF_BLUE_FLAMES);
            if (Dyn)
            {
                Dyn->GetPosition(dx, dy, dz);
                if (x == dx && dy == y && dz == z)
                {
                    Dyn->RemoveFromWorld();
                    break;
                }
            }
        }
        go->Refresh();
    }
    return true;
}

//AI for Kalecgos
struct boss_kalecgos_kjAI : public ScriptedAI
{
    boss_kalecgos_kjAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;
    Timer FelmystOutroTimer;

    GameObject* Orb[4];
    uint8 OrbsEmpowered;
    uint8 EmpowerCount;

    bool Searched;


    void Reset()
    {
        if (pInstance->GetData(DATA_EREDAR_TWINS_EVENT) == DONE)
            me->SetVisibility(VISIBILITY_OFF);

        for (uint8 i = 0; i < 4; ++i)
            Orb[i] = NULL;
        FindOrbs();

        FelmystOutroTimer = 0;
        OrbsEmpowered = 0;
        EmpowerCount = 0;
        m_creature->SetLevitate(true);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->setActive(true);
        Searched = false;
    }


    void FindOrbs()
    {
     // std::list<GameObject*> orbList;
        std::list<GameObject*> orbList2;
        AllOrbsInGrid check;
        Hellground::AllGameObjectsInRange objects(me, 100.0f);
    //  Hellground::ObjectListSearcher<GameObject, Hellground::AllGameObjectsInRange> searcher(orbList, objects);
        Hellground::ObjectListSearcher<GameObject, AllOrbsInGrid> searcher(orbList2, check);
        Cell::VisitGridObjects(me, searcher, me->GetMap()->GetVisibilityDistance()); 

        if (orbList2.empty())
             return;
        uint8 i = 0;
        for (std::list<GameObject*>::iterator itr = orbList2.begin(); itr != orbList2.end(); ++itr, ++i)
        {
            Orb[i] = GameObject::GetGameObject(*me, (*itr)->GetGUID());

        }
    }

    void ResetOrbs()
    {
        me->RemoveDynObject(SPELL_RING_OF_BLUE_FLAMES);
        for (uint8 i = 0; i < 4; ++i)
            if (Orb[i])
                Orb[i]->SetUInt32Value(GAMEOBJECT_FACTION, 0);
    }

    void EmpowerOrb(bool all)
    {
        if (!Orb[OrbsEmpowered])
            return;
        uint8 random = rand() % 3;
        if (all)
        {
            me->RemoveDynObject(SPELL_RING_OF_BLUE_FLAMES);
            for (uint8 i = 0; i < 4; ++i)
            {
                if (!Orb[i]) return;
                Orb[i]->CastSpell(me, SPELL_RING_OF_BLUE_FLAMES);
                Orb[i]->SetUInt32Value(GAMEOBJECT_FACTION, 35);
                Orb[i]->setActive(true);
                Orb[i]->Refresh();
            }
        }
        else
        {
            float x, y, z, dx, dy, dz;
            Orb[random]->GetPosition(x, y, z);
            for (uint8 i = 0; i < 4; ++i)
            {
                DynamicObject* Dyn = me->GetDynObject(SPELL_RING_OF_BLUE_FLAMES);
                if (Dyn)
                {
                    Dyn->GetPosition(dx, dy, dz);
                    if (x == dx && dy == y && dz == z)
                    {
                        Dyn->RemoveFromWorld();
                        break;
                    }
                }
            }
            Orb[random]->CastSpell(me, SPELL_RING_OF_BLUE_FLAMES);
            Orb[random]->SetUInt32Value(GAMEOBJECT_FACTION, 35);
            Orb[random]->setActive(true);
            Orb[random]->Refresh();
            ++OrbsEmpowered;
        }
        ++EmpowerCount;

        switch (EmpowerCount)
        {
            case 1: DoScriptText(SAY_KALEC_ORB_READY1, m_creature); break;
            case 2: DoScriptText(SAY_KALEC_ORB_READY2, m_creature); break;
            case 3: DoScriptText(SAY_KALEC_ORB_READY3, m_creature); break;
            case 4: DoScriptText(SAY_KALEC_ORB_READY4, m_creature); break;
        }
    }
    void DoAction(const int32 action)
    {
        if (action == KALEC_RESET_ORBS)
            ResetOrbs();
    }


    void MovementInform(uint32 Type, uint32 Id)
    {
        if (Type == POINT_MOTION_TYPE)
        {
            switch (Id)
            {
                case 50: // felmyst outro speach
                    DoScriptText(YELL_KALECGOS, me);
                    FelmystOutroTimer.Reset(10000);
                    break;
                case 60: // on starting phase 2
                    me->DisappearAndDie();
                    break;
                default:
                    break;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (FelmystOutroTimer.Expired(diff))
        {
            me->SetSpeed(MOVE_FLIGHT, 2.5);
            me->GetMotionMaster()->MovePoint(60, 1547, 531, 161);
            FelmystOutroTimer = 0;
        }

        if (!Searched)
        {
            FindOrbs();
            Searched = true;
        }

        if (OrbsEmpowered == 4) OrbsEmpowered = 0;
    }



};

CreatureAI* GetAI_boss_kalecgos_kj(Creature *_Creature)
{
    return new boss_kalecgos_kjAI(_Creature);
}

//AI for Kil'jaeden
struct boss_kiljaedenAI : public Scripted_NoMovementAI
{
    boss_kiljaedenAI(Creature* c) : Scripted_NoMovementAI(c), Summons(m_creature)
    {
        pInstance = (c->GetInstanceData());
        me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
    }

    ScriptedInstance* pInstance;
    SummonList Summons;
    Creature*  Kalec;
    Unit*      randomPlayer;

    uint8 Phase;
    uint8 ActiveTimers;
    uint8 SpikesLeft; // he releases 9 spikes while casting Shadow Spikes

    Timer _Timer[10];
    Timer WaitTimer;
    Timer StunTimer;
    Timer Emerging;   // we don't want to damage players when emerging, we do this when the fight starts

    /* Boolean */
    bool IsKalecJoined;
    bool IsInDarkness;
    bool TimerIsDeactiveted[10];
    bool IsWaiting;
    bool OrbActivated;
    bool IsEmerging;
    bool IsCastingSpikes;

    void Reset()
    {
        Summons.DespawnAll();
        // TODO: Fix timers
        _Timer[TIMER_KALEC_JOIN].Reset(26000);

        //Phase 2 Timer
        _Timer[TIMER_SOUL_FLAY].Reset(3000);
        _Timer[TIMER_LEGION_LIGHTNING].Reset(10000);
        _Timer[TIMER_FIRE_BLOOM].Reset(13000);
        _Timer[TIMER_SUMMON_SHILEDORB].Reset(12000);

        //Phase 3 Timer
        _Timer[TIMER_SHADOW_SPIKE].Reset(4000);
        _Timer[TIMER_FLAME_DART].Reset(urand(18000, 22000));
        _Timer[TIMER_DARKNESS].Reset(45000);
        _Timer[TIMER_ORBS_EMPOWER].Reset(35000);

        //Phase 4 Timer
        _Timer[TIMER_ARMAGEDDON].Reset(2000);

        ActiveTimers = 5;
        WaitTimer.Reset(1);
        StunTimer.Reset(5000);

        Phase = PHASE_DECEIVERS;

        Emerging.Reset(10000);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

        IsKalecJoined   = false;
        IsInDarkness    = false;
        IsWaiting       = false;
        OrbActivated    = false;
        IsEmerging      = true;

        IsCastingSpikes = false;
        SpikesLeft      = 9;

        Kalec = ((Creature*)Unit::GetUnit(*m_creature, pInstance->GetData64(DATA_KALECGOS_KJ)));
        ChangeTimers(false, 0);
    }

    void MoveInLineOfSight(Unit* who)
    {
        if (who->GetTypeId() != TYPEID_PLAYER)
            return;
        Creature* caster = me->SummonCreature(12999, me->GetPositionX(), me->GetPositionY(), 28.540001, 0, TEMPSUMMON_TIMED_DESPAWN, 1); // we use invisible trigger to avoid animation break
        if (caster)                                                                                                                      // on KJ; can't use controller because he has no skin
            if (me->GetExactDist2d(who->GetPositionX(), who->GetPositionY()) <= 12.0f)                                                   // which causes wrong-cast-height problem (0yd kb)
            {
                if (!IsEmerging)
                {
                    me->DealDamage(who, 475, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_FIRE);
                    me->SendSpellNonMeleeDamageLog(who, SPELL_SUNWELL_KNOCKBACK, 475, SPELL_SCHOOL_MASK_FIRE, 0, 0, false, 0);
                }
                caster->CastSpell(who, SPELL_SUNWELL_KNOCKBACK, false, 0, 0, me->GetGUID());
            }

    }

    void SpellHitTarget(Unit* target, const SpellEntry* spell)
    {    }

    void ChangeTimers(bool status, uint32 WTimer)
    {
        SendDebug("KJD: Change timers %u %u", status, WTimer);
        for (uint8 i = 0; i < 10; ++i)
            TimerIsDeactiveted[i] = status;
        TimerIsDeactiveted[TIMER_KALEC_JOIN] = IsKalecJoined;

        if (WTimer > 0)
        {
            IsWaiting = true;
            WaitTimer.Reset(WTimer);
        }

        if (OrbActivated)
            TimerIsDeactiveted[TIMER_ORBS_EMPOWER] = true;
        if (_Timer[TIMER_SHADOW_SPIKE].GetInterval() == 0)
            TimerIsDeactiveted[TIMER_SHADOW_SPIKE] = true;
        if (Phase == PHASE_SACRIFICE) 
            TimerIsDeactiveted[TIMER_SUMMON_SHILEDORB] = true;
    }

    void JustSummoned(Creature* summoned)
    {
        if (summoned->GetEntry() == CREATURE_ARMAGEDDON_TARGET)
        {
            summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }
        else
        {
            summoned->SetLevel(m_creature->getLevel());
        }
        summoned->setFaction(m_creature->getFaction());
        Summons.Summon(summoned);
    }

    void JustDied(Unit* killer)
    {
        DoScriptText(SAY_KJ_DEATH, m_creature);

        if (pInstance)
            pInstance->SetData(DATA_KILJAEDEN_EVENT, DONE);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_KJ_SLAY1, SAY_KJ_SLAY2), m_creature);
    }

    void EnterEvadeMode()
    {
        Scripted_NoMovementAI::EnterEvadeMode();
        Summons.DespawnAll();
        Kalec->AI()->DoAction(KALEC_RESET_ORBS);

        // Reset the controller
        if (pInstance)
        {
            Creature* Control = ((Creature*)Unit::GetUnit(*m_creature, pInstance->GetData64(DATA_KILJAEDEN_CONTROLLER)));
            if (Control)
                ((Scripted_NoMovementAI*)Control->AI())->EnterEvadeMode();
        }
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat();
    }

    void CastSinisterReflection()
    {
        DoScriptText(RAND(SAY_KJ_REFLECTION1, SAY_KJ_REFLECTION2), m_creature);
        float x, y, z;
        Unit* target;

        target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true);
        target->GetPosition(x, y, z);
        for (uint8 i = 0; i < 4; i++)
        {
            Creature* SinisterReflection = m_creature->SummonCreature(CREATURE_SINISTER_REFLECTION, x - sinf(M_PI/2*i), y - cosf(M_PI/2*i), z, 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
            if (SinisterReflection)
            {
                SinisterReflection->setFaction(me->getFaction());
                SinisterReflection->SetFacingToObject(target);
                SinisterReflection->Attack(target, false);

            }
            if (i == 3)
                target->CastSpell(SinisterReflection, SPELL_SINISTER_REFLECTION, true);
        }


    }

    void UpdateAI(const uint32 diff)
    {
        if (Emerging.Expired(diff))
        {
            Emerging = 0;
            IsEmerging = false;
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            DoScriptText(SAY_KJ_EMERGE, me);
        }

        if (Phase == PHASE_SACRIFICE)
        {
            if (StunTimer.Expired(diff))
            {
                m_creature->clearUnitState(UNIT_STAT_STUNNED);
                StunTimer = 0;
            }
        }

        if (Phase < PHASE_NORMAL || IsEmerging || !UpdateVictim())
            return;

        if (WaitTimer.Expired(diff))
        {
            IsWaiting = false;
            ChangeTimers(false, 0);
        }


        for (uint8 t = 0; t < ActiveTimers; t++)
        {
            if (!TimerIsDeactiveted[t] && _Timer[t].Expired(diff))
            {
                switch (t)
                {
                    case TIMER_KALEC_JOIN:
                    {
                        if (Kalec)
                        {
                            DoScriptText(SAY_KALECGOS_JOIN, Kalec);
                            IsKalecJoined = true;
                            TimerIsDeactiveted[TIMER_KALEC_JOIN] = true;
                        }
                    }
                    break;

                    case TIMER_SOUL_FLAY:
                    {
                        if (me->IsNonMeleeSpellCast(false))
                            break;

                        AddSpellToCast(me->getVictim(), SPELL_SOUL_FLAY);

                        _Timer[TIMER_SOUL_FLAY] = 4000;
                        
                    }
                    break;

                    case TIMER_LEGION_LIGHTNING:
                    {
                        randomPlayer = NULL;
                        randomPlayer = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true);
                        if (randomPlayer)
                            AddSpellToCast(randomPlayer, SPELL_LEGION_LIGHTNING, false, true);
                        else
                            error_log("try to cast SPELL_LEGION_LIGHTNING on invalid target");

                        _Timer[TIMER_LEGION_LIGHTNING] = (Phase == PHASE_SACRIFICE) ? 18000 : urand(13000, 17000); // 18 seconds in PHASE_SACRIFICE
                        _Timer[TIMER_SOUL_FLAY].Reset(3500);
                    }
                    break;

                    case TIMER_FIRE_BLOOM:
                    {
                        randomPlayer = NULL;
                        randomPlayer = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true);
                        if (randomPlayer)
                            AddSpellToCast(randomPlayer, SPELL_FIRE_BLOOM);
                        _Timer[TIMER_FIRE_BLOOM] = (Phase == PHASE_SACRIFICE) ? 25000 : 20000; // 25 seconds in PHASE_SACRIFICE

                    }
                    break;

                    case TIMER_SUMMON_SHILEDORB:
                    {
                        for (uint8 i = 1; i < Phase; ++i)
                        {
                            float sx, sy;
                            sx = ShieldOrbLocations[0][0] + ShieldOrbLocations[i][1]*sin(ShieldOrbLocations[i][0]);
                            sy = ShieldOrbLocations[0][1] + ShieldOrbLocations[i][1]*cos(ShieldOrbLocations[i][0]);
                            m_creature->SummonCreature(CREATURE_SHIELD_ORB, sx, sy, SHIELD_ORB_Z, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 45000);
                            if (m_creature)
                                m_creature->AI()->DoAction(i);
                        }
                        _Timer[TIMER_SUMMON_SHILEDORB] = 30000 + rand() % 30 * 1000; // 30-60seconds cooldown
                    }
                    break;


                    /*$$$$$$$$$$$$$$$$$$$$$$$$$$$
                              Phase 3
                    $$$$$$$$$$$$$$$$$$$$$$$$$$$$*/


                    case TIMER_SHADOW_SPIKE:
                    {
                        if (!IsCastingSpikes)
                        {
                            AddSpellToCast(SPELL_SHADOW_SPIKE, CAST_NULL);
                            _Timer[SPELL_FLAME_DART].Delay(30000);
                            _Timer[SPELL_SOUL_FLAY].Delay(30000);
                            _Timer[SPELL_FIRE_BLOOM].Delay(30000);
                            _Timer[SPELL_LEGION_LIGHTNING].Delay(30000);
                            _Timer[TIMER_SHADOW_SPIKE].Reset(2500);
                            IsCastingSpikes = true;
                        }
                        else
                        {
                            if (SpikesLeft)
                            {
                                Unit* random = SelectUnit(SELECT_TARGET_RANDOM, 0, 100.0f);
                                Position pos;
                                if (random)
                                {
                                    random->GetPosition(pos);
                                    me->SummonCreature(CREATURE_SPIKE_TARGET1, pos.x, pos.y, pos.z, 0, TEMPSUMMON_TIMED_DESPAWN, 2800);
                                }
                                SpikesLeft--;
                                _Timer[TIMER_SHADOW_SPIKE] = 3000;
                                me->SetSelection(0);
                            }
                            else
                            {
                                SpikesLeft = 9;
                                _Timer[TIMER_SHADOW_SPIKE] = 0;
                                TimerIsDeactiveted[TIMER_SHADOW_SPIKE] = true; 
                                IsCastingSpikes = false;
                            }

                        }
                    }
                    break;

                    case TIMER_FLAME_DART:
                    {
                        AddSpellToCast(SPELL_FLAME_DART, CAST_NULL);
                        _Timer[TIMER_FLAME_DART] = urand(18000, 22000);
                    }
                    break;

                    case TIMER_DARKNESS:
                    {
                        // Begins to channel for 8 seconds, then deals 50'000 damage to all raid members.
                        if (!IsInDarkness)
                        {
                            SendDebug("Channeling darkness");
                            DoScriptText(EMOTE_KJ_DARKNESS, m_creature);
                            DoCast(m_creature, SPELL_DARKNESS_OF_A_THOUSAND_SOULS, false);
                            ChangeTimers(true, 9000);
                            _Timer[TIMER_DARKNESS] = 8750;
                            TimerIsDeactiveted[TIMER_DARKNESS] = false;
                            if (Phase == PHASE_SACRIFICE)
                                TimerIsDeactiveted[TIMER_ARMAGEDDON] = false;
                            IsInDarkness = true;
                        }
                        else
                        {
                            _Timer[TIMER_DARKNESS] = (Phase == PHASE_SACRIFICE) ? 20000 + rand() % 15000 : 40000 + rand() % 30000;
                            IsInDarkness = false;
                            DoCast(NULL, SPELL_DARKNESS_OF_A_THOUSAND_SOULS_DAMAGE);

                            DoScriptText(RAND(SAY_KJ_DARKNESS1, SAY_KJ_DARKNESS2, SAY_KJ_DARKNESS3), m_creature);
                            SendDebug("Casting aoe darkness");
                        }
                        _Timer[TIMER_SOUL_FLAY].Reset(9000);
                    }
                    break;

                    case TIMER_ORBS_EMPOWER:
                    {
                        SendDebug("Sending orbs empower");
                        if (Phase == PHASE_SACRIFICE)
                        {
                            if (Kalec)((boss_kalecgos_kjAI*)Kalec->AI())->EmpowerOrb(true);
                        }
                        else if (Kalec)((boss_kalecgos_kjAI*)Kalec->AI())->EmpowerOrb(false);

                        _Timer[TIMER_ORBS_EMPOWER] = (Phase == PHASE_SACRIFICE) ? 45000 : 35000;

                        OrbActivated = true;
                        TimerIsDeactiveted[TIMER_ORBS_EMPOWER] = true;
                    }
                    break;



                    /*$$$$$$$$$$$$$$$$$$$$$$$$$$$
                             Phase 4
                    $$$$$$$$$$$$$$$$$$$$$$$$$$$$*/


                    case TIMER_ARMAGEDDON:
                    {
                        Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true);
                        if (target)
                        {
                            float x, y, z;
                            target->GetPosition(x, y, z);
                            m_creature->SummonCreature(CREATURE_ARMAGEDDON_TARGET, x, y, z, 0, TEMPSUMMON_TIMED_DESPAWN, 15000);
                        }
                        _Timer[TIMER_ARMAGEDDON] = 2000; // No, I'm not kidding
                    }
                    break;
                }
                break;
            }
        }

        //Phase 3
        if (Phase == PHASE_NORMAL && ((m_creature->GetHealth() * 100 / m_creature->GetMaxHealth()) < 85))
        {
            CastSinisterReflection();
            DoScriptText(SAY_KJ_PHASE3, m_creature);
            Phase = PHASE_DARKNESS;
            OrbActivated = false;
            ActiveTimers = 9;
            SendDebug("Entering phase 3");
        }

        //Phase 4
        if (Phase == PHASE_DARKNESS && ((m_creature->GetHealth() * 100 / m_creature->GetMaxHealth()) < 55))
        {
            DoScriptText(SAY_KJ_PHASE4, m_creature);
            Phase = PHASE_ARMAGEDDON;
            OrbActivated = false;
            ActiveTimers = 10;
            SendDebug("Entering phase 4");
        }

        //Phase 5 specific spells all we can
        if (Phase == PHASE_ARMAGEDDON && ((m_creature->GetHealth() * 100 / m_creature->GetMaxHealth()) < 25))
        {
            Phase = PHASE_SACRIFICE;
            Creature* Anveena = Unit::GetCreature((*m_creature), pInstance->GetData64(DATA_ANVEENA));
            if (Anveena)
                Anveena->CastSpell(m_creature, SPELL_SACRIFICE_OF_ANVEENA, false);
            OrbActivated = false;
            ChangeTimers(true, 10000);// He shouldn't cast spells for ~10 seconds after Anveena's sacrifice. This will be done within Anveena's script
            m_creature->addUnitState(UNIT_STAT_STUNNED);
            SendDebug("Entering phase 5");
        }

        CastNextSpellIfAnyAndReady();

    }

    void GetDebugInfo(ChatHandler& reader)
    {
        std::ostringstream str;
        str << "KJD Debugai, phase " << (int)Phase << "; OrbActivated " << (int)OrbActivated << "; ActiveTimers " << (int)ActiveTimers << "\n";
        for (uint8 i = 0; i < 10; i++)
            str << "Timer " << (int)i << " : " << (int)_Timer[i].GetTimeLeft() << "\n";
        str << "WaitTimer : " << (int)WaitTimer.GetTimeLeft();
        reader.SendSysMessage(str.str().c_str());
    }
};

CreatureAI* GetAI_boss_kiljaeden(Creature *_Creature)
{
    return new boss_kiljaedenAI(_Creature);
}

//AI for Kil'jaeden Event Controller
struct mob_kiljaeden_controllerAI : public Scripted_NoMovementAI
{
    mob_kiljaeden_controllerAI(Creature* c) : Scripted_NoMovementAI(c), Summons(m_creature)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;
    Creature* KalecKJ;
    SummonList Summons;

    bool SummonedAnveena;
    bool KiljaedenDeath;

    Timer RandomSayTimer;
    Timer CheckDeceivers;

    uint32 Phase;

    void Reset()
    {
        KalecKJ = Unit::GetCreature((*m_creature), pInstance->GetData64(DATA_KALECGOS_KJ));
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->addUnitState(UNIT_STAT_STUNNED);

        pInstance->SetData(DATA_KILJAEDEN_EVENT, NOT_STARTED);
        pInstance->SetData(DATA_HAND_OF_DECEIVER_COUNT, 1);
        Phase = PHASE_DECEIVERS;
        //if(KalecKJ)((boss_kalecgos_kjAI*)KalecKJ->AI())->ResetOrbs();
        //DeceiverDeathTimer = 0;
        SummonedAnveena = false;
        KiljaedenDeath = false;
        RandomSayTimer.Reset(30000);
        CheckDeceivers.Reset(1000);
        Summons.DespawnAll();
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void EnterCombat(Unit* who)
    {
        pInstance->SetData(DATA_KILJAEDEN_EVENT, IN_PROGRESS);
        DoZoneInCombat(1000.0f);
    }

    void JustSummoned(Creature* summoned)
    {
        switch (summoned->GetEntry())
        {
            case CREATURE_ANVEENA:
                summoned->SetLevitate(true);
                summoned->CastSpell(summoned, SPELL_ANVEENA_PRISON, true);
                summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                break;
            case CREATURE_KILJAEDEN:
                summoned->CastSpell(summoned, SPELL_REBIRTH, false);
                ((boss_kiljaedenAI*)summoned->AI())->Phase = PHASE_NORMAL;
                summoned->AddThreat(m_creature->getVictim(), 0.0f);
                summoned->AI()->DoZoneInCombat();
                break;
        }
        Summons.Summon(summoned);
    }

    void DoRandomSay(uint32 diff)
    {    // do not yell when any encounter in progress
        if (pInstance->IsEncounterInProgress())
            return;

        if (!pInstance->IsEncounterInProgress() && pInstance->GetData(DATA_MURU_EVENT) != DONE && RandomSayTimer.Expired(diff))
        {
            DoScriptText(RAND(SAY_KJ_OFFCOMBAT1, SAY_KJ_OFFCOMBAT2, SAY_KJ_OFFCOMBAT3, SAY_KJ_OFFCOMBAT4, SAY_KJ_OFFCOMBAT5), m_creature);
            RandomSayTimer = 60000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!pInstance)
            return;

        DoRandomSay(diff);

        if (!SummonedAnveena)
        {
            Unit* Anveena = DoSpawnCreature(CREATURE_ANVEENA, 0, 0, 40, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
            if (Anveena)
                Anveena->setHover(true);
            DoCast(m_creature, SPELL_ANVEENA_ENERGY_DRAIN);
            SummonedAnveena = true;
        }

        if (CheckDeceivers.Expired(diff))
        {
            CheckDeceivers = 1000;
            if (pInstance->GetData(DATA_HAND_OF_DECEIVER_COUNT) == 0 && pInstance->GetData(DATA_KILJAEDEN_EVENT) == IN_PROGRESS && Phase == PHASE_DECEIVERS)
            {
                m_creature->RemoveAurasDueToSpell(SPELL_ANVEENA_ENERGY_DRAIN);
                Phase = PHASE_NORMAL;
                DoSpawnCreature(CREATURE_KILJAEDEN, 0, 0, 0, 3.886, TEMPSUMMON_MANUAL_DESPAWN, 0);
                CheckDeceivers = 0;
            }
        }

        if (me->getThreatManager().isThreatListEmpty() && me->isInCombat())
            EnterEvadeMode(); // we use this instead of UpdateVictim()


    }
};

CreatureAI* GetAI_mob_kiljaeden_controller(Creature *_Creature)
{
    return new mob_kiljaeden_controllerAI(_Creature);
}

//AI for Hand of the Deceiver
struct mob_hand_of_the_deceiverAI : public ScriptedAI
{
    mob_hand_of_the_deceiverAI(Creature* c) : ScriptedAI(c), Summons(m_creature)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    Timer ShadowBoltVolleyTimer;
    Timer FelfirePortalTimer;
    SummonList Summons;

    void Reset()
    {
        if (!me->IsInEvadeMode())
            me->CastSpell(me, SPELL_SHADOW_CHANNELING, false);

        ShadowBoltVolleyTimer.Reset(1000 + urand(0, 3000)); // So they don't all cast it in the same moment.
        FelfirePortalTimer.Reset(20000);
        Summons.DoAction(0, DECEIVER_RESET);
        Summons.DespawnAll();
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->setFaction(m_creature->getFaction());
        summoned->SetLevel(m_creature->getLevel());
        summoned->AI()->DoZoneInCombat();
        Summons.Summon(summoned);
    }

    void JustReachedHome()
    {
        me->CastSpell(me, SPELL_SHADOW_CHANNELING, false);
    }

    void EnterCombat(Unit* who)
    {
        if (pInstance)
        {
            //pInstance->SetData(DATA_KILJAEDEN_EVENT, IN_PROGRESS);
            Creature* Control = ((Creature*)Unit::GetUnit(*m_creature, pInstance->GetData64(DATA_KILJAEDEN_CONTROLLER)));
            if (Control)
            {
                Control->AI()->EnterCombat(who);
                Control->AddThreat(who, 0.0f);
                Control->AI()->DoZoneInCombat();
            }
        }
        m_creature->InterruptNonMeleeSpells(true);
    }

    void EnterEvadeMode()
    {        
        if (!_EnterEvadeMode())
            return;

        float x, y, z, o;
        m_creature->GetRespawnCoord(x, y, z, &o);
        m_creature->SetHomePosition(x, y, z, o);
        me->GetMotionMaster()->MoveTargetedHome();
        Reset();
    }

    void JustDied(Unit* killer)
    {
        if (!pInstance)
            return;

        if (!me->GetMap()->GetCreatureById(CREATURE_HAND_OF_THE_DECEIVER, GET_ALIVE_CREATURE_GUID))
        {
            pInstance->SetData(DATA_HAND_OF_DECEIVER_COUNT, 0);
        }
    }


    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        // Gain Shadow Infusion at 20% health
        if (((m_creature->GetHealth() * 100 / m_creature->GetMaxHealth()) < 20) && !m_creature->HasAura(SPELL_SHADOW_INFUSION, 0))
            ForceSpellCast(me, SPELL_SHADOW_INFUSION, INTERRUPT_AND_CAST_INSTANTLY, true);

        // Shadow Bolt Volley - Shoots Shadow Bolts at all enemies within 30 yards, for ~2k Shadow damage.
        if (ShadowBoltVolleyTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_SHADOW_BOLT_VOLLEY, CAST_TANK);
            ShadowBoltVolleyTimer = urand(1000, 5000);
        }

        // Felfire Portal - Creatres a portal, that spawns Volatile Felfire Fiends, which do suicide bombing.
        if (FelfirePortalTimer.Expired(diff))
        {
            DoSpawnCreature(CREATURE_FELFIRE_PORTAL, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 20000);
            FelfirePortalTimer = 20000;
        }

        
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_hand_of_the_deceiver(Creature *_Creature)
{
    return new mob_hand_of_the_deceiverAI(_Creature);
}

//AI for Felfire Portal
struct mob_felfire_portalAI : public Scripted_NoMovementAI
{
    mob_felfire_portalAI(Creature* c) : Scripted_NoMovementAI(c), Summons(m_creature) {}

    Timer SpawnFiendTimer;

    SummonList Summons;

    void Reset()
    {
        SpawnFiendTimer = 3000;
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->setFaction(m_creature->getFaction());
        summoned->SetLevel(m_creature->getLevel());
        summoned->AI()->DoZoneInCombat();
        Summons.Summon(summoned);
    }
    void DoAction(const int32 action)
    {
        if (action == DECEIVER_RESET)
            Summons.DespawnAll();
    }



    void UpdateAI(const uint32 diff)
    {
        if (SpawnFiendTimer.Expired(diff))
        {
            Creature* Fiend = DoSpawnCreature(CREATURE_VOLATILE_FELFIRE_FIEND, 0, 0, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 20000);
            SpawnFiendTimer = 3000;
        }
    }
};

CreatureAI* GetAI_mob_felfire_portal(Creature *_Creature)
{
    return new mob_felfire_portalAI(_Creature);
}

//AI for Felfire Fiend
struct mob_volatile_felfire_fiendAI : public ScriptedAI
{
    mob_volatile_felfire_fiendAI(Creature* c) : ScriptedAI(c) {}

    Timer ExplodeTimer;
    Timer WaitTimer;

    bool LockedTarget;

    void Reset()
    {
        WaitTimer.Reset(1500);
        ExplodeTimer.Reset(5000);
        LockedTarget = false;
        me->SetReactState(REACT_PASSIVE);
    }


    void UpdateAI(const uint32 diff)
    {
        if (WaitTimer.Expired(diff))
        {
            WaitTimer = 0;
            me->SetReactState(REACT_AGGRESSIVE);
            DoZoneInCombat(100.0f);
        }

        if ((WaitTimer.GetInterval() && !WaitTimer.Passed()) || !UpdateVictim())
            return;


        if (!LockedTarget)
        {
            Unit* random = SelectUnit(SELECT_TARGET_RANDOM, 0, 28.0f, true, false);
            if (random)
            {
                me->AddThreat(random, 10000.0f);
                LockedTarget = true;
            }
        }

        if (ExplodeTimer.Expired(diff) || me->GetDistance(me->getVictim()) < 9.0f) // Explode if it's close enough to it's target
        {
            DoCast(m_creature->getVictim(), SPELL_FELFIRE_FISSION, true);
            me->DisappearAndDie();
        }
    }
};

CreatureAI* GetAI_mob_volatile_felfire_fiend(Creature *_Creature)
{
    return new mob_volatile_felfire_fiendAI(_Creature);
}

//AI for Armageddon target
struct mob_armageddonAI : public Scripted_NoMovementAI
{
    mob_armageddonAI(Creature* c) : Scripted_NoMovementAI(c) {}

    uint8 Spell;
    uint32 Timer;

    void Reset()
    {
        Spell = 0;
        Timer = 0;
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (Timer <= diff)
        {
            switch (Spell)
            {
                case 0:
                    DoCast(m_creature, SPELL_ARMAGEDDON_VISUAL, true);
                    ++Spell;
                    break;
                case 1:
                    DoCast(m_creature, SPELL_ARMAGEDDON_VISUAL2, true);
                    Timer = 9000;
                    ++Spell;
                    break;
                case 2:
                    DoCast(m_creature, SPELL_ARMAGEDDON_TRIGGER, true);
                    ++Spell;
                    Timer = 5000;
                    break;
                case 3:
                    m_creature->DealDamage(m_creature, m_creature->GetMaxHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                    m_creature->RemoveCorpse();
                    break;
            }
        }
        else Timer -= diff;
    }
};

CreatureAI* GetAI_mob_armageddon(Creature *_Creature)
{
    return new mob_armageddonAI(_Creature);
}

//AI for Shield Orbs
struct mob_shield_orbAI : public ScriptedAI
{
    mob_shield_orbAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    bool PointReached;
    bool Clockwise;
    Timer _Timer;
    Timer CheckTimer;
    ScriptedInstance* pInstance;
    float x, y, r, c, mx, my;

    void Reset()
    {
        me->SetIgnoreVictimSelection(true);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
        me->SetSelection(0);
        m_creature->SetLevitate(true);
        PointReached = true;
        _Timer.Reset(500 + rand() % 500);
        CheckTimer.Reset(1000);
        mx = ShieldOrbLocations[0][0];
        my = ShieldOrbLocations[0][1];
        Clockwise = true;
    }

    void DoAction(const int32 act)
    {
        c = ShieldOrbLocations[act][0];
        r = ShieldOrbLocations[act][1];
        if (act == 1) Clockwise = false;
    }

    void UpdateAI(const uint32 diff)
    {
        if (me->GetSelection())
            me->SetSelection(0);
        if (PointReached)
        {
            y = my + r * cos(c);
            x = mx + r * sin(c);
            PointReached = false;
            m_creature->GetMotionMaster()->MovePoint(1, x, y, SHIELD_ORB_Z);
            if (Clockwise)
                c -= M_PI / 50;
            else
                c += M_PI / 50;

            if (c > 2 * M_PI)
                c -= 2 * M_PI;
            if (c < 0)
                c += 2 * M_PI;
        }

        

        if (_Timer.Expired(diff))
        {
            ForceSpellCast(SPELL_SHADOW_BOLT, CAST_RANDOM, INTERRUPT_AND_CAST_INSTANTLY, true);
            _Timer = 500; // 2 per second
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type != POINT_MOTION_TYPE)
            return;

        PointReached = true;
    }
};

CreatureAI* GetAI_mob_shield_orb(Creature *_Creature)
{
    return new mob_shield_orbAI(_Creature);
}

//AI for Sinister Reflection
struct mob_sinster_reflectionAI : public ScriptedAI
{
    mob_sinster_reflectionAI(Creature* c) : ScriptedAI(c) {}

    uint8 Class;
    Timer _Timer[3];
    Timer Wait;

    void Reset()
    {
        _Timer[0].Reset(1);
        _Timer[1].Reset(1);
        _Timer[2].Reset(1);
        Wait.Reset(5000);
        Class = 0;
        m_creature->addUnitState(UNIT_STAT_CANNOT_AUTOATTACK);
        m_creature->addUnitState(UNIT_STAT_NOT_MOVE);
    }

    void UpdateAI(const uint32 diff)
    {
        
        if (Class == 0 && me->getVictim())
        {
            me->CastSpell(me, SPELL_SINISTER_REFLECTION_ENLARGE, true);
            Class = m_creature->getVictim()->getClass();
            switch (Class)
            {
                case CLASS_DRUID:
                    break;
                case CLASS_HUNTER:
                    break;
                case CLASS_MAGE:
                    break;
                case CLASS_WARLOCK:
                    break;
                case CLASS_WARRIOR:
                    m_creature->SetCanDualWield(true);
                    break;
                case CLASS_PALADIN:
                    break;
                case CLASS_PRIEST:
                    break;
                case CLASS_SHAMAN:
                    m_creature->SetCanDualWield(true);
                    break;
                case CLASS_ROGUE:
                    m_creature->SetCanDualWield(true);
                    break;
            }
        }

        if (Wait.Expired(diff))
        {
            m_creature->clearUnitState(UNIT_STAT_CANNOT_AUTOATTACK);
            m_creature->clearUnitState(UNIT_STAT_NOT_MOVE);
            Wait = 0;
        }

        if (Wait.GetTimeLeft())
            return;

        if (!UpdateVictim())
            return;

        switch (Class)
        {
            case CLASS_DRUID:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->getVictim(), SPELL_SR_MOONFIRE, false);
                    _Timer[1] = 3000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_HUNTER:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->getVictim(), SPELL_SR_MULTI_SHOT, false);
                    _Timer[1] = 9000;
                }
                if (_Timer[2].Expired(diff))
                {
                    DoCast(m_creature->getVictim(), SPELL_SR_SHOOT, false);
                    _Timer[2] = 5000;
                }
                if (m_creature->IsWithinMeleeRange(m_creature->getVictim(), 6))
                {
                    if (_Timer[3].Expired(diff))
                    {
                        DoCast(m_creature->getVictim(), SPELL_SR_MULTI_SHOT, false);
                        _Timer[3] = 7000;
                    }
                    DoMeleeAttackIfReady();
                }
                break;
            case CLASS_MAGE:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->getVictim(), SPELL_SR_FIREBALL, false);
                    _Timer[1] = 3000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_WARLOCK:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->getVictim(), SPELL_SR_SHADOW_BOLT, false);
                    _Timer[1] = 4000;
                }
                if (_Timer[2].Expired(diff))
                {
                    if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true))
                        DoCast(target, SPELL_SR_CURSE_OF_AGONY, true);
                    _Timer[2] = 3000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_WARRIOR:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->getVictim(), SPELL_SR_WHIRLWIND, false);
                    _Timer[1] = 10000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_PALADIN:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->getVictim(), SPELL_SR_HAMMER_OF_JUSTICE, false);
                    _Timer[1] = 7000;
                }
                if (_Timer[2].Expired(diff))
                {
                    DoCast(m_creature->getVictim(), SPELL_SR_HOLY_SHOCK, false);
                    _Timer[2] = 3000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_PRIEST:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->getVictim(), SPELL_SR_HOLY_SMITE, false);
                    _Timer[1] = 5000;
                }
                if (_Timer[2].Expired(diff))
                {
                    DoCast(m_creature, SPELL_SR_RENEW, false);
                    _Timer[2] = 7000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_SHAMAN:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->getVictim(), SPELL_SR_EARTH_SHOCK, false);
                    _Timer[1] = 5000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_ROGUE:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->getVictim(), SPELL_SR_HEMORRHAGE, true);
                    _Timer[1] = 5000;
                }
                DoMeleeAttackIfReady();
                break;
        }
    }

};

CreatureAI* GetAI_mob_sinster_reflection(Creature *_Creature)
{
    return new mob_sinster_reflectionAI(_Creature);
}

void AddSC_boss_kiljaeden()
{
    Script* newscript;

    newscript = new Script;
    newscript->pGOUse = &GOUse_go_orb_of_the_blue_flight;
    newscript->Name = "go_orb_of_the_blue_flight";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_boss_kalecgos_kj;
    newscript->Name = "boss_kalecgos_kj";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_boss_kiljaeden;
    newscript->Name = "boss_kiljaeden";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_kiljaeden_controller;
    newscript->Name = "mob_kiljaeden_controller";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_hand_of_the_deceiver;
    newscript->Name = "mob_hand_of_the_deceiver";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_felfire_portal;
    newscript->Name = "mob_felfire_portal";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_volatile_felfire_fiend;
    newscript->Name = "mob_volatile_felfire_fiend";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_armageddon;
    newscript->Name = "mob_armageddon";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_shield_orb;
    newscript->Name = "mob_shield_orb";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_sinster_reflection;
    newscript->Name = "mob_sinster_reflection";
    newscript->RegisterSelf();
};
