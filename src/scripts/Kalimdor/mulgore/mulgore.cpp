/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Mulgore
SD%Complete: 100
SDComment: Support for quest: 11129, 772
SDCategory: Mulgore
EndScriptData */

/* ContentData
npc_skorn_whitecloud
npc_kyle_frenzied
npc_plains_vision
EndContentData */

#include "scriptPCH.h"

/*######
# npc_skorn_whitecloud
######*/

#define GOSSIP_SW "Tell me a story, Skorn."

bool GossipHello_npc_skorn_whitecloud(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (!player->GetQuestRewardStatus(770))
        player->ADD_GOSSIP_ITEM( 0, GOSSIP_SW, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF );

    player->SEND_GOSSIP_MENU(522,_Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_skorn_whitecloud(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF)
        player->SEND_GOSSIP_MENU(523,_Creature->GetGUID());

    return true;
}

/*#####
# npc_kyle_frenzied
######*/

struct npc_kyle_frenziedAI : public ScriptedAI
{
    npc_kyle_frenziedAI(Creature *c) : ScriptedAI(c) {}

    int STATE;
    int32 wait;
    uint64 player;

    void Reset()
    {
        STATE = 0;
        m_creature->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
        m_creature->GetMotionMaster()->Initialize();
    }
    void SpellHit(Unit *caster, const SpellEntry* spell)
    {   // we can feed him without any quest
        if(spell->Id == 42222 && caster->GetTypeId() == TYPEID_PLAYER && ((Player*)caster)->GetTeam() == HORDE)
        {
            STATE = 1;
            player = caster->GetGUID();
            float x, y, z, z2;
            caster->GetPosition(x, y, z);
            x = x + 3.7*cos(caster->GetOrientation());
            y = y + 3.7*sin(caster->GetOrientation());
            z2 = m_creature->GetTerrain()->GetHeight(x,y,z,false);
            z = (z2 <= INVALID_HEIGHT) ? z : z2;
            m_creature->SetDefaultMovementType(IDLE_MOTION_TYPE);       //there is other way to stop waypoint movement?
            m_creature->GetMotionMaster()->Initialize();
            m_creature->SetWalk(false);
            m_creature->GetMotionMaster()->MovePoint(0,x, y, z);
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if(type == POINT_MOTION_TYPE)
        {
            switch(STATE)
            {
            case 1:
                {
                Unit *plr = Unit::GetUnit((*m_creature),player);
                if(plr)
                    m_creature->SetFacingTo(m_creature->GetAngle(plr));

                m_creature->HandleEmoteCommand(EMOTE_STATE_USESTANDING);    //eat
                wait = 3000;
                STATE = 2;
                break;
                }
            case 4:
                m_creature->setDeathState(JUST_DIED);
                m_creature->Respawn();
                break;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!STATE || STATE == 4)
            return;

        wait -= diff;
        if(wait <= diff)
        {
            switch(STATE)
            {
            case 2:
                STATE = 3; wait = 7000;
                m_creature->UpdateEntry(23622,HORDE);
                m_creature->HandleEmoteCommand(EMOTE_ONESHOT_DANCE);
                break;
            case 3:
                STATE = 4;  //go home
                Player *plr = Unit::GetPlayer(player);
                if(plr && plr->GetQuestStatus(11129) == QUEST_STATUS_INCOMPLETE)
                    plr->CompleteQuest(11129);
                float x, y, z, z2, angle;
                angle = m_creature->GetAngle(-2146, -430);
                m_creature->GetPosition(x,y,z);
                x = x + 40*cos(angle);
                y = y + 40*sin(angle);
                z2 = m_creature->GetTerrain()->GetHeight(x,y,MAX_HEIGHT,false);
                z = (z2 <= INVALID_HEIGHT) ? z : z2;
                m_creature->GetMotionMaster()->MovePoint(0,x,y,z);
                break;
            }
        }
    }
};

CreatureAI* GetAI_npc_kyle_frenzied(Creature *_Creature)
{
    return new npc_kyle_frenziedAI (_Creature);
}

/*#####
# npc_plains_vision
######*/

static float wp_plain_vision[50][3] =
{
    {-2226.32f,  -408.095f,   -9.36235f},
    {-2203.04f,  -437.212f,   -5.72498f},
    {-2163.91f,  -457.851f,   -7.09049f},
    {-2123.87f,  -448.137f,   -9.29591f},
    {-2104.66f,  -427.166f,   -6.49513f},
    {-2101.48f,  -422.826f,   -5.3567f},
    {-2097.56f,  -417.083f,   -7.16716f},
    {-2084.87f,  -398.626f,   -9.88973f},
    {-2072.71f,  -382.324f,   -10.2488f},
    {-2054.05f,  -356.728f,   -6.22468f},
    {-2051.8f,   -353.645f,   -5.35791f},
    {-2049.08f,  -349.912f,   -6.15723f},
    {-2030.6f,   -310.724f,   -9.59302f},
    {-2002.15f,  -249.308f,   -10.8124f},
    {-1972.85f,  -195.811f,   -10.6316f},
    {-1940.93f,  -147.652f,   -11.7055f},
    {-1888.06f,  -81.943f,    -11.4404f},
    {-1837.05f,  -34.0109f,   -12.258f},
    {-1796.12f,  -14.6462f,   -10.3581f},
    {-1732.61f,  -4.27746f,   -10.0213f},
    {-1688.94f,  -0.829945f,  -11.7103f},
    {-1681.32f,  13.0313f,    -9.48056f},
    {-1677.04f,  36.8349f,    -7.10318f},
    {-1675.2f,   68.559f,     -8.95384f},
    {-1676.57f,  89.023f,     -9.65104f},
    {-1678.16f,  110.939f,    -10.1782f},
    {-1677.86f,  128.681f,    -5.73869f},
    {-1675.27f,  144.324f,    -3.47916f},
    {-1671.7f,   163.169f,    -1.23098f},
    {-1666.61f,  181.584f,    5.26145f},
    {-1661.51f,  196.154f,    8.95252f},
    {-1655.47f,  210.811f,    8.38727f},
    {-1647.07f,  226.947f,    5.27755f},
    {-1621.65f,  232.91f,     2.69579f},
    {-1600.23f,  237.641f,    2.98539f},
    {-1576.07f,  242.546f,    4.66541f},
    {-1554.57f,  248.494f,    6.60377f},
    {-1547.53f,  259.302f,    10.6741f},
    {-1541.7f,   269.847f,    16.4418f},
    {-1539.83f,  278.989f,    21.0597f},
    {-1540.16f,  290.219f,    27.8247f},
    {-1538.99f,  298.983f,    34.0032f},
    {-1540.38f,  307.337f,    41.3557f},
    {-1536.61f,  314.884f,    48.0179f},
    {-1532.42f,  323.277f,    55.6667f},
    {-1528.77f,  329.774f,    61.1525f},
    {-1525.65f,  333.18f,     63.2161f},
    {-1517.01f,  350.713f,    62.4286f},
    {-1511.39f,  362.537f,    62.4539f},
    {-1508.68f,  366.822f,    62.733f}
};

struct npc_plains_visionAI  : public ScriptedAI
{
    npc_plains_visionAI(Creature *c) : ScriptedAI(c) {}

    bool newWaypoint;
    uint8 WayPointId;
    uint8 amountWP;

    void Reset()
    {
        WayPointId = 0;
        newWaypoint = true;
        amountWP  = 49;
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if(type != POINT_MOTION_TYPE)
            return;

        if (id < amountWP)
        {
            ++WayPointId;
            newWaypoint = true;
        }
        else
        {
            m_creature->setDeathState(JUST_DIED);
            m_creature->RemoveCorpse();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (newWaypoint)
        {
            m_creature->GetMotionMaster()->MovePoint(WayPointId, wp_plain_vision[WayPointId][0], wp_plain_vision[WayPointId][1], wp_plain_vision[WayPointId][2]);
            newWaypoint = false;
        }
    }
};

CreatureAI* GetAI_npc_plains_vision(Creature *_Creature)
{
      return new npc_plains_visionAI (_Creature);
}

/*#####
#
######*/

void AddSC_mulgore()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_skorn_whitecloud";
    newscript->pGossipHello = &GossipHello_npc_skorn_whitecloud;
    newscript->pGossipSelect = &GossipSelect_npc_skorn_whitecloud;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_kyle_frenzied";
    newscript->GetAI = &GetAI_npc_kyle_frenzied;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_plains_vision";
    newscript->GetAI = &GetAI_npc_plains_vision;
    newscript->RegisterSelf();
}

