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

#include "Object.h"
#include "Player.h"
#include "BattleGround.h"
#include "BattleGroundMgr.h"
#include "Creature.h"
#include "MapManager.h"
#include "Language.h"
#include "Chat.h"
#include "SpellAuras.h"
#include "ArenaTeam.h"
#include "World.h"
#include "Group.h"
#include "ObjectMgr.h"
#include "WorldPacket.h"
#include "Util.h"
#include "BattleGroundNA.h"

BattleGround::BattleGround()
{
    m_TypeID            = BattleGroundTypeId(0);
    m_BracketId         = MAX_BATTLEGROUND_BRACKETS;        // use as mark bg template
    m_InstanceID        = 0;
    m_Status            = 0;
    m_EndTime           = 0;
    m_LastResurrectTime = 0;
    m_Queue_type        = MAX_BATTLEGROUND_QUEUES;
    m_InvitedAlliance   = 0;
    m_InvitedHorde      = 0;
    m_ArenaType         = 0;
    m_IsArena           = false;
    m_Winner            = 2;
    m_StartTime         = 0;
    m_Events            = 0;
    m_IsRated           = false;
    m_BuffChange        = false;
    m_Name              = "";
    m_LevelMin          = 0;
    m_LevelMax          = 0;
    m_InBGFreeSlotQueue = false;
    m_SetDeleteThis     = false;

    m_MaxPlayersPerTeam = 0;
    m_MaxPlayers        = 0;
    m_MinPlayersPerTeam = 0;
    m_MinPlayers        = 0;

    m_MapId             = 0;
    m_Map               = NULL;

    m_TeamStartLocX[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocX[BG_TEAM_HORDE]      = 0;

    m_TeamStartLocY[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocY[BG_TEAM_HORDE]      = 0;

    m_TeamStartLocZ[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocZ[BG_TEAM_HORDE]      = 0;

    m_TeamStartLocO[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocO[BG_TEAM_HORDE]      = 0;

    m_ArenaTeamIds[BG_TEAM_ALLIANCE]   = 0;
    m_ArenaTeamIds[BG_TEAM_HORDE]      = 0;

    m_ArenaTeamRatingChanges[BG_TEAM_ALLIANCE]   = 0;
    m_ArenaTeamRatingChanges[BG_TEAM_HORDE]      = 0;

    m_BgRaids[BG_TEAM_ALLIANCE]         = NULL;
    m_BgRaids[BG_TEAM_HORDE]            = NULL;

    m_PlayersCount[BG_TEAM_ALLIANCE]    = 0;
    m_PlayersCount[BG_TEAM_HORDE]       = 0;

    m_guidsReady[BG_TEAM_ALLIANCE].clear();
    m_guidsReady[BG_TEAM_HORDE].clear();

    m_PrematureCountDown = false;
    m_PrematureCountDown = 0;
    m_TimeElapsedSinceBeggining = 0;

    m_HonorMode = BG_NORMAL;

    m_progressStart = 0;
}

BattleGround::~BattleGround()
{
    // remove objects and creatures
    // (this is done automatically in mapmanager update, when the instance is reset after the reset time)
    int size = m_BgCreatures.size();
    for (int i = 0; i < size; ++i)
    {
        DelCreature(i);
    }
    size = m_BgObjects.size();
    for (int i = 0; i < size; ++i)
    {
        DelObject(i);
    }

    if (GetInstanceID())
    {
        // delete creature and go respawn times
        RealmDataDatabase.PExecute("DELETE FROM creature_respawn WHERE instance = '%u'", GetInstanceID());
        RealmDataDatabase.PExecute("DELETE FROM gameobject_respawn WHERE instance = '%u'", GetInstanceID());
        // delete instance from db
        RealmDataDatabase.PExecute("DELETE FROM instance WHERE id = '%u'", GetInstanceID());
    }
    // remove from battlegrounds
    sBattleGroundMgr.RemoveBattleGround(GetInstanceID(), GetTypeID());
    // unload map
    Map * map = GetMap();
    if (map && map->IsBattleGroundOrArena())
    {
        ((BattleGroundMap*)map)->SetBattleGround(NULL);
        ((BattleGroundMap*)map)->SetUnload();
    }

    // remove from bg free slot queue
    this->RemoveFromBGFreeSlotQueue();

    for (BattleGroundScoreMap::iterator itr = m_PlayerScores.begin(); itr != m_PlayerScores.end(); itr++)
    {
        delete itr->second;
    }
}

void BattleGround::Update(uint32 diff)
{
    if (!GetPlayersSize() && !GetRemovedPlayersSize() && !GetReviveQueueSize())
        //BG is empty
        return;

    m_StartTime += diff;
    m_TimeElapsedSinceBeggining += diff;

    if (GetRemovedPlayersSize())
    {
        for (std::map<uint64, uint8>::iterator itr = m_RemovedPlayers.begin(); itr != m_RemovedPlayers.end(); ++itr)
        {
            Player *plr = sObjectMgr.GetPlayer(itr->first);
            switch (itr->second)
            {
                case 1:                                     // currently in bg and was removed from bg
                    if (plr)
                        RemovePlayerAtLeave(itr->first, true, true);
                    else
                        RemovePlayerAtLeave(itr->first, false, false);
                    break;
                case 2:                                     // revive queue
                    RemovePlayerFromResurrectQueue(itr->first);
                    break;
                default:
                    sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround: Unknown remove player case!");
            }
        }
        m_RemovedPlayers.clear();
    }

    // remove offline players from bg after 5 minutes
    if (GetPlayersSize())
    {
        for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        {
            Player *plr = sObjectMgr.GetPlayer(itr->first);
            itr->second.LastOnlineTime += diff;

            if (plr)
                itr->second.LastOnlineTime = 0;                 // update last online time
            else
                if (itr->second.LastOnlineTime >= MAX_OFFLINE_TIME)
                    m_RemovedPlayers[itr->first] = 1;           // add to remove list (BG)
        }
    }

    m_LastResurrectTime += diff;
    if (m_LastResurrectTime >= RESURRECTION_INTERVAL)
    {
        if (GetReviveQueueSize())
        {
            for (std::map<uint64, std::vector<uint64> >::iterator itr = m_ReviveQueue.begin(); itr != m_ReviveQueue.end(); ++itr)
            {
                Creature *sh = NULL;
                for (std::vector<uint64>::iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); ++itr2)
                {
                    Player *plr = sObjectMgr.GetPlayer(*itr2);
                    if (!plr)
                        continue;

                    if (!sh && plr->IsInWorld())
                    {
                        sh = plr->GetMap()->GetCreature(itr->first);
                        // only for visual effect
                        if (sh)
                            sh->CastSpell(sh, SPELL_SPIRIT_HEAL, true);   // Spirit Heal, effect 117
                    }

                    plr->CastSpell(plr, SPELL_RESURRECTION_VISUAL, true);   // Resurrection visual

                    RestorePet(plr);

                    m_ResurrectQueue.push_back(*itr2);
                }
                (itr->second).clear();
            }

            m_ReviveQueue.clear();
            m_LastResurrectTime = 0;
        }
        else
            // queue is clear and time passed, just update last resurrection time
            m_LastResurrectTime = 0;
    }
    else if (m_LastResurrectTime > 500)    // Resurrect players only half a second later, to see spirit heal effect on NPC
    {
        for (std::vector<uint64>::iterator itr = m_ResurrectQueue.begin(); itr != m_ResurrectQueue.end(); ++itr)
        {
            Player *plr = sObjectMgr.GetPlayer(*itr);
            if (!plr)
                continue;
            plr->ResurrectPlayer(1.0f);

            RestorePet(plr);
            plr->CastSpell(plr, SPELL_SPIRIT_HEAL_MANA, true);
            sObjectAccessor.ConvertCorpseForPlayer(*itr);
        }
        m_ResurrectQueue.clear();
    }

    // if less then minimum players are in on one side, then start premature finish timer
    if (GetStatus() == STATUS_IN_PROGRESS && !isArena() && sBattleGroundMgr.GetPrematureFinishTime() && (GetPlayersCountByTeam(ALLIANCE) < GetMinPlayersPerTeam() || GetPlayersCountByTeam(HORDE) < GetMinPlayersPerTeam()))
    {
        if (!m_PrematureCountDown)
        {
            m_PrematureCountDown = true;
            m_PrematureCountDownTimer = sBattleGroundMgr.GetPrematureFinishTime();
            if( sBattleGroundMgr.IsPrematureFinishTimerEnabled())
                PrepareMessageToAll("Not enough players. This battleground will close in %u min.",m_PrematureCountDownTimer / 60000);
            else
                SendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING);
        }
        else if (m_PrematureCountDownTimer <= diff)
        {
            // time's up!
            EndBattleGround(0); // noone wins
            m_PrematureCountDown = false;
        }
        else
        {
            uint32 newtime = m_PrematureCountDownTimer - diff;
            // announce every minute
            if (m_PrematureCountDownTimer != sBattleGroundMgr.GetPrematureFinishTime() && newtime / 60000 != m_PrematureCountDownTimer / 60000)
                if( sBattleGroundMgr.IsPrematureFinishTimerEnabled())
                    PrepareMessageToAll("Not enough players. This battleground will close in %u min.",m_PrematureCountDownTimer / 60000);
                else
                    SendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING);
            m_PrematureCountDownTimer = newtime;
        }
    }
    else if (m_PrematureCountDown)
        m_PrematureCountDown = false;

    if (GetStatus() == STATUS_WAIT_LEAVE)
    {
        // remove all players from battleground after 2 minutes
        m_EndTime += diff;
        if (m_EndTime >= TIME_TO_AUTOREMOVE)                 // 2 minutes
        {
            for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
            {
                m_RemovedPlayers[itr->first] = 1;           // add to remove list (BG)
            }
            // do not change any battleground's private variables
        }
    }

    if (isArena() && GetStatus() == STATUS_IN_PROGRESS)
    {
        if (GetMapId() == 559 && BG_NA_DOOR_DESPAWN_TIMER < (time(NULL) - m_progressStart))
        {
            for (uint32 i = BG_NA_OBJECT_DOOR_1; i <= BG_NA_OBJECT_DOOR_2; i++)
                DelObject(i, false);
        }
    }

    if (isArena() && sBattleGroundMgr.GetArenaEndAfterTime() && m_TimeElapsedSinceBeggining > sBattleGroundMgr.GetArenaEndAfterTime() && GetStatus() == STATUS_IN_PROGRESS)
    {
        if (!sBattleGroundMgr.IsArenaEndAfterAlwaysDraw())
        {
            if(GetAlivePlayersCountByTeam(HORDE) > GetAlivePlayersCountByTeam(ALLIANCE))
            {
                EndBattleGround(HORDE);
                return;
            }
            else if (GetAlivePlayersCountByTeam(HORDE) < GetAlivePlayersCountByTeam(ALLIANCE))
            {
                EndBattleGround(ALLIANCE);
                return;
            }
        }

        EndBattleGround(0);
        return;
    }
}

void BattleGround::RestorePet(Player* plr)
{
    if ((plr->GetClass() != CLASS_HUNTER && plr->GetClass() != CLASS_WARLOCK) || !plr->IsAlive())
        return;

    Pet* ThePet;
    if (plr->GetClass() == CLASS_HUNTER)
    {
        ThePet = new Pet();
        if (!ThePet->LoadPetFromDB(plr,0,0,false))
            return;

        if (ThePet->IsDead())
        {
            ThePet->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
            ThePet->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
            ThePet->setDeathState(ALIVE);
            ThePet->ClearUnitState(UNIT_STAT_ALL_STATE);
        }

        ThePet->SetHealth(ThePet->GetMaxHealth());
        ThePet->SetPower(POWER_HAPPINESS, ThePet->GetMaxPower(POWER_HAPPINESS));
        ThePet->RemoveAllAurasButPermanent();
        plr->DelayedPetSpellInitialize();
    }
    else if (plr->GetLastPetNumber()) // we're sure it is warlock
    {
        ThePet = new Pet();
        if (!ThePet->LoadPetFromDB(plr, 0, plr->GetLastPetNumber(), true))
            delete ThePet;
        else
        {
            // nerf doomguard/infernal
            if (ThePet->GetEntry() == 11859 || ThePet->GetEntry() == 89)
                ThePet->SetEntry(416);

            ThePet->SetHealth(ThePet->GetMaxHealth());
            ThePet->SetPower(POWER_MANA, ThePet->GetMaxPower(POWER_MANA));
        }
    }
}

Map* BattleGround::GetMap()
{
    return m_Map ? m_Map : m_Map = sMapMgr.FindMap(GetMapId(),GetInstanceID());
}

void BattleGround::SetTeamStartLoc(uint32 TeamID, float X, float Y, float Z, float O)
{
    BattleGroundTeamId idx = GetTeamIndexByTeamId(TeamID);
    m_TeamStartLocX[idx] = X;
    m_TeamStartLocY[idx] = Y;
    m_TeamStartLocZ[idx] = Z;
    m_TeamStartLocO[idx] = O;
}

void BattleGround::SendPacketToAll(WorldPacket *packet)
{
    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr.GetPlayer(itr->first);
        if (plr)
            plr->SendPacketToSelf(packet);
        else
            sLog.outDebug("BattleGround: Player " UI64FMTD " not found!", itr->first);
    }
}

void BattleGround::SendPacketToTeam(uint32 TeamID, WorldPacket *packet, Player *sender, bool self)
{
    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr.GetPlayer(itr->first);

        if (!plr)
        {
            sLog.outDebug("BattleGround: Player " UI64FMTD " not found!", itr->first);
            continue;
        }

        if (!self && sender == plr)
            continue;

        uint32 team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if (!team) team = plr->GetTeam();

        if (team == TeamID)
            plr->SendPacketToSelf(packet);
    }
}

void BattleGround::PlaySoundToAll(uint32 SoundID)
{
    WorldPacket data;
    sBattleGroundMgr.BuildPlaySoundPacket(&data, SoundID);
    SendPacketToAll(&data);
}

void BattleGround::PlaySoundToTeam(uint32 SoundID, uint32 TeamID)
{
    WorldPacket data;

    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr.GetPlayer(itr->first);

        if (!plr)
        {
            sLog.outDebug("BattleGround: Player " UI64FMTD " not found!", itr->first);
            continue;
        }

        uint32 team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if (!team) team = plr->GetTeam();

        if (team == TeamID)
        {
            sBattleGroundMgr.BuildPlaySoundPacket(&data, SoundID);
            plr->SendPacketToSelf(&data);
        }
    }
}

void BattleGround::CastSpellOnTeam(uint32 SpellID, uint32 TeamID)
{
    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr.GetPlayer(itr->first);

        if (!plr)
        {
            sLog.outDebug("BattleGround: Player " UI64FMTD " not found!", itr->first);
            continue;
        }

        uint32 team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if (!team) team = plr->GetTeam();

        if (team == TeamID)
            plr->CastSpell(plr, SpellID, true);
    }
}

void BattleGround::YellToAll(Creature* creature, const char* text, uint32 language)
{
    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        WorldPacket data(SMSG_MESSAGECHAT, 200);
        Player *plr = sObjectMgr.GetPlayer(itr->first);
        if (!plr)
        {
            sLog.outDebug("BattleGround: Player " UI64FMTD " not found!", itr->first);
            continue;
        }
        creature->BuildMonsterChat(&data,CHAT_MSG_MONSTER_YELL,text,language,creature->GetName(),itr->first);
        plr->SendPacketToSelf(&data);
    }
}

void BattleGround::RewardHonorToTeam(uint32 Honor, uint32 TeamID)
{
    Honor *= sWorld.getConfig(RATE_HONOR);
    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr.GetPlayer(itr->first);

        if (!plr)
        {
            sLog.outDebug("BattleGround: Player " UI64FMTD " not found!", itr->first);
            continue;
        }

        uint32 team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if (!team) team = plr->GetTeam();

        if (team == TeamID)
            UpdatePlayerScore(plr, SCORE_BONUS_HONOR, Honor);
    }
}

void BattleGround::RewardReputationToTeam(uint32 faction_id, uint32 Reputation, uint32 TeamID)
{
    FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction_id);

    if (!factionEntry)
        return;

    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr.GetPlayer(itr->first);

        if (!plr)
        {
            sLog.outDebug("BattleGround: Player " UI64FMTD " not found!", itr->first);
            continue;
        }

        uint32 team = itr->second.Team;
        if (!team) team = plr->GetTeam();

        if (team == TeamID && team == plr->GetTeam())
        {
            int32 rep = plr->CalculateReputationGain(REPUTATION_SOURCE_BG, Reputation, faction_id);
            plr->GetReputationMgr().ModifyReputation(factionEntry, Reputation);
            plr->UpdateBgTitle();
        }
    }
}

void BattleGround::UpdateWorldState(uint32 Field, uint32 Value)
{
    WorldPacket data;
    sBattleGroundMgr.BuildUpdateWorldStatePacket(&data, Field, Value);
    SendPacketToAll(&data);
}

void BattleGround::UpdateWorldStateForPlayer(uint32 Field, uint32 Value, Player *Source)
{
    WorldPacket data;
    sBattleGroundMgr.BuildUpdateWorldStatePacket(&data, Field, Value);
    Source->SendPacketToSelf(&data);
}

void BattleGround::EndBattleGround(uint32 winner)
{
    this->RemoveFromBGFreeSlotQueue();
    uint32 almost_winning_team = HORDE;
    ArenaTeam * winner_arena_team = NULL;
    ArenaTeam * loser_arena_team = NULL;
    uint32 loser_rating = 0;
    uint32 loser_hidden_rating = 0;
    uint32 winner_rating = 0;
    uint32 winner_hidden_rating = 0;
    WorldPacket data;
    Player *Source = NULL;
    const char *winmsg = "";

    bool hiddenEnabled = sWorld.getConfig(CONFIG_ENABLE_HIDDEN_RATING);

    if (winner == ALLIANCE)
    {
        if (isBattleGround())
            winmsg = GetMangosString(LANG_BG_A_WINS);
        else
            winmsg = GetMangosString(LANG_ARENA_GOLD_WINS);

        PlaySoundToAll(SOUND_ALLIANCE_WINS);                // alliance wins sound

        SetWinner(WINNER_ALLIANCE);
    }
    else if (winner == HORDE)
    {
        if (isBattleGround())
            winmsg = GetMangosString(LANG_BG_H_WINS);
        else
            winmsg = GetMangosString(LANG_ARENA_GREEN_WINS);

        PlaySoundToAll(SOUND_HORDE_WINS);                   // horde wins sound

        SetWinner(WINNER_HORDE);
    }
    else
    {
        SetWinner(3);
    }
    if (!isArena())
        sLog.outLog(LOG_SPECIAL,"Battleground %u ended, winner %s", GetTypeID(), winner == ALLIANCE ? "Alliance" : (winner == HORDE ? "Horde" : "None"));

    SetStatus(STATUS_WAIT_LEAVE);
    m_EndTime = 0;

    if (isArena() && !isRated())
    {
        std::stringstream str;
        str << "skirmish battle ended, winner team" << winner << " players: ";
        for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        {
            str << itr->first << "(" << (itr->second.Team) << ") ";
        }
        sLog.outLog(LOG_SPECIAL, str.str().c_str());
    }

    // arena rating calculation
    if (isArena() && isRated() && !sWorld.IsStopped())
    {
        if (winner == ALLIANCE)
        {
            winner_arena_team = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(ALLIANCE));
            loser_arena_team = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(HORDE));
        }
        else if (winner == HORDE)
        {
            winner_arena_team = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(HORDE));
            loser_arena_team = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(ALLIANCE));
        }
        if (winner_arena_team && loser_arena_team)
        {
            loser_rating = loser_arena_team->GetStats().rating;
            winner_rating = winner_arena_team->GetStats().rating;

            int32 winner_change = winner_arena_team->WonAgainst(loser_rating);
            int32 loser_change = loser_arena_team->LostAgainst(winner_rating);

            if (hiddenEnabled && sWorld.getConfig(CONFIG_ENABLE_HIDDEN_RATING_LOWER_LOSS))
            {
                if (winner_rating + 150 < winner_hidden_rating)
                    loser_change /= 2;
            }

            sLog.outDebug("--- Winner rating: %u, Loser rating: %u, Winner change: %u, Losser change: %u ---", winner_rating, loser_rating, winner_change, loser_change);

            if (winner == ALLIANCE)
            {
                SetArenaTeamRatingChangeForTeam(ALLIANCE, winner_change);
                SetArenaTeamRatingChangeForTeam(HORDE, loser_change);
            }
            else
            {
                SetArenaTeamRatingChangeForTeam(HORDE, winner_change);
                SetArenaTeamRatingChangeForTeam(ALLIANCE, loser_change);
            }

            sLog.outLog(LOG_ARENA, "Arena match Type: %u for Team1Id: %u - Team2Id: %u ended. WinnerTeamId: %u. Winner rating: %u, Loser rating: %u. RatingChange: %i.", m_ArenaType, m_ArenaTeamIds[BG_TEAM_ALLIANCE], m_ArenaTeamIds[BG_TEAM_HORDE], winner_arena_team->GetId(), winner_rating, loser_rating, winner_change);
            if (sWorld.getConfig(CONFIG_ARENA_LOG_EXTENDED_INFO))
                for (BattleGroundScoreMap::const_iterator itr = GetPlayerScoresBegin();itr !=GetPlayerScoresEnd(); ++itr)
                {
                    if (Player* player = sObjectMgr.GetPlayer(itr->first))
                        sLog.outLog(LOG_ARENA, "Statistics for %s (GUID: " UI64FMTD ", Team: %d, IP: %s): %u damage, %u healing, %u killing blows", player->GetName(), itr->first, player->GetArenaTeamId(m_ArenaType == 5 ? 2 : m_ArenaType == 3), player->GetSession()->GetRemoteAddress().c_str(), itr->second->DamageDone, itr->second->HealingDone, itr->second->KillingBlows);
                }

            if (sWorld.getConfig(CONFIG_ARENA_EXPORT_RESULTS))
            {
                RealmDataDatabase.PExecute("INSERT INTO arena_fights VALUES (%u, '%01u', %u, %u, %u, %u, %i, SYSDATE(), %u);",
                    GetInstanceID(), m_ArenaType, winner_arena_team->GetId(),loser_arena_team->GetId(), winner_rating, loser_rating, winner_change,uint32(m_TimeElapsedSinceBeggining/1000));
            }
        }
        else
        {
            SetArenaTeamRatingChangeForTeam(ALLIANCE, 0);
            SetArenaTeamRatingChangeForTeam(HORDE, 0);
        }
    }

    if (!isArena()){

    if (m_score[GetTeamIndexByTeamId(ALLIANCE)] == m_score[GetTeamIndexByTeamId(HORDE)])
        almost_winning_team = 0;         //no real winner
    if (m_score[GetTeamIndexByTeamId(ALLIANCE)] > m_score[GetTeamIndexByTeamId(HORDE)])
        almost_winning_team = ALLIANCE;

    }

    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr.GetPlayer(itr->first);
        if (!plr)
        {
            sLog.outDebug("BattleGround: Player " UI64FMTD " not found!", itr->first);
            continue;
        }

        // should remove spirit of redemption
        if (plr->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
            plr->RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT);

        if (isArena() && !plr->IsGameMaster())
        {
            plr->SetVisibility(VISIBILITY_ON);
            plr->SetFlying(false);

            plr->GetCamera().ResetView(true);
        }

        if (!plr->IsAlive())
        {
            plr->ResurrectPlayer(1.0f);
            plr->SpawnCorpseBones();
        }

        uint32 team = itr->second.Team;
        if (!team) team = plr->GetTeam();

        // per player calculation
        if (isArena() && isRated() && winner_arena_team && loser_arena_team && !sWorld.IsStopped())
        {
            uint32 persRating;
            int32 persDiff;
            loser_hidden_rating = GetArenaTeamMMRSum(GetOtherTeam(winner))/GetMaxPlayersPerTeam();
            winner_hidden_rating = GetArenaTeamMMRSum(winner)/GetMaxPlayersPerTeam();
            if (team == winner)
                winner_arena_team->MemberWon(plr, loser_rating, loser_hidden_rating,&persRating,&persDiff);
            else
                loser_arena_team->MemberLost(plr, winner_rating, winner_hidden_rating,&persRating,&persDiff);
            if (sWorld.getConfig(CONFIG_ARENA_EXPORT_RESULTS))
            {
                BattleGroundScore* score = m_PlayerScores[itr->first];
                RealmDataDatabase.PExecute("INSERT INTO arena_fights_detailed VALUES (%u, " UI64FMTD", %u, %u, %u, %u, %u, %i);",
                    GetInstanceID(), itr->first, (team == winner ? winner_arena_team->GetId() : loser_arena_team->GetId()),
                    score->DamageDone, score->HealingDone, score->KillingBlows,persRating,persDiff);
            }
        }

        if (team == winner)
        {
            if (!Source)
                Source = plr;
            RewardMark(plr,ITEM_WINNER_COUNT);
            RewardQuest(plr);
        }
        else if (winner !=0)
        {
            RewardMark(plr, ITEM_LOSER_COUNT);
        }
        else if (winner == 0)
        {
            if (sWorld.getConfig(CONFIG_PREMATURE_BG_REWARD))    // We're feeling generous, giving rewards to people who not earned them ;)
            {   //nested ifs for the win! its boring writing that, forgive me my unfunniness
                //if(almost_winning_team == team)                    //player's team had more points
                //    RewardMark(plr,ITEM_WINNER_COUNT);
                //else
                RewardMark(plr, ITEM_LOSER_COUNT);            // if scores were the same, each team gets 1 mark.
            }
        }

        plr->CombatStopWithPets(true);

        BlockMovement(plr);

        sBattleGroundMgr.BuildPvpLogDataPacket(&data, this);
        plr->SendPacketToSelf(&data);

        BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(GetTypeID(), GetArenaType());
        sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, team, plr->GetBattleGroundQueueIndex(bgQueueTypeId), STATUS_IN_PROGRESS, TIME_TO_AUTOREMOVE, GetStartTime());
        plr->SendPacketToSelf(&data);
    }

    if (isArena() && isRated() && winner_arena_team && loser_arena_team)
    {
        // update arena points only after increasing the player's match count!
        //obsolete: winner_arena_team->UpdateArenaPointsHelper();
        //obsolete: loser_arena_team->UpdateArenaPointsHelper();
        // save the stat changes
        winner_arena_team->SaveToDB();
        loser_arena_team->SaveToDB();
        // send updated arena team stats to players
        // this way all arena team members will get notified, not only the ones who participated in this match
        winner_arena_team->NotifyStatsChanged();
        loser_arena_team->NotifyStatsChanged();
    sLog.outDebug("Rated arena match between %s and %s finished, winner: %s", loser_arena_team->GetName().c_str(),winner_arena_team->GetName().c_str(),winner_arena_team->GetName().c_str());
    }

    // inform invited players about the removal
    sBattleGroundMgr.m_BattleGroundQueues[BattleGroundMgr::BGQueueTypeId(GetTypeID(), GetArenaType())].BGEndedRemoveInvites(this);

    if (Source)
    {
        ChatHandler(Source).FillMessageData(&data, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, Source->GetGUID(), winmsg);
        SendPacketToAll(&data);
    }
}

uint32 BattleGround::GetBattlemasterEntry() const
{
    switch (GetTypeID())
    {
        case BATTLEGROUND_AV: return 15972;
        case BATTLEGROUND_WS: return 14623;
        case BATTLEGROUND_AB: return 14879;
        case BATTLEGROUND_EY: return 22516;
        case BATTLEGROUND_NA: return 20200;
        default:              return 0;
    }
}

void BattleGround::RewardMark(Player *plr,uint32 count)
{
    // 'Inactive' this aura prevents the player from gaining honor points and battleground tokens
    if (plr->GetDummyAura(SPELL_AURA_PLAYER_INACTIVE))
        return;

    if (!plr || !count)
        return;

    BattleGroundMarks mark;
    switch (GetTypeID())
    {
        case BATTLEGROUND_AV:
            mark = ITEM_AV_MARK_OF_HONOR;
            break;
        case BATTLEGROUND_WS:
            mark = ITEM_WS_MARK_OF_HONOR;
            break;
        case BATTLEGROUND_AB:
            mark = ITEM_AB_MARK_OF_HONOR;
            break;
        case BATTLEGROUND_EY:
            mark = ITEM_EY_MARK_OF_HONOR;
            break;
        default:
            return;
    }

    if (ObjectMgr::GetItemPrototype(mark))
    {
        ItemPosCountVec dest;
        uint32 no_space_count = 0;
        uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, mark, count, &no_space_count);
        if (msg != EQUIP_ERR_OK)                       // convert to possible store amount
            count -= no_space_count;

        if (!dest.empty())                // can add some
            if (Item* item = plr->StoreNewItem(dest, mark, true, 0))
                plr->SendNewItem(item, count, true, false);

        if (no_space_count > 0)
        {
            if (plr->GetSession()->IsAccountFlagged(ACC_RESTRICT_BG_MARKS))
                ChatHandler(plr).SendSysMessage("This account has BG marks restriction enabled, you won't receive additional marks by mail. Use \".account bgmarks\" to disable");
            else
                SendRewardMarkByMail(plr, mark, no_space_count);
        }
    }
}

void BattleGround::SendRewardMarkByMail(Player *plr,uint32 mark, uint32 count)
{
    uint32 bmEntry = GetBattlemasterEntry();
    if (!bmEntry)
        return;

    ItemPrototype const* markProto = ObjectMgr::GetItemPrototype(mark);
    if (!markProto)
        return;

    if (Item* markItem = Item::CreateItem(mark,count,plr))
    {
        // save new item before send
        markItem->SaveToDB();                               // save for prevent lost at next mail load, if send fail then item will deleted

        // text
        std::string textFormat = plr->GetSession()->GetMangosString(LANG_BG_MARK_BY_MAIL);
        char textBuf[300];
        snprintf(textBuf,300,textFormat.c_str(),GetName(),GetName());
        uint32 itemTextId = sObjectMgr.CreateItemText(textBuf);

        MailDraft(markProto->Name1, itemTextId)
            .AddItem(markItem)
            .SendMailTo(plr, MailSender(MAIL_CREATURE, bmEntry));
    }
}

void BattleGround::RewardQuest(Player *plr)
{
    // 'Inactive' this aura prevents the player from gaining honor points and battleground tokens
    if (plr->GetDummyAura(SPELL_AURA_PLAYER_INACTIVE))
        return;

    uint32 quest;
    switch (GetTypeID())
    {
        case BATTLEGROUND_AV:
            quest = SPELL_AV_QUEST_REWARD;
            break;
        case BATTLEGROUND_WS:
            quest = SPELL_WS_QUEST_REWARD;
            break;
        case BATTLEGROUND_AB:
            quest = SPELL_AB_QUEST_REWARD;
            break;
        case BATTLEGROUND_EY:
            quest = SPELL_EY_QUEST_REWARD;
            break;
        default:
            return;
    }

    plr->CastSpell(plr, quest, true);
}

void BattleGround::BlockMovement(Player *plr)
{
    plr->SetClientControl(plr, false);                          // movement disabled NOTE: the effect will be automatically removed by client when the player is teleported from the battleground, so no need to send with uint8(1) in RemovePlayerAtLeave()
}

void BattleGround::RemovePlayerAtLeave(uint64 guid, bool Transport, bool SendPacket)
{
    uint32 team = GetPlayerTeam(guid);
    bool participant = false;
    // Remove from lists/maps
    BattleGroundPlayerMap::iterator itr = m_Players.find(guid);
    if (itr != m_Players.end())
    {
        UpdatePlayersCountByTeam(team, true);   // -1 player
        m_Players.erase(itr);
        // check if the player was a participant of the match, or only entered through gm command (goname)
        participant = true;

        if (isArena())
        {
            switch (GetArenaType())
            {
            case ARENA_TYPE_2v2:
            sBattleGroundMgr.inArenasCount[0]--; break;
            case ARENA_TYPE_3v3:
            sBattleGroundMgr.inArenasCount[1]--; break;
            case ARENA_TYPE_5v5:
            sBattleGroundMgr.inArenasCount[2]--; break;
            }
        }
    }

    if (isBattleGround())
    {
        std::map<uint64, BattleGroundScore*>::iterator itr2 = m_PlayerScores.find(guid);
        if (itr2 != m_PlayerScores.end())
        {
            BattleGroundScore *temp = itr2->second;  // delete player's score
            m_PlayerScores.erase(itr2);
            delete temp;
        }
    }

    RemovePlayerFromResurrectQueue(guid);

    Player *plr = sObjectMgr.GetPlayer(guid);

    // should remove spirit of redemption
    if (plr && plr->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
        plr->RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT);

    // should remove SPELL_AURA_MOD_STUN
    if (plr && plr->IsAlive() && plr->HasAuraType(SPELL_AURA_MOD_STUN))
        plr->RemoveSpellsCausingAura(SPELL_AURA_MOD_STUN);            //testfixbg

    if (plr && !plr->IsAlive())                              // resurrect on exit
    {
        plr->ResurrectPlayer(1.0f);
        plr->SpawnCorpseBones();
    }

    if (plr && participant && isArena() && isRated() && GetStatus() == STATUS_IN_PROGRESS && !sWorld.IsStopped())
    {
        //left a rated match while the encounter was in progress, consider as loser
        //need to be done before RemovePlayer which can cause EndBattleground and last removed player rating wont be updated
        if (!team) team = plr->GetTeam();
        ArenaTeam* winner_arena_team = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(team == HORDE? ALLIANCE : HORDE));
        ArenaTeam* loser_arena_team = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(team));
        uint32 persRating;
        int32 persDiff;
        if (winner_arena_team && loser_arena_team)
            loser_arena_team->MemberLost(plr, winner_arena_team->GetRating(),
            winner_arena_team->GetAverageMMR(GetBgRaid(team == HORDE ? ALLIANCE : HORDE)),&persRating,&persDiff);

        BattleGroundScore* score = m_PlayerScores[guid];
        sLog.outLog(LOG_ARENA, "[Early left] Statistics for %s (GUID: " UI64FMTD ", Team: %d, IP: %s): %u damage, %u healing, %u killing blows",
            plr->GetName(), guid, plr->GetArenaTeamId(m_ArenaType == 5 ? 2 : m_ArenaType == 3), plr->GetSession()->GetRemoteAddress().c_str(), score->DamageDone, score->HealingDone, score->KillingBlows);
        if (sWorld.getConfig(CONFIG_ARENA_EXPORT_RESULTS))
        {
            RealmDataDatabase.PExecute("INSERT INTO arena_fights_detailed VALUES (%u, " UI64FMTD", %u, %u, %u, %u, %u, %i);",
                GetInstanceID(), guid, loser_arena_team->GetId(),
                score->DamageDone, score->HealingDone, score->KillingBlows, persRating, persDiff);
        }
    }

    RemovePlayer(plr, guid);                                // BG subclass specific code
    DecreaseInvitedCount(team);

    if (plr)
    {
        plr->ClearAfkReports();

        if (participant) // if the player was a match participant, remove auras, calc rating, update queue
        {
            if (!team) team = plr->GetTeam();

            BattleGroundTypeId bgTypeId = GetTypeID();
            BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(GetTypeID(), GetArenaType());
            // if arena, remove the specific arena auras
            if (isArena())
            {
                plr->RemoveArenaAuras(true);    // removes debuffs / dots etc., we don't want the player to die after porting out
                plr->GetMotionMaster()->MovementExpired();

                bgTypeId=BATTLEGROUND_AA;       // set the bg type to all arenas (it will be used for queue refreshing)

                // summon old pet if there was one and there isn't a current pet
                if (!plr->GetPet() && plr->GetTemporaryUnsummonedPetNumber())
                {
                    Pet* NewPet = new Pet;
                    if (!NewPet->LoadPetFromDB(plr, 0, (plr)->GetTemporaryUnsummonedPetNumber(), true))
                        delete NewPet;

                    (plr)->SetTemporaryUnsummonedPetNumber(0);
                }
            }

            WorldPacket data;
            if (SendPacket)
            {
                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, team, plr->GetBattleGroundQueueIndex(bgQueueTypeId), STATUS_NONE, 0, 0);
                plr->SendPacketToSelf(&data);
            }

            // this call is important, because player, when joins to battleground, this method is not called, so it must be called when leaving bg
            plr->RemoveBattleGroundQueueId(bgQueueTypeId);

            //we should update battleground queue, but only if bg isn't ending
            if (isBattleGround() && GetStatus() < STATUS_WAIT_LEAVE)
                sBattleGroundMgr.ScheduleQueueUpdate(bgQueueTypeId, bgTypeId, GetBracketId());

            Group * group = plr->GetGroup();
            // remove from raid group if exist
            if (group && group == GetBgRaid(team))
            {
                if (!group->RemoveMember(guid, 0))               // group was disbanded
                {
                    SetBgRaid(team, NULL);
                    delete group;
                }
            }

            // Let others know
            sBattleGroundMgr.BuildPlayerLeftBattleGroundPacket(&data, plr);
            SendPacketToTeam(team, &data, plr, false);
        }

        // Do next only if found in battleground
        plr->SetBattleGroundId(0, BATTLEGROUND_TYPE_NONE);                          // We're not in BG.

        // reset destination bg team
        plr->SetBGTeam(0);
        plr->GetMotionMaster()->MovementExpired();

        if (Transport)
            plr->TeleportTo(plr->GetBattleGroundEntryPointMap(), plr->GetBattleGroundEntryPointX(), plr->GetBattleGroundEntryPointY(), plr->GetBattleGroundEntryPointZ(), plr->GetBattleGroundEntryPointO());

        // Log
        sLog.outDetail("BATTLEGROUND: Removed player %s from BattleGround.", plr->GetName());
    }

    if (!GetPlayersSize() && !GetInvitedCount(HORDE) && !GetInvitedCount(ALLIANCE))
    {
        // if no players left AND no invitees left, set this bg to delete in next update
        // direct deletion could cause crashes
        SetDeleteThis();
        // return to prevent addition to freeslotqueue
        return;
    }

    // a player exited the battleground, so there are free slots. add to queue
    this->AddToBGFreeSlotQueue();
}

// this method is called when no players remains in battleground
void BattleGround::Reset()
{
    SetQueueType(MAX_BATTLEGROUND_QUEUES);
    SetWinner(WINNER_NONE);
    SetStatus(STATUS_WAIT_QUEUE);
    SetStartTime(0);
    SetEndTime(0);
    SetLastResurrectTime(0);
    SetArenaType(0);
    SetRated(false);
    m_Map = NULL;

    m_Events = 0;

    if (m_InvitedAlliance > 0 || m_InvitedHorde > 0)
        sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround system ERROR: bad counter, m_InvitedAlliance: %d, m_InvitedHorde: %d", m_InvitedAlliance, m_InvitedHorde);

    m_InvitedAlliance = 0;
    m_InvitedHorde = 0;
    m_InBGFreeSlotQueue = false;

    for (BattleGroundScoreMap::iterator itr = m_PlayerScores.begin(); itr != m_PlayerScores.end(); itr++)
    {
        delete itr->second;
    }
    m_Players.clear();
    m_PlayerScores.clear();

    // reset BGSubclass
    ResetBGSubclass();
}

void BattleGround::StartBattleGround()
{
    ///this method should spawn spirit guides and so on
    SetStartTime(0);
    SetLastResurrectTime(0);
    AnnounceBGStart();
    if (m_IsRated)
        sLog.outLog(LOG_ARENA, "Arena match type: %u for Team1Id: %u - Team2Id: %u started.", m_ArenaType, m_ArenaTeamIds[BG_TEAM_ALLIANCE], m_ArenaTeamIds[BG_TEAM_HORDE]);
}

void BattleGround::AnnounceBGStart()
{
    if (!sWorld.getConfig(CONFIG_BATTLEGROUND_ANNOUNCE_START))
        return;

    std::stringstream ss;
    switch (m_TypeID)
    {
        case BATTLEGROUND_AV:
            ss << "Alterac Valley "; break;
        case BATTLEGROUND_WS:
            ss << "Warsong Gulch "; break;
        case BATTLEGROUND_EY:
            ss << "Eye of the Storm "; break;
        case BATTLEGROUND_AB:
            ss << "Arathi Basin "; break;
        default: return;
    }

    ss << "(#" << GetInstanceID() << ") started for levels: ";
    ss << m_BracketLevelMin;
    if (m_BracketLevelMin != sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
        ss << "-" << m_BracketLevelMax;

    sWorld.SendWorldTextForLevels(m_BracketLevelMin, sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL), ACC_DISABLED_BGANN, LANG_BG_START_ANNOUNCE, ss.str().c_str());
}

void BattleGround::AddPlayer(Player *plr)
{
    // remove afk from player
    if (plr->isAFK())
        plr->ToggleAFK();

    // score struct must be created in inherited class

    uint64 guid = plr->GetGUID();
    uint32 team = plr->GetBGTeam();

    BattleGroundPlayer bp;
    bp.LastOnlineTime = 0;
    bp.Team = team;

    // Add to list/maps
    m_Players[guid] = bp;

    UpdatePlayersCountByTeam(team, false);                  // +1 player

    WorldPacket data;
    sBattleGroundMgr.BuildPlayerJoinedBattleGroundPacket(&data, plr);
    SendPacketToTeam(team, &data, plr, false);

    plr->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
    plr->CombatStop();
    // should remove SPELL_AURA_MOD_STUN
    if (plr && plr->IsAlive() && plr->HasAuraType(SPELL_AURA_MOD_STUN))
        plr->RemoveSpellsCausingAura(SPELL_AURA_MOD_STUN);            //testfixbg

    // add arena specific auras
    if (isArena())
    {
        plr->RemoveArenaSpellCooldowns();
        plr->ClearComboPoints();
        plr->RemoveArenaAuras();
        plr->GetMotionMaster()->MoveIdle();

        plr->RemoveAllEnchantments(TEMP_ENCHANTMENT_SLOT, true);
        if (team == ALLIANCE)                                // gold
        {
            if (plr->GetTeam() == HORDE)
                plr->CastSpell(plr, SPELL_HORDE_GOLD_FLAG,true);
            else
                plr->CastSpell(plr, SPELL_ALLIANCE_GOLD_FLAG,true);
        }
        else                                                // green
        {
            if (plr->GetTeam() == HORDE)
                plr->CastSpell(plr, SPELL_HORDE_GREEN_FLAG,true);
            else
                plr->CastSpell(plr, SPELL_ALLIANCE_GREEN_FLAG,true);
        }

        plr->DestroyConjuredItems(true);

        Pet* pet = plr->GetPet();
        if (pet)
        {
            if (pet->getPetType() == SUMMON_PET || pet->getPetType() == HUNTER_PET)
            {
                (plr)->SetTemporaryUnsummonedPetNumber(pet->GetCharmInfo()->GetPetNumber());
                (plr)->SetOldPetSpell(pet->GetUInt32Value(UNIT_CREATED_BY_SPELL));
            }
            pet->RemoveArenaAuras();
            plr->RemovePet(NULL,PET_SAVE_NOT_IN_SLOT);
        }
        else
        {
            // Remove auras off unsummoned pet
            if (plr->GetTemporaryUnsummonedPetNumber())
                RealmDataDatabase.PExecute("DELETE FROM pet_aura WHERE guid = '%u'", plr->GetTemporaryUnsummonedPetNumber());

            plr->SetTemporaryUnsummonedPetNumber(0);
        }

        if (GetStatus() == STATUS_WAIT_JOIN)                 // not started yet
        {
            plr->CastSpell(plr, SPELL_ARENA_PREPARATION, true);

            plr->SetHealth(plr->GetMaxHealth());
            plr->SetPower(POWER_MANA, plr->GetMaxPower(POWER_MANA));

            if (plr->GetPower(POWER_RAGE))
                plr->SetPower(POWER_RAGE, 0);
        }

        switch (GetArenaType())
        {
        case ARENA_TYPE_2v2:
        sBattleGroundMgr.inArenasCount[0]++; break;
        case ARENA_TYPE_3v3:
        sBattleGroundMgr.inArenasCount[1]++; break;
        case ARENA_TYPE_5v5:
        sBattleGroundMgr.inArenasCount[2]++; break;
        }

        ChatHandler(plr).SendSysMessage("NOTICE: If you are ready, write: .arena ready. So, when everyone is ready, arena preparation will end earlier.");
    }
    else
    {
        if (GetStatus() == STATUS_WAIT_JOIN)                 // not started yet
            plr->CastSpell(plr, SPELL_PREPARATION, true);   // reduces all mana cost of spells.
    }

    plr->RemoveCharmAuras();
    plr->RemoveMovementImpairingAuras();

    AddOrSetPlayerToCorrectBgGroup(plr, guid, team);

    // Log
    sLog.outDetail("BATTLEGROUND: Player %s joined the battle.", plr->GetName());
}

void BattleGround::AddOrSetPlayerToCorrectBgGroup(Player *plr, uint64 plr_guid, uint32 team)
{
    if (Group* group = GetBgRaid(team))                     // raid already exist
    {
        if (group->IsMember(plr_guid))
        {
            uint8 subgroup = group->GetMemberGroup(plr_guid);
            plr->SetBattleGroundRaid(group, subgroup);
        }
        else
        {
            group->AddMember(plr_guid, plr->GetName());
            if (Group* originalGroup = plr->GetOriginalGroup())
                if (originalGroup->IsLeader(plr_guid))
                    group->ChangeLeader(plr_guid);
        }
    }
    else                                                    // first player joined
    {
        group = new Group;
        SetBgRaid(team, group);
        group->Create(plr_guid, plr->GetName());
    }
}
/* This method should be called only once ... it adds pointer to queue */
void BattleGround::AddToBGFreeSlotQueue()
{
    // make sure to add only once
    if (!m_InBGFreeSlotQueue && isBattleGround())
    {
        sBattleGroundMgr.BGFreeSlotQueue[m_TypeID].push_front(this);
        m_InBGFreeSlotQueue = true;
    }
}

/* This method removes this battleground from free queue - it must be called when deleting battleground - not used now*/
void BattleGround::RemoveFromBGFreeSlotQueue()
{
    // set to be able to re-add if needed
    m_InBGFreeSlotQueue = false;
    // uncomment this code when battlegrounds will work like instances
    for (BGFreeSlotQueueType::iterator itr = sBattleGroundMgr.BGFreeSlotQueue[m_TypeID].begin(); itr != sBattleGroundMgr.BGFreeSlotQueue[m_TypeID].end(); ++itr)
    {
        if ((*itr)->GetInstanceID() == GetInstanceID())
        {
            sBattleGroundMgr.BGFreeSlotQueue[m_TypeID].erase(itr);
            return;
        }
    }
}

// get the number of free slots for team
// returns the number how many players can join battleground to MaxPlayersPerTeam
uint32 BattleGround::GetFreeSlotsForTeam(uint32 Team) const
{
    //return free slot count to MaxPlayerPerTeam
    if (GetStatus() == STATUS_WAIT_JOIN || GetStatus() == STATUS_IN_PROGRESS)
        return (GetInvitedCount(Team) < GetMaxPlayersPerTeam()) ? GetMaxPlayersPerTeam() - GetInvitedCount(Team) : 0;

    return 0;
}

bool BattleGround::HasFreeSlots() const
{
    return GetPlayersSize() < GetMaxPlayers();
}

void BattleGround::UpdatePlayerScore(Player *Source, uint32 type, uint32 value)
{
    //this procedure is called from virtual function implemented in bg subclass
    std::map<uint64, BattleGroundScore*>::iterator itr = m_PlayerScores.find(Source->GetGUID());

    if (itr == m_PlayerScores.end())                         // player not found...
        return;

    switch (type)
    {
        case SCORE_KILLING_BLOWS:                           // Killing blows
            itr->second->KillingBlows += value;
            break;
        case SCORE_DEATHS:                                  // Deaths
            itr->second->Deaths += value;
            if (itr->second->Deaths >= 50)
                Source->ToUnit()->WorthHonor = true;
            break;
        case SCORE_HONORABLE_KILLS:                         // Honorable kills
            itr->second->HonorableKills += value;
            break;
        case SCORE_BONUS_HONOR:                             // Honor bonus
            // do not add honor in arenas
            if (isBattleGround())
            {
                // reward honor instantly
                if (Source->RewardHonor(NULL, 1, value))
                    itr->second->BonusHonor += value;
            }
            break;
            //used only in EY, but in MSG_PVP_LOG_DATA opcode
        case SCORE_DAMAGE_DONE:                             // Damage Done
            itr->second->DamageDone += value;
            break;
        case SCORE_HEALING_DONE:                            // Healing Done
            itr->second->HealingDone += value;
            break;
        default:
            sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround: Unknown player score type %u", type);
            break;
    }
}

void BattleGround::AddPlayerToResurrectQueue(uint64 npc_guid, uint64 player_guid)
{
    m_ReviveQueue[npc_guid].push_back(player_guid);

    Player *plr = sObjectMgr.GetPlayer(player_guid);
    if (!plr)
        return;

    plr->CastSpell(plr, SPELL_WAITING_FOR_RESURRECT, true);
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(SPELL_WAITING_FOR_RESURRECT);
    if (spellInfo)
    {
        Aura *Aur = CreateAura(spellInfo, 0, NULL, plr);
        plr->AddAura(Aur);
    }
}

void BattleGround::RemovePlayerFromResurrectQueue(uint64 player_guid)
{
    for (std::map<uint64, std::vector<uint64> >::iterator itr = m_ReviveQueue.begin(); itr != m_ReviveQueue.end(); ++itr)
    {
        for (std::vector<uint64>::iterator itr2 =(itr->second).begin(); itr2 != (itr->second).end(); ++itr2)
        {
            if (*itr2 == player_guid)
            {
                (itr->second).erase(itr2);

                Player *plr = sObjectMgr.GetPlayer(player_guid);
                if (!plr)
                    return;

                plr->RemoveAurasDueToSpell(SPELL_WAITING_FOR_RESURRECT);

                return;
            }
        }
    }
}

bool BattleGround::AddObject(uint32 type, uint32 entry, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime)
{
    Map * map = GetMap();
    if (!map)
        return false;

    // must be created this way, adding to godatamap would add it to the base map of the instance
    // and when loading it (in go::LoadFromDB()), a new guid would be assigned to the object, and a new object would be created
    // so we must create it specific for this instance
    GameObject * go = new GameObject;
    if (!go->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT),entry, map,x,y,z,o,rotation0,rotation1,rotation2,rotation3,100,GO_STATE_READY))
    {
        sLog.outLog(LOG_DB_ERR, "Gameobject template %u not found in database! BattleGround not created!", entry);
        sLog.outLog(LOG_DEFAULT, "ERROR: Cannot create gameobject template %u! BattleGround not created!", entry);
        delete go;
        return false;
    }
/*
    uint32 guid = go->GetGUIDLow();

    // without this, UseButtonOrDoor caused the crash, since it tried to get go info from godata
    // iirc that was changed, so adding to go data map is no longer required if that was the only function using godata from GameObject without checking if it existed
    GameObjectData& data = sObjectMgr.NewGOData(guid);

    data.id             = entry;
    data.mapid          = GetMapId();
    data.posX           = x;
    data.posY           = y;
    data.posZ           = z;
    data.orientation    = o;
    data.rotation0      = rotation0;
    data.rotation1      = rotation1;
    data.rotation2      = rotation2;
    data.rotation3      = rotation3;
    data.spawntimesecs  = respawnTime;
    data.spawnMask      = 1;
    data.animprogress   = 100;
    data.go_state       = 1;
*/
    // add to world, so it can be later looked up from HashMapHolder
    map->Add(go);
    m_BgObjects[type] = go->GetGUID();
    return true;
}

//some doors aren't despawned so we cannot handle their closing in gameobject::update()
//it would be nice to correctly implement GO_ACTIVATED state and open/close doors in gameobject code
void BattleGround::DoorClose(uint32 type)
{
    Map * tmpMap = GetMap();

    if (!tmpMap)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround::DoorClose: map not found");
        return;
    }

    GameObject *obj = tmpMap->GetGameObject(m_BgObjects[type]);
    if (obj)
    {
        //if doors are open, close it
        if (obj->getLootState() == GO_ACTIVATED && obj->GetGoState() != GO_STATE_READY)
        {
            //change state to allow door to be closed
            obj->SetLootState(GO_READY);
            obj->UseDoorOrButton(RESPAWN_ONE_DAY);
        }
    }
    else
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround: Door object not found (cannot close doors)");
    }
}

void BattleGround::DoorOpen(uint32 type)
{
    Map * tmpMap = GetMap();

    if (!tmpMap)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround::DoorClose: map not found");
        return;
    }

    GameObject *obj = tmpMap->GetGameObject(m_BgObjects[type]);
    if (obj)
    {
        //change state to be sure they will be opened
        obj->SetLootState(GO_READY);
        obj->UseDoorOrButton(RESPAWN_ONE_DAY);
    }
    else
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround: Door object not found! - doors will be closed.");
    }
}

GameObject* BattleGround::GetBGObject(uint32 type)
{
    Map * tmpMap = GetMap();

    if (!tmpMap)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround::GetBGObject: map not found");
        return NULL;
    }

    GameObject *obj = tmpMap->GetGameObject(m_BgObjects[type]);
    if (!obj)
        sLog.outLog(LOG_DEFAULT, "ERROR: couldn't get gameobject %i",type);
    return obj;
}

Creature* BattleGround::GetBGCreature(uint32 type)
{
    Map * tmp = GetMap();
    Creature *creature = NULL;

    if (tmp)
        creature = tmp->GetCreature(m_BgCreatures[type]);
    if (!creature)
        sLog.outLog(LOG_DEFAULT, "ERROR: couldn't get creature %i",type);
    return creature;
}

void BattleGround::SpawnBGObject(uint32 type, uint32 respawntime)
{
    Map * map = GetMap();
    if (!map)
        return;
    if (respawntime == 0)
    {
        GameObject *obj = map->GetGameObject(m_BgObjects[type]);
        if (obj)
        {
            //we need to change state from GO_JUST_DEACTIVATED to GO_READY in case battleground is starting again
            if (obj->getLootState() == GO_JUST_DEACTIVATED)
                obj->SetLootState(GO_READY);
            obj->SetRespawnTime(0);
            map->Add(obj);
        }
    }
    else
    {
        GameObject *obj = map->GetGameObject(m_BgObjects[type]);
        if (obj)
        {
            map->Add(obj);
            obj->SetRespawnTime(respawntime);
            obj->SetLootState(GO_JUST_DEACTIVATED);
            obj->UpdateObjectVisibility();
        }
    }
}

Creature* BattleGround::AddCreature(uint32 entry, uint32 type, uint32 teamval, float x, float y, float z, float o, uint32 respawntime)
{
    Map * map = GetMap();
    if (!map)
        return NULL;

    Creature* pCreature = new Creature;
    if (!pCreature->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_UNIT), map, entry, teamval, x, y, z, o))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Can't create creature entry: %u",entry);
        delete pCreature;
        return NULL;
    }

    pCreature->SetHomePosition(x, y, z, o);

    map->Add(pCreature);
    m_BgCreatures[type] = pCreature->GetGUID();

    return  pCreature;
}
/*
void BattleGround::SpawnBGCreature(uint32 type, uint32 respawntime)
{
    Map * map = sMapMgr.FindMap(GetMapId(),GetInstanceId());
    if (!map)
        return false;

    if (respawntime == 0)
    {
        Creature *obj = HashMapHolder<Creature>::Find(m_BgCreatures[type]);
        if (obj)
        {
            //obj->Respawn();                               // bugged
            obj->SetRespawnTime(0);
            sObjectMgr.SaveCreatureRespawnTime(obj->GetGUIDLow(), GetInstanceID(), 0);
            map->Add(obj);
        }
    }
    else
    {
        Creature *obj = HashMapHolder<Creature>::Find(m_BgCreatures[type]);
        if (obj)
        {
            obj->setDeathState(DEAD);
            obj->SetRespawnTime(respawntime);
            map->Add(obj);
        }
    }
}
*/
bool BattleGround::DelCreature(uint32 type)
{
    Map * map = GetMap();
    if (!map)
        return false;

    if (!m_BgCreatures[type])
        return true;

    Creature *cr = map->GetCreature(m_BgCreatures[type]);
    if (!cr)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Can't find creature guid: %u",GUID_LOPART(m_BgCreatures[type]));
        return false;
    }
    //TODO: only delete creature after not in combat
    cr->AddObjectToRemoveList();
    m_BgCreatures[type] = 0;
    return true;
}

bool BattleGround::DelObject(uint32 type, bool setGoState)
{
    Map * map = GetMap();
    if (!map)
        return false;

    if (!m_BgObjects[type])
        return true;

    GameObject *obj = map->GetGameObject(m_BgObjects[type]);
    if (!obj)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Can't find gobject guid: %u",GUID_LOPART(m_BgObjects[type]));
        return false;
    }
    obj->SetRespawnTime(0);                                 // not save respawn time
    obj->Delete(setGoState);
    m_BgObjects[type] = 0;
    return true;
}

bool BattleGround::AddSpiritGuide(uint32 type, float x, float y, float z, float o, uint32 team)
{
    uint32 entry = 0;

    if (team == ALLIANCE)
        entry = 13116;
    else
        entry = 13117;

    Creature* pCreature = AddCreature(entry,type,team,x,y,z,o);
    if (!pCreature)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Can't create Spirit guide. BattleGround not created!");
        EndNow();
        return false;
    }

    pCreature->setDeathState(DEAD);

    pCreature->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, pCreature->GetGUID());
    // aura
    pCreature->SetUInt32Value(UNIT_FIELD_AURA, SPELL_SPIRIT_HEAL_CHANNEL);
    pCreature->SetUInt32Value(UNIT_FIELD_AURAFLAGS, 0x00000009);
    pCreature->SetUInt32Value(UNIT_FIELD_AURALEVELS, 0x0000003C);
    pCreature->SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS, 0x000000FF);
    // casting visual effect
    pCreature->SetUInt32Value(UNIT_CHANNEL_SPELL, SPELL_SPIRIT_HEAL_CHANNEL);
    // correct cast speed
    pCreature->SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);

    //pCreature->CastSpell(pCreature, SPELL_SPIRIT_HEAL_CHANNEL, true);

    return true;
}

void BattleGround::SendMessageToAll(char const* text)
{
    WorldPacket data;
    ChatHandler::FillMessageData(&data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, text, NULL);
    SendPacketToAll(&data);
}

void BattleGround::SendMessageToTeam(uint32 team, char const* text)
{
    WorldPacket data;
    ChatHandler::FillMessageData(&data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, text, NULL);
    SendPacketToTeam(team, &data);
}

void BattleGround::SendMessageToTeam(uint32 team, int32 entry)
{
    char const* text = GetMangosString(entry);

    WorldPacket data;
    ChatHandler::FillMessageData(&data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, text, NULL);
    SendPacketToTeam(team, &data);
}

void BattleGround::PrepareMessageToAll(char const *format, ...)
{
    va_list ap;
    char str [1024];
    va_start(ap, format);
    vsnprintf(str,1024,format, ap);
    va_end(ap);
    SendMessageToAll(str);
}

void BattleGround::SendMessageToAll(int32 entry)
{
    char const* text = GetMangosString(entry);
    WorldPacket data;
    ChatHandler::FillMessageData(&data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, text, NULL);
    SendPacketToAll(&data);
}

void BattleGround::EndNow()
{
    RemoveFromBGFreeSlotQueue();
    SetStatus(STATUS_WAIT_LEAVE);
    SetEndTime(TIME_TO_AUTOREMOVE);
    // inform invited players about the removal
    sBattleGroundMgr.m_BattleGroundQueues[BattleGroundMgr::BGQueueTypeId(GetTypeID(), GetArenaType())].BGEndedRemoveInvites(this);
}

// Battleground messages are localized using the dbc lang, they are not client language dependent
const char *BattleGround::GetMangosString(int32 entry)
{
    // FIXME: now we have different DBC locales and need localized message for each target client
    return sObjectMgr.GetMangosStringForDBCLocale(entry);
}

bool BattleGround::HandlePlayerUnderMap(Player * plr, float z)
{
    if (z > -500.0f)
        return false;

    EventPlayerDroppedFlag(plr);

    WorldSafeLocsEntry const *graveyard = GetClosestGraveYard(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), plr->GetBGTeam());
    if (graveyard)
    {
        plr->NearTeleportTo(graveyard->x, graveyard->y, graveyard->z, plr->GetOrientation());
        if (plr->IsDead())                                        // not send if alive, because it used in TeleportTo()
        {
            WorldPacket data(SMSG_DEATH_RELEASE_LOC, 4*4);  // show spirit healer position on minimap
            data << graveyard->map_id;
            data << graveyard->x;
            data << graveyard->y;
            data << graveyard->z;
            plr->SendPacketToSelf(&data);
        }
        return true;
    }
    return false;
}

/*
important notice:
buffs aren't spawned/despawned when players captures anything
buffs are in their positions when battleground starts
*/
void BattleGround::HandleTriggerBuff(uint64 const& go_guid)
{
    Map * map = GetMap();
    if (!map)
        return;

    GameObject *obj = map->GetGameObject(go_guid);
    if (!obj || obj->GetGoType() != GAMEOBJECT_TYPE_TRAP || !obj->isSpawned())
        return;

    //change buff type, when buff is used:
    int32 index = m_BgObjects.size() - 1;
    while (index >= 0 && m_BgObjects[index] != go_guid)
        index--;
    if (index < 0)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround (Type: %u) has buff gameobject (Guid: %u Entry: %u Type:%u) but it hasn't that object in its internal data",GetTypeID(),GUID_LOPART(go_guid),obj->GetEntry(),obj->GetGoType());
        return;
    }

    //randomly select new buff
    uint8 buff = urand(0, 2);
    uint32 entry = obj->GetEntry();
    if (m_BuffChange && entry != Buff_Entries[buff])
    {
        //despawn current buff
        SpawnBGObject(index, RESPAWN_ONE_DAY);
        //set index for new one
        for (uint8 currBuffTypeIndex = 0; currBuffTypeIndex < 3; ++currBuffTypeIndex)
            if (entry == Buff_Entries[currBuffTypeIndex])
            {
                index -= currBuffTypeIndex;
                index += buff;
            }
    }

    SpawnBGObject(index, BUFF_RESPAWN_TIME);
}

void BattleGround::HandleKillPlayer(Player *player, Player *killer)
{
    //keep in mind that for arena this will have to be changed a bit

    // add +1 deaths
    UpdatePlayerScore(player, SCORE_DEATHS, 1);

    // add +1 kills to group and +1 killing_blows to killer
    if (killer)
    {
        if (!player->ToUnit()->WorthHonor)
            UpdatePlayerScore(killer, SCORE_HONORABLE_KILLS, 1);

        UpdatePlayerScore(killer, SCORE_KILLING_BLOWS, 1);

        for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        {
            Player *plr = sObjectMgr.GetPlayer(itr->first);

            if (!plr || plr == killer)
                continue;

            if (plr->GetBGTeam() == killer->GetBGTeam() && plr->IsAtGroupRewardDistance(player) && !player->ToUnit()->WorthHonor)
                UpdatePlayerScore(plr, SCORE_HONORABLE_KILLS, 1);
        }
    }

    // to be able to remove insignia -- ONLY IN BattleGrounds
    if (!isArena())
        player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
}

// return the player's team based on battlegroundplayer info
// used in same faction arena matches mainly
uint32 BattleGround::GetPlayerTeam(uint64 guid)
{
    BattleGroundPlayerMap::const_iterator itr = m_Players.find(guid);
    if (itr!=m_Players.end())
        return itr->second.Team;
    return 0;
}

bool BattleGround::IsPlayerInBattleGround(uint64 guid)
{
    BattleGroundPlayerMap::const_iterator itr = m_Players.find(guid);
    if (itr!=m_Players.end())
        return true;
    return false;
}

uint32 BattleGround::GetAlivePlayersCountByTeam(uint32 Team) const
{
    int count = 0;
    for (BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.Team == Team)
        {
            Player * pl = sObjectMgr.GetPlayer(itr->first);
            if (pl && pl->IsAlive() && !pl->HasByteFlag(UNIT_FIELD_BYTES_2, 3, FORM_SPIRITOFREDEMPTION))
                ++count;
        }
    }
    return count;
}

void BattleGround::SetHoliday(bool is_holiday)
{
    if (is_holiday)
        m_HonorMode = BG_HOLIDAY;
    else
        m_HonorMode = BG_NORMAL;
}

int32 BattleGround::GetObjectType(uint64 guid)
{
    for (uint32 i = 0;i <= m_BgObjects.size(); i++)
        if (m_BgObjects[i] == guid)
            return i;
    sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround: cheating? a player used a gameobject which isnt supposed to be a usable object!");
    return -1;
}

void BattleGround::HandleKillUnit(Creature *creature, Player *killer)
{
}

// This method should be called when player logs out from running battleground
void BattleGround::EventPlayerLoggedOut(Player* player)
{
    if (player->isArenaSpectating())
    {
        player->UnspectateArena(true);
        return;
    }

    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        if (isBattleGround())
            EventPlayerDroppedFlag(player);
    }

    if (isArena())
        RemovePlayerAtLeave(player->GetGUID(), true, true);
}

void BattleGround::SetBgRaid( uint32 TeamID, Group *bg_raid )
{
    Group* &old_raid = TeamID == ALLIANCE ? m_BgRaids[BG_TEAM_ALLIANCE] : m_BgRaids[BG_TEAM_HORDE];

    if (old_raid)
        old_raid->SetBattlegroundGroup(NULL);

    if (bg_raid)
        bg_raid->SetBattlegroundGroup(this);

    old_raid = bg_raid;
}

void BattleGround::SetStatus(uint32 Status)
{
    m_Status = Status;
    if (Status == STATUS_IN_PROGRESS)
    {
        m_progressStart = time(NULL);
        m_TimeElapsedSinceBeggining = 0;
    }
}

void BattleGround::SendObjectiveComplete(uint32 id, uint32 TeamID, float x, float y)
{
    float distance =45.0f;
    distance= distance*distance;
    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr.GetPlayer(itr->first);

        if (!plr)
        {
            sLog.outDebug("BattleGround: Player " UI64FMTD " not found!", itr->first);
            continue;
        }

        uint32 team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if (!team) team = plr->GetTeam();

        if (team == TeamID && plr->IsInWorld())
        {
            float dist = (plr->GetPositionX() - x)*(plr->GetPositionX() - x)+(plr->GetPositionY() - y)*(plr->GetPositionY() - y);
            if (dist < distance)
                plr->KilledMonster(id, 0);
        }
    }
}

uint8 BattleGround::SetPlayerReady(uint64 playerGUID)
{
    if ( !isArena() )
        return 1;

    uint32 readyCount = m_guidsReady[ 0 ].size() + m_guidsReady[ 1 ].size();
    if ( readyCount == GetMaxPlayers() )
        return 2;

    uint32 team = GetPlayerTeam( playerGUID );
    if ( team == TEAM_NONE )
        return 3;

    if ( GetStatus() != STATUS_WAIT_JOIN )
        return 4;

    if ( GetStartDelayTime() <= sWorld.getConfig(CONFIG_ARENA_READY_START_TIMER) )
        return 5;

    uint8 idx = team == ALLIANCE ? 0 : 1;
    m_guidsReady[ idx ].insert( playerGUID );

    readyCount = m_guidsReady[ 0 ].size() + m_guidsReady[ 1 ].size();
    if ( readyCount == GetMaxPlayers() )
    {
        SendMessageToAll("Everyone is ready. Let's rumble!");
        SetStartDelayTime(sWorld.getConfig(CONFIG_ARENA_READY_START_TIMER));
    }
    else if ( m_guidsReady[ idx ].size() == GetMaxPlayersPerTeam() )
    {
        SendMessageToTeam(team == ALLIANCE ? HORDE : ALLIANCE, "Your opponents are ready to start earlier. How about you?");
    }
    return 0;
}
