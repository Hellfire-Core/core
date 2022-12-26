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

#include "CreatureAI.h"
#include "CreatureAIImpl.h"
#include "Creature.h"
#include "World.h"
#include "SpellMgr.h"
#include "Chat.h"

void CreatureAI::OnCharmed(bool apply)
{
    if (!(m_creature->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_CHARM_AI))
    {
        me->NeedChangeAI = true;
        me->IsAIEnabled = false;
    }
}

void CreatureAI::DoZoneInCombat(float max_dist)
{
     Unit *creature = me;

    if (!me->CanHaveThreatList() || me->IsInEvadeMode() || !me->IsAlive())
        return;

    Map *pMap = me->GetMap();
    if (!pMap->IsDungeon())                                  //use IsDungeon instead of Instanceable, in case battlegrounds will be instantiated
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: DoZoneInCombat call for map that isn't an instance (creature entry = %d)", creature->GetTypeId() == TYPEID_UNIT ? ((Creature*)creature)->GetEntry() : 0);
        return;
    }

    Map::PlayerList const &plList = pMap->GetPlayers();
    if (plList.isEmpty())
        return;

    for (Map::PlayerList::const_iterator i = plList.begin(); i != plList.end(); ++i)
    {
        if (Player* pPlayer = i->getSource())
        {
            if (pPlayer->IsGameMaster() || pPlayer->IsFriendlyTo(me))
                continue;

            if (pPlayer->IsAlive() && me->IsWithinDistInMap(pPlayer, max_dist))
            {
                me->SetInCombatWith(pPlayer);
                pPlayer->SetInCombatWith(me);
                me->AddThreat(pPlayer, 0.0f);
            }
        }
    }
}

// scripts does not take care about MoveInLineOfSight loops
// MoveInLineOfSight can be called inside another MoveInLineOfSight and cause stack overflow
void CreatureAI::MoveInLineOfSight_Safe(Unit *who)
{
    if (m_MoveInLineOfSight_locked == true)
        return;

    m_MoveInLineOfSight_locked = true;
    MoveInLineOfSight(who);
    m_MoveInLineOfSight_locked = false;
}

void CreatureAI::MoveInLineOfSight(Unit *who)
{
    if (me->GetVictim())
        return;

    if (me->canStartAttack(who))
    {
        AttackStart(who);
        who->CombatStart(me);
    }
}

void CreatureAI::SelectNearestTarget(Unit *who)
{
    if (me->GetVictim() && me->GetDistanceOrder(who, me->GetVictim()) && me->canAttack(who))
    {
        float threat = me->getThreatManager().getThreat(me->GetVictim());
        me->getThreatManager().modifyThreatPercent(me->GetVictim(), -100);
        me->AddThreat(who, threat);
    }
}

void CreatureAI::EnterEvadeMode()
{
    if (!_EnterEvadeMode())
        return;

    sLog.outDebug("Creature %u enters evade mode.", me->GetEntry());

    me->GetMotionMaster()->MoveTargetedHome();

    if (CreatureGroup *formation = me->GetFormation())
        formation->EvadeFormation(me);

    Reset();

}

void CreatureAI::JustReachedHome()
{
    me->GetMotionMaster()->Initialize();
}

void CreatureAI::GetDebugInfo(ChatHandler& reader)
{
    reader.SendSysMessage("This AI does not support debugging.");
}

void CreatureAI::SendDebug(const char* fmt, ...)
{
    if (!m_debugInfoReceiver)
        return;
    Player *target = sObjectAccessor.GetPlayer(m_debugInfoReceiver);
    if (!target)
    {
        m_debugInfoReceiver = 0;
        return;
    }

    va_list ap;
    char message[1024];
    va_start(ap, fmt);
    vsnprintf(message, 1024, fmt, ap);
    va_end(ap);

    WorldPacket data;
    uint32 messageLength = (message ? strlen(message) : 0) + 1;

    data.Initialize(SMSG_MESSAGECHAT, 100);                // guess size
    data << uint8(CHAT_MSG_SYSTEM);
    data << uint32(LANG_UNIVERSAL);
    data << uint64(0);
    data << uint32(0);
    data << uint64(0);
    data << uint32(messageLength);
    data << message;
    data << uint8(0);

    target->SendPacketToSelf(&data);
}
