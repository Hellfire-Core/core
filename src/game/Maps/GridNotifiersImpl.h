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

#ifndef _GRIDNOTIFIERSIMPL_H
#define _GRIDNOTIFIERSIMPL_H

#include "GridNotifiers.h"
#include "WorldPacket.h"
#include "Corpse.h"
#include "Player.h"
#include "UpdateData.h"
#include "CreatureAI.h"
#include "SpellAuras.h"

namespace MaNGOS
{

template<class T>
inline void VisibleNotifier::Visit(GridRefManager<T> &m)
{
    for(typename GridRefManager<T>::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        vis_guids.erase(iter->getSource()->GetGUID());
        _camera.UpdateVisibilityOf(iter->getSource(), i_data, i_visibleNow);
    }
}

inline void PlayerCreatureRelocationWorker(Player* p, Creature* c)
{
    if (!p->IsAlive() || !c->IsAlive())
        return;

    if (p->IsTaxiFlying() && !c->CanReactToPlayerOnTaxi())
        return;

    if (c->HasUnitState(UNIT_STAT_LOST_CONTROL | UNIT_STAT_IGNORE_ATTACKERS))
        return;

    // Creature AI reaction
    if ((c->HasReactState(REACT_AGGRESSIVE) || c->isTrigger()) && !c->IsInEvadeMode() && c->IsAIEnabled)
        c->AI()->MoveInLineOfSight_Safe(p);
}

inline void CreatureCreatureRelocationWorker(Creature* c1, Creature* c2)
{
    if (c1->HasUnitState(UNIT_STAT_LOST_CONTROL | UNIT_STAT_IGNORE_ATTACKERS))
        return;

    if (c2->HasUnitState(UNIT_STAT_IGNORE_ATTACKERS))
        return;

    // Creature AI reaction
    if ((c1->HasReactState(REACT_AGGRESSIVE) || c1->isTrigger()) && !c1->IsInEvadeMode() && c1->IsAIEnabled)
        c1->AI()->MoveInLineOfSight_Safe(c2);
}
/*
inline void PlayerRelocationNotifier::Visit(CameraMapType &m)
{
    for(CameraMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        iter->getSource()->UpdateVisibilityOf(&_player);

        // need to choose this or below one (this should be faster :P
        _player.GetCamera().UpdateVisibilityOf(iter->getSource()->GetBody());
    }

    //_player.GetCamera().UpdateVisibilityForOwner();
}
*/
inline void PlayerRelocationNotifier::Visit(CreatureMapType &m)
{
    for (CreatureMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
        PlayerCreatureRelocationWorker(&_player, iter->getSource());
}

inline void CreatureRelocationNotifier::Visit(PlayerMapType &m)
{
    if (!_creature.IsAlive())
        return;

    for (PlayerMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
        PlayerCreatureRelocationWorker(iter->getSource(), &_creature);
}

inline void CreatureRelocationNotifier::Visit(CreatureMapType &m)
{
    if (!_creature.IsAlive())
        return;

    for (CreatureMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        CreatureCreatureRelocationWorker(iter->getSource(), &_creature);
        CreatureCreatureRelocationWorker(&_creature, iter->getSource());
    }
}

typedef std::list<Creature*> UpdateList;

struct UpdateListSorter : public std::binary_function<Creature*, Creature*, bool>
{
    // functor for operator ">"
    bool operator()(Creature* left, Creature* right) const
    {
        return left->GetUpdateCounter() > right->GetUpdateCounter();
    }
};

inline void ObjectUpdater::Visit(CreatureMapType &m)
{
    UpdateList updateList;
    for (CreatureMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        if (WorldObject::UpdateHelper::ProcessUpdate(iter->getSource()))
            updateList.push_back(iter->getSource());
    }

    uint32 maxListSize = sWorld.getConfig(CONFIG_MAPUPDATE_MAXVISITORS);
    if (maxListSize && maxListSize < updateList.size())
    {
        // sort list (objects updated old time ago will be first)
        updateList.sort(UpdateListSorter());
        updateList.resize(sWorld.getConfig(CONFIG_MAPUPDATE_MAXVISITORS), NULL); // set initial value for added elements to NULL
        
        for (UpdateList::iterator it = updateList.begin(); it != updateList.end(); ++it)
        {
            WorldObject::UpdateHelper helper(*it);
            if (maxListSize > 0)
            {
                helper.Update(i_timeDiff);
                maxListSize--;
            }
            else
                helper.NoUpdate(i_timeDiff); // just tell then they wont be updated
        }
        return;
    }

    for (UpdateList::iterator it = updateList.begin(); it != updateList.end(); ++it)
    {
        WorldObject::UpdateHelper helper(*it);
        helper.Update(i_timeDiff);
    }
}

template<class T, class Check>
void ObjectSearcher<T, Check>::Visit(GridRefManager<T>& m)
{
    // already found
    if (_object)
        return;

    for (typename GridRefManager<T>::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (_check(itr->getSource()))
        {
            _object = itr->getSource();
            return;
        }
    }
}

template<class T, class Check>
void ObjectLastSearcher<T, Check>::Visit(GridRefManager<T>& m)
{
    for (typename GridRefManager<T>::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (_check(itr->getSource()))
            _object = itr->getSource();
    }
}

template<class T, class Check>
void ObjectListSearcher<T, Check>::Visit(GridRefManager<T>& m)
{
    for (typename GridRefManager<T>::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (_check(itr->getSource()))
            _objects.push_back(itr->getSource());
    }
}

// Unit searchers
template<class Check>
void UnitSearcher<Check>::Visit(CreatureMapType &m)
{
    // already found
    if (i_object)
        return;

    for (CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if (i_check(itr->getSource()))
        {
            i_object = itr->getSource();
            return;
        }
    }
}

template<class Check>
void UnitSearcher<Check>::Visit(PlayerMapType &m)
{
    // already found
    if (i_object)
        return;

    for (PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if (i_check(itr->getSource()))
        {
            i_object = itr->getSource();
            return;
        }
    }
}

template<class Check>
void UnitLastSearcher<Check>::Visit(CreatureMapType &m)
{
    for (CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if (i_check(itr->getSource()))
            i_object = itr->getSource();
    }
}

template<class Check>
void UnitLastSearcher<Check>::Visit(PlayerMapType &m)
{
    for (PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if (i_check(itr->getSource()))
            i_object = itr->getSource();
    }
}

template<class Check>
void UnitListSearcher<Check>::Visit(PlayerMapType &m)
{
    for (PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if (i_check(itr->getSource()))
            i_objects.push_back(itr->getSource());
}

template<class Check>
void UnitListSearcher<Check>::Visit(CreatureMapType &m)
{
    for (CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if (i_check(itr->getSource()))
            i_objects.push_back(itr->getSource());
}

}

#endif
