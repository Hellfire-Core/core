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

/** \file
    \ingroup Trinityd
*/

#include "WorldSocketMgr.h"
#include "Common.h"
#include "World.h"
#include "WorldRunnable.h"
#include "Timer.h"
#include "ObjectAccessor.h"
#include "MapManager.h"
#include "BattleGroundMgr.h"
#include "InstanceSaveMgr.h"

#include "Database/DatabaseEnv.h"

/// Heartbeat for the World
void WorldRunnable::run()
{
    ///- Init new SQL thread for the world database
    GameDataDatabase.ThreadStart();                            // let thread do safe mySQL requests (one connection call enough)
    sWorld.InitResultQueue();

    WorldTimer::tick(); //initialize world timer
    uint32 desiredTickTime = sWorld.getConfig(CONFIG_WORLD_SLEEP);
    ///- While we have not World::m_stopEvent, update the world
    while (!World::IsStopped())
    {
        ++World::m_worldLoopCounter;
        uint32 diff = WorldTimer::tick();
        if (diff < desiredTickTime)
        {
            ACE_Based::Thread::Sleep(desiredTickTime - diff);
            WorldTimer::tickTimeRenew(); // need to update current time after sleep
            sWorld.Update(desiredTickTime);
        }
        else
            sWorld.Update(diff);
    }

    sLog.outBasic("Shutting down world...");
    sWorld.Shutdown();

    // unload battleground templates before different singletons destroyed
    sBattleGroundMgr.DeleteAllBattleGrounds();

    sWorldSocketMgr->StopNetwork();
    sMapMgr.UnloadAll();                                    // unload all grids (including locked in memory)

    ///- End the database thread
    GameDataDatabase.ThreadEnd();                              // free mySQL thread resources
}
