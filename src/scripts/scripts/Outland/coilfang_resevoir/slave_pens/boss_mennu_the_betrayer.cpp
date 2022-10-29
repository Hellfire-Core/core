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
enum mennu
{
    SPELL_CORRUPTED_NOVA_TOTEM    = 31991,
    SPELL_LIGHTNING_BOLT          = 35010,
    SPELL_EARTHGRAB_TOTEM         = 31981,
    SPELL_STONESKIN_TOTEM         = 31985,
    SPELL_HEALING_TOTEM           = 34980,

    SPELL_FIRE_NOVA               = 33132,
    SPELL_ENTANGLING_ROOTS        = 20654,

    SAY_AGGRO_1                   = -1545024,
    SAY_AGGRO_2                   = -1545025,
    SAY_AGGRO_3                   = -1545026,
    SAY_KILL_1                    = -1545027,
    SAY_KILL_2                    = -1545028,
    SAY_DIE                       = -1545029
};

struct boss_mennu_the_betrayerAI : public ScriptedAI
{
    boss_mennu_the_betrayerAI(Creature *c) : ScriptedAI(c), Summons(m_creature)
    {
        pInstance = (c->GetInstanceData());
    }


    ScriptedInstance *pInstance;

    SummonList Summons;

    Timer HealingWard_Timer;
    Timer NovaTotem_Timer;
    Timer LightningBolt_Timer;
    Timer EarthGrab_Timer;
    Timer StoneSkin_Timer;


    void Reset()
    {
        ClearCastQueue();

        HealingWard_Timer.Reset(15000);
        NovaTotem_Timer.Reset(45000);
        LightningBolt_Timer.Reset(10000);
        EarthGrab_Timer.Reset(15000);
        StoneSkin_Timer.Reset(15000);
        Summons.DespawnAll();
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(RAND(SAY_AGGRO_1, SAY_AGGRO_2, SAY_AGGRO_3), m_creature);
    }


    void JustSummoned(Creature* summoned)
    {
        Summons.Summon(summoned);
    }

    void SummonedCreatureDespawn(Creature *summon)
    {
        Summons.Despawn(summon);
    }

    void DamageTaken(Unit*, uint32 &)
    {
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_KILL_1, SAY_KILL_2), m_creature);
    }

    void JustDied(Unit *u)
    {
        DoScriptText(SAY_DIE, m_creature);
        Summons.DespawnAll();
    }


    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (HealingWard_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature, SPELL_HEALING_TOTEM);
            HealingWard_Timer = 30000;
        }
        

        if (NovaTotem_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature, SPELL_CORRUPTED_NOVA_TOTEM);
            NovaTotem_Timer = 45000;
        }


        if (LightningBolt_Timer.Expired(diff))
        {
            if (HeroicMode)
                AddCustomSpellToCast(m_creature->GetVictim(), SPELL_LIGHTNING_BOLT,142,0,0);
            else
            AddCustomSpellToCast(m_creature->GetVictim(), SPELL_LIGHTNING_BOLT,175,0,0);
            LightningBolt_Timer = 10000;
        }
        

        if (EarthGrab_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature, SPELL_EARTHGRAB_TOTEM);
            EarthGrab_Timer = 30000;
        }
        

        if (StoneSkin_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature, SPELL_STONESKIN_TOTEM);
            StoneSkin_Timer = 60000;
        }
        

        CastNextSpellIfAnyAndReady(diff);
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_mennu_the_betrayer(Creature *_Creature)
{
    return new boss_mennu_the_betrayerAI (_Creature);
}

struct npc_corrupted_nova_totemAI : public Scripted_NoMovementAI
{
    npc_corrupted_nova_totemAI(Creature *c) : Scripted_NoMovementAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;
    Timer Life_Timer;
    uint32 Phase;

    void Reset()
    {
        Life_Timer.Reset(HeroicMode ? 15000 : 5000);
        Phase = 0;
    }

    void JustDied(Unit *u)
    {

    }

    void DamageTaken(Unit*, uint32& damage)
    {
        if(Phase == 0)
        {
            if(damage >= m_creature->GetHealth())
            {
                DoCast(m_creature, SPELL_FIRE_NOVA);
                Life_Timer = 1500;
                Phase = 1;
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }
        }
        if(Phase == 1)
            damage = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (Life_Timer.Expired(diff))
        {
            if(Phase == 1)
                Phase = 2;
            if(Phase < 2)
                m_creature->DealDamage(m_creature, m_creature->GetHealth(), DIRECT_DAMAGE);
        }
    }

};

CreatureAI* GetAI_npc_corrupted_nova_totem(Creature *_Creature)
{
    return new npc_corrupted_nova_totemAI (_Creature);
}

struct npc_earthgrab_totemAI : public Scripted_NoMovementAI
{
    npc_earthgrab_totemAI(Creature *c) : Scripted_NoMovementAI(c)
    {
    }

    Timer Earthgrab_Timer;

    void Reset()
    {
        Earthgrab_Timer.Reset(4000);
    }

    void JustDied(Unit *u)
    {
    }


    void UpdateAI(const uint32 diff)
    {
        if (Earthgrab_Timer.Expired(diff))
        {
            DoCast(m_creature, SPELL_ENTANGLING_ROOTS);
            Earthgrab_Timer = 18000 + rand()%4000;
        }
    }

};

CreatureAI* GetAI_npc_earthgrab_totem(Creature *_Creature)
{
    return new npc_earthgrab_totemAI (_Creature);
}

void AddSC_boss_mennu_the_betrayer()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_mennu_the_betrayer";
    newscript->GetAI = &GetAI_boss_mennu_the_betrayer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_corrupted_nova_totem";
    newscript->GetAI = &GetAI_npc_corrupted_nova_totem;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_earthgrab_totem";
    newscript->GetAI = &GetAI_npc_earthgrab_totem;
    newscript->RegisterSelf();
}
