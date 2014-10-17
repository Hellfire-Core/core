/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * Copyright (C) 2008-2014 Hellground <http://hellground.net/>
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
SDName: Silvermoon_City
SD%Complete: 100
SDComment: Quest support: 9685
SDCategory: Silvermoon City
EndScriptData */

/* ContentData
npc_blood_knight_stillblade
npc_silvermoon_guard
EndContentData */

#include "precompiled.h"
#include "guard_ai.h"

/*#######
# npc_blood_knight_stillblade
#######*/

#define SAY_HEAL -1000334

#define QUEST_REDEEMING_THE_DEAD        9685
#define SPELL_SHIMMERING_VESSEL         31225
#define SPELL_REVIVE_SELF               32343

struct npc_blood_knight_stillbladeAI : public ScriptedAI
{
    npc_blood_knight_stillbladeAI(Creature *c) : ScriptedAI(c) {}

    uint32 lifeTimer;
    bool spellHit;

    void Reset()
    {
        lifeTimer = 120000;
        m_creature->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 32);
        m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1,7);   // lay down
        spellHit = false;
    }

    void EnterCombat(Unit *who)
    {
    }

    void MoveInLineOfSight(Unit *who)
    {
        return;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->GetUInt32Value(UNIT_FIELD_BYTES_1))
        {
            if(lifeTimer < diff)
                m_creature->AI()->EnterEvadeMode();
            else
                lifeTimer -= diff;
        }
    }

    void SpellHit(Unit *Hitter, const SpellEntry *Spellkind)
    {
        if((Spellkind->Id == SPELL_SHIMMERING_VESSEL) && !spellHit &&
            (Hitter->GetTypeId() == TYPEID_PLAYER) && (((Player*)Hitter)->IsActiveQuest(QUEST_REDEEMING_THE_DEAD)))
        {
            ((Player*)Hitter)->AreaExploredOrEventHappens(QUEST_REDEEMING_THE_DEAD);
            DoCast(m_creature,SPELL_REVIVE_SELF);
            m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
            m_creature->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
            //m_creature->RemoveAllAuras();
            DoScriptText(SAY_HEAL, m_creature);
            spellHit = true;
        }
    }
};

CreatureAI* GetAI_npc_blood_knight_stillblade(Creature *_Creature)
{
    return new npc_blood_knight_stillbladeAI (_Creature);
}

/*#######
# npc_silvermoon_guard
#######*/

//Obtain Guard AI
CreatureAI* GetAI_npc_silvermoon_guard(Creature *_Creature)
{
    return new guardAI (_Creature);
}

//Recive emote and replay for this
bool ReciveEmote_npc_silvermoon_guard(Player *player, Creature *_Creature, uint32 emote)
{
    switch(emote)
    {
        case TEXTEMOTE_DANCE:
            ((guardAI*)_Creature->AI())->EnterEvadeMode();
            break;
        case TEXTEMOTE_RUDE:
            if (_Creature->IsWithinDistInMap(player, 5))
                _Creature->HandleEmoteCommand(EMOTE_ONESHOT_RUDE);
            else
                _Creature->HandleEmoteCommand(EMOTE_ONESHOT_RUDE);
            break;
        case TEXTEMOTE_WAVE:
            _Creature->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
            break;
        case TEXTEMOTE_BOW:
            _Creature->HandleEmoteCommand(EMOTE_ONESHOT_BOW);
            break;
        case TEXTEMOTE_KISS:
            _Creature->HandleEmoteCommand(EMOTE_ONESHOT_FLEX);
            break;
    }

    return true;
}


void AddSC_silvermoon_city()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="npc_blood_knight_stillblade";
    newscript->GetAI = &GetAI_npc_blood_knight_stillblade;
    newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_silvermoon_guard";
	newscript->GetAI = &GetAI_npc_silvermoon_guard;
	newscript->pReceiveEmote = &ReciveEmote_npc_silvermoon_guard;
	newscript->RegisterSelf();
}

