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
SDName: Boss_Gyth
SD%Complete: 100
SDComment:
SDCategory: Blackrock Spire
EndScriptData */

#include "precompiled.h"

enum
{
    PHASE_NONE      = 0,
    PHASE_WAVE_1    = 1,
    PHASE_WAVE_4    = 4,
    PHASE_WAVE_5    = 5,
    PHASE_WAIT      = 6,
    PHASE_COMBAT    = 7,
    PHASE_REND      = 8,

    SPELL_CORROSIVEACID     = 20667,
    SPELL_FREEZE            = 18763,
    SPELL_FLAMEBREATH       = 20712,
};

struct boss_gythAI : public ScriptedAI
{
    boss_gythAI(Creature *c) : ScriptedAI(c) {}

    Timer CorrosiveAcidTimer;
    Timer FreezeTimer;
    Timer FlamebreathTimer;
    Timer wavesTimer;
    uint8 phase;
    

    void Reset()
    {
        CorrosiveAcidTimer.Reset(8000);
        FreezeTimer.Reset(11000);
        FlamebreathTimer.Reset(4000);
        wavesTimer.Reset(3000);
        phase = PHASE_NONE;

        //Invisible for event start
        m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, 11686);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (phase == PHASE_NONE && me->canStartAttack(who))
            phase = PHASE_WAVE_1;
        if (phase >= PHASE_COMBAT)
            ScriptedAI::MoveInLineOfSight(who);
    }

    void SummonCreatureWithRandomTarget(uint32 creatureId)
    {
        Creature* Summoned = m_creature->SummonCreature(creatureId, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 240000);
        if (Summoned)
            Summoned->AI()->DoZoneInCombat();
    }

    void UpdateAI(const uint32 diff)
    {
        if (phase >= PHASE_WAVE_1 && phase <= PHASE_WAVE_5)
        {
            if (wavesTimer.Expired(diff))
            {
                SummonCreatureWithRandomTarget(phase >= PHASE_WAVE_4 ? 10447: 10372);
                SummonCreatureWithRandomTarget(phase >= PHASE_WAVE_4 ? 10317 : 10372);
                SummonCreatureWithRandomTarget(10442);
                SummonCreatureWithRandomTarget(10442);
                SummonCreatureWithRandomTarget(10442);
                wavesTimer = 60000;
                phase++;
            }
        }

        if (phase == PHASE_WAIT && wavesTimer.Expired(diff))
        {
            m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, 9723);
            m_creature->setFaction(14);
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            phase++;
        }

        if (phase < PHASE_COMBAT)
            return;

        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (CorrosiveAcidTimer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_CORROSIVEACID);
            CorrosiveAcidTimer = 7000;
        } 

        if (FreezeTimer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_FREEZE);
            FreezeTimer = 16000;
        } 

        if (FlamebreathTimer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FLAMEBREATH);
            FlamebreathTimer = 10500;
        } 

        //Summon Rend
        if (phase == PHASE_COMBAT && HealthBelowPct(11))
        {
            //summon Rend and Change model to normal Gyth
            //Inturrupt any spell casting
            m_creature->InterruptNonMeleeSpells(false);
            //Gyth model
            m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, 9806);
            m_creature->SummonCreature(10429, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 900000);
            phase = PHASE_REND;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_gyth(Creature *_Creature)
{
    return new boss_gythAI (_Creature);
}

void AddSC_boss_gyth()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_gyth";
    newscript->GetAI = &GetAI_boss_gyth;
    newscript->RegisterSelf();
}

