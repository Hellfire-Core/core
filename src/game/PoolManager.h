/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
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

#ifndef HELLGROUND_POOLMGR_H
#define HELLGROUND_POOLMGR_H

#include "ace/Singleton.h"

#include "Common.h"
#include "Platform/Define.h"
#include "Creature.h"
#include "GameObject.h"

struct PoolObject
{
    uint32  guid;
    float   chance;

    PoolObject(uint32 _guid, float _chance): guid(_guid), chance(fabs(_chance)) {}
};

typedef std::set<uint32> SpawnedPoolObjects;
typedef std::map<uint32,uint32> SpawnedPoolPools;

class SpawnedPoolData
{
    public:
        template<typename T>
        bool IsSpawnedObject(uint32 db_guid_or_pool_id) const;

        uint32 GetSpawnedObjects(uint32 pool_id) const;

        template<typename T>
        void AddSpawn(uint32 db_guid_or_pool_id, uint32 pool_id);

        template<typename T>
        void RemoveSpawn(uint32 db_guid_or_pool_id, uint32 pool_id);
    private:
        SpawnedPoolObjects mSpawnedCreatures;
        SpawnedPoolObjects mSpawnedGameobjects;
        SpawnedPoolPools   mSpawnedPools;
};

template <class T>
class PoolGroup
{
    typedef std::vector<PoolObject> PoolObjectList;
    public:
        explicit PoolGroup() : poolId(0), maxLimit(0) { }
        ~PoolGroup() {};
        void SetPoolId(uint32 pool_id) { poolId = pool_id; };
        void SetLimit(uint32 l) { maxLimit = l; };

        bool isEmpty() const { return ExplicitlyChanced.empty() && EqualChanced.empty(); }
        void AddEntry(PoolObject& poolitem);
        bool CheckPool() const;
        PoolObject* RollOne(SpawnedPoolData& spawns);
        void DespawnObject(SpawnedPoolData& spawns, uint32 guid=0);
        void Despawn1Object(uint32 guid);
        void SpawnObject(SpawnedPoolData& spawns, bool instantly);

        void Spawn1Object(PoolObject* obj, bool instantly);
        void ReSpawn1Object(PoolObject* obj);
        void RemoveOneRelation(uint16 child_pool_id);
    private:
        uint32 poolId;
        uint32 maxLimit;
        PoolObjectList ExplicitlyChanced;
        PoolObjectList EqualChanced;
};

class PoolManager
{
    friend class ACE_Singleton<PoolManager, ACE_Null_Mutex>;
    PoolManager();

    public:
        ~PoolManager() {};

        void LoadFromDB();
        void Initialize();

        template<typename T>
        uint16 IsPartOfAPool(uint32 db_guid_or_pool_id) const;

        bool IsSpawnedOrNotInPoolGameobject(uint32 db_guid) const;
        

        bool CheckPool(uint16 pool_id) const;
        void SpawnPool(uint16 pool_id, bool instantly);
        void DespawnPool(uint16 pool_id);

        template<typename T>
        void UpdatePool(uint16 pool_id);
    protected:
        template<typename T>
        void SpawnPoolGroup(uint16 pool_id, bool instantly);

        uint16 max_pool_id;

        typedef std::vector<PoolGroup<Creature> >   PoolGroupCreatureMap;
        typedef std::vector<PoolGroup<GameObject> > PoolGroupGameObjectMap;
        typedef std::vector<PoolGroup<void> >       PoolGroupPoolMap;
        typedef std::pair<uint32, uint16> SearchPair;
        typedef std::map<uint32, uint16> SearchMap;

        PoolGroupCreatureMap mPoolCreatureGroups;
        PoolGroupGameObjectMap mPoolGameobjectGroups;
        PoolGroupPoolMap mPoolPoolGroups;

        // static maps DB low guid -> pool id
        SearchMap mCreatureSearchMap;
        SearchMap mGameobjectSearchMap;
        SearchMap mPoolSearchMap;

        // dynamic data
        SpawnedPoolData mSpawnedData;
};

#define sPoolMgr (*ACE_Singleton<PoolManager, ACE_Null_Mutex>::instance())

// Method that tell if the creature is part of a pool and return the pool id if yes
template<>
inline uint16 PoolManager::IsPartOfAPool<Creature>(uint32 db_guid) const
{
    SearchMap::const_iterator itr = mCreatureSearchMap.find(db_guid);
    if (itr != mCreatureSearchMap.end())
        return itr->second;

    return 0;
}

// Method that tell if the gameobject is part of a pool and return the pool id if yes
template<>
inline uint16 PoolManager::IsPartOfAPool<GameObject>(uint32 db_guid) const
{
    SearchMap::const_iterator itr = mGameobjectSearchMap.find(db_guid);
    if (itr != mGameobjectSearchMap.end())
        return itr->second;

    return 0;
}

// Method that tell if the pool is part of another pool and return the pool id if yes
template<>
inline uint16 PoolManager::IsPartOfAPool<void>(uint32 pool_id) const
{
    SearchMap::const_iterator itr = mPoolSearchMap.find(pool_id);
    if (itr != mPoolSearchMap.end())
        return itr->second;

    return 0;
}

#endif
