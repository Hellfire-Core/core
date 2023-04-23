/*
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
#ifndef _COOLMGR_H
#define _COOLMGR_H

#include "Common.h"
#include "DatabaseEnv.h"

// spellid for comands
#define COMMAND_COOLDOWN 2
// category cooldown pets gcd
#define PETS_GCD_CATEGORY 134

class Player;
class ByteBuffer;

class CooldownMgr
{
    friend class Player; // for RemoveAllSpellCooldowns, RemoveArenaSpellCooldowns
public:
    CooldownMgr() {}
    struct Cooldown
    {
        Cooldown(uint32 s = 0, uint32 d = 0) : start(s), duration(d) {};
        uint32 start;
        uint32 duration;
    };
    typedef std::map<uint32, Cooldown> CooldownList;

    bool HasGlobalCooldown(uint32 id) const;
    void AddGlobalCooldown(uint32 id, uint32 ms);
    void CancelGlobalCooldown(uint32 id);

    bool HasSpellCooldown(uint32 id) const;
    void AddSpellCooldown(uint32 id, uint32 ms);
    void CancelSpellCooldown(uint32 id);
    uint32 GetCooldownTimeLeft(uint32 id) const;

    void AddItemCooldown(uint32 item, uint32 ms);
    bool HasItemCooldown(uint32 item) const;

    std::string SendCooldownsDebug();
    void WriteCooldowns(ByteBuffer& bb);
    void LoadFromDB(QueryResultAutoPtr result);
    void SaveToDB(uint32 playerguid);
private:
    CooldownList m_SpellCooldowns;
    CooldownList m_ItemCooldowns;
    CooldownList m_GlobalCooldowns;
};

#endif