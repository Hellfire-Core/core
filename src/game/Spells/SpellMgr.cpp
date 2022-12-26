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

#include "Unit.h"
#include "SpellMgr.h"
#include "ObjectMgr.h"
#include "SpellAuraDefines.h"
#include "ProgressBar.h"
#include "DBCStores.h"
#include "World.h"
#include "Chat.h"
#include "Spell.h"
#include "CreatureAI.h"
#include "BattleGroundMgr.h"

bool IsAreaEffectTarget[TOTAL_SPELL_TARGETS];

SpellMgr::SpellMgr()
{
    for (int i = 0; i < TOTAL_SPELL_EFFECTS; ++i)
    {
        switch (i)
        {
            case SPELL_EFFECT_PERSISTENT_AREA_AURA: //27
            case SPELL_EFFECT_SUMMON:               //28
            case SPELL_EFFECT_TRIGGER_MISSILE:      //32
            case SPELL_EFFECT_SUMMON_WILD:          //41
            case SPELL_EFFECT_SUMMON_GUARDIAN:      //42
            case SPELL_EFFECT_TRANS_DOOR:           //50 summon object
            case SPELL_EFFECT_SUMMON_PET:           //56
            case SPELL_EFFECT_ADD_FARSIGHT:         //72
            case SPELL_EFFECT_SUMMON_POSSESSED:     //73
            case SPELL_EFFECT_SUMMON_TOTEM:         //74
            case SPELL_EFFECT_SUMMON_OBJECT_WILD:   //76
            case SPELL_EFFECT_SUMMON_TOTEM_SLOT1:   //87
            case SPELL_EFFECT_SUMMON_TOTEM_SLOT2:   //88
            case SPELL_EFFECT_SUMMON_TOTEM_SLOT3:   //89
            case SPELL_EFFECT_SUMMON_TOTEM_SLOT4:   //90
            case SPELL_EFFECT_SUMMON_CRITTER:       //97
            case SPELL_EFFECT_SUMMON_OBJECT_SLOT1:  //104
            case SPELL_EFFECT_SUMMON_OBJECT_SLOT2:  //105
            case SPELL_EFFECT_SUMMON_OBJECT_SLOT3:  //106
            case SPELL_EFFECT_SUMMON_OBJECT_SLOT4:  //107
            case SPELL_EFFECT_RESURRECT_PET:        //109
            case SPELL_EFFECT_SUMMON_DEMON:         //112
            case SPELL_EFFECT_TRIGGER_SPELL_2:      //151 ritual of summon
                EffectTargetType[i] = SPELL_REQUIRE_DEST;
                break;
            case SPELL_EFFECT_PARRY: // 0
            case SPELL_EFFECT_BLOCK: // 0
            case SPELL_EFFECT_SKILL: // always with dummy 3 as A
            //case SPELL_EFFECT_LEARN_SPELL: // 0 may be 5 pet
            case SPELL_EFFECT_TRADE_SKILL: // 0 or 1
            case SPELL_EFFECT_PROFICIENCY: // 0
                EffectTargetType[i] = SPELL_REQUIRE_NONE;
                break;
            case SPELL_EFFECT_ENCHANT_ITEM:
            case SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY:
            case SPELL_EFFECT_DISENCHANT:
            case SPELL_EFFECT_FEED_PET:
            case SPELL_EFFECT_PROSPECTING:
                EffectTargetType[i] = SPELL_REQUIRE_ITEM;
                break;
            //caster must be pushed otherwise no sound
            case SPELL_EFFECT_APPLY_AREA_AURA_PARTY:
            case SPELL_EFFECT_APPLY_AREA_AURA_FRIEND:
            case SPELL_EFFECT_APPLY_AREA_AURA_ENEMY:
            case SPELL_EFFECT_APPLY_AREA_AURA_PET:
            case SPELL_EFFECT_APPLY_AREA_AURA_OWNER:
                EffectTargetType[i] = SPELL_REQUIRE_CASTER;
                break;
            default:
                EffectTargetType[i] = SPELL_REQUIRE_UNIT;
                break;
        }
    }

    for (int i = 0; i < TOTAL_SPELL_TARGETS; ++i)
    {
        switch (i)
        {
            case TARGET_UNIT_CASTER:
            case TARGET_UNIT_CASTER_FISHING:
            case TARGET_UNIT_MASTER:
            case TARGET_UNIT_PET:
            case TARGET_UNIT_PARTY_CASTER:
            case TARGET_UNIT_RAID_CASTER:
                SpellTargetType[i] = TARGET_TYPE_UNIT_CASTER;
                break;
            case TARGET_UNIT_MINIPET:
            case TARGET_UNIT_TARGET_ALLY:
            case TARGET_UNIT_TARGET_RAID:
            case TARGET_UNIT_TARGET_ANY:
            case TARGET_UNIT_TARGET_ENEMY:
            case TARGET_UNIT_TARGET_PARTY:
            case TARGET_UNIT_PARTY_TARGET:
            case TARGET_UNIT_CLASS_TARGET:
            case TARGET_UNIT_CHAINHEAL:
                SpellTargetType[i] = TARGET_TYPE_UNIT_TARGET;
                break;
            case TARGET_UNIT_NEARBY_ENEMY:
            case TARGET_UNIT_NEARBY_ALLY:
            case TARGET_UNIT_NEARBY_ALLY_UNK:
            case TARGET_UNIT_NEARBY_ENTRY:
            case TARGET_UNIT_NEARBY_RAID:
            case TARGET_OBJECT_USE:
                SpellTargetType[i] = TARGET_TYPE_UNIT_NEARBY;
                break;
            case TARGET_UNIT_AREA_ENEMY_SRC:
            case TARGET_UNIT_AREA_ALLY_SRC:
            case TARGET_UNIT_AREA_ENTRY_SRC:
            case TARGET_UNIT_AREA_PARTY_SRC:
            case TARGET_OBJECT_AREA_SRC:
                SpellTargetType[i] = TARGET_TYPE_AREA_SRC;
                break;
            case TARGET_UNIT_AREA_ENEMY_DST:
            case TARGET_UNIT_AREA_ALLY_DST:
            case TARGET_UNIT_AREA_ENTRY_DST:
            case TARGET_UNIT_AREA_PARTY_DST:
            case TARGET_OBJECT_AREA_DST:
                SpellTargetType[i] = TARGET_TYPE_AREA_DST;
                break;
            case TARGET_UNIT_CONE_ENEMY:
            case TARGET_UNIT_CONE_ALLY:
            case TARGET_UNIT_CONE_ENTRY:
            case TARGET_UNIT_CONE_ENEMY_UNKNOWN:
                SpellTargetType[i] = TARGET_TYPE_AREA_CONE;
                break;
            case TARGET_DST_CASTER:
            case TARGET_SRC_CASTER:
            case TARGET_MINION:
            case TARGET_DEST_CASTER_FRONT_LEAP:
            case TARGET_DEST_CASTER_FRONT:
            case TARGET_DEST_CASTER_BACK:
            case TARGET_DEST_CASTER_RIGHT:
            case TARGET_DEST_CASTER_LEFT:
            case TARGET_DEST_CASTER_FRONT_LEFT:
            case TARGET_DEST_CASTER_BACK_LEFT:
            case TARGET_DEST_CASTER_BACK_RIGHT:
            case TARGET_DEST_CASTER_FRONT_RIGHT:
            case TARGET_DEST_CASTER_RANDOM:
            case TARGET_DEST_CASTER_RADIUS:
                SpellTargetType[i] = TARGET_TYPE_DEST_CASTER;
                break;
            case TARGET_DST_TARGET_ENEMY:
            case TARGET_DEST_TARGET_ANY:
            case TARGET_DEST_TARGET_FRONT:
            case TARGET_DEST_TARGET_BACK:
            case TARGET_DEST_TARGET_RIGHT:
            case TARGET_DEST_TARGET_LEFT:
            case TARGET_DEST_TARGET_FRONT_LEFT:
            case TARGET_DEST_TARGET_BACK_LEFT:
            case TARGET_DEST_TARGET_BACK_RIGHT:
            case TARGET_DEST_TARGET_FRONT_RIGHT:
            case TARGET_DEST_TARGET_RANDOM:
            case TARGET_DEST_TARGET_RADIUS:
                SpellTargetType[i] = TARGET_TYPE_DEST_TARGET;
                break;
            case TARGET_DEST_DYNOBJ_ENEMY:
            case TARGET_DEST_DYNOBJ_ALLY:
            case TARGET_DEST_DYNOBJ_NONE:
            case TARGET_DEST_DEST:
            case TARGET_DEST_TRAJ:
            case TARGET_DEST_DEST_FRONT_LEFT:
            case TARGET_DEST_DEST_BACK_LEFT:
            case TARGET_DEST_DEST_BACK_RIGHT:
            case TARGET_DEST_DEST_FRONT_RIGHT:
            case TARGET_DEST_DEST_FRONT:
            case TARGET_DEST_DEST_BACK:
            case TARGET_DEST_DEST_RIGHT:
            case TARGET_DEST_DEST_LEFT:
            case TARGET_DEST_DEST_RANDOM:
                SpellTargetType[i] = TARGET_TYPE_DEST_DEST;
                break;
            case TARGET_DST_DB:
            case TARGET_DST_HOME:
            case TARGET_DST_NEARBY_ENTRY:
                SpellTargetType[i] = TARGET_TYPE_DEST_SPECIAL;
                break;
            case TARGET_UNIT_CHANNEL:
            case TARGET_DEST_CHANNEL:
                SpellTargetType[i] = TARGET_TYPE_CHANNEL;
                break;
            default:
                SpellTargetType[i] = TARGET_TYPE_DEFAULT;
        }
    }

    for (int i = 0; i < TOTAL_SPELL_TARGETS; ++i)
    {
        switch (i)
        {
            case TARGET_UNIT_AREA_ENEMY_DST:
            case TARGET_UNIT_AREA_ENEMY_SRC:
            case TARGET_UNIT_AREA_ALLY_DST:
            case TARGET_UNIT_AREA_ALLY_SRC:
            case TARGET_UNIT_AREA_ENTRY_DST:
            case TARGET_UNIT_AREA_ENTRY_SRC:
            case TARGET_UNIT_AREA_PARTY_DST:
            case TARGET_UNIT_AREA_PARTY_SRC:
            case TARGET_UNIT_PARTY_TARGET:
            case TARGET_UNIT_PARTY_CASTER:
            case TARGET_UNIT_CONE_ENEMY:
            case TARGET_UNIT_CONE_ALLY:
            case TARGET_UNIT_CONE_ENEMY_UNKNOWN:
            case TARGET_UNIT_RAID_CASTER:
                IsAreaEffectTarget[i] = true;
                break;
            default:
                IsAreaEffectTarget[i] = false;
                break;
        }
    }
}

SpellMgr::~SpellMgr()
{
}

int32 SpellMgr::GetSpellDuration(SpellEntry const *spellInfo)
{
    if (!spellInfo)
        return 0;
    SpellDurationEntry const *du = sSpellDurationStore.LookupEntry(spellInfo->DurationIndex);
    if (!du)
        return 0;
    return (du->Duration[0] == -1) ? -1 : abs(du->Duration[0]);
}

int32 SpellMgr::GetSpellMaxDuration(SpellEntry const *spellInfo)
{
    if (!spellInfo)
        return 0;
    SpellDurationEntry const *du = sSpellDurationStore.LookupEntry(spellInfo->DurationIndex);
    if (!du)
        return 0;
    return (du->Duration[2] == -1) ? -1 : abs(du->Duration[2]);
}

uint32 SpellMgr::GetSpellBaseCastTime(SpellEntry const *spellInfo)
{
    SpellCastTimesEntry const *spellCastTimeEntry = sSpellCastTimesStore.LookupEntry(spellInfo->CastingTimeIndex);
    if (!spellCastTimeEntry)
        return 0;
    return spellCastTimeEntry->CastTime;
}

uint32 SpellMgr::GetSpellCastTime(SpellEntry const* spellInfo, Spell const* spell)
{
    int32 castTime = SpellMgr::GetSpellBaseCastTime(spellInfo);

    // if castTime == 0 no sense to apply modifiers
    if (!castTime)
        return 0;

    if (spell)
    {
        if (Player* modOwner = spell->GetCaster()->GetSpellModOwner())
            modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_CASTING_TIME, castTime, spell);

        if (!(spellInfo->Attributes & (SPELL_ATTR_ABILITY|SPELL_ATTR_TRADESPELL)))
            castTime = int32(castTime * spell->GetCaster()->GetFloatValue(UNIT_MOD_CAST_SPEED));
        else
        {
            if (spell->IsRangedSpell() && !spell->IsAutoRepeat() && !(spell->GetCaster()->getClassMask() & CLASSMASK_WAND_USERS))
                castTime = int32(castTime * spell->GetCaster()->m_modAttackSpeedPct[RANGED_ATTACK]);
        }
    }

    if (spellInfo->Attributes & SPELL_ATTR_RANGED && (!spell || !(spell->IsAutoRepeat())))
        castTime += 500;

    return (castTime > 0) ? uint32(castTime) : 0;
}

bool SpellMgr::IsPassiveSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if (!spellInfo)
        return false;
    return (spellInfo->Attributes & SPELL_ATTR_PASSIVE) != 0;
}

bool SpellMgr::IsPassiveSpell(SpellEntry const *spellInfo)
{
    return (spellInfo->Attributes & SPELL_ATTR_PASSIVE) != 0;
}

void SpellMgr::ApplySpellThreatModifiers(SpellEntry const *spellInfo, float &threat)
{
    if (!spellInfo)
        return;

    if (spellInfo->SpellFamilyName == SPELLFAMILY_WARLOCK && spellInfo->SpellFamilyFlags & 0x100LL) // Searing Pain
        threat *= 2.0f;

    else if (spellInfo->SpellFamilyName == SPELLFAMILY_SHAMAN && spellInfo->SpellFamilyFlags & SPELLFAMILYFLAG_SHAMAN_FROST_SHOCK)
        threat *= 2.0f;

    else if (spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN && spellInfo->SpellFamilyFlags & 0x4000000000LL) // Holy shield
        threat *= 1.35f;

    else if (spellInfo->SpellFamilyName == SPELLFAMILY_DRUID && spellInfo->SpellFamilyFlags & 0x10000000000LL) // Lacerate
        threat *= 0.20f;

    else if (spellInfo->SpellFamilyName == SPELLFAMILY_PRIEST && spellInfo->SpellFamilyFlags & 0x8400000LL)    // Holy Nova
        threat = 1.0f;

    else if (spellInfo->Id == 31616) // Nature's Guardian - shaman talent
        threat *= 0.9f;

    else if (spellInfo->Id == 33763) // Lifebloom HOT, Last instant heal counts as zero threat (at least should count as zero)
        threat *= 0.47f;

    else if (spellInfo->Id == 32546) // Binging heal
        threat *= 0.5f;

    else if (spellInfo->Id == 20647) // Execute
        threat *= 1.25f;

    else if (spellInfo->SpellFamilyName == SPELLFAMILY_PRIEST && spellInfo->SpellFamilyFlags & 0x1000000000LL) // Chastise
        threat *= 0.5f;

    else if (spellInfo->Id == 37661) // Lightning Capacitor
        threat *= 0.5f;

    else if (spellInfo->SpellFamilyName == SPELLFAMILY_WARRIOR && spellInfo->SpellFamilyFlags & 0x80LL) // Thunder Clap
        threat *= 1.75f;
}

uint32 SpellMgr::CalculatePowerCost(SpellEntry const * spellInfo, Unit const * caster, SpellSchoolMask schoolMask, Spell* spell)
{
    // Spell drain all exist power on cast (Only paladin lay of Hands)
    if (spellInfo->AttributesEx & SPELL_ATTR_EX_DRAIN_ALL_POWER)
    {
        // If power type - health drain all
        if (spellInfo->powerType == POWER_HEALTH)
            return caster->GetHealth();
        // Else drain all power
        if (spellInfo->powerType < MAX_POWERS)
            return caster->GetPower(Powers(spellInfo->powerType));
        sLog.outLog(LOG_DEFAULT, "ERROR: Spell::CalculateManaCost: Unknown power type '%d' in spell %d", spellInfo->powerType, spellInfo->Id);
        return 0;
    }

    // Base powerCost
    int32 powerCost = spellInfo->manaCost;
    // PCT cost from total amount
    if (spellInfo->ManaCostPercentage)
    {
        switch (spellInfo->powerType)
        {
            // health as power used
            case POWER_HEALTH:
                powerCost += spellInfo->ManaCostPercentage * caster->GetCreateHealth() / 100;
                break;
            case POWER_MANA:
                powerCost += spellInfo->ManaCostPercentage * caster->GetCreateMana() / 100;
                break;
            case POWER_RAGE:
            case POWER_FOCUS:
            case POWER_ENERGY:
            case POWER_HAPPINESS:
                //            case POWER_RUNES:
                powerCost += spellInfo->ManaCostPercentage * caster->GetMaxPower(Powers(spellInfo->powerType)) / 100;
            break;
            default:
                sLog.outLog(LOG_DEFAULT, "ERROR: Spell::CalculateManaCost: Unknown power type '%d' in spell %d", spellInfo->powerType, spellInfo->Id);
                return 0;
        }
    }
    SpellSchools school = GetFirstSchoolInMask(schoolMask);
    // Flat mod from caster auras by spell school
    powerCost += caster->GetInt32Value(UNIT_FIELD_POWER_COST_MODIFIER + school);
    // Shiv - costs 20 + weaponSpeed*10 energy (apply only to non-triggered spell with energy cost)
    if (spellInfo->AttributesEx4 & SPELL_ATTR_EX4_SPELL_VS_EXTEND_COST)
        powerCost += caster->GetAttackTime(OFF_ATTACK)/100;
    // Apply cost mod by spell
    if (Player* modOwner = caster->GetSpellModOwner())
    {
        // surge of light special case (hacky, but this is only spell with percentage bonus to mana cost)
        if (modOwner->HasAura(33151) && sSpellMgr.IsAffectedBySpell(spellInfo, 33151, 0, 0)) 
        {
            powerCost = 0;
            modOwner->RemoveAura(33151, 0);// remove only cost bonus, others will be removed on apply spell mod
        }
        else
            modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_COST, powerCost, spell);
    }

    if (spellInfo->Attributes & SPELL_ATTR_LEVEL_DAMAGE_CALCULATION)
        powerCost = int32(powerCost/ (1.117f* spellInfo->spellLevel / caster->GetLevel() -0.1327f));

    // PCT mod from user auras by school
    powerCost = int32(powerCost * (1.0f+caster->GetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER+school)));
    if (powerCost < 0)
        powerCost = 0;
    return powerCost;
}

int32 SpellMgr::CompareAuraRanks(uint32 spellId_1, uint32 effIndex_1, uint32 spellId_2, uint32 effIndex_2)
{
    SpellEntry const*spellInfo_1 = sSpellStore.LookupEntry(spellId_1);
    SpellEntry const*spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
    if (!spellInfo_1 || !spellInfo_2) return 0;
    if (spellId_1 == spellId_2) return 0;

    int32 diff = spellInfo_1->EffectBasePoints[effIndex_1] - spellInfo_2->EffectBasePoints[effIndex_2];
    if (spellInfo_1->CalculateSimpleValue(effIndex_1) < 0 && spellInfo_2->CalculateSimpleValue(effIndex_2) < 0) return -diff;
    else return diff;
}

SpellSpecific SpellMgr::GetSpellSpecific(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    return GetSpellSpecific(spellInfo);
}

SpellSpecific SpellMgr::GetSpellSpecific(SpellEntry const* spellInfo)
{
    if (!spellInfo)
        return SPELL_NORMAL;

    if (spellInfo->AttributesCu & SPELL_ATTR_CU_TREAT_AS_WELL_FED)
        return SPELL_WELL_FED;

    switch (spellInfo->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            //food/drink
            if (spellInfo->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED)
            {
                for (int i = 0; i < 3; i++)
                {
                    if (spellInfo->EffectApplyAuraName[i]==SPELL_AURA_MOD_POWER_REGEN)
                        return SPELL_DRINK;
                    else if (spellInfo->EffectApplyAuraName[i]==SPELL_AURA_MOD_REGEN)
                        return SPELL_FOOD;
                }
            }
            // this may be a hack
            else
            {
                if (spellInfo->AttributesEx2 & SPELL_ATTR_EX2_FOOD && !spellInfo->Category)
                    return SPELL_WELL_FED;
            }

            switch (spellInfo->Id)
            {
                case 12880: // warrior's Enrage rank 1
                case 14201: //           Enrage rank 2
                case 14202: //           Enrage rank 3
                case 14203: //           Enrage rank 4
                case 14204: //           Enrage rank 5
                case 12292: //             Death Wish
                    return SPELL_WARRIOR_ENRAGE;
                case 38245: //             Polymorph
                case 43309: //             Polymorph
                    return SPELL_MAGE_POLYMORPH;
                break;
                default: break;
            }
            break;
        }
        case SPELLFAMILY_MAGE:
        {
            // family flags 18(Molten), 25(Frost/Ice), 28(Mage)
            if (spellInfo->SpellFamilyFlags & 0x12040000)
                return SPELL_MAGE_ARMOR;

            if ((spellInfo->SpellFamilyFlags & 0x1000000) && spellInfo->EffectApplyAuraName[0]==SPELL_AURA_MOD_CONFUSE)
                return SPELL_MAGE_POLYMORPH;

            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            if (spellInfo->SpellFamilyFlags & 0x00008000010000LL)
                return SPELL_POSITIVE_SHOUT;

            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // only warlock curses have this
            if (spellInfo->Dispel == DISPEL_CURSE)
                return SPELL_CURSE;

            // family flag 37 (only part spells have family name)
            if (spellInfo->SpellFamilyFlags & 0x2000000000LL)
                return SPELL_WARLOCK_ARMOR;

            //seed of corruption and corruption
            if (spellInfo->SpellFamilyFlags & 0x1000000002LL)
                return SPELL_WARLOCK_CORRUPTION;
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // only hunter stings have this
            if (spellInfo->Dispel == DISPEL_POISON)
                return SPELL_STING;

            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            if (SpellMgr::IsSealSpell(spellInfo))
                return SPELL_SEAL;

            if (spellInfo->SpellFamilyFlags & 0x10000180LL)
                return SPELL_BLESSING;

            if ((spellInfo->SpellFamilyFlags & 0x00000820180400LL) && (spellInfo->AttributesEx3 & 0x200))
                return SPELL_JUDGEMENT;

            for (int i = 0; i < 3; i++)
            {
                // only paladin auras have this
                if (spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
                    return SPELL_AURA;
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            if (SpellMgr::IsElementalShield(spellInfo))
                return SPELL_ELEMENTAL_SHIELD;

            break;
        }

        case SPELLFAMILY_POTION:
            return sSpellMgr.GetSpellElixirSpecific(spellInfo->Id);
            break;
    }

    // only warlock armor/skin have this (in additional to family cases)
    if (spellInfo->SpellVisual == 130 && spellInfo->SpellIconID == 89)
    {
        return SPELL_WARLOCK_ARMOR;
    }

    // only hunter aspects have this (but not all aspects in hunter family)
    if (spellInfo->activeIconID == 122 && (GetSpellSchoolMask(spellInfo) & SPELL_SCHOOL_MASK_NATURE) &&
        (spellInfo->Attributes & 0x50000) != 0 && (spellInfo->Attributes & 0x9000010) == 0)
    {
        return SPELL_ASPECT;
    }

    for (int i = 0; i < 3; ++i)
    {
        if (spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA)
        {
            switch (spellInfo->EffectApplyAuraName[i])
            {
                case SPELL_AURA_TRACK_CREATURES:
                case SPELL_AURA_TRACK_RESOURCES:
                case SPELL_AURA_TRACK_STEALTHED:
                    return SPELL_TRACKER;
                case SPELL_AURA_MOD_CHARM:
                case SPELL_AURA_MOD_POSSESS_PET:
                case SPELL_AURA_MOD_POSSESS:
                    return SPELL_CHARM;
            }
        }
    }

    // elixirs can have different families, but potion most ofc.
    if (SpellSpecific sp = sSpellMgr.GetSpellElixirSpecific(spellInfo->Id))
        return sp;

    return SPELL_NORMAL;
}

bool SpellMgr::IsSingleFromSpellSpecificPerCaster(SpellSpecific spellSpec1,SpellSpecific spellSpec2)
{
    switch (spellSpec1)
    {
        case SPELL_SEAL:
        case SPELL_BLESSING:
        case SPELL_AURA:
        case SPELL_STING:
        case SPELL_CURSE:
        case SPELL_ASPECT:
        case SPELL_POSITIVE_SHOUT:
        case SPELL_JUDGEMENT:
        case SPELL_WARLOCK_CORRUPTION:
            return spellSpec1 == spellSpec2;
        default:
            return false;
    }
}

bool SpellMgr::IsSingleFromSpellSpecificPerTarget(SpellSpecific spellSpec1, SpellSpecific spellSpec2)
{
    switch (spellSpec1)
    {
        case SPELL_TRACKER:
        case SPELL_WARLOCK_ARMOR:
        case SPELL_MAGE_ARMOR:
        case SPELL_ELEMENTAL_SHIELD:
        case SPELL_MAGE_POLYMORPH:
        case SPELL_WELL_FED:
        case SPELL_DRINK:
        case SPELL_FOOD:
        case SPELL_CHARM:
        case SPELL_WARRIOR_ENRAGE:
            return spellSpec1 == spellSpec2;
        case SPELL_BATTLE_ELIXIR:
            return spellSpec2 == SPELL_BATTLE_ELIXIR
                || spellSpec2 == SPELL_FLASK_ELIXIR;
        case SPELL_GUARDIAN_ELIXIR:
            return spellSpec2 == SPELL_GUARDIAN_ELIXIR
                || spellSpec2 == SPELL_FLASK_ELIXIR;
        case SPELL_FLASK_ELIXIR:
            return spellSpec2 == SPELL_BATTLE_ELIXIR
                || spellSpec2 == SPELL_GUARDIAN_ELIXIR
                || spellSpec2 == SPELL_FLASK_ELIXIR;
        default:
            return false;
    }
}

bool SpellMgr::IsSingleFromSpellSpecificRanksPerTarget(SpellSpecific spellId_spec, SpellSpecific i_spellId_spec)
{
    switch (spellId_spec)
    {
        case SPELL_BLESSING:
        case SPELL_AURA:
        case SPELL_CURSE:
            return spellId_spec == i_spellId_spec;
        default:
            return false;
    }
}

bool SpellMgr::IsPositiveTarget(uint32 targetA, uint32 targetB)
{
    // non-positive targets
    switch (targetA)
    {
        case TARGET_UNIT_TARGET_ENEMY:
        case TARGET_UNIT_AREA_ENEMY_SRC:
        case TARGET_UNIT_AREA_ENEMY_DST:
        case TARGET_UNIT_CONE_ENEMY:
        case TARGET_DEST_DYNOBJ_ENEMY:
        case TARGET_DST_TARGET_ENEMY:
        case TARGET_UNIT_CHANNEL:
            return false;
        case TARGET_SRC_CASTER:
            return (targetB == TARGET_UNIT_AREA_PARTY_SRC || targetB == TARGET_UNIT_AREA_ALLY_SRC);
        default:
            break;
    }
    if (targetB)
        return SpellMgr::IsPositiveTarget(targetB, 0);
    return true;
}

bool SpellMgr::IsPositiveEffect(uint32 spellId, uint32 effIndex)
{
    SpellEntry const *spellproto = sSpellStore.LookupEntry(spellId);
    if (!spellproto)
        return false;

    // talents
    if (SpellMgr::IsPassiveSpell(spellId) && GetTalentSpellCost(spellId))
        return true;

    /*
    // explicit targeting set positiveness independent from real effect
    // Note: IsExplicitNegativeTarget can't be used symmetric (look some TARGET_SINGLE_ENEMY spells for example)
    if (IsExplicitPositiveTarget(spellproto->EffectImplicitTargetA[effIndex]) ||
        IsExplicitPositiveTarget(spellproto->EffectImplicitTargetB[effIndex]))
        return true;
    */

    // should this work fine?
    if (spellproto->Attributes & SPELL_ATTR_NEGATIVE_1)
        return false;
    // SPELL_CHECK_POSITIVE
    switch (spellId)
    {
        case 23333:                                         // BG spell
        case 23335:                                         // BG spell
        case 24732:                                         // Bat Costume
        case 24740:                                         // Wisp Costume
        case 34976:                                         // BG spell
        case 31579:                                         // Arcane Empowerment Rank1 talent aura with one positive and one negative (check not needed in wotlk)
        case 31582:                                         // Arcane Empowerment Rank2
        case 31583:                                         // Arcane Empowerment Rank3
        case 37441:                                         // Improved Arcane Blast
        case 12042:                                         // Arcane Power
        case 40268:                                         // Spiritual Vengeance
        case 40322:                                         // Spirit Shield
        case 41151:                                         // Lightning Shield
        case 34970:                                         // Frenzy
        case 43550:                                         // Mind Control (Hex Lord Malacrass)
        case 35336:                                         // Energizing Spores
        case 40604:                                         // Fel Rage 1
        case 40616:                                         // Fel Rage 2
        case 41625:                                         // Fel Rage 3
        case 46787:                                         // Fel Rage scale
        case 38318:                                         // Orb of Blackwhelp
        case 37851:                                         // tag greater felfire diemetradon
        case 37907:                                         // kill credit felfire diemetradon
        case 40825:                                         // blade edge banish the demons quest (dont get in combat with trigger)
            return true;
        case 46392:                                         // Focused Assault
        case 46393:                                         // Brutal Assault
        case 43437:                                         // Paralyzed
        case 28441:                                         // not positive dummy spell
        case 37675:                                         // Chaos Blast
        case 41519:                                         // Mark of Stormrage
        case 34877:                                         // Custodian of Time
        case 34700:                                         // Allergic Reaction
        case 31719:                                         // Suspension
        case 41406:                                         // Dementia +
        case 41409:                                         // Dementia -
        case 30529:                                         // Chess event: Recently In Game
        case 37469:
        case 37465:
        case 37128:                                         // Doomwalker - Mark of Death
        case 30421:                                         // Neterspite - Player buffs(3)
        case 30422:
        case 30423:
        case 30457:                                         // Complete Vulnerability
        case 47002:                                         // Noxious Fumes (not sure if needed, just in case)
        case 41350:                                         // Aura of Desire
        case 43501:                                         // Siphon Soul (Hexlord Spell)
            return false;
        case 32375:
            if(effIndex == 0)                               // Mass Dispel on friendly targets
                return true;
            else                                            // Mass Dispel on enemy targets
                return false;
    }

    switch (spellproto->SpellFamilyName)
    {
        case SPELLFAMILY_MAGE:
            // Amplify Magic, Dampen Magic
            if (spellproto->SpellFamilyFlags & 0x20000000000)
                return true;
            break;
        case SPELLFAMILY_HUNTER:
            // Aspect of the Viper
            if (spellproto->Id == 34074)
                return true;
            break;
        default:
            break;
    }

    switch (spellproto->Mechanic)
    {
        case MECHANIC_IMMUNE_SHIELD:
            return true;
        default:
            break;
    }

    switch (spellproto->Effect[effIndex])
    {
        // always positive effects (check before target checks that provided non-positive result in some case for positive effects)
        case SPELL_EFFECT_HEAL:
        case SPELL_EFFECT_LEARN_SPELL:
        case SPELL_EFFECT_SKILL_STEP:
        case SPELL_EFFECT_HEAL_PCT:
        case SPELL_EFFECT_ENERGIZE_PCT:
            return true;

            // non-positive aura use
        case SPELL_EFFECT_APPLY_AURA:
        case SPELL_EFFECT_APPLY_AREA_AURA_FRIEND:
        {
            switch (spellproto->EffectApplyAuraName[effIndex])
            {
                case SPELL_AURA_DUMMY:
                {
                    // dummy aura can be positive or negative dependent from cast spell
                    switch (spellproto->Id)
                    {
                        case 13139:                         // net-o-matic special effect
                        case 23445:                         // evil twin
                        case 35679:                         // Protectorate Demolitionist
                        case 38637:                         // Nether Exhaustion (red)
                        case 38638:                         // Nether Exhaustion (green)
                        case 38639:                         // Nether Exhaustion (blue)
                            return false;
                        default:
                            break;
                    }
                }   break;
                case SPELL_AURA_MOD_STAT:
                case SPELL_AURA_MOD_DAMAGE_DONE:            // dependent from bas point sign (negative -> negative)
                case SPELL_AURA_MOD_HEALING_DONE:
                    if (spellproto->CalculateSimpleValue(effIndex) < 0)
                        return false;
                    break;
                case SPELL_AURA_MOD_SPELL_CRIT_CHANCE:
                    if (spellproto->EffectBasePoints[effIndex] > 0)
                        return true;
                    break;
                case SPELL_AURA_ADD_TARGET_TRIGGER:
                    return true;
                case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                    if (spellId != spellproto->EffectTriggerSpell[effIndex])
                    {
                        uint32 spellTriggeredId = spellproto->EffectTriggerSpell[effIndex];
                        SpellEntry const *spellTriggeredProto = sSpellStore.LookupEntry(spellTriggeredId);

                        if (spellTriggeredProto)
                        {
                            // non-positive targets of main spell return early
                            for (int i = 0; i < 3; ++i)
                            {
                                // if non-positive trigger cast targeted to positive target this main cast is non-positive
                                // this will place this spell auras as debuffs
                                if (SpellMgr::IsPositiveTarget(spellTriggeredProto->EffectImplicitTargetA[effIndex],spellTriggeredProto->EffectImplicitTargetB[effIndex]) && !IsPositiveEffect(spellTriggeredId,i))
                                    return false;
                            }
                        }
                    }
                    break;
                case SPELL_AURA_PROC_TRIGGER_SPELL:
                    // many positive auras have negative triggered spells at damage for example and this not make it negative (it can be canceled for example)
                    break;
                case SPELL_AURA_MOD_STUN:                   //have positive and negative spells, we can't sort its correctly at this moment.
                    if (effIndex==0 && spellproto->Effect[1]==0 && spellproto->Effect[2]==0)
                        return false;                       // but all single stun aura spells is negative

                    // Petrification
                    if (spellproto->Id == 17624)
                        return false;
                    break;
                case SPELL_AURA_MOD_ROOT:
                case SPELL_AURA_MOD_SILENCE:
                case SPELL_AURA_GHOST:
                case SPELL_AURA_PERIODIC_LEECH:
                case SPELL_AURA_MOD_PACIFY_SILENCE:
                case SPELL_AURA_MOD_STALKED:
                case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
                    return false;
                case SPELL_AURA_PERIODIC_DAMAGE:            // used in positive spells also.
                    // part of negative spell if cast at self (prevent cancel)
                    if (spellproto->EffectImplicitTargetA[effIndex] == TARGET_UNIT_TARGET_ANY)
                        return false;
                    // part of negative spell if cast at self (prevent cancel)
                    else if (spellproto->EffectImplicitTargetA[effIndex] == TARGET_UNIT_CASTER)
                        return false;
                    break;
                case SPELL_AURA_MOD_DECREASE_SPEED:         // used in positive spells also
                    // part of positive spell if cast at self
                    if (spellproto->EffectImplicitTargetA[effIndex] != TARGET_UNIT_CASTER)
                        return false;
                    // but not this if this first effect (don't found batter check)
                    if (spellproto->Attributes & 0x4000000 && effIndex==0)
                        return false;
                    break;
                case SPELL_AURA_TRANSFORM:
                    // some spells negative
                    switch (spellproto->Id)
                    {
                        case 36897:                         // Transporter Malfunction (race mutation to horde)
                        case 36899:                         // Transporter Malfunction (race mutation to alliance)
                            return false;
                    }
                    break;
                case SPELL_AURA_MOD_SCALE:
                    // some spells negative
                    switch (spellproto->Id)
                    {
                        case 36900:                         // Soul Split: Evil!
                        case 36901:                         // Soul Split: Good
                        case 36893:                         // Transporter Malfunction (decrease size case)
                        case 36895:                         // Transporter Malfunction (increase size case)
                            return false;
                    }
                    break;
                case SPELL_AURA_MECHANIC_IMMUNITY:
                {
                    // non-positive immunities
                    switch (spellproto->EffectMiscValue[effIndex])
                    {
                        case MECHANIC_BANDAGE:
                        case MECHANIC_SHIELD:
                        case MECHANIC_MOUNT:
                        case MECHANIC_INVULNERABILITY:
                            return false;
                        default:
                            break;
                    }
                }   break;
                case SPELL_AURA_ADD_FLAT_MODIFIER:          // mods
                case SPELL_AURA_ADD_PCT_MODIFIER:
                {
                    // non-positive mods
                    switch (spellproto->EffectMiscValue[effIndex])
                    {
                        case SPELLMOD_COST:                 // dependent from bas point sign (negative -> positive)
                            if (spellproto->CalculateSimpleValue(effIndex) > 0)
                                return false;
                            break;
                        default:
                            break;
                    }
                }   break;
                case SPELL_AURA_MOD_HEALING_PCT:
                    if (spellproto->CalculateSimpleValue(effIndex) < 0)
                        return false;
                    break;
                case SPELL_AURA_MOD_SKILL:
                    if (spellproto->CalculateSimpleValue(effIndex) < 0)
                        return false;
                    break;
                case SPELL_AURA_FORCE_REACTION:
                    if (spellproto->Id==42792)               // Recently Dropped Flag (prevent cancel)
                        return false;
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }

    // non-positive targets
    if (!SpellMgr::IsPositiveTarget(spellproto->EffectImplicitTargetA[effIndex],spellproto->EffectImplicitTargetB[effIndex]))
        return false;

    // ok, positive
    return true;
}

bool SpellMgr::IsPositiveSpell(uint32 spellId)
{
    SpellEntry const *spellproto = sSpellStore.LookupEntry(spellId);
    if (!spellproto) return false;

    // talents
    if (SpellMgr::IsPassiveSpell(spellId) && GetTalentSpellCost(spellId))
        return true;

    // spells with at least one negative effect are considered negative
    // some self-applied spells have negative effects but in self casting case negative check ignored.
    for (int i = 0; i < 3; i++)
    {
        if (!SpellMgr::IsPositiveEffect(spellId, i))
            return false;
    }

    return true;
}

bool SpellMgr::IsSingleTargetSpell(SpellEntry const *spellInfo)
{
    // all other single target spells have if it has AttributesEx5
    if (spellInfo->AttributesEx5 & SPELL_ATTR_EX5_SINGLE_TARGET_SPELL)
        return true;

    // TODO - need found Judgements rule
    switch (SpellMgr::GetSpellSpecific(spellInfo))
    {
        case SPELL_JUDGEMENT:
            return true;
    }

    // single target triggered spell.
    // Not real client side single target spell, but it' not triggered until prev. aura expired.
    // This is allow store it in single target spells list for caster for spell proc checking
    if (spellInfo->Id==38324)                                // Regeneration (triggered by 38299 (HoTs on Heals))
        return true;

    return false;
}

bool SpellMgr::IsSingleTargetSpells(SpellEntry const *spellInfo1, SpellEntry const *spellInfo2)
{
    // TODO - need better check
    // Equal icon and spellfamily
    if (spellInfo1->SpellFamilyName == spellInfo2->SpellFamilyName &&
        spellInfo1->SpellIconID == spellInfo2->SpellIconID)
        return true;

    // TODO - need found Judgements rule
    SpellSpecific spec1 = SpellMgr::GetSpellSpecific(spellInfo1);
    // spell with single target specific types
    switch (spec1)
    {
        case SPELL_JUDGEMENT:
        case SPELL_MAGE_POLYMORPH:
            if (SpellMgr::GetSpellSpecific(spellInfo2) == spec1)
                return true;
            break;
    }

    return false;
}

bool SpellMgr::IsAuraAddedBySpell(uint32 auraType, uint32 spellId)
{
    SpellEntry const *spellproto = sSpellStore.LookupEntry(spellId);
    if (!spellproto) return false;

    for (int i = 0; i < 3; i++)
        if (spellproto->EffectApplyAuraName[i] == auraType)
            return true;
    return false;
}

SpellCastResult SpellMgr::GetErrorAtShapeshiftedCast (SpellEntry const *spellInfo, uint32 form)
{
    // talents that learn spells can have stance requirements that need ignore
    // (this requirement only for client-side stance show in talent description)
    if (GetTalentSpellCost(spellInfo->Id) > 0 &&
        (spellInfo->Effect[0]==SPELL_EFFECT_LEARN_SPELL || spellInfo->Effect[1]==SPELL_EFFECT_LEARN_SPELL || spellInfo->Effect[2]==SPELL_EFFECT_LEARN_SPELL))
        return SPELL_CAST_OK;

    uint32 stanceMask = (form ? 1 << (form - 1) : 0);

    if (stanceMask & spellInfo->StancesNot)                 // can explicitly not be cast in this stance
        return SPELL_FAILED_NOT_SHAPESHIFT;

    if (stanceMask & spellInfo->Stances)                    // can explicitly be cast in this stance
        return SPELL_CAST_OK;

    bool actAsShifted = false;
    if (form > 0)
    {
        SpellShapeshiftEntry const *shapeInfo = sSpellShapeshiftStore.LookupEntry(form);
        if (!shapeInfo)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: GetErrorAtShapeshiftedCast: unknown shapeshift %u", form);
            return SPELL_CAST_OK;
        }
        actAsShifted = !(shapeInfo->flags1 & 1);            // shapeshift acts as normal form for spells
    }

    if (actAsShifted)
    {
        if (spellInfo->Attributes & SPELL_ATTR_NOT_SHAPESHIFT) // not while shapeshifted
            return SPELL_FAILED_NOT_SHAPESHIFT;
        else if (spellInfo->Stances != 0)                   // needs other shapeshift
            return SPELL_FAILED_ONLY_SHAPESHIFT;
    }
    else
    {
        // needs shapeshift
        if (!(spellInfo->AttributesEx2 & SPELL_ATTR_EX2_NOT_NEED_SHAPESHIFT) && spellInfo->Stances != 0)
            return SPELL_FAILED_ONLY_SHAPESHIFT;
    }

    return SPELL_CAST_OK;
}

bool SpellMgr::IsBinaryResistable(SpellEntry const* spellInfo)
{
    if(spellInfo->SchoolMask & SPELL_SCHOOL_MASK_HOLY)                  // can't resist holy spells
        return false;

    if(spellInfo->SpellFamilyName)         // only player's spells, bosses don't follow that simple rule
    {
        for(int eff = 0; eff < 3; eff++)
        {
            if(!spellInfo->Effect[eff])
                continue;

            if (SpellMgr::IsPositiveEffect(spellInfo->Id, eff))
                continue;

            switch(spellInfo->Effect[eff])
            {
                case SPELL_EFFECT_SCHOOL_DAMAGE:
                    break;
                case SPELL_EFFECT_APPLY_AURA:
                case SPELL_EFFECT_APPLY_AREA_AURA_ENEMY:
                    if(spellInfo->EffectApplyAuraName[eff] != SPELL_AURA_PERIODIC_DAMAGE)       // spells that apply aura other then DOT are binary resistable
                        return true;
                    break;
                default:
                    return true;                                                                // spells that have other effects then damage or apply aura are binary resistable
            }
        }
    }

    switch (spellInfo->Id)
    {
        case 31306:     // Anetheron - Carrion Swarm
        case 31344:     // Howl of Azgalor
        case 31447:     // Mark of Kaz'Rogal
        case 34190:     // Void - Arcane Orb
        case 37730:     // Morogrim - Tidal Wave
        case 38441:     // Fathom - Cataclysm bolt
        case 38509:     // Vashj - Shock Blast
        case 37675:     // Leotheras - Chaos Blast
            return true;
    }
    return false;
}

bool SpellMgr::IsPartialyResistable(SpellEntry const* spellInfo)
{
    if (spellInfo->AttributesEx4 & SPELL_ATTR_EX4_IGNORE_RESISTANCES)
        return false;

    // check which of them have above attribute and remove it from switch :]
    switch (spellInfo->Id)
    {
        case 30115:     // Terestian - Sacrifice
        case 33051:     // Krosh Firehand - Greater Fireball
        case 36805:     // Kael'thas - Fireball
        case 36819:     // Kael'thas - Pyroblast
        case 31944:     // Archimond - Doomfire
        case 31972:     // Archimond - Grip of the Legion
        case 32053:     // Archimond - Soul Charge, red
        case 32054:     // Archimond - Soul Charge, yellow
        case 32057:     // Archimond - Soul Charge, green
        case 41545:     // RoS: Soul Scream
        case 41376:     // RoS: Spite
        case 41352:     // RoS: Aura of Desire dmg back
        case 41337:     // RoS: Aura of Anger
        case 40239:     // Teron: Incinerate
        case 40325:     // Teron: Spirit Strike
        case 40157:     // Teron: Spirit Lance
        case 40175:     // Teron: Spirit Chains
        case 41483:     // High Nethermancer Zerevor: Arcane Bolt
        case 44335:     // Vexallus: Energy Feedback
        case 47002:     // Felmyst: Noxious Fumes
        case 45866:     // Felmyst: Corrosion
        case 45855:     // Felmyst: Gas Nova
            return false;
    }

    if (spellInfo->SchoolMask & SPELL_SCHOOL_MASK_HOLY)                  // can't resist holy spells
        return false;

    if (SpellMgr::IsBinaryResistable(spellInfo))
        return false;
    else
        return true;
}

void SpellMgr::LoadSpellTargetPositions()
{
    mSpellTargetPositions.clear();                                // need for reload case

    uint32 count = 0;

    //                                                       0   1           2                  3                  4                  5
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT id, target_map, target_position_x, target_position_y, target_position_z, target_orientation FROM spell_target_position");
    if (!result)
    {

        BarGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u spell target coordinates", count);
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        ++count;

        uint32 Spell_ID = fields[0].GetUInt32();

        SpellTargetPosition st;

        st.target_mapId       = fields[1].GetUInt32();
        st.target_X           = fields[2].GetFloat();
        st.target_Y           = fields[3].GetFloat();
        st.target_Z           = fields[4].GetFloat();
        st.target_Orientation = fields[5].GetFloat();

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(Spell_ID);
        if (!spellInfo)
        {
            sLog.outLog(LOG_DB_ERR, "Spell (ID:%u) listed in `spell_target_position` does not exist.",Spell_ID);
            continue;
        }

        bool found = false;
        for (int i = 0; i < 3; ++i)
        {
            if (spellInfo->EffectImplicitTargetA[i]==TARGET_DST_DB || spellInfo->EffectImplicitTargetB[i]==TARGET_DST_DB)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            sLog.outLog(LOG_DB_ERR, "Spell (Id: %u) listed in `spell_target_position` does not have target TARGET_DST_DB (17).",Spell_ID);
            continue;
        }

        MapEntry const* mapEntry = sMapStore.LookupEntry(st.target_mapId);
        if (!mapEntry)
        {
            sLog.outLog(LOG_DB_ERR, "Spell (ID:%u) target map (ID: %u) does not exist in `Map.dbc`.",Spell_ID,st.target_mapId);
            continue;
        }

        if (st.target_X==0 && st.target_Y==0 && st.target_Z==0)
        {
            sLog.outLog(LOG_DB_ERR, "Spell (ID:%u) target coordinates not provided.",Spell_ID);
            continue;
        }

        mSpellTargetPositions[Spell_ID] = st;

    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u spell teleport coordinates", count);
}

void SpellMgr::LoadSpellAffects()
{
    mSpellAffectMap.clear();                                // need for reload case

    uint32 count = 0;

    //                                                       0      1         2
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT entry, effectId, SpellFamilyMask FROM spell_affect");
    if (!result)
    {

        BarGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u spell affect definitions", count);
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint16 entry = fields[0].GetUInt16();
        uint8 effectId = fields[1].GetUInt8();

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(entry);

        if (!spellInfo)
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_affect` does not exist", entry);
            continue;
        }

        if (effectId >= 3)
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_affect` have invalid effect index (%u)", entry,effectId);
            continue;
        }

        if (spellInfo->Effect[effectId] != SPELL_EFFECT_APPLY_AURA ||
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_FLAT_MODIFIER &&
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_PCT_MODIFIER  &&
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_TARGET_TRIGGER)
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_affect` have not SPELL_AURA_ADD_FLAT_MODIFIER (%u) or SPELL_AURA_ADD_PCT_MODIFIER (%u) or SPELL_AURA_ADD_TARGET_TRIGGER (%u) for effect index (%u)", entry,SPELL_AURA_ADD_FLAT_MODIFIER,SPELL_AURA_ADD_PCT_MODIFIER,SPELL_AURA_ADD_TARGET_TRIGGER,effectId);
            continue;
        }

        uint64 spellAffectMask = fields[2].GetUInt64();

        // Spell.dbc have own data for low part of SpellFamilyMask
        if (spellInfo->EffectItemType[effectId])
        {
            if (spellInfo->EffectItemType[effectId] == spellAffectMask)
            {
                sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_affect` have redundant (same with EffectItemType%d) data for effect index (%u) and not needed, skipped.", entry,effectId+1,effectId);
                continue;
            }

            // 24429 have wrong data in EffectItemType and overwrites by DB, possible bug in client
            if (spellInfo->Id!=24429 && spellInfo->EffectItemType[effectId] != spellAffectMask)
            {
                sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_affect` have different low part from EffectItemType%d for effect index (%u) and not needed, skipped.", entry,effectId+1,effectId);
                continue;
            }
        }

        mSpellAffectMap.insert(SpellAffectMap::value_type((entry<<8) + effectId,spellAffectMask));

        ++count;
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u spell affect definitions", count);

    for (uint32 id = 0; id < sSpellStore.GetNumRows(); ++id)
    {
        SpellEntry const* spellInfo = sSpellStore.LookupEntry(id);
        if (!spellInfo)
            continue;

        for (int effectId = 0; effectId < 3; ++effectId)
        {
            if (spellInfo->Effect[effectId] != SPELL_EFFECT_APPLY_AURA ||
                (spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_FLAT_MODIFIER &&
                spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_PCT_MODIFIER  &&
                spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_TARGET_TRIGGER))
                continue;

            if (spellInfo->EffectItemType[effectId] != 0)
                continue;

            if (mSpellAffectMap.find((id<<8) + effectId) !=  mSpellAffectMap.end())
                continue;

            sLog.outLog(LOG_DB_ERR, "Spell %u (%s) misses spell_affect for effect %u",id,spellInfo->SpellName[sWorld.GetDefaultDbcLocale()], effectId);
        }
    }
}

bool SpellMgr::IsAffectedBySpell(SpellEntry const *spellInfo, uint32 spellId, uint8 effectId, uint64 familyFlags) const
{
    // false for spellInfo == NULL
    if (!spellInfo)
        return false;

    SpellEntry const *affect_spell = sSpellStore.LookupEntry(spellId);
    // false for affect_spell == NULL
    if (!affect_spell)
        return false;

    if (spellId == 37706 && (spellInfo->Effect[0] == SPELL_EFFECT_HEAL || spellInfo->EffectApplyAuraName[0] == SPELL_AURA_PERIODIC_HEAL))
        return true;

    // False if spellFamily not equal
    if (affect_spell->SpellFamilyName != spellInfo->SpellFamilyName)
        return false;

    // If familyFlags == 0
    if (!familyFlags)
    {
        // Get it from spellAffect table
        familyFlags = GetSpellAffectMask(spellId,effectId);
        // false if familyFlags == 0
        if (!familyFlags)
            return false;
    }

    // true
    if (familyFlags & spellInfo->SpellFamilyFlags)
        return true;

    return false;
}

void SpellMgr::LoadSpellProcEvents()
{
    mSpellProcEventMap.clear();                             // need for reload case

    uint32 count = 0;

    //                                                       0      1           2                3                4          5       6        7             8
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT entry, SchoolMask, SpellFamilyName, SpellFamilyMask, procFlags, procEx, ppmRate, CustomChance, Cooldown FROM spell_proc_event");
    if (!result)
    {

        BarGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u spell proc event conditions", count );
        return;
    }

    BarGoLink bar(result->GetRowCount());
    uint32 customProc = 0;
    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint16 entry = fields[0].GetUInt16();

        const SpellEntry *spell = sSpellStore.LookupEntry(entry);
        if (!spell)
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_proc_event` does not exist", entry);
            continue;
        }

        SpellProcEventEntry spe;

        spe.schoolMask      = fields[1].GetUInt32();
        spe.spellFamilyName = fields[2].GetUInt32();
        spe.spellFamilyMask = fields[3].GetUInt64();
        spe.procFlags       = fields[4].GetUInt32();
        spe.procEx          = fields[5].GetUInt32();
        spe.ppmRate         = fields[6].GetFloat();
        spe.customChance    = fields[7].GetFloat();
        spe.cooldown        = fields[8].GetUInt32();

        mSpellProcEventMap[entry] = spe;

        if (spell->procFlags==0)
        {
            if (spe.procFlags == 0)
            {
                sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_proc_event` probally not triggered spell", entry);
                continue;
            }
            customProc++;
        }
        ++count;
    } while (result->NextRow());

    sLog.outString();
    if (customProc)
        sLog.outString(">> Loaded %u custom spell proc event conditions +%u custom",  count, customProc);
    else
        sLog.outString(">> Loaded %u spell proc event conditions", count);

    /*
    // Commented for now, as it still produces many errors (still quite many spells miss spell_proc_event)
    for (uint32 id = 0; id < sSpellStore.GetNumRows(); ++id)
    {
        SpellEntry const* spellInfo = sSpellStore.LookupEntry(id);
        if (!spellInfo)
            continue;

        bool found = false;
        for (int effectId = 0; effectId < 3; ++effectId)
        {
            // at this moment check only SPELL_AURA_PROC_TRIGGER_SPELL
            if (spellInfo->EffectApplyAuraName[effectId] == SPELL_AURA_PROC_TRIGGER_SPELL)
            {
                found = true;
                break;
            }
        }

        if (!found)
            continue;

        if (GetSpellProcEvent(id))
            continue;

        sLog.outLog(LOG_DB_ERR, "Spell %u (%s) misses spell_proc_event",id,spellInfo->SpellName[sWorld.GetDBClang()]);
    }
    */
}

/*
bool SpellMgr::IsSpellProcEventCanTriggeredBy(SpellProcEventEntry const * spellProcEvent, SpellEntry const * procSpell, uint32 procFlags)
{
    if ((procFlags & spellProcEvent->procFlags) == 0)
        return false;

    // Additional checks in case spell cast/hit/crit is the event
    // Check (if set) school, category, skill line, spell talent mask
    if (spellProcEvent->schoolMask && (!procSpell || (GetSpellSchoolMask(procSpell) & spellProcEvent->schoolMask) == 0))
        return false;
    if (spellProcEvent->category && (!procSpell || procSpell->Category != spellProcEvent->category))
        return false;
    if (spellProcEvent->skillId)
    {
        if (!procSpell)
            return false;

        SkillLineAbilityMap::const_iterator lower = sSpellMgr.GetBeginSkillLineAbilityMap(procSpell->Id);
        SkillLineAbilityMap::const_iterator upper = sSpellMgr.GetEndSkillLineAbilityMap(procSpell->Id);

        bool found = false;
        for (SkillLineAbilityMap::const_iterator _spell_idx = lower; _spell_idx != upper; ++_spell_idx)
        {
            if (_spell_idx->second->skillId == spellProcEvent->skillId)
            {
                found = true;
                break;
            }
        }
        if (!found)
            return false;
    }
    if (spellProcEvent->spellFamilyName && (!procSpell || spellProcEvent->spellFamilyName != procSpell->SpellFamilyName))
        return false;
    if (spellProcEvent->spellFamilyMask && (!procSpell || (spellProcEvent->spellFamilyMask & procSpell->SpellFamilyFlags) == 0))
        return false;

    return true;
}
*/

bool SpellMgr::IsSpellProcEventCanTriggeredBy(SpellProcEventEntry const * spellProcEvent, uint32 EventProcFlag, SpellEntry const * procSpell, uint32 procFlags, uint32 procExtra, bool active)
{
    // No extra req need
    uint32 procEvent_procEx = PROC_EX_NONE;

    // check prockFlags for condition
    if ((procFlags & EventProcFlag) == 0)
        return false;

    /* Check Periodic Auras

    * Both hots and dots can trigger if spell has no PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL
        nor PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT

    *Only Hots can trigger if spell has PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL

    *Only dots can trigger if spell has both positivity flags or PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT

    */

    if (procFlags & PROC_FLAG_ON_DO_PERIODIC)
    {
        if (EventProcFlag & PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT)
        {
            if (!(procExtra & PROC_EX_INTERNAL_DOT))
                return false;
        }
        else if (EventProcFlag & PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL
            && !(procExtra & PROC_EX_INTERNAL_HOT))
            return false;
    }

    if (procFlags & PROC_FLAG_ON_TAKE_PERIODIC)
    {
        if (EventProcFlag & PROC_FLAG_TAKEN_NEGATIVE_SPELL_HIT)
        {
            if (!(procExtra & PROC_EX_INTERNAL_DOT))
                return false;
        }
        else if (EventProcFlag & PROC_FLAG_TAKEN_POSITIVE_SPELL
            && !(procExtra & PROC_EX_INTERNAL_HOT))
            return false;
    }

    // Always trigger for this
    if (EventProcFlag & (PROC_FLAG_KILLED | PROC_FLAG_KILL_AND_GET_XP))
        return true;

    if (spellProcEvent)     // Exist event data
    {
        // Store extra req
        procEvent_procEx = spellProcEvent->procEx;

        // For melee triggers
        if (procSpell == NULL)
        {
            // Check (if set) for school (melee attack have Normal school)
            if (spellProcEvent->schoolMask && (spellProcEvent->schoolMask & SPELL_SCHOOL_MASK_NORMAL) == 0)
                return false;
        }
        else // For spells need check school/spell family/family mask
        {
            // Potions can trigger only if spellfamily given
            if (procSpell->SpellFamilyName == SPELLFAMILY_POTION)
            {
                if (procSpell->SpellFamilyName == spellProcEvent->spellFamilyName)
                    return true;
                return false;
            }

            // Check (if set) for school
            if (spellProcEvent->schoolMask && (spellProcEvent->schoolMask & procSpell->SchoolMask) == 0)
                return false;

            // Check (if set) for spellFamilyName
            if (spellProcEvent->spellFamilyName && (spellProcEvent->spellFamilyName != procSpell->SpellFamilyName))
                return false;

            // spellFamilyName is Ok need check for spellFamilyMask if present
            if (spellProcEvent->spellFamilyMask)
            {
                if ((spellProcEvent->spellFamilyMask & procSpell->SpellFamilyFlags) == 0)
                    return false;
                active = true; // Spell added manualy -> so its active spell
            }
        }
    }
    // potions can trigger only if have spell_proc entry
    else if (procSpell && procSpell->SpellFamilyName==SPELLFAMILY_POTION)
        return false;

    // Check for extra req (if none) and hit/crit
    if (procEvent_procEx == PROC_EX_NONE)
    {
        // No extra req, so can trigger only for active (damage/healing present) and hit/crit
        if ((procExtra & (PROC_EX_NORMAL_HIT|PROC_EX_CRITICAL_HIT)) && active)
            return true;
    }
    else // Passive spells hits here only if resist/reflect/immune/evade
    {
        if (procEvent_procEx & PROC_EX_NETHER_PROTECTION)
        {
            if (procSpell == NULL)
                return false;
            if (procSpell->Effect[0] == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE)
                return false;
            if (procExtra & PROC_EX_NORMAL_HIT)
                return true;

            return false;
        }
        // Exist req for PROC_EX_EX_TRIGGER_ALWAYS
        if (procEvent_procEx & PROC_EX_EX_TRIGGER_ALWAYS)
            return true;
        // Passive spells can`t trigger if need hit
        if ((procEvent_procEx & PROC_EX_NORMAL_HIT & procExtra) && !active)
            return false;
        // Check Extra Requirement like (hit/crit/miss/resist/parry/dodge/block/immune/reflect/absorb and other)
        if (procEvent_procEx & procExtra)
            return true;
    }
    return false;
}

void SpellMgr::LoadSpellElixirs()
{
    mSpellElixirs.clear();                                  // need for reload case

    uint32 count = 0;

    //                                                       0      1
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT entry, mask FROM spell_elixir");
    if (!result)
    {

        BarGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u spell elixir definitions", count);
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint16 entry = fields[0].GetUInt16();
        uint8 mask = fields[1].GetUInt8();

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(entry);

        if (!spellInfo)
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_elixir` does not exist", entry);
            continue;
        }

        mSpellElixirs[entry] = mask;

        ++count;
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u spell elixir definitions", count);
}

void SpellMgr::LoadSpellThreats()
{
    sSpellThreatStore.Free();                               // for reload

    sSpellThreatStore.Load();

    sLog.outString(">> Loaded %u aggro generating spells", sSpellThreatStore.RecordCount);
    sLog.outString();
}

void SpellMgr::LoadSpellBonusData()
{
    mSpellBonusDataMap.clear();
    uint32 count = 0;

    //                                                0      1          2       3             4
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT entry, direct_co, dot_co, direct_ap_co, dot_ap_co, FROM spell_bonus_data");
    if (!result)
    {

        BarGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u spell bonus data info", count);
        return;
    }

    BarGoLink bar(result->GetRowCount());
    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint32 entry        = fields[0].GetUInt32();

        SpellEntry const *spellInfo = sSpellStore.LookupEntry(entry);
        if (!spellInfo)
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_bonus_data` does not exist", entry);
            continue;
        }

        SpellBonusData bd;
        bd.direct_co    = fields[1].GetFloat();
        bd.dot_co       = fields[2].GetFloat();
        bd.direct_ap_co = fields[3].GetFloat();
        bd.dot_ap_co    = fields[4].GetFloat();

        mSpellBonusDataMap[entry] = bd;

        ++count;
    }
    while (result->NextRow());

    sLog.outString(">> Loaded %u spell bonus data definitions", count);
}

void SpellMgr::LoadSpellEnchantProcData()
{
    mSpellEnchantProcEventMap.clear();                             // need for reload case

    uint32 count = 0;

    //                                                       0      1             2          3          4
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT entry, customChance, PPMChance, procFlags, procEx FROM spell_enchant_proc_data");
    if (!result)
    {

        BarGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u spell enchant proc event conditions", count);
        return;
    }

    BarGoLink bar(result->GetRowCount());
    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint32 enchantId = fields[0].GetUInt32();

        SpellItemEnchantmentEntry const *ench = sSpellItemEnchantmentStore.LookupEntry(enchantId);
        if (!ench)
        {
            sLog.outLog(LOG_DB_ERR, "Enchancment %u listed in `spell_enchant_proc_data` does not exist", enchantId);
            continue;
        }

        SpellEnchantProcEntry spe;

        spe.customChance = fields[1].GetUInt32();
        spe.PPMChance = fields[2].GetFloat();
        spe.procFlags = fields[3].GetUInt32();
        spe.procEx = fields[4].GetUInt32();

        mSpellEnchantProcEventMap[enchantId] = spe;

        ++count;
    } while (result->NextRow());

    sLog.outString(">> Loaded %u enchant proc data definitions", count);
}

bool SpellMgr::IsRankSpellDueToSpell(SpellEntry const *spellInfo_1,uint32 spellId_2) const
{
    SpellEntry const *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
    if (!spellInfo_1 || !spellInfo_2) return false;
    if (spellInfo_1->Id == spellId_2) return false;

    return GetFirstSpellInChain(spellInfo_1->Id)==GetFirstSpellInChain(spellId_2);
}

bool SpellMgr::canStackSpellRanks(SpellEntry const *spellInfo)
{
    // exception: faerie fire (feral)
    if (spellInfo->SpellFamilyName == SPELLFAMILY_DRUID && spellInfo->SpellFamilyFlags & 0x400)
        return true;

    if (spellInfo->powerType != POWER_MANA && spellInfo->powerType != POWER_HEALTH)
        return false;

    if (SpellMgr::IsProfessionSpell(spellInfo->Id))
        return false;

    // All stance spells. if any better way, change it.
    for (int i = 0; i < 3; i++)
    {
        // Paladin aura Spell
        if (spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN
            && spellInfo->Effect[i]==SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
            return false;
        // Druid form Spell
        if (spellInfo->SpellFamilyName == SPELLFAMILY_DRUID
            && spellInfo->Effect[i]==SPELL_EFFECT_APPLY_AURA
            && spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOD_SHAPESHIFT)
            return false;
        // Rogue Stealth
        if (spellInfo->SpellFamilyName == SPELLFAMILY_ROGUE
            && spellInfo->Effect[i]==SPELL_EFFECT_APPLY_AURA
            && spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOD_SHAPESHIFT)
            return false;
    }
    return true;
}

uint8 SpellMgr::IsNoStackSpellDueToSpell(SpellEntry const* spellInfo_1, SpellEntry const* spellInfo_2, bool sameCaster, uint8 effect)
{
    // this funcion returns flag describing which effect of spellInfo_2 must be removed when applying (spellInfo_1,effect)
    if (!spellInfo_1 || !spellInfo_2)
        return 0;

    if (SpellMgr::IsSpecialStackCase(spellInfo_1, spellInfo_2, sameCaster))
        return 0;

    if (SpellMgr::IsSpecialNoStackCase(spellInfo_1, spellInfo_2, sameCaster))
        return 0x7;

    // kiru song of victory, its special no stack, but uses effectid and returns other values than 0x7
    if (spellInfo_1->Id == 46302) // song is a new spell
    {
        if (effect == 1 && spellInfo_2->SpellFamilyName == SPELLFAMILY_PRIEST && spellInfo_2->SpellFamilyFlags & 8)// stamina
            return 0x7;
        if (effect == 0 && spellInfo_2->SpellFamilyName == SPELLFAMILY_MAGE && spellInfo_2->SpellFamilyFlags & 1024)// intelect
            return 0x7;
    }
    if (spellInfo_2->Id == 46302) // song is old spell, remove only proper effect
    {
        if (spellInfo_1->SpellFamilyName == SPELLFAMILY_PRIEST && spellInfo_1->SpellFamilyFlags & 8)// stamina
            return 0x2;
        if (spellInfo_1->SpellFamilyName == SPELLFAMILY_MAGE && spellInfo_1->SpellFamilyFlags & 1024)// intelect
            return 0x1;
    }

    SpellSpecific spellId_spec_1 = SpellMgr::GetSpellSpecific(spellInfo_1);
    SpellSpecific spellId_spec_2 = SpellMgr::GetSpellSpecific(spellInfo_2);
    if (spellId_spec_1 && spellId_spec_2)
        if (SpellMgr::IsSingleFromSpellSpecificPerTarget(spellId_spec_1, spellId_spec_2)
            ||(SpellMgr::IsSingleFromSpellSpecificPerCaster(spellId_spec_1, spellId_spec_2) && sameCaster) ||
            (SpellMgr::IsSingleFromSpellSpecificRanksPerTarget(spellId_spec_1, spellId_spec_2) && sSpellMgr.IsRankSpellDueToSpell(spellInfo_1, spellId_spec_2)))
            return 0x7;

    // spells with different specific always stack
    if (spellId_spec_1 != spellId_spec_2)
        return 0;

    if (spellInfo_1->SpellFamilyName != spellInfo_2->SpellFamilyName)
        return 0;

    // generic spells
    if (!spellInfo_1->SpellFamilyName)
    {
        if (!spellInfo_1->SpellIconID
            || spellInfo_1->SpellIconID == 1
            || spellInfo_1->SpellIconID != spellInfo_2->SpellIconID)
            return 0;
    }

    // check for class spells
    else
    {
        if (spellInfo_1->SpellFamilyFlags != spellInfo_2->SpellFamilyFlags)
            return 0;
    }

    if (!sameCaster)
    {
        for (uint32 i = 0; i < 3; ++i)
            if (spellInfo_1->Effect[i] == SPELL_EFFECT_APPLY_AURA
                || spellInfo_1->Effect[i] == SPELL_EFFECT_PERSISTENT_AREA_AURA)
                // not area auras (shaman totem)
                switch (spellInfo_1->EffectApplyAuraName[i])
                {
                    // DOT or HOT from different casters will stack
                    case SPELL_AURA_PERIODIC_DAMAGE:
                    case SPELL_AURA_PERIODIC_HEAL:
                    case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                    case SPELL_AURA_PERIODIC_ENERGIZE:
                    case SPELL_AURA_PERIODIC_MANA_LEECH:
                    case SPELL_AURA_PERIODIC_LEECH:
                    case SPELL_AURA_POWER_BURN_MANA:
                    case SPELL_AURA_OBS_MOD_MANA:
                    case SPELL_AURA_OBS_MOD_HEALTH:
                        return 0;
                    default:
                        break;
                }
    }

    //use data of highest rank spell(needed for spells which ranks have different effects)
    spellInfo_1=sSpellStore.LookupEntry(sSpellMgr.GetLastSpellInChain(spellInfo_1->Id));
    spellInfo_2=sSpellStore.LookupEntry(sSpellMgr.GetLastSpellInChain(spellInfo_2->Id));

    //if spells have exactly the same effect they cannot stack
    for (uint32 i = 0; i < 3; ++i)
        if (spellInfo_1->Effect[i] != spellInfo_2->Effect[i]
            || spellInfo_1->EffectApplyAuraName[i] != spellInfo_2->EffectApplyAuraName[i]
            || spellInfo_1->EffectMiscValue[i] != spellInfo_2->EffectMiscValue[i]) // paladin resist aura
            return 0; // need itemtype check? need an example to add that check

    return 0x7;
}

bool SpellMgr::IsSpecialStackCase(SpellEntry const *spellInfo_1, SpellEntry const *spellInfo_2, bool sameCaster, bool recur)
{
    // put here all spells that should stack, but accoriding to rules in method IsNoStackSpellDueToSpell don't stack
    uint32 spellId_1 = spellInfo_1->Id;
    uint32 spellId_2 = spellInfo_2->Id;

    // judgement of light stacks with judgement of wisdom
    if (spellInfo_1->SpellFamilyName == SPELLFAMILY_PALADIN && spellInfo_1->SpellFamilyFlags & 0x80000 && spellInfo_1->SpellIconID == 299 // light
            && spellInfo_2->SpellFamilyName == SPELLFAMILY_PALADIN && spellInfo_2->SpellFamilyFlags & 0x80000 && spellInfo_2->SpellIconID == 206) // wisdom
        return !sameCaster;

    // Dragonmaw Illusion - should stack with everything ?
    if (spellId_1 == 40214 || spellId_2 == 40214)
        return true;

    // hourglass of unraveller stacks with blood fury
    if (spellId_1 == 33649 && spellId_2 == 20572)
        return true;

    if (spellId_1 == 22620 && spellId_2 == 22618)
        return true; // force reactive disc

    // Sextant of Unstable Currents, Shiffar's Nexus-Hornand, Scryer's Bloodgem and Band of the Ethernal Sage stacks with each other
    if ((spellId_1 == 38348 || spellId_1 == 34321 || spellId_1 == 35084 || spellId_1 == 35337) &&
            (spellId_2 == 38348 || spellId_2 == 34321 || spellId_2 == 35084 || spellId_2 == 35337) &&
            (spellId_1 != spellId_2))
        return true;

    // Warlord's Rage for Warlord Kalithresh event in Steamvault
    if (spellId_1 == 36453 && spellId_2 == 37076)
        return true;

    // Scroll of Agility and Idol of Terror
    if (spellId_1 == 43738 && (spellId_2 == 8115 || spellId_2 == 8116 || spellId_2 == 8117 || spellId_2 == 12174 || spellId_2 == 33077))
        return true;

    // Enh shaman t6 bonus proc and t6 trinket proc
    if(spellId_1 == 40466 && spellId_2 == 38430)
        return true;

    if (recur)
        return SpellMgr::IsSpecialStackCase(spellInfo_2, spellInfo_1, sameCaster, false);

    return false;
}

bool SpellMgr::IsSpecialNoStackCase(SpellEntry const *spellInfo_1, SpellEntry const *spellInfo_2, bool sameCaster, bool recur)
{
    // put here all spells that should NOT stack, but accoriding to rules in method IsNoStackSpellDueToSpell stack

    // Sunder Armor effect doesn't stack with Expose Armor
    if (spellInfo_1->SpellFamilyName == SPELLFAMILY_WARRIOR && spellInfo_1->SpellFamilyFlags & 0x4000L
        && spellInfo_2->SpellFamilyName == SPELLFAMILY_ROGUE && spellInfo_2->SpellFamilyFlags & 0x80000L)
        return true;

    // Power Infusion
    if (spellInfo_1->Id == 10060)
    {
        switch (spellInfo_2->Id)
        {
            // Icy Veins
            case 12472:
            // Heroism
            case 32182:
            // Bloodlust
            case 2825:
                return true;
        }
    }

    // Scrolls no stack case
    if (spellInfo_1->AttributesCu & SPELL_ATTR_CU_NO_SCROLL_STACK && spellInfo_2->AttributesCu & SPELL_ATTR_CU_NO_SCROLL_STACK)
    {
        // if it's same stat
        if (spellInfo_1->EffectMiscValue[0] == spellInfo_2->EffectMiscValue[0])
            return true;
    }

    if (spellInfo_1->Id == 45848 && (spellInfo_2->Id == 45737 || spellInfo_2->Id == 45641))
        return true; // shield of the blue removes fire bloom and flame dart

    // blood fury and wound poison
    if (spellInfo_1->SpellFamilyName == SPELLFAMILY_ROGUE && spellInfo_1->SpellFamilyFlags == 0x10000000 &&
        spellInfo_2->Id == 23230)
        return true;

    if (recur)
        return SpellMgr::IsSpecialNoStackCase(spellInfo_2, spellInfo_1, sameCaster, false);

    return false;
}

bool SpellMgr::IsProfessionSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if (!spellInfo)
        return false;

    if (spellInfo->Effect[1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[1];

    return SpellMgr::IsProfessionSkill(skill);
}

bool SpellMgr::IsPrimaryProfessionSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if (!spellInfo)
        return false;

    if (spellInfo->Effect[1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[1];

    return SpellMgr::IsPrimaryProfessionSkill(skill);
}

bool SpellMgr::IsPrimaryProfessionFirstRankSpell(uint32 spellId) const
{
    return SpellMgr::IsPrimaryProfessionSpell(spellId) && GetSpellRank(spellId)==1;
}

bool SpellMgr::IsSplashBuffAura(SpellEntry const* spellInfo)
{
    for (uint8 i = 0; i < 3; i++)
    {
        if (SpellMgr::IsPositiveEffect(spellInfo->Id, i))
        {
            if (spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
               return true;

            if (spellInfo->EffectImplicitTargetA[i] == TARGET_UNIT_PARTY_CASTER)
                return true;

            if (spellInfo->EffectImplicitTargetB[i] == TARGET_UNIT_AREA_PARTY_SRC)
                return true;

            if (spellInfo->EffectImplicitTargetB[i] == TARGET_UNIT_AREA_PARTY_DST)
                return true;
        }

    }
    return false;
}

SpellEntry const* SpellMgr::SelectAuraRankForPlayerLevel(SpellEntry const* spellInfo, uint32 playerLevel) const
{
    // ignore passive spells
    if (SpellMgr::IsPassiveSpell(spellInfo->Id))
        return spellInfo;

    bool needRankSelection = false;
    for (int i=0;i<3;i++)
    {
        if (SpellMgr::IsPositiveEffect(spellInfo->Id, i) && (
            spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA ||
            spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AREA_AURA_PARTY
           ))
        {
            needRankSelection = true;
            break;
        }
    }

    // not required
    if (!needRankSelection)
        return spellInfo;

    for (uint32 nextSpellId = spellInfo->Id; nextSpellId != 0; nextSpellId = GetPrevSpellInChain(nextSpellId))
    {
        SpellEntry const *nextSpellEntry = sSpellStore.LookupEntry(nextSpellId);
        if (!nextSpellEntry)
            break;

        // if found appropriate level
        if (playerLevel + 10 >= nextSpellEntry->spellLevel)
            return nextSpellEntry;

        // one rank less then
    }

    // not found
    return NULL;
}

void SpellMgr::LoadSpellRequired()
{
    mSpellsReqSpell.clear();                                   // need for reload case
    mSpellReq.clear();                                         // need for reload case

    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT spell_id, req_spell from spell_required");

    if ( ! result )
    {
        BarGoLink bar(1);
        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded 0 spell required records");
        sLog.outLog(LOG_DB_ERR, "`spell_required` table is empty!");
        return;
    }
    uint32 rows = 0;

    BarGoLink bar(result->GetRowCount());
    do
    {
        bar.step();
        Field *fields = result->Fetch();

        uint32 spell_id = fields[0].GetUInt32();
        uint32 spell_req = fields[1].GetUInt32();

        mSpellsReqSpell.insert (std::pair<uint32, uint32>(spell_req, spell_id));
        mSpellReq[spell_id] = spell_req;
        ++rows;
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u spell required records", rows);
}

struct SpellRankEntry
{
    uint32 SkillId;
    char const *SpellName;
    uint32 DurationIndex;
    uint32 RangeIndex;
    uint32 SpellVisual;
    uint32 ProcFlags;
    uint64 SpellFamilyFlags;
    uint32 TargetAuraState;
    uint32 ManaCost;

    bool operator()(const SpellRankEntry & _Left,const SpellRankEntry & _Right)const
    {
        return (_Left.SkillId != _Right.SkillId ? _Left.SkillId < _Right.SkillId
            : _Left.SpellName!=_Right.SpellName ? _Left.SpellName < _Right.SpellName
            : _Left.ProcFlags!=_Right.ProcFlags ? _Left.ProcFlags < _Right.ProcFlags
            : _Left.SpellFamilyFlags!=_Right.SpellFamilyFlags ? _Left.SpellFamilyFlags < _Right.SpellFamilyFlags
            : (_Left.SpellVisual!=_Right.SpellVisual) && (!_Left.SpellVisual || !_Right.SpellVisual) ? _Left.SpellVisual < _Right.SpellVisual
            : (_Left.ManaCost!=_Right.ManaCost) && (!_Left.ManaCost || !_Right.ManaCost) ? _Left.ManaCost < _Right.ManaCost
            : (_Left.DurationIndex!=_Right.DurationIndex) && (!_Left.DurationIndex || !_Right.DurationIndex)? _Left.DurationIndex < _Right.DurationIndex
            : (_Left.RangeIndex!=_Right.RangeIndex) && (!_Left.RangeIndex || !_Right.RangeIndex || _Left.RangeIndex==1 || !_Right.RangeIndex==1) ? _Left.RangeIndex < _Right.RangeIndex
            : _Left.TargetAuraState < _Right.TargetAuraState
           );
    }
};

struct SpellRankValue
{
    uint32 Id;
    char const *Rank;
};

void SpellMgr::LoadSpellChains()
{
    mSpellChains.clear();                                   // need for reload case

    std::vector<uint32> ChainedSpells;
    for (uint32 ability_id=0;ability_id<sSkillLineAbilityStore.GetNumRows();ability_id++)
    {
        SkillLineAbilityEntry const *AbilityInfo=sSkillLineAbilityStore.LookupEntry(ability_id);
        if (!AbilityInfo)
            continue;
        if (AbilityInfo->spellId==20154) //exception to these rules (not needed in 3.0.3)
            continue;
        if (!AbilityInfo->forward_spellid)
            continue;
        ChainedSpells.push_back(AbilityInfo->forward_spellid);
    }

    std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry> RankMap;

    for (uint32 ability_id=0;ability_id<sSkillLineAbilityStore.GetNumRows();ability_id++)
    {
        SkillLineAbilityEntry const *AbilityInfo=sSkillLineAbilityStore.LookupEntry(ability_id);
        if (!AbilityInfo)
            continue;

        //get only spell with lowest ability_id to prevent doubles
        uint32 spell_id=AbilityInfo->spellId;
        if (spell_id == 20154) //exception to these rules (not needed in 3.0.3)
            continue;

        bool found=false;
        for (uint32 i=0; i<ChainedSpells.size(); i++)
        {
           if (ChainedSpells.at(i)==spell_id)
               found=true;
        }
        if (found)
            continue;

        if (mSkillLineAbilityMap.lower_bound(spell_id)->second->id!=ability_id)
            continue;
        SpellEntry const *SpellEntry=sSpellStore.LookupEntry(spell_id);
        if (!SpellEntry)
            continue;
        std::string sRank = SpellEntry->Rank[sWorld.GetDefaultDbcLocale()];
        if (sRank.empty())
            continue;
        //exception to polymorph spells-make pig and turtle other chain than sheep
        if ((SpellEntry->SpellFamilyName==SPELLFAMILY_MAGE) && (SpellEntry->SpellFamilyFlags & 0x1000000) && (SpellEntry->SpellIconID!=82))
            continue;

        SpellRankEntry entry;
        SpellRankValue value;
        entry.SkillId=AbilityInfo->skillId;
        entry.SpellName=SpellEntry->SpellName[sWorld.GetDefaultDbcLocale()];
        entry.DurationIndex=SpellEntry->DurationIndex;
        entry.RangeIndex=SpellEntry->rangeIndex;
        entry.ProcFlags=SpellEntry->procFlags;
        entry.SpellFamilyFlags=SpellEntry->SpellFamilyFlags;
        entry.TargetAuraState=SpellEntry->TargetAuraState;
        entry.SpellVisual=SpellEntry->SpellVisual;
        entry.ManaCost=SpellEntry->manaCost;

        for (;;)
        {
            AbilityInfo=mSkillLineAbilityMap.lower_bound(spell_id)->second;
            value.Id=spell_id;
            value.Rank=SpellEntry->Rank[sWorld.GetDefaultDbcLocale()];
            RankMap.insert(std::pair<SpellRankEntry, SpellRankValue>(entry,value));
            spell_id=AbilityInfo->forward_spellid;
            SpellEntry=sSpellStore.LookupEntry(spell_id);
            if (!SpellEntry)
                break;
        }
    }

    BarGoLink bar(RankMap.size());

    uint32 count=0;

    for (std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry>::iterator itr = RankMap.begin();itr!=RankMap.end();)
    {
        SpellRankEntry entry=itr->first;
        //trac errors in extracted data
        std::multimap<char const *, std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry>::iterator> RankErrorMap;
        for (std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry>::iterator itr2 = RankMap.lower_bound(entry);itr2!=RankMap.upper_bound(entry);itr2++)
        {
            bar.step();
            RankErrorMap.insert(std::pair<char const *, std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry>::iterator>(itr2->second.Rank,itr2));
        }
        for (std::multimap<char const *, std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry>::iterator>::iterator itr2 = RankErrorMap.begin();itr2!=RankErrorMap.end();)
        {
            char const * err_entry=itr2->first;
            uint32 rank_count=RankErrorMap.count(itr2->first);
            if (rank_count>1)
            for (itr2 = RankErrorMap.lower_bound(err_entry);itr2!=RankErrorMap.upper_bound(err_entry);itr2++)
            {
                sLog.outDebug("There is a duplicate rank entry (%s) for spell: %u",itr2->first,itr2->second->second.Id);
                sLog.outDebug("Spell %u removed from chain data.",itr2->second->second.Id);
                RankMap.erase(itr2->second);
                itr=RankMap.lower_bound(entry);
            }
            else
                itr2++;
        }
        //do not proceed for spells with less than 2 ranks
        uint32 spell_max_rank=RankMap.count(entry);
        if (spell_max_rank<2)
        {
            itr=RankMap.upper_bound(entry);
            continue;
        }

        itr=RankMap.upper_bound(entry);

        //order spells by spells by spellLevel
        std::list<uint32> RankedSpells;
        uint32 min_spell_lvl=0;
        std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry>::iterator min_itr;
        for (;RankMap.count(entry);)
        {
            for (std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry>::iterator itr2 = RankMap.lower_bound(entry);itr2!=RankMap.upper_bound(entry);itr2++)
            {
                SpellEntry const *SpellEntry=sSpellStore.LookupEntry(itr2->second.Id);
                if (SpellEntry->spellLevel<min_spell_lvl || itr2==RankMap.lower_bound(entry))
                {
                    min_spell_lvl=SpellEntry->spellLevel;
                    min_itr=itr2;
                }
            }
            RankedSpells.push_back(min_itr->second.Id);
            RankMap.erase(min_itr);
        }

        //use data from talent.dbc
        uint16 talent_id=0;
        for (std::list<uint32>::iterator itr2 = RankedSpells.begin();itr2!=RankedSpells.end();)
        {
            if (TalentSpellPos const* TalentPos=GetTalentSpellPos(*itr2))
            {
                talent_id=TalentPos->talent_id;
                RankedSpells.erase(itr2);
                itr2 = RankedSpells.begin();
            }
            else
                itr2++;
        }
        if (talent_id)
        {
            TalentEntry const *TalentInfo = sTalentStore.LookupEntry(talent_id);
            for (uint8 rank=5;rank;rank--)
            {
                if (TalentInfo->RankID[rank-1])
                    RankedSpells.push_front(TalentInfo->RankID[rank-1]);
            }
        }

        count++;

        itr=RankMap.upper_bound(entry);
        uint32 spell_rank=1;
        for (std::list<uint32>::iterator itr2 = RankedSpells.begin(); itr2 != RankedSpells.end(); spell_rank++)
        {
            uint32 spell_id = *itr2;
            mSpellChains[spell_id].rank = spell_rank;
            mSpellChains[spell_id].first = RankedSpells.front();
            mSpellChains[spell_id].last = RankedSpells.back();
            mSpellChains[spell_id].cur = *itr2;

            itr2++;
            if (spell_rank < 2)
                mSpellChains[spell_id].prev = 0;

            if (spell_id == RankedSpells.back())
                mSpellChains[spell_id].next = 0;
            else
            {
                mSpellChains[*itr2].prev = spell_id;
                mSpellChains[spell_id].next = *itr2;
            }
        }
    }

    // BIG UGLY PIECE OF CODE ! BUT WORKS :p
    // Bear Form
    uint32 spell_id = 5487;
    mSpellChains[spell_id].prev = 0;
    mSpellChains[spell_id].next = 9634;
    mSpellChains[spell_id].first = 5487;
    mSpellChains[spell_id].last = 9634;
    mSpellChains[spell_id].rank = 1;

    spell_id = 9634;
    mSpellChains[spell_id].prev = 5487;
    mSpellChains[spell_id].next = 0;
    mSpellChains[spell_id].first = 5487;
    mSpellChains[spell_id].last = 9634;
    mSpellChains[spell_id].rank = 2;

    // Flight form
    spell_id = 33943;
    mSpellChains[spell_id].prev = 0;
    mSpellChains[spell_id].next = 40120;
    mSpellChains[spell_id].first = 33943;
    mSpellChains[spell_id].last = 40120;
    mSpellChains[spell_id].rank = 1;

    spell_id = 40120;
    mSpellChains[spell_id].prev = 33943;
    mSpellChains[spell_id].next = 0;
    mSpellChains[spell_id].first = 33943;
    mSpellChains[spell_id].last = 40120;
    mSpellChains[spell_id].rank = 2;
    /* moved to SpellMgr::GetSpellGreaterVersion()
    // Blessing of Kings
    spell_id = 20217;
    mSpellChains[spell_id].prev = 0;
    mSpellChains[spell_id].next = 25898;
    mSpellChains[spell_id].first = 20217;
    mSpellChains[spell_id].last = 25898;
    mSpellChains[spell_id].rank = 1;

    // Greater Blessing of Kings
    spell_id = 25898;
    mSpellChains[spell_id].prev = 20217;
    mSpellChains[spell_id].next = 0;
    mSpellChains[spell_id].first = 20217;
    mSpellChains[spell_id].last = 25898;
    mSpellChains[spell_id].rank = 2;

    // Greater Blessing of Sanctuary I
    spell_id = 25899;
    mSpellChains[spell_id].prev = 27168;    // BoS V
    mSpellChains[spell_id].next = 27169;    // GBoS II
    mSpellChains[spell_id].first = 20911;   // BoS I
    mSpellChains[spell_id].last = 27169;    // GBoS II
    mSpellChains[spell_id].rank = 6;

    // link BoS V with GBoS I
    mSpellChains[27168].next = spell_id;

    // Greater Blessing of Sanctuary II
    spell_id = 27169;
    mSpellChains[spell_id].prev = 25899;    // GBoS I
    mSpellChains[spell_id].next = 0;        // none
    mSpellChains[spell_id].first = 20911;   // BoS I
    mSpellChains[spell_id].last = 27169;    // GBoS II
    mSpellChains[spell_id].rank = 7;

    // set GBoS II as last for all BoS ranks
    mSpellChains[20911].last = spell_id;
    mSpellChains[20912].last = spell_id;
    mSpellChains[20913].last = spell_id;
    mSpellChains[20914].last = spell_id;
    mSpellChains[27168].last = spell_id;
    */
//uncomment these two lines to print yourself list of spell_chains on startup
//    for (UNORDERED_MAP<uint32, SpellChainNode>::iterator itr=mSpellChains.begin();itr!=mSpellChains.end();itr++)
//       sLog.outString("Id: %u, Rank: %d , %s",itr->first,itr->second.rank, sSpellStore.LookupEntry(itr->first)->Rank[sWorld.GetDefaultDbcLocale()]);

    sLog.outString();
    sLog.outString(">> Loaded %u spell chains",count);
}

void SpellMgr::LoadSpellLearnSkills()
{
    mSpellLearnSkills.clear();                              // need for reload case

    // search auto-learned skills and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    for (uint32 spell = 0; spell < sSpellStore.GetNumRows(); ++spell)
    {
        SpellEntry const* entry = sSpellStore.LookupEntry(spell);

        if (!entry)
            continue;

        for (int i = 0; i < 3; ++i)
        {
            if (entry->Effect[i] == SPELL_EFFECT_SKILL)
            {
                SpellLearnSkillNode dbc_node;
                dbc_node.skill = entry->EffectMiscValue[i];
                if (dbc_node.skill != SKILL_RIDING)
                    dbc_node.value = 1;
                else
                    dbc_node.value = entry->CalculateSimpleValue(i)*75;
                dbc_node.maxvalue = entry->CalculateSimpleValue(i)*75;

                SpellLearnSkillNode const* db_node = GetSpellLearnSkill(spell);

                mSpellLearnSkills[spell] = dbc_node;
                ++dbc_count;
                break;
            }
        }
    }

    sLog.outString();
    sLog.outString(">> Loaded %u Spell Learn Skills from DBC", dbc_count);
}

void SpellMgr::LoadSpellLearnSpells()
{
    mSpellLearnSpells.clear();                              // need for reload case

    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT entry, SpellID FROM spell_learn_spell");
    if (!result)
    {
        BarGoLink bar(1);
        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded 0 spell learn spells");
        sLog.outLog(LOG_DB_ERR, "`spell_learn_spell` table is empty!");
        return;
    }

    uint32 count = 0;

    BarGoLink bar(result->GetRowCount());
    do
    {
        bar.step();
        Field *fields = result->Fetch();

        uint32 spell_id    = fields[0].GetUInt32();

        SpellLearnSpellNode node;
        node.spell      = fields[1].GetUInt32();
        node.autoLearned= false;

        if (!sSpellStore.LookupEntry(spell_id))
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_learn_spell` does not exist",spell_id);
            continue;
        }

        if (!sSpellStore.LookupEntry(node.spell))
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_learn_spell` does not exist",node.spell);
            continue;
        }

        mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell_id,node));

        ++count;
    } while (result->NextRow());

    // search auto-learned spells and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    for (uint32 spell = 0; spell < sSpellStore.GetNumRows(); ++spell)
    {
        SpellEntry const* entry = sSpellStore.LookupEntry(spell);

        if (!entry)
            continue;

        for (int i = 0; i < 3; ++i)
        {
            if (entry->Effect[i]==SPELL_EFFECT_LEARN_SPELL)
            {
                SpellLearnSpellNode dbc_node;
                dbc_node.spell       = entry->EffectTriggerSpell[i];
                dbc_node.autoLearned = true;

                SpellLearnSpellMap::const_iterator db_node_begin = GetBeginSpellLearnSpell(spell);
                SpellLearnSpellMap::const_iterator db_node_end   = GetEndSpellLearnSpell(spell);

                bool found = false;
                for (SpellLearnSpellMap::const_iterator itr = db_node_begin; itr != db_node_end; ++itr)
                {
                    if (itr->second.spell == dbc_node.spell)
                    {
                        sLog.outLog(LOG_DB_ERR, "Spell %u auto-learn spell %u in spell.dbc then the record in `spell_learn_spell` is redundant, please fix DB.",
                            spell,dbc_node.spell);
                        found = true;
                        break;
                    }
                }

                if (!found)                                  // add new spell-spell pair if not found
                {
                    mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell,dbc_node));
                    ++dbc_count;
                }
            }
        }
    }

    sLog.outString();
    sLog.outString(">> Loaded %u spell learn spells + %u found in DBC", count, dbc_count);
}

void SpellMgr::LoadSpellScriptTarget()
{
    mSpellScriptTarget.clear();                             // need for reload case

    uint32 count = 0;

    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT entry,type,targetEntry FROM spell_script_target");

    if (!result)
    {
        BarGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded 0 spell script target");
        sLog.outLog(LOG_DB_ERR, "`spell_script_target` table is empty!");
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 spellId     = fields[0].GetUInt32();
        uint32 type        = fields[1].GetUInt32();
        uint32 targetEntry = fields[2].GetUInt32();

        SpellEntry const* spellProto = sSpellStore.LookupEntry(spellId);

        if (!spellProto)
        {
            sLog.outLog(LOG_DB_ERR, "Table `spell_script_target`: spellId %u listed for TargetEntry %u does not exist.",spellId,targetEntry);
            continue;
        }

        if (type >= MAX_SPELL_TARGET_TYPE)
        {
            sLog.outLog(LOG_DB_ERR, "Table `spell_script_target`: target type %u for TargetEntry %u is incorrect.",type,targetEntry);
            continue;
        }

        switch (type)
        {
            case SPELL_TARGET_TYPE_GAMEOBJECT:
            {
                if (targetEntry==0)
                    break;

                if (!sGOStorage.LookupEntry<GameObjectInfo>(targetEntry))
                {
                    sLog.outLog(LOG_DB_ERR, "Table `spell_script_target`: gameobject template entry %u does not exist.",targetEntry);
                    continue;
                }
                break;
            }
            default:
            {
                //players
                /*if(targetEntry==0)
                {
                    sLog.outLog(LOG_DB_ERR, "Table `spell_script_target`: target entry == 0 for not GO target type (%u).",type);
                    continue;
                }*/
                if (targetEntry && !sCreatureStorage.LookupEntry<CreatureInfo>(targetEntry))
                {
                    sLog.outLog(LOG_DB_ERR, "Table `spell_script_target`: creature template entry %u does not exist.",targetEntry);
                    continue;
                }
                const CreatureInfo* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(targetEntry);

                if (spellId == 30427 && !cInfo->SkinLootId)
                {
                    sLog.outLog(LOG_DB_ERR, "Table `spell_script_target` has creature %u as a target of spellid 30427, but this creature has no skinlootid. Gas extraction will not work!", cInfo->Entry);
                    continue;
                }
                break;
            }
        }

        mSpellScriptTarget.insert(SpellScriptTarget::value_type(spellId,SpellTargetEntry(SpellScriptTargetType(type),targetEntry)));

        ++count;
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u Spell Script Targets", count);
}

void SpellMgr::LoadSpellPetAuras()
{
    mSpellPetAuraMap.clear();                                  // need for reload case

    uint32 count = 0;

    //                                                       0      1    2
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT spell, pet, aura FROM spell_pet_auras");
    if (!result)
    {

        BarGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u spell pet auras", count);
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint16 spell = fields[0].GetUInt16();
        uint16 pet = fields[1].GetUInt16();
        uint16 aura = fields[2].GetUInt16();

        SpellPetAuraMap::iterator itr = mSpellPetAuraMap.find(spell);
        if (itr != mSpellPetAuraMap.end())
        {
            itr->second.AddAura(pet, aura);
        }
        else
        {
            SpellEntry const* spellInfo = sSpellStore.LookupEntry(spell);
            if (!spellInfo)
            {
                sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_pet_auras` does not exist", spell);
                continue;
            }
            int i = 0;
            for (; i < 3; ++i)
                if ((spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA &&
                    spellInfo->EffectApplyAuraName[i] == SPELL_AURA_DUMMY) ||
                    spellInfo->Effect[i] == SPELL_EFFECT_DUMMY)
                    break;

            if (i == 3)
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Spell %u listed in `spell_pet_auras` does not have dummy aura or dummy effect", spell);
                continue;
            }

            SpellEntry const* spellInfo2 = sSpellStore.LookupEntry(aura);
            if (!spellInfo2)
            {
                sLog.outLog(LOG_DB_ERR, "Aura %u listed in `spell_pet_auras` does not exist", aura);
                continue;
            }

            PetAura pa(pet, aura, spellInfo->EffectImplicitTargetA[i] == TARGET_UNIT_PET, spellInfo->CalculateSimpleValue(i));
            mSpellPetAuraMap[spell] = pa;
        }

        ++count;
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u spell pet auras", count);
}

// set data in core for now
void SpellMgr::LoadSpellCustomAttr()
{
    SpellEntry *spellInfo;
    for (uint32 i = 0; i < GetSpellStore()->GetNumRows(); ++i)
    {
        spellInfo = (SpellEntry*)GetSpellStore()->LookupEntry(i);
        if (!spellInfo)
            continue;
        spellInfo->AttributesCu = 0;

        bool auraSpell = true;
        for (uint32 j = 0; j < 3; ++j)
        {
            if (spellInfo->Effect[j])
                if (spellInfo->Effect[j] != SPELL_EFFECT_APPLY_AURA
                || SpellTargetType[spellInfo->EffectImplicitTargetA[j]] != TARGET_TYPE_UNIT_TARGET)
                //ignore target party for now
                {
                    auraSpell = false;
                    break;
                }
        }
        if (auraSpell)
            spellInfo->AttributesCu |= SPELL_ATTR_CU_AURA_SPELL;

        for (uint32 j = 0; j < 3; ++j)
        {
            switch (spellInfo->EffectApplyAuraName[j])
            {
                case SPELL_AURA_PERIODIC_DAMAGE:
                case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
                case SPELL_AURA_PERIODIC_LEECH:
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_AURA_DOT;
                    break;
                case SPELL_AURA_PERIODIC_HEAL:
                case SPELL_AURA_OBS_MOD_HEALTH:
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_AURA_HOT;
                    break;
                case SPELL_AURA_MOD_ROOT:
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_FAKE_DELAY;
                    // no break intended
                case SPELL_AURA_MOD_DECREASE_SPEED:
                    // Creature daze exception
                    if (spellInfo->Id == 1604)
                        break;
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_MOVEMENT_IMPAIR;
                    break;
                case SPELL_AURA_MOD_POSSESS:
                case SPELL_AURA_MOD_CONFUSE:
                case SPELL_AURA_MOD_CHARM:
                case SPELL_AURA_MOD_FEAR:
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_FAKE_DELAY;
                    // no break intended
                case SPELL_AURA_MOD_STUN:
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_AURA_CC;
                    spellInfo->AttributesCu &= ~SPELL_ATTR_CU_MOVEMENT_IMPAIR;
                    break;
                default:
                    break;
            }

            switch (spellInfo->Effect[j])
            {
                case SPELL_EFFECT_SCHOOL_DAMAGE:
                case SPELL_EFFECT_WEAPON_DAMAGE:
                case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
                case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
                case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                case SPELL_EFFECT_HEAL:
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_DIRECT_DAMAGE;
                    break;
                case SPELL_EFFECT_CHARGE:
                case SPELL_EFFECT_CHARGE2:
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_CHARGE;
                    break;
                case SPELL_EFFECT_TRIGGER_SPELL:
                    if (spellInfo->Id == 39897)   // Mass Dispel should not have effect_trigger_missile? to be verified now
                        break;
                    if (IsPositionTarget(spellInfo->EffectImplicitTargetA[j]) ||
                        spellInfo->Targets & (TARGET_FLAG_SOURCE_LOCATION|TARGET_FLAG_DEST_LOCATION))
                        spellInfo->Effect[j] = SPELL_EFFECT_TRIGGER_MISSILE;
                    break;
                case SPELL_EFFECT_TELEPORT_UNITS:
                    if (spellInfo->EffectImplicitTargetA[j] == 17 && spellInfo->EffectImplicitTargetB[j] == 0)
                    {
                        spellInfo->EffectImplicitTargetA[j] = 1;
                        spellInfo->EffectImplicitTargetB[j] = 17;
                    }
                    break;
            }
        }

        if (spellInfo->SpellVisual == 3879)
            spellInfo->AttributesCu |= SPELL_ATTR_CU_CONE_BACK;

        if ((spellInfo->SpellFamilyName == SPELLFAMILY_DRUID && spellInfo->SpellFamilyFlags & 0x1000LL && spellInfo->SpellIconID == 494) || spellInfo->Id == 33745 /* Lacerate */)
            spellInfo->AttributesCu |= SPELL_ATTR_CU_IGNORE_ARMOR;

        // Modify SchoolMask to allow them critically heal
        // Healthstones
        if (spellInfo->SpellFamilyName == SPELLFAMILY_WARLOCK && spellInfo->SpellFamilyFlags & 0x10000LL)
            spellInfo->SchoolMask = SPELL_SCHOOL_MASK_SHADOW;

        // so channeled  spell can NOT be interrupted by movement :p
        if (spellInfo->Effect[0] == SPELL_EFFECT_STUCK)
        {
            if (IsChanneledSpell(spellInfo))
                spellInfo->ChannelInterruptFlags &= ~AURA_INTERRUPT_FLAG_MOVE;
            else
                spellInfo->InterruptFlags &= ~SPELL_INTERRUPT_FLAG_MOVEMENT;
        }

        // test, in most cases turning is caused by channeling, so spells just dont work
        spellInfo->ChannelInterruptFlags &= ~AURA_INTERRUPT_FLAG_TURNING;

        LoadCustomSpellCooldowns(spellInfo);

        if (spellInfo->HasApplyAura(SPELL_AURA_DAMAGE_SHIELD) ||
            spellInfo->HasApplyAura(SPELL_AURA_PERIODIC_LEECH) ||
            spellInfo->HasEffect(SPELL_EFFECT_HEALTH_LEECH))
            spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;

        // if spellInfo has mechanic - then its effects SHOULD NOT have same mechanic.
        // override spell effects mechanic to none if spell has same overall mechanic
        // This is tested - for sure this code is needed
        if (spellInfo->Mechanic)
        {
            for (uint32 j = 0; j < 3; ++j)
            {
                if (spellInfo->EffectMechanic[j] == spellInfo->Mechanic)
                    spellInfo->EffectMechanic[j] = MECHANIC_NONE;
            }
        }
        // this is done for the reason that mechanic is already worked out as SPELL mechanic. Leaving effect mechanic the same will cause double-effect on spell

        if (spellInfo->SpellIconID == 109 && spellInfo->SpellVisual == 192)
            spellInfo->AttributesCu |= SPELL_ATTR_CU_BLOCK_STEALTH;

        switch (spellInfo->SpellFamilyName)
        {
            case SPELLFAMILY_GENERIC:
            {
                switch (spellInfo->Id)
                {
                case 126: //Eye of Kilrog -> no haste to duration
                    spellInfo->Attributes |= SPELL_ATTR_TRADESPELL;
                    break;
                case 52009: // Goblin Rocket Launcher
                    spellInfo->EffectMiscValue[0] = 20865;
                    break;
                case 15852:
                    spellInfo->Dispel = DISPEL_NONE;
                    break;
                case 46337: // Crab disguise
                    spellInfo->AuraInterruptFlags |= AURA_INTERRUPT_FLAG_CAST;
                    break;
                case 34171: // Underbat - Tentacle Lash
                case 37956:
                    spellInfo->AttributesEx |= SPELL_ATTR_EX_UNK9;
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_FROM_BEHIND;
                    break;
                case 16613: // some quest spell spamming with non-existing triggered
                    spellInfo->Effect[2] = 0;
                    break;
                case 39280: // same here
                    spellInfo->Effect[1] = 0;
                    break;
                case 38829: // arcatraz sentinels prevent spam
                    spellInfo->EffectImplicitTargetB[0] = TARGET_UNIT_AREA_ENEMY_DST;
                    break;
                case 26635: // berserking troll racial
                    spellInfo->EffectDieSides[0] = 1;
                    break;
                case 40669: // egbert egg- run away spell
                case 36200: // doomsaw 12 sec
                    spellInfo->DurationIndex = 29; // 12 secs, guess
                    break;
                case 37674: // leotheras chaos blast
                    spellInfo->AttributesEx |= SPELL_ATTR_EX_CANT_BE_REFLECTED;
                    break;
                case 37675: // leotheras the blind - chaos blast should be spell not ability
                    spellInfo->AttributesEx |= SPELL_ATTR_EX_CANT_BE_REFLECTED;
                    spellInfo->Attributes &= ~SPELL_ATTR_ABILITY;
                    break;
                case 8690: // hearthstone - no haste bonus
                    spellInfo->Attributes |= SPELL_ATTR_TRADESPELL;
                    break;
                case 7620: // fishing - no haste bonus
                case 7731:
                case 7732:
                case 18248:
                case 33095:
                    spellInfo->Attributes |= SPELL_ATTR_TRADESPELL;
                    break;
                case 44586: // NPC prayer of mending
                    spellInfo->procFlags &= ~PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL;
                    break;
                case 40281: // simon game spells
                case 40287:
                case 40288:
                case 40289:
                    spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_DEST;
                    break;
                case 30532: // karazhan chess teleport
                    spellInfo->rangeIndex = 6;
                    break;
                case 18818: // skullflame shield flamestrike
                case 7712:  // blazefury medalion
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                    break;
                case 9806: // faerie-fire-alikes
                case 9991:
                case 16432:
                case 35325:
                case 35328:
                case 35329:
                case 35331:
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_BLOCK_STEALTH;
                    break;
                case 45275:
                case 42336: // bucket/torch visuals duration 60sec
                    spellInfo->DurationIndex = 3;
                    break;
                case 44969: // energize crystal ward
                    spellInfo->RequiresSpellFocus = 1483;
                    break;
                case 44999: // conversions on ioqd, set guardians duration to 12 sec and let them spawn multiple times
                    spellInfo->DurationIndex = 29;
                    spellInfo->RecoveryTime = 1;
                    break;
                case 26194: // FIXME: pretbc spell scaling does not work with resistance correctly
                    spellInfo->Attributes &= ~SPELL_ATTR_LEVEL_DAMAGE_CALCULATION;
                    break;
                case 46648: // no pull for cosmetic spell
                    spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
                    break;
                case 45885: // KJ shadow spike
                    spellInfo->EffectImplicitTargetB[0] = TARGET_UNIT_AREA_ENEMY_DST;
                    spellInfo->EffectImplicitTargetB[1] = TARGET_UNIT_AREA_ENEMY_DST;
                    break;
                case 46589: // Shadow spike, target destination set in aura::triggerspell
                    spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_DEST;
                    spellInfo->speed = 6;
                    break;
                case 38112: // vashj shield generator
                    spellInfo->Attributes |= SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY;
                    break;
                case 45839: // KJ drake control, no haste
                case 45838:
                    spellInfo->Attributes |= SPELL_ATTR_TRADESPELL;
                    break;
                case 45072: // Arcane charges ignore los
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
                    break;
                case 38544: // coax mamrot
                    spellInfo->EffectMiscValueB[0] = SUMMON_TYPE_POSESSED;
                    break;
                case 7720: // summon effect
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
                    break;
                case 37851: // tag diemetradon, frankly it makes no sense, done manually in apply dummy aura
                    spellInfo->Effect[1] = 0;
                    break;
                case 42338: // raptor bait, target quest credit
                    spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_NEARBY_ENTRY;
                    break;
                case 42339: // water bucket throw
                    spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_DEST;
                    break;
                case 36576: // no damage calc for shaleskin
                case 37950: // no damage calc for mutated blood
                case 33551: // corrosive acid (sl first boss -armor debuff)
                    spellInfo->Attributes &= ~SPELL_ATTR_LEVEL_DAMAGE_CALCULATION;
                    break;
                case 36817: // ashtongue corruptors channel spell
                case 37204:
                case 37205:
                case 37206:
                    spellInfo->Attributes |= SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY;
                    spellInfo->AttributesEx4 &= ~SPELL_ATTR_EX4_FORCE_TRIGGERED;
                    spellInfo->AttributesEx |= SPELL_ATTR_EX_UNAFFECTED_BY_SCHOOL_IMMUNE;
                    spellInfo->InterruptFlags = 0xF;
                    break;
                case 36826: // ashtongue corruptor summon amount
                case 37201:
                    spellInfo->EffectBasePoints[0] = 2;
                    break;
                case 37202:
                case 37203:
                    spellInfo->EffectBasePoints[0] = 3;
                    break;
                case 48025: // headless horseman mount
                    spellInfo->Attributes |= SPELL_ATTR_CANT_USED_IN_COMBAT | SPELL_ATTR_OUTDOORS_ONLY;
                    break;
                case 38469: // channel, interrupt casting on combat
                case 35756:
                    spellInfo->ChannelInterruptFlags |= AURA_INTERRUPT_FLAG_MOVE;
                    break;
                case 33814: // visual meteor summon infernal
                    spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_DEST;
                    spellInfo->EffectImplicitTargetA[1] = TARGET_DEST_DEST;
                    break;
                case 36558: // artor prison visual
                    spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_NEARBY_ENTRY;
                    break;
                case 36449: // maghteridon roof falling at phase change
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_IGNORE_ARMOR;
                    break;
                case 31717: // black stalker chain dmg
                    spellInfo->EffectBasePoints[0] = 1700;
                    spellInfo->EffectDieSides[0] = 400;
                    spellInfo->Attributes &= ~SPELL_ATTR_LEVEL_DAMAGE_CALCULATION;
                    break;
                case 38511: // vashj persuasion
                    spellInfo->MaxAffectedTargets = 1;
                    spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                    spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_TARGET_ENEMY;
                    spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_TARGET_ENEMY;
                    spellInfo->EffectImplicitTargetB[0] = 0;
                    spellInfo->EffectImplicitTargetB[1] = 0;
                    spellInfo->EffectImplicitTargetB[2] = 0;
                    break;
                case 31673: // hungarfen remove visual from shrooms
                    spellInfo->Effect[2] = 0;
                    break;
                case 38523: // netherspite netherbreath
                    spellInfo->rangeIndex = 6;
                    spellInfo->MaxAffectedTargets = 4;
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
                    break;
                case 32723: // summon bonechewer riding wolf 60 sec stay only
                case 34368:
                case 12746: // wastewalker shadow mage void
                    spellInfo->DurationIndex = 3;
                    break;
                case 30846: // proximity bomb make it trap
                    spellInfo->Effect[0] = SPELL_EFFECT_SUMMON_OBJECT_SLOT1;
                    break;
                case 31689: // underbog mushroom effect proper targeting
                    spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_CASTER;
                    spellInfo->EffectImplicitTargetB[0] = 0;
                    break;
                case 37476: // chess horde king cleave in front
                    spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_CONE_ENTRY;
                    spellInfo->EffectImplicitTargetB[0] = 0;
                    break;
                case 29683: // spotlight also on players?
                    spellInfo->EffectImplicitTargetB[1] = 0;
                    break;
                case 38459: // broggok poison bolt
                case 30917:
                    spellInfo->MaxAffectedTargets = 2;
                    break;
                case 39045: //summon ssc parasite
                    spellInfo->AreaId = 0;
                    break;
                case 30843: // prince enfeblee efect
                    spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_CASTER;
                    spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
                    spellInfo->EffectImplicitTargetB[0] = 0;
                    spellInfo->EffectImplicitTargetB[1] = 0;
                    break;
                case 29690: // drunken skull krack drunk 100 instead of 1000
                    spellInfo->EffectBasePoints[2] = 99;
                    break;
                case 14157: // ruthlessness no cause combat on proc
                    spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
                    break;
                case 31479: // black morass dragon helper 60 sec despawn
                case 11939: // some random add summon
                    spellInfo->DurationIndex = 3;
                    break;
                case 33493: // mark of malice
                    spellInfo->Attributes &= ~SPELL_ATTR_ON_NEXT_SWING_2;
                    break;
                case 30009: // dip in a moonwell
                    spellInfo->DurationIndex = 4;
                    break;
                case 37116: // primal fury (without that learning 1/2 talent gives you 2/2)
                case 37117:
                    spellInfo->Attributes |= SPELL_ATTR_PASSIVE;
                    break;
                case 31626: // unholy frenzy GCD
                    spellInfo->StartRecoveryCategory = 133;
                    spellInfo->StartRecoveryTime = 1500;
                    break;
                case 39985: // posses - shartuul transporter
                    spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                    break;
                case 32096: // Thrallmar's Favor
                    spellInfo->EffectBasePoints[0] = 24;
                    spellInfo->EffectApplyAuraName[0] = 190;
                    spellInfo->EffectMiscValue[0] = 947;
                    spellInfo->Effect[1] = 6;
                    spellInfo->EffectBasePoints[1] = 4;
                    spellInfo->EffectImplicitTargetA[1] = 1;
                    spellInfo->EffectApplyAuraName[1] = 200;
                    spellInfo->EffectBaseDice[1] = 1;
                    break;
                case 32098: // Honor Hold's Favor
                    spellInfo->EffectBasePoints[0] = 24;
                    spellInfo->EffectApplyAuraName[0] = 190;
                    spellInfo->EffectMiscValue[0] = 946;
                    spellInfo->Effect[1] = 6;
                    spellInfo->EffectBasePoints[1] = 4;
                    spellInfo->EffectImplicitTargetA[1] = 1;
                    spellInfo->EffectApplyAuraName[1] = 200;
                    spellInfo->EffectBaseDice[1] = 1;
                    break;
                    //PRE NERF ZONE
                case 37060: // nightbane immolation aura efect
                    spellInfo->EffectBasePoints[0] = 470;
                    break;
                case 44032: // manticron cube exhaustion 180sec
                    spellInfo->DurationIndex = 25;
                    break;
                case 36152: 
                    // No min range (not a nerf)
                    spellInfo->rangeIndex = 5;
                    spellInfo->EffectBasePoints[0] = 2500;
                    spellInfo->EffectDieSides[0] = 1000;
                    break;
                case 33237:
                    spellInfo->EffectBasePoints[0] = 2250;
                    break;
                case 34435:
                    spellInfo->EffectBasePoints[0] = 1800;
                    break;
                case 30510:
                    spellInfo->CastingTimeIndex = 4;
                    break;
                case 37566:
                    spellInfo->CastingTimeIndex = 1;
                    break;
                case 30065:
                    spellInfo->DurationIndex = 9;
                    break;

                    // mana tombs crap
                case 35944:
                case 35945:
                case 34922:
                case 34930:
                case 38760:
                case 32365:
                case 32364:
                    spellInfo->EffectBasePoints[0] *= 2;
                    break;
                case 33919:
                case 37669:
                    spellInfo->EffectBasePoints[0] *= 4;
                    if (spellInfo->Id == 37669)
                        spellInfo->EffectBasePoints[1] *= 4;
                    break;
                }
                if (spellInfo->SpellIconID == 184 && spellInfo->Attributes == 4259840)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                else if (spellInfo->SpellIconID == 2367) // remove flag from steam tonk & crashin trashin racers
                    spellInfo->AttributesEx4 &= ~SPELL_ATTR_EX4_FORCE_TRIGGERED;

                break;
            }
            case SPELLFAMILY_DRUID:
            {
                if ((spellInfo->SpellFamilyFlags & 128) &&
                   ((spellInfo->EffectImplicitTargetA[0] == TARGET_DEST_DYNOBJ_ALLY) ||
                   (spellInfo->EffectImplicitTargetA[0] == TARGET_DEST_CHANNEL)))
                        spellInfo->EffectImplicitTargetA[0] = 0; // tranquility log spam

                switch (spellInfo->Id)
                {
                    case 16998: // Savage Fury
                    case 16999:
                        spellInfo->Effect[2] = 0;
                        break;
                    case 17768: //Wolfshead Helm - Wolfshead Helm Energy in cat form
                        spellInfo->EffectTriggerSpell[1] = 29940;
                        break;
                }
                break;
            }
            case SPELLFAMILY_SHAMAN:
            {
                if (spellInfo->Id == 16368)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                else if (spellInfo->SpellFamilyFlags & 0x800000LL) // wf attack
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_TRIGGERED_CAN_TRIGGER;
                break;
            }
            case SPELLFAMILY_PALADIN:
            {
                // Judgement & seal of Light
                if (spellInfo->SpellFamilyFlags & 0x100040000LL)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                // Seal of Righteousness trigger - already computed for parent spell
                else if (spellInfo->SpellIconID==25 && spellInfo->AttributesEx4 & 0x00800000LL)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_FIXED_DAMAGE;
                // Blessing of Sanctuary, greater and normal
                else if (spellInfo->SpellIconID == 1804 || spellInfo->SpellIconID == 19)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                // Devotion Aura
                else if (spellInfo->SpellFamilyFlags & 0x40 && spellInfo->SpellIconID == 291)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SCROLL_STACK;
                else if (spellInfo->Id == 25997) // Eye for an eye
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                else if (spellInfo->Id == 43743) // improved Seal of Righteousness
                    spellInfo->Effect[1] = 0;

                if (spellInfo->SpellFamilyFlags & 0x0001040002200000LL) // seal of blood/command/holy shock
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_TRIGGERED_CAN_TRIGGER;
                break;
            }
            case SPELLFAMILY_PRIEST:
            {
                // Mana Burn
                if (spellInfo->SpellFamilyFlags & 0x10LL && spellInfo->SpellIconID == 212)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                // Health Link T5 Hunter/Warlock bonus
                else if (spellInfo->Id == 37382)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                // Reflective Shield
                else if (!spellInfo->SpellFamilyFlags && spellInfo->SpellIconID == 566)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;

                else if (!spellInfo->SpellFamilyFlags && spellInfo->SpellIconID == 237)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;

                // Divine Spirit/Prayer of Spirit
                else if (spellInfo->SpellFamilyFlags & 0x20)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SCROLL_STACK;
                // Power Word: Fortitude/Prayer of Fortitude
                else if (spellInfo->SpellFamilyFlags & 0x08 && spellInfo->SpellVisual == 278)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SCROLL_STACK;
                // mass dispel simplification - combine 3 spells limiting triggering
                else if (spellInfo->Id == 32375)
                {
                    spellInfo->Effect[1] = SPELL_EFFECT_DISPEL;
                    spellInfo->EffectRadiusIndex[1] = 18;
                    spellInfo->EffectMiscValue[1] = 1;
                    spellInfo->Effect[2] = SPELL_EFFECT_TRIGGER_SPELL;
                    spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_AREA_ENEMY_DST;
                    spellInfo->EffectRadiusIndex[2] = 13;
                    spellInfo->EffectTriggerSpell[2] = 39897;
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
                }
                else if (spellInfo->Id == 32409) // shadow word: death back damage, unaffected by anything
                {
                    spellInfo->SpellFamilyFlags = 0;
                }
                else if (spellInfo->Id == 10909 || spellInfo->Id == 2096) // mind vision no haste
                    spellInfo->Attributes |= SPELL_ATTR_TRADESPELL;
                else if (spellInfo->Id == 33619) // reflective shield effect
                {
                    spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
                }
                else if (spellInfo->Id == 32445) // maiden of virtue holy wrath
                {
                    spellInfo->EffectBasePoints[0] = 2299;
                    spellInfo->EffectDieSides[0] = 201;
                }
                break;
            }
            case SPELLFAMILY_MAGE:
            {
                // Molten Armor
                if (spellInfo->SpellFamilyFlags & 0x800000000LL)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                // Arcane Intellect/Brilliance
                else if (spellInfo->SpellFamilyFlags & 0x0400)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SCROLL_STACK;
                // some quest spell spamming with non-existing triggered
                else if (spellInfo->Id == 39280)
                    spellInfo->Effect[1] = 0;
                else if (spellInfo->Id == 33395) // elementals' freeze (blocks frost nova)
                    spellInfo->Category = 0;

                if (spellInfo->SpellFamilyFlags & 0x0000000800200080LL) // Arcane Missles / Blizzard / Molten Armor proc
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_TRIGGERED_CAN_TRIGGER;

                if (spellInfo->Id == 33462) // mana tombs crap
                    spellInfo->EffectBasePoints[0] *= 2;
                break;
            }
            case SPELLFAMILY_WARLOCK:
            {
                // Drain Mana
                if (spellInfo->SpellFamilyFlags & 0x10LL && spellInfo->SpellIconID == 548)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                // Healthstone
                else if (spellInfo->SpellFamilyFlags & 0x10000LL ||
                    spellInfo->Id == 30294) // soul leech effect
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                // Death Coil
                else if (spellInfo->SpellVisual == 9152)
                    spellInfo->Attributes |= SPELL_ATTR_CANT_CANCEL;
                if (spellInfo->SpellFamilyFlags & 0x0000800000000000LL) // Seed of corruption (proc one from another)
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_TRIGGERED_CAN_TRIGGER;

                if (spellInfo->Id == 27285) // soc effectnot ignore los
                    spellInfo->AttributesEx2 &= ~SPELL_ATTR_EX2_IGNORE_LOS;
                break;
            }
            case SPELLFAMILY_HUNTER:
            {
                if (spellInfo->Id == 45172) // BE guards shooting at flying players on isle, prevent spam
                    spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_AREA_ENEMY_DST;
                else if (spellInfo->Id == 34026) // kill command, all handled by first dummy effect
                    spellInfo->Effect[1] = 0;
                else if (spellInfo->Id == 1543 || spellInfo->Id == 28822) // flares
                {
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_BLOCK_STEALTH;
                    spellInfo->speed = 15;
                    spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
                }
                else if (spellInfo->Id == 3045 || spellInfo->Id == 36828) // Rapid Fire
                    spellInfo->speed = 0;
                if (spellInfo->SpellFamilyFlags & 0x0000200000000014LL) // trap effects
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_TRIGGERED_CAN_TRIGGER;
                spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                break;
            }
            case SPELLFAMILY_WARRIOR:
                spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                break;
            case SPELLFAMILY_ROGUE:
                if (spellInfo->SpellFamilyFlags & 0x600000000LL) // mutilate
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_TRIGGERED_CAN_TRIGGER;

                spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                break;
        }
        switch (i)
        {
            /* FIXED DAMAGE SPELLS */
            case 12654: // Ignite
                spellInfo->AttributesCu |= SPELL_ATTR_CU_FIXED_DAMAGE;
                break;
            case 20532: // Intense Heat (Majordomo Executus lava pit)
                spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_CANT_CRIT;
                // no break here
            /* NO SPELL DMG COEFF */
            case 16614: // Storm Gauntlets
            case 7714:  // Fiery Plate Gauntlets
            case 40471: // Enduring Light - T6 proc
            case 40472: // Enduring Judgement - T6 proc
            case 32221: // Seal of Blood
            case 32220: // Judgement of Blood
            case 28715: // Flame Cap
            case 37284: // Scalding Water
            case 6297:  // Fiery Blaze
            case 46579: // Deathfrost
            case 28734: // Mana Tap
            case 32409: // SW: Death

            // Six Demon Bag spells
            case 45297: // Chain Lightning
            case 23102: // Frostbolt
            case 9487:  // Fireball

            case 45429: // Shattered Sun Pendant of Acumen: Scryers ex proc
            case 27655: // Heart of Wyrmthalak: Flame Lash proc
            case 45055: // Shadow Bolt (Timbal's Focusing Crystal)
            case 37661: // The Lightning Capacitor, lightning bolt spell
            case 38324: // Regeneration (Fel Reaver's Piston)
            case 28733: // Arcane Torrent
            case 43731: // Lightning Zap on critters (Stormchops)
            case 43733: // Lightning Zap on others (Stormchops)
                spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                break;
            /* WELL FEED */
            case 18191:
            case 46687:
            /* RUMS */
            case 5257:
            case 5021:
            case 5020:
            case 22789:
            case 37058:
            case 25804:
            case 25722:
            case 25037:
            case 20875:
            /* DIFF FOOD */
            case 18193:
            case 18125:
            case 18192:
            case 18141:
            case 18194:
            case 18222:
            case 22730:
            case 23697:
            case 25661: // Dirge's Kickin' Chimaerok Chops
                spellInfo->AttributesCu |= SPELL_ATTR_CU_TREAT_AS_WELL_FED;
                break;
            /* Scrolls - no stack */
            case 8112:  // Spirit I
            case 8113:  // Spirit II
            case 8114:  // Spirit III
            case 12177: // Spirit IV
            case 33080: // Spirit V
            case 8099:  // Stamina I
            case 8100:  // Stamina II
            case 8101:  // Stamina III
            case 12178: // Stamina IV
            case 33081: // Stamina V
            case 8096:  // Intellect I
            case 8097:  // Intellect II
            case 8098:  // Intellect III
            case 12176: // Intellect IV
            case 33078: // Intellect V
            case 8091:  // Protection I
            case 8094:  // Protection II
            case 8095:  // Protection III
            case 12175: // Protection IV
            case 33079: // Protection V
            /* Other to not stack with scrolls */
            case 35078: // Band of the Eternal Defender
                spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SCROLL_STACK;
                break;
            /* ROGUE CUSTOM ATTRIBUTES */
            case 5171:
            case 6774:                     // Slice'n'Dice
                // spellInfo->AttributesEx |= SPELL_ATTR_EX_NOT_BREAK_STEALTH; wotlk feature
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO; // Do not put caster in combat after use
                break;
            /* SHAMAN CUSTOM ATTRIBUTES */
            case 2895:                      // Wrath of Air Totem - disallow weird stacking
                spellInfo->EffectImplicitTargetA[0] = spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
                spellInfo->EffectImplicitTargetB[0] = spellInfo->EffectImplicitTargetB[1] = 0;
                break;
            /* WARLOCK CUSTOM ATTRIBUTES */
            /* HUNTER CUSTOM ATTRIBUTES */
            /* WARRIOR CUSTOM ATTRIBUTES */
            case 12721: // Deep Wounds
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_DONE_BONUS;
                break;
            // Triggered spells that should be delayed
            case 20272: // Illumination
            case 32848: // Mana Restore
            case 14189: // Seal Fate
            case 14157: // Ruthlessness
            case 14181: // Relentless Strikes
            case 17794: // Improved Shadow Bolt ranks 1-5
            case 17797:
            case 17798:
            case 17799:
            case 17800:
            case 20424: // seal of command damage
            case 20253: // Intercept Stun (Rank 1)
            case 20614: // Intercept Stun (Rank 2)
            case 20615: // Intercept Stun (Rank 3)
            case 25273: // Intercept Stun (Rank 4)
            case 25274: // Intercept Stun (Rank 5)
            case 7922:  // Charge Stun
            case 30153: // Felguard Intercept Stun rank 1
            case 30195: // Felguard Intercept Stun rank 2
            case 30197: // Felguard Intercept Stun rank 3
            case 25999: // Boar Charge Immobilize
                        // Feral charge? - unaffected
                spellInfo->AttributesCu |= SPELL_ATTR_CU_FAKE_DELAY;
                break;
            /* UNSORTED */
            /* Damage Corrections */
            case 16785: // Flamebreak
                spellInfo->EffectBasePoints[0] = 24;
                break;
            case 17289: // Shadow Shock
                spellInfo->EffectBasePoints[0] = 74;
                break;
            case 36920: // Fireball (Vazruden)
                spellInfo->EffectBasePoints[0] = 151;
                break;
            case 34934: // Shadow Bolt Volley
                spellInfo->EffectBasePoints[0] = 124;
                break;
            case 40317: // Throw
                spellInfo->EffectBasePoints[0] = 199;
                break;
            case 40103: // Sludge Nova
                spellInfo->EffectBasePoints[0] = urand(24, 29);
                break;
            case 40076://Electric Spur (used by Coilskar Wrangler 22877 in BlackTemple)
               spellInfo->EffectBasePoints[1] = 2;
               break;
            /****************/
            case 40447: // BT: Akama - Soul Channel
                spellInfo->Effect[0] = 0;
                break;
            case 29538:
                spellInfo->EffectApplyAuraName[0] = 0;
                break;
            case 24311: // Powerful Healing Ward
                spellInfo->CastingTimeIndex = 14;
                break;
            case 24178: // Will of Hakkar
                spellInfo->AttributesEx |= SPELL_ATTR_EX_CHANNELED_1;
                break;
            case 28282: // Ashbringer
                spellInfo->Effect[2] = SPELL_EFFECT_APPLY_AURA;
                spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_CASTER;
                spellInfo->EffectApplyAuraName[2] = SPELL_AURA_FORCE_REACTION;
                spellInfo->EffectMiscValue[2] = 56; // Scarlet Crusade
                spellInfo->EffectBasePoints[2] = 4; // Friendly
                break;
            case 38297: // Leggins of BeastMastery
                spellInfo->Effect[0] = 0;
                spellInfo->EffectApplyAuraName[1] = SPELL_AURA_DUMMY;
                break;
            case 41350:
            case 41337:
                spellInfo->Attributes |= SPELL_ATTR_CANT_CANCEL;
                break;
            // do NOT remove encapsulate on druid shapeshift, attribute is added higher, so is safe to remove it here
            case 45665:
                spellInfo->AttributesCu &= ~SPELL_ATTR_CU_MOVEMENT_IMPAIR;
                break;
            case 45680: // Kil'jaeden orbs shadowbolts
                spellInfo->EffectImplicitTargetB[0] = TARGET_UNIT_AREA_ENEMY_DST;
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 39042: // Rampant Infection
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 40017: // If we can't adjust speed :P we spawn it in bigger periods
                spellInfo->EffectAmplitude[1] = 1900;
                break;
            case 40841:
                spellInfo->EffectRadiusIndex[0] = 15;
                break;
            case 41120:
                spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_TARGET_LEFT;
                break;
            case 41117:
                spellInfo->Effect[0] = 0;
                spellInfo->Effect[1] = 0;
                break;
            case 38054:
                spellInfo->MaxAffectedTargets = 10;
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_AREA_ENTRY_SRC;
                spellInfo->Targets = TARGET_FLAG_GAMEOBJECT & TARGET_FLAG_DEST_LOCATION;
                spellInfo->EffectMiscValue[0] = 0;
                break;
            case 33824:
                spellInfo->Effect[2] = 0;
                break;
            case 34121: // Al'ar Flame Buffet
                spellInfo->InterruptFlags &= ~SPELL_INTERRUPT_FLAG_MOVEMENT;
            case 26029: // dark glare
            case 43140: case 43215: // flame breath
                spellInfo->AttributesCu |= SPELL_ATTR_CU_CONE_LINE;
                break;
            case 24340: case 26558: case 28884:     // Meteor
            case 36837: case 38903: case 41276:     // Meteor
            case 26789:                             // Shard of the Fallen Star
            case 31436:                             // Malevolent Cleave
            case 35181:                             // Dive Bomb
            case 40810: case 43267: case 43268:     // Saber Lash
            case 42384:                             // Brutal Swipe
            case 45150:                             // Meteor Slash
                spellInfo->AttributesCu |= SPELL_ATTR_CU_SHARE_DAMAGE;
                switch (i) // Saber Lash Targets
                {
                    case 40810:
                        spellInfo->MaxAffectedTargets = 3;
                        break;
                    case 43267:
                    case 43268:
                        spellInfo->MaxAffectedTargets = 2;
                        break;
                    case 45150:
                        spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
                        break;
                }
                break;
            case 44978:
            case 45001:
            case 45002:     // Wild Magic
            case 45004:
            case 45006:
            case 45010:     // Wild Magic
            //case 31347: // Doom
            case 41635: // Prayer of Mending
            case 44869: // Spectral Blast
            case 45027: // Revitalize
            case 45976: // Muru Portal Channel
            case 39365: // Thundering Storm
            case 41071: // Raise Dead
            case 41172: // Rapid Shot
            case 40834: // Agonizing Flames
            case 45032: // Curse of Boundless Agony
            case 42357: // Axe Throw, triggered by 42359
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 45034:
                spellInfo->MaxAffectedTargets = 1;
                spellInfo->AttributesEx |= SPELL_ATTR_EX_CANT_TARGET_SELF;
                break;
            case 38281: // Static Charge (LV)
            case 39992: // Najentus: Needle Spine
            case 46019: // Teleport: Spectral Realm
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_PLAYERS_ONLY;
            case 41357: // L1 Acane Charge
            case 41376: // Spite
            case 29576: // Multi-Shot
            case 37790: // Spread Shot
            case 41303: // Soul Drain
            case 31298: // Anetheron: Sleep
            case 30004: // Aran: Flame Wreath
                spellInfo->MaxAffectedTargets = 3;
                break;
            case 38310: // Multi-Shot
                spellInfo->MaxAffectedTargets = 4;
                break;
            case 42005: // Bloodboil
            case 31347: // Doom
            case 39594: // Cyclone
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                spellInfo->EffectImplicitTargetB[0] = 0;
                if (i == 42005)
                    spellInfo->rangeIndex = 6;
                break;
            case 41625: // Fel Rage 3
                spellInfo->Stances = 0;
                break;
            case 45641: // Fire Bloom
                spellInfo->MaxAffectedTargets = 5;
                spellInfo->DmgClass = SPELL_DAMAGE_CLASS_MAGIC; // dispelable by cloak
                break;
            case 38296: // Spitfire Totem
            case 37676: // Insidious Whisper
            case 46008: // Negative Energy
            case 46771: // Flame Sear
                spellInfo->MaxAffectedTargets = 5;
                break;
            case 40827: // Sinful Beam
            case 40859: // Sinister Beam
            case 40860: // Vile Beam
            case 40861: // Wicked Beam
                spellInfo->MaxAffectedTargets = 10;
                break;
            case 5246:
            case 8122:
            case 8124:
            case 10888:
            case 10890: // Psychic Scream
            case 12494: // Frostbite
                spellInfo->Attributes |= SPELL_ATTR_BREAKABLE_BY_DAMAGE;
                break;
            case 32727: // Arena Preparation - remove invisibility aura
            case 44949: // Whirlwind's offhand attack - TODO: remove this (50% weapon damage effect)
                spellInfo->Effect[1] = 0;
                break;
            case 24905: // Moonkin form -> elune's touch
                spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_CASTER;
                break;
            case 27066: // Trueshot r4 - poprzednie ranki nie maja dispel type: MAGIC o.O
                spellInfo->Dispel = DISPEL_NONE;
                break;
            case 31117: // UA dispell effect
                spellInfo->SpellFamilyFlags = 0x010000000000LL;
                break;
            case 32045: // Archimonde: Soul Charge - yellow
            case 32051: // Archimonde: Soul Charge - green
            case 32052: // Archimonde: Soul Charge - red
                spellInfo->procCharges = 0;
                spellInfo->procChance = 101;
                spellInfo->procFlags = 0;
                break;
            case 40105:
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_CASTER;
                spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
                spellInfo->Effect[2] = 0;
            case 40106:
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                spellInfo->EffectTriggerSpell[0] = 0;
            case 41001: // Fatal Attraction Aura
                spellInfo->EffectTriggerSpell[1] = 0;
                break;
            case 40869: // Fatal Attraction
                spellInfo->EffectRadiusIndex[0] = 12;
                spellInfo->MaxAffectedTargets = 3;
                spellInfo->Effect[1] = 0;
                break;
            case 40870: // Fatal Attraction Trigger
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ALLY;
                spellInfo->EffectImplicitTargetB[0] = 0;
                break;
            case 40594: // Fel Rage
                spellInfo->EffectBasePoints[1] = 99;
                break;
            case 40855: // Akama Soul Expel
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                break;
            case 40401: // Shade of Akama Channeling
                spellInfo->Effect[2] = spellInfo->Effect[0];
                spellInfo->EffectApplyAuraName[2] = spellInfo->EffectApplyAuraName[0];
                spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_CASTER;
                break;
            case 40251:
                spellInfo->EffectApplyAuraName[1] = SPELL_AURA_DUMMY;
                break;
            case 36819: // Kael Pyroblast
                spellInfo->rangeIndex = 6;  // from 40yd to 100yd to avoid running from dmg
                break;
            case 40334:
                spellInfo->procFlags = PROC_FLAG_SUCCESSFUL_MELEE_HIT;
                break;
            case 13278: // Gnomish Death Ray
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                break;
            case 6947:  // Curse of the Bleakheart
                spellInfo->procFlags = 65876;      //any succesfull melee, ranged or negative spell hit
                break;
            case 34580: // Impale(Despair item)
                spellInfo->AttributesCu |= SPELL_ATTR_CU_IGNORE_ARMOR;
                break;
            case 66:    // Invisibility (fading) - break on casting spell
                spellInfo->AuraInterruptFlags |= AURA_INTERRUPT_FLAG_CAST;
                break;
            case 37363: // set 5y radius instead of 25y
                spellInfo->EffectRadiusIndex[0] = 8;
                spellInfo->EffectRadiusIndex[1] = 8;
                spellInfo->EffectMiscValue[1] = 50;
                break;
            case 42835: // set visual only
                spellInfo->Effect[0] = 0;
                break;
            case 46037:
            case 46040:
                spellInfo->EffectBasePoints[1] = 1;
            case 46038:
            case 46039:
            case 45661:
                spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
                break;
            case 21358: // Aqual Quintessence / Eternal Quintessence
            case 47977: // Broom Broom
            case 42679:
            case 42673:
            case 42680:
            case 42681:
            case 42683:
            case 42684:
                spellInfo->AttributesEx4 |= SPELL_ATTR_EX4_NOT_USABLE_IN_ARENA;
                break;
            case 43730: // Stormchops effect
                spellInfo->EffectTriggerSpell[0] = 43733;
                spellInfo->EffectTriggerSpell[1] = 43731;
                spellInfo->EffectImplicitTargetA[1] = 1;
                spellInfo->EffectImplicitTargetB[1] = 0;
                spellInfo->AttributesCu |= SPELL_ATTR_CU_TREAT_AS_WELL_FED;
                break;
            case 41470: //Judgement of Command should be reflectable
                spellInfo->AttributesEx2 = 0;
                break;
            case 26373: // Lunar Invitation
                spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
                spellInfo->EffectImplicitTargetB[1] = 0;
                break;
            case 31532: // Repair from Mekgineer event in Steamvault
            case 37936:
                spellInfo->Attributes &= ~SPELL_ATTR_BREAKABLE_BY_DAMAGE;
                break;
            case 37454: // Chess event: Bite
            case 37453: // Chess event: Smash
            case 37413: // Chess event: Visious Strike
            case 37406: // Chess event: Heroic Blow
                spellInfo->EffectRadiusIndex[0] = 15;    // effect radius from 8 to 3 yd
                break;
            case 37502: // Chess event: Howl
            case 37498: // Chess event: Stomp
                spellInfo->EffectRadiusIndex[0] = 29;    // effect radius from 8 to 6 yd
                break;
            case 37834: spellInfo->Attributes |= SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY; //Unbanish Azaloth
                break;
            case 37461: // Chess event: Shadow Spear
            case 37459: // Chess event: Holy Lance
                spellInfo->AttributesCu |= SPELL_ATTR_CU_CONE_LINE;
                spellInfo->EffectRadiusIndex[0] = 18;   // effect radius from 18 to 15 yd
                break;
            case 41363: // Shared Bonds
                spellInfo->AttributesEx &= ~SPELL_ATTR_EX_CHANNELED_1;
            case 16007: // DRACO_INCARCINATRIX_900
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                break;
            case 39331: // In Session
                spellInfo->DurationIndex = 21;  // infinity
                spellInfo->Effect[0] = SPELL_EFFECT_APPLY_AREA_AURA_FRIEND;
                spellInfo->EffectRadiusIndex[0] = 27;   // effect radius from 65 to 50 yd
                break;
            case 19937: //Illusion: Black Dragonkin
                spellInfo->AreaId = 15;
                break;
            case 28062: // Positive Charge
            case 28085: // Negative Charge
            case 39090: // Positive Charge
            case 39093: // Negative Charge
            case 39968: // Needle Spine Explosion
            case 39692: // Cannon
                spellInfo->AttributesEx |= SPELL_ATTR_EX_CANT_TARGET_SELF;
                break;
            case 42992: //ram - neutral
            case 43310: //ram - trot
                spellInfo->EffectImplicitTargetA[1] = 1;
                break;
            case 37370: // Kelidan the breaker - vortex
                spellInfo->EffectMiscValue[0] /= 3;
                break;
            case 41345: // Infatuation (BT Trash)
                spellInfo->AttributesEx2 &= ~SPELL_ATTR_EX2_IGNORE_LOS;
                break;
            case 43383: // Spirit Bolts (HexLord)
                spellInfo->ChannelInterruptFlags = 0;
                spellInfo->InterruptFlags &= ~SPELL_INTERRUPT_FLAG_INTERRUPT;
                break;
            case 29962: // Summon Elemental (Shade of Aran)
            case 37053:
            case 37051:
            case 37052:
                spellInfo->rangeIndex = 6;
                break;
            case 29951: // Blizzard (Shade of Aran)
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                spellInfo->EffectImplicitTargetB[0] = 0;
                spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_TARGET_ENEMY;
                spellInfo->EffectImplicitTargetB[1] = 0;
                break;
            case 36952: // (temporary) Ogre Building Bunny Curse Visual Large - penalty curse for SWP
            case 40214: // Dragonmaw illusion
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_DEATH_PERSISTENT;
                break;
            case 29955: // Arcane Missiles (Shade of Aran)
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_CASTER;
                spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
                spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_TARGET_ENEMY;
                break;
            case 29956: // arcane missiles shade of aran
                spellInfo->EffectBasePoints[0] = 2500;
                break;
            case 30541: // Magtheridon's Blaze
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                spellInfo->EffectImplicitTargetB[0] = 0;
                break;
            case 36449: // Magtheridon's Debris (30% hp)
                spellInfo->EffectImplicitTargetA[0] = TARGET_SRC_CASTER;
                spellInfo->EffectImplicitTargetB[0] = TARGET_UNIT_AREA_ENEMY_SRC;
                spellInfo->EffectImplicitTargetA[1] = TARGET_SRC_CASTER;
                spellInfo->EffectImplicitTargetB[1] = TARGET_UNIT_AREA_ENEMY_SRC;
                break;
            case 30631: // Magtheridon's Debris damage
                spellInfo->EffectImplicitTargetB[0] = TARGET_UNIT_AREA_ENEMY_DST;
                break;
            case 30207: // Magtheridon's creatures Shadow Grasp
                spellInfo->StackAmount = 5;
                break;
            case 20814: // Collect Dire Water
                spellInfo->InterruptFlags = SPELL_INTERRUPT_FLAG_MOVEMENT | SPELL_INTERRUPT_FLAG_DAMAGE | SPELL_INTERRUPT_FLAG_AUTOATTACK | SPELL_INTERRUPT_FLAG_PUSH_BACK | SPELL_INTERRUPT_FLAG_INTERRUPT;
                break;
            case 30346: // Fel Iron Shells duplicated effect, weird
            case 30254: // Curator Evocation
                spellInfo->Effect[1] = 0;   // remove self stun
                break;
            case 29200: // Purify Helboar Meat
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_CASTER;
                break;
            case 31790: // Righteous Defense taunt
                spellInfo->Attributes |= SPELL_ATTR_ABILITY;
                break;
            case 28509: // Greater Mana Regeneration - Elixir of Major Mageblood
            case 24363: // Mana Regeneration - Mageblood Potion
            case 31462: // Moonwell Restoration
            case 36746: // Shadowy Fortitude
            case 36749: // Arcane Might
            case 42965: // Tricky Treat
                spellInfo->Attributes = 0x28000000LL; // Remove when entering arena - Originally has flag 0x28000100LL but the attribute 0x100 makes them non-removable on arena.
                // Not known for sure, but logically these spells should be removed. How that was on blizz? Don't know for sure, but i think everyone would use that if so. (or maybe it wasn't used cause it's pretty expensive)
                // Anyway on free-realms it's abused - so remove it on arena.
                break;
            case 20271: // Paladins Judgement
                spellInfo->AttributesEx3 &= ~SPELL_ATTR_EX3_CANT_TRIGGER_PROC;
                break;
            case 30502: // Dark Spin
                spellInfo->Effect[0] = 0;
                spellInfo->Effect[1] = 0;
                spellInfo->Effect[2] = 0;
                break;
            case 38629: // Poison Keg
                spellInfo->RecoveryTime = 1080;
                break;
            case 25678:
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ANY;
                break;
            case 40129: // Summon Air Elemental (by quest  [The Soul Cannon of Reth'hedron], fix cast freeze - fix quest)
                spellInfo->AttributesEx &= ~SPELL_ATTR_EX_CHANNELED_1;
                break;
            case 6495: // Sentry totem
                spellInfo->EffectRadiusIndex[0] = 0;
                break;
            case 33666: ///sonic boom. the radius is less than 34 yards, because the tank and melee also can avoid it.
            case 38795:
                spellInfo->EffectRadiusIndex[0] = 9;
                spellInfo->EffectRadiusIndex[1] = 9;
                spellInfo->EffectRadiusIndex[2] = 9;
                break;
            case 33711: // Murmur's Touch
                spellInfo->MaxAffectedTargets = 1;
                spellInfo->EffectTriggerSpell[0] = 33760;
                break;
            case 38794:
                spellInfo->Effect[1] = 0;
                spellInfo->MaxAffectedTargets = 1;
                spellInfo->EffectTriggerSpell[0] = 33760;
                break;
            case 36717: //energy discharge
                spellInfo->MaxAffectedTargets = 1;
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_NEARBY_ENEMY;
                break;
            case 38829: // energy discharge hc
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 29838: //Second Wind (Rank 2)
                spellInfo->procFlags &= ~PROC_FLAG_ON_TAKE_PERIODIC;
                break;
            case 45342: // Conflagration Eredar Twins
                spellInfo->Mechanic = MECHANIC_CONFUSED;
                spellInfo->EffectMechanic[1] = MECHANIC_NONE;
                break;
            case 30834: // Infernal Relay
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ANY;
                break;
            // exploited quiver/ammo pouch spells for incredible ranged attack speed bug
            case 14824:
            case 14825:
            case 14826:
            case 14827:
            case 14828:
            case 14829:
            case 29413:
            case 29414:
            case 29415:
            case 29416:
            case 29417:
            case 29418:
            case 44972:
                spellInfo->speed = 0;
                break;
            default:
                break;
        }
    }

    for (uint32 i = 1; i < sSpellStore.GetNumRows(); ++i)
    {
        SpellEntry const * spell = sSpellStore.LookupEntry(i);
        if (spell && spell->Category)
            sSpellCategoryStore[spell->Category].insert(i);
    }
}

// TODO: move this to database along with slot position in cast bar
void SpellMgr::LoadCustomSpellCooldowns(SpellEntry* spellInfo)
{
    if (!spellInfo)
        return;

    switch (spellInfo->Id)
    {
        // 2 sec cooldown
        case 22907:     // Shoot
        case 35946:     // Shoot
        case 44533:     // Wretched Stab
            spellInfo->RecoveryTime = 2000;
            break;
        // 6 sec cooldown
        case 44639:     // Frost Arrow
        case 46082:     // Shadow Bolt Volley
            spellInfo->RecoveryTime = 6000;
            break;
        // 8 sec cooldown
        case 44534:     // Wretched Strike
        case 44640:     // Lash of Pain
            spellInfo->RecoveryTime = 8000;
            break;
        // 10 sec cooldown
        case 21885:     // heal vylestem vine
        case 44518:     // Immolate
        case 46042:     // Immolate
        case 44479:     // holy Light
        case 46029:     // Holy Light
            spellInfo->RecoveryTime = 10000;
            break;
        // 12 sec cooldown
        case 44547:     // Deadly Embrace
        case 44599:     // Inject Poison
        case 46046:     // Inject Poison
            spellInfo->RecoveryTime = 12000;
            break;
        // 15 sec cooldown
        case 44478:     // Glaive Throw
        case 46028:     // Glaive Throw
        case 20299:     // Forked Lightning
        case 46150:     // Forked Lightning
        case 17741:     // Mana Shield
        case 46151:     // Mana Shield
            spellInfo->RecoveryTime = 15000;
            break;
        // 20 sec cooldown
        case 44480:     // Seal of Wrath
        case 46030:     // Seal of Wrath
            spellInfo->RecoveryTime = 20000;
            break;
        // 24 sec cooldown
        case 44505:     // Drink Fel Infusion
            spellInfo->RecoveryTime = 24000;
            break;
        // 30 sec cooldown
        case 44475:     // magic dampening field
            spellInfo->RecoveryTime = 30000;
            break;
        // 5 min cooldown
        case 30015: // Summon Naias
        case 35413: // Summon Goliathon
            spellInfo->RecoveryTime = 300000;
            break;
        // 30 min cooldown
        case 44520:
            spellInfo->RecoveryTime = 1800000;
            break;
        // 3h cooldown
        case 16054: // Flames of the Black Flight
            spellInfo->RecoveryTime = 10800000;
            break;
        case 44935: //Expose Razorthorn Root
        case 29992: //Quest spell - needs cooldown to be able to add to possessed unit
            spellInfo->RecoveryTime = 1080;
            break;
        default:
            break;
    }
}

void SpellMgr::LoadSpellLinked()
{
    mSpellLinkedMap.clear();    // need for reload case
    uint32 count = 0;

    //                                                       0              1             2
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT spell_trigger, spell_effect, type FROM spell_linked_spell");
    if (!result)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outString();
        sLog.outString(">> Loaded %u linked spells", count);
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        int32 trigger = fields[0].GetInt32();
        int32 effect = fields[1].GetInt32();
        int32 type = fields[2].GetInt32();

        SpellEntry *strigger = (SpellEntry *)sSpellStore.LookupEntry(abs(trigger));
        if (!strigger)
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_linked_spell` does not exist", abs(trigger));
            continue;
        }
        SpellEntry *seffect = (SpellEntry *)sSpellStore.LookupEntry(abs(effect));
        if (!seffect)
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_linked_spell` does not exist", abs(effect));
            continue;
        }

        if (trigger > 0)
        {
            switch (type)
            {
                case 0: strigger->AttributesCu |= SPELL_ATTR_CU_LINK_CAST; break;
                case 1: strigger->AttributesCu |= SPELL_ATTR_CU_LINK_HIT;  break;
                case 2: strigger->AttributesCu |= SPELL_ATTR_CU_LINK_AURA; break;
                case 3: strigger->AttributesCu |= SPELL_ATRR_CU_LINK_PRECAST; break;
            }
        }
        else
        {
            strigger->AttributesCu |= SPELL_ATTR_CU_LINK_REMOVE;
        }

        if (type) //we will find a better way when more types are needed
        {
            if (trigger > 0)
                trigger += SPELL_LINKED_MAX_SPELLS * type;
            else
                trigger -= SPELL_LINKED_MAX_SPELLS * type;
        }
        mSpellLinkedMap[trigger].push_back(effect);

        ++count;
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u linked spells", count);
}

/// Some checks for spells, to prevent adding depricated/broken spells for trainers, spell book, etc
bool SpellMgr::IsSpellValid(SpellEntry const* spellInfo, Player* pl, bool msg)
{
    // not exist
    if (!spellInfo)
        return false;

    bool need_check_reagents = false;

    // check effects
    for (int i=0; i<3; ++i)
    {
        switch (spellInfo->Effect[i])
        {
            case 0:
                continue;

                // craft spell for crafting non-existed item (break client recipes list show)
            case SPELL_EFFECT_CREATE_ITEM:
            {
                if (!ObjectMgr::GetItemPrototype(spellInfo->EffectItemType[i]))
                {
                    if (msg)
                    {
                        if (pl)
                            ChatHandler(pl).PSendSysMessage("Craft spell %u create not-exist in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->EffectItemType[i]);
                        else
                            sLog.outLog(LOG_DB_ERR, "Craft spell %u create not-exist in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->EffectItemType[i]);
                    }
                    return false;
                }

                need_check_reagents = true;
                break;
            }
            case SPELL_EFFECT_LEARN_SPELL:
            {
                SpellEntry const* spellInfo2 = sSpellStore.LookupEntry(spellInfo->EffectTriggerSpell[i]);
                if (!IsSpellValid(spellInfo2,pl,msg))
                {
                    if (msg)
                    {
                        if (pl)
                            ChatHandler(pl).PSendSysMessage("Spell %u learn to broken spell %u, and then...",spellInfo->Id,spellInfo->EffectTriggerSpell[i]);
                        else
                            sLog.outLog(LOG_DB_ERR, "Spell %u learn to invalid spell %u, and then...",spellInfo->Id,spellInfo->EffectTriggerSpell[i]);
                    }
                    return false;
                }
                break;
            }
        }
    }

    if (need_check_reagents)
    {
        for (int j = 0; j < 8; ++j)
        {
            if (spellInfo->Reagent[j] > 0 && !ObjectMgr::GetItemPrototype(spellInfo->Reagent[j]))
            {
                if (msg)
                {
                    if (pl)
                        ChatHandler(pl).PSendSysMessage("Craft spell %u have not-exist reagent in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->Reagent[j]);
                    else
                        sLog.outLog(LOG_DB_ERR, "Craft spell %u have not-exist reagent in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->Reagent[j]);
                }
                return false;
            }
        }
    }

    return true;
}

bool SpellMgr::IsSpellAllowedInLocation(SpellEntry const *spellInfo,uint32 map_id,uint32 zone_id,uint32 area_id)
{
    // hack moved from Player::UpdateAreaDependentAuras <--- is still needed ? Oo i don't think so ...
    if (spellInfo->Id == 38157)
    {
        if (area_id == 3522 || area_id == 3785)
            return true;
    }

    if (spellInfo->Id == 1002 || spellInfo->Id == 126)
    {
        if (!(map_id == 0 || map_id == 1 || map_id == 530 ))
            return false;
    }

    // normal case
    if (spellInfo->AreaId && spellInfo->AreaId != zone_id && spellInfo->AreaId != area_id)
        return false;

    // elixirs (all area dependent elixirs have family SPELLFAMILY_POTION, use this for speedup)
    if (spellInfo->SpellFamilyName==SPELLFAMILY_POTION)
    {
        if (uint32 mask = sSpellMgr.GetSpellElixirMask(spellInfo->Id))
        {
            if (mask & ELIXIR_BATTLE_MASK)
            {
                if (spellInfo->Id==45373)                    // Bloodberry Elixir
                    return zone_id==4075;
            }
            if (mask & ELIXIR_UNSTABLE_MASK)
            {
                // in the Blade's Edge Mountains Plateaus and Gruul's Lair.
                return zone_id ==3522 || map_id==565;
            }
            if (mask & ELIXIR_SHATTRATH_MASK)
            {
                // in Tempest Keep, Serpentshrine Cavern, Caverns of Time: Mount Hyjal, Black Temple, Sunwell Plateau
                if (zone_id ==3607 || map_id==534 || map_id==564 || zone_id==4075)
                    return true;

                MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
                if (!mapEntry)
                    return false;

                return mapEntry->multimap_id==206;
            }

            // elixirs not have another limitations
            return true;
        }
    }

    // special cases zone check (maps checked by multimap common id) SPELL_CHECK_ARENA_AND_OR_ZONE
    switch (spellInfo->Id)
    {
        case 45403:
        case 45401:
        {
            if (map_id == 580 || map_id == 585)
                return true;

            if (map_id == 530 && zone_id == 4080)
                return true;

            return false;
        }
        case 48025: // Headless Horseman's Mount
        case 42684: // Swift Magic Broom
        case 42683:
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            if (!mapEntry->IsMountAllowed())
                return false;

            return true;
        }
        case 23333:                                         // Warsong Flag
        case 23335:                                         // Silverwing Flag
        case 46392:                                         // Focused Assault
        case 46393:                                         // Brutal Assault
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            if (!mapEntry->IsBattleGround())
                return false;

            if (zone_id == 3277)
                return true;

            return false;
        }
        case 34976:                                         // Netherstorm Flag
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            if (!mapEntry->IsBattleGround())
                return false;

            if (zone_id == 3820)
                return true;

            return false;
        }
        case 32724:                                         // Gold Team (Alliance)
        case 32725:                                         // Green Team (Alliance)
        case 32727:                                         // Arena Preparation
        case 35774:                                         // Gold Team (Horde)
        case 35775:                                         // Green Team (Horde)
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            //the follow code doesn't work.
            //if(!mapEntry->IsBattleArena())
            //    return false;

            //this is the working code, HACK
            if (zone_id == 3702 || zone_id == 3968 || zone_id == 3698)
                return true;

            return false;
        }
        case 41618:                                         // Bottled Nethergon Energy
        case 41620:                                         // Bottled Nethergon Vapor
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            return mapEntry->multimap_id==206;
        }
        case 41617:                                         // Cenarion Mana Salve
        case 41619:                                         // Cenarion Healing Salve
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            return mapEntry->multimap_id==207;
        }
        case 40216:                                         // Dragonmaw Illusion
        case 42016:                                         // Dragonmaw Illusion
            return area_id == 3759 || area_id == 3966 || area_id == 3939 || area_id == 3965 || area_id == 3967;
        case 2584:                                          // Waiting to Resurrect
        case 22011:                                         // Spirit Heal Channel
        case 22012:                                         // Spirit Heal
        case 24171:                                         // Resurrection Impact Visual
        case 42792:                                         // Recently Dropped Flag
        case 43681:                                         // Inactive
        case 44535:                                         // Spirit Heal (mana)
        case 44521:                                         // Preparation
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            if (!mapEntry->IsBattleGround())
                return false;
            break;
        }
        // karazhan books
        case 30562:
        case 30550:
        case 30567:
        case 30557:
            return map_id == 532;
        // Zul'Aman Amani Charms
        case 43818:
        case 43816:
        case 43822:
        case 43820:
            return map_id == 568;
        // Ritual of summoning
        case 698:
            if (map_id == 568) return false; // Zul'aman
            if (zone_id == 3523 || zone_id == 3845 || zone_id == 3847 || zone_id == 3848 || zone_id == 3849)
                return false; // netherstorm; shall be removed with SWP RELEASE

            if (map_id == 30 || map_id == 489 || map_id == 529 || map_id == 566)
                return false; // bgs

            return true;
        case 32096:
        case 32098:
        case 39911:
        case 39913:
        {
            if (zone_id == 3483)
                return true;

            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            return mapEntry->multimap_id == 199 || mapEntry->multimap_id == 208;
        }
        case 40817: // summon banishing portal
            return (area_id == 3785) || (area_id == 3784);
    }

    return true;
}

void SpellMgr::LoadSkillLineAbilityMap()
{
    mSkillLineAbilityMap.clear();

    uint32 count = 0;

    for (uint32 i = 0; i < sSkillLineAbilityStore.GetNumRows(); i++)
    {
        SkillLineAbilityEntry const *SkillInfo = sSkillLineAbilityStore.LookupEntry(i);
        if (!SkillInfo)
            continue;

        mSkillLineAbilityMap.insert(SkillLineAbilityMap::value_type(SkillInfo->spellId,SkillInfo));
        ++count;
    }

    sLog.outString();
    sLog.outString(">> Loaded %u SkillLineAbility MultiMap", count);
}

SpellEntry const* SpellMgr::GetHighestSpellRankForPlayer(uint32 spellId, Player* player)
{
    SpellEntry const* highest_rank = NULL;

    SpellChainNode const* node = sSpellMgr.GetSpellChainNode(sSpellMgr.GetLastSpellInChain(spellId));

    // should work for spells with multiple ranks
    if (node)
    {
        while (node)
        {
            if (player->HasSpell(node->cur))
                if (highest_rank = sSpellStore.LookupEntry(node->cur))
                    break;

            node = sSpellMgr.GetSpellChainNode(node->prev);
        }
    }
    // if spell don't have ranks check spell itself
    else if (player->HasSpell(spellId))
        highest_rank = sSpellStore.LookupEntry(spellId);

    return highest_rank;
}

float SpellMgr::GetSpellRadiusForHostile(SpellRadiusEntry const *radius)
{
    return (radius ? radius->radiusHostile : 0);
}

float SpellMgr::GetSpellRadiusForFriend(SpellRadiusEntry const *radius)
{
    return (radius ? radius->radiusFriend : 0);
}

float SpellMgr::GetSpellMaxRange( SpellEntry const *spellInfo )
{
    return SpellMgr::GetSpellMaxRange(sSpellRangeStore.LookupEntry(spellInfo->rangeIndex));
}

float SpellMgr::GetSpellMaxRange( uint32 id )
{
    SpellEntry const *spellInfo = GetSpellStore()->LookupEntry(id);
    if (!spellInfo) return 0;
    return GetSpellMaxRange(spellInfo);
}

float SpellMgr::GetSpellMaxRange( SpellRangeEntry const *range )
{
    return (range ? range->maxRange : 0);
}

float SpellMgr::GetSpellMinRange( SpellEntry const *spellInfo )
{
    return SpellMgr::GetSpellMinRange(sSpellRangeStore.LookupEntry(spellInfo->rangeIndex));
}

float SpellMgr::GetSpellMinRange( uint32 id )
{
    SpellEntry const *spellInfo = GetSpellStore()->LookupEntry(id);
    if (!spellInfo) return 0;
    return SpellMgr::GetSpellMinRange(spellInfo);
}

float SpellMgr::GetSpellMinRange( SpellRangeEntry const *range )
{
    return (range ? range->minRange : 0);
}

uint32 SpellMgr::GetSpellRangeType( SpellRangeEntry const *range )
{
    return (range ? range->type : 0);
}

uint32 SpellMgr::GetSpellRecoveryTime( SpellEntry const *spellInfo )
{
    return spellInfo->RecoveryTime > spellInfo->CategoryRecoveryTime ? spellInfo->RecoveryTime : spellInfo->CategoryRecoveryTime;
}

float SpellMgr::GetSpellRadius( SpellEntry const *spellInfo, uint32 effectIdx, bool positive )
{
    return positive
        ? SpellMgr::GetSpellRadiusForFriend(sSpellRadiusStore.LookupEntry(spellInfo->EffectRadiusIndex[effectIdx]))
        : SpellMgr::GetSpellRadiusForHostile(sSpellRadiusStore.LookupEntry(spellInfo->EffectRadiusIndex[effectIdx]));
}

bool SpellMgr::IsSealSpell( SpellEntry const *spellInfo )
{
    // Seal of Command should meet requirements, but mysteriously is not
    if (spellInfo->Id == 41469)
        return true;
    //Collection of all the seal family flags. No other paladin spell has any of those.
    return spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN &&
        (spellInfo->SpellFamilyFlags & 0x4000A000200LL);
}

bool SpellMgr::IsElementalShield( SpellEntry const *spellInfo )
{
    // family flags 10 (Lightning), 42 (Earth), 37 (Water), proc shield from T2 8 pieces bonus
    return (spellInfo->SpellFamilyFlags & 0x42000000400LL) || spellInfo->Id == 23552;
}

bool SpellMgr::IsPassiveSpellStackableWithRanks( SpellEntry const* spellProto )
{
    if (!SpellMgr::IsPassiveSpell(spellProto))
        return false;

    return !spellProto->HasEffect(SPELL_EFFECT_APPLY_AURA);
}

bool SpellMgr::IsDeathOnlySpell( SpellEntry const *spellInfo )
{
    return spellInfo->AttributesEx3 & SPELL_ATTR_EX3_CAST_ON_DEAD
        || spellInfo->Id == 2584;
}

bool SpellMgr::IsDeathPersistentSpell( SpellEntry const *spellInfo )
{
    if (!spellInfo)
        return false;

    return spellInfo->AttributesEx3 & SPELL_ATTR_EX3_DEATH_PERSISTENT;
}

bool SpellMgr::IsNonCombatSpell( SpellEntry const *spellInfo )
{
    return (spellInfo->Attributes & SPELL_ATTR_CANT_USED_IN_COMBAT) != 0;
}

bool SpellMgr::IsAreaOfEffectSpell( SpellEntry const *spellInfo )
{
    if (IsAreaEffectTarget[spellInfo->EffectImplicitTargetA[0]] || IsAreaEffectTarget[spellInfo->EffectImplicitTargetB[0]])
        return true;
    if (IsAreaEffectTarget[spellInfo->EffectImplicitTargetA[1]] || IsAreaEffectTarget[spellInfo->EffectImplicitTargetB[1]])
        return true;
    if (IsAreaEffectTarget[spellInfo->EffectImplicitTargetA[2]] || IsAreaEffectTarget[spellInfo->EffectImplicitTargetB[2]])
        return true;
    return false;
}

bool SpellMgr::IsAreaAuraEffect( uint32 effect )
{
    if (effect == SPELL_EFFECT_APPLY_AREA_AURA_PARTY    ||
        effect == SPELL_EFFECT_APPLY_AREA_AURA_FRIEND   ||
        effect == SPELL_EFFECT_APPLY_AREA_AURA_ENEMY    ||
        effect == SPELL_EFFECT_APPLY_AREA_AURA_PET      ||
        effect == SPELL_EFFECT_APPLY_AREA_AURA_OWNER)
        return true;

    return false;
}

bool SpellMgr::IsDispel( SpellEntry const *spellInfo )
{
    //spellsteal is also dispel
    if (spellInfo->Effect[0] == SPELL_EFFECT_DISPEL ||
        spellInfo->Effect[1] == SPELL_EFFECT_DISPEL ||
        spellInfo->Effect[2] == SPELL_EFFECT_DISPEL)
        return true;
    return false;
}

bool SpellMgr::IsDispelSpell( SpellEntry const *spellInfo )
{
    //spellsteal is also dispel
    if (spellInfo->Effect[0] == SPELL_EFFECT_STEAL_BENEFICIAL_BUFF ||
        spellInfo->Effect[1] == SPELL_EFFECT_STEAL_BENEFICIAL_BUFF ||
        spellInfo->Effect[2] == SPELL_EFFECT_STEAL_BENEFICIAL_BUFF
        || SpellMgr::IsDispel(spellInfo))
        return true;

    return false;
}

bool SpellMgr::isSpellBreakStealth( SpellEntry const* spellInfo )
{
    return spellInfo && !(spellInfo->AttributesEx & SPELL_ATTR_EX_NOT_BREAK_STEALTH);
}

bool SpellMgr::IsNotIgnoreTriggeredSpell(SpellEntry const* spellInfo)
{
    switch (spellInfo->Id)
    {
    case 19185: // frost trap
    case 27285: // seed of corruption
    case 43733: // stormchops
        return true;
    }
    return false;
}

bool SpellMgr::IsChanneledSpell( SpellEntry const* spellInfo )
{
    return spellInfo && spellInfo->AttributesEx & (SPELL_ATTR_EX_CHANNELED_1 | SPELL_ATTR_EX_CHANNELED_2);
}

bool SpellMgr::NeedsComboPoints( SpellEntry const* spellInfo )
{
    return spellInfo && spellInfo->AttributesEx & (SPELL_ATTR_EX_REQ_COMBO_POINTS1 | SPELL_ATTR_EX_REQ_COMBO_POINTS2);
}

SpellSchoolMask SpellMgr::GetSpellSchoolMask( SpellEntry const* spellInfo )
{
    return SpellSchoolMask(spellInfo->SchoolMask);
}

uint32 SpellMgr::GetSpellMechanicMask( SpellEntry const* spellInfo)
{
    uint32 mask = 0;
    if (spellInfo->Mechanic)
        mask |= 1<<spellInfo->Mechanic;
    return mask;
}

uint32 SpellMgr::GetEffectMechanicMask( SpellEntry const* spellInfo, int32 effect )
{
    uint32 mask = 0;
    if (spellInfo->EffectMechanic[effect])
        mask |= 1<<spellInfo->EffectMechanic[effect];
    return mask;
}

Mechanics SpellMgr::GetSpellMechanic( SpellEntry const* spellInfo)
{
    if (spellInfo->Mechanic)
        return Mechanics(spellInfo->Mechanic);
    return MECHANIC_NONE;
}

Mechanics SpellMgr::GetEffectMechanic( SpellEntry const* spellInfo, int32 effect )
{
    if (spellInfo->EffectMechanic[effect])
        return Mechanics(spellInfo->EffectMechanic[effect]);
    return MECHANIC_NONE;
}

uint32 SpellMgr::GetDispellMask( DispelType dispel )
{
    // If dispel all
    if (dispel == DISPEL_ALL)
        return DISPEL_ALL_MASK;
    else
        return (1 << dispel);
}

bool SpellMgr::IsPrimaryProfessionSkill( uint32 skill )
{
    SkillLineEntry const *pSkill = sSkillLineStore.LookupEntry(skill);
    if (!pSkill)
        return false;

    if (pSkill->categoryId != SKILL_CATEGORY_PROFESSION)
        return false;

    return true;
}

bool SpellMgr::IsProfessionSkill( uint32 skill )
{
    return SpellMgr::IsPrimaryProfessionSkill(skill) || skill == SKILL_FISHING || skill == SKILL_COOKING || skill == SKILL_FIRST_AID;
}

DiminishingGroup SpellMgr::GetDiminishingReturnsGroupForSpell(SpellEntry const* spellproto, bool triggered)
{
    if (!spellproto)
        return DIMINISHING_NONE;

    // Explicit Diminishing Groups
    switch (spellproto->SpellFamilyName)
    {
        case SPELLFAMILY_MAGE:
        {
            // Polymorph
            if ((spellproto->SpellFamilyFlags & 0x00001000000LL) && spellproto->EffectApplyAuraName[0]==SPELL_AURA_MOD_CONFUSE)
                return DIMINISHING_INCAPACITATE;
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            // Kidney Shot
            if (spellproto->SpellFamilyFlags & 0x00000200000LL)
                return DIMINISHING_KIDNEYSHOT;
            // Sap
            else if (spellproto->SpellFamilyFlags & 0x00000000080LL)
                return DIMINISHING_INCAPACITATE;
            // Gouge
            else if (spellproto->SpellFamilyFlags & 0x00000000008LL)
                return DIMINISHING_INCAPACITATE;
            // Blind
            else if (spellproto->SpellFamilyFlags & 0x00001000000LL)
                return DIMINISHING_BLIND_CYCLONE;
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // Freezing trap
            if (spellproto->SpellFamilyFlags & 0x00000000008LL)
                return DIMINISHING_FREEZE;
            // Intimidation
            else if (spellproto->Id == 24394)
                return DIMINISHING_CONTROL_STUN;
            // scatter shot
            else if (spellproto->Id == 19503)
                return DIMINISHING_BLIND_CYCLONE;
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // Death Coil
            if (spellproto->SpellFamilyFlags & 0x00000080000LL)
                return DIMINISHING_DEATHCOIL;
            // Seduction
            else if (spellproto->Id == 6358)
                return DIMINISHING_FEAR;
            // Fear
            //else if (spellproto->SpellFamilyFlags & 0x40840000000LL)
            //    return DIMINISHING_WARLOCK_FEAR;
            // Curse of Tongues
            else if ((spellproto->SpellFamilyFlags & 0x00080000000LL) && (spellproto->SpellIconID == 692))
                return DIMINISHING_LIMITONLY;
            // Unstable Affliction dispel silence
            else if (spellproto->Id == 31117)
                return DIMINISHING_UNSTABLE_AFFLICTION;
            // Enslave deamon
            else if(spellproto->SpellFamilyFlags & 0x800LL)
                return DIMINISHING_ENSLAVE;
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            // Cyclone
            if (spellproto->SpellFamilyFlags & 0x02000000000LL)
                return DIMINISHING_BLIND_CYCLONE;
            // Nature's Grasp trigger
            if (spellproto->SpellFamilyFlags & 0x00000000200LL && spellproto->Attributes == 0x49010000)
                return DIMINISHING_CONTROL_ROOT;
            // feral charge effect should not be in any dr
            if (spellproto->Id == 45334)
                return DIMINISHING_NONE;
            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            // Hamstring - limit duration to 10s in PvP
            if (spellproto->SpellFamilyFlags & 0x00000000002LL)
                return DIMINISHING_LIMITONLY;
            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            // Turn Evil - share group with fear, seduction
            if (spellproto->Id == 10326)
                return DIMINISHING_FEAR;
            break;
        }
        case SPELLFAMILY_POTION:
            return DIMINISHING_NONE;
        default:
        {
            if (spellproto->Id == 12494) // frostbite
                return DIMINISHING_TRIGGER_ROOT;
            break;
        }
    }

    Mechanics mech = SpellMgr::GetSpellMechanic(spellproto);
    if (mech == MECHANIC_NONE) // try to find it in effects
    {
    // Get by mechanic
        for (uint8 i=0;i<3;++i)
        {
            if (mech = SpellMgr::GetEffectMechanic(spellproto, i))
                break; // found something
        }
    }

    switch (mech)
    {
        case MECHANIC_STUN:
            return triggered ? DIMINISHING_TRIGGER_STUN : DIMINISHING_CONTROL_STUN;
        case MECHANIC_SLEEP:
            return DIMINISHING_SLEEP;
        case MECHANIC_ROOT:
            return triggered ? DIMINISHING_TRIGGER_ROOT : DIMINISHING_CONTROL_ROOT;
        case MECHANIC_FEAR:
            return DIMINISHING_FEAR;
        case MECHANIC_CHARM:
            return DIMINISHING_CHARM;
        case MECHANIC_DISARM:
            return DIMINISHING_DISARM;
        case MECHANIC_FREEZE:
            return DIMINISHING_FREEZE;
        case MECHANIC_KNOCKOUT:
        case MECHANIC_SAPPED:
            return DIMINISHING_INCAPACITATE;
        case MECHANIC_BANISH:
            return DIMINISHING_BANISH;
    }

    return DIMINISHING_NONE;
}

bool SpellMgr::IsDiminishingReturnsGroupDurationLimited(DiminishingGroup group)
{
    switch (group)
    {
        case DIMINISHING_CONTROL_STUN:
        case DIMINISHING_TRIGGER_STUN:
        case DIMINISHING_KIDNEYSHOT:
        case DIMINISHING_SLEEP:
        case DIMINISHING_CONTROL_ROOT:
        case DIMINISHING_TRIGGER_ROOT:
        case DIMINISHING_FEAR:
        case DIMINISHING_WARLOCK_FEAR:
        case DIMINISHING_CHARM:
        case DIMINISHING_INCAPACITATE:
        case DIMINISHING_FREEZE:
        case DIMINISHING_BLIND_CYCLONE:
        case DIMINISHING_BANISH:
        case DIMINISHING_LIMITONLY:
            return true;
    }
    return false;
}

DiminishingReturnsType SpellMgr::GetDiminishingReturnsGroupType(DiminishingGroup group)
{
    switch (group)
    {
        case DIMINISHING_BLIND_CYCLONE:
        case DIMINISHING_CONTROL_STUN:
        case DIMINISHING_TRIGGER_STUN:
        case DIMINISHING_KIDNEYSHOT:
            return DRTYPE_ALL;
        case DIMINISHING_SLEEP:
        case DIMINISHING_CONTROL_ROOT:
        case DIMINISHING_TRIGGER_ROOT:
        case DIMINISHING_FEAR:
        case DIMINISHING_CHARM:
        case DIMINISHING_INCAPACITATE:
        case DIMINISHING_UNSTABLE_AFFLICTION:
        case DIMINISHING_DISARM:
        case DIMINISHING_DEATHCOIL:
        case DIMINISHING_FREEZE:
        case DIMINISHING_BANISH:
        case DIMINISHING_WARLOCK_FEAR:
            return DRTYPE_PLAYER;
    }

    return DRTYPE_NONE;
}

bool SpellMgr::SpellIgnoreLOS(SpellEntry const* spellproto, uint8 effIdx)
{
    if (spellproto->AttributesEx2 & SPELL_ATTR_EX2_IGNORE_LOS)
        return true;

    if (SpellMgr::IsSplashBuffAura(spellproto))
        return true;

    // Most QuestItems should ommit los ;]
    if (spellproto->Effect[effIdx] == SPELL_EFFECT_DUMMY && spellproto->NeedFillTargetMapForTargets(effIdx))
        return true;

    return false;
}

bool SpellMgr::EffectCanScaleWithLevel(const SpellEntry * spellInfo, uint8 eff)
{
    if (!(spellInfo->Attributes & SPELL_ATTR_LEVEL_DAMAGE_CALCULATION) || !spellInfo->spellLevel)
        return false;

    if (spellInfo->Effect[eff] == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE ||
        spellInfo->Effect[eff] == SPELL_EFFECT_KNOCK_BACK ||
        spellInfo->Effect[eff] == SPELL_EFFECT_ADD_EXTRA_ATTACKS)
        return false;

    if (spellInfo->EffectApplyAuraName[eff] == SPELL_AURA_MOD_SPEED_ALWAYS ||
        spellInfo->EffectApplyAuraName[eff] == SPELL_AURA_MOD_SPEED_NOT_STACK ||
        spellInfo->EffectApplyAuraName[eff] == SPELL_AURA_MOD_INCREASE_SPEED ||
        spellInfo->EffectApplyAuraName[eff] == SPELL_AURA_MOD_DECREASE_SPEED ||
        spellInfo->EffectApplyAuraName[eff] == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE ||
        spellInfo->EffectApplyAuraName[eff] == SPELL_AURA_HASTE_MELEE ||
        spellInfo->EffectApplyAuraName[eff] == SPELL_AURA_MOD_HASTE)
        return false;

    //there are many more than above: slow speed, -healing pct

    return true;
}

bool SpellMgr::CanSpellCrit(const SpellEntry* spellInfo)
{
    if (spellInfo->AttributesEx2 & SPELL_ATTR_EX2_CANT_CRIT)
        return false;

    switch (spellInfo->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_NONE:
            switch (spellInfo->Id)// We need more spells to find a general way (if there is any)
            {
                case 379:   // Earth Shield
                case 33778: // Lifebloom
                case 45064: // Vessel of the Naaru
                    break;
                default:
                    return false;
            } // fall through to break
        case SPELL_DAMAGE_CLASS_MAGIC:
        case SPELL_DAMAGE_CLASS_MELEE:
        case SPELL_DAMAGE_CLASS_RANGED:
            break;
        default:
            return false;
    }

    for (uint8 eff=0; eff<3; ++eff)
        switch(spellInfo->Effect[eff])
        {
            case SPELL_EFFECT_HEAL:
            case SPELL_EFFECT_HEALTH_LEECH:
            case SPELL_EFFECT_HEAL_MECHANICAL:
            case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
            case SPELL_EFFECT_SCHOOL_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                return true;
        }

    return false;
}

uint64 SpellMgr::GetSpellAffectMask(uint16 spellId, uint8 effectId) const
{
    SpellAffectMap::const_iterator itr = mSpellAffectMap.find((spellId<<8) + effectId);
    if (itr != mSpellAffectMap.end())
        return itr->second;
    return 0;
}

bool SpellMgr::IsPositionTarget(uint32 target)
{
    switch (SpellTargetType[target])
    {
    case TARGET_TYPE_DEST_CASTER:
    case TARGET_TYPE_DEST_TARGET:
    case TARGET_TYPE_DEST_DEST:
        return true;
    default:
        break;
    }
    return false;
}

bool SpellMgr::IsTauntSpell(SpellEntry const* spellInfo)
{
    if (!spellInfo)
        return false;

    for (uint8 i = 0; i < 3; ++i)
    {
        if (spellInfo->Effect[i] == SPELL_EFFECT_ATTACK_ME)
            return true;
        else if (spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA && spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOD_TAUNT)
            return true;
    }
    return false;
}

bool SpellMgr::IsAuraCountdownContinueOffline(uint32 spellID, uint32 effIndex)
{
    if (IsPositiveEffect(spellID, effIndex))
        return false;

    if (spellID == 26013 && sWorld.getConfig(CONFIG_BATTLEGROUND_DESERTER_REALTIME))
        return false;

    return true;
}
