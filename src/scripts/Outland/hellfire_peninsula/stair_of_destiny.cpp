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

#include "scriptPCH.h"

// npc_pit_commander
// this is the main handler of everything happening on stair.

static const float summonpos[9][3] = {
    -214.350647f,1637.153931f,39.914082f,
    -207.305817f,1637.377075f,39.924305f,
    -193.851624f,1636.948730f,39.490860f,
    -187.019592f,1635.961182f,39.356621f,
    -189.144714f,1645.597534f,40.894733f,
    -196.218124f,1645.822754f,41.168633f,
    -205.629456f,1646.132812f,41.303810f,
    -213.563522f,1648.108154f,41.882431f,
    -201.142731f,1636.422363f,40.535400f,

};

enum
{
    ATTACKERS_PATH_BEGIN    = 16010,

    NPC_INFERNAL_RELAY      = 19215,
    NPC_INFERNAL_SIEGEBREAKER = 18946,
    NPC_WRATH_MASTER        = 19005,
    NPC_FEL_SOLDIER         = 18944,
    NPC_PIT_COMMANDER       = 18945,

    SPELL_INFERNAL_RAIN     = 33814,
    SPELL_CLEAVE            = 16044,
    SPELL_RAIN_OF_FIRE      = 33972,

    SPELL_SOLDIER_CLEAVE    = 15496,
    SPELL_SOLDIER_CUTDOWN   = 32009,
};

struct npc_pit_commanderAI : public ScriptedAI
{
    npc_pit_commanderAI(Creature* c) : ScriptedAI(c) { }

    Timer infernalSummonTimer;
    Timer invadersSummonTimer;
    Timer cleaveTimer;
    Timer rainOfFireTimer;

    void InitializeAI() // once
    {
        m_creature->setActive(true);
        infernalSummonTimer.Reset(40000);
        invadersSummonTimer.Reset(60000);
        cleaveTimer.Reset(20000);
        rainOfFireTimer.Reset(30000);
        JustRespawned();
    }

    void Reset()
    {
        m_creature->setActive(true);
    }

    void JustRespawned()
    {
        m_creature->GetMotionMaster()->MovePath(ATTACKERS_PATH_BEGIN + 8, false);
    }
    
    void MovementInform(uint32 type, uint32 point)
    {
        if (type == WAYPOINT_MOTION_TYPE)
            m_creature->SetHomePosition(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), m_creature->GetOrientation());
    }

    void SummonInfernals()
    {
        std::list<uint64> relays = m_creature->GetMap()->GetCreaturesGUIDList(NPC_INFERNAL_RELAY);
        for (std::list<uint64>::iterator itr = relays.begin(); itr != relays.end(); itr++)
        {
            Creature* relay = m_creature->GetCreature(*itr);
            if (!relay)
                continue;

            if (infernalSummonTimer.GetInterval() == 2000)
            {
                if (Creature* infernal = m_creature->SummonCreature(NPC_INFERNAL_SIEGEBREAKER, relay->GetPositionX(), relay->GetPositionY(), 41.7f, 4.76f, TEMPSUMMON_CORPSE_DESPAWN, 1000))
                {
                    infernal->GetMotionMaster()->MovePoint(0, -250, 1085, 47);
                    infernal->setActive(true);
                }
            }
            else
            {
                relay->CastSpell(relay->GetPositionX(), relay->GetPositionY(), 41.7f, SPELL_INFERNAL_RAIN, false);
            }
        }
        infernalSummonTimer = (infernalSummonTimer.GetInterval() == 2000) ? urand(30000,50000) : 2000;
    }

    void SummonInvaders()
    {
        if (urand(0, 1))
        {
            for (uint8 i = 0; i < 8; i++) // 8 fel soldiers
            {
                Creature* soldier = m_creature->SummonCreature(NPC_FEL_SOLDIER, summonpos[i][0], summonpos[i][1], summonpos[i][2], 4.76f, TEMPSUMMON_CORPSE_DESPAWN, 1000);
                soldier->GetMotionMaster()->MovePath(ATTACKERS_PATH_BEGIN + i, false);
                soldier->setActive(true);
            }

        }
        else
        {
            for (uint8 i = 0; i < 4; i++) // 4 fel soldiers
            {
                Creature* soldier = m_creature->SummonCreature(NPC_FEL_SOLDIER, summonpos[i][0], summonpos[i][1], summonpos[i][2], 4.76f, TEMPSUMMON_CORPSE_DESPAWN, 1000);
                soldier->GetMotionMaster()->MovePath(ATTACKERS_PATH_BEGIN + i, false);
                soldier->setActive(true);
            }
            // and wrath master
            Creature* master = m_creature->SummonCreature(NPC_WRATH_MASTER, summonpos[8][0], summonpos[8][1], summonpos[8][2], 4.76f, TEMPSUMMON_CORPSE_DESPAWN, 1000);
            master->GetMotionMaster()->MovePath(ATTACKERS_PATH_BEGIN + 9,false);
            master->setActive(true);
        }
        invadersSummonTimer = urand(30000,60000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (infernalSummonTimer.Expired(diff))
            SummonInfernals();

        if (invadersSummonTimer.Expired(diff))
            SummonInvaders();

        if (!UpdateVictim())
            return;

        if (cleaveTimer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_CLEAVE);
            cleaveTimer = 20000;
        }

        if (rainOfFireTimer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_RAIN_OF_FIRE);
            rainOfFireTimer = 30000;
        }

        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_pit_commander(Creature* c)
{
    return new npc_pit_commanderAI(c);
}

struct npc_stair_attackerAI : public ScriptedAI
{
    npc_stair_attackerAI(Creature* c) : ScriptedAI(c) { despawnTimer.Reset(0); }

    Timer skill1timer;
    Timer skill2timer;
    Timer despawnTimer;

    void Reset()
    {
        skill1timer.Reset(10000);
        skill2timer.Reset(20000);
    }

    void EnterCombat(Unit* who)
    {
        if (!despawnTimer.GetInterval())
            despawnTimer.Reset(60000); // live one minute from first combat, dont stack infinitely
    }

    void MovementInform(uint32 type, uint32 point)
    {
        if (type == WAYPOINT_MOTION_TYPE)
            m_creature->SetHomePosition(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), m_creature->GetOrientation());
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (despawnTimer.Expired(diff))
        {
            me->DealDamage(me, me->GetHealth());
        }

        if (skill1timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_SOLDIER_CLEAVE);
            skill1timer = 10000;
        }

        if (skill2timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_SOLDIER_CUTDOWN);
            skill2timer = 10000;
        }

        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_stair_attacker(Creature* c)
{
    return new npc_stair_attackerAI(c);
}


static const float defenderpath[][3] = {
    -160.907440f,966.141113f,54.281906f,
    -179.534409f,1013.573242f,54.292568f,
    -220.318283f,1030.221191f,54.322639f,
    -236.330658f,1073.005249f,54.307281f,
    -336.493225f,963.379272f,54.284126f,
    -316.957062f,1011.548279f,54.301487f,
    -282.645508f,1028.710083f,54.308292f,
    -264.332001f,1069.990845f,54.309059f,
};

enum
{
    // ranged
    SPELL_SHOOT     = 15620,
    // mages
    SPELL_FIREBALL  = 33417,
    SPELL_ICEBOLT   = 33463,
    SPELL_BLIZZARD  = 33624,
    // warrior
    SPELL_STRIKE    = 33626,
    // shaman
    SPELL_BLOODLUST = 16170,
    SPELL_SHOCK     = 15616,
    SPELL_SHIELD    = 20545,
    // paladin
    SPELL_EXORCISM  = 33632,
    SPELL_STUN      = 13005,
    SPELL_BLESS     = 13903,
    // justinius
    SPELL_CONSA     = 33559,
    SPELL_BUBBLE    = 33581,
    SPELL_FLASH     = 33641,
    SPELL_GBOM      = 33564,
    SPELL_JOC       = 33554,
    // melgrom
    SPELL_EARTH_TOT = 33570,
    SPELL_FIRE_TOT  = 33560,
    SPELL_SHOCK2    = 22885,
    SPELL_CHAIN_L   = 33643,
    SPELL_CHAIN_H   = 33642,
};

struct npc_stair_defender_baseAI : public ScriptedAI
{
    npc_stair_defender_baseAI(Creature* c) : ScriptedAI(c) {}
    
    uint8 movement;
    Timer searchTimer;
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
        searchTimer.Reset(1000);
        m_creature->SetWalk(false);
        //m_creature->SetReactState(REACT_AGGRESSIVE);
        m_creature->setActive(true);
    }

    void JustReachedHome()
    {
        AttackStart(m_creature->SelectNearbyTarget(30.0f));
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
        m_creature->SetHomePosition(defenderpath[movement][0], defenderpath[movement][1], defenderpath[movement][2], 1.75f);
    }
};

struct npc_defender_rangedAI : public npc_stair_defender_baseAI
{
    npc_defender_rangedAI(Creature* c) : npc_stair_defender_baseAI(c) {}

    Timer shootTimer;
    void Reset()
    {
        npc_stair_defender_baseAI::Reset();
        shootTimer.Reset(2000);
    }

    void AttackStart(Unit* who)
    {
        ScriptedAI::AttackStartNoMove(who, CHECK_TYPE_SHOOTER);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (shootTimer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_SHOOT);
            shootTimer = 1500;
        }

        CheckShooterNoMovementInRange(diff, 30.0);
        DoMeleeAttackIfReady();
    }
};

struct npc_defender_mageAI : public npc_stair_defender_baseAI
{
    npc_defender_mageAI(Creature* c) : npc_stair_defender_baseAI(c) {}

    Timer blizzardTimer;
    Timer maincastTimer;
    void Reset()
    {
        npc_stair_defender_baseAI::Reset();
        maincastTimer.Reset(1500);
        blizzardTimer.Reset(10000);
    }

    void AttackStart(Unit* who)
    {
        ScriptedAI::AttackStartNoMove(who, CHECK_TYPE_CASTER);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (blizzardTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_BLIZZARD, CAST_RANDOM);
            blizzardTimer = 10000;
        }

        if (maincastTimer.Expired(diff))
        {
            AddSpellToCast(ishorde() ? SPELL_ICEBOLT : SPELL_FIREBALL);
            maincastTimer = ishorde() ? 1600 : 3100;
        }

        CheckCasterNoMovementInRange(diff, 30.0);
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

struct npc_defender_warriorAI : public npc_stair_defender_baseAI
{
    npc_defender_warriorAI(Creature* c) : npc_stair_defender_baseAI(c) {}

    void Reset()
    {
        npc_stair_defender_baseAI::Reset();
        SetAutocast(SPELL_STRIKE, 5000);
        StartAutocast();
    }


    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

struct npc_defender_shamanAI : public npc_stair_defender_baseAI
{
    npc_defender_shamanAI(Creature* c) : npc_stair_defender_baseAI(c) {}

    Timer bloodlustTimer;
    Timer shockTimer;
    Timer shieldTimer;
    void Reset()
    {
        npc_stair_defender_baseAI::Reset();
        bloodlustTimer.Reset(10000);
        shockTimer.Reset(1000);
        shieldTimer.Reset(1000);
    }


    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (shieldTimer.Expired(diff))
        {
            shieldTimer = 10000;
            if (!m_creature->HasAura(SPELL_SHIELD))
                AddSpellToCast(SPELL_SHIELD, CAST_SELF);
        }

        if (shockTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_SHOCK);
            shockTimer = 6000;
        }

        if (bloodlustTimer.Expired(diff))
        {
            bloodlustTimer = 20000;
            std::list<Creature*> friends = FindFriendlyMissingBuff(20.0f, SPELL_BLOODLUST);
            if (friends.size())
            {
                AddSpellToCast(*friends.begin(), SPELL_BLOODLUST);
            }
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

struct npc_defender_paladinAI : public npc_stair_defender_baseAI
{
    npc_defender_paladinAI(Creature* c) : npc_stair_defender_baseAI(c) {}

    Timer exorcismTimer;
    Timer stunTimer;
    Timer blessingTimer;
    void Reset()
    {
        npc_stair_defender_baseAI::Reset();
        blessingTimer.Reset(10000);
        stunTimer.Reset(5000);
        exorcismTimer.Reset(1000);
    }


    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (exorcismTimer.Expired(diff))
        {
            exorcismTimer = 10000;
            AddSpellToCast(SPELL_EXORCISM);
        }

        if (stunTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_STUN);
            stunTimer = 6000;
        }

        if (blessingTimer.Expired(diff))
        {
            blessingTimer = 20000;
            AddSpellToCast(SPELL_BLESS, CAST_LOWEST_HP_FRIENDLY);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

struct npc_justiniusAI : public npc_stair_defender_baseAI
{
    npc_justiniusAI(Creature* c) : npc_stair_defender_baseAI(c) {}

    Timer consaTimer;
    Timer bubbleCooldown;
    Timer flashTimer;
    Timer bomTimer;
    Timer judgementTimer;
    void Reset()
    {
        npc_stair_defender_baseAI::Reset();
        consaTimer.Reset(10000);
        bubbleCooldown.Reset(1);
        flashTimer.Reset(7000);
        bomTimer.Reset(1000);
        judgementTimer.Reset(3000);
        if (Creature* pit = m_creature->GetMap()->GetCreatureById(NPC_PIT_COMMANDER))
            pit->setActive(true);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        bubbleCooldown.Update(diff);

        if (consaTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_CONSA);
            consaTimer = 25000;
        }

        if (flashTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_FLASH, CAST_LOWEST_HP_FRIENDLY);
            flashTimer = 5000;
        }

        if (bomTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_GBOM, CAST_SELF);
            bomTimer = 40000;
        }

        if (judgementTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_JOC);
            judgementTimer = 5000;
        }

        if (m_creature->HealthBelowPct(20) && bubbleCooldown.Passed())
        {
            DoCast(m_creature, SPELL_BUBBLE);
            bubbleCooldown.Reset(60000);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

struct npc_melgromAI : public npc_stair_defender_baseAI
{
    npc_melgromAI(Creature* c) : npc_stair_defender_baseAI(c) {}

    Timer earthtotTimer;
    Timer firetotTimer;
    Timer shockTimer;
    Timer healTimer;
    Timer chainTimer;
    void Reset()
    {
        npc_stair_defender_baseAI::Reset();
        earthtotTimer.Reset(3000);
        firetotTimer.Reset(3100);
        healTimer.Reset(7000);
        chainTimer.Reset(1000);
        shockTimer.Reset(5000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;


        if (earthtotTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_EARTH_TOT);
            earthtotTimer = 120000;
        }

        if (healTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_CHAIN_H, CAST_LOWEST_HP_FRIENDLY);
            healTimer = 10000;
        }

        if (chainTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_CHAIN_L);
            chainTimer = 10000;
        }

        if (shockTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_SHOCK2);
            shockTimer = 5000;
        }

        if (firetotTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_FIRE_TOT);
            firetotTimer = 20000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_stair_defender_base(Creature* c)
{
    switch (c->GetEntry())
    {
    case 18965:
    case 18970:
        return new npc_defender_rangedAI(c);
    case 18949:
    case 18971:
        return new npc_defender_mageAI(c);
    case 18948:
    case 18950:
        return new npc_defender_warriorAI(c);
    case 18986:
        return new npc_defender_paladinAI(c);
    case 18972:
        return new npc_defender_shamanAI(c);
    case 18969:
        return new npc_melgromAI(c);
    case 18966:
        return new npc_justiniusAI(c);
    default:
        return new npc_stair_defender_baseAI(c);
    }
    
}


void AddSC_stair_of_destiny()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_pit_commander";
    newscript->GetAI = &GetAI_npc_pit_commander;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_stair_attacker";
    newscript->GetAI = &GetAI_npc_stair_attacker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_stair_defender_base";
    newscript->GetAI = &GetAI_npc_stair_defender_base;
    newscript->RegisterSelf();
}
