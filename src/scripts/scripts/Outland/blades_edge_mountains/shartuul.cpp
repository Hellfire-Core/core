/*
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

#include "precompiled.h"

enum someshartuuldata {
    NPC_EVENT_CONTROLLER    = 23059,
    NPC_FELGUARD_DEGRADER   = 23055,
    NPC_ARENA_BORDER        = 23313,
    NPC_SHIELD_GUY          = 23116,
    NPC_SHIELD_TARGET       = 23500,

    SPELL_ACTIVATE_EVENT    = 40309,
    SPELL_SHIELD_VISUAL     = 40158,
    SPELL_BORDER            = 40071,
    SPELL_PLAYER_IMMUNITY   = 40357,
    SPELL_PLAYER_STUN       = 41592,
    SPELL_POSSESS           = 39985, // there are multiple ones
    SPELL_SHIELD_SMASH      = 40222,

    WORLD_STATE_PERCENTAGE  = 3054,
    WORLD_STATE_SHIELD      = 3055,
    /*
LR_TRIGGER              23260
LR_STUN_FIELD           23312
LR_EREDAR_BREATH_TARGET 23328
LR_SHIELD_ZAPPER        23500
LR_DOOMGUARD_PUNISHER   23113
LR_SHIVAN_ASSASSIN      23220
LR_EYE_OF_SHARTUUL      23228
LR_DREADMAW             23275
LR_SHARTUUL             23230
LR_SHARTUUL_VOICE       23063
*/
};

struct npc_shartuul_triggerAI : public ScriptedAI
{
    npc_shartuul_triggerAI(Creature* c) : ScriptedAI(c) {}

    uint64 borderguys[4];
    uint64 shieldguy;

    uint64 playerGUID;
    uint8 stage;
    Timer shieldCountdown;
    uint8 shieldLevel;
    void Reset()
    {
        // neighbourhood cleanup
        if (Unit* shield = FindCreature(NPC_SHIELD_GUY, 50, m_creature))
            shield->RemoveAurasDueToSpell(SPELL_SHIELD_VISUAL);
        std::list<Creature*> borders = FindAllCreaturesWithEntry(NPC_ARENA_BORDER, 120);
        for (std::list<Creature*>::iterator itr = borders.begin(); itr != borders.end(); itr++)
        {
            (*itr)->CastStop();
            (*itr)->RemoveAllAuras();
        }

        if (Player* player = sObjectAccessor.GetPlayer(playerGUID))
        {
            player->RemoveAurasDueToSpell(SPELL_PLAYER_IMMUNITY);
            player->RemoveAurasDueToSpell(SPELL_PLAYER_STUN);
            player->StopCastingCharm();
            player->SendUpdateWorldState(WORLD_STATE_SHIELD, 0);
        }

        stage = 0;
        borderguys[0] = borderguys[1] = borderguys[2] = borderguys[3] = 0;
        shieldguy = 0;
        playerGUID = 0;
        shieldCountdown.Reset(0);
        shieldLevel = 8;
    }

    void StartFor(Unit* caster, Unit* felguard)
    {
        if (stage || !caster || !felguard || caster->GetTypeId() != TYPEID_PLAYER)
            return;
        stage = 1;
        std::list<Creature*> borders = FindAllCreaturesWithEntry(NPC_ARENA_BORDER, 120);
        for (std::list<Creature*>::iterator itr = borders.begin(); itr != borders.end(); itr++)
        {
            (*itr)->Respawn();
            if ((*itr)->GetPositionY() > 7170)
                borderguys[0] = (*itr)->GetGUID(); //west
            else if ((*itr)->GetPositionX() > 2780)
                borderguys[1] = (*itr)->GetGUID(); //north
            else if ((*itr)->GetPositionY() < 7060)
                borderguys[2] = (*itr)->GetGUID(); //east
            else if ((*itr)->GetPositionX() < 2670)
                borderguys[3] = (*itr)->GetGUID(); //south
        }
        
        if (Unit* shield = FindCreature(NPC_SHIELD_GUY, 50, m_creature))
        {
            shieldguy = shield->GetGUID();
            shield->CastSpell(shield, SPELL_SHIELD_VISUAL, true);
        }

        for (uint8 i = 0; i < 4; i++)
        {
            Unit* border = m_creature->GetUnit(borderguys[i]);
            Unit* next = m_creature->GetUnit(borderguys[(i + 1) % 4]);
            if (!border || !next) continue;
            border->CastSpell(next, SPELL_BORDER, false);
        }

        playerGUID = caster->GetGUID();
        caster->ToPlayer()->SendUpdateWorldState(WORLD_STATE_SHIELD, 1);
        caster->ToPlayer()->SendUpdateWorldState(WORLD_STATE_PERCENTAGE, 100);
        caster->CastSpell(felguard, SPELL_POSSESS, true);
        caster->CastSpell(caster, SPELL_PLAYER_IMMUNITY, true);
        caster->CastSpell(caster, SPELL_PLAYER_STUN, true);
        shieldCountdown.Reset(4 * MINUTE*IN_MILISECONDS);
    }

    void ShieldHit()
    {
        if (stage != 1)
            return;
        shieldLevel--;
        if (shieldLevel == 0)
        {
            Reset(); // go stage 2
            return;
        }
        if (Player* player = sObjectAccessor.GetPlayer(playerGUID))
        {
            player->SendUpdateWorldState(WORLD_STATE_PERCENTAGE, uint32(12.5 * shieldLevel));
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (shieldCountdown.Expired(diff))
        {
            Reset();
        }
    }
};

CreatureAI* GetAI_npc_shartuul_trigger(Creature* creature)
{
    return new npc_shartuul_triggerAI(creature);
}

struct npc_shartuul_shield_targetAI : public ScriptedAI
{
    npc_shartuul_shield_targetAI(Creature* c) : ScriptedAI(c) {}

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if (spell->Id == SPELL_SHIELD_SMASH)
        {
            Creature* trigger = m_creature->GetMap()->GetCreatureById(NPC_EVENT_CONTROLLER);
            if (!trigger)
                return;
            CAST_AI(npc_shartuul_triggerAI, trigger->AI())->ShieldHit();
        }
    }
};

CreatureAI* GetAI_npc_shartuul_shield_target(Creature* creature)
{
    return new npc_shartuul_shield_targetAI(creature);
}

struct npc_felguard_degraderAI : public ScriptedAI
{
    npc_felguard_degraderAI(Creature* c) : ScriptedAI(c) {}

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if (spell->Id == SPELL_ACTIVATE_EVENT)
        {
            Creature* trigger = m_creature->GetMap()->GetCreatureById(NPC_EVENT_CONTROLLER);
            if (!trigger)
                return;
            CAST_AI(npc_shartuul_triggerAI,trigger->AI())->StartFor(caster, m_creature);
        }
    }
};

CreatureAI* GetAI_npc_felguard_degrader(Creature* creature)
{
    return new npc_felguard_degraderAI(creature);
}

void AddSC_shartuul()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "npc_shartuul_trigger";
    newscript->GetAI = &GetAI_npc_shartuul_trigger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_shartuul_shield_target";
    newscript->GetAI = &GetAI_npc_shartuul_shield_target;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_felguard_degrader";
    newscript->GetAI = &GetAI_npc_felguard_degrader;
    newscript->RegisterSelf();
}