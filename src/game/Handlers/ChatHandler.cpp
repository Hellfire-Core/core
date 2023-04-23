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

#include "Common.h"
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "Opcodes.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "Database/DatabaseEnv.h"
#include "ChannelMgr.h"
#include "Group.h"
#include "Guild.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "SpellAuras.h"
#include "Language.h"
#include "Util.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "GuildMgr.h"

enum ChatDenyMask
{
    DENY_NONE       = 0,
    DENY_SAY        = 1,
    DENY_EMOTE      = 2,
    DENY_PARTY      = 4,
    DENY_GUILD      = 8,
    DENY_WHISP      = 16,
    DENY_CHANNEL    = 32,
    DENY_ADDON      = 64,
};

void WorldSession::HandleMessagechatOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,4+4+1);

    uint32 type;
    uint32 lang;
    uint32 team = _player->GetTeam();
    recv_data >> type;
    recv_data >> lang;

    if (type >= MAX_CHAT_MSG_TYPE)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: CHAT: Wrong message type received: %u", type);
        return;
    }

    // check if addon channel is disabled
    if (lang == LANG_ADDON && !sWorld.getConfig(CONFIG_ADDON_CHANNEL))
        return;

    std::string channel = "";
    std::string msg = "";
    std::string to = "";

    if (type == CHAT_MSG_WHISPER)
    {
        recv_data >> to;
        if (!normalizePlayerName(to))
        {
            WorldPacket data(SMSG_CHAT_PLAYER_NOT_FOUND, (to.size() + 1));
            data << to;
            SendPacket(&data);
        }
    }
    else if(type == CHAT_MSG_CHANNEL)
        recv_data >> channel;

    recv_data >> msg;
    if (type != CHAT_MSG_AFK && type != CHAT_MSG_DND)
    {
        if (msg.empty())
            return;

        if (ChatHandler(this).ParseCommands(msg.c_str()) > 0)
            return;
    }

    // prevent talking at unknown language (cheating)
    LanguageDesc const* langDesc = GetLanguageDescByID(lang);
    if (!langDesc)
    {
        SendNotification(LANG_UNKNOWN_LANGUAGE);
        return;
    }

    if (langDesc->skill_id != 0 && !_player->HasSkill(langDesc->skill_id))
    {
        // also check SPELL_AURA_COMPREHEND_LANGUAGE (client offers option to speak in that language)
        Unit::AuraList const& langAuras = _player->GetAurasByType(SPELL_AURA_COMPREHEND_LANGUAGE);
        bool foundAura = false;
        for (Unit::AuraList::const_iterator i = langAuras.begin();i != langAuras.end(); ++i)
        {
            if ((*i)->GetModifier()->m_miscvalue == lang)
            {
                foundAura = true;
                break;
            }
        }

        if (!foundAura)
        {
            SendNotification(LANG_NOT_LEARNED_LANGUAGE);
            return;
        }
    }

    // mass mute for players check
    if (!HasHigherGMLevel(SEC_GAMEMASTER) && sWorld.GetMassMuteTime() && sWorld.GetMassMuteTime() > time(NULL))
    {
        if (sWorld.GetMassMuteReason())
            ChatHandler(_player).PSendSysMessage("Server has been muted. Mass mute reason: %s", sWorld.GetMassMuteReason());

        return;
    }

    // LANG_ADDON should not be changed nor be affected by flood control
    if (lang != LANG_ADDON)
    {
        // send in universal language if player in .gmon mode (ignore spell effects)
        if (_player->IsGameMaster())
            lang = LANG_UNIVERSAL;
        else
        {
            // send in universal language in two side iteration allowed mode
            if (sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHAT))
                lang = LANG_UNIVERSAL;
            else
            {
                switch (type)
                {
                    case CHAT_MSG_PARTY:
                    case CHAT_MSG_RAID:
                    case CHAT_MSG_RAID_LEADER:
                    case CHAT_MSG_RAID_WARNING:
                        // allow two side chat at group channel if two side group allowed
                        if (sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP))
                            lang = LANG_UNIVERSAL;
                        break;
                    case CHAT_MSG_GUILD:
                    case CHAT_MSG_OFFICER:
                        // allow two side chat at guild channel if two side guild allowed
                        if (sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD))
                            lang = LANG_UNIVERSAL;
                        break;
                    case CHAT_MSG_BATTLEGROUND:
                    case CHAT_MSG_BATTLEGROUND_LEADER:
                        lang = LANG_UNIVERSAL;
                        break;
                }
            }

            // but overwrite it by SPELL_AURA_MOD_LANGUAGE auras (only single case used)
            Unit::AuraList const& ModLangAuras = _player->GetAurasByType(SPELL_AURA_MOD_LANGUAGE);
            if (!ModLangAuras.empty())
                lang = ModLangAuras.front()->GetModifier()->m_miscvalue;
        }

        if (type != CHAT_MSG_AFK && type != CHAT_MSG_DND)
            GetPlayer()->UpdateSpeakTime();
    }

    if (!_player->CanSpeak())
    {
        if (lang != LANG_ADDON)
        {
            std::string timeStr = secsToTimeString(m_muteTime - time(NULL));
            SendNotification(GetMangosString(LANG_WAIT_BEFORE_SPEAKING),timeStr.c_str());
            ChatHandler(_player).PSendSysMessage(LANG_YOUR_CHAT_IS_DISABLED, timeStr.c_str(), m_muteReason.c_str());
        }

        return;
    }

    if (GetPlayer()->GetLevel() < sWorld.getConfig(CONFIG_CHAT_MINIMUM_LVL))
    {
        int mask = 0;
        switch (type)
        {
            case CHAT_MSG_SAY:
            case CHAT_MSG_YELL:
                mask = DENY_SAY;
                break;
            case CHAT_MSG_EMOTE:
            case CHAT_MSG_TEXT_EMOTE:
                mask = DENY_EMOTE;
                break;
            case CHAT_MSG_PARTY:
            case CHAT_MSG_RAID:
            case CHAT_MSG_RAID_LEADER:
            case CHAT_MSG_RAID_WARNING:
            case CHAT_MSG_BATTLEGROUND:
            case CHAT_MSG_BATTLEGROUND_LEADER:
                mask = DENY_PARTY;
                break;
            case CHAT_MSG_GUILD:
            case CHAT_MSG_OFFICER:
                mask = DENY_GUILD;
                break;
            case CHAT_MSG_WHISPER:
            case CHAT_MSG_WHISPER_INFORM:
            case CHAT_MSG_REPLY:
            {
                Player *target = sObjectMgr.GetPlayer(to.c_str());
                if (target && !target->IsGameMaster())
                    mask = DENY_WHISP;

                break;
            }
            case CHAT_MSG_CHANNEL:
            case CHAT_MSG_CHANNEL_JOIN:
            case CHAT_MSG_CHANNEL_LEAVE:
            case CHAT_MSG_CHANNEL_NOTICE:
            case CHAT_MSG_CHANNEL_NOTICE_USER:
                mask = DENY_CHANNEL;
                break;
            case CHAT_MSG_ADDON:
                mask = DENY_ADDON;
                break;
            default:
                break;
        }

        if (lang == LANG_ADDON)
            mask |= DENY_ADDON;

        if (sWorld.getConfig(CONFIG_CHAT_DENY_MASK) & mask)
        {
            SendNotification("You can't send message of this type until you reach level %i.", sWorld.getConfig(CONFIG_CHAT_MINIMUM_LVL));
            return;
        }
    }

    switch (type)
    {
        case CHAT_MSG_SAY:
        case CHAT_MSG_EMOTE:
        case CHAT_MSG_YELL:
        {
            if (!_player->IsAlive())
                return;

            // strip invisible characters for non-addon messages
            if (lang != LANG_ADDON && sWorld.getConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
                stripLineInvisibleChars(msg);

            if (msg.empty())
                break;

            if (ChatHandler(this).ContainsNotAllowedSigns(msg))
                return;

            switch (type)
            {
                case CHAT_MSG_SAY:
                    GetPlayer()->Say(msg, lang);
                    break;
                case CHAT_MSG_EMOTE:
                    GetPlayer()->TextEmote(msg);
                    break;
                case CHAT_MSG_YELL:
                    GetPlayer()->Yell(msg, lang);
                    break;
                default:
                    break;
            }
            if (lang != LANG_ADDON)
                sLog.outChat(LOG_CHAT_SAY_A, team,_player->GetName(), msg.c_str());
        } 
        break;

        case CHAT_MSG_WHISPER:
        {
            // strip invisible characters for non-addon messages
            if (lang != LANG_ADDON && sWorld.getConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
                stripLineInvisibleChars(msg);

            if (msg.empty())
                break;

            if (ChatHandler(this).ContainsNotAllowedSigns(msg))
                return;

            Player *player = sObjectMgr.GetPlayer(to.c_str());
            if (!player ||
                ( player->GetSession()->HasHigherGMLevel(SEC_GAMEMASTER) && !HasHigherGMLevel(SEC_DEVELOPER) &&
                !player->isAcceptWhispers() && !GetPlayer()->canWhisperToGM())
                )
            {
                WorldPacket data(SMSG_CHAT_PLAYER_NOT_FOUND, (to.size()+1));
                data<<to;
                SendPacket(&data);
                return;
            }

            if (!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHAT) && !HasHigherGMLevel(SEC_DEVELOPER) && !player->GetSession()->HasHigherGMLevel(SEC_DEVELOPER))
            {
                uint32 sidea = GetPlayer()->GetTeam();
                uint32 sideb = player->GetTeam();
                if (sidea != sideb)
                {
                    WorldPacket data(SMSG_CHAT_PLAYER_NOT_FOUND, (to.size()+1));
                    data<<to;
                    SendPacket(&data);
                    return;
                }
            }

            GetPlayer()->Whisper(msg, lang,player->GetGUID());
            if (!player->CanSpeak())
                ChatHandler(_player).PSendSysMessage("The player you are whispering to is muted and can not reply.");
        } 
        break;

        case CHAT_MSG_PARTY:
        {
            // strip invisible characters for non-addon messages
            if (lang != LANG_ADDON && sWorld.getConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
                stripLineInvisibleChars(msg);

            if (msg.empty())
                break;

            if (ChatHandler(this).ContainsNotAllowedSigns(msg))
                return;

            Group *group = GetPlayer()->GetOriginalGroup();
            if (!group && (!(group = GetPlayer()->GetGroup()) || group->isBGGroup()))
                return;

            WorldPacket data;
            ChatHandler::FillMessageData(&data, this, CHAT_MSG_PARTY, lang, NULL, 0, msg.c_str(),NULL);
            group->BroadcastPacket(&data, false, group->GetMemberGroup(GetPlayer()->GetGUID()));
            if (lang != LANG_ADDON)
                sLog.outChat(LOG_CHAT_PARTY_A, team, _player->GetName(), msg.c_str());
        }
        break;
        case CHAT_MSG_GUILD:
        {
            // strip invisible characters for non-addon messages
            if (lang != LANG_ADDON && sWorld.getConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
                stripLineInvisibleChars(msg);

            if (msg.empty())
                break;

            if (ChatHandler(this).ContainsNotAllowedSigns(msg))
                return;

            if (GetPlayer()->GetGuildId())
            {
                Guild *guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId());
                if (guild)
                    guild->BroadcastToGuild(this, msg, lang == LANG_ADDON ? LANG_ADDON : LANG_UNIVERSAL);
                if (lang != LANG_ADDON)
                {
                    std::string widename = std::string(_player->GetName()) + " " + guild->GetName();
                    sLog.outChat(LOG_CHAT_GUILD_A, team, widename.c_str(), msg.c_str());
                }
            }
            
            break;
        }
        case CHAT_MSG_OFFICER:
        {
            // strip invisible characters for non-addon messages
            if (lang != LANG_ADDON && sWorld.getConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
                stripLineInvisibleChars(msg);

            if (msg.empty())
                break;

            if (ChatHandler(this).ContainsNotAllowedSigns(msg))
                return;


            if (GetPlayer()->GetGuildId())
            {
                Guild *guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId());
                if (guild)
                    guild->BroadcastToOfficers(this, msg, lang == LANG_ADDON ? LANG_ADDON : LANG_UNIVERSAL);

                if (lang != LANG_ADDON)
                {
                    std::string widename = std::string(_player->GetName()) + " oficer " + guild->GetName();
                    sLog.outChat(LOG_CHAT_GUILD_A, team, widename.c_str(), msg.c_str());
                }
            }
            break;
        }
        case CHAT_MSG_RAID:
        {
            // strip invisible characters for non-addon messages
            if (lang != LANG_ADDON && sWorld.getConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
                stripLineInvisibleChars(msg);

            if (msg.empty())
                break;

            if (ChatHandler(this).ContainsNotAllowedSigns(msg))
                return;

            Group *group = GetPlayer()->GetOriginalGroup();
            if (!group && (!(group = GetPlayer()->GetGroup()) || group->isBGGroup()))
                return;

            WorldPacket data;
            ChatHandler::FillMessageData(&data, this, CHAT_MSG_RAID, lang, "", 0, msg.c_str(),NULL);
            group->BroadcastPacket(&data, false);
            if (lang != LANG_ADDON)
                sLog.outChat(LOG_CHAT_RAID_A, team, _player->GetName(), msg.c_str());
        } break;
        case CHAT_MSG_RAID_LEADER:
        {
            // strip invisible characters for non-addon messages
            if (lang != LANG_ADDON && sWorld.getConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
                stripLineInvisibleChars(msg);

            if (msg.empty())
                break;

            if (ChatHandler(this).ContainsNotAllowedSigns(msg))
                return;

            Group *group = GetPlayer()->GetOriginalGroup();
            if (!group && !(group = GetPlayer()->GetGroup()) || group->isBGGroup() || !group->isRaidGroup() || !group->IsLeader(GetPlayer()->GetGUID()))
                return;

            WorldPacket data;
            ChatHandler::FillMessageData(&data, this, CHAT_MSG_RAID_LEADER, lang, "", 0, msg.c_str(),NULL);
            group->BroadcastPacket(&data, false);
            if (lang != LANG_ADDON)
                sLog.outChat(LOG_CHAT_RAID_A, team, _player->GetName(), msg.c_str());
        } break;
        case CHAT_MSG_RAID_WARNING:
        {
            // strip invisible characters for non-addon messages
            if (lang != LANG_ADDON && sWorld.getConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
                stripLineInvisibleChars(msg);

            if (msg.empty())
                break;

            Group *group = GetPlayer()->GetGroup();
            if (!group || !group->isRaidGroup() || !(group->IsLeader(GetPlayer()->GetGUID()) || group->IsAssistant(GetPlayer()->GetGUID())) || group->isBGGroup())
                return;

            WorldPacket data;
            ChatHandler::FillMessageData(&data, this, CHAT_MSG_RAID_WARNING, lang, "", 0, msg.c_str(),NULL);
            group->BroadcastPacket(&data, false);
            if (lang != LANG_ADDON)
                sLog.outChat(LOG_CHAT_RAID_A, team, _player->GetName(), msg.c_str());
        } break;

        case CHAT_MSG_BATTLEGROUND:
        {
            // strip invisible characters for non-addon messages
            if (lang != LANG_ADDON && sWorld.getConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
                stripLineInvisibleChars(msg);

            if (msg.empty())
                break;

            Group *group = GetPlayer()->GetGroup();
            if (!group || !group->isBGGroup())
                return;

            WorldPacket data;
            ChatHandler::FillMessageData(&data, this, CHAT_MSG_BATTLEGROUND, lang, "", 0, msg.c_str(),NULL);
            group->BroadcastPacket(&data, false);
            if (lang != LANG_ADDON)
                sLog.outChat(LOG_CHAT_BG_A, team, _player->GetName(), msg.c_str());
        } break;

        case CHAT_MSG_BATTLEGROUND_LEADER:
        {
            // strip invisible characters for non-addon messages
            if (lang != LANG_ADDON && sWorld.getConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
                stripLineInvisibleChars(msg);

            if (msg.empty())
                break;

            Group *group = GetPlayer()->GetGroup();
            if (!group || !group->isBGGroup() || !group->IsLeader(GetPlayer()->GetGUID()))
                return;

            WorldPacket data;
            ChatHandler::FillMessageData(&data, this, CHAT_MSG_BATTLEGROUND_LEADER, lang, "", 0, msg.c_str(),NULL);
            group->BroadcastPacket(&data, false);
            if (lang != LANG_ADDON)
                sLog.outChat(LOG_CHAT_BG_A, team, _player->GetName(), msg.c_str());
        } break;

        case CHAT_MSG_CHANNEL:
        {
            // strip invisible characters for non-addon messages
            if (lang != LANG_ADDON && sWorld.getConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
                stripLineInvisibleChars(msg);

            if (msg.empty())
                break;

            if (ChannelMgr* cMgr = channelMgr(team))
            {
                if (Channel *chn = cMgr->GetChannel(channel,_player))
                    chn->Say(_player->GetGUID(),msg.c_str(),lang);
            }
        } break;

        case CHAT_MSG_AFK:
        {
            if ((msg.empty() || !_player->isAFK()) && !_player->IsInCombat())
            {
                if (!_player->isAFK())
                {
                    if (msg.empty())
                        msg  = GetMangosString(LANG_PLAYER_AFK_DEFAULT);
                    _player->afkMsg = msg;
                }
                _player->ToggleAFK();
                if (_player->isAFK() && _player->isDND())
                    _player->ToggleDND();
            }
        } break;

        case CHAT_MSG_DND:
        {
            if (msg.empty() || !_player->isDND())
            {
                if (!_player->isDND())
                {
                    if (msg.empty())
                        msg  = GetMangosString(LANG_PLAYER_DND_DEFAULT);
                    _player->dndMsg = msg;
                }
                _player->ToggleDND();
                if (_player->isDND() && _player->isAFK())
                    _player->ToggleAFK();
            }
        } break;

        default:
            sLog.outLog(LOG_DEFAULT, "ERROR: CHAT: unknown message type %u, lang: %u", type, lang);
            break;
    }
}

void WorldSession::HandleEmoteOpcode(WorldPacket & recv_data)
{
    if (!GetPlayer()->IsAlive() || GetPlayer()->isPossessed() || GetPlayer()->isCharmed())
        return;
    CHECK_PACKET_SIZE(recv_data,4);

    uint32 emote;
    recv_data >> emote;
    GetPlayer()->HandleEmoteCommand(emote);
}

void WorldSession::HandleTextEmoteOpcode(WorldPacket & recv_data)
{
    Player * player = GetPlayer();
    if (!player->IsAlive() || player->isPossessed() || player->isCharmed())
        return;

    if (!player->CanSpeak())
    {
        std::string timeStr = secsToTimeString(m_muteTime - time(NULL));
        SendNotification(GetMangosString(LANG_WAIT_BEFORE_SPEAKING),timeStr.c_str());
        ChatHandler(player).PSendSysMessage(LANG_YOUR_CHAT_IS_DISABLED, timeStr.c_str(), m_muteReason.c_str());
        return;
    }

    CHECK_PACKET_SIZE(recv_data, 4+4+8);

    uint32 textEmote, emoteNum;
    uint64 guid;

    recv_data >> textEmote;
    recv_data >> emoteNum;
    recv_data >> guid;

    EmotesTextEntry const *em = sEmotesTextStore.LookupEntry(textEmote);
    if (!em)
        return;

    uint32 emoteId = em->textid;

    switch (emoteId)
    {
        case EMOTE_STATE_SLEEP:
        case EMOTE_STATE_SIT:
        case EMOTE_STATE_KNEEL:
        case EMOTE_ONESHOT_NONE:
            break;
        default:
        {
            // in feign death state allowed only text emotes.
            if (!player->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
                player->HandleEmote(emoteId);
            break;
        }
    }

    Unit* unit = player->GetMap()->GetUnit(guid);

    WorldPacket data;
    char const* nam = unit ? unit->GetName() : NULL;
    uint32 namlen = (nam ? strlen(nam) : 0) + 1;

    data.Initialize(SMSG_TEXT_EMOTE, (20 + namlen));
    data << player->GetGUID();
    data << uint32(textEmote);
    data << emoteNum;
    data << uint32(namlen);
    if (namlen > 1)
        data.append(nam, namlen);
    else
        data << (uint8)0x00;

    MaNGOS::PacketBroadcaster broadcast(*player, &data, NULL, sWorld.getConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE));
    Cell::VisitWorldObjects(player, broadcast, sWorld.getConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE));

    //Send scripted event call
    if (unit && unit->GetTypeId() == TYPEID_UNIT)
    {
        if (((Creature*)unit)->AI())
            ((Creature*)unit)->AI()->ReceiveEmote(player, textEmote);

        sScriptMgr.OnReceiveEmote(GetPlayer(), (Creature*)unit, textEmote);
    }
}

void WorldSession::HandleChatIgnoredOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8+1);

    uint64 iguid;
    uint8 unk;
    //sLog.outDebug("WORLD: Received CMSG_CHAT_IGNORED");

    recv_data >> iguid;
    recv_data >> unk;                                       // probably related to spam reporting

    Player *player = sObjectMgr.GetPlayer(iguid);
    if (!player || !player->GetSession())
        return;

    WorldPacket data;
    ChatHandler::FillMessageData(&data, this, CHAT_MSG_IGNORED, LANG_UNIVERSAL, NULL, GetPlayer()->GetGUID(), GetPlayer()->GetName(),NULL);
    player->SendPacketToSelf(&data);
}

void WorldSession::HandleChannelDeclineInvite(WorldPacket &recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
}
