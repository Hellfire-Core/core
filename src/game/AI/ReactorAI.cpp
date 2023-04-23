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

#include "ByteBuffer.h"
#include "ReactorAI.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "CreatureAIImpl.h"

#define REACTOR_VISIBLE_RANGE (26.46f)

int
ReactorAI::Permissible(const Creature *creature)
{
    if (creature->isCivilian() || creature->IsNeutralToAll())
        return PERMIT_BASE_REACTIVE;

    return PERMIT_BASE_NO;
}

void
ReactorAI::MoveInLineOfSight(Unit *)
{
}

void
ReactorAI::UpdateAI(const uint32 /*time_diff*/)
{
    // update i_victimGuid if m_creature->GetVictim() != 0 and changed
    if (!UpdateVictim())
        return;

    if (m_creature->isAttackReady())
    {
        if (m_creature->IsWithinMeleeRange(m_creature->GetVictim()))
        {
            m_creature->AttackerStateUpdate(m_creature->GetVictim());
            m_creature->resetAttackTimer();
        }
    }
}