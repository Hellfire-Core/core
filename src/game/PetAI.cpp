/*
 * Copyright (C) 2005-2008 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2008-2017 Hellground <http://wow-hellground.com/>
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

#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "PathFinder.h"
#include "PetAI.h"
#include "Log.h"
#include "Pet.h"
#include "Player.h"
#include "DBCStores.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "ObjectAccessor.h"
#include "SpellMgr.h"
#include "Creature.h"
#include "Util.h"


int PetAI::Permissible(const Creature *creature)
{
    if (creature->isPet())
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

void PetAI::ForcedAttackStart(Unit* target)
{
    forced_attack = true;
    if (me->Attack(target, true) && !me->HasUnitState(UNIT_STAT_LOST_CONTROL))
        me->GetMotionMaster()->MoveChase(target);
}

void PetAI::AttackStart(Unit* target)
{
    forced_attack = false; // on change not by owners order we stop forced attacks
    if (me->Attack(target, true) && !me->HasUnitState(UNIT_STAT_LOST_CONTROL))
        me->GetMotionMaster()->MoveChase(target);
}

PetAI::PetAI(Creature *c) : CreatureAI(c), i_tracker(TIME_INTERVAL_LOOK), forced_attack(false) 
{
    m_AllySet.clear();
    m_owner = me->GetCharmerOrOwner();

    UpdateAllies();
}

void PetAI::EnterEvadeMode()
{
}

bool PetAI::targetHasInterruptableAura(Unit *target) const
{
    if (!target)
        return false;


    Unit::AuraMap const &auramap = target->GetAuras();
    for (Unit::AuraMap::const_iterator itr = auramap.begin(); itr != auramap.end(); ++itr)
    {
        if (itr->second && !sSpellMgr.IsPositiveSpell(itr->second->GetId()) && itr->second->GetSpellProto()->AuraInterruptFlags & (AURA_INTERRUPT_FLAG_DIRECT_DAMAGE | AURA_INTERRUPT_FLAG_HITBYSPELL | AURA_INTERRUPT_FLAG_DAMAGE))
        {
            //seduction should return false, this aura can be interrupted but not by us, we are to busy channeling, so dont stop combat
            if (sSpellMgr.IsChanneledSpell(itr->second->GetSpellProto()) && itr->second->GetCasterGUID() == m_creature->GetGUID())
                return false;
            return true;
        }
    }
    return false;
}

void PetAI::ownerOrMeAttackedBy(uint64 enemy)
{
    if (enemy == m_creature->GetGUID() || enemy == m_creature->GetCharmerOrOwnerGUID())
        return;
    if (me->HasReactState(REACT_DEFENSIVE))
        m_EnemySet.insert(enemy);
}

bool PetAI::_needToStop()
{
    // This is needed for charmed creatures, as once their target was reset other effects can trigger threat
    if (me->isCharmed() && me->GetVictim() == me->GetCharmer())
        return true;

    // also pet should stop attacking if his target of his owner is in sanctuary (applies only to player and player-pets targets)
    if (me->GetOwner() && me->GetOwner()->isInSanctuary() && me->GetVictim()->GetCharmerOrOwnerPlayerOrPlayerItself())
        return true;

    // also should stop if cannot attack
    if (!me->canAttack(me->GetVictim()))
        return true;

    if (targetHasInterruptableAura(me->GetVictim()))
        return !forced_attack; // if owner explicitly told us to attack him then we do

    forced_attack = false; // if target even for a moment does not have interruptable aura stop forced state
    return false;
}

void PetAI::_stopAttack()
{
    if (!me->IsAlive())
    {
        DEBUG_LOG("Creature stoped attacking cuz his dead [guid=%u]", me->GetGUIDLow());

        me->GetMotionMaster()->StopControlledMovement();
        me->CombatStop();
        me->getHostileRefManager().deleteReferences();

        return;
    }

    UpdateMotionMaster();

    me->CombatStop();
}

void PetAI::UpdateMotionMaster()
{
    if (m_owner && me->GetCharmInfo() && me->GetCharmInfo()->HasCommandState(COMMAND_FOLLOW))
        me->GetMotionMaster()->MoveFollow(m_owner,PET_FOLLOW_DIST,PET_FOLLOW_ANGLE);
    else
        me->GetMotionMaster()->StopControlledMovement();
}

void PetAI::PrepareSpellForAutocast(uint32 spellID)
{
    if (!spellID)
        return;

    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellID);
    if (!spellInfo)
        return;

    bool inCombat = me->GetVictim();

    // ignore some combinations of combat state and combat/noncombat spells
    if (!inCombat)
    {
        if (!SpellMgr::IsPositiveSpell(spellInfo->Id))
            return;
    }
    else
    {
        if (SpellMgr::IsNonCombatSpell(spellInfo))
            return;
    }
    /*
    if (m_owner && m_owner->GetTypeId() == TYPEID_PLAYER)
    if(((Player*)m_owner)->HasSpellCooldown(spellID))
    return;
    */
    Spell *spell = new Spell(me, spellInfo, false, 0);

    if (inCombat && !me->HasUnitState(UNIT_STAT_FOLLOW) && spell->CanAutoCast(me->GetVictim()))
    {
        m_targetSpellStore.push_back(std::make_pair/*<Unit*, Spell*>*/(me->GetVictim(), spell));
        return;
    }
    else
    {

        bool spellUsed = false;
        for (std::set<uint64>::iterator tar = m_AllySet.begin(); tar != m_AllySet.end(); ++tar)
        {
            Unit* Target = me->GetMap()->GetUnit(*tar);

            //only buff targets that are in combat, unless the spell can only be cast while out of combat
            if (!Target)
                continue;

            if (spell->CanAutoCast(Target))
            {
                m_targetSpellStore.push_back(std::make_pair/*<Unit*, Spell*>*/(Target, spell));
                spellUsed = true;
                break;
            }
        }
        if (!spellUsed)
            delete spell;
    }
}

void PetAI::AddSpellForAutocast(uint32 spellID, Unit* target)
{
    if (!spellID)
        return;

    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellID);
    if (!spellInfo)
        return;

    Spell *spell = new Spell(me, spellInfo, false, 0);
    if (spell->CanAutoCast(target))
        m_targetSpellStore.push_back(std::make_pair/*<Unit*, Spell*>*/(target, spell));
    else
        delete spell;
}

void PetAI::AutocastPreparedSpells()
{
    //found units to cast on to
    if (!m_targetSpellStore.empty())
    {
        uint32 index = urand(0, m_targetSpellStore.size() - 1);

        Spell* spell  = m_targetSpellStore[index].second;
        Unit*  target = m_targetSpellStore[index].first;

        m_targetSpellStore.erase(m_targetSpellStore.begin() + index);

        SpellCastTargets targets;
        targets.setUnitTarget(target);

        if (!me->HasInArc(M_PI, target))
        {
            me->SetInFront(target);
            if (target->GetTypeId() == TYPEID_PLAYER)
                me->SendCreateUpdateToPlayer((Player*)target);

            if (m_owner && m_owner->GetTypeId() == TYPEID_PLAYER)
                me->SendCreateUpdateToPlayer((Player*)m_owner);
        }

        if (me->isPet())
            ((Pet*)me)->CheckLearning(spell->GetSpellEntry()->Id);

        spell->prepare(&targets);
    }

    while (!m_targetSpellStore.empty())
    {
        Spell *temp = m_targetSpellStore.begin()->second;
        m_targetSpellStore.erase(m_targetSpellStore.begin());
        delete temp;
    }
}

void PetAI::MovementInform(uint32 type, uint32 data)
{
    if (type != CHASE_MOTION_TYPE || data != 2) // target reached only
        return;

    if (Unit *target = me->GetVictim())
        if (target->GetVictim() && target->GetVictim() != me && target->isInFront(me, 7.0f, M_PI))
        {
            float x, y, z;
            target->GetGroundPointAroundUnit(x, y, z, target->GetObjectSize(), M_PI);

            if (fabs(z - me->GetPositionZ()) <= NOMINAL_MELEE_RANGE) // height difference check
                me->GetMotionMaster()->MovePoint(0, x, y, z);
        }
}

void PetAI::UpdateAI(const uint32 diff)
{
    m_owner = me->GetCharmerOrOwner();

    // quest support - Razorthorn Ravager, switch to CreatureAI when charmed and not in combat
    if (me->GetEntry() == 24922 && me->isCharmed() && !me->IsInCombat())
        me->NeedChangeAI = true;

    if (updateAlliesTimer.Expired(diff))
        UpdateAllies();

    if (me->GetVictim())
    {
        if (_needToStop())
        {
            _stopAttack();
            return;
        }

        DoMeleeAttackIfReady();
    }
    else
    {
        if (m_owner)
        {
            TargetSelectHelper();

            // we still do NOT have target, if follow command were appliend and we are NOT followin or casting, reapply movegen :P
            if (!me->GetVictim() && me->GetCharmInfo()->HasCommandState(COMMAND_FOLLOW) &&
                !me->HasUnitState(UNIT_STAT_FOLLOW | UNIT_STAT_CASTING))
                me->GetMotionMaster()->MoveFollow(m_owner, PET_FOLLOW_DIST,PET_FOLLOW_ANGLE);
        }
    }

    if (!me->GetCharmInfo())
        return;

    if (!me->HasUnitState(UNIT_STAT_CASTING))
    {
        //Autocast
        for (uint8 i = 0; i < me->GetPetAutoSpellSize(); i++)
            PrepareSpellForAutocast(me->GetPetAutoSpellOnPos(i));

        AutocastPreparedSpells();
    }
}

void PetAI::UpdateAllies()
{
    Group *pGroup = NULL;

    updateAlliesTimer.Reset(5000);                            //update friendly targets every 5 seconds, lesser checks increase performance

    if (m_creature->GetReactState() != REACT_DEFENSIVE)
        m_EnemySet.clear(); // clear if changed

    if (!m_owner)
        return;
    else if (m_owner->GetTypeId() == TYPEID_PLAYER)
        pGroup = ((Player*)m_owner)->GetGroup();

    //only pet and owner/not in group->ok
    if (m_AllySet.size() == 2 && !pGroup)
        return;
    //owner is in group; group members filled in already (no raid -> subgroupcount = whole count)
    if (pGroup && !pGroup->isRaidGroup() && m_AllySet.size() == (pGroup->GetMembersCount() + 2))
        return;

    m_AllySet.clear();
    m_AllySet.insert(me->GetGUID());
    if (pGroup)                                              //add group
    {
        for (GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player* Target = itr->getSource();
            if (!Target || !pGroup->SameSubGroup((Player*)m_owner, Target))
                continue;

            m_AllySet.insert(Target->GetGUID());
        }
    }
    else                                                    //remove group
        m_AllySet.insert(m_owner->GetGUID());
}

void PetAI::TargetSelectHelper()
{
    if (me->HasReactState(REACT_PASSIVE) || me->GetCharmInfo()->HasCommandState(COMMAND_STAY))
        return;
    Unit* target = NULL;
    
    if (me->HasReactState(REACT_AGGRESSIVE))
    {
        if (m_owner->GetVictim() && !targetHasInterruptableAura(m_owner->GetVictim()))
            target = m_owner->GetVictim();
        else if (!m_owner->GetAttackers().empty() && !targetHasInterruptableAura(m_owner->GetAttackerForHelper()))
            target = m_owner->GetAttackerForHelper();
        else if (!me->GetAttackers().empty() && !targetHasInterruptableAura(me->GetAttackerForHelper()))
            target = me->GetAttackerForHelper();
        else
            target = FindValidTarget();
    }
    else if (me->HasReactState(REACT_DEFENSIVE))
    {
        for (std::set<uint64>::iterator itr = m_EnemySet.begin(); itr != m_EnemySet.end();)
        {
            Unit* possibletarget = m_creature->GetUnit(*itr);
            if (!possibletarget || !m_creature->IsInRange(possibletarget,0,50) || !possibletarget->IsInCombat()
                /*(m_owner->GetAttackers().find(possibletarget) == m_owner->GetAttackers().end() &&
                m_creature->GetAttackers().find(possibletarget) == m_creature->GetAttackers().end())*/)
            { // remove if not found, too far away or already not in combat
                itr = m_EnemySet.erase(itr);
                continue;
            }
            if (!targetHasInterruptableAura(possibletarget))
                target = possibletarget; // do not break, clear everything
            itr++;
        }
    }
    if (target)
        AttackStart(target);
}

Unit* PetAI::FindValidTarget()
{
    std::list<Unit *> targets;
    Hellground::AnyUnfriendlyUnitInPetAttackRangeCheck u_check(m_creature);
    Hellground::UnitListSearcher<Hellground::AnyUnfriendlyUnitInPetAttackRangeCheck> searcher(targets, u_check);
    Cell::VisitAllObjects(me, searcher, 30);

    // remove not LoS targets and interruptable
    for (std::list<Unit *>::iterator tIter = targets.begin(); tIter != targets.end();)
    {
        if (!m_creature->IsWithinLOSInMap(*tIter) || (*tIter)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE) ||
            targetHasInterruptableAura(*tIter) ||
            (*tIter)->GetTypeId() == TYPEID_UNIT && ((Creature*)(*tIter))->isTrigger() )
        {
            std::list<Unit *>::iterator tIter2 = tIter;
            ++tIter;    
            targets.erase(tIter2);
        }
        else
            ++tIter;
    }

    if (targets.empty())
        return NULL;

    uint32 rIdx = urand(0, targets.size() - 1);
    std::list<Unit *>::const_iterator tcIter = targets.begin();
    for (uint32 i = 0; i < rIdx; ++i)
        ++tcIter;

    return *tcIter;
}

int ImpAI::Permissible(const Creature *creature)
{
    if (creature->isPet())
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

void ImpAI::AttackStart(Unit *victim)
{
    if (!victim)
        return;

    if (me->Attack(victim, true))
    {
        me->GetMotionMaster()->MoveChase(victim, 0, 0);
        m_chasing = true;
    }
}

void ImpAI::UpdateAI(const uint32 diff)
{
    if (!me->IsAlive())
        return;

    m_owner = me->GetCharmerOrOwner();

    if (updateAlliesTimer.Expired(diff))
        UpdateAllies();

    // me->GetVictim() can't be used for check in case stop fighting, me->GetVictim() clear at Unit death etc.
    if (Unit *target = me->GetVictim())
    {
        if (_needToStop())
        {
            DEBUG_LOG("Pet AI stoped attacking [guid=%u]", me->GetGUIDLow());
            _stopAttack();
            return;
        }
        float dist = me->GetDistance(target);
        if (dist <= 30 && !me->IsWithinLOSInMap(target))
            dist = 50;
        if (dist < 30 && m_chasing)
        {
            me->ClearUnitState(UNIT_STAT_FOLLOW);
            me->GetMotionMaster()->StopControlledMovement();
            m_chasing = false;
        }
        if (dist > 30 && !m_chasing)
        {
            me->GetMotionMaster()->MoveChase(target);
            m_chasing = true;
        }
    }
    else
    {
        if (me->IsInCombat())
        {
            if (!m_owner|| !m_owner->GetObjectGuid().IsPlayer())
                _stopAttack();
        }
        else if (m_owner && me->GetCharmInfo()) //no victim
        {
            if (m_owner->IsInCombat() && !(me->HasReactState(REACT_PASSIVE) || me->GetCharmInfo()->HasCommandState(COMMAND_STAY)))
                AttackStart(m_owner->GetAttackerForHelper());
            else if (me->GetCharmInfo()->HasCommandState(COMMAND_FOLLOW) && !me->HasUnitState(UNIT_STAT_FOLLOW))
                me->GetMotionMaster()->MoveFollow(m_owner,PET_FOLLOW_DIST,PET_FOLLOW_ANGLE);
        }
    }

    if (!me->GetCharmInfo())
        return;

    if (!me->HasUnitState(UNIT_STAT_CASTING))
    {
        //Autocast
        for (uint8 i = 0; i < me->GetPetAutoSpellSize(); i++)
            PrepareSpellForAutocast(me->GetPetAutoSpellOnPos(i));

        AutocastPreparedSpells();
    }

    /*
    if (!me->HasUnitState(UNIT_STAT_CASTING))
        DoMeleeAttackIfReady();*/ // Imps should not have melee attacks. Leaving it here for the note
}

int FelhunterAI::Permissible(const Creature *creature)
{
    if (creature->isPet())
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

void FelhunterAI::PrepareSpellForAutocast(uint32 spellID)
{
    if (!spellID)
        return;

    if (sSpellMgr.GetFirstSpellInChain(spellID) == 19505) // Devour Magic
    {
        SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellID);
        Unit *target = me->GetVictim();
        if (!spellInfo || !target)
            return;
        Unit::AuraMap const& auras = target->GetAuras();
        for (Unit::AuraMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
        {
            Aura *aur = (*itr).second;
            if (aur && aur->GetSpellProto()->Dispel == DISPEL_MAGIC)
            {
                if (aur->IsPositive())
                {
                    AddSpellForAutocast(spellID, target);
                    return;
                }
            }
        }
    }
    else
        PetAI::PrepareSpellForAutocast(spellID);
}

int WaterElementalAI::Permissible(const Creature *creature)
{
    if (creature->isPet())
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

void WaterElementalAI::UpdateAI(const uint32 diff)
{
    if (!me->IsAlive())
        return;

    m_owner = me->GetCharmerOrOwner();

    if (updateAlliesTimer.Expired(diff))
        UpdateAllies();

    if (Unit *target = me->GetVictim())
    {
        if (_needToStop())
        {
            _stopAttack();
            return;
        }
    }
    else
    {
        if (me->IsInCombat())
        {
            if (!m_owner || !m_owner->GetObjectGuid().IsPlayer())
                _stopAttack();
        }
        else if (m_owner && me->GetCharmInfo())
        {
            if (m_owner->IsInCombat() && !(me->HasReactState(REACT_PASSIVE) || me->GetCharmInfo()->HasCommandState(COMMAND_STAY)))
                AttackStart(m_owner->GetAttackerForHelper());
            else if (me->GetCharmInfo()->HasCommandState(COMMAND_FOLLOW) && !me->HasUnitState(UNIT_STAT_FOLLOW))
                me->GetMotionMaster()->MoveFollow(m_owner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
        }
    }

    if (!me->GetCharmInfo())
        return;

    if (!me->HasUnitState(UNIT_STAT_CASTING))
    {
        for (uint8 i = 0; i < me->GetPetAutoSpellSize(); i++)
            PrepareSpellForAutocast(me->GetPetAutoSpellOnPos(i));

        AutocastPreparedSpells();
    }
}
