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

#include "Totem.h"
#include "WorldPacket.h"
#include "MapManager.h"
#include "Log.h"
#include "Group.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "CreatureAI.h"

Totem::Totem() : Creature()
{
    m_isTotem = true;
    m_duration = 0;
    m_type = TOTEM_PASSIVE;
}

void Totem::Update(uint32 update_diff, uint32 diff)
{
    SendHeartBeat();
    UpdateVisibilityAndView();
    Unit *owner = GetOwner();
    if (!owner || !owner->IsAlive() || !this->IsAlive())
    {
        UnSummon();                                         // remove self
        return;
    }

    if (m_duration <= update_diff)
    {
        UnSummon();                                         // remove self
        return;
    }
    else
        m_duration -= update_diff;

    Creature::Update(update_diff, diff);
}

void Totem::Summon(Unit* owner)
{
    CreatureInfo const *cinfo = GetCreatureInfo();
    if (owner->GetTypeId()==TYPEID_PLAYER && cinfo)
    {
        uint32 modelid = 0;
        if (((Player*)owner)->GetTeam() == HORDE)
        {
            if (cinfo->Modelid_H1)
                modelid = cinfo->Modelid_H1;
            else if (cinfo->Modelid_H2)
                modelid = cinfo->Modelid_H2;
        }
        else
        {
            if (cinfo->Modelid_A1)
                modelid = cinfo->Modelid_A1;
            else if (cinfo->Modelid_A2)
                modelid = cinfo->Modelid_A2;
        }
        if (modelid)
            SetDisplayId(modelid);
        else
            sLog.outLog(LOG_DB_ERR, "Totem::Summon: Missing modelid information for entry %u, team %u, totem will use default values.",GetEntry(),((Player*)owner)->GetTeam());
    }

    // Only add if a display exists.
    sLog.outDebug("AddObject at Totem.cpp line 49");
    SetInstanceId(owner->GetInstanceId());
    owner->GetMap()->Add((Creature*)this);

    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM_OBSOLETE, 8);
    data << GetGUID();
    BroadcastPacket(&data,true);

    switch (m_type)
    {
        case TOTEM_PASSIVE:
            if (GetSpell(1))
                CastSpell(this, GetSpell(1), true);
            if (GetSpell())
                CastSpell(this, GetSpell(), false);
            break;
        case TOTEM_STATUE:  CastSpell(GetOwner(), GetSpell(), true); break;
        default: break;
    }

    if (GetEntry() == SENTRY_TOTEM_ENTRY)
        SetReactState(REACT_AGGRESSIVE);

    // call JustSummoned function when totem summoned from spell
    if (owner->GetTypeId() == TYPEID_UNIT && ((Creature*)owner)->IsAIEnabled)
        ((Creature*)owner)->AI()->JustSummoned(this);
}

void Totem::UnSummon()
{
    SendObjectDeSpawnAnim(GetGUID());

    CombatStop();
    RemoveAurasDueToSpell(GetSpell());
    Unit *owner = this->GetOwner();
    if (owner)
    {
        // clear owenr's totem slot
        for (int i = 0; i < MAX_TOTEM; ++i)
        {
            if (owner->m_TotemSlot[i]==GetGUID())
            {
                owner->m_TotemSlot[i] = 0;
                break;
            }
        }

        owner->RemoveAurasDueToSpell(GetSpell());

        //remove aura all party members too
        Group *pGroup = NULL;
        if (owner->GetTypeId() == TYPEID_PLAYER)
        {
            // Not only the player can summon the totem (scripted AI)
            pGroup = ((Player*)owner)->GetGroup();
            if (pGroup)
            {
                for (GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
                {
                    Player* Target = itr->getSource();
                    if (Target && pGroup->SameSubGroup((Player*)owner, Target))
                        Target->RemoveAurasDueToSpell(GetSpell());
                }
            }
        }

        // call SummonedCreatureDespawn function when totem UnSummoned
        if (owner->GetTypeId() == TYPEID_UNIT && ((Creature*)owner)->IsAIEnabled)
            ((Creature*)owner)->AI()->SummonedCreatureDespawn(this);
    }

    AddObjectToRemoveList();
}

void Totem::SetOwner(uint64 guid)
{
    SetCreatorGUID(guid);
    SetOwnerGUID(guid);
    if (Unit *owner = GetOwner())
    {
        this->setFaction(owner->getFaction());
        this->SetLevel(owner->GetLevel());
    }
}

Unit *Totem::GetOwner()
{
    uint64 ownerid = GetOwnerGUID();
    if (!ownerid)
        return NULL;
    return GetMap()->GetUnit(ownerid);
}

void Totem::SetTypeBySummonSpell(SpellEntry const * spellProto)
{
    // Get spell cast by totem
    SpellEntry const * totemSpell = sSpellStore.LookupEntry(GetSpell());
    if (totemSpell)
    {
        if(~totemSpell->Attributes & SPELL_ATTR_PASSIVE)
            m_type = TOTEM_ACTIVE;
        
    }
    if (spellProto->SpellIconID==2056)
        m_type = TOTEM_STATUE;                              //Jewelery statue
}

bool Totem::IsImmunedToSpell(SpellEntry const* spellInfo, bool useCharges)
{
    if (!(spellInfo->AttributesCu & SPELL_ATTR_CU_DIRECT_DAMAGE))
    {
        for (int i=0;i<3;i++)
        {
            switch (spellInfo->EffectApplyAuraName[i])
            {
                case SPELL_AURA_PERIODIC_DAMAGE:
                case SPELL_AURA_PERIODIC_LEECH:
                case SPELL_AURA_MOD_FEAR:
                case SPELL_AURA_TRANSFORM:
                    return true;
                default:
                    continue;
            }
        }
    }
    return Creature::IsImmunedToSpell(spellInfo, useCharges);
}
