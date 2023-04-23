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
SDName: Felwood
SD%Complete: 95
SDComment: Quest support: 4101, 4102
SDCategory: Felwood
EndScriptData */

/* ContentData
npcs_riverbreeze_and_silversky
EndContentData */

#include "scriptPCH.h"
#include "ScriptedFollowerAI.h"

/*######
## npcs_riverbreeze_and_silversky
######*/

#define GOSSIP_ITEM_BEACON  "Please make me a Cenarion Beacon"

bool GossipHello_npcs_riverbreeze_and_silversky(Player *player, Creature *_Creature)
{
    uint32 eCreature = _Creature->GetEntry();

    if( _Creature->isQuestGiver() )
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if( eCreature==9528 )
    {
        if( player->GetQuestRewardStatus(4101) )
        {
            player->ADD_GOSSIP_ITEM(0, GOSSIP_ITEM_BEACON, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->SEND_GOSSIP_MENU(2848, _Creature->GetGUID());
        }else if( player->GetTeam()==HORDE )
        player->SEND_GOSSIP_MENU(2845, _Creature->GetGUID());
        else
            player->SEND_GOSSIP_MENU(2844, _Creature->GetGUID());
    }

    if( eCreature==9529 )
    {
        if( player->GetQuestRewardStatus(4102) )
        {
            player->ADD_GOSSIP_ITEM(0, GOSSIP_ITEM_BEACON, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->SEND_GOSSIP_MENU(2849, _Creature->GetGUID());
        }else if( player->GetTeam() == ALLIANCE )
        player->SEND_GOSSIP_MENU(2843, _Creature->GetGUID());
        else
            player->SEND_GOSSIP_MENU(2842, _Creature->GetGUID());
    }

    return true;
}

bool GossipSelect_npcs_riverbreeze_and_silversky(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if( action==GOSSIP_ACTION_INFO_DEF+1 )
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->CastSpell(player, 15120, false);
    }
    return true;
}

enum
{
    QUEST_CORRUPTED_SABERS = 4506,
    NPC_KITTEN = 9937,
    NPC_SABER = 10657,
    NPC_WINNA = 9996,
    GOB_MOONWELL = 300025,
};

struct npc_winnas_kittenAI : public FollowerAI
{
    npc_winnas_kittenAI(Creature* c) : FollowerAI(c) {}

    uint8 status;
    Timer timer;

    void Reset()
    {
        status = 0;
        if (me->GetEntry() == NPC_KITTEN)
            timer.Reset(1000);
    }

    void UpdateFollowerAI(const uint32 diff)
    {
        if (timer.Expired(diff))
        {
            switch (status)
            {
                case 0: // look for moonwell
                {
                    GameObject* ok = NULL;
                    MaNGOS::AllGameObjectsWithEntryInGrid go_check(GOB_MOONWELL);
                    MaNGOS::ObjectSearcher<GameObject, MaNGOS::AllGameObjectsWithEntryInGrid> checker(ok, go_check);

                    Cell::VisitGridObjects(m_creature, checker, 10.0f);
                    if (ok)
                    {
                        status++;
                        timer = 500;
                        break;
                    }
                    timer = 3000;
                    break;
                }
                case 1: // emote
                {
                    DoTextEmote("jumps into moonwell and goes underwater.", m_creature);
                    timer = 2000;
                    status++;
                    break;
                }
                case 2: // change
                {
                    m_creature->UpdateEntry(NPC_SABER);
                    DoTextEmote("follows obediently.", m_creature->GetCharmerOrOwnerPlayerOrPlayerItself());
                    StartFollow(m_creature->GetCharmerOrOwnerPlayerOrPlayerItself());
                    m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    timer = 1000;
                    status++;
                    break;
                }
                case 3: // check for quest completion, not blizzlike
                {
                    Creature* questgiver = m_creature->GetMap()->GetCreatureById(NPC_WINNA);
                    if (questgiver && questgiver->IsWithinDist(m_creature, 20.0f))
                    {
                        m_creature->GetCharmerOrOwnerPlayerOrPlayerItself()->CompleteQuest(QUEST_CORRUPTED_SABERS);
                        m_creature->ForcedDespawn(1000);
                    }
                    timer = 2000;
                    break;
                }
            }
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_winnas_kitten(Creature* c)
{
    return new npc_winnas_kittenAI(c);
}

#define GOSSIP_ITEM_HASTAT "[Obtain Stave of the Ancient Keepers]"
bool GossipHello_npc_hastat_the_ancient(Player *player, Creature *_Creature)
{
    uint32 eCreature = _Creature->GetEntry();

    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu(_Creature->GetGUID());

    if (player->GetQuestRewardStatus(7635) && player->GetQuestRewardStatus(7636) && !player->HasItemCount(18715,1,true))
    {
        player->ADD_GOSSIP_ITEM(0, GOSSIP_ITEM_HASTAT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(2848, _Creature->GetGUID());
    }

    return true;
}

bool GossipSelect_npc_hastat_the_ancient(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->CastSpell(player, 24872, false);
    }
    return true;
}

void AddSC_felwood()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npcs_riverbreeze_and_silversky";
    newscript->pGossipHello = &GossipHello_npcs_riverbreeze_and_silversky;
    newscript->pGossipSelect = &GossipSelect_npcs_riverbreeze_and_silversky;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_winnas_kitten";
    newscript->GetAI = &GetAI_winnas_kitten;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_hastat_the_ancient";
    newscript->pGossipHello = &GossipHello_npc_hastat_the_ancient;
    newscript->pGossipSelect = &GossipSelect_npc_hastat_the_ancient;
    newscript->RegisterSelf();
}

