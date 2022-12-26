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
SDName: Boss_Supremus
SD%Complete: 95
SDComment: Need to implement molten punch
SDCategory: Black Temple
EndScriptData */

#include "scriptPCH.h"
#include "def_black_temple.h"

#define EMOTE_NEW_TARGET            -1564010
#define EMOTE_PUNCH_GROUND          -1564011
#define EMOTE_GROUND_CRACK          -1564012

//Spells
#define SPELL_MOLTEN_PUNCH          40126
#define SPELL_HATEFUL_STRIKE        41926
#define SPELL_MOLTEN_FLAME          40980
#define SPELL_VOLCANIC_ERUPTION     40117
#define SPELL_VOLCANIC_SUMMON       40276
#define SPELL_BERSERK               27680//45078
#define SPELL_CHARGE                41581
#define SPELL_DIVE_CUSTOM           40279

#define CREATURE_VOLCANO            23085
#define CREATURE_STALKER            23095

struct molten_flameAI : public NullCreatureAI
{
    molten_flameAI(Creature *c) : NullCreatureAI(c)
    {
        pInstance = ((ScriptedInstance *)c->GetInstanceData());
        Punch();
    }

    ScriptedInstance *pInstance;

    float FloatRandRange(float min, float max)
    {
        return ((max-min)*((float)rand()/RAND_MAX))+min;
    }

    void Punch()
    {
        m_creature->SetSpeed(MOVE_WALK, 3.5f, true);
        if(pInstance)
        {
            Unit *supremus = Unit::GetUnit(*m_creature, pInstance->GetData64(DATA_SUPREMUS));
            if(supremus)
            {
                m_creature->setFaction(supremus->getFaction());
                float angle = FloatRandRange(0, 2*M_PI);    //randomise angle of Punch, geting angle from player not working like should
                float x = m_creature->GetPositionX() + 100.0f * cos(angle);
                float y = m_creature->GetPositionY() + 100.0f * sin(angle);
                float z = m_creature->GetTerrain()->GetHeight(x, y, MAX_HEIGHT, true);
                m_creature->GetMotionMaster()->MovePoint(0, x, y, z + 0.05f);
            }
        }
    }
};

struct boss_supremusAI : public ScriptedAI
{
    boss_supremusAI(Creature *c) : ScriptedAI(c), summons(m_creature)
    {
        pInstance = (c->GetInstanceData());
        m_creature->GetPosition(wLoc);

        me->SetAggroRange(50.0f);
    }

    ScriptedInstance* pInstance;

    Timer MoltenFlameTimer;
    
    Timer SummonFlameTimer;
    Timer SwitchTargetTimer;
    Timer PhaseSwitchTimer;
    Timer MoltenPunch_Timer;
    Timer SummonVolcanoTimer;
    Timer HatefulStrikeTimer;
    Timer BerserkTimer;

    Timer CheckTimer;
    WorldLocation wLoc;

    bool Phase1;
    bool DoEmote;

    SummonList summons;

    void Reset()
    {
        ClearCastQueue();

        if (pInstance)
            pInstance->SetData(EVENT_SUPREMUS, NOT_STARTED);

        MoltenFlameTimer.Reset(10000);
        HatefulStrikeTimer.Reset(5000);
        SummonFlameTimer.Reset(20000);
        SwitchTargetTimer.Reset(90000);
        MoltenPunch_Timer.Reset(4000);
        PhaseSwitchTimer.Reset(60000);
        SummonVolcanoTimer.Reset(5000);
        BerserkTimer.Reset(900000);                              // 15 minute enrage
        CheckTimer.Reset(1000);

        WorldLocation wLoc;

        Phase1 = true;
        DoEmote = false;
        summons.DespawnAll();

        m_creature->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 50);   //custom, should be verified
        m_creature->SetFloatValue(UNIT_FIELD_COMBATREACH, 8);
    }

    void EnterCombat(Unit *who)
    {
        DoZoneInCombat();

        if(pInstance)
            pInstance->SetData(EVENT_SUPREMUS, IN_PROGRESS);
    }

    void ToggleDoors(bool close)
    {
        if(!pInstance)
            return;

        if(GameObject* Doors = GameObject::GetGameObject(*m_creature, pInstance->GetData64(DATA_GAMEOBJECT_SUPREMUS_DOORS)))
        {
            if(close)
                Doors->SetGoState(GO_STATE_READY);                   // Closed
            else
                Doors->SetGoState(GO_STATE_ACTIVE);                   // Opened
        }
    }

    void JustDied(Unit *Killer)
    {
        if(pInstance)
        {
            pInstance->SetData(EVENT_SUPREMUS, DONE);
            ToggleDoors(false);
        }
        summons.DespawnAll();
    }

    void JustSummoned(Creature *summon) {summons.Summon(summon);}
    void SummonedCreatureDespawn(Creature *summon) {summons.Despawn(summon);}

    Unit* CalculateHatefulStrikeTarget()
    {
        uint32 health = 0;
        Unit* target = NULL;

        std::list<HostileReference*>& m_threatlist = m_creature->getThreatManager().getThreatList();
        std::list<HostileReference*>::iterator i = m_threatlist.begin();
        for (i = m_threatlist.begin(); i!= m_threatlist.end();++i)
        {
            Unit* pUnit = Unit::GetUnit((*m_creature), (*i)->getUnitGuid());
            if(pUnit && m_creature->IsWithinMeleeRange(pUnit))
            {
                if(pUnit->GetHealth() > health)
                {
                    health = pUnit->GetHealth();
                    target = pUnit;
                }
            }
        }

        return target;
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(!m_creature->HasAura(SPELL_BERSERK, 0))
            if (BerserkTimer.Expired(diff))
                DoCast(m_creature, SPELL_BERSERK);

        if (SummonFlameTimer.Expired(diff))
        {
            AddSpellToCast(m_creature, SPELL_MOLTEN_PUNCH);
            SummonFlameTimer = 20000;
        }


        if(DoEmote)
        {
            DoScriptText(EMOTE_PUNCH_GROUND, m_creature, 0, true);
            DoEmote = false;
        }

        if(Phase1)
        {
            if (CheckTimer.Expired(diff))
            {
                DoZoneInCombat();
                m_creature->SetSpeed(MOVE_RUN, 2.5f);
                CheckTimer = 1000;
            }
            

            if (HatefulStrikeTimer.Expired(diff))
            {
                if(Unit* target = CalculateHatefulStrikeTarget())
                {
                    AddSpellToCast(target, SPELL_HATEFUL_STRIKE);
                    HatefulStrikeTimer = 5000;
                }
            }
        }
        else
        {
            if (CheckTimer.Expired(diff))
            {
                DoZoneInCombat();
                m_creature->SetSpeed(MOVE_RUN, 0.90f);
                CheckTimer = 1000;
            }

            if (SwitchTargetTimer.Expired(diff))
            {
                if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true, m_creature->getVictimGUID()))
                {
                    DoResetThreat();
                    m_creature->getThreatManager().setCurrentVictim((HostileReference*)target);
                    m_creature->AI()->AttackStart(target);
                    m_creature->AddThreat(target, 5000000.0f);
                    DoScriptText(EMOTE_NEW_TARGET, m_creature, 0, true);
                    SwitchTargetTimer = 10000;
                }
            }
            

            if (MoltenPunch_Timer.Expired(diff))
            {
                Unit *target = m_creature->GetVictim();
                if(target)
                {
                    if(m_creature->IsWithinDistInMap(target, 40))
                    {
                        //workaround to make dmg after knockback when distance < 40yd
                        int32 damage = 5600;
                        int32 knock = 175;
                        m_creature->CastCustomSpell(target, SPELL_DIVE_CUSTOM, NULL, &damage, &knock, false);
                    }
                    else
                        m_creature->CastSpell(target, SPELL_CHARGE, true);
                }
                MoltenPunch_Timer = 8000+rand()%4000;
            }
            

            if (SummonVolcanoTimer.Expired(diff))
            {
                if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 999, true))
                {
                    m_creature->CastSpell(target, SPELL_VOLCANIC_SUMMON, true);      //must have target for not to do DB errors
                    SummonVolcanoTimer = 10000;
                }
            }
        }

        if (PhaseSwitchTimer.Expired(diff))
        {
            if(!Phase1)
            {
                Phase1 = true;
                DoEmote = true;
                DoResetThreat();
                PhaseSwitchTimer = 60000;
                m_creature->SetSpeed(MOVE_RUN, 2.5f);
                DoZoneInCombat();
            }
            else
            {
                Phase1 = false;
                DoResetThreat();
                SwitchTargetTimer = 10000;
                SummonVolcanoTimer = 2000;
                PhaseSwitchTimer = 60000;
                m_creature->SetSpeed(MOVE_RUN, 0.90f);
                DoScriptText(EMOTE_GROUND_CRACK, m_creature, 0, true);
                DoZoneInCombat();
            }
        }
        

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

struct npc_volcanoAI : public Scripted_NoMovementAI
{
    npc_volcanoAI(Creature *c) : Scripted_NoMovementAI(c) { }

    Timer CastTimer;

    void Reset()
    {
        CastTimer.Reset(1000);
        me->SetIgnoreVictimSelection(true);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void EnterCombat(Unit *who) {}

    void MoveInLineOfSight(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        if (CastTimer.Expired(diff))
        {
            m_creature->CastSpell(m_creature, SPELL_VOLCANIC_ERUPTION, false);
            CastTimer = 0;
        }
    }
};

CreatureAI* GetAI_boss_supremus(Creature *_Creature)
{
    return new boss_supremusAI (_Creature);
}

CreatureAI* GetAI_molten_flame(Creature *_Creature)
{
    return new molten_flameAI (_Creature);
}

CreatureAI* GetAI_npc_volcano(Creature *_Creature)
{
    return new npc_volcanoAI (_Creature);
}

void AddSC_boss_supremus()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_supremus";
    newscript->GetAI = &GetAI_boss_supremus;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="molten_flame";
    newscript->GetAI = &GetAI_molten_flame;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_volcano";
    newscript->GetAI = &GetAI_npc_volcano;
    newscript->RegisterSelf();
}

