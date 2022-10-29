/*
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Boss_Victor_Nefarius
SD%Complete: 75
SDComment: Missing some text, Vael beginning event, and spawns Nef in wrong place
SDCategory: Blackwing Lair
EndScriptData */

#include "precompiled.h"
#include "def_blackwing_lair.h"

#define SAY_GAMESBEGIN_1        -1469004
#define SAY_GAMESBEGIN_2        -1469005
#define SAY_VAEL_INTRO          -1469006                    //when he corrupts Vaelastrasz

#define GOSSIP_ITEM_1           "I've made no mistakes."
#define GOSSIP_ITEM_2           "You have lost your mind, Nefarius. You speak in riddles."
#define GOSSIP_ITEM_3           "Please do."

#define ADD_X1 -7591.151855
#define ADD_Y1 -1204.051880
#define ADD_Z1 476.800476

#define ADD_X2 -7514.598633
#define ADD_Y2 -1150.448853
#define ADD_Z2 476.796570

#define NEF_X   -7445
#define NEF_Y   -1332
#define NEF_Z   536

#define HIDE_X  -7592
#define HIDE_Y  -1264
#define HIDE_Z  481

//This script is complicated
//Instead of morphing Victor Nefarius we will have him control phase 1
//And then have him spawn "Nefarian" for phase 2
//When phase 2 starts Victor Nefarius will go into hiding and stop attacking
//If Nefarian despawns because he killed the players then this guy will EnterEvadeMode
//and allow players to start the event over
//If nefarian dies then he will kill himself then he will kill himself in his hiding place
//To prevent players from doing the event twice

struct boss_victor_nefariusAI : public ScriptedAI
{
    boss_victor_nefariusAI(Creature *c) : ScriptedAI(c), summons(me), SpawnedAdds(0), NefarianGUID(0)
    {
        instance = c->GetInstanceData();

        drakTypes[0] = urand(0, 4);
        drakTypes[1] = (drakTypes[0] + urand(1, 4)) % 5;
    }

    ScriptedInstance* instance;
    EventMap events;
    SummonList summons;
    uint32 SpawnedAdds;
    uint64 NefarianGUID;
    uint8 drakTypes[2];
    Timer NefarianCheckTimer;

    enum Spawns
    {
        CREATURE_BLUE_DRAKANOID         = 14261,
        CREATURE_GREEN_DRAKANOID        = 14262,
        CREATURE_BRONZE_DRAKANOID       = 14263,
        CREATURE_RED_DRAKANOID          = 14264,
        CREATURE_BLACK_DRAKANOID        = 14265,

        CREATURE_CHROMATIC_DRAKANOID    = 14302,
        CREATURE_NEFARIAN               = 11583,
    };

    enum Spells
    {
        SPELL_SHADOWBOLT    = 21077,
        SPELL_FEAR          = 26070,
    };

    enum Events
    {
        EVENT_SPAWN_ADDS = 1,
        EVENT_SPAWN_NEFARIAN,
        EVENT_CAST_SHADOWBOLT,
        EVENT_CAST_FEAR,
        EVENT_CAST_MIND_CONTROL,
    };

    void Reset()
    {
        summons.DespawnAll();
        NefarianCheckTimer = 0;
        NefarianGUID = 0;
        SpawnedAdds = 0;

        m_creature->SetUInt32Value(UNIT_NPC_FLAGS,1);
        m_creature->setFaction(35);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

        if (instance)
            instance->SetData(DATA_NEFARIAN_EVENT, NOT_STARTED);
    }

    void BeginEvent(Player* target)
    {
        events.Reset()
              .ScheduleEvent(EVENT_SPAWN_ADDS, 10000)
              .ScheduleEvent(EVENT_CAST_SHADOWBOLT, 5000)
              .ScheduleEvent(EVENT_CAST_FEAR, 8000);

        DoScriptText(SAY_GAMESBEGIN_2, m_creature);
        DoZoneInCombat();

        m_creature->SetUInt32Value(UNIT_NPC_FLAGS,0);
        m_creature->setFaction(103);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

        AttackStart(target);
    }

    void EnterCombat(Unit *who)
    {
        if (instance)
            instance->SetData(DATA_NEFARIAN_EVENT, IN_PROGRESS);
    }

    void JustSummoned(Creature* summon)
    {
        summons.Summon(summon);
        summon->setFaction(103);

        if (summon->GetEntry() != CREATURE_NEFARIAN)
            summon->AI()->AttackStart(SelectUnit(SELECT_TARGET_RANDOM, 0));
    }

    void UpdateAI(const uint32 diff)
    {
        if (NefarianCheckTimer.Expired(diff))
        {
            if (NefarianGUID)
                if (Unit* Nefarian = me->GetUnit(NefarianGUID))
                {
                    if (!Nefarian->ToCreature()->IsAlive())
                        me->DisappearAndDie();
                    else if (!Nefarian->ToCreature()->IsInCombat())
                        EnterEvadeMode();
                }

            NefarianCheckTimer = 1000;
        }

        if (!UpdateVictim() || SpawnedAdds >= 42)
            return;

        events.Update(diff);
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CAST_SHADOWBOLT:
                {
                    if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                        DoCast(target, SPELL_SHADOWBOLT);

                    events.ScheduleEvent(EVENT_CAST_SHADOWBOLT, urand(3000, 10000));
                    break;
                }
                case EVENT_CAST_FEAR:
                {
                    if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
                        DoCast(target, SPELL_FEAR);

                    events.ScheduleEvent(EVENT_CAST_FEAR, urand(10000, 20000));
                    break;
                }
// FIXME: Implement Mind Control
//              case EVENT_CAST_MIND_CONTROL:
//              {
//                  break;
//              }
                case EVENT_SPAWN_ADDS:
                {
                    //Spawn 2 random types of creatures at the 2 locations
                    //Spawn creature and force it to start attacking a random target
                    //1 in 3 chance it will be a chromatic

                    uint32 CreatureID = (rand() % 3 == 0) ? CREATURE_CHROMATIC_DRAKANOID : drakTypes[0] + CREATURE_BLUE_DRAKANOID;
                    m_creature->SummonCreature(CreatureID, ADD_X1, ADD_Y1, ADD_Z1, 5.000, TEMPSUMMON_CORPSE_DESPAWN, 0);

                    CreatureID = (rand() % 3 == 0) ? CREATURE_CHROMATIC_DRAKANOID : drakTypes[1] + CREATURE_BLUE_DRAKANOID;
                    m_creature->SummonCreature(CreatureID, ADD_X2, ADD_Y2, ADD_Z2, 5.000, TEMPSUMMON_CORPSE_DESPAWN, 0);

                    SpawnedAdds += 2;

                    if (SpawnedAdds < 42)
                    {
                        events.ScheduleEvent(EVENT_SPAWN_ADDS, 4000);
                        break;
                    }
                }
                case EVENT_SPAWN_NEFARIAN:
                {
                    //Begin phase 2 by spawning Nefarian and what not

                    //Inturrupt any spell casting
                    m_creature->InterruptNonMeleeSpells(false);

                    //Root self
                    DoCast(m_creature,33356);

                    //Make super invis
                    DoCast(m_creature,8149);

                    //Teleport self to a hiding spot (this causes errors in the Trinity log but no real issues)
                    DoTeleportTo(HIDE_X,HIDE_Y,HIDE_Z);
                    m_creature->addUnitState(UNIT_STAT_FLEEING);

                    //Spawn Nefarian
                    if (Creature* Nefarian = m_creature->SummonCreature(CREATURE_NEFARIAN, NEF_X, NEF_Y, NEF_Z, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 900000))
                    {
                        NefarianGUID = Nefarian->GetGUID();
                        NefarianCheckTimer = 30000;
                    }
                    else
                    {
                        error_log("TSCR: Blackwing Lair: Unable to spawn Nefarian properly.");
                        EnterEvadeMode();
                    }
                    break;
                }
            }
        }
    }
};

CreatureAI* GetAI_boss_victor_nefarius(Creature *_Creature)
{
    return new boss_victor_nefariusAI (_Creature);
}

bool GossipHello_boss_victor_nefarius(Player *player, Creature *_Creature)
{
    player->ADD_GOSSIP_ITEM(0, GOSSIP_ITEM_1 , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    player->SEND_GOSSIP_MENU(7134,_Creature->GetGUID());
    return true;
}

bool GossipSelect_boss_victor_nefarius(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(0, GOSSIP_ITEM_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(7198, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM(0, GOSSIP_ITEM_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->SEND_GOSSIP_MENU(7199, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->CLOSE_GOSSIP_MENU();
            DoScriptText(SAY_GAMESBEGIN_1, _Creature);
            ((boss_victor_nefariusAI*)_Creature->AI())->BeginEvent(player);
            break;
    }
    return true;
}

void AddSC_boss_victor_nefarius()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_victor_nefarius";
    newscript->GetAI = &GetAI_boss_victor_nefarius;
    newscript->pGossipHello = &GossipHello_boss_victor_nefarius;
    newscript->pGossipSelect = &GossipSelect_boss_victor_nefarius;
    newscript->RegisterSelf();
}
