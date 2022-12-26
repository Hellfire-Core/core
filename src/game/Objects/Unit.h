/*
 * Copyright (C) 2005-2008 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2017 MaNGOSOne <https://github.com/mangos/one>
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

#ifndef _UNIT_H
#define _UNIT_H

#include "Common.h"
#include "Object.h"
#include "Opcodes.h"
#include "SpellAuraDefines.h"
#include "UpdateFields.h"
#include "SharedDefines.h"
#include "ThreatManager.h"
#include "HostileRefManager.h"
#include "FollowerReference.h"
#include "FollowerRefManager.h"
#include "Utilities/EventProcessor.h"
#include "StateMgr.h"
#include "MotionMaster.h"
#include "DBCStructure.h"
#include <list>

#include "WorldPacket.h"

#define WORLD_TRIGGER   12999

enum SpellModOp
{
    SPELLMOD_DAMAGE                 = 0,
    SPELLMOD_DURATION               = 1,
    SPELLMOD_THREAT                 = 2,
    SPELLMOD_EFFECT1                = 3,
    SPELLMOD_CHARGES                = 4,
    SPELLMOD_RANGE                  = 5,
    SPELLMOD_RADIUS                 = 6,
    SPELLMOD_CRITICAL_CHANCE        = 7,
    SPELLMOD_ALL_EFFECTS            = 8,
    SPELLMOD_NOT_LOSE_CASTING_TIME  = 9,
    SPELLMOD_CASTING_TIME           = 10,
    SPELLMOD_COOLDOWN               = 11,
    SPELLMOD_EFFECT2                = 12,
    // spellmod 13 unused
    SPELLMOD_COST                   = 14,
    SPELLMOD_CRIT_DAMAGE_BONUS      = 15,
    SPELLMOD_RESIST_MISS_CHANCE     = 16,
    SPELLMOD_JUMP_TARGETS           = 17,
    SPELLMOD_CHANCE_OF_SUCCESS      = 18,
    SPELLMOD_ACTIVATION_TIME        = 19,
    SPELLMOD_EFFECT_PAST_FIRST      = 20,
    SPELLMOD_CASTING_TIME_OLD       = 21,
    SPELLMOD_DOT                    = 22,
    SPELLMOD_EFFECT3                = 23,
    SPELLMOD_SPELL_BONUS_DAMAGE     = 24,
    // spellmod 25, 26 unused
    SPELLMOD_MULTIPLE_VALUE         = 27,
    SPELLMOD_RESIST_DISPEL_CHANCE   = 28
};

#define MAX_SPELLMOD 32

enum SpellValueMod
{
    SPELLVALUE_BASE_POINT0,
    SPELLVALUE_BASE_POINT1,
    SPELLVALUE_BASE_POINT2,
    SPELLVALUE_MAX_TARGETS,
};

typedef std::pair<SpellValueMod, int32>     CustomSpellValueMod;
class CustomSpellValues : public std::vector<CustomSpellValueMod>
{
    public:
        void AddSpellMod(SpellValueMod mod, int32 value)
        {
            push_back(std::make_pair(mod, value));
        }
};

enum SpellFacingFlags
{
    SPELL_FACING_FLAG_INFRONT = 0x0001
};

#define BASE_MINDAMAGE 1.0f
#define BASE_MAXDAMAGE 2.0f
#define BASE_ATTACK_TIME 2000

// No orientation check for auto attacks or spells below this distance.
#define NO_FACING_CHECKS_DISTANCE 1.4f

enum UnitBytes0Offsets : uint8
{
    UNIT_BYTES_0_OFFSET_RACE       = 0,
    UNIT_BYTES_0_OFFSET_CLASS      = 1,
    UNIT_BYTES_0_OFFSET_GENDER     = 2,
    UNIT_BYTES_0_OFFSET_POWER_TYPE = 3,
};

enum UnitBytes1Offsets : uint8
{
    UNIT_BYTES_1_OFFSET_STAND_STATE     = 0,
    UNIT_BYTES_1_OFFSET_PET_LOYALTY     = 1,
    UNIT_BYTES_1_OFFSET_VIS_FLAGS       = 2,
    UNIT_BYTES_1_OFFSET_MISC_FLAGS      = 3,
};

enum UnitBytes2Offsets : uint8
{
    UNIT_BYTES_2_OFFSET_SHEATH_STATE = 0,
    UNIT_BYTES_2_OFFSET_DEBUFF_LIMIT = 1,
    UNIT_BYTES_2_OFFSET_PET_FLAGS    = 2,
    UNIT_BYTES_2_OFFSET_SHAPESHIFT   = 3,
};

// high byte (3 from 0..3) of UNIT_FIELD_BYTES_2
enum ShapeshiftForm
{
    FORM_NONE               = 0x00,
    FORM_CAT                = 0x01,
    FORM_TREE               = 0x02,
    FORM_TRAVEL             = 0x03,
    FORM_AQUA               = 0x04,
    FORM_BEAR               = 0x05,
    FORM_AMBIENT            = 0x06,
    FORM_GHOUL              = 0x07,
    FORM_DIREBEAR           = 0x08,
    FORM_CREATUREBEAR       = 0x0E,
    FORM_CREATURECAT        = 0x0F,
    FORM_GHOSTWOLF          = 0x10,
    FORM_BATTLESTANCE       = 0x11,
    FORM_DEFENSIVESTANCE    = 0x12,
    FORM_BERSERKERSTANCE    = 0x13,
    FORM_TEST               = 0x14,
    FORM_ZOMBIE             = 0x15,
    FORM_FLIGHT_EPIC        = 0x1B,
    FORM_SHADOW             = 0x1C,
    FORM_FLIGHT             = 0x1D,
    FORM_STEALTH            = 0x1E,
    FORM_MOONKIN            = 0x1F,
    FORM_SPIRITOFREDEMPTION = 0x20
};

// low byte (0 from 0..3) of UNIT_FIELD_BYTES_2
enum SheathState
{
    SHEATH_STATE_UNARMED  = 0,                              // non prepared weapon
    SHEATH_STATE_MELEE    = 1,                              // prepared melee weapon
    SHEATH_STATE_RANGED   = 2                               // prepared ranged weapon
};

#define MAX_SHEATH_STATE    3

// byte (1 from 0..3) of UNIT_FIELD_BYTES_2
enum UnitBytes2_Flags
{
    UNIT_BYTE2_FLAG_UNK0  = 0x01,
    UNIT_BYTE2_FLAG_UNK1  = 0x02,
    UNIT_BYTE2_FLAG_UNK2  = 0x04,
    UNIT_BYTE2_FLAG_SANCTUARY   = 0x08,
    UNIT_BYTE2_FLAG_AURAS = 0x10,                           // show possitive auras as positive, and allow its dispel
    UNIT_BYTE2_FLAG_UNK5  = 0x20,                           // always show negative auras as positive, disallowing dispel
    UNIT_BYTE2_FLAG_UNK6  = 0x40,
    UNIT_BYTE2_FLAG_UNK7  = 0x80
};

// byte (2 from 0..3) of UNIT_FIELD_BYTES_2
enum UnitRename
{
    UNIT_RENAME_NOT_ALLOWED = 0x02,
    UNIT_RENAME_ALLOWED     = 0x03
};

#define CREATURE_MAX_SPELLS     5

enum Swing
{
    NOSWING                    = 0,
    SINGLEHANDEDSWING          = 1,
    TWOHANDEDSWING             = 2
};

enum VictimState
{
    VICTIMSTATE_UNKNOWN1       = 0,
    VICTIMSTATE_NORMAL         = 1,
    VICTIMSTATE_DODGE          = 2,
    VICTIMSTATE_PARRY          = 3,
    VICTIMSTATE_INTERRUPT      = 4,
    VICTIMSTATE_BLOCKS         = 5,
    VICTIMSTATE_EVADES         = 6,
    VICTIMSTATE_IS_IMMUNE      = 7,
    VICTIMSTATE_DEFLECTS       = 8
};

enum HitInfo
{
    HITINFO_NORMALSWING         = 0x00000000,
    HITINFO_UNK1                = 0x00000001,               // req correct packet structure
    HITINFO_NORMALSWING2        = 0x00000002,
    HITINFO_LEFTSWING           = 0x00000004,
    HITINFO_RANGED              = 0x00000008,               // test
    HITINFO_MISS                = 0x00000010,
    HITINFO_ABSORB              = 0x00000020,               // plays absorb sound
    HITINFO_RESIST              = 0x00000040,               // resisted at least some damage
    HITINFO_CRITICALHIT         = 0x00000080,
    HITINFO_UNK2                = 0x00000100,               // wotlk?
    HITINFO_UNK3                = 0x00002000,               // wotlk?
    HITINFO_GLANCING            = 0x00004000,
    HITINFO_CRUSHING            = 0x00008000,
    HITINFO_NOACTION            = 0x00010000,
    HITINFO_SWINGNOHITSOUND     = 0x00080000
};

//i would like to remove this: (it is defined in item.h
enum InventorySlot
{
    NULL_BAG                   = 0,
    NULL_SLOT                  = 255
};

struct FactionTemplateEntry;
struct Modifier;
struct SpellEntry;
struct SpellValue;
struct CasterModifiers;
struct CharmInfo;

class Aura;
class Creature;
class Spell;
class DynamicObject;
class GameObject;
class Item;
class Pet;
class PetAura;
class UnitAI;

struct SpellImmune
{
    uint32 type;
    uint32 spellId;
};

typedef std::list<SpellImmune> SpellImmuneList;

enum UnitModifierType
{
    BASE_VALUE = 0,
    BASE_PCT = 1,
    TOTAL_VALUE = 2,
    TOTAL_PCT = 3,
    MODIFIER_TYPE_END = 4
};

enum WeaponDamageRange
{
    MINDAMAGE,
    MAXDAMAGE
};

enum DamageTypeToSchool
{
    RESISTANCE,
    DAMAGE_DEALT,
    DAMAGE_TAKEN
};

enum AuraRemoveMode
{
    AURA_REMOVE_BY_DEFAULT,
    AURA_REMOVE_BY_STACK,                                   // at replace by semillar aura
    AURA_REMOVE_BY_CANCEL,
    AURA_REMOVE_BY_DISPEL,
    AURA_REMOVE_BY_DEATH,
    AURA_REMOVE_BY_EXPIRE
};

enum UnitMods
{
    UNIT_MOD_STAT_STRENGTH,                                 // UNIT_MOD_STAT_STRENGTH..UNIT_MOD_STAT_SPIRIT must be in existed order, it's accessed by index values of Stats enum.
    UNIT_MOD_STAT_AGILITY,
    UNIT_MOD_STAT_STAMINA,
    UNIT_MOD_STAT_INTELLECT,
    UNIT_MOD_STAT_SPIRIT,
    UNIT_MOD_HEALTH,
    UNIT_MOD_MANA,                                          // UNIT_MOD_MANA..UNIT_MOD_HAPPINESS must be in existed order, it's accessed by index values of Powers enum.
    UNIT_MOD_RAGE,
    UNIT_MOD_FOCUS,
    UNIT_MOD_ENERGY,
    UNIT_MOD_HAPPINESS,
    UNIT_MOD_ARMOR,                                         // UNIT_MOD_ARMOR..UNIT_MOD_RESISTANCE_ARCANE must be in existed order, it's accessed by index values of SpellSchools enum.
    UNIT_MOD_RESISTANCE_HOLY,
    UNIT_MOD_RESISTANCE_FIRE,
    UNIT_MOD_RESISTANCE_NATURE,
    UNIT_MOD_RESISTANCE_FROST,
    UNIT_MOD_RESISTANCE_SHADOW,
    UNIT_MOD_RESISTANCE_ARCANE,
    UNIT_MOD_ATTACK_POWER,
    UNIT_MOD_ATTACK_POWER_RANGED,
    UNIT_MOD_DAMAGE_MAINHAND,
    UNIT_MOD_DAMAGE_OFFHAND,
    UNIT_MOD_DAMAGE_RANGED,
    UNIT_MOD_END,
    // synonyms
    UNIT_MOD_STAT_START = UNIT_MOD_STAT_STRENGTH,
    UNIT_MOD_STAT_END = UNIT_MOD_STAT_SPIRIT + 1,
    UNIT_MOD_RESISTANCE_START = UNIT_MOD_ARMOR,
    UNIT_MOD_RESISTANCE_END = UNIT_MOD_RESISTANCE_ARCANE + 1,
    UNIT_MOD_POWER_START = UNIT_MOD_MANA,
    UNIT_MOD_POWER_END = UNIT_MOD_HAPPINESS + 1
};

enum BaseModGroup
{
    CRIT_PERCENTAGE,
    RANGED_CRIT_PERCENTAGE,
    OFFHAND_CRIT_PERCENTAGE,
    SHIELD_BLOCK_VALUE,
    BASEMOD_END
};

enum BaseModType
{
    FLAT_MOD,
    PCT_MOD
};

#define MOD_END (PCT_MOD+1)

enum DeathState
{
    ALIVE       = 0,
    JUST_DIED   = 1,
    CORPSE      = 2,
    DEAD        = 3,
    JUST_ALIVED = 4,
};

enum UnitState
{
    UNIT_STAT_MELEE_ATTACKING    = 0x00000001,        // player is melee attacking someone
    UNIT_STAT_IGNORE_ATTACKERS   = 0x00000002,        // unit will ignore all attackers and won't add them to threat list(NYI)
    UNIT_STAT_STUNNED            = 0x00000004,        // unit is stunned
    UNIT_STAT_IGNORE_PATHFINDING = 0x00000008,        // unit won't generate path
    UNIT_STAT_CHASE              = 0x00000010,        // unit is chasing someone
    UNIT_STAT_FLEEING            = 0x00000020,        // unit is feared
    UNIT_STAT_TAXI_FLIGHT        = 0x00000040,        // player is flying using taxi mode
    UNIT_STAT_FOLLOW             = 0x00000080,        // unit is following someone
    UNIT_STAT_ROOT               = 0x00000100,        // unit is rooted
    UNIT_STAT_CONFUSED           = 0x00000200,        // unit is disoriented
    UNIT_STAT_DISTRACTED         = 0x00000400,        // unit is distracted
    UNIT_STAT_ISOLATED           = 0x00000800,        // area auras do not affect other players
    UNIT_STAT_ATTACK_PLAYER      = 0x00001000,        // unit is attacking a player
    UNIT_STAT_CASTING            = 0x00002000,        // unit is casting a spell with cast time
    UNIT_STAT_POSSESSED          = 0x00004000,        // unit is possessed
    UNIT_STAT_CHARGING           = 0x00008000,        // unit is charging
    UNIT_STAT_ROTATING           = 0x00010000,        // unit is rotating
    UNIT_STAT_CASTING_NOT_MOVE   = 0x00020000,        // unit is casting a spell and can NOT move
    UNIT_STAT_STAND_UP_PENDING   = 0x00040000,        // unit is will stand up after proc checks run

    // stay or scripted movement for effect( = in player case you can't move by client command)
    UNIT_STAT_NO_FREE_MOVE    = UNIT_STAT_ROOT | UNIT_STAT_STUNNED | UNIT_STAT_TAXI_FLIGHT |
                                UNIT_STAT_CONFUSED | UNIT_STAT_FLEEING,

    UNIT_STAT_LOST_CONTROL    = (UNIT_STAT_CONFUSED | UNIT_STAT_STUNNED | UNIT_STAT_FLEEING | UNIT_STAT_CHARGING),
    UNIT_STAT_CAN_NOT_REACT   = (UNIT_STAT_STUNNED | UNIT_STAT_CONFUSED | UNIT_STAT_FLEEING),
    UNIT_STAT_CAN_NOT_MOVE    = (UNIT_STAT_ROOT | UNIT_STAT_STUNNED),
    UNIT_STAT_NOT_MOVE        = (UNIT_STAT_ROOT | UNIT_STAT_STUNNED | UNIT_STAT_DISTRACTED),
    UNIT_STAT_CANNOT_AUTOATTACK = (UNIT_STAT_LOST_CONTROL | UNIT_STAT_CASTING | UNIT_STAT_CASTING_NOT_MOVE),
    UNIT_STAT_CANNOT_TURN     = (UNIT_STAT_LOST_CONTROL | UNIT_STAT_ROTATING),
    UNIT_STAT_ALL_STATE       = 0xffffffff
};

enum UnitStandStateType
{
    UNIT_STAND_STATE_STAND             = 0,
    UNIT_STAND_STATE_SIT               = 1,
    UNIT_STAND_STATE_SIT_CHAIR         = 2,
    UNIT_STAND_STATE_SLEEP             = 3,
    UNIT_STAND_STATE_SIT_LOW_CHAIR     = 4,
    UNIT_STAND_STATE_SIT_MEDIUM_CHAIR  = 5,
    UNIT_STAND_STATE_SIT_HIGH_CHAIR    = 6,
    UNIT_STAND_STATE_DEAD              = 7,
    UNIT_STAND_STATE_KNEEL             = 8,
    UNIT_STAND_STATE_SUBMERGED         = 9
};

enum UnitMoveType
{
    MOVE_WALK           = 0,
    MOVE_RUN            = 1,
    MOVE_RUN_BACK       = 2,
    MOVE_SWIM           = 3,
    MOVE_SWIM_BACK      = 4,
    MOVE_TURN_RATE      = 5,
    MOVE_FLIGHT         = 6,
    MOVE_FLIGHT_BACK    = 7,
};

#define MAX_MOVE_TYPE 8

extern float baseMoveSpeed[MAX_MOVE_TYPE];

enum WeaponAttackType
{
    BASE_ATTACK   = 0,
    OFF_ATTACK    = 1,
    RANGED_ATTACK = 2
};

#define MAX_ATTACK  3

enum CombatRating
{
    CR_WEAPON_SKILL             = 0,
    CR_DEFENSE_SKILL            = 1,
    CR_DODGE                    = 2,
    CR_PARRY                    = 3,
    CR_BLOCK                    = 4,
    CR_HIT_MELEE                = 5,
    CR_HIT_RANGED               = 6,
    CR_HIT_SPELL                = 7,
    CR_CRIT_MELEE               = 8,
    CR_CRIT_RANGED              = 9,
    CR_CRIT_SPELL               = 10,
    CR_HIT_TAKEN_MELEE          = 11,
    CR_HIT_TAKEN_RANGED         = 12,
    CR_HIT_TAKEN_SPELL          = 13,
    CR_CRIT_TAKEN_MELEE         = 14,
    CR_CRIT_TAKEN_RANGED        = 15,
    CR_CRIT_TAKEN_SPELL         = 16,
    CR_HASTE_MELEE              = 17,
    CR_HASTE_RANGED             = 18,
    CR_HASTE_SPELL              = 19,
    CR_WEAPON_SKILL_MAINHAND    = 20,
    CR_WEAPON_SKILL_OFFHAND     = 21,
    CR_WEAPON_SKILL_RANGED      = 22,
    CR_EXPERTISE                = 23
};

#define MAX_COMBAT_RATING         24

enum DamageEffectType
{
    DIRECT_DAMAGE           = 0,                            // used for normal weapon damage (not for class abilities or spells)
    SPELL_DIRECT_DAMAGE     = 1,                            // spell/class abilities damage
    DOT                     = 2,
    HEAL                    = 3,
    NODAMAGE                = 4,                            // used also in case when damage applied to health but not applied to spell channelInterruptFlags/etc
    SELF_DAMAGE             = 5
};

enum UnitVisibility
{
    VISIBILITY_OFF                = 0,                      // absolute, not detectable, GM-like, can see all other
    VISIBILITY_ON                 = 1,
    VISIBILITY_GROUP_STEALTH      = 2,                      // detect chance, seen and can see group members
    //VISIBILITY_GROUP_INVISIBILITY = 3,                      // invisibility, can see and can be seen only another invisible unit or invisible detection unit, set only if not stealthed, and in checks not used (mask used instead)
    //VISIBILITY_GROUP_NO_DETECT    = 4,                      // state just at stealth apply for update Grid state. Don't remove, otherwise stealth spells will break
    VISIBILITY_RESPAWN            = 5                       // special totally not detectable visibility for force delete object at respawn command
};

// Value masks for UNIT_FIELD_FLAGS
enum UnitFlags
{
    UNIT_FLAG_UNKNOWN7            = 0x00000001,
    UNIT_FLAG_SPAWNING            = 0x00000002,                // not attackable
    UNIT_FLAG_DISABLE_MOVE        = 0x00000004,
    UNIT_FLAG_PVP_ATTACKABLE      = 0x00000008,                // allow apply pvp rules to attackable state in addition to faction dependent state
    UNIT_FLAG_RENAME              = 0x00000010,
    UNIT_FLAG_PREPARATION         = 0x00000020,                // don't take reagents for spells with SPELL_ATTR_EX5_NO_REAGENT_WHILE_PREP
    UNIT_FLAG_UNKNOWN9            = 0x00000040,
    UNIT_FLAG_NOT_ATTACKABLE_1    = 0x00000080,                // ?? (UNIT_FLAG_PVP_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1) is NON_PVP_ATTACKABLE
    UNIT_FLAG_NOT_ATTACKABLE_2    = 0x00000100,                // not attackable when out of combat
    UNIT_FLAG_PASSIVE             = 0x00000200,                // makes you unable to attack everything. Almost identical to our "civilian"-term. Will ignore it's surroundings and not engage in combat unless "called upon" or engaged by another unit.
    UNIT_FLAG_LOOTING             = 0x00000400,                // loot animation
    UNIT_FLAG_PET_IN_COMBAT       = 0x00000800,                // in combat?, 2.0.8
    UNIT_FLAG_PVP                 = 0x00001000,
    UNIT_FLAG_SILENCED            = 0x00002000,                // silenced, 2.1.1
    UNIT_FLAG_UNKNOWN4            = 0x00004000,                // 2.0.8
    UNIT_FLAG_UNKNOWN13           = 0x00008000,
    UNIT_FLAG_NOT_PL_SPELL_TARGET = 0x00010000,                // player can't cast spells on NPC with that flag.
    UNIT_FLAG_PACIFIED            = 0x00020000,
    UNIT_FLAG_DISABLE_ROTATE      = 0x00040000,                // stunned, 2.1.1
    UNIT_FLAG_IN_COMBAT           = 0x00080000,
    UNIT_FLAG_TAXI_FLIGHT         = 0x00100000,                // disable casting at client side spell not allowed by taxi flight (mounted?), probably used with 0x4 flag
    UNIT_FLAG_DISARMED            = 0x00200000,                // disable melee spells casting..., "Required melee weapon" added to melee spells tooltip.
    UNIT_FLAG_CONFUSED            = 0x00400000,
    UNIT_FLAG_FLEEING             = 0x00800000,
    UNIT_FLAG_PLAYER_CONTROLLED   = 0x01000000,                // used in spell Eyes of the Beast for pet... let attack by controlled creature
    UNIT_FLAG_NOT_SELECTABLE      = 0x02000000,
    UNIT_FLAG_SKINNABLE           = 0x04000000,
    UNIT_FLAG_MOUNT               = 0x08000000,
    UNIT_FLAG_UNKNOWN17           = 0x10000000,
    UNIT_FLAG_UNKNOWN6            = 0x20000000,                // used in Feing Death spell
    UNIT_FLAG_SHEATHE             = 0x40000000
};

// Value masks for UNIT_FIELD_FLAGS_2
enum UnitFlags2
{
    UNIT_FLAG2_FEIGN_DEATH      = 0x00000001,
    UNIT_FLAG2_COMPREHEND_LANG  = 0x00000008,
    UNIT_FLAG2_CLONED           = 0x00000010, // Used in SPELL_AURA_MIRROR_IMAGE
    UNIT_FLAG2_FORCE_MOVE       = 0x00000040,
    UNIT_FLAG2_UNKNOWN1         = 0x00000800
};

/// Non Player Character flags
enum NPCFlags
{
    UNIT_NPC_FLAG_NONE                  = 0x00000000,
    UNIT_NPC_FLAG_GOSSIP                = 0x00000001,       // 100%
    UNIT_NPC_FLAG_QUESTGIVER            = 0x00000002,       // guessed, probably ok
    UNIT_NPC_FLAG_UNK1                  = 0x00000004,
    UNIT_NPC_FLAG_UNK2                  = 0x00000008,
    UNIT_NPC_FLAG_TRAINER               = 0x00000010,       // 100%
    UNIT_NPC_FLAG_TRAINER_CLASS         = 0x00000020,       // 100%
    UNIT_NPC_FLAG_TRAINER_PROFESSION    = 0x00000040,       // 100%
    UNIT_NPC_FLAG_VENDOR                = 0x00000080,       // 100%
    UNIT_NPC_FLAG_VENDOR_AMMO           = 0x00000100,       // 100%, general goods vendor
    UNIT_NPC_FLAG_VENDOR_FOOD           = 0x00000200,       // 100%
    UNIT_NPC_FLAG_VENDOR_POISON         = 0x00000400,       // guessed
    UNIT_NPC_FLAG_VENDOR_REAGENT        = 0x00000800,       // 100%
    UNIT_NPC_FLAG_REPAIR                = 0x00001000,       // 100%
    UNIT_NPC_FLAG_FLIGHTMASTER          = 0x00002000,       // 100%
    UNIT_NPC_FLAG_SPIRITHEALER          = 0x00004000,       // guessed
    UNIT_NPC_FLAG_SPIRITGUIDE           = 0x00008000,       // guessed
    UNIT_NPC_FLAG_INNKEEPER             = 0x00010000,       // 100%
    UNIT_NPC_FLAG_BANKER                = 0x00020000,       // 100%
    UNIT_NPC_FLAG_PETITIONER            = 0x00040000,       // 100% 0xC0000 = guild petitions, 0x40000 = arena team petitions
    UNIT_NPC_FLAG_TABARDDESIGNER        = 0x00080000,       // 100%
    UNIT_NPC_FLAG_BATTLEMASTER          = 0x00100000,       // 100%
    UNIT_NPC_FLAG_AUCTIONEER            = 0x00200000,       // 100%
    UNIT_NPC_FLAG_STABLEMASTER          = 0x00400000,       // 100%
    UNIT_NPC_FLAG_GUILD_BANKER          = 0x00800000,       // cause client to send 997 opcode
    UNIT_NPC_FLAG_SPELLCLICK            = 0x01000000,       // cause client to send 1015 opcode (spell click)
    UNIT_NPC_FLAG_GUARD                 = 0x10000000,       // custom flag for guards
    UNIT_NPC_FLAG_OUTDOORPVP            = 0x20000000,       // custom flag for outdoor pvp creatures
};

enum MovementFlags
{
    MOVEFLAG_NONE               = 0x00000000,
    MOVEFLAG_FORWARD            = 0x00000001,
    MOVEFLAG_BACKWARD           = 0x00000002,
    MOVEFLAG_STRAFE_LEFT        = 0x00000004,
    MOVEFLAG_STRAFE_RIGHT       = 0x00000008,
    MOVEFLAG_TURN_LEFT          = 0x00000010,
    MOVEFLAG_TURN_RIGHT         = 0x00000020,
    MOVEFLAG_PITCH_UP           = 0x00000040,
    MOVEFLAG_PITCH_DOWN         = 0x00000080,
    MOVEFLAG_WALK_MODE          = 0x00000100,               // Walking
    MOVEFLAG_ONTRANSPORT        = 0x00000200,               // Used for flying on some creatures
    MOVEFLAG_LEVITATING         = 0x00000400,
    MOVEFLAG_ROOT               = 0x00000800,
    MOVEFLAG_FALLING            = 0x00001000,
    MOVEFLAG_FALLINGFAR         = 0x00004000,
    MOVEFLAG_SWIMMING           = 0x00200000,               // appears with fly flag also
    MOVEFLAG_ASCENDING          = 0x00400000,               // swim up also
    MOVEFLAG_CAN_FLY            = 0x00800000,
    SPLINEFLAG_FLYINGING        = 0x01000000,
    MOVEFLAG_FLYING             = 0x02000000,               // Actual flying mode
    MOVEFLAG_SPLINE_ELEVATION   = 0x04000000,               // used for flight paths
    MOVEFLAG_SPLINE_ENABLED     = 0x08000000,               // used for flight paths
    MOVEFLAG_WATERWALKING       = 0x10000000,               // prevent unit from falling through water
    MOVEFLAG_SAFE_FALL          = 0x20000000,               // active rogue safe fall spell (passive)
    MOVEFLAG_HOVER              = 0x40000000,

    MOVEFLAG_MOVING         =
        MOVEFLAG_FORWARD |MOVEFLAG_BACKWARD  |MOVEFLAG_STRAFE_LEFT |MOVEFLAG_STRAFE_RIGHT|
        MOVEFLAG_PITCH_UP|MOVEFLAG_PITCH_DOWN|
        MOVEFLAG_FALLING |MOVEFLAG_FALLINGFAR|MOVEFLAG_ASCENDING   |
        MOVEFLAG_FLYING |MOVEFLAG_SPLINE_ELEVATION,
    MOVEFLAG_TURNING        =
        MOVEFLAG_TURN_LEFT | MOVEFLAG_TURN_RIGHT,
};

namespace Movement
{
    class MoveSpline;
}

class MovementInfo
{
    public:
        MovementInfo() : moveFlags(MOVEFLAG_NONE), moveFlags2(0), time(0), t_guid(0),
            t_time(0), s_pitch(0.0f), fallTime(0), j_velocity(0.0f), j_sinAngle(0.0f),
            j_cosAngle(0.0f), j_xyspeed(0.0f), u_unk1(0.0f) {}

        // Read/Write methods
        void Read(ByteBuffer &data);
        void Write(ByteBuffer &data) const;

        // Movement flags manipulations
        void AddMovementFlag(MovementFlags f) { moveFlags |= f; }
        void RemoveMovementFlag(MovementFlags f) { moveFlags &= ~f; }
        bool HasMovementFlag(MovementFlags f) const { return moveFlags & f; }
        MovementFlags GetMovementFlags() const { return MovementFlags(moveFlags); }
        void SetMovementFlags(MovementFlags f) { moveFlags = f; }

        // Position manipulations
        Position const *GetPos() const { return &pos; }
        void SetTransportData(uint64 guid, float x, float y, float z, float o, uint32 time)
        {
            t_guid = guid;
            t_pos.m_positionX = x;
            t_pos.m_positionY = y;
            t_pos.m_positionZ = z;
            t_pos.o = o;
            t_time = time;
        }
        void ClearTransportData()
        {
            t_guid = 0;
            t_pos.m_positionX = 0.0f;
            t_pos.m_positionY = 0.0f;
            t_pos.m_positionZ = 0.0f;
            t_pos.o = 0.0f;
            t_time = 0;
        }
        uint64 const& GetTransportGuid() const { return t_guid; }
        Position const *GetTransportPos() const { return &t_pos; }
        uint32 GetTransportTime() const { return t_time; }
        uint32 GetFallTime() const { return fallTime; }

        void ChangeOrientation(float o) { pos.o = o; }
        void ChangePosition(float x, float y, float z, float o) { pos.m_positionX = x; pos.m_positionY = y; pos.m_positionZ = z; pos.o = o; }
        void UpdateTime(uint32 _time) { time = _time; }

    //private:
        // common
        uint32  moveFlags;                                  // see enum MovementFlags
        uint8   moveFlags2;
        uint32  time;
        Position pos;
        // transport
        uint64  t_guid;
        Position t_pos;
        uint32  t_time;
        // swimming and unknown
        float   s_pitch;
        // last fall time
        uint32  fallTime;
        // jumping
        float   j_velocity, j_sinAngle, j_cosAngle, j_xyspeed;
        // spline
        float   u_unk1;
};

inline ByteBuffer& operator<< (ByteBuffer& buf, MovementInfo const& mi)
{
    mi.Write(buf);
    return buf;
}

inline ByteBuffer& operator>> (ByteBuffer& buf, MovementInfo& mi)
{
    mi.Read(buf);
    return buf;
}

enum DiminishingLevels
{
    DIMINISHING_LEVEL_1             = 0,
    DIMINISHING_LEVEL_2             = 1,
    DIMINISHING_LEVEL_3             = 2,
    DIMINISHING_LEVEL_IMMUNE        = 3
};

struct DiminishingReturn
{
    DiminishingReturn(DiminishingGroup group, uint32 t, uint32 count) : DRGroup(group), hitTime(t), hitCount(count), stacks(0) {}

    DiminishingGroup        DRGroup:16;
    uint16                  stacks:16;
    uint32                  hitTime;
    uint32                  hitCount;
};

struct DamageLog
{
    DamageLog(uint32 op, Unit *attacker, Unit *victim, uint8 school_mask = SPELL_SCHOOL_NORMAL, WeaponAttackType attType = BASE_ATTACK) :
              opcode(op), source(attacker), target(victim), schoolMask(school_mask), attackType(attType),
              damage(0), absorb(0), resist(0), blocked(0), hitInfo(0), rageDamage(0), threatTarget(0) {}

    uint32 opcode;
    Unit *source;
    Unit *target;
    uint8 schoolMask;

    uint32 damage;

    uint32 absorb;
    uint32 resist;
    uint32 blocked;

    uint32 hitInfo;
    uint32 rageDamage;
    WeaponAttackType attackType;

    uint64 threatTarget;
};

struct SpellDamageLog : public DamageLog
{
    SpellDamageLog(uint32 spellId, Unit *attacker, Unit *victim, uint8 school_mask = SPELL_SCHOOL_NORMAL, uint8 dmgType = 0) :
         DamageLog(SMSG_SPELLNONMELEEDAMAGELOG, attacker, victim, school_mask, BASE_ATTACK), spell_id(spellId), damageType(dmgType){}

    uint8 damageType;
    uint32 spell_id;
};

struct MeleeDamageLog : public DamageLog
{
    MeleeDamageLog(Unit *attacker, Unit *victim, uint8 school_mask = SPELL_SCHOOL_NORMAL, WeaponAttackType attType = BASE_ATTACK) :
         DamageLog(SMSG_ATTACKERSTATEUPDATE, attacker, victim, school_mask, attType), targetState(0),
                   procAttacker(0), procVictim(0), procEx(0) {}

    uint32 targetState;

    // helpers
    uint32 procAttacker;
    uint32 procVictim;
    uint32 procEx;
};

uint32 createProcExtendMask(SpellDamageLog *damageInfo, SpellMissInfo missCondition);

#define MAX_DECLINED_NAME_CASES 5

struct DeclinedName
{
    std::string name[MAX_DECLINED_NAME_CASES];
};

enum CurrentSpellTypes
{
    CURRENT_MELEE_SPELL = 0,
    CURRENT_FIRST_NON_MELEE_SPELL = 1,                      // just counter
    CURRENT_GENERIC_SPELL = 1,
    CURRENT_AUTOREPEAT_SPELL = 2,
    CURRENT_CHANNELED_SPELL = 3,
    CURRENT_MAX_SPELL = 4                                   // just counter
};

class CastSpellEvent : public BasicEvent
{
    public:
        CastSpellEvent(Unit& owner, uint64 target, uint32 spellId, bool triggered = false, uint64 orginalCaster = 0) :
            BasicEvent(), m_owner(owner), m_target(target),  m_spellId(spellId), m_triggered(triggered), m_orginalCaster(orginalCaster), m_custom(false) { }
        CastSpellEvent(Unit& owner, uint64 target, uint32 spellId, int32* bp0, int32* bp1, int32* bp2, bool triggered = false, uint64 orginalCaster = 0);

        bool Execute(uint64 e_time, uint32 p_time);
    private:

        uint64            m_target;
        uint64            m_orginalCaster;
        uint32            m_spellId;
        bool              m_custom;
        bool              m_triggered;
        CustomSpellValues m_values;

        Unit&             m_owner;
};

typedef std::list<Player*> SharedVisionList;

// for clearing special attacks
#define REACTIVE_TIMER_START 4000

enum ReactiveType
{
    REACTIVE_DEFENSE      = 1,
    REACTIVE_HUNTER_PARRY = 2,
    REACTIVE_CRIT         = 3,
    REACTIVE_HUNTER_CRIT  = 4,
    REACTIVE_OVERPOWER    = 5
};

#define MAX_REACTIVE 6

// Stored in SummonProperties.dbc with slot+1 values
enum TotemSlot
{
    TOTEM_SLOT_FIRE   = 0,
    TOTEM_SLOT_EARTH  = 1,
    TOTEM_SLOT_WATER  = 2,
    TOTEM_SLOT_AIR    = 3,
};

#define MAX_TOTEM 4

// delay time next attack to prevent client attack animation problems
#define ATTACK_DISPLAY_DELAY 200
#define MAX_PLAYER_STEALTH_DETECT_RANGE 45.0f               // max distance for detection targets by player

struct SpellProcEventEntry;                                 // used only privately

class Unit : public WorldObject
{
    public:
        typedef std::set<Unit*> AttackerSet;
        typedef std::pair<uint32, uint8> spellEffectPair;
        typedef std::multimap< spellEffectPair, Aura*> AuraMap;
        typedef std::list<Aura *> AuraList;
        typedef std::list<DiminishingReturn> Diminishing;
        typedef std::set<AuraType> AuraTypeSet;
        typedef std::set<uint32> ComboPointHolderSet;

        virtual ~Unit ();

        void AddToWorld();
        void RemoveFromWorld();

        void setHover(bool val);

        void CleanupsBeforeDelete();                        // used in ~Creature/~Player (or before mass creature delete to remove cross-references to already deleted units)

        float GetObjectBoundingRadius() const               // overwrite WorldObject version
        {
            return m_floatValues[UNIT_FIELD_BOUNDINGRADIUS];
        }

        DiminishingLevels GetDiminishing(DiminishingGroup  group);
        void IncrDiminishing(DiminishingGroup group);
        void ApplyDiminishingToDuration(DiminishingGroup  group, int32 &duration, DiminishingLevels Level, SpellEntry const *tSpell = NULL);
        void ApplyDiminishingAura(DiminishingGroup  group, bool apply);
        void ClearDiminishings() { m_Diminishing.clear(); }

        //target dependent checks
        uint32 GetSpellRadiusForTarget(Unit* target,const SpellRadiusEntry * radiusEntry);

        virtual void Update(uint32 update_diff, uint32 p_time);

        void SetTargetGuid(ObjectGuid targetGuid) { SetGuidValue(UNIT_FIELD_TARGET, targetGuid); }
        ObjectGuid const& GetTargetGuid() const { return GetGuidValue(UNIT_FIELD_TARGET); }
        void ClearTarget() { SetTargetGuid(ObjectGuid()); }
        void SetSelection(uint64 guid){ SetUInt64Value(UNIT_FIELD_TARGET, guid); }
        uint64 GetSelection() { return GetUInt64Value(UNIT_FIELD_TARGET); }

        void setAttackTimer(WeaponAttackType type, uint32 time) { m_attackTimer[type] = time; }
        void resetAttackTimer(WeaponAttackType type = BASE_ATTACK);
        uint32 getAttackTimer(WeaponAttackType type) const { return m_attackTimer[type]; }
        bool isAttackReady(WeaponAttackType type = BASE_ATTACK) const { return m_attackTimer[type] == 0; }
        bool haveOffhandWeapon() const;
        bool CanDualWield() const { return m_canDualWield; }
        void SetCanDualWield(bool value) { m_canDualWield = value; }
        float GetCombatReach(bool forMeleeRange = false) const;
        float GetCombatReach(Unit const* pVictim, bool ability, float flat_mod) const;
        void RecalculateCombatReach();
        bool IsWithinCombatRange(const Unit *obj, float dist2compare) const;
        bool IsWithinMeleeRange(Unit *obj, float dist = MELEE_RANGE) const;
        bool CanReachWithMeleeAutoAttack(Unit * pVictim, float flat_mod = 0.0f) const { return IsWithinMeleeRange(pVictim, MELEE_RANGE + flat_mod); };
        bool CanReachWithMeleeAutoAttackAtPosition(Unit const* pVictim, float x, float y, float z, float flat_mod = 0.0f) const;
        void GetRandomContactPoint(const Unit* target, float &x, float &y, float &z, float distance2dMin, float distance2dMax) const;
        uint32 m_extraAttacks;
        bool m_canDualWield;

        bool isInSanctuary();

        void _addAttacker(Unit *pAttacker)                  // must be called only from Unit::Attack(Unit*)
        {
            AttackerSet::iterator itr = m_attackers.find(pAttacker);
            if (itr == m_attackers.end())
                m_attackers.insert(pAttacker);
        }
        void _removeAttacker(Unit *pAttacker)               // must be called only from Unit::AttackStop()
        {
            AttackerSet::iterator itr = m_attackers.find(pAttacker);
            if (itr != m_attackers.end())
                m_attackers.erase(itr);
        }
        Unit * GetAttackerForHelper()                       // If someone wants to help, who to give them
        {
            if (GetVictim() != NULL)
                return GetVictim();

            if (!m_attackers.empty())
                return *(m_attackers.begin());

            return NULL;
        }
        bool Attack(Unit *victim, bool meleeAttack);
        void CastStop(uint32 except_spellid = 0);
        bool AttackStop();
        void RemoveAllAttackers();
        AttackerSet const& GetAttackers() const { return m_attackers; }
        bool isAttackingPlayer() const;
        Unit* GetVictim() const { return m_attacking; }
        uint64 getVictimGUID() const { return m_attacking ? m_attacking->GetGUID() : 0; }
        void CombatStop(bool cast = false);
        void CombatStopWithPets(bool cast = false);
        Unit* SelectNearbyTarget(float dist = NOMINAL_MELEE_RANGE, Unit* target = NULL) const;
        uint8 GetEnemyCountInRadiusAround(Unit* pTarget, float radius) const;
        void SendMeleeAttackStop(Unit* victim);
        void SendMeleeAttackStart(uint64 victimGUID);

        void addUnitState(uint32 f) { m_state |= f; }
        bool HasUnitState(const uint32 f) const { return (m_state & f); }
        void ClearUnitState(uint32 f) { m_state &= ~f; }
        bool CanFreeMove() const
        {
            return !HasUnitState(UNIT_STAT_CONFUSED | UNIT_STAT_FLEEING | UNIT_STAT_TAXI_FLIGHT |
                UNIT_STAT_ROOT | UNIT_STAT_STUNNED | UNIT_STAT_DISTRACTED) && GetOwnerGUID() == 0;
        }

        uint32 GetLevel() const { return GetUInt32Value(UNIT_FIELD_LEVEL); }
        virtual uint32 getLevelForTarget(Unit const* /*target*/) const { return GetLevel(); }
        void SetLevel(uint32 lvl);
        uint8 GetRace() const { return GetByteValue(UNIT_FIELD_BYTES_0, 0); }
        uint32 getRaceMask() const { return 1 << (GetRace()-1); }
        uint8 GetClass() const { return GetByteValue(UNIT_FIELD_BYTES_0, 1); }
        uint32 getClassMask() const { return 1 << (GetClass()-1); }
        uint8 getGender() const { return GetByteValue(UNIT_FIELD_BYTES_0, 2); }

        virtual float GetXPMod() const { return 1.0f; }

        float GetStat(Stats stat) const { return float(GetUInt32Value(UNIT_FIELD_STAT0+stat)); }
        void SetStat(Stats stat, int32 val) { SetStatInt32Value(UNIT_FIELD_STAT0+stat, val); }
        uint32 GetArmor() const { return GetResistance(SPELL_SCHOOL_NORMAL) ; }
        void SetArmor(int32 val) { SetResistance(SPELL_SCHOOL_NORMAL, val); }

        uint32 GetResistance(SpellSchools school) const { return GetUInt32Value(UNIT_FIELD_RESISTANCES+school); }
        void SetResistance(SpellSchools school, int32 val) { SetStatInt32Value(UNIT_FIELD_RESISTANCES+school,val); }

        uint32 GetHealth()    const { return GetUInt32Value(UNIT_FIELD_HEALTH); }
        uint32 GetMaxHealth() const { return GetUInt32Value(UNIT_FIELD_MAXHEALTH); }
        float GetHealthPercent() const { return (GetHealth()*100.0f) / GetMaxHealth(); }

        bool HealthBelowPct(uint32 pct) const { return GetHealth() *100 < GetMaxHealth() *pct; }

        void SetHealth(uint32 val, bool ignoreAliveCheck = false);
        void SetMaxHealth(uint32 val);
        void SetHealthPercent(float percent);
        int32 ModifyHealth(int32 val);

        Powers GetPowerType() const { return Powers(GetByteValue(UNIT_FIELD_BYTES_0, 3)); }
        void setPowerType(Powers power);
        uint32 GetPower(  Powers power) const { return GetUInt32Value(UNIT_FIELD_POWER1   +power); }
        uint32 GetMaxPower(Powers power) const { return GetUInt32Value(UNIT_FIELD_MAXPOWER1+power); }
        float GetPowerPercent(Powers power) const { return GetMaxPower(power) ? ((GetPower(power)*100.0f) / GetMaxPower(power)) : 100.0f; }
        void SetPower(  Powers power, uint32 val);
        void SetMaxPower(Powers power, uint32 val);
        void SetPowerPercent(Powers power, float percent);
        int32 ModifyPower(Powers power, int32 val);
        void ApplyPowerMod(Powers power, uint32 val, bool apply);

        uint32 GetAttackTime(WeaponAttackType att) const { return (uint32)(GetFloatValue(UNIT_FIELD_BASEATTACKTIME+att)/m_modAttackSpeedPct[att]); }
        void SetAttackTime(WeaponAttackType att, uint32 val) { SetFloatValue(UNIT_FIELD_BASEATTACKTIME+att,val*m_modAttackSpeedPct[att]); }
        void ApplyAttackTimePercentMod(WeaponAttackType att,float val, bool apply);
        void ApplyCastTimePercentMod(float val, bool apply);

        // faction template id
        uint32 getFaction() const { return GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE); }
        void setFaction(uint32 faction) { SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, faction); }
        FactionTemplateEntry const* getFactionTemplateEntry() const;
        bool IsHostileTo(Unit const* unit) const;
        bool IsHostileToPlayers() const;
        bool IsFriendlyTo(Unit const* unit) const;
        bool IsNeutralToAll() const;
        bool IsInPartyWith(Unit const* unit) const;
        bool IsInRaidWith(Unit const* unit) const;
        void GetPartyMember(std::list<Unit*> &units, float dist);
        void GetRaidMember(std::list<Unit*> &units, float dist);
        Unit* GetNextRandomRaidMember(float radius, bool PlayerOnly = false);
        bool IsContestedGuard() const
        {
            if (FactionTemplateEntry const* entry = getFactionTemplateEntry())
                return entry->IsContestedGuardFaction();

            return false;
        }
        bool IsPvP() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP); }
        void SetPvP(bool state) { if (state) SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP); else RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP); }
        uint32 GetCreatureType() const;
        uint32 GetCreatureTypeMask() const
        {
            uint32 creatureType = GetCreatureType();
            return (creatureType >= 1) ? (1 << (creatureType - 1)) : 0;
        }

        uint8 GetStandState() const { return GetByteValue(UNIT_FIELD_BYTES_1, 0); }
        bool IsSitState() const;
        bool IsStandState() const;
        void SetStandState(uint8 state);

        bool IsMounted() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT); }
        uint32 GetMountID() const { return GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID); }
        void Mount(uint32 mount);
        void Unmount();

        uint16 GetMaxSkillValueForLevel(Unit const* target = NULL) const { return (target ? getLevelForTarget(target) : GetLevel()) * 5; }
        void RemoveSpellbyDamageTaken(uint32 damage, uint32 spell);

        void SendDamageLog(DamageLog *damageInfo);

        uint32 DealDamage(DamageLog *damageInfo, DamageEffectType damagetype = DIRECT_DAMAGE, SpellEntry const *spellProto = NULL, bool durabilityLoss = true);

        uint32 DealDamage(Unit *pVictim, uint32 damage, DamageEffectType damagetype = DIRECT_DAMAGE, SpellSchoolMask damageSchoolMask = SPELL_SCHOOL_MASK_NORMAL, SpellEntry const *spellProto = NULL, bool durabilityLoss = true);
        void Kill(Unit *pVictim, bool durabilityLoss = true);

        void ProcDamageAndSpell(Unit *pVictim, uint32 procAttacker, uint32 procVictim, uint32 procEx, uint32 amount, WeaponAttackType attType = BASE_ATTACK, SpellEntry const *procSpell = NULL, bool canTrigger = true);
        void ProcDamageAndSpellfor (bool isVictim, Unit * pTarget, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, SpellEntry const * procSpell, uint32 damage);

        void HandleEmote(uint32 emoteId);                  // auto-select command/state
        void HandleEmoteCommand(uint32 emoteId);
        void HandleEmoteState(uint32 emoteId);
        void AttackerStateUpdate (Unit *pVictim, WeaponAttackType attType = BASE_ATTACK, bool extra = false);
        void HandleProcExtraAttackFor(Unit* victim);

        //float MeleeMissChanceCalc(const Unit *pVictim, WeaponAttackType attType) const;

        void CalculateMeleeDamage(MeleeDamageLog *damageInfo);
        void DealMeleeDamage(MeleeDamageLog *damageInfo, bool durabilityLoss);

        void CalculateSpellDamageTaken(SpellDamageLog *damageInfo, int32 damage, SpellEntry const *spellInfo, WeaponAttackType attackType = BASE_ATTACK, bool crit = false, bool blocked = false);
        void DealSpellDamage(SpellDamageLog *damageInfo, bool durabilityLoss);

        float MeleeSpellMissChance(const Unit *pVictim, WeaponAttackType attType, int32 skillDiff, uint32 spellId) const;
        SpellMissInfo MeleeSpellHitResult(Unit *pVictim, SpellEntry const *spell, bool cMiss = true);
        SpellMissInfo MagicSpellHitResult(Unit *pVictim, SpellEntry const *spell);
        SpellMissInfo SpellHitResult(Unit *pVictim, SpellEntry const *spell, bool canMiss = true);
        SpellMissInfo SpellReflectCheck(SpellEntry const* spell);

        float GetUnitDodgeChance()    const;
        float GetUnitParryChance()    const;
        float GetUnitBlockChance()    const;
        float GetUnitCriticalChance(WeaponAttackType attackType, const Unit *pVictim) const;
        int32 GetSpellMechanicResistChance(const SpellEntry *spell);
        int32 GetEffectMechanicResistChance(const SpellEntry *spell, uint8 eff);

        virtual uint32 GetShieldBlockValue() const =0;
        uint32 GetUnitMeleeSkill(Unit const* target = NULL) const { return (target ? getLevelForTarget(target) : GetLevel()) * 5; }
        uint32 GetDefenseSkillValue(Unit const* target = NULL) const;
        uint32 GetWeaponSkillValue(WeaponAttackType attType, Unit const* target = NULL) const;
        float GetWeaponProcChance() const;
        float GetPPMProcChance(uint32 WeaponSpeed, float PPM) const;

        void RollMeleeHit(MeleeDamageLog *) const;
        void RollMeleeHit(MeleeDamageLog *, int32 crit_chance, int32 miss_chance, int32 dodge_chance, int32 parry_chance, int32 block_chance) const;

        bool isVendor()       const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR); }
        bool isTrainer()      const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER); }
        bool isQuestGiver()   const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER); }
        bool isGossip()       const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP); }
        bool isTaxi()         const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_FLIGHTMASTER); }
        bool isGuildMaster()  const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_PETITIONER); }
        bool isBattleMaster() const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BATTLEMASTER); }
        bool isBanker()       const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BANKER); }
        bool isInnkeeper()    const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_INNKEEPER); }
        bool isSpiritHealer() const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER); }
        bool isSpiritGuide()  const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITGUIDE); }
        bool isTabardDesigner()const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TABARDDESIGNER); }
        bool isAuctioner()    const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_AUCTIONEER); }
        bool isArmorer()      const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_REPAIR); }
        bool isServiceProvider() const
        {
            return HasFlag(UNIT_NPC_FLAGS,
                UNIT_NPC_FLAG_VENDOR | UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_FLIGHTMASTER |
                UNIT_NPC_FLAG_PETITIONER | UNIT_NPC_FLAG_BATTLEMASTER | UNIT_NPC_FLAG_BANKER |
                UNIT_NPC_FLAG_INNKEEPER | UNIT_NPC_FLAG_GUARD | UNIT_NPC_FLAG_SPIRITHEALER |
                UNIT_NPC_FLAG_SPIRITGUIDE | UNIT_NPC_FLAG_TABARDDESIGNER | UNIT_NPC_FLAG_AUCTIONEER);
        }
        bool isSpiritService() const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER | UNIT_NPC_FLAG_SPIRITGUIDE); }

        //Need fix or use this
        bool isGuard() const  { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GUARD); }

        bool IsTaxiFlying()  const { return HasUnitState(UNIT_STAT_TAXI_FLIGHT); }

        bool IsInCombat()  const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT); }
        void CombatStart(Unit* target, bool initialAggro = true);
        void SetInCombatState(bool PvP, Unit* enemy = NULL);
        void SetInCombatWith(Unit* enemy);
        void ClearInCombat();
        uint32 GetCombatTimer() const { return m_CombatTimer; }

        bool HasAuraType(AuraType auraType) const;
        uint32 GetAurasAmountByType(AuraType auraType) const
        {
            return m_modAuras[auraType].size();
        }
        uint32 GetAurasAmountByMiscValue(AuraType auraType, uint32 misc);
        bool hasNegativeAuraWithInterruptFlag(uint32 flag);
        bool HasAuraTypeWithFamilyFlags(AuraType auraType, uint32 familyName,  uint64 familyFlags) const;
        bool HasAuraByCasterWithFamilyFlags(uint64 pCaster, uint32 familyName,  uint64 familyFlags, const Aura * except = NULL) const;
        bool HasAura(uint32 spellId, uint32 effIndex) const
        {
            return m_Auras.find(spellEffectPair(spellId, effIndex)) != m_Auras.end();
        }

        bool HasAura(uint32 spellId) const;

        bool virtual HasSpell(uint32 /*spellID*/) const { return false; }

        bool HasStealthAura()      const { return HasAuraType(SPELL_AURA_MOD_STEALTH); }
        bool HasInvisibilityAura() const { return HasAuraType(SPELL_AURA_MOD_INVISIBILITY); }
        bool isCrowdControlled() const { return HasUnitState(UNIT_STAT_LOST_CONTROL | UNIT_STAT_POSSESSED); }
        bool CantMove() const { return HasUnitState(UNIT_STAT_ROOT | UNIT_STAT_STUNNED); }
        bool isInRoots() const { return HasUnitState(UNIT_STAT_ROOT); }
        bool IsPolymorphed() const;

        bool isFrozen() const;

        bool isTargetableForAttack() const;
        bool isAttackableByAOE() const;
        bool canAttack(Unit const* target, bool force = true) const;
        virtual bool IsInWater() const;
        virtual bool IsUnderWater() const;
        bool isInAccessiblePlacefor (Creature const* c) const;

        void SendHealSpellLog(Unit *pVictim, uint32 SpellID, uint32 Damage, bool critical = false);
        void SendEnergizeSpellLog(Unit *pVictim, uint32 SpellID, uint32 Damage,Powers powertype);
        uint32 SpellNonMeleeDamageLog(Unit *pVictim, uint32 spellID, uint32 damage, bool isTriggeredSpell = false, bool useSpellDamage = true);
        SpellCastResult CastSpell(Unit* Victim, uint32 spellId, bool triggered, Item *castItem = NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        SpellCastResult CastSpell(Unit* Victim,SpellEntry const *spellInfo, bool triggered, Item *castItem= NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        SpellCastResult CastCustomSpell(Unit* Victim, uint32 spellId, int32 const* bp0, int32 const* bp1, int32 const* bp2, bool triggered, Item *castItem= NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        SpellCastResult CastCustomSpell(uint32 spellId, SpellValueMod mod, uint32 value, Unit* Victim = NULL, bool triggered = true, Item *castItem = NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        SpellCastResult CastCustomSpell(uint32 spellId, CustomSpellValues const &value, Unit* Victim = NULL, bool triggered = true, Item *castItem = NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        SpellCastResult CastSpell(float x, float y, float z, uint32 spellId, bool triggered, Item *castItem = NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        SpellCastResult CastSpell(GameObject *go, uint32 spellId, bool triggered, Item *castItem = NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        void AddAura(uint32 spellId, Unit *target);

        void DeMorph();

        void SendAttackStateUpdate(MeleeDamageLog *damageInfo);
        void SendAttackStateUpdate(uint32 HitInfo, Unit *target, uint8 SwingType, SpellSchoolMask damageSchoolMask, uint32 Damage, uint32 AbsorbDamage, uint32 Resist, VictimState TargetState, uint32 BlockedAmount);
        void SendSpellNonMeleeDamageLog(SpellDamageLog *log);
        void SendSpellNonMeleeDamageLog(Unit *target,uint32 SpellID,uint32 Damage, SpellSchoolMask damageSchoolMask,uint32 AbsorbedDamage, uint32 Resist,bool PhysicalDamage, uint32 Blocked, bool CriticalHit = false);
        void SendSpellMiss(Unit *target, uint32 spellID, SpellMissInfo missInfo);

        void NearTeleportTo(float x, float y, float z, float orientation, bool casting = false);

        void MonsterMove(float x, float y, float z);
        void MonsterMoveWithSpeed(float x, float y, float z, float speed, bool time=false, bool generatePath = false, bool forceDestination = false);

        //Sends mobs movement (and movement end?) to client
        void SendMonsterStop();
        //Sends visual updates to client, DOES NOT UPDATE ON SERVER SIDE;
        //for server side update use UpdateVisibilityAndView()
        void SendHeartBeat();

        void SetInFront(Unit const* target);
        void SetFacingTo(float ori);
        void SetFacingToObject(WorldObject* pObject);
        bool IsBehindTarget(Unit const* pTarget, bool strict = true) const;

        virtual void MoveOutOfRange(Player &) {};

        bool IsAlive() const { return (m_deathState == ALIVE); };
        bool isDying() const { return (m_deathState == JUST_DIED); };
        bool IsDead() const { return (m_deathState == DEAD || m_deathState == CORPSE); };
        DeathState GetDeathState() { return m_deathState; };
        virtual void setDeathState(DeathState s);           // overwrited in Creature/Player/Pet

        uint64 GetOwnerGUID() const { return  GetUInt64Value(UNIT_FIELD_SUMMONEDBY); }
        void SetOwnerGUID(uint64 owner) { SetUInt64Value(UNIT_FIELD_SUMMONEDBY, owner); }
        uint64 GetCreatorGUID() const { return GetUInt64Value(UNIT_FIELD_CREATEDBY); }
        void SetCreatorGUID(uint64 creator) { SetUInt64Value(UNIT_FIELD_CREATEDBY, creator); }
        uint64 GetPetGUID() const { return  GetUInt64Value(UNIT_FIELD_SUMMON); }
        uint64 GetCharmerGUID() const { return GetUInt64Value(UNIT_FIELD_CHARMEDBY); }
        void SetCharmerGUID(uint64 owner) { SetUInt64Value(UNIT_FIELD_CHARMEDBY, owner); }
        uint64 GetCharmGUID() const { return  GetUInt64Value(UNIT_FIELD_CHARM); }

        uint64 GetCharmerOrOwnerGUID() const { return GetCharmerGUID() ? GetCharmerGUID() : GetOwnerGUID(); }
        uint64 GetCharmerOrOwnerOrOwnGUID() const
        {
            if (uint64 guid = GetCharmerOrOwnerGUID())
                return guid;
            return GetGUID();
        }
        bool isCharmedOwnedByPlayerOrPlayer() const { return IS_PLAYER_GUID(GetCharmerOrOwnerOrOwnGUID()); }

        Player* GetSpellModOwner() const;

        Unit* GetOwner() const;
        Pet* GetPet() const;
        Unit* GetCharmer() const;
        Unit* GetCharm() const;
        Unit* GetCharmerOrOwner() const 
        {
            if (GetCharmer())
                return GetCharmer();
            else if (GetOwner())
                return GetOwner();
            else
                return NULL;
        }
        Unit* GetCharmerOrOwnerOrSelf() const
        {
            if (Unit *u = GetCharmerOrOwner())
                return u;

            return (Unit*)this;
        }
        Player* GetCharmerOrOwnerPlayerOrPlayerItself() const;
        float GetCombatDistance(const Unit* target) const;

        void SetPet(Pet* pet);
        void SetCharm(Unit* pet);

        void SetCharmedOrPossessedBy(Unit*, bool possess);
        void RemoveCharmedOrPossessedBy(Unit* charmer);

        void RestoreFaction();

        bool isCharmed() const { return GetCharmerGUID() != 0; }
        bool isPossessed() const { return HasUnitState(UNIT_STAT_POSSESSED); }
        bool isPossessedByPlayer() const { return HasUnitState(UNIT_STAT_POSSESSED) && IS_PLAYER_GUID(GetCharmerGUID()); }
        bool isPossessing() const
        {
            if (Unit *u = GetCharm())
                return u->isPossessed();
            else
                return false;
        }
        bool isPossessing(Unit* u) const { return u->isPossessed() && GetCharmGUID() == u->GetGUID(); }

        CharmInfo* GetCharmInfo() { return m_charmInfo; }
        CharmInfo* InitCharmInfo();
        void       DeleteCharmInfo();
        void UpdateCharmAI();

        void RemoveBindSightAuras();
        void RemoveCharmAuras();

        Pet* CreateTamedPetFrom(Creature* creatureTarget,uint32 spell_id = 0);

        bool AddAura(Aura *aur);

        void RemoveAura(AuraMap::iterator &i, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAura(uint32 spellId, uint32 effindex, Aura* except = NULL);
        void RemoveSingleAuraFromStackByDispel(uint32 spellId);
        void RemoveSingleAuraFromStack(uint32 spellId, uint32 effindex);
        void RemoveSingleAuraFromStackByCaster(uint32 spellId, uint32 effindex, uint64 casterGUID);
        void RemoveAurasDueToSpell(uint32 spellId, Aura* except = NULL);
        void RemoveAurasDueToItemSpell(Item* castItem,uint32 spellId);
        void RemoveAurasByCasterSpell(uint32 spellId, uint64 casterGUID);
        void SetAurasDurationByCasterSpell(uint32 spellId, uint64 casterGUID, int32 duration);
        Aura* GetAuraByCasterSpell(uint32 spellId, uint64 casterGUID);
        void RemoveAurasDueToSpellByDispel(uint32 spellId, uint64 casterGUID, Unit *dispeler);
        void RemoveAurasDueToSpellBySteal(uint32 spellId, uint64 casterGUID, Unit *stealer);
        void RemoveAurasDueToSpellByCancel(uint32 spellId);
        void RemoveAurasAtChanneledTarget(SpellEntry const* spellInfo, Unit * caster);
        void RemoveNotOwnSingleTargetAuras();
        void RemoveAurasWithFamilyFlagsAndTypeByCaster(uint32 familyName,  uint64 familyFlags, AuraType aurType, uint64 casterGUID);

        void RemoveSpellsCausingAura(AuraType auraType);
        void RemoveAuraTypeByCaster(AuraType auraType, uint64 casterGUID);
        void RemoveRankAurasDueToSpell(uint32 spellId);
        bool RemoveNoStackAurasDueToAura(Aura *Aur);
        bool CheckForStrongerAuras(Aura* Aur);
        void RemoveAurasWithAttribute(uint32 flags, bool notPassiveOnly = false);
        void RemoveAurasWithInterruptFlags(uint32 flags, uint32 except = 0, bool PositiveOnly = false);
        void RemoveAurasWithDispelType(DispelType type);
        void RemoveAurasDueToRaidTeleport();
        void RemoveAllAurasButPermanent();    // WARLOCK PET unbuff after resummon with current PET
        void RemoveMovementImpairingAuras();

        void RemoveAllAuras();
        void RemoveArenaAuras(bool onleave = false);
        void RemoveAllAurasOnDeath();
        void DelayAura(uint32 spellId, uint32 effindex, int32 delaytime);

        float GetResistanceBuffMods(SpellSchools school, bool positive) const { return GetFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+school); }
        void SetResistanceBuffMods(SpellSchools school, bool positive, float val) { SetFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+school,val); }
        void ApplyResistanceBuffModsMod(SpellSchools school, bool positive, float val, bool apply) { ApplyModSignedFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+school, val, apply); }
        void ApplyResistanceBuffModsPercentMod(SpellSchools school, bool positive, float val, bool apply) { ApplyPercentModFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+school, val, apply); }
        void InitStatBuffMods()
        {
            for (int i = STAT_STRENGTH; i < MAX_STATS; ++i) SetFloatValue(UNIT_FIELD_POSSTAT0+i, 0);
            for (int i = STAT_STRENGTH; i < MAX_STATS; ++i) SetFloatValue(UNIT_FIELD_NEGSTAT0+i, 0);
        }
        void ApplyStatBuffMod(Stats stat, float val, bool apply) { ApplyModSignedFloatValue((val > 0 ? UNIT_FIELD_POSSTAT0+stat : UNIT_FIELD_NEGSTAT0+stat), val, apply); }
        void ApplyStatPercentBuffMod(Stats stat, float val, bool apply)
        {
            ApplyPercentModFloatValue(UNIT_FIELD_POSSTAT0+stat, val, apply);
            ApplyPercentModFloatValue(UNIT_FIELD_NEGSTAT0+stat, val, apply);
        }
        void SetCreateStat(Stats stat, float val) { m_createStats[stat] = val; }
        void SetCreateHealth(uint32 val) { SetUInt32Value(UNIT_FIELD_BASE_HEALTH, val); }
        uint32 GetCreateHealth() const { return GetUInt32Value(UNIT_FIELD_BASE_HEALTH); }
        void SetCreateMana(uint32 val) { SetUInt32Value(UNIT_FIELD_BASE_MANA, val); }
        uint32 GetCreateMana() const { return GetUInt32Value(UNIT_FIELD_BASE_MANA); }
        uint32 GetCreatePowers(Powers power) const;
        float GetPosStat(Stats stat) const { return GetFloatValue(UNIT_FIELD_POSSTAT0+stat); }
        float GetNegStat(Stats stat) const { return GetFloatValue(UNIT_FIELD_NEGSTAT0+stat); }
        float GetCreateStat(Stats stat) const { return m_createStats[stat]; }

        void SetCurrentCastSpell(Spell * pSpell);
        virtual void ProhibitSpellSchool(SpellSchoolMask /*idSchoolMask*/, uint32 /*unTimeMs*/) { }

        void InterruptSpell(uint32 spellType, bool withDelayed = true, bool withInstant = true);
        void FinishSpell(CurrentSpellTypes spellType, bool ok = true);

        // set withDelayed to true to account delayed spells as cast
        // delayed+channeled spells are always accounted as cast
        // we can skip channeled or delayed checks using flags
        bool IsNonMeleeSpellCast(bool withDelayed, bool skipChanneled = false, bool skipAutorepeat = false) const;

        // set withDelayed to true to interrupt delayed spells too
        // delayed+channeled spells are always interrupted
        void InterruptNonMeleeSpells(bool withDelayed, uint32 spellid = 0, bool withInstant = true);

        Spell* FindCurrentSpellBySpellId(uint32 spell_id) const;
        int32 GetCurrentSpellCastTime(uint32 spell_id) const;
        uint32 GetCurrentSpellId() const;

        Spell* m_currentSpells[CURRENT_MAX_SPELL];

        Spell* GetCurrentSpell(CurrentSpellTypes type) const;
        SpellEntry const* GetCurrentSpellProto(CurrentSpellTypes type) const;

        ShapeshiftForm GetShapeshiftForm() const { return ShapeshiftForm(GetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_SHAPESHIFT)); }
        void  SetShapeshiftForm(ShapeshiftForm form) { SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_SHAPESHIFT, form); }
        bool IsInDisallowedMountForm() const { return m_form == FORM_CAT || m_form == FORM_TREE || m_form == FORM_TRAVEL || m_form == FORM_AQUA || m_form == FORM_BEAR || m_form == FORM_DIREBEAR || m_form == FORM_CREATUREBEAR || m_form == FORM_GHOSTWOLF || m_form == FORM_FLIGHT || m_form == FORM_FLIGHT_EPIC || m_form == FORM_MOONKIN; }

        ObjectGuid GetTotemGuid(TotemSlot slot) const { return ObjectGuid(m_TotemSlot[slot]); }
        Totem* GetTotem(TotemSlot slot) const;

        uint32 m_addDmgOnce;
        uint64 m_TotemSlot[MAX_TOTEM];
        uint64 m_ObjectSlot[4];
        uint32 m_detectInvisibilityMask;
        uint32 m_invisibilityMask;
        uint32 m_ShapeShiftFormSpellId;
        ShapeshiftForm m_form;
        bool IsInFeralForm(bool checkGhostWolf = false) const { return m_form == FORM_CAT || m_form == FORM_BEAR || m_form == FORM_DIREBEAR || (checkGhostWolf && m_form == FORM_GHOSTWOLF); }

        float m_modMeleeHitChance;
        float m_modRangedHitChance;
        float m_modSpellHitChance;
        int32 m_baseSpellCritChance;

        float m_threatModifier[MAX_SPELL_SCHOOL];
        float m_modAttackSpeedPct[3];

        float m_TempSpeed;

        // Event handler
        EventProcessor m_Events;
        EventProcessor* GetEvents();
        void KillAllEvents(bool force);
        void AddEvent(BasicEvent* Event, uint64 e_time, bool set_addtime = true);

        // stat system
        bool HandleStatModifier(UnitMods unitMod, UnitModifierType modifierType, float amount, bool apply);
        void SetModifierValue(UnitMods unitMod, UnitModifierType modifierType, float value) { m_auraModifiersGroup[unitMod][modifierType] = value; }
        float GetModifierValue(UnitMods unitMod, UnitModifierType modifierType) const;
        float GetTotalStatValue(Stats stat) const;
        float GetTotalAuraModValue(UnitMods unitMod) const;
        SpellSchools GetSpellSchoolByAuraGroup(UnitMods unitMod) const;
        Stats GetStatByAuraGroup(UnitMods unitMod) const;
        Powers GetPowerTypeByAuraGroup(UnitMods unitMod) const;
        bool CanModifyStats() const { return m_canModifyStats; }
        void SetCanModifyStats(bool modifyStats) { m_canModifyStats = modifyStats; }
        virtual bool UpdateStats(Stats stat) = 0;
        virtual bool UpdateAllStats() = 0;
        virtual void UpdateResistances(uint32 school) = 0;
        virtual void UpdateArmor() = 0;
        virtual void UpdateMaxHealth() = 0;
        virtual void UpdateMaxPower(Powers power) = 0;
        virtual void UpdateAttackPowerAndDamage(bool ranged = false) = 0;
        virtual void UpdateDamagePhysical(WeaponAttackType attType) = 0;
        float GetTotalAttackPowerValue(WeaponAttackType attType) const;
        float GetWeaponDamageRange(WeaponAttackType attType ,WeaponDamageRange type) const;
        void SetBaseWeaponDamage(WeaponAttackType attType ,WeaponDamageRange damageRange, float value) { m_weaponDamage[attType][damageRange] = value; }

        bool isInFront(Unit const* target,float distance, float arc = M_PI) const;
        bool isInFront(GameObject const* target,float distance, float arc = M_PI) const;
        bool isInBack(Unit const* target, float distance, float arc = M_PI) const;
        bool isInBack(GameObject const* target, float distance, float arc = M_PI) const;
        bool isInLine(Unit const* target, float distance) const;
        bool isInLine(GameObject const* target, float distance) const;

        bool isBetween(WorldObject *s, WorldObject *e, float offset = 1.5f) const;

#pragma region VisibilityRelocation

        UnitVisibility GetVisibility() const { return m_Visibility; }
        void SetVisibility(UnitVisibility x);
        void DestroyForNearbyPlayers();

        //Updates mobs and objects on SERVER SIDE, does not send update to CLIENTS;
        //use SendHeartBeat() to update visibility for clients
        void UpdateVisibilityAndView();

        // common function for visibility checks for player/creatures with detection code
        virtual bool canSeeOrDetect(Unit const* u, WorldObject const*, bool detect, bool inVisibleList = false, bool is3dDistance = true) const;

        bool canDetectInvisibilityOf(Unit const* u) const;
        bool canDetectStealthOf(Unit const* u, WorldObject const*) const;

        // virtual functions for all world objects types
        bool isVisibleForInState(Player const*, WorldObject const*, bool) const;
        bool isVisibleForOrDetect(Unit const*, WorldObject const*, bool, bool = false, bool = true) const;

        // function for low level grid visibility checks in player/creature cases
        virtual bool IsVisibleInGridForPlayer(Player const* pl) const = 0;

        void OnRelocated();
        void ScheduleAINotify(uint32 delay);
        bool IsAINotifyScheduled() const { return _AINotifyScheduled;}
        void _SetAINotifyScheduled(bool on) { _AINotifyScheduled = on;}

        Position _notifiedPosition;

        bool WorthHonor;

    private:
        bool _AINotifyScheduled;

#pragma endregion VisibilityRelocation

    public:
        AuraList      & GetSingleCastAuras()       { return m_scAuras; }
        AuraList const& GetSingleCastAuras() const { return m_scAuras; }
        SpellImmuneList m_spellImmune[MAX_SPELL_IMMUNITY];

        // Threat related methods
        bool CanHaveThreatList() const;
        void AddThreat(Unit* pVictim, float threat, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NORMAL, SpellEntry const *threatSpell = NULL);
        float ApplyTotalThreatModifier(float threat, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NORMAL);
        void DeleteThreatList();
        void TauntApply(Unit* pVictim);
        void TauntFadeOut(Unit *taunter);

        HostileRefManager& getHostileRefManager() { return _hostileRefManager; }
        ThreatManager& getThreatManager() { return _threatManager; }

        void addHatedBy(HostileReference* pHostileReference) { getHostileRefManager().insertFirst(pHostileReference); };
        void removeHatedBy(HostileReference* /*pHostileReference*/) { /* nothing to do yet */ }

        Aura* GetAura(uint32 spellId, uint32 effindex);
        AuraMap      & GetAuras()       { return m_Auras; }
        AuraMap const& GetAuras() const { return m_Auras; }
        AuraList const& GetAurasByType(AuraType type) const { return m_modAuras[type]; }
        void ApplyAuraProcTriggerDamage(Aura* aura, bool apply);

        int32 GetTotalAuraModifier(AuraType auratype) const;
        float GetTotalAuraMultiplier(AuraType auratype) const;
        int32 GetMaxPositiveAuraModifier(AuraType auratype) const;
        int32 GetMaxNegativeAuraModifier(AuraType auratype) const;

        int32 GetTotalAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        float GetTotalAuraMultiplierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        int32 GetMaxPositiveAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        int32 GetMaxNegativeAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;

        int32 GetTotalAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;
        float GetTotalAuraMultiplierByMiscValue(AuraType auratype, int32 misc_value) const;
        int32 GetMaxPositiveAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;
        int32 GetMaxNegativeAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;

        Aura* GetDummyAura(uint32 spell_id) const;
        uint32 GetInterruptMask() const { return m_interruptMask; }
        void AddInterruptMask(uint32 mask) { m_interruptMask |= mask; }
        void UpdateInterruptMask();

        uint32 GetDisplayId() { return GetUInt32Value(UNIT_FIELD_DISPLAYID); }
        void SetDisplayId(uint32 modelId);
        uint32 GetNativeDisplayId() { return GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID); }
        void SetNativeDisplayId(uint32 modelId) { SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, modelId); }
        void setTransForm(uint32 spellid) { m_transform = spellid;}
        uint32 getTransForm() const { return m_transform;}

        DynamicObject* GetDynObject(uint32 spellId, uint32 effIndex);
        DynamicObject* GetDynObject(uint32 spellId);
        void AddDynObject(DynamicObject* dynObj);
        void RemoveDynObject(uint32 spellid);
        void RemoveDynObjectWithGUID(uint64 guid) { m_dynObjGUIDs.remove(guid); }
        void RemoveAllDynObjects();

        GameObject* GetGameObject(uint32 spellId) const;
        void AddGameObject(GameObject* gameObj);
        void RemoveGameObject(GameObject* gameObj, bool del);
        void RemoveGameObject(uint32 spellid, bool del);
        void RemoveAllGameObjects();

        uint32 CalculateDamage(WeaponAttackType attType, bool normalized);
        float GetAPMultiplier(WeaponAttackType attType, bool normalized);
        void ModifyAuraState(AuraState flag, bool apply);
        bool HasAuraState(AuraState flag) const { return HasFlag(UNIT_FIELD_AURASTATE, 1<<(flag-1)); }
        void UnsummonAllTotems();
        int32 SpellBaseDamageBonus(SpellSchoolMask schoolMask);
        int32 SpellBaseHealingBonus(SpellSchoolMask schoolMask);
        int32 SpellBaseDamageBonusForVictim(SpellSchoolMask schoolMask, Unit *pVictim);
        int32 SpellBaseHealingBonusForVictim(SpellSchoolMask schoolMask, Unit *pVictim);
        uint32 SpellDamageBonus(Unit *pVictim, SpellEntry const *spellProto, uint32 damage, DamageEffectType damagetype, CasterModifiers *casterModifiers = NULL);
        uint32 SpellHealingBonus(SpellEntry const *spellProto, uint32 healamount, DamageEffectType damagetype, Unit *pVictim, CasterModifiers *casterModifiers = NULL);
        bool   isSpellCrit(Unit *pVictim, SpellEntry const *spellProto, SpellSchoolMask schoolMask, WeaponAttackType attackType = BASE_ATTACK, float extraChance = 0);
        uint32 SpellCriticalBonus(SpellEntry const *spellProto, uint32 damage, Unit *pVictim);

        void SetLastManaUse(uint32 spellCastTime) { m_lastManaUse = spellCastTime; }
        bool IsUnderLastManaUseEffect() const;

        void SetContestedPvP(Player *attackedPlayer = NULL);

        void MeleeDamageBonus(Unit *pVictim, uint32 *damage, WeaponAttackType attType, SpellEntry const *spellProto = NULL);
        uint32 GetCastingTimeForBonus(SpellEntry const *spellProto, DamageEffectType damagetype, uint32 CastingTime);

        void ApplySpellImmune(uint32 spellId, uint32 op, uint32 type, bool apply);
        void ApplySpellDispelImmunity(const SpellEntry * spellProto, DispelType type, bool apply);
        virtual bool IsImmunedToSpell(SpellEntry const* spellInfo, bool useCharges = false);
                                                            // redefined in Creature
        bool IsImmunedToDamage(SpellSchoolMask meleeSchoolMask, bool useCharges = false);
        virtual bool IsImmunedToSpellEffect(uint32 effect, uint32 mechanic) const;
                                                            // redefined in Creature
        bool IsTotalImmune() const;
        bool IsImmuneToMechanic(Mechanics mechanic) const;

        uint32 CalcArmorReducedDamage(Unit* pVictim, const uint32 damage);
        void CalcAbsorbResist(Unit *pVictim, SpellSchoolMask schoolMask, DamageEffectType damagetype, const uint32 & damage, uint32 *absorb, uint32 *resist);
        void CalcAbsorb(Unit *pVictim, SpellSchoolMask schoolMask, const uint32 damage, uint32 *absorb, uint32 *resist);
        bool CalcBinaryResist(Unit *pVictim, SpellSchoolMask schoolMask);

        void  UpdateSpeed(UnitMoveType mtype, bool forced);
        float GetSpeed(UnitMoveType mtype) const;
        float GetMaxSpeedRate(UnitMoveType mtype) const { return m_max_speed_rate[mtype]; }
        float GetSpeedRate(UnitMoveType mtype) const { return m_speed_rate[mtype]; }
        void SetSpeed(UnitMoveType mtype, float rate, bool forced = false);

        void KnockBackFrom(Unit* target, float horizontalSpeed, float verticalSpeed);
        void KnockBack(float angle, float horizontalSpeed, float verticalSpeed);
        void GetLeapForwardDestination(Position& pos, float distance);

        void _RemoveAllAuraMods();
        void _ApplyAllAuraMods();

        int32 CalculateSpellDamage(SpellEntry const* spellProto, uint8 effect_index, int32 basePoints, Unit const* target);
        int32 CalculateSpellDuration(SpellEntry const* spellProto, uint8 effect_index, Unit const* target);
        float CalculateLevelPenalty(SpellEntry const* spellProto) const;
        void ModSpellCastTime(SpellEntry const* spellProto, int32 & castTime, Spell * spell);

        void addFollower(FollowerReference* pRef) { m_FollowingRefManager.insertFirst(pRef); }
        void removeFollower(FollowerReference* /*pRef*/) { /* nothing to do yet */ }
        static Unit* GetUnit(WorldObject& object, uint64 guid);
        static Unit* GetUnit(const Unit& unit, uint64 guid);
        static Player* GetPlayer(uint64 guid);
        static Creature* GetCreature(WorldObject& object, uint64 guid);

        Unit* GetUnit(uint64 guid);
        Creature* GetCreature(uint64 guid);
        Player* GetPlayerByName(const char *name);

        MotionMaster* GetMotionMaster() { return &i_motionMaster; }
        UnitStateMgr& GetUnitStateMgr() { return m_stateMgr; }

        void SetFeared(bool apply,  Unit*, uint32 = 0);
        void SetConfused(bool apply);
        void SetStunned(bool apply);
        void SetRooted(bool apply);

        bool IsStopped() const;

        virtual bool SetPosition(float x, float y, float z, float ang, bool teleport = false);

        void StopMoving();

        void AddUnitMovementFlag(uint32 f) { m_movementInfo.AddMovementFlag(MovementFlags(f)); }
        void RemoveUnitMovementFlag(uint32 f) { m_movementInfo.RemoveMovementFlag(MovementFlags(f)); }
        uint32 HasUnitMovementFlag(uint32 f) const { return m_movementInfo.HasMovementFlag(MovementFlags(f)); }
        uint32 GetUnitMovementFlags() const { return m_movementInfo.GetMovementFlags(); }
        void SetUnitMovementFlags(uint32 f) { m_movementInfo.SetMovementFlags(MovementFlags(f)); }

        bool IsLevitating() const { return m_movementInfo.HasMovementFlag(MOVEFLAG_LEVITATING); }
        bool IsFlying() const { return m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING); }
        bool IsWalking() const { return m_movementInfo.HasMovementFlag(MOVEFLAG_WALK_MODE); }
        bool IsWalkingBackward() const { return m_movementInfo.HasMovementFlag(MOVEFLAG_BACKWARD); }
        bool IsMoving() const { return m_movementInfo.HasMovementFlag(MOVEFLAG_MOVING); }
        bool IsSwimming() const { return m_movementInfo.HasMovementFlag(MOVEFLAG_SWIMMING); }
        bool IsMovingButNotWalking() const { return IsMoving() && !(IsWalking() || IsWalkingBackward()); }

        void AddComboPointHolder(uint32 lowguid) { m_ComboPointHolders.insert(lowguid); }
        void RemoveComboPointHolder(uint32 lowguid) { m_ComboPointHolders.erase(lowguid); }
        void ClearComboPointHolders();

        ///----------Pet responses methods-----------------
        void SendPetCastFail(uint32 spellid, SpellCastResult msg);
        void SendPetActionFeedback (uint8 msg);
        void SendPetTalk (uint32 pettalk);
        void SendPetAIReaction(uint64 guid);
        ///----------End of Pet responses methods----------

        void propagateSpeedChange() { GetMotionMaster()->propagateSpeedChange(); }

        // reactive attacks
        void ClearAllReactives();
        void StartReactiveTimer(ReactiveType reactive) { m_reactiveTimer[reactive] = REACTIVE_TIMER_START;}
        void UpdateReactives(uint32 p_time);

        // group updates
        void UpdateAuraForGroup(uint8 slot);

        // pet auras
        typedef std::set<PetAura const*> PetAuraSet;
        PetAuraSet m_petAuras;
        void AddPetAura(PetAura const* petSpell);
        void RemovePetAura(PetAura const* petSpell);

        void SetReducedThreatPercent(uint32 pct, uint64 guid)
        {
            m_reducedThreatPercent = pct;
            m_misdirectionTargetGUID = guid;
        }

        uint32 GetReducedThreatPercent() { return m_reducedThreatPercent; }
        Unit *GetMisdirectionTarget() { return m_misdirectionTargetGUID ? GetUnit(*this, m_misdirectionTargetGUID) : NULL; }

        void ApplyMeleeAPAttackerBonus(int32 value, bool apply);
        int32 GetMeleeApAttackerBonus() { return m_meleeAPAttackerBonus; }

        virtual float GetFollowAngle() const { return PET_FOLLOW_ANGLE; }
        void SetFlying(bool apply);

        bool HasEventAISummonedUnits ();

        bool preventApplyPersistentAA(SpellEntry const *spellInfo, uint8 eff_index);

        bool IsAIEnabled, NeedChangeAI;

        float GetDeterminativeSize() const;

        void SetGMToSendCombatStats(uint64 guid, uint32 flag) { m_GMToSendCombatStats = guid; m_CombatStatsFlag = flag; }
        void SendCombatStats(uint32 flag, const char* str, Unit *pVictim, ...) const;

        bool RollPRD(float baseChance, float extraChance, uint32 spellId);

        // Movement info
        Movement::MoveSpline * movespline;
        MovementInfo m_movementInfo;

        void TriggerAutocastSpell();

    protected:
        explicit Unit ();

        UnitAI *i_AI, *i_disabledAI;

        bool m_AI_locked;

        void _UpdateSpells(uint32 time);
        void _DeleteAuras();

        void _UpdateAutoRepeatSpell();
        bool m_AutoRepeatFirstCast;

        uint32 m_attackTimer[MAX_ATTACK];

        float m_createStats[MAX_STATS];

        AttackerSet m_attackers;
        Unit* m_attacking;

        DeathState m_deathState;

        AuraMap m_Auras;
        AuraMap::iterator m_AurasUpdateIterator;
        uint32 m_removedAurasCount;

        typedef std::list<uint64> DynObjectGUIDs;
        DynObjectGUIDs m_dynObjGUIDs;

        typedef std::list<GameObject*> GameObjectList;
        GameObjectList m_gameObj;
        bool m_isSorted;
        uint32 m_transform;
        AuraList m_removedAuras;

        AuraList *m_modAuras;
        AuraList m_scAuras;                        // cast singlecast auras
        AuraList m_interruptableAuras;
        AuraList m_ccAuras;
        uint32 m_interruptMask;

        float m_auraModifiersGroup[UNIT_MOD_END][MODIFIER_TYPE_END];
        float m_weaponDamage[MAX_ATTACK][2];
        bool m_canModifyStats;

        float m_speed_rate[MAX_MOVE_TYPE];                      // current speed
        float m_max_speed_rate[MAX_MOVE_TYPE];                  // max possible speed

        CharmInfo *m_charmInfo;

        virtual SpellSchoolMask GetMeleeDamageSchoolMask() const;

        MotionMaster i_motionMaster;
        UnitStateMgr m_stateMgr;

        uint32 m_reactiveTimer[MAX_REACTIVE];
        uint32 m_regenTimer;

        int32 m_meleeAPAttackerBonus;

    public:
        void DisableSpline();
        uint32 m_state;                                     // Even derived shouldn't modify

    private:
        bool IsTriggeredAtSpellProcEvent(Aura* aura, SpellEntry const* procSpell, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, bool isVictim, bool active, SpellProcEventEntry const*& spellProcEvent);
        bool HandleDummyAuraProc(  Unit *pVictim, uint32 damage, Aura* triggredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        bool HandleHasteAuraProc(  Unit *pVictim, uint32 damage, Aura* triggredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        bool HandleProcTriggerSpell(Unit *pVictim, uint32 damage, Aura* triggredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        bool HandleOverrideClassScriptAuraProc(Unit *pVictim, Aura* triggredByAura, SpellEntry const *procSpell, uint32 cooldown);
        bool HandleMendingAuraProc(Aura* triggeredByAura);
        bool HandleMendingNPCAuraProc(Aura* triggeredByAura);

        uint32 m_CombatTimer;
        uint32 m_lastManaUse;                               // msecs

        UnitVisibility m_Visibility;

        Diminishing m_Diminishing;

        // Manage all Units that are threatened by us
        HostileRefManager _hostileRefManager;
        ThreatManager _threatManager;

        FollowerRefManager m_FollowingRefManager;

        ComboPointHolderSet m_ComboPointHolders;

        uint32 m_reducedThreatPercent;
        uint64 m_misdirectionTargetGUID;

        uint32 m_procDeep;

        uint64 m_GMToSendCombatStats;
        uint32 m_CombatStatsFlag;
        UNORDERED_MAP<uint32, uint32> m_PRDMap;

        void UpdateSplineMovement(uint32 t_diff);
        TimeTrackerSmall m_movesplineTimer;
};

typedef std::set<Unit*> UnitSet;
typedef std::list<Unit*> UnitList;

namespace MaNGOS
{
    template<class T>
    void RandomResizeList(std::list<T> &_list, uint32 _size)
    {
        while (_list.size() > _size)
        {
            typename std::list<T>::iterator itr = _list.begin();
            advance(itr, urand(0, _list.size() - 1));
            _list.erase(itr);
        }
    }
}

#endif
