/* 
* Copyright (C) 2009 TrinityCore <http://www.trinitycore.org/>
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
SDName: Boss_Muru/Entropius
SD%Complete: 99
SDComment:
*/

/* Additional scripts

* Shadowsword Berserker
* Shadowsword Fury Mage
* Void Sentinel
* Void Spawn

*/

#include "scriptPCH.h"
#include "def_sunwell_plateau.h"

// Muru & Entropius's spells
enum Spells
{
    SPELL_ENRAGE                                = 26662,

    // Muru's spells
    SPELL_NEGATIVE_ENERGY_PERIODIC              = 46009, //(trigger for 46008)
    SPELL_OPEN_PORTAL_PERIODIC                  = 45994, //(trigger for 45976)
    SPELL_DARKNESS_PERIODIC                     = 45998, //(trigger for 45999)
    SPELL_SUMMON_BLOOD_ELVES_PERIODIC           = 46041, //(trigger for 46037-46040)
    SPELL_OPEN_ALL_PORTALS                      = 46177,
    SPELL_SUMMON_VOID_SENTINEL                  = 45988,
    SPELL_SUMMON_ENTROPIUS                      = 46217,
    SPELL_ENTROPIUS_COSMETIC_SPAWN              = 46223,

    // Entropius's spells
    SPELL_DARKNESS                              = 46269,
    SPELL_BLACK_HOLE                            = 46282,
    SPELL_NEGATIVE_ENERGY_PERIODIC_E            = 46284,

    // M'uru portal target spells
    SPELL_SUMMON_VOID_SENTINEL_SUMMONER         = 45978,
    SPELL_SUMMON_VOID_SENTINEL_SUMMONER_VISUAL  = 45989,

    // Void Sentinel's spells
    SPELL_SHADOW_PULSE_PERIODIC                 = 46086,
    SPELL_SUMMON_VOID_SPAWN                     = 46071,
    SPELL_VOID_BLAST                            = 46161,

    // Void Spawn's spells
    SPELL_SHADOW_BOLT_VOLLEY                    = 46082,

    //Dark Fiend spells
    SPELL_DARKFIEND_AOE                         = 45944,
    SPELL_DARKFIEND_VISUAL                      = 45936,
    SPELL_DARKFIEND_SKIN                        = 45934,

    //Black Hole spells
    SPELL_BLACK_HOLE_SUMMON_VISUAL              = 46242,
    SPELL_BLACK_HOLE_SUMMON_VISUAL_2            = 46247,
    SPELL_BLACK_HOLE_VISUAL_2                   = 46235,
    SPELL_BLACK_HOLE_PASSIVE                    = 46228,

    // Darkness spells
    SPELL_VOID_ZONE_PERIODIC                    = 46262,
    SPELL_SUMMON_DARK_FIEND                     = 46263,
    SPELL_VOID_ZONE_PRE_EFFECT_VISUAL           = 46265,

    // additional spells used
    SPELL_OPEN_PORTAL                           = 45977,
    SPELL_OPEN_PORTAL_2                         = 45976,
    SPELL_TRANSFORM_VISUAL_MISSILE_PERIODIC     = 46205
};

enum Creatures
{
    CREATURE_WORLD_TRIGGER      = 22515,
    CREATURE_DARKNESS           = 25879,
    CREATURE_DARK_FIENDS        = 25744,
    CREATURE_BERSERKER          = 25798,
    CREATURE_FURY_MAGE          = 25799,
    CREATURE_VOID_SENTINEL      = 25772,
    CREATURE_VOID_SPAWN         = 25824,
    CREATURE_BLACK_HOLE         = 25855,
    BOSS_MURU                   = 25741,
    BOSS_ENTROPIUS              = 25840
};

Timer EnrageTimer = 600000;

struct boss_muruAI : public Scripted_NoMovementAI
{
    boss_muruAI(Creature *c) : Scripted_NoMovementAI(c), Summons(me)
    {
        pInstance = c->GetInstanceData();
        me->SetAggroRange(20.0);
    }

    ScriptedInstance* pInstance;
    SummonList Summons;

    Timer ResetTimer;
    Timer HumanoidStart;
    Timer TransitionTimer;

    void Reset()
    {
        ResetTimer = 0;
        EnrageTimer.Reset(600000);
        HumanoidStart.Reset(10000);
        TransitionTimer = 0;
        Summons.DespawnAll();

        if(pInstance->GetData(DATA_EREDAR_TWINS_EVENT) == DONE)
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
            me->SetVisibility(VISIBILITY_ON);
        }
        else
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
            me->SetVisibility(VISIBILITY_OFF);
        }

        pInstance->SetData(DATA_MURU_EVENT, NOT_STARTED);
    }

    void EnterEvadeMode()
    {
        CreatureAI::_EnterEvadeMode();
        me->SetVisibility(VISIBILITY_OFF);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
        Summons.DespawnAll();
        ResetTimer.Reset(30000);
    }

    void EnterCombat(Unit *who)
    {
        me->SetIgnoreVictimSelection(true);
        DoCast(me, SPELL_NEGATIVE_ENERGY_PERIODIC);
        DoCast(me, SPELL_OPEN_PORTAL_PERIODIC);
        DoCast(me, SPELL_DARKNESS_PERIODIC);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if(ResetTimer.GetInterval())
            return;
        if(pInstance->GetData(DATA_EREDAR_TWINS_EVENT) != DONE)
            return;

        ScriptedAI::MoveInLineOfSight(who);
    }

    void DamageTaken(Unit* /*done_by*/, uint32 &damage)
    {
        if (damage >= me->GetHealth())
        {
            damage = 0;
            me->RemoveAllAuras();
            DoCast(me, SPELL_OPEN_ALL_PORTALS);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            TransitionTimer = 5000;
        }
    }

    void JustSummoned(Creature* summoned)
    {
        Summons.Summon(summoned);
    }

    void UpdateAI(const uint32 diff)
    {

        if (ResetTimer.Expired(diff))
        {
            Reset();
        }
        

        if (!UpdateVictim())
            return;

        DoSpecialThings(diff, DO_COMBAT_N_EVADE, 80.0f);

        if (me->GetSelection())
            me->SetSelection(0);

        if (EnrageTimer.Expired(diff))
        {
            DoCast(me, SPELL_ENRAGE, true);
            EnrageTimer = 60000;
        }
        

        if (HumanoidStart.Expired(diff))
        {
            pInstance->SetData(DATA_MURU_EVENT, IN_PROGRESS);
            // if anyone trapped outside front door, evade
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 400, true, 0, 60))
                {
                    EnterEvadeMode();
                    return;
            }
            DoCast(me, SPELL_SUMMON_BLOOD_ELVES_PERIODIC, true);
            HumanoidStart = 0;
        }
        

 
      
        // SOMEHOW we had 2x entropius spawned, so better check it
        if (TransitionTimer.Expired(diff) && !GetClosestCreatureWithEntry(me, BOSS_ENTROPIUS, 400.0f))
        {
            DoCast(me, SPELL_ENTROPIUS_COSMETIC_SPAWN);
            DoCast(me, SPELL_SUMMON_ENTROPIUS);
            me->RemoveAllAuras();
            TransitionTimer = 0;
        }
        
    }
};

CreatureAI* GetAI_boss_muru(Creature *_Creature)
{
    return new boss_muruAI (_Creature);
}

struct boss_entropiusAI : public ScriptedAI
{
    boss_entropiusAI(Creature *c) : ScriptedAI(c), Summons(m_creature)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;
    SummonList Summons;

    Timer TransitionTimer;
    Timer DarknessTimer;
    Timer BlackHole;

    void Reset()
    {
        me->GetMotionMaster()->MoveIdle();
        DoZoneInCombat(80);
        DarknessTimer.Reset(urand(6000, 8000));
        BlackHole.Reset(urand(4000, 10000));
        TransitionTimer.Reset(3000);
        me->SetRooted(true);
    }

    void EnterEvadeMode()
    {
        if(Unit* Muru = me->GetUnit(pInstance->GetData64(DATA_MURU)))
            ((boss_muruAI*)Muru)->EnterEvadeMode();
        me->DisappearAndDie();
        Summons.DespawnAll();
    }

    void JustSummoned(Creature* summoned)
    {
        if( Unit* Muru = me->GetUnit(pInstance->GetData64(DATA_MURU)))
            ((boss_muruAI*)Muru)->JustSummoned(summoned);
        Summons.Summon(summoned);
    }

    void JustDied(Unit* killer)
    {
        if(killer->GetGUID() == me->GetGUID())
            return;
        pInstance->SetData(DATA_MURU_EVENT, DONE);
        if(Unit* Muru = me->GetUnit(pInstance->GetData64(DATA_MURU)))
        {
            Muru->Kill(Muru, false);
            Muru->ToCreature()->RemoveCorpse();
        }
        Summons.DespawnAll();
    }

    void UpdateAI(const uint32 diff)
    {
        if (TransitionTimer.GetInterval())
        {
            if (TransitionTimer.Expired(diff))
            {
                if(Unit* Muru = me->GetUnit(pInstance->GetData64(DATA_MURU)))
                {
                    Muru->SetVisibility(VISIBILITY_OFF);
                    Muru->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
                }
                me->SetRooted(false);
                AttackStart(me->GetVictim());
                DoCast(me, SPELL_NEGATIVE_ENERGY_PERIODIC_E);
                TransitionTimer = 0;
            }
            return;
        }

        if (!UpdateVictim())
            return;

        DoSpecialThings(diff, DO_COMBAT_N_EVADE, 100.0f);

        if (EnrageTimer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_ENRAGE);
            EnrageTimer = 60000;
        }
        
        if (DarknessTimer.Expired(diff))
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true))
                AddSpellToCast(target, SPELL_DARKNESS);
            DarknessTimer = 15000;
        }
        
        if (BlackHole.Expired(diff))
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true, me->getVictimGUID(), 10.0))
                AddSpellToCast(target, SPELL_BLACK_HOLE);
            BlackHole = urand(15000, 18000);
        }
        

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_boss_entropius(Creature *_Creature)
{
    return new boss_entropiusAI (_Creature);
}

struct npc_muru_portalAI : public Scripted_NoMovementAI
{
    npc_muru_portalAI(Creature *c) : Scripted_NoMovementAI(c)
    {
        pInstance = c->GetInstanceData();
        c->GetMotionMaster()->MoveIdle();
    }

    ScriptedInstance* pInstance;
    Creature* Muru;
    Timer SummonTimer;
    Timer TransformTimer;
    Timer CheckTimer;

    void Reset()
    {
        SummonTimer = 0;
        TransformTimer = 0;
        CheckTimer.Reset(1000);
        me->SetReactState(REACT_PASSIVE);

        if (Creature* Muru = me->GetCreature(pInstance->GetData64(DATA_MURU)))
            me->SetFacingToObject(Muru);

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
    }

    void EnterCombat(Unit *who) {}

    void JustSummoned(Creature* summoned)
    {
        if (summoned->GetEntry() == 25782)  // Void Summoner
            DoCast(summoned, SPELL_SUMMON_VOID_SENTINEL_SUMMONER_VISUAL);

        if (Creature* Muru = me->GetCreature(pInstance->GetData64(DATA_MURU)))
            Muru->ToCreature()->AI()->JustSummoned(summoned);
    }

    void SpellHit(Unit* caster, const SpellEntry* Spell)
    {
        switch (Spell->Id)
        {
            case SPELL_OPEN_ALL_PORTALS:
                DoCast(me, SPELL_OPEN_PORTAL);
                TransformTimer = 2000;
                break;
            case SPELL_OPEN_PORTAL_2:
                DoCast(me, SPELL_OPEN_PORTAL);
                SummonTimer = 5000;
                break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(me->IsInCombat())
        {
            if (CheckTimer.Expired(diff))
            {
                if (pInstance->GetData(DATA_MURU_EVENT) == DONE || pInstance->GetData(DATA_MURU_EVENT) == NOT_STARTED)
                    EnterEvadeMode();
                CheckTimer = 1000;
            }
        }

        if (SummonTimer.Expired(diff) && pInstance->GetData(DATA_MURU_EVENT) == IN_PROGRESS)
        {
            DoCast(me, SPELL_SUMMON_VOID_SENTINEL_SUMMONER);
            SummonTimer = 0;
        }

        if (TransformTimer.Expired(diff))
        {
            DoCast(me, SPELL_TRANSFORM_VISUAL_MISSILE_PERIODIC);
            TransformTimer = 0;
        }
       
    }
};

CreatureAI* GetAI_npc_muru_portal(Creature *_Creature)
{
    return new npc_muru_portalAI (_Creature);
}

struct npc_void_summonerAI : public Scripted_NoMovementAI
{
    npc_void_summonerAI(Creature *c) : Scripted_NoMovementAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    Timer SummonTimer;
    ScriptedInstance* pInstance;

    void Reset()
    {
        SummonTimer.Reset(2000);
    }

    void JustSummoned(Creature* summoned)
    {
        if (Creature* Muru = me->GetCreature(pInstance->GetData64(DATA_MURU)))
            Muru->ToCreature()->AI()->JustSummoned(summoned);
    }

    void IsSummonedBy(Unit*)
    {
        if (Creature* Muru = me->GetCreature(pInstance->GetData64(DATA_MURU)))
            Muru->ToCreature()->AI()->JustSummoned(me);
    }

    void UpdateAI(const uint32 diff)
    {    
        if (SummonTimer.Expired(diff))
        {
            DoCast(me, SPELL_SUMMON_VOID_SENTINEL);
            SummonTimer = 10000;
        }
    }
};

CreatureAI* GetAI_npc_void_summoner(Creature *_Creature)
{
    return new npc_void_summonerAI (_Creature);
}

struct npc_dark_fiendAI : public ScriptedAI
{
    npc_dark_fiendAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;
    Timer ActivationTimer;
    Timer CheckTimer;
    Timer DespawnTimer;

    void Reset()
    {
        ActivationTimer.Reset(2000);
        CheckTimer.Reset(500);
        DespawnTimer = 0;
        me->SetRooted(true);
        DoCast(me, SPELL_DARKFIEND_SKIN);
        me->SetSpeed(MOVE_RUN, 0.55);
        me->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.8f);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        DoZoneInCombat(100);
    }

    void EnterEvadeMode()
    {
        me->DisappearAndDie();
    }
    
    void IsSummonedBy(Unit*)
    {
        if (Creature* Muru = me->GetCreature(pInstance->GetData64(DATA_MURU)))
            Muru->ToCreature()->AI()->JustSummoned(me);
    }
    
    void DamageTaken(Unit* /*done_by*/, uint32 &damage)
    {
        if(damage > me->GetHealth())
        {
            damage = 0;
            me->SetRooted(true);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            DoCast(me, SPELL_DARKFIEND_VISUAL);
            DespawnTimer = 3000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!me->HasAura(SPELL_DARKFIEND_SKIN) && !DespawnTimer.GetInterval())
        {
            me->SetRooted(true);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            DoCast(me, SPELL_DARKFIEND_VISUAL);
            DespawnTimer = 3000;
            ActivationTimer = 0;
        }

        if(ActivationTimer.GetInterval())
        {
            if (ActivationTimer.Expired(diff))
            {
                if(Unit* target = SelectUnit(SELECT_TARGET_NEAREST, 0, 100, true))
                {
                    me->SetRooted(false);
                    DoStartMovement(target);
                    me->getThreatManager().addThreat(target, 100000);
                }
                ActivationTimer = 0;
            }
            return;
        }

        if (DespawnTimer.Expired(diff))
        {
            me->DisappearAndDie();
            DespawnTimer = 0;
        }
        

        if (!UpdateVictim() || DespawnTimer.GetInterval())
            return;

        
        if (CheckTimer.Expired(diff))
        {
            if(me->IsWithinDistInMap(me->GetVictim(), 1.0))
            {
                DoCast(((Unit*)NULL), SPELL_DARKFIEND_AOE);
                me->SetRooted(true);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                DespawnTimer = 500;
            }
            CheckTimer = 500;
        }
    }
};

CreatureAI* GetAI_npc_dark_fiend(Creature *_Creature)
{
    return new npc_dark_fiendAI (_Creature);
}

struct npc_void_sentinelAI : public ScriptedAI
{
    npc_void_sentinelAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;
    Timer VoidBlastTimer;
    Timer ActivationTimer;

    void Reset()
    {
        float x,y,z,o;
        me->GetHomePosition(x,y,z,o);
        DoTeleportTo(x,y,71);
        DoCast(me, SPELL_SHADOW_PULSE_PERIODIC);
        VoidBlastTimer.Reset(urand(8000, 12000));
        ActivationTimer.Reset(1500);
        me->SetRooted(true);
        if(pInstance->GetData(DATA_MURU_EVENT) == NOT_STARTED)
            me->DisappearAndDie();
    };

    void EnterEvadeMode()
    {
        me->DisappearAndDie();
    }

    void IsSummonedBy(Unit*)
    {
        if (Creature* Muru = me->GetCreature(pInstance->GetData64(DATA_MURU)))
            Muru->ToCreature()->AI()->JustSummoned(me);
    }

    void DamageTaken(Unit*, uint32 &damage)
    {
        if(damage >= me->GetHealth())
        {
            damage = 0;
            for(uint8 i = 0; i < 8; ++i)
                DoCast(me, SPELL_SUMMON_VOID_SPAWN, true);
            me->DisappearAndDie();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(ActivationTimer.GetInterval())
        {
            if (ActivationTimer.Expired(diff))
            {
                ActivationTimer = 0;
                DoZoneInCombat(100);
                me->SetRooted(false);
            }
            return;
        }

        if (!UpdateVictim())
            return;

        
        if (VoidBlastTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_VOID_BLAST, false);
            VoidBlastTimer = 30000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_void_sentinel(Creature *_Creature)
{
    return new npc_void_sentinelAI (_Creature);
}

struct mob_void_spawnAI : public ScriptedAI
{
    mob_void_spawnAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;
    Timer Volley;
    Timer ActivationTimer;

    void EnterEvadeMode()
    {
        me->DisappearAndDie();
    }
    
    void IsSummonedBy(Unit*)
    {
        if (Creature* Muru = me->GetCreature(pInstance->GetData64(DATA_MURU)))
            Muru->ToCreature()->AI()->JustSummoned(me);
    }
    
    void Reset()
    {
        Volley.Reset(urand(3000, 7000));
        ActivationTimer.Reset(2000);
        me->SetRooted(true);
        DoZoneInCombat(100);
        if(pInstance->GetData(DATA_MURU_EVENT) == NOT_STARTED)
            me->DisappearAndDie();
    };

    void UpdateAI(const uint32 diff)
    {
        if(ActivationTimer.GetInterval())
        {
            if (ActivationTimer.Expired(diff))
            {
                ActivationTimer = 0;
                DoZoneInCombat(100);
                me->SetRooted(false);
            }
            return;
        }

        if (!UpdateVictim())
            return;

       
        if (Volley.Expired(diff))
        {
            DoCast(me, SPELL_SHADOW_BOLT_VOLLEY);
            Volley = urand(5000, 10000);
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_void_spawn(Creature *_Creature)
{
    return new mob_void_spawnAI (_Creature);
}

struct npc_blackholeAI : public ScriptedAI
{
    npc_blackholeAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;
    Timer DespawnTimer;
    Timer VisualTimer;
    Timer ActivationTimer;
    Timer ChasingTimer;
    uint64 victimGUID;

    void Reset()
    {
        DespawnTimer.Reset(urand(15000, 17000));
        me->SetLevitate(true);
        me->SetSpeed(MOVE_FLIGHT, 0.5);
        me->setFaction(14);
        float x,y,z,o;
        me->GetHomePosition(x,y,z,o);
        DoTeleportTo(x,y,72);
        VisualTimer.Reset(2000);
        ActivationTimer.Reset(2000);
        ChasingTimer = 0;
        victimGUID = 0;
        me->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->GetMotionMaster()->MoveIdle();
    }

    void MoveInLineOfSight(Unit *who)
    {
        if(!ChasingTimer.GetInterval())
            return;
        ScriptedAI::MoveInLineOfSight(who);
    }

    void AttackStart(Unit* who) { return; }

    void IsSummonedBy(Unit*)
    {
        if (Creature* Muru = me->GetCreature(pInstance->GetData64(DATA_MURU)))
            Muru->ToCreature()->AI()->JustSummoned(me);

        DoCast(me, SPELL_BLACK_HOLE_SUMMON_VISUAL, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if(VisualTimer.GetInterval())
        {
            if (VisualTimer.Expired(diff))
            {
                DoCast(me, SPELL_BLACK_HOLE_SUMMON_VISUAL, true);
                DoCast(me, SPELL_BLACK_HOLE_SUMMON_VISUAL_2, true);
                VisualTimer = 0;
            }
            return;
        }


        if (ActivationTimer.Expired(diff))
        {
            DoCast(me, SPELL_BLACK_HOLE_VISUAL_2, true);
            DoCast(me, SPELL_BLACK_HOLE_PASSIVE, true);
            ActivationTimer = 0;
            if (Player* victim = GetClosestPlayer(me, 100))
            {
                victimGUID = victim->GetGUID();
                me->GetMotionMaster()->MovePoint(0, victim->GetPositionX(), victim->GetPositionY(), 72.0, false);
            }
            ChasingTimer = 1000;
        }
        

        if(victimGUID && ChasingTimer.GetInterval())
        {
            if (ChasingTimer.Expired(diff))
            {
                if (Unit* victim = me->GetUnit(victimGUID))
                {
                    if(me->IsWithinDistInMap(victim, 5.0))
                    {
                        if(Unit* victim = SelectUnit(SELECT_TARGET_NEAREST, 0, 200, true, me->getVictimGUID(), 8.0))
                        {
                            victimGUID = victim->GetGUID();
                            me->GetMotionMaster()->MovePoint(0, victim->GetPositionX(), victim->GetPositionY(), 72.0, false);
                        }
                    }
                    else
                        me->GetMotionMaster()->MovePoint(0, victim->GetPositionX(), victim->GetPositionY(), 72.0, false);
                }
                ChasingTimer = 2000;
            }
        }

        if (DespawnTimer.Expired(diff))
        {
            me->Kill(me, false);
            me->RemoveCorpse();
        }
        
    }
};

CreatureAI* GetAI_npc_blackhole(Creature *_Creature)
{
    return new npc_blackholeAI (_Creature);
}

struct npc_darknessAI : public Scripted_NoMovementAI
{
    npc_darknessAI(Creature *c) : Scripted_NoMovementAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;
    Timer VoidZoneTimer;
    Timer CheckTimer;

    void Reset()
    {
        DoCast(me, SPELL_VOID_ZONE_PRE_EFFECT_VISUAL, true);
        VoidZoneTimer.Reset(3000);
        CheckTimer.Reset(1000);
    }

    void IsSummonedBy(Unit*)
    {
        if (Creature* Muru = me->GetCreature(pInstance->GetData64(DATA_MURU)))
            Muru->ToCreature()->AI()->JustSummoned(me);
    }

    void UpdateAI(const uint32 diff)
    {

        if (VoidZoneTimer.Expired(diff))
        {
            DoCast(me, SPELL_VOID_ZONE_PERIODIC);
            DoCast(me, SPELL_SUMMON_DARK_FIEND);
            me->RemoveAurasDueToSpell(SPELL_VOID_ZONE_PRE_EFFECT_VISUAL);
            VoidZoneTimer = 0;
        }
        
    
        if (CheckTimer.Expired(diff))
        {
            if (pInstance->GetData(DATA_MURU_EVENT) == DONE || pInstance->GetData(DATA_MURU_EVENT) == NOT_STARTED)
                me->DisappearAndDie();
            CheckTimer = 1000;
        }
        
    }
};

CreatureAI* GetAI_npc_darkness(Creature *_Creature)
{
    return new npc_darknessAI (_Creature);
}

/*
Shadowsword Fury Mage
*/

enum ShadowswordFuryMage
{
    SPELL_FEL_FIREBALL                  = 46101,
    SPELL_SPELL_FURY                    = 46102
};

struct mob_shadowsword_fury_mageAI : public ScriptedAI
{
    mob_shadowsword_fury_mageAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;
    Timer SpellFury;
    Timer ActivationTimer;
    WorldLocation wLoc;

    void Reset()
    {
        me->setActive(true);
        ActivationTimer = 0;
        if(Unit* Trigger = GetClosestCreatureWithEntry(me, CREATURE_WORLD_TRIGGER, 100))
        {
            Trigger->GetPosition(wLoc);
            me->GetMotionMaster()->MovePoint(1, wLoc.coord_x, wLoc.coord_y, wLoc.coord_z, false);
        }
        else
            DoZoneInCombat(400.0f);
        SpellFury.Reset(urand(25000, 35000));
    }

    void OnAuraApply(Aura* aur, Unit* caster, bool stackApply)
    {
        if (aur->GetId() == SPELL_SPELL_FURY)
        {
            ClearCastQueue();
            SetAutocast(SPELL_FEL_FIREBALL, 1000, true);    // 1 sec GCD
        }
    }

    void OnAuraRemove(Aura* aur, bool stackApply)
    {
        if (aur->GetId() == SPELL_SPELL_FURY)
        {
            ClearCastQueue();
            SetAutocast(SPELL_FEL_FIREBALL, RAND(6300, 8300), true);
        }
    }

    void MovementInform(uint32 Type, uint32 Id)
    {
        if(Type == POINT_MOTION_TYPE && Id == 1)
        {
            me->GetMotionMaster()->Clear();
            me->SetRooted(true);
            ActivationTimer = 1000;
        }
    }

    void IsSummonedBy(Unit*)
    {
        if (Creature* Muru = me->GetCreature(pInstance->GetData64(DATA_MURU)))
            Muru->ToCreature()->AI()->JustSummoned(me);
    }

    void UpdateAI(const uint32 diff)
    {
        if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE)
        {
            if (me->GetSelection())
                me->SetSelection(0);
            return;
        }

        if(ActivationTimer.GetInterval())
        {
            if (ActivationTimer.Expired(diff))
            {
                ActivationTimer = 0;
                DoZoneInCombat(400);
                me->SetRooted(false);
                if(me->GetVictim())
                    DoStartMovement(me->GetVictim());
                SetAutocast(SPELL_FEL_FIREBALL, RAND(6300, 8300), true);
            }
            return;
        }

        if(!UpdateVictim())
            return;


        if (SpellFury.Expired(diff))
        {
            ClearCastQueue();
            AddSpellToCast(SPELL_SPELL_FURY, CAST_SELF);
            SpellFury = 60000;
        }

        CastNextSpellIfAnyAndReady(diff);
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_shadowsword_fury_mage(Creature *_Creature)
{
    return new mob_shadowsword_fury_mageAI(_Creature);
}

/*
Shadowsword Berserker
*/

enum ShadowswordBerserker
{
    SPELL_FLURRY                        = 46160,
    SPELL_DUAL_WIELD                    = 29651,
};

struct mob_shadowsword_berserkerAI : public ScriptedAI
{
    mob_shadowsword_berserkerAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;
    WorldLocation wLoc;
    Timer Flurry;
    Timer ActivationTimer;

    void Reset()
    {
        me->setActive(true);
        ActivationTimer = 0;
        if(Unit* Trigger = GetClosestCreatureWithEntry(me, CREATURE_WORLD_TRIGGER, 100))
        {
            Trigger->GetPosition(wLoc);
            me->GetMotionMaster()->MovePoint(1, wLoc.coord_x, wLoc.coord_y, wLoc.coord_z, false);
        }
        else
            DoZoneInCombat(400.0f);
        DoCast(me, SPELL_DUAL_WIELD, true);
        Flurry.Reset(urand(16000, 20000));
    }
    void IsSummonedBy(Unit*)
    {
        if (Creature* Muru = me->GetCreature(pInstance->GetData64(DATA_MURU)))
            Muru->ToCreature()->AI()->JustSummoned(me);
    }

    void MovementInform(uint32 Type, uint32 Id)
    {
        if(Type == POINT_MOTION_TYPE && Id == 1)
        {
            me->GetMotionMaster()->Clear();
            me->SetRooted(true);
            ActivationTimer = 1000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE)
        {
            if (me->GetSelection())
                me->SetSelection(0);
            return;
        }

        if(ActivationTimer.GetInterval())
        {
            if (ActivationTimer.Expired(diff))
            {
                ActivationTimer = 0;
                DoZoneInCombat(400);
                me->SetRooted(false);
                if(me->GetVictim())
                    DoStartMovement(me->GetVictim());
            }
            return;
        }

        if(!UpdateVictim())
            return;

        
        if (Flurry.Expired(diff))
        {
            DoCast(me, SPELL_FLURRY);
            Flurry = urand(15000, 20000);
        }
        

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_shadowsword_berserker(Creature *_Creature)
{
    return new mob_shadowsword_berserkerAI(_Creature);
}

void AddSC_boss_muru()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_muru";
    newscript->GetAI = &GetAI_boss_muru;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_entropius";
    newscript->GetAI = &GetAI_boss_entropius;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_muru_portal";
    newscript->GetAI = &GetAI_npc_muru_portal;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_void_summoner";
    newscript->GetAI = &GetAI_npc_void_summoner;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_dark_fiend";
    newscript->GetAI = &GetAI_npc_dark_fiend;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_void_sentinel";
    newscript->GetAI = &GetAI_npc_void_sentinel;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_void_spawn";
    newscript->GetAI = &GetAI_mob_void_spawn;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_blackhole";
    newscript->GetAI = &GetAI_npc_blackhole;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_darkness";
    newscript->GetAI = &GetAI_npc_darkness;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_shadowsword_fury_mage";
    newscript->GetAI = &GetAI_mob_shadowsword_fury_mage;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_shadowsword_berserker";
    newscript->GetAI = &GetAI_mob_shadowsword_berserker;
    newscript->RegisterSelf();
}
