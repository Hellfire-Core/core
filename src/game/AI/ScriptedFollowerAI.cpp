/*
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: FollowerAI
SD%Complete: 50
SDComment: This AI is under development
SDCategory: Npc
EndScriptData */

#include "scriptPCH.h"
#include "ScriptedFollowerAI.h"

const float MAX_PLAYER_DISTANCE = 100.0f;

FollowerAI::FollowerAI(Creature* pCreature) : ScriptedAI(pCreature),
    m_uiLeaderGUID(0),
    m_uiUpdateFollowTimer(2500),
    m_uiFollowState(STATE_FOLLOW_NONE),
    m_pQuestForFollow(NULL)
{}

void FollowerAI::AttackStart(Unit* pWho)
{
    if (!pWho)
        return;
    
    if (me->Attack(pWho, true))
    {
        SendDebug("FollowerAI: attack start");
        me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0);
        me->AddThreat(pWho, 0.0f);
        me->SetInCombatWith(pWho);
        pWho->SetInCombatWith(me);

        if (me->HasUnitState(UNIT_STAT_FOLLOW))
            me->ClearUnitState(UNIT_STAT_FOLLOW);

        if (IsCombatMovement())
            me->GetMotionMaster()->MoveChase(pWho);
    }
}

//This part provides assistance to a player that are attacked by pWho, even if out of normal aggro range
//It will cause me to attack pWho that are attacking _any_ player (which has been confirmed may happen also on offi)
//The flag (type_flag) is unconfirmed, but used here for further research and is a good candidate.
bool FollowerAI::AssistPlayerInCombat(Unit* pWho)
{
    if (!pWho || !pWho->GetVictim())
        return false;

    //not a player
    if (!pWho->GetVictim()->GetCharmerOrOwnerPlayerOrPlayerItself())
        return false;

    //never attack friendly
    if (me->IsFriendlyTo(pWho))
        return false;

    //too far away and no free sight?
    if (me->IsWithinDistInMap(pWho, MAX_PLAYER_DISTANCE) && me->IsWithinLOSInMap(pWho))
    {
        //already fighting someone?
        if (!me->GetVictim())
        {
            AttackStart(pWho);
            return true;
        }
        else
        {
            pWho->SetInCombatWith(me);
            me->AddThreat(pWho, 0.0f);
            return true;
        }
    }

    return false;
}

void FollowerAI::MoveInLineOfSight(Unit* pWho)
{
    if (!me->HasUnitState(UNIT_STAT_STUNNED) && pWho->isTargetableForAttack() && pWho->isInAccessiblePlacefor(me))
    {
        if (HasFollowState(STATE_FOLLOW_INPROGRESS) && AssistPlayerInCombat(pWho))
            return;

        if (!me->CanFly() && me->GetDistanceZ(pWho) > CREATURE_Z_ATTACK_RANGE)
            return;

        if (me->IsHostileTo(pWho))
        {
            float fAttackRadius = me->GetAttackDistance(pWho);
            if (me->IsWithinDistInMap(pWho, fAttackRadius) && me->IsWithinLOSInMap(pWho))
            {
                if (!me->GetVictim())
                {
                    pWho->RemoveAurasDueToSpell(SPELL_AURA_MOD_STEALTH);
                    AttackStart(pWho);
                }
                else if (me->GetMap()->IsDungeon())
                {
                    pWho->SetInCombatWith(me);
                    me->AddThreat(pWho, 0.0f);
                }
            }
        }
    }
}

void FollowerAI::JustDied(Unit* /*pKiller*/)
{
    if (!HasFollowState(STATE_FOLLOW_INPROGRESS) || !m_uiLeaderGUID || !m_pQuestForFollow)
        return;
    SendDebug("FollowerAI: just died");

    //TODO: need a better check for quests with time limit.
    if (Player* pPlayer = GetLeaderForFollower())
    {
        if (Group* pGroup = pPlayer->GetGroup())
        {
            for (GroupReference* pRef = pGroup->GetFirstMember(); pRef != NULL; pRef = pRef->next())
            {
                if (Player* pMember = pRef->getSource())
                {
                    if (pMember->GetQuestStatus(m_pQuestForFollow->GetQuestId()) == QUEST_STATUS_INCOMPLETE)
                        pMember->FailQuest(m_pQuestForFollow->GetQuestId());
                }
            }
        }
        else
        {
            if (pPlayer->GetQuestStatus(m_pQuestForFollow->GetQuestId()) == QUEST_STATUS_INCOMPLETE)
                pPlayer->FailQuest(m_pQuestForFollow->GetQuestId());
        }
    }
}

void FollowerAI::JustRespawned()
{
    m_uiFollowState = STATE_FOLLOW_NONE;

    if (!IsCombatMovement())
        SetCombatMovement(true);

    if (me->getFaction() != me->GetCreatureInfo()->faction_A)
        me->setFaction(me->GetCreatureInfo()->faction_A);

    Reset();
}

void FollowerAI::EnterEvadeMode()
{
    SendDebug("FollowerAI: enter evade");
    me->RemoveAllAuras();
    me->DeleteThreatList();
    me->CombatStop(true);
    me->SetLootRecipient(NULL);

    if (!HasFollowState(STATE_FOLLOW_INPROGRESS))
    {
        if (me->HasUnitState(UNIT_STAT_CHASE))
        {
            me->GetMotionMaster()->MoveTargetedHome();
            SendDebug("FollowerAI: move targeted home");
        }
    }

    Reset();
}

void FollowerAI::UpdateAI(const uint32 uiDiff)
{
    if (HasFollowState(STATE_FOLLOW_INPROGRESS) && !me->GetVictim())
    {
        if (m_uiUpdateFollowTimer <= uiDiff)
        {
            if (HasFollowState(STATE_FOLLOW_COMPLETE) && !HasFollowState(STATE_FOLLOW_POSTEVENT))
            {
                me->ForcedDespawn();
                return;
            }

            bool bIsMaxRangeExceeded = true;

            Player* pPlayer = GetLeaderForFollower();
            if (pPlayer)
            {
                if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE &&
                    !me->HasUnitState(UNIT_STAT_LOST_CONTROL) && !HasFollowState(STATE_FOLLOW_PAUSED | STATE_FOLLOW_COMPLETE))
                {
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveFollow(pPlayer, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
                    SendDebug("FollowerAI: returning to follow");
                    return;
                }

                if (Group* pGroup = pPlayer->GetGroup())
                {
                    for (GroupReference* pRef = pGroup->GetFirstMember(); pRef != NULL; pRef = pRef->next())
                    {
                        Player* pMember = pRef->getSource();

                        if (pMember && me->IsWithinDistInMap(pMember, MAX_PLAYER_DISTANCE))
                        {
                            bIsMaxRangeExceeded = false;
                            break;
                        }
                    }
                }
                else
                {
                    if (me->IsWithinDistInMap(pPlayer, MAX_PLAYER_DISTANCE))
                        bIsMaxRangeExceeded = false;
                }
            }

            if (bIsMaxRangeExceeded || !pPlayer)
            {
                me->ForcedDespawn();
                SendDebug("FollowerAI: out of range, forcing despawn");
                return;
            }

            m_uiUpdateFollowTimer = 1000;
        }
        else
            m_uiUpdateFollowTimer -= uiDiff;
    }

    UpdateFollowerAI(uiDiff);
}

void FollowerAI::UpdateFollowerAI(const uint32 /*uiDiff*/)
{
    if (!UpdateVictim())
        return;

    DoMeleeAttackIfReady();
}

void FollowerAI::MovementInform(uint32 uiMotionType, uint32 uiPointId)
{
    if (uiMotionType != POINT_MOTION_TYPE || !HasFollowState(STATE_FOLLOW_INPROGRESS))
        return;
}

void FollowerAI::StartFollow(Player* pLeader, uint32 uiFactionForFollower, const Quest* pQuest)
{
    SendDebug("FollowerAI: start follow");
    if (me->GetVictim() || HasFollowState(STATE_FOLLOW_INPROGRESS))
        return;

    //set variables
    m_uiLeaderGUID = pLeader->GetGUID();

    if (uiFactionForFollower)
        me->setFaction(uiFactionForFollower);

    m_pQuestForFollow = pQuest;

    if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
        me->GetMotionMaster()->MoveIdle();

    me->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);

    AddFollowState(STATE_FOLLOW_INPROGRESS);

    me->GetMotionMaster()->MoveFollow(pLeader, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
}

Player* FollowerAI::GetLeaderForFollower()
{
    if (Player* pLeader = Unit::GetPlayer(m_uiLeaderGUID))
    {
        if (pLeader->IsAlive())
            return pLeader;
        else
        {
            if (Group* pGroup = pLeader->GetGroup())
            {
                for (GroupReference* pRef = pGroup->GetFirstMember(); pRef != NULL; pRef = pRef->next())
                {
                    Player* pMember = pRef->getSource();

                    if (pMember && pMember->IsAlive() && me->IsWithinDistInMap(pMember, MAX_PLAYER_DISTANCE))
                    {
                        m_uiLeaderGUID = pMember->GetGUID();
                        return pMember;
                        break;
                    }
                }
            }
        }
    }

    return NULL;
}

void FollowerAI::SetFollowComplete(bool bWithEndEvent)
{
    SendDebug("FollowerAI: follow complete");
    if (me->HasUnitState(UNIT_STAT_FOLLOW))
    {
        me->ClearUnitState(UNIT_STAT_FOLLOW);

        me->GetMotionMaster()->MoveIdle();
    }

    if (bWithEndEvent)
        AddFollowState(STATE_FOLLOW_POSTEVENT);
    else
    {
        if (HasFollowState(STATE_FOLLOW_POSTEVENT))
            RemoveFollowState(STATE_FOLLOW_POSTEVENT);
    }

    AddFollowState(STATE_FOLLOW_COMPLETE);
}

void FollowerAI::SetFollowPaused(bool bPaused)
{
    SendDebug("FollowerAI: follow paused");
    if (!HasFollowState(STATE_FOLLOW_INPROGRESS) || HasFollowState(STATE_FOLLOW_COMPLETE))
        return;

    if (bPaused)
    {
        AddFollowState(STATE_FOLLOW_PAUSED);

        if (me->HasUnitState(UNIT_STAT_FOLLOW))
        {
            me->ClearUnitState(UNIT_STAT_FOLLOW);

            me->GetMotionMaster()->MoveIdle();
        }
    }
    else
    {
        RemoveFollowState(STATE_FOLLOW_PAUSED);

        if (Player* pLeader = GetLeaderForFollower())
            me->GetMotionMaster()->MoveFollow(pLeader, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
    }
}
