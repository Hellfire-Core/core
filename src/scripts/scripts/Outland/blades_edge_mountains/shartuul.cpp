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

    SPELL_ACTIVATE_EVENT    = 40309,
    SPELL_SHIELD_VISUAL     = 40158,
    SPELL_BORDER            = 40071, // ??
    /*
WS_REMAINING_SHIELD     3055
LR_EVENT_CONTROLLER     23059
LR_TRIGGER              23260
LR_STUN_FIELD           23312
LR_STUN_ROPE_DUMMY      23313
LR_EREDAR_BREATH_TARGET 23328
LR_SHIELD_ZAPPER        23500
LR_PORTAL_SHIELD        23116
LR_SPELL_IMMUNE_INVIS   40357
LR_COSMETIC_SHIELD      40158
LR_FELGUARD_DEGRADER    23055
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
    uint8 stage;
    void Reset()
    {
        if (Unit* shield = FindCreature(NPC_SHIELD_GUY, 50, m_creature))
            shield->RemoveAurasDueToSpell(SPELL_SHIELD_VISUAL);

        stage = 0;
        borderguys[0] = borderguys[1] = borderguys[2] = borderguys[3] = 0;
        shieldguy = 0;
    }

    void DoAction(int32 param)
    {
        if (param != 175 || stage)
            return;
        stage = 1;
        std::list<Creature*> borders = FindAllCreaturesWithEntry(NPC_ARENA_BORDER, 120);
        for (std::list<Creature*>::iterator itr = borders.begin(); itr != borders.end(); itr++)
        {
            if ((*itr)->GetPositionY() > 7170)
                borderguys[0] = (*itr)->GetGUID(); //west
            else if ((*itr)->GetPositionX() > 2780)
                borderguys[1] = (*itr)->GetGUID(); //north
            else if ((*itr)->GetPositionY() < 7060)
                borderguys[2] = (*itr)->GetGUID(); //east
            else if ((*itr)->GetPositionY() < 2670)
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
            border->CastSpell(next, SPELL_BORDER, true);
        }
    }
};

CreatureAI* GetAI_npc_shartuul_trigger(Creature* creature)
{
    return new npc_shartuul_triggerAI(creature);
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
            trigger->AI()->DoAction(175);
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
    newscript->Name = "npc_felguard_degrader";
    newscript->GetAI = &GetAI_npc_felguard_degrader;
    newscript->RegisterSelf();
}