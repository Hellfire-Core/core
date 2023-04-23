/*
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Boss_Balinda
SD%Complete:
SDComment: Timers should be adjusted
EndScriptData */

#include "scriptPCH.h"

#define YELL_AGGRO                        -1001019
#define YELL_EVADE                        -1001020

enum Balinda
{
    SPELL_ARCANE_EXPLOSION  = 46608,
    SPELL_CONE_OF_COLD      = 38384,
    SPELL_FIREBALL          = 46988,
    SPELL_FROSTBOLT         = 46987,
    SPELL_WATER_ELEMENTAL   = 45067
};

struct boss_balindaAI : public ScriptedAI
{
    boss_balindaAI(Creature *c) : ScriptedAI(c), summons(c)
    {
        m_creature->GetPosition(wLoc);
    }

    Timer CoCTimer;
    Timer CheckTimer;
    Timer WaterElementalTimer;
    Timer CastTimer;
    uint32 SpellId;
    WorldLocation wLoc;
    SummonList summons;

    void Reset()
    {
        CoCTimer.Reset(8000);
        CheckTimer.Reset(2000);
        CastTimer.Reset(1);
        SpellId = 0;
        WaterElementalTimer.Reset(1);

        summons.DespawnAll();
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(YELL_AGGRO, m_creature);
    }

    void JustRespawned()
    {
        Reset();
    }

    void JustSummoned(Creature* summ)
    {
        if (summ)
        {
            summons.Summon(summ);
            summ->setFaction(me->getFaction());
            summ->SetLevel(me->GetLevel());
            summ->SetMaxHealth(6300 + (summ->GetLevel() - 60)*360);
            summ->SetPower(POWER_MANA, 6000 + (summ->GetLevel() - 60)*300);
            summ->AI()->AttackStart(me->GetVictim());
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (CheckTimer.Expired(diff))
        {
            if (!m_creature->IsWithinDistInMap(&wLoc, 20))
            {
                m_creature->InterruptNonMeleeSpells(false);
                EnterEvadeMode();
                return;
            }
            CheckTimer = 2000;
        }

        if (WaterElementalTimer.Expired(diff))
        {
            ForceSpellCast(m_creature, SPELL_WATER_ELEMENTAL);
            WaterElementalTimer = 90000; // 90s
        }

        // update CoC timer
        if (CoCTimer.Expired(diff))
            CoCTimer = 0;

        // select spell
        if (CastTimer.Expired(diff))
        {
            // if victim is in range of 6.5 yards and there are 3 attackers cast explosion or CoC if ready
            if (m_creature->GetAttackers().size() >= 3 && m_creature->IsWithinDistInMap(m_creature->GetVictim(), 6.5f, false))
            {
                if (!CoCTimer.GetInterval())
                {
                    ForceSpellCast(me->GetVictim(), SPELL_CONE_OF_COLD);
                    CoCTimer = urand(8000, 12000);
                    CastTimer = 1;
                }
                else
                {
                    ForceSpellCast(me->GetVictim(), SPELL_ARCANE_EXPLOSION);
                    CastTimer = 2000;
                }
            }
            else
            {
                AddSpellToCast(m_creature->GetVictim(), RAND(SPELL_FROSTBOLT, SPELL_FIREBALL));
                CastTimer = 2500;
            }
        }

        CastNextSpellIfAnyAndReady();
        //DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_balinda(Creature *_Creature)
{
    return new boss_balindaAI (_Creature);
}

// WATER ELEMENTAL

enum WaterElemental
{
    SPELL_WATER_BOLT    = 46983
};

struct mob_av_water_elementalAI : public ScriptedAI
{
    mob_av_water_elementalAI(Creature *c) : ScriptedAI(c) {}

    void Reset() {}

    void EnterCombat(Unit *who)
    {
        SetAutocast(SPELL_WATER_BOLT, 1000, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady(diff);
    }
};

CreatureAI* GetAI_mob_av_water_elemental(Creature *_Creature)
{
    return new mob_av_water_elementalAI (_Creature);
}

void AddSC_boss_balinda()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_balinda";
    newscript->GetAI = &GetAI_boss_balinda;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_av_water_elemental";
    newscript->GetAI = &GetAI_mob_av_water_elemental;
    newscript->RegisterSelf();
}
