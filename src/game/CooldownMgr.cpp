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

#include "CooldownMgr.h"
#include "ByteBuffer.h"
#include "Timer.h"
#include "DBCStores.h"
#include "ObjectMgr.h"

bool CooldownMgr::HasGlobalCooldown(uint32 id) const
{
    if (id)
    {
        CooldownList::const_iterator itr = m_GlobalCooldowns.find(id);
        if (itr != m_GlobalCooldowns.end() &&
            WorldTimer::getMSTimeDiff(WorldTimer::getMSTime(), itr->second.start) < itr->second.duration)
            return true;
    }
    return false;
};

void CooldownMgr::AddGlobalCooldown(uint32 id, uint32 ms)
{
    if (id)
        m_GlobalCooldowns[id] = Cooldown(WorldTimer::getMSTime(), ms);
};

void CooldownMgr::CancelGlobalCooldown(uint32 id)
{
    if (id)
        m_GlobalCooldowns[id].duration = 0;
};

bool CooldownMgr::HasSpellCooldown(uint32 id) const
{
    if (id)
    {
        CooldownList::const_iterator itr = m_SpellCooldowns.find(id);
        if (itr != m_SpellCooldowns.end() &&
            WorldTimer::getMSTimeDiff(WorldTimer::getMSTime(), itr->second.start) < itr->second.duration)
            return true;
    }
    return false;
};

void CooldownMgr::AddSpellCooldown(uint32 id, uint32 ms)
{
    if (!id)
        return;

    CooldownList::const_iterator itr = m_SpellCooldowns.find(id);
    if (itr != m_SpellCooldowns.end())
    {
        uint32 diff = WorldTimer::getMSTimeDiff(WorldTimer::getMSTime(), itr->second.start);
        if (itr->second.duration > ms + diff)
            return; // do not overwrite longer one
    }
    m_SpellCooldowns[id] = Cooldown(WorldTimer::getMSTime(), ms);
};

void CooldownMgr::CancelSpellCooldown(uint32 id)
{
    if (id)
        m_SpellCooldowns[id].duration = 0;
};

uint32 CooldownMgr::GetCooldownTimeLeft(uint32 id) const
{
    CooldownList::const_iterator itr = m_SpellCooldowns.find(id);
    if (itr == m_SpellCooldowns.end())
        return 0;
    uint32 diff = WorldTimer::getMSTimeDiff(WorldTimer::getMSTime(), itr->second.start);
    if (diff >= itr->second.duration)
        return 0;
    else
        return itr->second.duration - diff;
};

void CooldownMgr::AddItemCooldown(uint32 item, uint32 ms)
{
    if (!item)
        return;

    CooldownList::const_iterator itr = m_ItemCooldowns.find(item);
    if (itr != m_ItemCooldowns.end())
    {
        uint32 diff = WorldTimer::getMSTimeDiff(WorldTimer::getMSTime(), itr->second.start);
        if (itr->second.duration > ms + diff)
            return; // do not overwrite longer one
    }
    m_ItemCooldowns[item] = Cooldown(WorldTimer::getMSTime(), ms);
}

bool CooldownMgr::HasItemCooldown(uint32 item) const
{
    if (item)
    {
        CooldownList::const_iterator itr = m_ItemCooldowns.find(item);
        if (itr != m_ItemCooldowns.end() &&
            WorldTimer::getMSTimeDiff(WorldTimer::getMSTime(), itr->second.start) < itr->second.duration)
            return true;
    }
    return false;
};

std::string CooldownMgr::SendCooldownsDebug()
{
    std::ostringstream str;
    uint32 now = WorldTimer::getMSTime();
    for (CooldownList::const_iterator itr = m_SpellCooldowns.begin(); itr != m_SpellCooldowns.end(); itr++)
    {
        uint32 diff = WorldTimer::getMSTimeDiff(now, itr->second.start);
        if (diff < itr->second.duration)
            str << itr->first << ": " << uint32(itr->second.duration - diff) << "ms; ";
    }
    str << "\nItem cooldowns\n";
    for (CooldownList::const_iterator itr = m_ItemCooldowns.begin(); itr != m_ItemCooldowns.end(); itr++)
    {
        uint32 diff = WorldTimer::getMSTimeDiff(now, itr->second.start);
        if (diff < itr->second.duration)
            str << itr->first << ": " << uint32(itr->second.duration - diff) << "ms; ";
    }
    // category cooldowns
    return str.str();
}

void CooldownMgr::WriteCooldowns(ByteBuffer& bb)
{
    uint16 size = m_SpellCooldowns.size() + m_ItemCooldowns.size();
    bb.reserve(size*14 + 2);
    bb << uint16(size);
    uint32 now = WorldTimer::getMSTime();

    for (CooldownList::const_iterator itr=m_SpellCooldowns.begin(); itr!=m_SpellCooldowns.end(); ++itr)
    {
        SpellEntry const *sEntry = sSpellStore.LookupEntry(itr->first);
        if (!sEntry)
        continue;

        bb << uint16(itr->first);

        uint32 diff = WorldTimer::getMSTimeDiff(now, itr->second.start);
        if (diff >= itr->second.duration)
            diff = itr->second.duration;

        bb << uint16(0);                                // cast item id
        bb << uint16(sEntry->Category);                 // spell category
        if (sEntry->Category)
        {
            bb << uint32(0);                            // cooldown
            bb << uint32(itr->second.duration - diff);  // category cooldown
        }
        else
        {
            bb << uint32(itr->second.duration - diff);  // cooldown
            bb << uint32(0);                            // category cooldown
        }
    }
    for (CooldownList::const_iterator itr = m_ItemCooldowns.begin(); itr != m_ItemCooldowns.end(); ++itr)
    {
        const ItemPrototype* ip = sObjectMgr.GetItemPrototype(itr->first);
        if (!ip)
            continue;

        uint8 i;
        for (i = 0; i < MAX_ITEM_PROTO_SPELLS; i++)
        {
            if (ip->Spells[i].SpellId != 0 &&
                (ip->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE ||
                ip->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_NO_DELAY_USE))
            {
                break;
            }
        }
        if (i == MAX_ITEM_PROTO_SPELLS)
            continue;

        bb << uint16(ip->Spells[i].SpellId);

        uint32 diff = WorldTimer::getMSTimeDiff(now, itr->second.start);
        if (diff >= itr->second.duration)
            diff = itr->second.duration;

        bb << uint16(itr->first);                       // cast item id
        bb << uint16(ip->Spells[i].SpellCategory);      // spell category
        if (ip->Spells[i].SpellCategory)
        {
            bb << uint32(0);                            // cooldown
            bb << uint32(itr->second.duration - diff);  // category cooldown
        }
        else
        {
            bb << uint32(itr->second.duration - diff);  // cooldown
            bb << uint32(0);                            // category cooldown
        }
    }
}

void CooldownMgr::LoadFromDB(QueryResultAutoPtr result)
{
    m_SpellCooldowns.clear();
    m_ItemCooldowns.clear();
    if (result)
    {
        time_t curTime = time(NULL);

        do
        {
            Field *fields = result->Fetch();

            int32 spell_id = fields[0].GetUInt32();
            int32 item_id = fields[1].GetUInt32();
            time_t db_time = (time_t)fields[2].GetUInt64();

            // skip outdated cooldown
            if (db_time <= curTime)
                continue;

            if (spell_id)
            {
                if (!sSpellStore.LookupEntry(spell_id) &&
                    spell_id != COMMAND_COOLDOWN)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: Player have unknown spell %u in `character_spell_cooldown`, skipping.", spell_id);
                    continue;
                }
                AddSpellCooldown(spell_id, (db_time - curTime)*IN_MILISECONDS);
            }
            else
                AddItemCooldown(item_id, (db_time - curTime)*IN_MILISECONDS);
        } while (result->NextRow());
    }
}

void CooldownMgr::SaveToDB(uint32 playerguid)
{
    RealmDataDatabase.PExecute("DELETE FROM character_spell_cooldown WHERE guid = '%u'", playerguid);

    uint32 now = WorldTimer::getMSTime();
    time_t curTime = time(NULL);

    // remove outdated and save active
    for (CooldownList::iterator itr = m_SpellCooldowns.begin(); itr != m_SpellCooldowns.end();)
    {
        uint32 diff = WorldTimer::getMSTimeDiff(now, itr->second.start);
        if (diff >= itr->second.duration)
            m_SpellCooldowns.erase(itr++);
        else if ((itr->second.duration - diff) > 7 * IN_MILISECONDS) // skip shorter than 7sec
        {
            RealmDataDatabase.PExecute("INSERT INTO character_spell_cooldown (guid,spell,item,time) VALUES ('%u', '%u', '%u', '" UI64FMTD "')",
                playerguid, itr->first, 0, curTime + uint64((itr->second.duration - diff)/1000)); // store just seconds
            ++itr;
        }
        else 
            itr++;
    }
    for (CooldownList::iterator itr = m_ItemCooldowns.begin(); itr != m_ItemCooldowns.end();)
    {
        uint32 diff = WorldTimer::getMSTimeDiff(now, itr->second.start);
        if (diff >= itr->second.duration)
            m_ItemCooldowns.erase(itr++);
        else if ((itr->second.duration - diff) > 7 * IN_MILISECONDS) // skip shorter than 7sec
        {
            RealmDataDatabase.PExecute("INSERT INTO character_spell_cooldown (guid,spell,item,time) VALUES ('%u', '%u', '%u', '" UI64FMTD "')",
                playerguid, 0, itr->first, curTime + uint64((itr->second.duration - diff) / 1000)); // store just seconds
            ++itr;
        }
        else
            itr++;
    }
}
