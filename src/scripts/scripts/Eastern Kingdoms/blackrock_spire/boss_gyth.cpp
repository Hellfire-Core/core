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

#define SPELL_CORROSIVEACID      20667
#define SPELL_FREEZE             18763
#define SPELL_FLAMEBREATH        20712

struct boss_gythAI : public ScriptedAI
{
    boss_gythAI(Creature *c) : ScriptedAI(c) {}

    int32 Aggro_Timer;
    int32 Dragons_Timer;
    int32 Orc_Timer;
    int32 CorrosiveAcid_Timer;
    int32 Freeze_Timer;
    int32 Flamebreath_Timer;
    uint32 Line1Count;
    uint32 Line2Count;

    bool Event;
    bool SummonedDragons;
    bool SummonedOrcs;
    bool SummonedRend;
    bool bAggro;
    bool RootSelf;
    Creature *SummonedCreature;

    void Reset()
    {
        Dragons_Timer = 3000;
        Orc_Timer = 60000;
        Aggro_Timer = 60000;
        CorrosiveAcid_Timer = 8000;
        Freeze_Timer = 11000;
        Flamebreath_Timer = 4000;
        Event = false;
        SummonedDragons = false;
        SummonedOrcs= false;
        SummonedRend = false;
        bAggro = false;
        RootSelf = false;

        // how many times should the two lines of summoned creatures be spawned
        // min 2 x 2, max 7 lines of attack in total
        Line1Count = rand() % 4 + 2;
        if (Line1Count < 5)
            Line2Count = rand() % (5 - Line1Count) + 2;
        else
            Line2Count = 2;

        //Invisible for event start
        m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, 11686);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void EnterCombat(Unit *who)
    {
    }

    void SummonCreatureWithRandomTarget(uint32 creatureId)
    {
        Unit* Summoned = m_creature->SummonCreature(creatureId, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 240000);
        if (Summoned)
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0);
            if (target)
                Summoned->AddThreat(target, 1.0f);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //char buf[200];

        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (!RootSelf)
        {
            //m_creature->m_canMove = true;
            DoCast(m_creature, 33356);
            RootSelf = true;
        }

        if (!bAggro && Line1Count == 0 && Line2Count == 0)
        {
            Aggro_Timer -= diff;
            if (Aggro_Timer <= diff)
            {
                bAggro = true;
                // Visible now!
                m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, 9723);
                m_creature->setFaction(14);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            } 
        }

        // Summon Dragon pack. 2 Dragons and 3 Whelps
        if (!bAggro && !SummonedRend && Line1Count > 0)
        {
            Dragons_Timer -= diff;
            if (Dragons_Timer <= diff)
            {
                SummonCreatureWithRandomTarget(10372);
                SummonCreatureWithRandomTarget(10372);
                SummonCreatureWithRandomTarget(10442);
                SummonCreatureWithRandomTarget(10442);
                SummonCreatureWithRandomTarget(10442);
                Line1Count = Line1Count - 1;
                Dragons_Timer += 60000;
            } 
        }

        //Summon Orc pack. 1 Orc Handler 1 Elite Dragonkin and 3 Whelps
        if (!bAggro && !SummonedRend && Line1Count == 0 && Line2Count > 0)
        {
            Orc_Timer -= diff;
            if (Orc_Timer <= diff)
            {
                SummonCreatureWithRandomTarget(10447);
                SummonCreatureWithRandomTarget(10317);
                SummonCreatureWithRandomTarget(10442);
                SummonCreatureWithRandomTarget(10442);
                SummonCreatureWithRandomTarget(10442);
                Line2Count = Line2Count - 1;
                Orc_Timer += 60000;
            } 
        }

        // we take part in the fight
        if (bAggro)
        {
            CorrosiveAcid_Timer -= diff;
            if (CorrosiveAcid_Timer <= diff)
            {
                DoCast(m_creature->getVictim(), SPELL_CORROSIVEACID);
                CorrosiveAcid_Timer += 7000;
            } 

            Freeze_Timer -= diff;
            if (Freeze_Timer <= diff)
            {
                DoCast(m_creature->getVictim(), SPELL_FREEZE);
                Freeze_Timer += 16000;
            } 

            Flamebreath_Timer -= diff;
            if (Flamebreath_Timer <= diff)
            {
                DoCast(m_creature->getVictim(),SPELL_FLAMEBREATH);
                Flamebreath_Timer += 10500;
            } 

            //Summon Rend
            if (!SummonedRend && m_creature->GetHealth()*100 / m_creature->GetMaxHealth() < 11
                && m_creature->GetHealth() > 0 )
            {
                //summon Rend and Change model to normal Gyth
                //Inturrupt any spell casting
                m_creature->InterruptNonMeleeSpells(false);
                //Gyth model
                m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, 9806);
                m_creature->SummonCreature(10429, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 900000);
                SummonedRend = true;
            }

            DoMeleeAttackIfReady();
        }                                                   // end if Aggro
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

