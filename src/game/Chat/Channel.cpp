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

#include "Channel.h"
#include "ObjectMgr.h"
#include "World.h"
#include "SocialMgr.h"
#include "Chat.h"

Channel::Channel(const std::string& name)
: m_announce(false), m_moderate(false), m_name(name), m_ownerGUID(0)
{
    // DO NOT TRUST channel ids send from client!!
    if (name == "world")
        m_channelId = CHANNEL_ID_WORLD;
    else if (name.substr(0, 7) == "General")
        m_channelId = CHANNEL_ID_GENERAL;
    else if (name.substr(0, 5) == "Trade")
        m_channelId = CHANNEL_ID_TRADE;
    else if (name.substr(0, 12) == "LocalDefense")
        m_channelId = CHANNEL_ID_LOCALDEFENSE;
    else if (name == "WorldDefense")
        m_channelId = CHANNEL_ID_WORLDDEFENSE;
    else if (name.substr(0, 16) == "GuildRecruitment")
        m_channelId = CHANNEL_ID_GUILDRECRUITMENT;
    else if (name == "LookingForGroup")
        m_channelId = CHANNEL_ID_LFG;
    else                                                 // it's custom channel
        m_channelId = CHANNEL_ID_CUSTOM;
}

void Channel::Join(uint64 p, const char *pass)
{
    WorldPacket data;
    if (IsOn(p))
    {
        if (!IsConstant())                                   // non send error message for built-in channels
        {
            MakePlayerAlreadyMember(&data, p);
            SendToOne(&data, p);
        }
        return;
    }

    Player *plr = sObjectMgr.GetPlayer(p);

    if ((!plr || !plr->IsGameMaster()) && !IsConstant())
    {
        uint32 limitCount = sWorld.getConfig(CONFIG_PRIVATE_CHANNEL_LIMIT);

        if (limitCount && players.size() > limitCount)
        {
            MakeInvalidName(&data);
            SendToOne(&data, p);
            return;
        }
    }

    if (!m_ownerGUID && (!plr || !plr->CanSpeak())) // muted players can't create new channels
    {
        MakeBanned(&data);//no idea what to send
        SendToOne(&data, p);
        return;
    }

    if (IsBanned(p) && (!plr || !plr->IsGameMaster()))
    {
        MakeBanned(&data);
        SendToOne(&data, p);
        return;
    }

    if (m_password.length() > 0 && strcmp(pass, m_password.c_str()) && (!plr || !plr->IsGameMaster()))
    {
        MakeWrongPassword(&data);
        SendToOne(&data, p);
        return;
    }

    if (plr)
    {
        if (IsLFG() &&
            sWorld.getConfig(CONFIG_RESTRICTED_LFG_CHANNEL) && !plr->GetSession()->HasHigherGMLevel(SEC_GAMEMASTER) &&
            plr->m_lookingForGroup.Empty())
        {
            MakeNotInLfg(&data);
            SendToOne(&data, p);
            return;
        }

        plr->JoinedChannel(this);
    }

    if (m_announce && (!plr || !plr->GetSession()->HasHigherGMLevel(SEC_GAMEMASTER) || !sWorld.getConfig(CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL)))
    {
        MakeJoined(&data, p);
        SendToAll(&data);
    }

    data.clear();

    PlayerInfo pinfo;
    pinfo.player = p;
    pinfo.flags = 0;
    players[p] = pinfo;

    MakeYouJoined(&data);
    SendToOne(&data, p);

    JoinNotify(p);

    // if no owner first logged will become
    if (!IsConstant() && !m_ownerGUID)
    {
        SetOwner(p, (players.size() > 1 ? true : false));
        players[p].SetModerator(true);
    }
}

void Channel::Leave(uint64 p, bool send)
{
    if (!IsOn(p))
    {
        if (send)
        {
            WorldPacket data;
            MakeNotMember(&data);
            SendToOne(&data, p);
        }
    }
    else
    {
        Player *plr = sObjectMgr.GetPlayer(p);

        if (send)
        {
            WorldPacket data;
            MakeYouLeft(&data);
            SendToOne(&data, p);
            if (plr)
                plr->LeftChannel(this);
            data.clear();
        }

        bool changeowner = players[p].IsOwner();

        players.erase(p);
        if (m_announce && (!plr || !plr->GetSession()->HasHigherGMLevel(SEC_GAMEMASTER) || !sWorld.getConfig(CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL)))
        {
            WorldPacket data;
            MakeLeft(&data, p);
            SendToAll(&data);
        }

        LeaveNotify(p);

        if (changeowner)
            ChangeOwner();
    }
}

void Channel::KickOrBan(uint64 good, const char *badname, bool ban)
{
    uint8 sec = 0;
    Player *gplr = sObjectMgr.GetPlayer(good);
    if (gplr)
        sec = gplr->GetSession()->GetGMLevel();

    if (!IsOn(good))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, good);
    }
    else if (!players[good].IsModerator() && !(sec >= SEC_GAMEMASTER))
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, good);
    }
    else
    {
        Player *bad = sObjectMgr.GetPlayer(badname);
        if (bad == NULL || !IsOn(bad->GetGUID()))
        {
            WorldPacket data;
            MakePlayerNotFound(&data, badname);
            SendToOne(&data, good);
        }
        else if (!(sec >= SEC_GAMEMASTER) && bad->GetGUID() == m_ownerGUID && good != m_ownerGUID)
        {
            WorldPacket data;
            MakeNotOwner(&data);
            SendToOne(&data, good);
        }
        else
        {
            bool changeowner = (m_ownerGUID == bad->GetGUID());

            WorldPacket data;

            if (ban && !IsBanned(bad->GetGUID()))
            {
                banned.insert(bad->GetGUID());
                MakePlayerBanned(&data, bad->GetGUID(), good);
            }
            else
                MakePlayerKicked(&data, bad->GetGUID(), good);

            SendToAll(&data);
            players.erase(bad->GetGUID());
            bad->LeftChannel(this);

            if (changeowner)
                ChangeOwner();
        }
    }
}

void Channel::UnBan(uint64 good, const char *badname)
{
    uint8 sec = 0;
    Player *gplr = sObjectMgr.GetPlayer(good);
    if (gplr)
        sec = gplr->GetSession()->GetGMLevel();

    if (!IsOn(good))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, good);
    }
    else if (!players[good].IsModerator() && !(sec & SEC_GAMEMASTER))
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, good);
    }
    else
    {
        Player *bad = sObjectMgr.GetPlayer(badname);
        if (bad == NULL || !IsBanned(bad->GetGUID()))
        {
            WorldPacket data;
            MakePlayerNotFound(&data, badname);
            SendToOne(&data, good);
        }
        else
        {
            banned.erase(bad->GetGUID());

            WorldPacket data;
            MakePlayerUnbanned(&data, bad->GetGUID(), good);
            SendToAll(&data);
        }
    }
}

void Channel::Password(uint64 p, const char *pass)
{
    uint8 sec = 0;
    Player *plr = sObjectMgr.GetPlayer(p);
    if (plr)
        sec = plr->GetSession()->GetGMLevel();

    if (!IsOn(p))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, p);
    }
    else if (!players[p].IsModerator() && !(sec >= SEC_GAMEMASTER))
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, p);
    }
    else
    {
        m_password = pass;

        WorldPacket data;
        MakePasswordChanged(&data, p);
        SendToAll(&data);
    }
}

void Channel::SetMode(uint64 p, const char *p2n, bool mod, bool set)
{
    Player *plr = sObjectMgr.GetPlayer(p);
    if (!plr)
        return;

    uint8 sec = plr->GetSession()->GetGMLevel();

    if (!IsOn(p))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, p);
    }
    else if (!players[p].IsModerator() && !(sec >= SEC_GAMEMASTER))
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, p);
    }
    else
    {
        Player *newp = sObjectMgr.GetPlayer(p2n);
        if (!newp)
        {
            WorldPacket data;
            MakePlayerNotFound(&data, p2n);
            SendToOne(&data, p);
            return;
        }

        PlayerInfo inf = players[newp->GetGUID()];
        if (p == m_ownerGUID && newp->GetGUID() == m_ownerGUID && mod)
            return;

        if (!IsOn(newp->GetGUID()))
        {
            WorldPacket data;
            MakePlayerNotFound(&data, p2n);
            SendToOne(&data, p);
            return;
        }

        // allow make moderator from another team only if both is GMs
        // at this moment this only way to show channel post for GM from another team
        if ((!plr->GetSession()->HasHigherGMLevel(SEC_GAMEMASTER) || !newp->GetSession()->HasHigherGMLevel(SEC_GAMEMASTER)) &&
            plr->GetTeam() != newp->GetTeam() && !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
        {
            WorldPacket data;
            MakePlayerNotFound(&data, p2n);
            SendToOne(&data, p);
            return;
        }

        if (m_ownerGUID == newp->GetGUID() && m_ownerGUID != p)
        {
            WorldPacket data;
            MakeNotOwner(&data);
            SendToOne(&data, p);
            return;
        }

        if (mod)
            SetModerator(newp->GetGUID(), set);
        else
            SetMute(newp->GetGUID(), set);
    }
}

void Channel::SetOwner(uint64 p, const char *newname)
{
    Player *plr = sObjectMgr.GetPlayer(p);

    if (!plr)
        return;

    uint8 sec = plr->GetSession()->GetGMLevel();

    if (!IsOn(p))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, p);
        return;
    }

    if (!(sec >= SEC_GAMEMASTER) && p != m_ownerGUID)
    {
        WorldPacket data;
        MakeNotOwner(&data);
        SendToOne(&data, p);
        return;
    }

    Player *newp = sObjectMgr.GetPlayer(newname);
    if (newp == NULL || !IsOn(newp->GetGUID()))
    {
        WorldPacket data;
        MakePlayerNotFound(&data, newname);
        SendToOne(&data, p);
        return;
    }

    if (newp->GetTeam() != plr->GetTeam() && !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
    {
        WorldPacket data;
        MakePlayerNotFound(&data, newname);
        SendToOne(&data, p);
        return;
    }

    players[newp->GetGUID()].SetModerator(true);
    SetOwner(newp->GetGUID());
}

void Channel::SendWhoOwner(uint64 p)
{
    if (!IsOn(p))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, p);
    }
    else
    {
        WorldPacket data;
        MakeChannelOwner(&data);
        SendToOne(&data, p);
    }
}

void Channel::List(Player* player)
{
    // exploit fix
    if (player->InArena())
        return;

    uint64 p = player->GetGUID();

    if (!IsOn(p))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, p);
    }
    else
    {
        WorldPacket data(SMSG_CHANNEL_LIST, 1+(GetName().size()+1)+1+4+players.size()*(8+1));
        data << uint8(1);                                   // channel type?
        data << GetName();                                  // channel name
        data << uint8(0);                          // channel flags?

        size_t pos = data.wpos();
        data << uint32(0);                                  // size of list, placeholder

        bool gmInWhoList = sWorld.getConfig(CONFIG_GM_IN_WHO_LIST) || player->GetSession()->HasHigherGMLevel(SEC_DEVELOPER);

        uint32 count  = 0;
        for (PlayerList::iterator i = players.begin(); i != players.end(); ++i)
        {
            Player *user = sObjectMgr.GetPlayer(i->first);
            if (!user)
                continue;

            // PLAYER can't see MODERATOR, GAME MASTER, ADMINISTRATOR characters
            // MODERATOR, GAME MASTER, ADMINISTRATOR can see all
            if (!user->GetSession()->HasHigherGMLevel(SEC_GAMEMASTER) || gmInWhoList)
            {
                data << uint64(i->first);
                data << uint8(i->second.flags);             // flags seems to be changed...
                ++count;
            }
        }

        data.put<uint32>(pos,count);

        SendToOne(&data, p);
    }
}

void Channel::Announce(uint64 p)
{
    uint8 sec = 0;
    Player *plr = sObjectMgr.GetPlayer(p);
    if (plr)
        sec = plr->GetSession()->GetGMLevel();

    if (!IsOn(p))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, p);
    }
    else if (!players[p].IsModerator() && !(sec >= SEC_GAMEMASTER))
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, p);
    }
    else
    {
        m_announce = !m_announce;

        WorldPacket data;
        if (m_announce)
            MakeAnnouncementsOn(&data, p);
        else
            MakeAnnouncementsOff(&data, p);
        SendToAll(&data);
    }
}

void Channel::Moderate(uint64 p)
{
    uint8 sec = 0;
    Player *plr = sObjectMgr.GetPlayer(p);
    if (plr)
        sec = plr->GetSession()->GetGMLevel();

    if (!IsOn(p))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, p);
    }
    else if (!players[p].IsModerator() && !(sec >= SEC_GAMEMASTER))
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, p);
    }
    else
    {
        m_moderate = !m_moderate;

        WorldPacket data;
        if (m_moderate)
            MakeModerationOn(&data, p);
        else
            MakeModerationOff(&data, p);
        SendToAll(&data);
    }
}

void Channel::Say(uint64 p, const char *what, uint32 lang)
{
    if (!what)
        return;
    if (IsConstant() && ChatHandler::ContainsNotAllowedSigns(what,true))
        return;

    if (sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
        lang = LANG_UNIVERSAL;

    uint32 sec = 0;
    Player *plr = sObjectMgr.GetPlayer(p);
    if (plr)
        sec = plr->GetSession()->GetGMLevel();

    if (!IsOn(p))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, p);
    }
    else if (players[p].IsMuted())
    {
        WorldPacket data;
        MakeMuted(&data);
        SendToOne(&data, p);
    }
    else if (m_moderate && !players[p].IsModerator() && !(sec >= SEC_GAMEMASTER))
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, p);
    }
    else
    {
        uint32 messageLength = strlen(what) + 1;

        WorldPacket data(SMSG_MESSAGECHAT, 1+4+8+4+m_name.size()+1+8+4+messageLength+1);
        data << (uint8)CHAT_MSG_CHANNEL;
        data << (uint32)lang;
        data << p;                                          // 2.1.0
        data << uint32(0);                                  // 2.1.0
        data << m_name;
        data << p;
        data << messageLength;
        data << what;
        data << uint8(plr ? plr->chatTag() : 0);

        if (!plr || !plr->IsTrollmuted())
        {
            // exclude LFG and Trade from two-side channels
            if (sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL) && (IsLFG() || m_channelId == CHANNEL_ID_TRADE) && plr)
            {
                uint32 fromteam = plr->GetTeam();
                for (PlayerList::iterator i = players.begin(); i != players.end(); ++i)
                {
                    Player *to = sObjectMgr.GetPlayer(i->first);
                    if (!to || to->GetTeam() != fromteam)
                        continue;

                    if (!p || !to->GetSocial()->HasIgnore(GUID_LOPART(p)))
                        to->SendPacketToSelf(&data);
                }
            }
            else 
            SendToAll(&data, !players[p].IsModerator() ? p : false);
        }
        else
            plr->SendPacketToSelf(&data);
        
        if (lang == LANG_ADDON)
            return;

        switch (m_channelId)
        {
        case CHANNEL_ID_LFG:
            sLog.outChat(LOG_CHAT_LFG_A, plr->GetTeam(), plr->GetName(), what);
            break;
        case CHANNEL_ID_TRADE:
            sLog.outChat(LOG_CHAT_TRADE_A, plr->GetTeam(), plr->GetName(), what);
            break;
        case CHANNEL_ID_WORLD:
            sLog.outChat(LOG_CHAT_WORLD_A, plr->GetTeam(), plr->GetName(), what);
            break;
        case CHANNEL_ID_CUSTOM:
            break;
        default: // general, localdefense, worlddefense, guildrecruitment
            sLog.outChat(LOG_CHAT_LOCAL_A, plr->GetTeam(), plr->GetName(), what);
            break;
        }
    }
}

void Channel::Invite(uint64 p, const char *newname)
{
    if (!IsOn(p))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, p);
        return;
    }

    Player *newp = sObjectMgr.GetPlayer(newname);
    if (!newp)
    {
        WorldPacket data;
        MakePlayerNotFound(&data, newname);
        SendToOne(&data, p);
        return;
    }

    Player *plr = sObjectMgr.GetPlayer(p);
    if (!plr)
        return;

    if (newp->GetTeam() != plr->GetTeam() && !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
    {
        WorldPacket data;
        MakeInviteWrongFaction(&data);
        SendToOne(&data, p);
        return;
    }

    if (IsOn(newp->GetGUID()))
    {
        WorldPacket data;
        MakePlayerAlreadyMember(&data, newp->GetGUID());
        SendToOne(&data, p);
        return;
    }

    WorldPacket data;
    if (!newp->GetSocial()->HasIgnore(GUID_LOPART(p)))
    {
        MakeInvite(&data, p);
        SendToOne(&data, newp->GetGUID());
        data.clear();
    }
    MakePlayerInvited(&data, newp->GetName());
    SendToOne(&data, p);
}

void Channel::SetOwner(uint64 guid, bool exclaim)
{
    if (m_ownerGUID)
    {
        // [] will re-add player after it possible removed
        PlayerList::iterator p_itr = players.find(m_ownerGUID);
        if (p_itr != players.end())
            p_itr->second.SetOwner(false);
    }

    m_ownerGUID = guid;
    if (m_ownerGUID)
    {
        uint8 oldFlag = GetPlayerFlags(m_ownerGUID);
        players[m_ownerGUID].SetOwner(true);

        WorldPacket data;
        MakeModeChange(&data, m_ownerGUID, oldFlag);
        SendToAll(&data);

        if (exclaim)
        {
            MakeOwnerChanged(&data, m_ownerGUID);
            SendToAll(&data);
        }
    }
}

void Channel::SendToAll(WorldPacket *data, uint64 p)
{
    for (PlayerList::iterator i = players.begin(); i != players.end(); ++i)
    {
        Player *plr = sObjectMgr.GetPlayer(i->first);
        if (plr)
        {
            if (!p || !plr->GetSocial()->HasIgnore(GUID_LOPART(p)))
                plr->SendPacketToSelf(data);
        }
    }
}

void Channel::SendToAllButOne(WorldPacket *data, uint64 who)
{
    for (PlayerList::iterator i = players.begin(); i != players.end(); ++i)
    {
        if (i->first != who)
        {
            Player *plr = sObjectMgr.GetPlayer(i->first);
            if (plr)
                plr->SendPacketToSelf(data);
        }
    }
}

void Channel::SendToOne(WorldPacket *data, uint64 who)
{
    Player *plr = sObjectMgr.GetPlayer(who);
    if (plr)
        plr->SendPacketToSelf(data);
}

void Channel::Voice(uint64 /*guid1*/, uint64 /*guid2*/)
{

}

void Channel::DeVoice(uint64 /*guid1*/, uint64 /*guid2*/)
{

}

// done
void Channel::MakeNotifyPacket(WorldPacket *data, uint8 notify_type)
{
    data->Initialize(SMSG_CHANNEL_NOTIFY, 1+m_name.size()+1);
    *data << uint8(notify_type);
    *data << m_name;
}

// done 0x00
void Channel::MakeJoined(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_JOINED_NOTICE);
    *data << uint64(guid);
}

// done 0x01
void Channel::MakeLeft(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_LEFT_NOTICE);
    *data << uint64(guid);
}

// done 0x02
void Channel::MakeYouJoined(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_YOU_JOINED_NOTICE);
    *data << uint8(0);
    *data << uint32(GetChannelId());
    *data << uint32(0);
}

// done 0x03
void Channel::MakeYouLeft(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_YOU_LEFT_NOTICE);
    *data << uint32(GetChannelId());
    *data << uint8(0);                                      // can be 0x00 and 0x01
}

// done 0x04
void Channel::MakeWrongPassword(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_WRONG_PASSWORD_NOTICE);
}

// done 0x05
void Channel::MakeNotMember(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_NOT_MEMBER_NOTICE);
}

// done 0x06
void Channel::MakeNotModerator(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_NOT_MODERATOR_NOTICE);
}

// done 0x07
void Channel::MakePasswordChanged(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_PASSWORD_CHANGED_NOTICE);
    *data << uint64(guid);
}

// done 0x08
void Channel::MakeOwnerChanged(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_OWNER_CHANGED_NOTICE);
    *data << uint64(guid);
}

// done 0x09
void Channel::MakePlayerNotFound(WorldPacket *data, const std::string& name)
{
    MakeNotifyPacket(data, CHAT_PLAYER_NOT_FOUND_NOTICE);
    *data << name;
}

// done 0x0A
void Channel::MakeNotOwner(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_NOT_OWNER_NOTICE);
}

// done 0x0B
void Channel::MakeChannelOwner(WorldPacket *data)
{
    std::string name = "";

    if (!sObjectMgr.GetPlayerNameByGUID(m_ownerGUID, name) || name.empty())
        name = "PLAYER_NOT_FOUND";

    MakeNotifyPacket(data, CHAT_CHANNEL_OWNER_NOTICE);
    *data << ((IsConstant() || !m_ownerGUID) ? "Nobody" : name);
}

// done 0x0C
void Channel::MakeModeChange(WorldPacket *data, uint64 guid, uint8 oldflags)
{
    MakeNotifyPacket(data, CHAT_MODE_CHANGE_NOTICE);
    *data << uint64(guid);
    *data << uint8(oldflags);
    *data << uint8(GetPlayerFlags(guid));
}

// done 0x0D
void Channel::MakeAnnouncementsOn(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_ANNOUNCEMENTS_ON_NOTICE);
    *data << uint64(guid);
}

// done 0x0E
void Channel::MakeAnnouncementsOff(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_ANNOUNCEMENTS_OFF_NOTICE);
    *data << uint64(guid);
}

// done 0x0F
void Channel::MakeModerationOn(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_MODERATION_ON_NOTICE);
    *data << uint64(guid);
}

// done 0x10
void Channel::MakeModerationOff(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_MODERATION_OFF_NOTICE);
    *data << uint64(guid);
}

// done 0x11
void Channel::MakeMuted(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_MUTED_NOTICE);
}

// done 0x12
void Channel::MakePlayerKicked(WorldPacket *data, uint64 bad, uint64 good)
{
    MakeNotifyPacket(data, CHAT_PLAYER_KICKED_NOTICE);
    *data << uint64(bad);
    *data << uint64(good);
}

// done 0x13
void Channel::MakeBanned(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_BANNED_NOTICE);
}

// done 0x14
void Channel::MakePlayerBanned(WorldPacket *data, uint64 bad, uint64 good)
{
    MakeNotifyPacket(data, CHAT_PLAYER_BANNED_NOTICE);
    *data << uint64(bad);
    *data << uint64(good);
}

// done 0x15
void Channel::MakePlayerUnbanned(WorldPacket *data, uint64 bad, uint64 good)
{
    MakeNotifyPacket(data, CHAT_PLAYER_UNBANNED_NOTICE);
    *data << uint64(bad);
    *data << uint64(good);
}

// done 0x16
void Channel::MakePlayerNotBanned(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_PLAYER_NOT_BANNED_NOTICE);
    *data << uint64(guid);
}

// done 0x17
void Channel::MakePlayerAlreadyMember(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_PLAYER_ALREADY_MEMBER_NOTICE);
    *data << uint64(guid);
}

// done 0x18
void Channel::MakeInvite(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_INVITE_NOTICE);
    *data << uint64(guid);
}

// done 0x19
void Channel::MakeInviteWrongFaction(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_INVITE_WRONG_FACTION_NOTICE);
}

// done 0x1A
void Channel::MakeWrongFaction(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_WRONG_FACTION_NOTICE);
}

// done 0x1B
void Channel::MakeInvalidName(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_INVALID_NAME_NOTICE);
}

// done 0x1C
void Channel::MakeNotModerated(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_NOT_MODERATED_NOTICE);
}

// done 0x1D
void Channel::MakePlayerInvited(WorldPacket *data, const std::string& name)
{
    MakeNotifyPacket(data, CHAT_PLAYER_INVITED_NOTICE);
    *data << name;
}

// done 0x1E
void Channel::MakePlayerInviteBanned(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_PLAYER_INVITE_BANNED_NOTICE);
    *data << uint64(guid);
}

// done 0x1F
void Channel::MakeThrottled(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_THROTTLED_NOTICE);
}

// done 0x20
void Channel::MakeNotInArea(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_NOT_IN_AREA_NOTICE);
}

// done 0x21
void Channel::MakeNotInLfg(WorldPacket *data)
{
    MakeNotifyPacket(data, CHAT_NOT_IN_LFG_NOTICE);
}

// done 0x22
void Channel::MakeVoiceOn(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_VOICE_ON_NOTICE);
    *data << uint64(guid);
}

// done 0x23
void Channel::MakeVoiceOff(WorldPacket *data, uint64 guid)
{
    MakeNotifyPacket(data, CHAT_VOICE_OFF_NOTICE);
    *data << uint64(guid);
}

void Channel::JoinNotify(uint64 guid)
{
    WorldPacket data;

    if (IsConstant())
        data.Initialize(SMSG_USERLIST_ADD, 8+1+1+4+GetName().size()+1);
    else
        data.Initialize(SMSG_USERLIST_UPDATE, 8+1+1+4+GetName().size()+1);

    data << uint64(guid);
    data << uint8(GetPlayerFlags(guid));
    data << uint8(0);
    data << uint32(GetNumPlayers());
    data << GetName();
    SendToAll(&data);
}

void Channel::LeaveNotify(uint64 guid)
{
    WorldPacket data(SMSG_USERLIST_REMOVE, 8+1+4+GetName().size()+1);
    data << uint64(guid);
    data << uint8(0);
    data << uint32(GetNumPlayers());
    data << GetName();
    SendToAll(&data);
}

std::list<uint64> Channel::GetPlayers()
{
    std::list<uint64> tmpList;

    for (PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        tmpList.push_back(itr->second.player);

    return tmpList;
}

void Channel::ChangeOwner()
{
    uint64 newOwner = 0;

    // ignore GM for automatic owner change ;)
    for (PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
    {
        Player * tmpPlr = ObjectAccessor::GetPlayer(itr->second.player);
        if (tmpPlr && !tmpPlr->GetSession()->HasHigherGMLevel(SEC_GAMEMASTER))
        {
            newOwner = itr->second.player;
            break;
        }
    }

    SetOwner(newOwner);
}
