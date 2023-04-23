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
SDName: Isle_of_Queldanas
SD%Complete: 100
SDComment: Quest support: 11524, 11525, 11532, 11533, 11542, 11543, 11541
SDCategory: Isle Of Quel'Danas
EndScriptData */

/* ContentData
npc_archmage_nethul
npc_ayren_cloudbreaker
npc_converted_sentry
npc_unrestrained_dragonhawk
npc_greengill_slave
npc_madrigosa
npc_brutallus
EndContentData */

#include "scriptPCH.h"
#include "GameEvent.h"

/*######
## npc_archmage_nethul
######*/

#define GOSSIP_SWP_STATE "What is the current progress on Sunwell's offensive?"

bool GossipHello_npc_archmage_nethul(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu(_Creature->GetGUID());

    for(uint32 i = 50; i < 54; ++i)
    {
        if(isGameEventActive(i))
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SWP_STATE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+i);
            break;
        }
    }
    player->SEND_GOSSIP_MENU(12309,_Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_archmage_nethul(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    switch(action)
    {
        case GOSSIP_ACTION_INFO_DEF+50:
            HandleWorldEventGossip(player, _Creature);
            player->SEND_GOSSIP_MENU(12400, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+51:
            HandleWorldEventGossip(player, _Creature);
            player->SEND_GOSSIP_MENU(12401, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+52:
            HandleWorldEventGossip(player, _Creature);
            player->SEND_GOSSIP_MENU(12402, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+54:
            HandleWorldEventGossip(player, _Creature);
            player->SEND_GOSSIP_MENU(12403, _Creature->GetGUID());
            break;
        default:
            break;
    }
    return true;
}

/*######
## npc_ayren_cloudbreaker
######*/

#define GOSSIP_FLY1 "Speaking of action, I've been ordered to undertake an air strike."
#define GOSSIP_FLY2 "I need to intercept the Dawnblade reinforcements."
bool GossipHello_npc_ayren_cloudbreaker(Player *player, Creature *_Creature)
{
    if( player->GetQuestStatus(11532) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(11533) == QUEST_STATUS_INCOMPLETE ||
        (player->GetQuestStatus(11532) == QUEST_STATUS_COMPLETE && !player->GetQuestRewardStatus(11532)) || (player->GetQuestStatus(11533) == QUEST_STATUS_COMPLETE && !player->GetQuestRewardStatus(11533)))
        player->ADD_GOSSIP_ITEM(0, GOSSIP_FLY1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    if( player->GetQuestStatus(11542) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(11543) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, GOSSIP_FLY2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);

    player->SEND_GOSSIP_MENU(12252,_Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_ayren_cloudbreaker(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,45071,true);               //TaxiPath 779
    }
    if (action == GOSSIP_ACTION_INFO_DEF+2)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,45113,true);               //TaxiPath 784
    }
    return true;
}

const char* WretchedQuotes[3] =
{
    "It's not meant for you! Get away from here!",
    "Mine! You shall not take this place!",
    "The rift's power is ours!"
};

/*######
## npc_wretched_devourer
######*/

#define SPELL_ARCANE_TORRENT        33390
#define SPELL_MANA_TAP              33483
#define SPELL_NETHER_SHOCK          35334
// do not regenerates mana OOC - creature extra flag (dec value 16777216)

struct npc_wretched_devourerAI : public ScriptedAI
{
    npc_wretched_devourerAI(Creature* c) : ScriptedAI(c) {}

    Timer ArcaneTorrent;
    Timer ManaTap;
    Timer NetherShock;

    void Reset()
    {
        me->SetPower(POWER_MANA, 0);
        ArcaneTorrent.Reset(RAND(urand(1500, 4500), urand(5000, 10000)));
        ManaTap.Reset(urand(3000, 6000));
        NetherShock.Reset(urand(4000, 8000));
    }

    void EnterCombat(Unit* who)
    {
        if(roll_chance_f(20))
            DoYell(WretchedQuotes[urand(0,2)], LANG_THALASSIAN, who);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (ArcaneTorrent.Expired(diff))
        {
            AddSpellToCast(SPELL_ARCANE_TORRENT, CAST_SELF);
            ArcaneTorrent = RAND(urand(1500, 4500),urand(6000, 11000));
        }

        
        if (ManaTap.Expired(diff))
        {
            AddSpellToCast(SPELL_MANA_TAP, CAST_TANK);
            ManaTap = urand(15000, 24000);
        }
        
        
        if (NetherShock.Expired(diff))
        {
            AddSpellToCast(SPELL_NETHER_SHOCK, CAST_TANK);
            NetherShock = urand(4000, 8000);
        }
        
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_wretched_devourer(Creature* _Creature)
{
    return new npc_wretched_devourerAI(_Creature);
}

/*######
## npc_wretched_fiend
######*/

#define SPELL_SUNDER_ARMOR          11971
#define SPELL_BITTER_WITHDRAWAL     29098

#define SPELL_SLEEPING_SLEEP        42648
// do not regenerates mana OOC - creature extra flag (dec value 16777216)

struct npc_wretched_fiendAI : public ScriptedAI
{
    npc_wretched_fiendAI(Creature* c) : ScriptedAI(c) {}

    Timer SunderArmor;
    Timer BitterWithdrawal;

    void Reset()
    {
        me->SetPower(POWER_MANA, 0);
        SunderArmor.Reset(urand(6000, 10000));
        BitterWithdrawal.Reset(1000);
    }

    void EnterCombat(Unit* who)
    {
        if(roll_chance_f(20))
            DoYell(WretchedQuotes[urand(0,2)], LANG_THALASSIAN, who);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (SunderArmor.Expired(diff))
        {
            AddSpellToCast(SPELL_SUNDER_ARMOR, CAST_TANK);
            SunderArmor = urand(12000, 16000);
        }
        
        if(HealthBelowPct(85))
        {
            if (BitterWithdrawal.Expired(diff))
            {
                AddSpellToCast(SPELL_BITTER_WITHDRAWAL, CAST_TANK);
                BitterWithdrawal = urand(10000, 15000);
            }
            
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_wretched_fiend(Creature* _Creature)
{
    return new npc_wretched_fiendAI(_Creature);
}

/*######
## npc_erratic_sentry
######*/
#define YELL_CORE_OVERLOAD    "Core overload detected. System malfunction detected..."

#define CAPACITATOR_OVERLOAD        45014
#define SPELL_SUPPRESSION           35892
#define SPELL_ELECTRICAL_OVERLOAD   45336
#define SPELL_CRYSTAL_STRIKE        33688
// do not regenerates health OOC, but self repairs when at or below 50%- creature extra flag (dec value 33554432)

struct npc_erratic_sentryAI : public ScriptedAI
{
    npc_erratic_sentryAI(Creature* c) : ScriptedAI(c) {}

    Timer CapacitatorOverload;
    Timer Suppression;
    Timer ElectricalOverload;
    Timer CrystalStrike;

    void Reset()
    {
        CapacitatorOverload.Reset(5000);
        Suppression.Reset(urand(3000, 10000));
        ElectricalOverload.Reset(1000);
        CrystalStrike.Reset(urand(2000, 14000));
    }

    void UpdateAI(const uint32 diff)
    {
        if(!me->IsInCombat())
        {
            if(HealthBelowPct(90))
            {
                if(!me->HasAura(44994))
                {
                    if (CapacitatorOverload.Expired(diff))
                    {
                        DoCast(me, CAPACITATOR_OVERLOAD, true);
                        CapacitatorOverload = 500;
                    }
                    
                }
            }
            else
            {
                if (CapacitatorOverload.Expired(diff))
                {
                    if(roll_chance_i(5))
                    {
                        int32 dmg = 1714;
                        me->CastCustomSpell(me, CAPACITATOR_OVERLOAD, 0, 0, 0, true);
                        CapacitatorOverload = 500;
                        return;
                    }
                    if(HealthBelowPct(100) && roll_chance_i(15))
                        me->SetHealth(me->GetMaxHealth());
                    CapacitatorOverload = 5000;
                }
                
            }
        }

        if(!UpdateVictim())
            return;


        if (Suppression.Expired(diff))
        {
            AddSpellToCast(SPELL_SUPPRESSION, CAST_NULL);
            Suppression = urand(15000, 25000);
        }
        


        if (CrystalStrike.Expired(diff))
        {
            AddSpellToCast(SPELL_CRYSTAL_STRIKE, CAST_TANK);
            CrystalStrike = 14000;
        }
        

        if(HealthBelowPct(80) && !HealthBelowPct(50))
        {
            if (ElectricalOverload.Expired(diff))
            {
                if(roll_chance_i(20))
                    DoYell(YELL_CORE_OVERLOAD, 0, me->GetVictim());
                AddSpellToCast(SPELL_ELECTRICAL_OVERLOAD, CAST_SELF);
                ElectricalOverload = 10000;
            }
            
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_erratic_sentry(Creature* _Creature)
{
    return new npc_erratic_sentryAI(_Creature);
}

/*######
## npc_sunblade_lookout
######*/

#define SPELL_LOOKOUT_SHOOT     45172

const char* LookoutYell[3] =
{
    "Shattered Sun scum! Fire at will!",
    "Don't let that dragonhawk through! Open fire!",
    "Dragonhawk incoming from the west! Shoot that $c down!"
};

struct npc_sunblade_lookoutAI : public Scripted_NoMovementAI
{
    npc_sunblade_lookoutAI(Creature* c) : Scripted_NoMovementAI(c) {}
    void MoveInLineOfSight(Unit *who)
    {
        if(who->GetTypeId() == TYPEID_PLAYER && who->IsTaxiFlying())
        {
            if(me->IsWithinDistInMap(who, 80))
            {
                if(roll_chance_i(8))
                    DoCast(who, SPELL_LOOKOUT_SHOOT);
                if(roll_chance_f(0.3))
                  me->Yell(LookoutYell[urand(0,2)], 0, who->GetGUID());
            }
        }
    }
};

CreatureAI* GetAI_npc_sunblade_lookout(Creature* _Creature)
{
    return new npc_sunblade_lookoutAI(_Creature);
}

/*######
## npc_wrath_enforcer
######*/
#define SPELL_ARCANE_CHARGE         45072
#define SPELL_DUAL_WIELD            29651
#define SPELL_FLAME_WAVE            33803
#define MOB_RAVAGER                 25028
#define MOB_GHOUL                   25027

struct npc_wrath_enforcerAI : public ScriptedAI
{
    npc_wrath_enforcerAI(Creature* c) : ScriptedAI(c) {}

    Timer FlameWave;

    void Reset()
    {
        me->setActive(true);
        DoCast(me, 29651, true);
        FlameWave.Reset(urand(5000, 35000));
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if (spell->Id == SPELL_ARCANE_CHARGE && caster->GetTypeId() == TYPEID_PLAYER)
        {
            m_creature->LowerPlayerDamageReq(m_creature->GetMaxHealth());
            caster->Kill(m_creature);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!me->IsInCombat())
        {
            Unit* Ravager = GetClosestCreatureWithEntry(me, MOB_RAVAGER, 50);
            Unit* Ghoul = GetClosestCreatureWithEntry(me, MOB_GHOUL, 50);
            Unit* target = NULL;

            if(Ravager && Ghoul)
                target = me->GetDistance(Ravager)>me->GetDistance(Ghoul)?Ghoul:Ravager;
            else
                target = Ravager?Ravager:(Ghoul?Ghoul:NULL);
            if(target)
            {
                me->AddThreat(target, 10.0f);
                AttackStart(target);
            }
        }

        if(!UpdateVictim())
            return;


        if (FlameWave.Expired(diff))
        {
            AddSpellToCast(SPELL_FLAME_WAVE, CAST_SELF);
            FlameWave = urand(20000, 30000);
        }
        

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_wrath_enforcer(Creature* _Creature)
{
    return new npc_wrath_enforcerAI(_Creature);
}

/*######
## npc_flame_wave
######*/

#define SPELL_BURN          33802

struct npc_flame_waveAI : public ScriptedAI
{
    npc_flame_waveAI(Creature* c) : ScriptedAI(c) {}

    Timer Burn;

    void IsSummonedBy(Unit *summoner)
    {
        Burn.Reset(1);
        me->SetReactState(REACT_PASSIVE);
        float x, y, z;
        me->SetWalk(true);
        me->SetSpeed(MOVE_WALK, 1.7);
        me->GetNearPoint( x, y, z, 0, 20, summoner->GetAngle(me));
        me->UpdateAllowedPositionZ(x, y, z);
        me->GetMotionMaster()->MovePoint(1, x, y, z);
    }

    void UpdateAI(const uint32 diff)
    {
        if (Burn.Expired(diff))
        {
            DoCast(me, SPELL_BURN, true);
            Burn = 500;
        }
    }
};

CreatureAI* GetAI_npc_flame_wave(Creature* _Creature)
{
    return new npc_flame_waveAI(_Creature);
}

/*######
## npc_pit_overlord
######*/

#define SPELL_CLEAVE                15284
#define SPELL_CONE_OF_FIRE          19630
#define SPELL_DEATH_COIL            32709

struct npc_pit_overlordAI : public ScriptedAI
{
    npc_pit_overlordAI(Creature* c) : ScriptedAI(c) {}

    Timer Cleave;
    Timer ConeOfFire;
    Timer DeathCoil;

    void Reset()
    {
        me->setActive(true);
        Cleave.Reset(urand(5000, 15000));
        ConeOfFire.Reset(urand(1000, 5000));
        DeathCoil.Reset(urand(3000, 8000));
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if (spell->Id == SPELL_ARCANE_CHARGE && caster->GetTypeId() == TYPEID_PLAYER)
        {
            m_creature->LowerPlayerDamageReq(m_creature->GetMaxHealth());
            caster->Kill(m_creature);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!me->IsInCombat())
        {
            Unit* Ravager = GetClosestCreatureWithEntry(me, MOB_RAVAGER, 50);
            Unit* Ghoul = GetClosestCreatureWithEntry(me, MOB_GHOUL, 50);
            Unit* target = NULL;

            if(Ravager && Ghoul)
                target = me->GetDistance(Ravager)>me->GetDistance(Ghoul)?Ghoul:Ravager;
            else
                target = Ravager?Ravager:(Ghoul?Ghoul:NULL);
            if(target)
            {
                me->AddThreat(target, 10.0f);
                AttackStart(target);
            }
        }

        if(!UpdateVictim())
            return;


        if (Cleave.Expired(diff))
        {
            AddSpellToCast(SPELL_CLEAVE);
            Cleave = urand(10000, 20000);
        }
        

        if (ConeOfFire.Expired(diff))
        {
            AddSpellToCast(SPELL_CONE_OF_FIRE, CAST_NULL);
            ConeOfFire = urand(8000, 16000);
        }
        

        
        if (DeathCoil.Expired(diff))
        {
            AddSpellToCast(SPELL_DEATH_COIL);
            DeathCoil = urand(8000, 12000);
        }
        

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_pit_overlord(Creature* _Creature)
{
    return new npc_pit_overlordAI(_Creature);
}

/*######
## npc_eredar_sorcerer
######*/

#define SPELL_FLAMES_OF_DOOM        45046

struct npc_eredar_sorcererAI : public Scripted_NoMovementAI
{
    npc_eredar_sorcererAI(Creature* c) : Scripted_NoMovementAI(c) {}

    void Reset()
    {
        me->setActive(true);
        SetAutocast(SPELL_FLAMES_OF_DOOM, 10000, true);
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if (spell->Id == SPELL_ARCANE_CHARGE && caster->GetTypeId() == TYPEID_PLAYER)
        {
            m_creature->LowerPlayerDamageReq(m_creature->GetMaxHealth());
            caster->Kill(m_creature);
        }

    }

    void UpdateAI(const uint32 diff)
    {
        if(!me->IsInCombat())
        {
            Unit* Ravager = GetClosestCreatureWithEntry(me, MOB_RAVAGER, 100);
            Unit* Ghoul = GetClosestCreatureWithEntry(me, MOB_GHOUL, 100);
            Unit* target = NULL;

            if(Ravager && Ghoul)
                target = me->GetDistance(Ravager)>me->GetDistance(Ghoul)?Ghoul:Ravager;
            else
                target = Ravager?Ravager:(Ghoul?Ghoul:NULL);
            if(target)
            {
                me->AddThreat(target, 10.0f);
                AttackStart(target);
            }
        }
        if(!UpdateVictim())
            return;
        CastNextSpellIfAnyAndReady(diff);
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_eredar_sorcerer(Creature* _Creature)
{
    return new npc_eredar_sorcererAI(_Creature);
}

/*######
## npc_shattered_sun_bombardier
######*/

#define BOMBARDIER_FLY_PATH     1776

const char* BombardierYell[6] =
{
    "Fall into formation! We're approaching the Dead Scar.",
    "Keep your eye on the demons. We're not concerned with killing Scourge today.",
    "Be quick with those charges. Some of those demons are going to take more than one direct hit to bring down.",
    "Move up and hit them from above. Let's try to get $n some cover.",
    "It's show time! Blast them hard, blast them fast!",
    "We've got your back, $n"
};

struct npc_shattered_sun_bombardierAI : public ScriptedAI
{
    npc_shattered_sun_bombardierAI(Creature* c) : ScriptedAI(c) {}

    uint64 PlayerGUID;
    Timer yell_timer;
    int8 yell;
    bool PathFly;

    void Reset()
    {
        //me->SetVisibility(VISIBILITY_OFF);
        //me->setActive(true);
        PlayerGUID = 0;
        yell_timer.Reset(60000000);
        PathFly = false;
        yell = 0;
        me->GetMotionMaster()->MoveIdle();
    }

    void MoveInLineOfSight(Unit* who)
    {
        if(who->GetTypeId() != TYPEID_PLAYER)
            return;

        if (who->IsTaxiFlying() && who->IsWithinDistInMap(me, 40) && !PathFly)
        {
            PlayerGUID = who->GetGUID();
            me->GetMotionMaster()->Clear(false);
            m_creature->SetLevitate(true);
            m_creature->GetMotionMaster()->MovePath(BOMBARDIER_FLY_PATH, false);
            me->SetSpeed(MOVE_WALK, 1.4*who->GetSpeed(MOVE_FLIGHT));
            //me->SetVisibility(VISIBILITY_ON);
            yell_timer = 5000;
            PathFly = true;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (yell_timer.Expired(diff))
        {
            switch(yell)
            {
                case 0:
                    if(me->GetGUIDLow() == 85370)
                        me->Yell(BombardierYell[rand()%3], 0, PlayerGUID);
                    me->SetSpeed(MOVE_WALK, 1.01*me->GetSpeed(MOVE_WALK));
                    yell++;
                    yell_timer = 7000;
                    break;
                case 1:
                    if(me->GetGUIDLow() == 85370)
                        me->Yell(BombardierYell[3+rand()%3], 0, PlayerGUID);
                    yell++;
                    yell_timer = 7000;
                    break;
                case 2:
                    //me->DisappearAndDie();
                    //me->Respawn();
                    break;
                default:
                    break;
            }
        }
        
    }
};

CreatureAI* GetAI_npc_shattered_sun_bombardierAI(Creature* _Creature)
{
    return new npc_shattered_sun_bombardierAI(_Creature);
}

/*######
## npc_unrestrained_dragonhawk
######*/

#define GOSSIP_UD "<Ride the dragonhawk to Sun's Reach>"

bool GossipHello_npc_unrestrained_dragonhawk(Player *player, Creature *_Creature)
{
    player->ADD_GOSSIP_ITEM(0, GOSSIP_UD, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    player->SEND_GOSSIP_MENU(12371,_Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_unrestrained_dragonhawk(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,45353,true);               //TaxiPath 788
    }
    return true;
}

/*######
## npc_greengill_slave
######*/

#define ENRAGE  45111
#define ORB     45109
#define QUESTG  11541
#define DM      25060
#define SIREN   25073

struct npc_greengill_slaveAI : public ScriptedAI
{
    npc_greengill_slaveAI(Creature* c) : ScriptedAI(c) {}

    uint64 PlayerGUID;
    Timer enrageTimer;

    void Reset()
    {
        PlayerGUID = 0;
        enrageTimer.Reset(30000);
    }
    void MovementInform(uint32 type, uint32 id)
    {
        if(type == POINT_MOTION_TYPE && id == 1)
            me->ForcedDespawn();
    }
    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(!caster)
            return;

        if(caster->GetTypeId() == TYPEID_PLAYER && spell->Id == ORB && !m_creature->HasAura(ENRAGE))
        {
            PlayerGUID = caster->GetGUID();
            if(PlayerGUID)
            {
                Player* plr = Unit::GetPlayer(PlayerGUID);
                if(plr && plr->GetQuestStatus(QUESTG) == QUEST_STATUS_INCOMPLETE)
                    plr->KilledMonster(25086, m_creature->GetGUID());
            }
            DoCast(m_creature, ENRAGE);
            me->SetWalk(false);
            me->SetSpeed(MOVE_RUN, 1.5);
            Unit* Myrmidon = GetClosestCreatureWithEntry(me, DM, 100);
            Unit* Siren = GetClosestCreatureWithEntry(me, SIREN, 100);
            Unit* target = NULL;
            if(Myrmidon && Siren)
                target = me->GetDistance(Myrmidon)>me->GetDistance(Siren)?Siren:Myrmidon;
            else
                target = Myrmidon?Myrmidon:(Siren?Siren:NULL);
            if(target)
            {
                me->AddThreat(target, 100000.0f);
                AttackStart(target);
            }
            else
            {
                float x, y, z;
                me->GetNearPoint( x, y, z, 0, 50, frand(0,2*M_PI));
                me->UpdateAllowedPositionZ(x, y, z);
                me->GetMotionMaster()->MovePoint(1, x, y, z);
            }
        }
    }
    void UpdateAI(const uint32 diff)
    {
        if(me->HasAura(ENRAGE))
        {
            if (enrageTimer.Expired(diff))
            {
                me->CombatStop();
                float x, y, z;
                me->GetNearPoint( x, y, z, 0, 15, frand(0,2*M_PI));
                me->UpdateAllowedPositionZ(x, y, z);
                me->GetMotionMaster()->MovePoint(1, x, y, z);
                enrageTimer = 60000;
            }
            
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_greengill_slaveAI(Creature* _Creature)
{
    return new npc_greengill_slaveAI(_Creature);
}


const char* BrutalYell[10] =
{
    //when hitted by q item spell
    "What is this pathetic magic? How about you come back with twenty-four of your best friends and try again, $c",
    //random yells
    "No horror here can compare with what you'll face whe I'm through with you!",
    "Beat or be beaten! This is the way of the Legion!",
    "Burn their bodies, shred their skins, crush their creaking carapaces!",
    "Crush these stinking husks!",
    "Smash them! Grind the bones into the dirt!",
    "Harder, maggots! We must keep the sunwell clear for the master's return!",
    //Brutallus to Magrigosa
    "Grraaarrr! You think to make an icicle out of me? Come down, then I will add real fire to your life."
    "Come down! I tear your wings from your shoulders and feed you to the dirt. Then YOU be the maggot, dragon!"
    "Big talk from a blue birdie! How about you come down and see if you can pluck this maggot from the dirt!"
};

/*######
## npc_ioqd_brutallus
######*/

struct npc_ioqd_brutallusAI : public ScriptedAI
{
    npc_ioqd_brutallusAI(Creature* c) : ScriptedAI(c) {}

    Timer RandYell_timer;

    void Reset()
    {
        RandYell_timer.Reset(urand(15000, 25000));
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(spell->Id == 45072 && caster->GetTypeId() == TYPEID_PLAYER && caster->IsInWorld())
        {
            if(roll_chance_i(40))
                DoYell(BrutalYell[0], 0, caster);
        }
    }

    void UpdateAI(const uint32 diff)
    {    
        if (RandYell_timer.Expired(diff))
        {
            DoYell(BrutalYell[urand(1, 6)], 0, 0);
            RandYell_timer = urand(15000, 25000);
        }
        

        // TODO-> answers to taunting
    }
};

CreatureAI* GetAI_npc_ioqd_brutallus(Creature* _Creature)
{
    return new npc_ioqd_brutallusAI(_Creature);
}

#define SPELL_FROST_BLAST       45201
#define MADRIGOSA_PATH          2499

/*######
## npc_ioqd_madrigosa
######*/

struct npc_ioqd_madrigosaAI : public ScriptedAI
{
    npc_ioqd_madrigosaAI(Creature* c) : ScriptedAI(c) {}

    Timer RandYell_timer;

    void Reset()
    {
        me->SetLevitate(true);
        me->SetSpeed(MOVE_FLIGHT, 1.5);
        me->GetMotionMaster()->MovePath(MADRIGOSA_PATH, true);
        RandYell_timer.Reset(urand(15000, 25000));
    }

    void UpdateAI(const uint32 diff)
    {
        // taunting and casting TODO
    }
};

CreatureAI* GetAI_npc_ioqd_madrigosa(Creature* _Creature)
{
    return new npc_ioqd_madrigosaAI(_Creature);
}

void AddSC_isle_of_queldanas()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_archmage_nethul";
    newscript->pGossipHello = &GossipHello_npc_archmage_nethul;
    newscript->pGossipSelect = &GossipSelect_npc_archmage_nethul;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_ayren_cloudbreaker";
    newscript->pGossipHello = &GossipHello_npc_ayren_cloudbreaker;
    newscript->pGossipSelect = &GossipSelect_npc_ayren_cloudbreaker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_wretched_devourer";
    newscript->GetAI = &GetAI_npc_wretched_devourer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_wretched_fiend";
    newscript->GetAI = &GetAI_npc_wretched_fiend;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_erratic_sentry";
    newscript->GetAI = &GetAI_npc_erratic_sentry;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_sunblade_lookout";
    newscript->GetAI = &GetAI_npc_sunblade_lookout;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_wrath_enforcer";
    newscript->GetAI = &GetAI_npc_wrath_enforcer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_flame_wave";
    newscript->GetAI = &GetAI_npc_flame_wave;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_pit_overlord";
    newscript->GetAI = &GetAI_npc_pit_overlord;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_eredar_sorcerer";
    newscript->GetAI = &GetAI_npc_eredar_sorcerer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_shattered_sun_bombardier";
    newscript->GetAI = &GetAI_npc_shattered_sun_bombardierAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_unrestrained_dragonhawk";
    newscript->pGossipHello = &GossipHello_npc_unrestrained_dragonhawk;
    newscript->pGossipSelect = &GossipSelect_npc_unrestrained_dragonhawk;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_greengill_slave";
    newscript->GetAI = &GetAI_npc_greengill_slaveAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_ioqd_brutallus";
    newscript->GetAI = &GetAI_npc_ioqd_brutallus;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_ioqd_madrigosa";
    newscript->GetAI = &GetAI_npc_ioqd_madrigosa;
    newscript->RegisterSelf();
}
