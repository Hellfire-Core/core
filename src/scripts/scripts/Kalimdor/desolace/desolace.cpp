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

/* ScriptData
SDName: Desolace
SD%Complete:
SDComment: Quest support: 1440, 5561, 5381, 6132
SDCategory: Desolace
EndScriptData */

/* ContentData
npc_aged_dying_ancient_kodo
go_iruxos
npc_dalinda_malem
npc_melizza_brimbuzzle
npc_rokaro
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_aged_dying_ancient_kodo. Quest 5561
######*/

enum eDyingKodo
{
    // signed for 9999
    SAY_SMEED_HOME_1                = -1600348,
    SAY_SMEED_HOME_2                = -1600349,
    SAY_SMEED_HOME_3                = -1600350,

    NPC_SMEED                       = 11596,
    NPC_AGED_KODO                   = 4700,
    NPC_DYING_KODO                  = 4701,
    NPC_ANCIENT_KODO                = 4702,
    NPC_TAMED_KODO                  = 11627,

    SPELL_KODO_KOMBO_ITEM           = 18153,
    SPELL_KODO_KOMBO_PLAYER_BUFF    = 18172,
    SPELL_KODO_KOMBO_DESPAWN_BUFF   = 18377,
    SPELL_KODO_KOMBO_GOSSIP         = 18362

};

struct npc_aged_dying_ancient_kodoAI : public ScriptedAI
{
    npc_aged_dying_ancient_kodoAI(Creature* pCreature) : ScriptedAI(pCreature) {}

    void MoveInLineOfSight(Unit* pWho)
    {
        if (pWho->GetEntry() == NPC_SMEED && !m_creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP) &&
            m_creature->IsWithinDistInMap(pWho, 10.0f))
        {
            DoScriptText(RAND(SAY_SMEED_HOME_1,SAY_SMEED_HOME_2,SAY_SMEED_HOME_3), pWho);
            m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }
    }

    void SpellHit(Unit* pCaster, SpellEntry const* pSpell)
    {
        if (pSpell->Id == SPELL_KODO_KOMBO_ITEM)
        {
            //no effect if player/creature already have aura from spells
            if (pCaster->HasAura(SPELL_KODO_KOMBO_PLAYER_BUFF, 0) || m_creature->HasAura(SPELL_KODO_KOMBO_DESPAWN_BUFF, 0))
                return;

            if (m_creature->GetEntry() != NPC_AGED_KODO && m_creature->GetEntry() != NPC_DYING_KODO &&
                m_creature->GetEntry() != NPC_ANCIENT_KODO)
                return;

            pCaster->CastSpell(pCaster, SPELL_KODO_KOMBO_PLAYER_BUFF, true);

            m_creature->UpdateEntry(NPC_TAMED_KODO);
            m_creature->CastSpell(m_creature, SPELL_KODO_KOMBO_DESPAWN_BUFF, false);
            m_creature->GetMotionMaster()->MoveIdle();

            m_creature->GetMotionMaster()->MoveFollow(pCaster, PET_FOLLOW_DIST, m_creature->GetFollowAngle());
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (me->GetEntry() == NPC_TAMED_KODO)
        {
            me->CombatStop();
            if (!me->HasAura(SPELL_KODO_KOMBO_DESPAWN_BUFF))
            {
                me->ForcedDespawn(0);
            }
        }
        
        
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_aged_dying_ancient_kodo(Creature* pCreature)
{
    return new npc_aged_dying_ancient_kodoAI(pCreature);
}

bool GossipHello_npc_aged_dying_ancient_kodo(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->HasAura(SPELL_KODO_KOMBO_PLAYER_BUFF, 0) && pCreature->HasAura(SPELL_KODO_KOMBO_DESPAWN_BUFF, 0))
    {
        //the expected quest objective
        pPlayer->CastCreatureOrGO(pCreature->GetEntry(), pCreature->GetGUID(), SPELL_KODO_KOMBO_GOSSIP);

        pPlayer->RemoveAurasDueToSpell(SPELL_KODO_KOMBO_PLAYER_BUFF);
        pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        pCreature->ForcedDespawn(5000);
    }

    pPlayer->SEND_GOSSIP_MENU(pCreature->GetNpcTextId(), pCreature->GetGUID());
    return true;
}

/*######
## go_iruxos. Quest 5381
######*/

bool GOUse_go_iruxos(Player *pPlayer, GameObject* pGO)
{
        if (pPlayer->GetQuestStatus(5381) == QUEST_STATUS_INCOMPLETE)
        {
            Creature* Demon = pPlayer->SummonCreature(11876, pPlayer->GetPositionX()+frand(-2,2),pPlayer->GetPositionY()+frand(-2,2),pPlayer->GetPositionZ(),0,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,60000);
            if(Demon)
            {
                Demon->AI()->AttackStart(pPlayer);
                pGO->SetLootState(GO_JUST_DEACTIVATED);
                return true;
            }
        }
        return false;
}

/*######
## npc_dalinda_malem. Quest 1440
######*/

#define QUEST_RETURN_TO_VAHLARRIEL     1440

struct npc_dalindaAI : public npc_escortAI
{
    npc_dalindaAI(Creature* pCreature) : npc_escortAI(pCreature) { }

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();
        switch (i)
        {
            case 1:
                me->SetStandState(PLAYER_STATE_NONE);
                break;
            case 15:
                if (pPlayer)
                pPlayer->GroupEventHappens(QUEST_RETURN_TO_VAHLARRIEL, m_creature);
                break;
            case 16:
                me->Kill(me, false);
                me->Respawn();
                break;
        }
    }

    void EnterCombat(Unit* pWho) { }

    void Reset()
    {
        me->SetStandState(PLAYER_STATE_KNEEL);
    }

    void JustDied(Unit* pKiller)
    {
        if(pKiller->GetGUID() == me->GetGUID())
            return;
        Player* pPlayer = GetPlayerForEscort();
        if (pPlayer)
            pPlayer->FailQuest(QUEST_RETURN_TO_VAHLARRIEL);
        return;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        npc_escortAI::UpdateAI(uiDiff);
        if (!UpdateVictim())
            return;
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_dalinda(Creature* pCreature)
{
    return new npc_dalindaAI(pCreature);
}

bool QuestAccept_npc_dalinda(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_RETURN_TO_VAHLARRIEL)
   {
        if (npc_escortAI* pEscortAI = CAST_AI(npc_dalindaAI, pCreature->AI()))
        {
            pEscortAI->Start(true, false, pPlayer->GetGUID(), quest, true);
            pCreature->setFaction(113);
        }
    }
    return false;
}

/*#######
## npc_melizza_brimbuzzle
#######*/

enum
{
    SAY_START                   = -1000607,
    SAY_COMPLETE                = -1000608,
    SAY_POST_EVENT_1            = -1000609,
    SAY_POST_EVENT_2            = -1000610,
    SAY_POST_EVENT_3            = -1000611,

    NPC_MARAUDINE_BONEPAW       = 4660,
    NPC_MARAUDINE_SCOUT         = 4654,

    GO_MELIZZAS_CAGE            = 177706,
    QUEST_GET_ME_OUT_OF_HERE    = 6132
};

static float m_afAmbushSpawn[4][3]=
{
    {-1388.37f, 2427.81f, 88.8286f},
    {-1388.78f, 2431.85f, 88.7838f},
    {-1386.95f, 2429.76f, 88.8444f},
    {-1389.99f, 2429.93f, 88.7692f} 
};

struct npc_melizza_brimbuzzleAI : public npc_escortAI
{
    npc_melizza_brimbuzzleAI(Creature* pCreature) : npc_escortAI(pCreature) { }

    uint32 m_uiPostEventCount;
    uint64 m_uiPostEventTimer;
        
    void Reset()
    {
        m_uiPostEventCount = 0;
        m_uiPostEventTimer = 0;
    }

    void WaypointReached(uint32 uiPointId)
    {
        if (Player* pPlayer = GetPlayerForEscort())
        {
            switch (uiPointId)
            {
            case 1:
                me->setFaction(113);
                DoScriptText(SAY_START, me, pPlayer);
                break;
            case 7:
                 me->SummonCreature(NPC_MARAUDINE_SCOUT, m_afAmbushSpawn[0][0], m_afAmbushSpawn[0][1], m_afAmbushSpawn[0][2], 1.6f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 600000);
                 me->SummonCreature(NPC_MARAUDINE_SCOUT, m_afAmbushSpawn[1][0], m_afAmbushSpawn[1][1], m_afAmbushSpawn[1][2], 1.6f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 600000);
                 me->SummonCreature(NPC_MARAUDINE_SCOUT, m_afAmbushSpawn[2][0], m_afAmbushSpawn[2][1], m_afAmbushSpawn[2][2], 1.6f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 600000);
                 me->SummonCreature(NPC_MARAUDINE_BONEPAW, m_afAmbushSpawn[3][0], m_afAmbushSpawn[3][1], m_afAmbushSpawn[3][2], 1.6f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 600000);
                 break;
            case 10:
                DoScriptText(SAY_COMPLETE, me);
                me->RestoreFaction();
                SetRun();
                pPlayer->GroupEventHappens(QUEST_GET_ME_OUT_OF_HERE, me);
                break;
            case 15:
                m_uiPostEventCount = 1;
                break;

            }
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {        
        npc_escortAI::UpdateAI(uiDiff);

        if (!UpdateVictim())
        {
            if (m_uiPostEventCount && HasEscortState(STATE_ESCORT_ESCORTING))
            {
                if (m_uiPostEventTimer <= uiDiff)
                {
                    m_uiPostEventTimer = 3000;

                    if (Unit* pPlayer = GetPlayerForEscort())
                    {
                        switch(m_uiPostEventCount)
                        {
                            case 1:
                                DoScriptText(SAY_POST_EVENT_1, me);
                                ++m_uiPostEventCount;
                                break;
                            case 2:
                                DoScriptText(SAY_POST_EVENT_2, me);
                                ++m_uiPostEventCount;
                                break;
                            case 3:
                                DoScriptText(SAY_POST_EVENT_3, me);
                                m_uiPostEventCount = 0;
                                me->ForcedDespawn(60000);
                                break;
                        }
                    }
                }
                else
                    m_uiPostEventTimer -= uiDiff;
            }

            return;
        }

        DoMeleeAttackIfReady();
    }

    void JustSummoned(Creature* pSummoned)
    {
        pSummoned->AI()->AttackStart(me);
    }
};

CreatureAI* GetAI_npc_melizza_brimbuzzle(Creature* pCreature)
{
    return new npc_melizza_brimbuzzleAI(pCreature);
}

bool QuestAccept_npc_melizza_brimbuzzle(Player* pPlayer, Creature* pCreature, Quest const* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_GET_ME_OUT_OF_HERE)
    {
        if (GameObject* pGo = FindGameObject(GO_MELIZZAS_CAGE, INTERACTION_DISTANCE, pCreature))
            pGo->UseDoorOrButton();

        if (npc_melizza_brimbuzzleAI* pEscortAI = CAST_AI(npc_melizza_brimbuzzleAI, pCreature->AI()))
            pEscortAI->Start(false, false, pPlayer->GetGUID(), pQuest);
    }
    return true;
}

/*####
# npc_rokaro
####*/

#define GOSSIP_ITEM_ROKARO "Restore Drakefire Amulet."

bool GossipHello_npc_rokaro(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if(player->GetQuestRewardStatus(6602) && !player->HasItemCount(16309,1))
        player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_ROKARO, GOSSIP_SENDER_MAIN, GOSSIP_SENDER_INFO );
        player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_rokaro(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_SENDER_INFO )
    {
            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 16309, 1);
            if (msg == EQUIP_ERR_OK)
            {
                 Item* item = player->StoreNewItem(dest, 16309, true);
                 player->SendNewItem(item,1,true,false,true);
            }
    player->CLOSE_GOSSIP_MENU();
    }
    return true;
}

struct npc_magram_spectreAI : public ScriptedAI
{
    npc_magram_spectreAI(Creature* c) : ScriptedAI(c) {}
    
    Timer checker;

    void Reset()
    {
        m_creature->SetWalk(true);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetVisibility(VISIBILITY_OFF);
        checker = 60000;
        checker.SetCurrent(urand(0, 60000));
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim() && checker.Expired(diff))
        {
            checker = 60000;
            GameObject* gob = NULL;
            Hellground::NearestGameObjectEntryInObjectRangeCheck check(*m_creature,177746,50.0f);
            Hellground::ObjectSearcher<GameObject, Hellground::NearestGameObjectEntryInObjectRangeCheck> checker(gob,check);

            Cell::VisitGridObjects(m_creature, checker, 50);
            if (gob)
            {
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                m_creature->SetVisibility(VISIBILITY_ON);
                float x, y, z;
                gob->GetNearPoint(x, y, z, 10, 0, frand(0, 2 * M_PI));
                m_creature->GetMotionMaster()->MovePoint(0, x, y, z);
            }
            return;
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_magram_spectre(Creature* crea)
{
    return new npc_magram_spectreAI(crea);
}

/*######
# Gizelton caravan
# this script is supposed to deal with 300 waypoints long path, 2 defense quests, some events and stuff <3
######*/

enum eGizeltonCaravan
{
    NPC_CARAVAN         = 11630,
    NPC_RIGGER          = 11626,
    NPC_CORK            = 11625,
    NPC_GIZELTON_KODO   = 11564,
    NPC_VENDOR_TRON     = 12245,
    NPC_SUPER_SELLER    = 12246,

    NPC_KOLKAR_AMBUSHER = 12977,
    NPC_KOLKAR_WAYLAYER = 12976,
    NPC_NETHER_SORCERES = 4684,
    NPC_DOOMWARDER      = 4677,
    NPC_LESSER_INFERNAL = 4676,

    QUEST_GIZELTON_CARAVAN      = 5943,
    QUEST_BODYGUARD_FOR_HIRE    = 5821,

    WAYPOINT_NORTH_STOP     = 52,
    WAYPOINT_NORTH_QUEST    = 76,
    WAYPOINT_NORTH_WAVE1    = 84,
    WAYPOINT_NORTH_WAVE2    = 92,
    WAYPOINT_NORTH_WAVE3    = 100,
    WAYPOINT_NORTH_FINISH   = 107,
    WAYPOINT_SOUTH_STOP     = 193,
    WAYPOINT_SOUTH_QUEST    = 207,
    WAYPOINT_SOUTH_WAVE1    = 216,
    WAYPOINT_SOUTH_WAVE2    = 224,
    WAYPOINT_SOUTH_WAVE3    = 234,
    WAYPOINT_SOUTH_FINISH   = 239,

    GIZELTON_TEXT_BEGIN = -1000050,

};
    
static const float caravan_pos[][4] = {
    { -721.4f, 1474.2f, 91.3f, 5.8f },
    { -716.8f, 1467.0f, 91.3f, 5.6f },
    { -715.4f, 1482.1f, 91.3f, 5.5f },
    { -719.8f, 1480.5f, 91.3f, 5.6f },
    { -721.1f, 1476.2f, 91.3f, 6.0f },

    { -1922.6f, 2432.4f, 60.9f, 0.25f },
    { -1919.8f, 2421.7f, 60.9f, 0.25f },
    { -1922.5f, 2423.2f, 60.9f, 0.6f },
    { -1922.4f, 2434.8f, 60.9f, 6.1f },
    { -1923.1f, 2424.2f, 60.9f, 0.6f },
    
};

struct npc_gizelton_caravanAI : public ScriptedAI
{
    npc_gizelton_caravanAI(Creature* c) : ScriptedAI(c) { }

    std::vector<ScriptPointMove> points;
    std::vector<ScriptPointMove>::iterator current;
    Timer pointWait;
    Timer checkTimer;
    uint64 members[5];//rigger,kodo,gizelton,kodo,robot
    std::list<uint64> playerGUIDs;

    void Reset()
    {
        points = sScriptMgr.GetPointMoveList(me->GetEntry());
        pointWait.Reset(100);
        current = points.begin();
        me->SetWalk(false);

        playerGUIDs.empty();
        GetMembers();
        me->setActive(true);
        members[4] = 0;
        checkTimer.Reset(10000);
    }
    
    void GetMembers()
    {
        Map* map = me->GetMap();
        if (!map)
            return;
        members[0] = map->GetCreatureGUID(NPC_RIGGER);
        if (Creature* rigger = me->GetCreature(members[0]))
            rigger->setActive(true);

        members[2] = map->GetCreatureGUID(NPC_CORK);
        if (Creature* cork = me->GetCreature(members[2]))
            cork->setActive(true);

        if (!members[1] || !me->GetCreature(members[1]))
        {
            Creature* kodo = me->SummonCreature(NPC_GIZELTON_KODO, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_DEAD_DESPAWN, 1000);
            if (kodo)
            {
                members[1] = kodo->GetGUID();
                kodo->setActive(true);
            }
        }
        if (!members[3] || !me->GetCreature(members[3]))
        {
            Creature* kodo = me->SummonCreature(NPC_GIZELTON_KODO, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_DEAD_DESPAWN, 1000);
            if (kodo)
            {
                members[3] = kodo->GetGUID();
                kodo->setActive(true);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (pointWait.Expired(diff))
        {
            switch (current->uiPointId)
            {
            case WAYPOINT_NORTH_STOP:
            {
                if (Creature* robot = me->GetCreature(members[4]))
                    robot->ForcedDespawn();
                members[4] = 0;
                if (Creature* cork = me->GetCreature(members[2]))
                    DoScriptText(GIZELTON_TEXT_BEGIN, cork);
                break;
            }
            case WAYPOINT_SOUTH_STOP:
            {
                if (Creature* robot = me->GetCreature(members[4]))
                    robot->ForcedDespawn();
                members[4] = 0;
                if (Creature* rigger = me->GetCreature(members[0]))
                    DoScriptText(GIZELTON_TEXT_BEGIN - 6, rigger);
                break;
            }
            case WAYPOINT_NORTH_QUEST:
            case WAYPOINT_SOUTH_QUEST:
            {
                if (!playerGUIDs.empty())
                {
                    for (uint8 i = 0; i < 4; i++)
                    {
                        Creature* member = me->GetCreature(members[i]);
                        if (member)
                            member->setFaction(FACTION_ESCORT_N_NEUTRAL_PASSIVE);
                    }
                    checkTimer.Reset(1000);
                }
                if (Creature* member = me->GetCreature(members[0]))
                    member->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                if (Creature* member = me->GetCreature(members[2]))
                    member->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                break;
            }
            }

            current++;
            if (current == points.end())
                current = points.begin();

            CaravanMovesTo(current->uiPointId, current->fX, current->fY, current->fZ);
            pointWait.Reset(0);
        }

        if (checkTimer.Expired(diff))
        {
            if (playerGUIDs.empty()) // not during quest every 10 sec check if we have all members
            {
                GetMembers();
                checkTimer = 10000; 
            }
            else // during quest check every second if we are alive
            {
                Creature* rigger = me->GetCreature(members[0]);
                Creature* cork = me->GetCreature(members[2]);
                if (!rigger || !cork || !rigger->isAlive() || !cork->isAlive())
                {
                    for (std::list<uint64>::iterator itr = playerGUIDs.begin(); itr != playerGUIDs.end(); itr++)
                    {
                        Player* plr = me->GetPlayer(*itr);
                        if (plr && plr->IsActiveQuest(QUEST_BODYGUARD_FOR_HIRE))
                            plr->FailQuest(QUEST_BODYGUARD_FOR_HIRE);
                        if (plr && plr->IsActiveQuest(QUEST_GIZELTON_CARAVAN))
                            plr->FailQuest(QUEST_GIZELTON_CARAVAN);
                    }
                    playerGUIDs.clear();

                    if (cork)
                    {
                        cork->CombatStop();
                        cork->RestoreFaction();
                        cork->Respawn();
                    }

                    if (rigger)
                    {
                        rigger->CombatStop();
                        rigger->RestoreFaction();
                        rigger->Respawn();
                    }
                }
                checkTimer = 1000;
            }
        }
    }

    void CaravanMovesTo(uint32 point, float x, float y, float z)
    {
        SendDebug("Starting movement to point %u (%f %f %f)", point, x, y, z);
        float pathangle = atan2(me->GetPositionY() - y, me->GetPositionX() - x);
        me->GetMotionMaster()->MovePoint(current->uiPointId, current->fX, current->fY, current->fZ);
        me->SetHomePosition(current->fX, current->fY, current->fZ, 0);
        
        for (uint8 i = 0; i < 4; i++)
        {
            Creature* member = me->GetCreature(members[i]);
            if (!member || !member->isAlive())
                continue;
            float dx = x + cos(pathangle) * (i*5.0f + 1.0f);
            float dy = y + sin(pathangle) * (i*5.0f + 1.0f);
            float dz = z;
            Hellground::NormalizeMapCoord(dx);
            Hellground::NormalizeMapCoord(dy);
            member->UpdateGroundPositionZ(dx, dy, dz);

            if (member->IsWithinDist(me, 30.0f))
            {
                member->SetUnitMovementFlags(me->GetUnitMovementFlags());
            }
            else
            {
                me->GetMap()->CreatureRelocation(member, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f);
            }

            member->GetMotionMaster()->MovePoint(0, dx, dy, dz, true, true, UNIT_ACTION_HOME);
            member->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), pathangle);
        }
    }

    void MovementInform(uint32 uiMoveType, uint32 uiPointId)
    {
        if (uiMoveType != POINT_MOTION_TYPE)
            return;
        switch (current->uiPointId)
        {
        case WAYPOINT_NORTH_STOP:
        {
            Creature* member;
            for (uint8 i = 0; i < 4; i++)
            {
                member = me->GetCreature(members[i]);
                if (member)
                    member->Relocate(caravan_pos[i][0], caravan_pos[i][1], caravan_pos[i][2], caravan_pos[i][3]);
            }

            member = me->SummonCreature(NPC_VENDOR_TRON, caravan_pos[4][0], caravan_pos[4][1], caravan_pos[4][2], caravan_pos[4][3], TEMPSUMMON_CORPSE_DESPAWN, 1000);
            if (member)
                members[4] = member->GetGUID();
            SendDebug("reached north stop waypoint, 10 minute break");
            pointWait.Reset(600000);
            break;
        }
        case WAYPOINT_NORTH_QUEST:
            if (Creature* cork = me->GetCreature(members[2]))
            {
                cork->YellToZone(GIZELTON_TEXT_BEGIN - 1, 0, 0);
                cork->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            }
            SendDebug("reached north quest waypoint, 3 minute break");
            pointWait.Reset(180000);
            me->SetWalk(true);
            break;
        case WAYPOINT_NORTH_WAVE1:
            pointWait.Reset(3000);
            if (!playerGUIDs.empty())
            {
                if (Creature* cork = me->GetCreature(members[2]))
                    DoScriptText(GIZELTON_TEXT_BEGIN - 2, cork);
                SendDebug("north quest wave 1");
                MakeWave(true);
            }
            break;
        case WAYPOINT_NORTH_WAVE2:
            pointWait.Reset(3000);
            if (!playerGUIDs.empty())
            {
                if (Creature* cork = me->GetCreature(members[2]))
                    DoScriptText(GIZELTON_TEXT_BEGIN - 3, cork);
                SendDebug("north quest wave 2");
                MakeWave(true);
            }
            break;
        case WAYPOINT_NORTH_WAVE3:
            pointWait.Reset(3000);
            if (!playerGUIDs.empty())
            {
                if (Creature* cork = me->GetCreature(members[2]))
                    DoScriptText(GIZELTON_TEXT_BEGIN - 4, cork);
                SendDebug("north quest wave 3");
                MakeWave(true);
            }
            break;
        case WAYPOINT_NORTH_FINISH:
            SendDebug("north quest travel complete");
            pointWait.Reset(10);
            me->SetWalk(false);
            if (!playerGUIDs.empty())
            {
                if (Creature* cork = me->GetCreature(members[2]))
                    DoScriptText(GIZELTON_TEXT_BEGIN - 5, cork);
                for (uint8 i = 0; i < 4; i++)
                {
                    Creature* member = me->GetCreature(members[i]);
                    if (member)
                        member->RestoreFaction();
                }
                for (std::list<uint64>::iterator itr = playerGUIDs.begin(); itr != playerGUIDs.end(); itr++)
                {
                    Player* plr = me->GetPlayer(*itr);
                    if (plr && plr->IsAtGroupRewardDistance(me))
                        plr->AreaExploredOrEventHappens(QUEST_BODYGUARD_FOR_HIRE);
                }
                playerGUIDs.clear();
            }
            break;
        case WAYPOINT_SOUTH_STOP:
        {
            Creature* member;
            for (uint8 i = 0; i < 4; i++)
            {
                member = me->GetCreature(members[i]);
                if (member)
                    member->Relocate(caravan_pos[5 + i][0], caravan_pos[5 + i][1], caravan_pos[5 + i][2], caravan_pos[5 + i][3]);
            }

            member = me->SummonCreature(NPC_SUPER_SELLER, caravan_pos[9][0], caravan_pos[9][1], caravan_pos[9][2], caravan_pos[9][3], TEMPSUMMON_CORPSE_DESPAWN, 1000);
            if (member)
                members[4] = member->GetGUID();
            SendDebug("reached south stop waypoint, 10 minute break");
            pointWait.Reset(600000);
            break;
        }
        case WAYPOINT_SOUTH_QUEST:
            if (Creature* rigger = me->GetCreature(members[0]))
            {
                rigger->YellToZone(GIZELTON_TEXT_BEGIN - 7, 0, 0);
                rigger->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            }
            SendDebug("reached south quest waypoint, 3 minute break");
            pointWait.Reset(180000);
            me->SetWalk(true);
            break;
        case WAYPOINT_SOUTH_WAVE1:
            pointWait.Reset(3000);
            if (!playerGUIDs.empty())
            {
                if (Creature* rigger = me->GetCreature(members[0]))
                    DoScriptText(GIZELTON_TEXT_BEGIN - 8, rigger);
                SendDebug("south quest wave 1");
                MakeWave(false);
            }
            break;
        case WAYPOINT_SOUTH_WAVE2:
            pointWait.Reset(3000);
            if (!playerGUIDs.empty())
            {
                if (Creature* rigger = me->GetCreature(members[0]))
                    DoScriptText(GIZELTON_TEXT_BEGIN - 9, rigger);
                SendDebug("south quest wave 2");
                MakeWave(false);
            }
            break;
        case WAYPOINT_SOUTH_WAVE3:
            pointWait.Reset(3000);
            if (!playerGUIDs.empty())
            {
                if (Creature* rigger = me->GetCreature(members[0]))
                    DoScriptText(GIZELTON_TEXT_BEGIN - 9, rigger);
                SendDebug("south quest wave 3");
                MakeWave(false);
            }
            break;
        case WAYPOINT_SOUTH_FINISH:
            SendDebug("south quest travel complete");
            pointWait.Reset(10);
            me->SetWalk(false);
            if (!playerGUIDs.empty())
            {
                if (Creature* rigger = me->GetCreature(members[0]))
                    DoScriptText(GIZELTON_TEXT_BEGIN - 10, rigger);
                for (uint8 i = 0; i < 4; i++)
                {
                    Creature* member = me->GetCreature(members[i]);
                    if (member)
                        member->RestoreFaction();
                }

                for (std::list<uint64>::iterator itr = playerGUIDs.begin(); itr != playerGUIDs.end(); itr++)
                {
                    Player* plr = me->GetPlayer(*itr);
                    if (plr && plr->IsAtGroupRewardDistance(me))
                        plr->AreaExploredOrEventHappens(QUEST_GIZELTON_CARAVAN);
                }
                playerGUIDs.clear();
            }
            break;
        
        default:
            pointWait.Reset(10);
            break;
        }
    }

    void QuestAccepted(Player* plr)
    {
        if (current->uiPointId != WAYPOINT_NORTH_QUEST && current->uiPointId != WAYPOINT_SOUTH_QUEST)
            return;

        if (Group *pGroup = plr->GetGroup())
        {
            for (GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                if (Player* pGroupGuy = itr->getSource())
                    playerGUIDs.push_back(pGroupGuy->GetGUID());
            }
        }
        else
            playerGUIDs.push_back(plr->GetGUID());
    }

    void MakeWave(bool north)
    {
        Creature* rigger = me->GetCreature(members[0]);
        if (!rigger)
            return;
        Position pos;
        for (uint8 i = 0; i < 4; i++)
        {
            uint32 id;
            switch (i)
            {
            case 0:
            case 1: id = north ? NPC_KOLKAR_AMBUSHER : NPC_NETHER_SORCERES; break;
            case 2: id = north ? NPC_KOLKAR_WAYLAYER : NPC_DOOMWARDER; break;
            case 3: id = north ? NPC_KOLKAR_WAYLAYER : NPC_LESSER_INFERNAL; break;
            }
            me->GetValidPointInAngle(pos, frand(15, 25), frand(-0.5, +0.5), true);
            if (Creature* enemy = me->SummonCreature(id, pos.x, pos.y, pos.z, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000))
                enemy->CombatStart(rigger);
        }
    }

    void EnteringCombat(Unit* enemy)
    {
        for (uint8 i = 0; i < 4; i++)
        {
            if (Creature* member = me->GetCreature(members[i]))
                member->CombatStart(enemy);
        }
    }
};

CreatureAI* GetAI_npc_gizelton_caravan(Creature* c)
{
    return new npc_gizelton_caravanAI(c);
}

bool QuestAccept_npc_gizelton_caravan_member(Player* plr, Creature* cre, const Quest* quest)
{
    Creature* caravan = plr->GetMap()->GetCreatureById(NPC_CARAVAN);
    if (caravan)
        CAST_AI(npc_gizelton_caravanAI, caravan->AI())->QuestAccepted(plr);
    return true;
}

bool GossipHello_npc_gizelton_caravan_member(Player* plr, Creature* cre)
{
    if (cre->isQuestGiver()) // disable/enable showing quest by seting npc flag
        plr->PrepareQuestMenu(cre->GetGUID());

    plr->SEND_GOSSIP_MENU(cre->GetNpcTextId(), cre->GetGUID());
    return true;
}

struct npc_gizelton_caravan_memberAI : public ScriptedAI
{
    npc_gizelton_caravan_memberAI(Creature* c) : ScriptedAI(c) {}

    void EnterCombat(Unit* enemy)
    {
        Creature* caravan = me->GetMap()->GetCreatureById(NPC_CARAVAN);
        if (caravan)
            CAST_AI(npc_gizelton_caravanAI, caravan->AI())->EnteringCombat(enemy);
    }
};

CreatureAI* GetAI_npc_gizelton_caravan_member(Creature* cre)
{
    return new npc_gizelton_caravan_memberAI(cre);
}

void AddSC_desolace()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_aged_dying_ancient_kodo";
    newscript->GetAI = &GetAI_npc_aged_dying_ancient_kodo;
    newscript->pGossipHello = &GossipHello_npc_aged_dying_ancient_kodo;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_iruxos";
    newscript->pGOUse = &GOUse_go_iruxos;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_dalinda";
    newscript->GetAI = &GetAI_npc_dalinda;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_dalinda;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_melizza_brimbuzzle";
    newscript->GetAI = &GetAI_npc_melizza_brimbuzzle;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_melizza_brimbuzzle;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name="npc_rokaro";
    newscript->pGossipHello = &GossipHello_npc_rokaro;
    newscript->pGossipSelect = &GossipSelect_npc_rokaro;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_magram_spectre";
    newscript->GetAI = &GetAI_npc_magram_spectre;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_gizelton_caravan";
    newscript->GetAI = &GetAI_npc_gizelton_caravan;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_gizelton_caravan_member";
    newscript->GetAI = &GetAI_npc_gizelton_caravan_member;
    newscript->pGossipHello = &GossipHello_npc_gizelton_caravan_member;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_gizelton_caravan_member;
    newscript->RegisterSelf();
}
