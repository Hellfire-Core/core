/*
* Copyright (C) 2009 TrinityCore <http://www.trinitycore.org/>
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

#ifndef BOSS_KILJAEDEN_H
#define BOSS_KILJAEDEN_H

#include "scriptPCH.h"

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
    /* adds spells */
    SPELL_SHADOW_BOLT_VOLLEY                            = 45770,
    SPELL_SHADOW_INFUSION                               = 45772, 
    SPELL_FELFIRE_PORTAL                                = 46875, 
    SPELL_SHADOW_CHANNELING                             = 46757, 
    SPELL_FELFIRE_FISSION                               = 45779, 
    SPELL_SHADOW_BOLT                                   = 45680,

    /* Kil'Jaeden's spells */
    SPELL_TRANS                                         = 23188,
    SPELL_REBIRTH                                       = 44200,
    SPELL_SOUL_FLAY                                     = 45442,
    SPELL_SOUL_FLAY_SLOW                                = 47106,
    SPELL_LEGION_LIGHTNING                              = 45664,
    SPELL_FIRE_BLOOM                                    = 45641,
    SPELL_SUNWELL_KNOCKBACK                             = 40191,
    SPELL_SINISTER_REFLECTION                           = 45785,
    SPELL_SINISTER_REFLECTION_ENLARGE                   = 45893,
    SPELL_SHADOW_SPIKE                                  = 46680,
    SPELL_FLAME_DART                                    = 45737,
    SPELL_DARKNESS_OF_A_THOUSAND_SOULS                  = 46605,
    SPELL_DARKNESS_OF_A_THOUSAND_SOULS_DAMAGE           = 45657,
    SPELL_ARMAGEDDON_TRIGGER                            = 45909,
    SPELL_ARMAGEDDON_VISUAL                             = 45911,
    SPELL_ARMAGEDDON_VISUAL2                            = 45914,

    /* Anveena related spells */
    SPELL_ANVEENA_PRISON                                = 46367,
    SPELL_ANVEENA_ENERGY_DRAIN                          = 46410,
    SPELL_SACRIFICE_OF_ANVEENA                          = 46474,

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

    /*** Other Spells (used by players, etc) ***/
    SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT                  = 45839, // Possess the blue dragon from the orb to help the raid.
    SPELL_ENTROPIUS_BODY                                = 46819, // Visual for Entropius at the Epilogue
    SPELL_RING_OF_BLUE_FLAMES                           = 45825, // Cast this spell when the go is activated
    SPELL_POSSESS_DRAKE_IMMUNE                          = 45838, // Gives immunity to dragon controller
    SPELL_KILL_DRAKES                                   = 46707, // Used on wipe to kill remaining drakes
};

enum CreatureIds
{
    CREATURE_ANVEENA                        = 26046,
    CREATURE_KALECGOS                       = 25319,
    CREATURE_KILJAEDEN                      = 25315,
    CREATURE_HAND_OF_THE_DECEIVER           = 25588,
    CREATURE_FELFIRE_PORTAL                 = 25603,
    CREATURE_VOLATILE_FELFIRE_FIEND         = 25598,
    CREATURE_ARMAGEDDON_TARGET              = 25735,
    CREATURE_SHIELD_ORB                     = 25502,
    CREATURE_POWER_OF_THE_BLUE_DRAGONFLIGHT = 25653,
    CREATURE_SPIKE_TARGET1                  = 30598,
    CREATURE_SINISTER_REFLECTION            = 25708,
    CREATURE_KILJAEDEN_CONTROLLER           = 25608,

    CREATURE_SPIKE_TARGET2                  = 30614, // unused
    CREATURE_PROPHET                        = 26246, // Outro
    CREATURE_THE_CORE_OF_ENTROPIUS          = 26262, // Used in the ending cinematic?
};

enum Phase
{
    PHASE_DECEIVERS     = 1, // Fight adds
    PHASE_NORMAL        = 2, // Kil'Jaeden emerges
    PHASE_DARKNESS      = 3, // 85%
    PHASE_ARMAGEDDON    = 4, // 55%
    PHASE_SACRIFICE     = 5, // 25%
};

//Timers
enum KilJaedenTimers
{
    TIMER_KALEC_JOIN        = 0,
    TIMER_SOUL_FLAY         = 1,
    TIMER_LEGION_LIGHTNING  = 2,
    TIMER_FIRE_BLOOM        = 3,
    TIMER_SUMMON_SHILEDORB  = 4,
    TIMER_SHADOW_SPIKE      = 5,
    TIMER_FLAME_DART        = 6,
    TIMER_DARKNESS          = 7,
    TIMER_ORBS_EMPOWER      = 8,
    TIMER_ARMAGEDDON        = 9,
    TIMER_ANVEENA_SPEECH    = 10,

    TIMER_KJ_MAX
};

// Locations, where Shield Orbs will spawn
static float ShieldOrbLocations[4][2] =
{
    { 1698.900f     , 627.870f},  //middle pont of Sunwell
    {(3.14f * 0.75f), 17.0f   }, 
    {(3.14f * 1.25f), 17.0f   },
    {(3.14f * 1.75f), 17.0f   }
};

#define GAMEOBJECT_ORB_OF_THE_BLUE_DRAGONFLIGHT 188415
#define SHIELD_ORB_Z 45.000f

struct Speech
{
    int32 textid;
    uint32 creature, timer;
};

static Speech Sacrifice[] =
{
    {SAY_KALECGOS_AWAKEN,       CREATURE_KALECGOS,  5000},
    {SAY_ANVEENA_IMPRISONED,    CREATURE_ANVEENA,   4000},
    {SAY_KALECGOS_LETGO,        CREATURE_KALECGOS,  7000},
    {SAY_ANVEENA_LOST,          CREATURE_ANVEENA,   5000},
    {SAY_KALECGOS_FOCUS,        CREATURE_KALECGOS,  7000},
    {SAY_ANVEENA_KALEC,         CREATURE_ANVEENA,   2000},
    {SAY_KALECGOS_FATE,         CREATURE_KALECGOS,  3000},
    {SAY_ANVEENA_GOODBYE,       CREATURE_ANVEENA,   6000},
    {SAY_KALECGOS_GOODBYE,      CREATURE_KALECGOS,  12000},
    {SAY_KJ_PHASE5,             CREATURE_KILJAEDEN, 8000},
    {SAY_KALECGOS_ENCOURAGE,    CREATURE_KALECGOS,  0}
};

#endif