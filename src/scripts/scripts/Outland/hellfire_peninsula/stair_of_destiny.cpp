/*
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

#include "precompiled.h"

// npc_pit_commander
// this is the main handler of everything happening on stair.

const float commanderpath[][3] = {
    -210.618866,1779.591553,76.286537,
    -202.692627,1710.041748,56.442822,
    -202.777313,1640.829224,40.275764,
    -213.661926,1567.820435,28.033577,
    -231.166138,1498.812134,20.344385,
    -247.481506,1430.740112,13.883158,
    -249.923660,1351.830444,11.782126,
    -242.117218,1296.420654,20.150530,
    -238.794800,1238.643433,30.184883,
    -243.448166,1178.422119,41.714531,
};

enum
{
    NPC_INFERNAL_RELAY = 19215,
    NPC_INFERNAL_SIEGEBREAKER = 18946,

    SPELL_INFERNAL_RAIN = 33814,
};

struct npc_pit_commanderAI : public ScriptedAI
{
    npc_pit_commanderAI(Creature* c) : ScriptedAI(c) { }

    uint8 movement;
    Timer infernalSummonTimer;
    void InitializeAI() // once
    {
        m_creature->setActive(true);
        movement = 0;
        infernalSummonTimer.Reset(60000);
        JustRespawned();
    }

    void Reset()
    {
        m_creature->setActive(true);
    }

    void JustReachedHome()
    {
        MovementInform(POINT_MOTION_TYPE, movement);
    }

    void JustRespawned()
    {
        movement = 0;
        MovementInform(POINT_MOTION_TYPE, 0);
    }

    void MovementInform(uint32 type, uint32 point)
    {
        if (type != POINT_MOTION_TYPE || point != movement)
            return;
        if (movement == 9) // we're at the spot
            return;
        movement++;
        m_creature->GetMotionMaster()->MovePoint(movement, commanderpath[movement][0], commanderpath[movement][1], commanderpath[movement][2]);
        m_creature->SetHomePosition(commanderpath[movement][0], commanderpath[movement][1], commanderpath[movement][2], 0.0f);
        
    }
    
    void UpdateAI(const uint32 diff)
    {
        if (infernalSummonTimer.Expired(diff))
        {
            std::list<uint64> relays = m_creature->GetMap()->GetCreaturesGUIDList(NPC_INFERNAL_RELAY);
            for (std::list<uint64>::iterator itr = relays.begin(); itr != relays.end(); itr++)
            {
                Creature* relay = m_creature->GetCreature(*itr);
                if (!relay)
                    continue;

                if (infernalSummonTimer.GetInterval() == 2000)
                {
                    m_creature->SummonCreature(NPC_INFERNAL_SIEGEBREAKER, relay->GetPositionX(), relay->GetPositionY(), 41.7f, 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 1000);
                }
                else
                {
                    relay->CastSpell(relay->GetPositionX(), relay->GetPositionY(), 41.7f, SPELL_INFERNAL_RAIN, false);
                }
            }
            infernalSummonTimer = (infernalSummonTimer.GetInterval() == 2000) ? 60000 : 2000;

        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_pit_commander(Creature* c)
{
    return new npc_pit_commanderAI(c);
}

const float defenderpath[][3] = {
    -160.907440,966.141113,54.281906,
    -179.534409,1013.573242,54.292568,
    -220.318283,1030.221191,54.322639,
    -236.330658,1073.005249,54.307281,
    -336.493225,963.379272,54.284126,
    -316.957062,1011.548279,54.301487,
    -282.645508,1028.710083,54.308292,
    -264.332001,1069.990845,54.309059,
};

struct npc_stair_defender_baseAI : public ScriptedAI
{
    npc_stair_defender_baseAI(Creature* c) : ScriptedAI(c) {}
    
    uint8 movement;

    bool ishorde()
    {
        switch (m_creature->getFaction())
        {
        case 1758:
        case 1759:
        case 1760:
            return true;
        }
        return false;
    }

    void Reset()
    {
        movement = ishorde() ? 0 : 4;
        m_creature->SetWalk(false);
    }

    void JustRespawned()
    {
        m_creature->Relocate(defenderpath[movement][0], defenderpath[movement][1], defenderpath[movement][2]);
        MovementInform(POINT_MOTION_TYPE, movement);
    }

    void MovementInform(uint32 type, uint32 point)
    {
        if (type != POINT_MOTION_TYPE || point != movement)
            return;
        if (movement == 3 || movement == 7) // reached last point of path
        {
            float x, y, z, o;
            m_creature->GetRespawnCoord(x, y, z, &o);
            m_creature->SetHomePosition(x, y, z, o);
            m_creature->GetMotionMaster()->MoveTargetedHome();
            return;
        }
        movement++;
        m_creature->GetMotionMaster()->MovePoint(movement, defenderpath[movement][0], defenderpath[movement][1], defenderpath[movement][2]);
        m_creature->SetHomePosition(defenderpath[movement][0], defenderpath[movement][1], defenderpath[movement][2], 0);
    }
};

CreatureAI* GetAI_npc_stair_defender_base(Creature* c) // should be removed after we set up all scripts
{
    return new npc_stair_defender_baseAI(c);
}

void AddSC_stair_of_destiny()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_pit_commander";
    newscript->GetAI = &GetAI_npc_pit_commander;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_stair_defender_base";
    newscript->GetAI = &GetAI_npc_stair_defender_base;
    newscript->RegisterSelf();
}
