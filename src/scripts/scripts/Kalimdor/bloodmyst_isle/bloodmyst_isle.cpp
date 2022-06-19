/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Bloodmyst_Isle
SD%Complete: 80
SDComment: Quest support: 9670, 9667, 9756.
SDCategory: Bloodmyst Isle
EndScriptData */

/* ContentData
mob_webbed_creature
npc_captured_sunhawk_agent
npc_exarch_admetius
npc_princess_stillpine
go_princess_stillpine_cage
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## mob_webbed_creature
######*/

//possible creatures to be spawned
const uint32 possibleSpawns[32] = {17322, 17661, 17496, 17522, 17340, 17352, 17333, 17524, 17654, 17348, 17339, 17345, 17359, 17353, 17336, 17550, 17330, 17701, 17321, 17680, 17325, 17320, 17683, 17342, 17715, 17334, 17341, 17338, 17337, 17346, 17344, 17327};

struct mob_webbed_creatureAI : public ScriptedAI
{
    mob_webbed_creatureAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
    }

    void JustDied(Unit* Killer)
    {
        uint32 spawnCreatureID;

        switch(rand()%3)
        {
            case 0:
                spawnCreatureID = 17681;
                if (Killer->GetTypeId() == TYPEID_PLAYER)
                    ((Player*)Killer)->KilledMonster(spawnCreatureID, m_creature->GetGUID());
                break;
            case 1:
            case 2:
                spawnCreatureID = possibleSpawns[rand()%31];
                break;
        }

        if(spawnCreatureID)
            DoSpawnCreature(spawnCreatureID,0,0,0,m_creature->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
    }
};
CreatureAI* GetAI_mob_webbed_creature(Creature *_Creature)
{
    return new mob_webbed_creatureAI (_Creature);
}

/*######
## npc_captured_sunhawk_agent
######*/

#define C_SUNHAWK_TRIGGER 17974

#define GOSSIP_ITEM1 "I'm a prisoner, what does it look like? The draenei filth captured me as I exited the sun gate. They killed our portal controllers and destroyed the gate. The Sun King will be most displeased with this turn of events."

#define GOSSIP_ITEM2 "Ah yes, Sironas. I had nearly forgoten that Sironas was here. I served under Sironas back on Outland. I hadn't heard of this abomination though; those damnable draenei captured me before I even fully materialized on this world."
#define GOSSIP_ITEM3 "Incredible. How did Sironas accomplish such a thing?"
#define GOSSIP_ITEM4 "Sironas is an eredar... I mean, yes, obviously."
#define GOSSIP_ITEM5 "The Vector Coil is massive. I hope we have more than one abomination guarding the numerous weak points."
#define GOSSIP_ITEM6 "I did and you believed me. Thank you for the information, blood elf. You have helped us more than you could know."
#define say_captured_sunhawk_agent "Treacherous whelp! Sironas will destroy you and your people!"

bool GossipHello_npc_captured_sunhawk_agent(Player *player, Creature *_Creature)
{
    if (player->HasAura(31609,1) && player->GetQuestStatus(9756) == QUEST_STATUS_INCOMPLETE)
    {
        player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(9136, _Creature->GetGUID());
    }
    else
        player->SEND_GOSSIP_MENU(9134, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_captured_sunhawk_agent(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(9137, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->SEND_GOSSIP_MENU(9138, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->SEND_GOSSIP_MENU(9139, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            player->SEND_GOSSIP_MENU(9140, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
            player->SEND_GOSSIP_MENU(9141, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            player->CLOSE_GOSSIP_MENU();
            _Creature->Say(say_captured_sunhawk_agent,LANG_UNIVERSAL,player->GetGUID() );
            player->TalkedToCreature(C_SUNHAWK_TRIGGER, _Creature->GetGUID());
            break;
    }
    return true;
}

/*######
## npc_exarch_admetius
######*/

#define GOSSIP_ITEM_EXARCH "Create bloodelf disguise."

bool GossipHello_npc_exarch_admetius(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if( player->GetQuestStatus(9756) == QUEST_STATUS_INCOMPLETE )
        player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_EXARCH, GOSSIP_SENDER_MAIN, GOSSIP_SENDER_INFO );

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_exarch_admetius(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_SENDER_INFO )
    {
        player->CastSpell( player, 31609, false);
        player->AddAura( 31609, player );
    }
    return true;
}

/*########
## Quest: Saving Princess Stillpine
########*/
struct npc_princess_stillpineAI : public ScriptedAI
{
        npc_princess_stillpineAI(Creature *c) : ScriptedAI(c){}

        int32 FleeTimer;

        void Reset()
        {
            FleeTimer = 0;
        }

        void UpdateAI(const uint32 diff)
        {
            if(FleeTimer)
            {
                FleeTimer -= diff;
                if(FleeTimer <= diff)
                    m_creature->ForcedDespawn();  
            }
        }
};

CreatureAI* GetAI_npc_princess_stillpineAI(Creature *_Creature)
{
    return new npc_princess_stillpineAI (_Creature);
}

bool GOUse_go_princess_stillpine_cage(Player* pPlayer, GameObject* pGO)
{
    Unit *Prisoner = FindCreature(17682, 4.0f, pPlayer);
    if(!Prisoner)
        return true;

    if (pGO->GetGoType() == GAMEOBJECT_TYPE_DOOR)
    {
        DoScriptText(-1230010-urand(0, 2), Prisoner, pPlayer);
        pPlayer->CastCreatureOrGO(17682, Prisoner->GetGUID(), 31003);
        ((Creature*)Prisoner)->GetMotionMaster()->MoveFleeing(pPlayer,4000);
        CAST_AI(npc_princess_stillpineAI, ((Creature*)Prisoner)->AI())->FleeTimer = 4000;
    }
        
    return false;
}

// Ending their world
enum
{
    QUEST_ENDING_THEIR_WORLD = 9759,

    TEXTS_START = -1000800,
};

struct npc_demolitionist_legosoAI : public npc_escortAI
{
    npc_demolitionist_legosoAI(Creature* c) : npc_escortAI(c) {};

    uint8 phase;
    Timer timer;

    void Reset()
    {
        phase = 0;
        timer = 0;
    }

    void WaypointReached(uint32 point)
    {
        switch (point)
        {
        case 1:
            DoScriptText(TEXTS_START - 1, m_creature, GetPlayerForEscort());
            break;
        case 11:
            DoScriptText(TEXTS_START - 2, m_creature, GetPlayerForEscort());
            break;
        case 12:
            SetEscortPaused(true);
            SetCanAttack(false);
            phase = 1;
            timer.Reset(10000);
            m_creature->HandleEmoteCommand(EMOTE_ONESHOT_KNEEL);
            DoScriptText(TEXTS_START - 3, m_creature, GetPlayerForEscort());
            break;
        case 13:
            SetEscortPaused(true);
            timer.Reset(2000);
            break;
        case 16:
            DoScriptText(TEXTS_START - 10, m_creature, GetPlayerForEscort());
            break;
        case 17:
            DoScriptText(TEXTS_START - 11, m_creature, GetPlayerForEscort());
            break;
        case 18:
            DoScriptText(TEXTS_START - 12, m_creature, GetPlayerForEscort());
            break;
        case 19:
            DoScriptText(TEXTS_START - 13, m_creature, GetPlayerForEscort());
            break;
        case 20:
            SetEscortPaused(true);
            SetCanAttack(false);
            timer.Reset(10000);
            m_creature->HandleEmoteCommand(EMOTE_ONESHOT_KNEEL);
            break;
        case 21:
            SetEscortPaused(true);
            timer.Reset(2000);
            break;
        default:
            return;
        }
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (timer.Expired(diff))
            {
                switch (phase)
                {
                case 1:
                    DoScriptText(TEXTS_START - 4, m_creature, GetPlayerForEscort());
                    timer = 10000;
                    break;
                case 2:
                    DoScriptText(TEXTS_START - 5, m_creature, GetPlayerForEscort());
                    timer = 0;
                    m_creature->HandleEmoteCommand(EMOTE_ONESHOT_NONE);
                    SetEscortPaused(false);
                    SetCanAttack(true);
                    break;
                case 3:
                case 9:
                    DoScriptText(TEXTS_START - 6, m_creature, GetPlayerForEscort());
                    timer = 1000;
                    break;
                case 4:
                case 10:
                    DoScriptText(TEXTS_START - 7, m_creature, GetPlayerForEscort());
                    timer = 1000;
                    break;
                case 5:
                case 11:
                    DoScriptText(TEXTS_START - 8, m_creature, GetPlayerForEscort());
                    timer = 5000;
                    break;
                case 6:
                    DoScriptText(TEXTS_START - 9, m_creature, GetPlayerForEscort());
                    timer = 0;
                    SetEscortPaused(false);
                    break;
                case 7:
                    DoScriptText(TEXTS_START - 14, m_creature, GetPlayerForEscort());
                    timer = 10000;
                    break;
                case 8:
                    DoScriptText(TEXTS_START - 15, m_creature, GetPlayerForEscort());
                    timer = 0;
                    m_creature->HandleEmoteCommand(EMOTE_ONESHOT_NONE);
                    SetEscortPaused(false);
                    SetCanAttack(true);
                    break;
                case 12:
                    DoScriptText(TEXTS_START - 16, m_creature, GetPlayerForEscort());
                    SetCanAttack(true);
                    timer = 10000;
                    break;
                case 13:
                    DoScriptText(TEXTS_START - 17, m_creature, GetPlayerForEscort());
                    timer = 3000;
                    break;
                case 14:
                    DoScriptText(TEXTS_START - 18, m_creature, GetPlayerForEscort());
                    timer = 0;
                    if (Player* plr = GetPlayerForEscort())
                        plr->GroupEventHappens(QUEST_ENDING_THEIR_WORLD, m_creature);
                    m_creature->setDeathState(JUST_DIED);
                    m_creature->Respawn();
                    break;
                }

                phase++;

            }
            return;
        }

        DoMeleeAttackIfReady();

    }
};

CreatureAI* GetAI_npc_demolitionist_legoso(Creature* c)
{
    return new npc_demolitionist_legosoAI(c);
}

bool QuestAccept_demolitionist_legoso(Player* plr, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_ENDING_THEIR_WORLD)
    {
        creature->GetMotionMaster()->Clear();
        if (npc_escortAI* pEscortAI = CAST_AI(npc_demolitionist_legosoAI, creature->AI()))
        {
            pEscortAI->Start(true, true, plr->GetGUID(), quest);
            pEscortAI->SetDespawnAtEnd(false);
        }
        creature->SetSpeed(MOVE_RUN, 0.9);
    }
    return true;
}

void AddSC_bloodmyst_isle()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="mob_webbed_creature";
    newscript->GetAI = &GetAI_mob_webbed_creature;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_captured_sunhawk_agent";
    newscript->pGossipHello =  &GossipHello_npc_captured_sunhawk_agent;
    newscript->pGossipSelect = &GossipSelect_npc_captured_sunhawk_agent;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name="npc_exarch_admetius";
    newscript->pGossipHello =  &GossipHello_npc_exarch_admetius;
    newscript->pGossipSelect = &GossipSelect_npc_exarch_admetius;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name="npc_princess_stillpine";
    newscript->GetAI = &GetAI_npc_princess_stillpineAI;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "go_princess_stillpine_cage";
    newscript->pGOUse = &GOUse_go_princess_stillpine_cage;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_demolitionist_legoso";
    newscript->GetAI = &GetAI_npc_demolitionist_legoso;
    newscript->pQuestAcceptNPC = &QuestAccept_demolitionist_legoso;
    newscript->RegisterSelf();
}

