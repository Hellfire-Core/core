/*
 * Copyright (C) 2005-2008 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008 TrinityCore <http://www.trinitycore.org/>
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

#ifndef HELLGROUND_INSTANCE_DATA_H
#define HELLGROUND_INSTANCE_DATA_H

#include <vector>

#include "ZoneScript.h"
//#include "GameObject.h"
#include "Map.h"
#include "Player.h"

class Map;
class Unit;
class Player;
class GameObject;
class Creature;

enum EncounterState
{
    NOT_STARTED   = 0,
    IN_PROGRESS   = 1,
    FAIL          = 2,
    DONE          = 3,
    SPECIAL       = 4,
    TO_BE_DECIDED = 5,
};

typedef std::set<GameObject*> DoorSet;

enum DoorType
{
    DOOR_TYPE_ROOM = 0,
    DOOR_TYPE_PASSAGE,
    MAX_DOOR_TYPES,
};

struct BossInfo
{
    BossInfo() : state(TO_BE_DECIDED) {}
    EncounterState state;
    DoorSet door[MAX_DOOR_TYPES];
};

struct DoorInfo
{
    explicit DoorInfo(BossInfo *_bossInfo, DoorType _type)
        : bossInfo(_bossInfo), type(_type) {}
    BossInfo *bossInfo;
    DoorType type;
};

typedef std::multimap<uint32 /*entry*/, DoorInfo> DoorInfoMap;

struct DoorData
{
    uint32 entry, bossId;
    DoorType type;
};

class HELLGROUND_IMPORT_EXPORT InstanceData : public ZoneScript
{
    public:

        explicit InstanceData(Map *map) : instance(map) {}
        virtual ~InstanceData() {}

        Map *instance;

        //On creation, NOT load.
        virtual void Initialize() {}

        //On load
        virtual void Load(const char * data) { LoadBossState(data); }

        //When save is needed, this function generates the data
        virtual std::string GetSaveData() { return GetBossSaveData(); }

        void SaveToDB();

        virtual void Update(uint32 /*diff*/) {}

        //Used by the map's CanEnter function.
        //This is to prevent players from entering during boss encounters.
        virtual bool IsEncounterInProgress() const;
        virtual void ResetEncounterInProgress();

        //Called when a player successfully enters the instance.
        virtual void OnPlayerEnter(Player *) {}

        //Called when a player deaths in the instance.
        virtual void OnPlayerDeath(Player *) {}

        //Called when player deals damage/heals someone. Mainly for pve logs.
        virtual void OnPlayerDealDamage(Player*, uint32) {};
        virtual void OnPlayerHealDamage(Player*, uint32) {};

        //Called on creature death (after CreatureAI::JustDied)
        virtual void OnCreatureDeath(Creature *) {}

        //Called when a gameobject is created
        void OnGameObjectCreate(GameObject *go, bool add) { if (add) OnObjectCreate(go); }

        //called on creature creation
        void OnCreatureCreate(Creature *, bool add);

        //Handle open / close objects
        //use HandleGameObject(NULL,boolen,GO); in OnObjectCreate in instance scripts
        //use HandleGameObject(GUID,boolen,NULL); in any other script
        void HandleGameObject(uint64 GUID, bool open, GameObject *go = NULL);

        virtual bool SetBossState(uint32 id, EncounterState state);

        Creature *GetCreature(uint64 guid){ return instance->GetCreature(guid); }

        virtual uint32 GetEncounterForEntry(uint32 entry) { return 0; }
        virtual uint32 GetRequiredEncounterForEntry(uint32 entry) { return 0; }

        virtual void HandleInitCreatureState(Creature * mob);
        virtual void HandleRequiredEncounter(uint32 encounter);

        void LogPossibleCheaters(const char* cheatName);
    protected:
        void SetBossNumber(uint32 number) { bosses.resize(number); }
        void LoadDoorData(const DoorData *data);

        void AddDoor(GameObject *door, bool add);
        void UpdateDoorState(GameObject *door);

        std::string LoadBossState(const char * data);
        std::string GetBossSaveData();

        UNORDERED_MAP<uint32, std::vector<uint64> > requiredEncounterToMobs;

    private:
        std::vector<BossInfo> bosses;
        DoorInfoMap doors;


        virtual void OnObjectCreate(GameObject *) {}
        virtual void OnCreatureCreate(Creature *, uint32 entry) {}
};

#define GBK_REQUIRED_AMOUNT (float)0.75

class GBK_handler
{
    struct GBKStats
    {
        GBKStats() : healing(0), damage(0), deaths(0) {};
        uint32 healing;
        uint32 damage;
        uint8 deaths;
    };
public:
    GBK_handler(Map* map) : m_timer(0), m_encounter(GBK_NONE), m_map(map) {};

    void HealingDone(uint32 guid, uint32 amount)
    {
        if (m_encounter != GBK_NONE)
            stats[guid].healing += amount;
    };
    void DamageDone(uint32 guid, uint32 amount)
    {
        if (m_encounter != GBK_NONE)
            stats[guid].damage += amount;
    };
    void PlayerDied(uint32 guid)
    {
        if (m_encounter != GBK_NONE)
            stats[guid].deaths++;
    };

    void StopCombat(GBK_Encounters encounter, bool win)
    {
        if (m_encounter == GBK_ANTISPAMINLOGSINATOR)
            return;

        if (win)
        {
            if (m_encounter == GBK_NONE || m_encounter != encounter)
            {
                sLog.outLog(LOG_DEFAULT, "GBK_handler: problems in StopCombat(%u,%u), m_encouter %u Map %u InstanceId %u",
                    uint32(encounter), win, uint32(m_encounter), m_map->GetId(), m_map->GetInstanceId());
                m_timer = 0;
                stats.clear();
                m_encounter = GBK_ANTISPAMINLOGSINATOR;
                return;
            }

            uint32 guild_id = 0;
            uint32 totalcount = 0;
            std::map<uint32, uint32> guilds;
            Map::PlayerList const& list = m_map->GetPlayers();
            for (Map::PlayerList::const_iterator i = list.begin(); i != list.end(); ++i)
            {
                if (Player* plr = i->getSource())
                {
                    guilds[plr->GetGuildId()]++;
                    totalcount++;
                }
            }
            for (std::map<uint32, uint32>::iterator mitr = guilds.begin(); mitr != guilds.end(); mitr++)
            {
                if (mitr->second >= GBK_REQUIRED_AMOUNT * totalcount)
                {
                    guild_id = mitr->first;
                    break;
                }
            }          
            if (guild_id)
            {   // to get the last record before inserting our result
                QueryResultAutoPtr record = RealmDataDatabase.PQuery(
                    "SELECT `length` FROM boss_fights WHERE mob_id = %u ORDER BY LENGTH ASC LIMIT 1", uint32(m_encounter));

                RealmDataDatabase.DirectPExecute("INSERT INTO boss_fights VALUES (NULL,%u,%u,%u,%u,SYSDATE())",
                    uint32(m_encounter), m_map->GetInstanceId(), guild_id, uint32(time(NULL) - m_timer));
                QueryResultAutoPtr result = RealmDataDatabase.PQuery(
                    "SELECT kill_id, MAX('date') FROM boss_fights WHERE instance_id = %u AND mob_id = %u",
                    m_map->GetInstanceId(), uint32(m_encounter));
                if (!result)
                {
                    sLog.outLog(LOG_DEFAULT,"GBK_handler: StopCombat - problem getting kill id, guild %u Map %u InstanceId %u",
                        guild_id,m_map->GetId(),m_map->GetInstanceId());
                    m_encounter = GBK_NONE;
                    m_timer = 0;
                    stats.clear();
                    return;
                }
                uint32 kill_id = result->Fetch()[0].GetUInt32();
                RealmDataDatabase.BeginTransaction();
                for (Map::PlayerList::const_iterator i = list.begin(); i != list.end(); ++i)
                {
                    if (Player* plr = i->getSource())
                    {
                        if (plr->GetGuildId() == guild_id)
                        {
                            RealmDataDatabase.PExecute("INSERT INTO boss_fights_detailed VALUES (%u,%u,%u,%u,%u)",
                                kill_id, plr->GetGUIDLow(), stats[plr->GetGUIDLow()].damage,
                                stats[plr->GetGUIDLow()].healing, stats[plr->GetGUIDLow()].deaths);
                        }
                    }
                }
                RealmDataDatabase.PExecute("INSERT INTO boss_fights_loot "
                    "(SELECT \"%u\", itemId, SUM(itemCount) FROM group_saved_loot WHERE instanceId = %u GROUP BY itemId);",
                    kill_id,m_map->GetInstanceId());
                RealmDataDatabase.CommitTransaction();

                
                if (record) //TODO: add disable/enable to config
                {
                    uint32 last_record = record->Fetch()[0].GetUInt32();
                    if (last_record > uint32(time(NULL) - m_timer))
                    {
                        QueryResultAutoPtr names = RealmDataDatabase.PQuery(
                            "SELECT g.name, n.boss_name FROM boss_fights f JOIN guild g ON g.guildid = f.guild_id JOIN boss_id_names n ON n.boss_id = f.mob_id "
                            "WHERE kill_id = %u", kill_id);
                        if (names)
                        {
                            std::string message = "New server record: " + std::to_string(uint32(time(NULL) - m_timer)) + " seconds (last record: "
                                + std::to_string(last_record) + " seconds) for boss " + names->Fetch()[1].GetCppString()
                                + " by guild <" + names->Fetch()[0].GetCppString() + ">.";
                            
                            sWorld.SendServerMessage(SERVER_MSG_STRING, message.c_str());
                        }
                    }
                }
            }
        }

        if (m_encounter == encounter) //do not reset timers when some boss is just spamming not_started
        {
            m_encounter = GBK_NONE;
            m_timer = 0;
            stats.clear();
        }
    }

    void StartCombat(GBK_Encounters encounter)
    {
        if (m_encounter != GBK_NONE && m_encounter != encounter)
        {
            if (m_encounter != GBK_ANTISPAMINLOGSINATOR)
            {
                sLog.outLog(LOG_DEFAULT, "GBK_handler: StartCombat(%u) while already in combat(%u) Map %u InstanceId %u",
                    encounter, m_encounter, m_map->GetId(), m_map->GetInstanceId());
                m_encounter = GBK_ANTISPAMINLOGSINATOR;
            }
            return;
        }
        else if (m_encounter == encounter)
            return; // combat in progress anyway, just dance

        m_encounter = encounter;
        m_timer = time(NULL);
    }

private:
    GBK_Encounters m_encounter;
    time_t m_timer;
    std::map<uint32, GBKStats> stats;
    Map* m_map;
};

#endif
