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

enum
{
    SPELL_HEAD_BUTT		= 11650,
    SPELL_PIERCE_ARMOUR	= 6016,
    SPELL_ARCANE_EXPLOSION = 37106,

    NPC_HOGGER          = 66716,
    NPC_GRUFF           = 66717,
    NPC_RIVERPAW_ELITE  = 66718,
    NPC_IMP             = 66719,
};

struct npc_hogger_theAI : public ScriptedAI
{
    npc_hogger_theAI(Creature* c) : ScriptedAI(c), summons(c) {};

    Timer pierceTimer;
    Timer stunTimer;
    Timer spawnTimer;
    uint8 pierceCounter;
    uint64 stunTargetGUID;
    uint8 phase; // 1,3 - tank and spank, 2 - ads
    uint8 adsWave;
    SummonList summons;

    void Reset()
    {
        pierceTimer.Reset(3000);
        stunTimer.Reset(5000);
        spawnTimer.Reset(3000);
        pierceCounter = 0;
        stunTargetGUID = 0;
        phase = 1;
        adsWave = 0;
        summons.DespawnAll();
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetVisibility(VISIBILITY_ON);
    }

    void EnterCombat(Unit* who)
    {
        m_creature->Yell("Heroes must die!!",0,0);
    }

    void SummonAds()
    {
        switch (adsWave)
        {
        case 0:
        case 2:
        case 4:
            SummonMobs(15, NPC_IMP);
            break;
        case 1:
            SummonMobs(1, NPC_GRUFF);
            SummonMobs(5, NPC_RIVERPAW_ELITE);
            break;
        case 3:
            SummonMobs(10, NPC_RIVERPAW_ELITE);
            break;
        case 5:
            phase = 3;
            m_creature->SetHealth(m_creature->GetMaxHealth());
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            m_creature->SetVisibility(VISIBILITY_ON);
            DoCast(NULL, SPELL_ARCANE_EXPLOSION, true);
            SummonMobs(5, NPC_RIVERPAW_ELITE);
            return;
        }

        adsWave++;
        spawnTimer = 3000;
    }

    void SummonMobs(uint8 count, uint32 entry)
    {
        for (uint8 i = 0; i < count; i++)
        {
            Position pos;
            m_creature->GetValidPointInAngle(pos, frand(1.0f, 20.0f), frand(0.0f, 2 * M_PI), true);
            if (Creature* imp = m_creature->SummonCreature(entry, pos.x, pos.y, pos.z, pos.o, TEMPSUMMON_CORPSE_DESPAWN, 1000))
            {
                summons.Summon(imp);
                if (Unit* target = imp->SelectNearbyTarget(30.0f))
                    imp->CombatStart(target);
            }
        }
    }

    void SummonedCreatureDespawn(Creature* spawn)
    {
        summons.Despawn(spawn);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        Unit* target = m_creature->getVictim();

        if (phase == 1 || phase == 3)
        {
            if (stunTimer.Expired(diff))
            {
                if (!stunTargetGUID)
                {
                    DoCast(target, SPELL_HEAD_BUTT);
                    stunTargetGUID = target->GetGUID();
                    stunTimer = 2900;
                }
                else
                {
                    if (m_creature->getVictimGUID() == stunTargetGUID && target->HasAura(SPELL_HEAD_BUTT))
                        m_creature->Kill(target);
                    stunTargetGUID = 0;
                    stunTimer = 12100;
                    return; // target could be dead, avoid problems
                }
            }

            if (pierceTimer.Expired(diff))
            {
                DoCast(target, SPELL_PIERCE_ARMOUR);
                pierceTimer = 6000;
            }

            DoMeleeAttackIfReady();

            if (phase == 1 && m_creature->HealthBelowPct(10))
            {
                phase = 2;

                m_creature->Yell("Enough of this!",0,0);
                DoCast(NULL, SPELL_ARCANE_EXPLOSION, true);
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                m_creature->SetVisibility(VISIBILITY_OFF);
            }
        }

        if (phase == 2)
        {
            if (summons.empty() && spawnTimer.Expired(diff))
                SummonAds();
        }
    }
};

CreatureAI* GetAI_npc_hogger_the(Creature* c)
{
    return new npc_hogger_theAI(c);
}

struct npc_gruff_ai : public ScriptedAI
{
    npc_gruff_ai(Creature* c) : ScriptedAI(c) {}

    Timer arcaneTimer;

    void Reset()
    {
        arcaneTimer.Reset(2000);
    }

    void IsSummonedBy(Creature* hogger)
    {
        m_creature->Yell("YELL SOMETHING!!!!111",0,0);
    }

    void JustDied(Unit* killer)
    {
        m_creature->Yell("Ouch, me died...", 0, 0);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;
        
        if (arcaneTimer.Expired(7000))
            DoCast(m_creature->getVictim(), SPELL_ARCANE_EXPLOSION);

        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_gruff(Creature* c)
{
    return new npc_gruff_ai(c);
}

void AddSC_hogger_the()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_event_hogger_the";
    newscript->GetAI = &GetAI_npc_hogger_the;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_event_gruff_swiftbite";
    newscript->GetAI = &GetAI_npc_gruff;
    newscript->RegisterSelf();
}
