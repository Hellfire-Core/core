/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Burning_Steppes
SD%Complete: 100
SDComment: Quest support: 4224, 4866
SDCategory: Burning Steppes
EndScriptData */

/* ContentData
npc_ragged_john
EndContentData */

#include "scriptPCH.h"

/*######
## npc_ragged_john
######*/

#define GOSSIP_HELLO    "Official buisness, John. I need some information about Marsha Windsor. Tell me about the last time you saw him."
#define GOSSIP_SELECT1  "So what did you do?"
#define GOSSIP_SELECT2  "Start making sense, dwarf. I don't want to have anything to do with your cracker, your pappy, or any sort of 'discreditin'."
#define GOSSIP_SELECT3  "Ironfoe?"
#define GOSSIP_SELECT4  "Interesting... continue John."
#define GOSSIP_SELECT5  "So that's how Windsor died..."
#define GOSSIP_SELECT6  "So how did he die?"
#define GOSSIP_SELECT7  "Ok so where the hell is he? Wait a minute! Are you drunk?"
#define GOSSIP_SELECT8  "WHY is he in Blackrock Depths?"
#define GOSSIP_SELECT9  "300? So the Dark Irons killed him and dragged him into the Depths?"
#define GOSSIP_SELECT10 "Ahh... Ironfoe"
#define GOSSIP_SELECT11 "Thanks, Ragged John. Your story was very uplifting and informative"

struct npc_ragged_johnAI : public ScriptedAI
{
    npc_ragged_johnAI(Creature *c) : ScriptedAI(c) {}

    void Reset() {}

    void MoveInLineOfSight(Unit *who)
    {
        if( who->HasAura(16468,0) )
        {
            if( who->GetTypeId() == TYPEID_PLAYER && m_creature->IsWithinDistInMap(who, 15) && who->isInAccessiblePlacefor(m_creature) )
            {
                DoCast(who,16472);
                ((Player*)who)->AreaExploredOrEventHappens(4866);
            }
        }

        ScriptedAI::MoveInLineOfSight(who);
    }

    void EnterCombat(Unit *who) {}
};

CreatureAI* GetAI_npc_ragged_john(Creature *_Creature)
{
    return new npc_ragged_johnAI (_Creature);
}

bool GossipHello_npc_ragged_john(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestStatus(4224) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, GOSSIP_HELLO, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(2713, _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_ragged_john(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_SELECT1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(2714, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_SELECT2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(2715, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_SELECT3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            player->SEND_GOSSIP_MENU(2716, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_SELECT4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            player->SEND_GOSSIP_MENU(2717, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_SELECT5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
            player->SEND_GOSSIP_MENU(2718, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_SELECT6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
            player->SEND_GOSSIP_MENU(2719, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_SELECT7, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
            player->SEND_GOSSIP_MENU(2720, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+7:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_SELECT8, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 8);
            player->SEND_GOSSIP_MENU(2721, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+8:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_SELECT9, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
            player->SEND_GOSSIP_MENU(2722, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+9:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_SELECT10, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
            player->SEND_GOSSIP_MENU(2723, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+10:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_SELECT11, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
            player->SEND_GOSSIP_MENU(2725, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+11:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(4224);
            break;
    }
    return true;
}

#define NPC_SHANI_PROUDTUSK 9136 

bool GOUse_go_proudtuskremains(Player *player, GameObject* _GO)
{
    if (!GetClosestCreatureWithEntry(_GO, NPC_SHANI_PROUDTUSK, 30.0f))
    {
        float x,y,z;
        player->GetNearPoint(x,y,z, 0.0f, 5.0f, frand(0, 2*M_PI));
        player->SummonCreature(NPC_SHANI_PROUDTUSK, x,y,z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 120000);
    }
    else
        return true;
    return false;
}

/*######
## Script for Quest: Broodling Essence
######*/

// Spells
#define SPELL_DRACO_INCARCINATRIX_900   16007
#define SPELL_CREATE_BROODLING_ESSENCE  16027
#define SPELL_FIREBALL                    13375

struct mob_broodlingessenceAI : public ScriptedAI
{

    mob_broodlingessenceAI(Creature *c) : ScriptedAI(c) {}

    bool onSpellEffect;
    Timer Fireball_Timer;

    void Reset()
    {
        Fireball_Timer.Reset(0);
        onSpellEffect = false;
    }

    void EnterCombat(Unit *who){}

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if(spell->Id == SPELL_DRACO_INCARCINATRIX_900)
        {
            onSpellEffect = true;
        }
    }

    void JustDied(Unit* killer)
    {
        if(onSpellEffect)
        {
            me->CastSpell(me, SPELL_CREATE_BROODLING_ESSENCE, true);
            me->RemoveCorpse();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;


        if (Fireball_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FIREBALL);
            Fireball_Timer = 10000;
        }
        
            

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_broodlingessence(Creature *_Creature)
{
    return new mob_broodlingessenceAI (_Creature);
}

void AddSC_burning_steppes()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_ragged_john";
    newscript->GetAI = &GetAI_npc_ragged_john;
    newscript->pGossipHello =  &GossipHello_npc_ragged_john;
    newscript->pGossipSelect = &GossipSelect_npc_ragged_john;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_proudtuskremains";
    newscript->pGOUse = &GOUse_go_proudtuskremains;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_broodlingessence";
    newscript->GetAI = &GetAI_mob_broodlingessence;
    newscript->RegisterSelf();
}

