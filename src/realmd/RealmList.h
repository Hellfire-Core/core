/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
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

/// \addtogroup realmd
/// @{
/// \file

#ifndef _REALMLIST_H
#define _REALMLIST_H

#include "Common.h"

struct RealmBuildInfo
{
    int build;
    int major_version;
    int minor_version;
    int bugfix_version;
    int hotfix_version;
};

RealmBuildInfo const* FindBuildInfo(uint16 _build);

typedef std::set<uint32> RealmBuilds;

/// Storage object for a realm
struct Realm
{
    std::string address;
    uint8 icon;
    RealmFlags realmflags;                                  // realmflags
    uint8 timezone;
    uint32 m_ID;
    uint8 requiredGMLevel;                                  // current minimum join permission mask requirements (show as locked for not fit accounts)
    float populationLevel;
    RealmBuilds realmbuilds;                                // list of supported builds (updated in DB by mangosd)
    RealmBuildInfo realmBuildInfo;                          // build info for show version in list
};

/// Storage object for the list of realms on the server
class RealmList
{
    public:
        typedef std::map<std::string, Realm> RealmMap;

        static RealmList& Instance();

        RealmList();
        ~RealmList() {}

        void Initialize();

        void UpdateIfNeed();

        RealmMap::const_iterator begin() const { return m_realms.begin(); }
        RealmMap::const_iterator end() const { return m_realms.end(); }
        uint32 size() const { return m_realms.size(); }
        std::string GetChatboxOsName() const {return m_ChatboxOsName;}
        uint32 GetWrongPassCount() const {return m_WrongPassCount;}
        uint32 GetWrongPassBanTime() const {return m_WrongPassBanTime;}
        bool GetWrongPassBanType() const {return m_WrongPassBanType;}
    private:
        void UpdateRealms(bool init);
        void UpdateRealm(uint32 ID, const std::string& name, const std::string& address, uint32 port, uint8 icon, RealmFlags realmflags, uint8 timezone, uint8 requiredGMLevel, float popu, const std::string& builds);
    private:
        RealmMap m_realms;                                  /// Internal map of realms
        uint32   m_UpdateInterval;
        time_t   m_NextUpdateTime;
        std::string m_ChatboxOsName;
        uint32   m_WrongPassCount;
        uint32   m_WrongPassBanTime;
        bool     m_WrongPassBanType;
};

#define sRealmList RealmList::Instance()

#endif
/// @}
