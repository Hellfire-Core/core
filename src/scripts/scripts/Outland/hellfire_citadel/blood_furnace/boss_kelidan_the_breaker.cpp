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
SDName: Boss_Kelidan_The_Breaker
SD%Complete: 100
SDComment:
SDCategory: Hellfire Citadel, Blood Furnace
EndScriptData */

/* ContentData
boss_kelidan_the_breaker
mob_shadowmoon_channeler
EndContentData */

#include "precompiled.h"
#include "def_blood_furnace.h"

#define SAY_WAKE                    -1542000

#define SAY_ADD_AGGRO_1             -1542001
#define SAY_ADD_AGGRO_2             -1542002
#define SAY_ADD_AGGRO_3             -1542003

#define SAY_KILL_1                  -1542004
#define SAY_KILL_2                  -1542005
#define SAY_NOVA                    -1542006
#define SAY_DIE                     -1542007

#define SPELL_CORRUPTION            30938
#define SPELL_EVOCATION             30935
#define SPELL_BURNING_NOVA          30940
#define SPELL_VORTEX                37370
#define SPELL_FIRE_NOVA             (HeroicMode ? 37371 : 33132)
#define SPELL_SHADOW_BOLT_VOLLEY    (HeroicMode ? 40070 : 28599)

#define SPELL_CHANNELING            39123

#define ENTRY_KELIDAN               17377
#define ENTRY_CHANNELER             17653

static const float ShadowmoonChannelers[5][4]=
{
    {346, -74.5f,  -24.6f, 3.595f},
    {321, -63.5f,  -24.6f, 4.887f},
    {302, -87,     -24.4f, 0.157f},
    {316, -109,    -24.6f, 1.257f},
    {344, -103.5f, -24.5f, 2.356f}

};

class BurningNovaAura : public Aura
{
    public:
        BurningNovaAura(SpellEntry *spell, uint32 eff, Unit *target, Unit *caster) : Aura(spell, eff, NULL, target, caster, NULL){}
};

struct boss_kelidan_the_breakerAI : public ScriptedAI
{
    boss_kelidan_the_breakerAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
        for(int i=0; i<5; ++i) Channelers[i] = 0;
    }

    ScriptedInstance* pInstance;

    Timer ShadowVolley_Timer;
    Timer BurningNova_Timer;
    Timer Firenova_Timer;
    Timer Corruption_Timer;
    bool addYell;

    uint64 Channelers[5];

    void Reset()
    {
        ShadowVolley_Timer.Reset(1000);
        BurningNova_Timer.Reset(15000);
        Corruption_Timer.Reset(5000);
        Firenova_Timer = 0;
        addYell = false;

        if (pInstance)
            pInstance->SetData(DATA_KELIDANEVENT, NOT_STARTED);
        if (!m_creature->IsInEvadeMode())
            JustReachedHome();
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_WAKE, m_creature);

        if (pInstance)
            pInstance->SetData(DATA_KELIDANEVENT, IN_PROGRESS);

        if (m_creature->IsNonMeleeSpellCast(false))
            m_creature->InterruptNonMeleeSpells(true);
        DoStartMovement(who);
    }

    void KilledUnit(Unit* victim)
    {
        if (rand()%2)
            return;

        DoScriptText(RAND(SAY_KILL_1, SAY_KILL_2), m_creature);
    }

    void ChannelerEngaged(Unit* who)
    {
        if(who && !addYell)
        {
            addYell = true;
            DoScriptText(RAND(SAY_ADD_AGGRO_1, SAY_ADD_AGGRO_2, SAY_ADD_AGGRO_3), m_creature);
        }

        for(int i=0; i<5; ++i)
        {
            Creature *channeler = Unit::GetCreature(*m_creature, Channelers[i]);
            if(who && channeler && !channeler->IsInCombat())
                channeler->AI()->AttackStart(who);
        }
    }

    void ChannelerDied(Unit* killer)
    {
        for(int i=0; i<5; ++i)
        {
            Creature *channeler = Unit::GetCreature(*m_creature, Channelers[i]);
            if(channeler && channeler->IsAlive())
                return;
        }

        if(killer)
            AttackStart(killer);
    }

    void SummonChannelers()
    {
        while (Creature *channeler = GetClosestCreatureWithEntry(me, ENTRY_CHANNELER, 500.0f))
        {
            channeler->DisappearAndDie();
        }


        for(uint8 i=0; i<5; i++)
        {
            Creature *channeler = Unit::GetCreature(*m_creature, Channelers[i]);
            if(!channeler || channeler->IsDead())
                channeler = m_creature->SummonCreature(ENTRY_CHANNELER,ShadowmoonChannelers[i][0],ShadowmoonChannelers[i][1],ShadowmoonChannelers[i][2],ShadowmoonChannelers[i][3],TEMPSUMMON_CORPSE_TIMED_DESPAWN,300000);
            if(channeler)
                Channelers[i] = channeler->GetGUID();
            else
                Channelers[i] = 0;
        }

        for (uint8 i=0; i<5; i++)
        {

            if (Unit* caster = me->GetCreature(Channelers[i]))
                if (Unit* receiver = me->GetCreature(Channelers[GetChanneler(i)]))
                    caster->CastSpell(receiver, SPELL_CHANNELING, true);
        }

    }

    uint32 GetChanneler(uint8 i)
    {
        switch (i)
        {
            case 0: return 2; break;
            case 1: return 3; break;
            case 2: return 4; break;
            case 3: return 0; break;
            case 4: return 1; break;
        }
        return 0;
    }
    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DIE, m_creature);

        if(pInstance)
            pInstance->SetData(DATA_KELIDANEVENT, DONE);
    }

    void JustReachedHome()
    {
        SummonChannelers();
        DoCast(m_creature,SPELL_EVOCATION);
    }

    void UpdateAI(const uint32 diff)
    {
        if (Firenova_Timer.Expired(diff))
        {
            ForceSpellCast(me, SPELL_FIRE_NOVA, INTERRUPT_AND_CAST_INSTANTLY);
            ShadowVolley_Timer = 2000;
            Firenova_Timer = 0;
        }

        if (!UpdateVictim())
            return;

        if (ShadowVolley_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature, SPELL_SHADOW_BOLT_VOLLEY);
            ShadowVolley_Timer = urand(5000, 13000);
        }

        if (Corruption_Timer.Expired(diff))
        {
            AddSpellToCast(me,SPELL_CORRUPTION);
            Corruption_Timer = urand(30000, 50000);
        }

        if (BurningNova_Timer.Expired(diff))
        {
            if (m_creature->IsNonMeleeSpellCast(false))
                m_creature->InterruptNonMeleeSpells(true);

            DoScriptText(SAY_NOVA, m_creature);

            if (HeroicMode)
                ForceSpellCast(me, SPELL_VORTEX, INTERRUPT_AND_CAST_INSTANTLY);

            if (SpellEntry *nova = (SpellEntry*)GetSpellStore()->LookupEntry(SPELL_BURNING_NOVA))
            {
                for (uint32 i = 0; i < 3; ++i)
                {
                    if (nova->Effect[i] == SPELL_EFFECT_APPLY_AURA)
                    {
                        Aura *Aur = new BurningNovaAura(nova, i, m_creature, m_creature);
                        m_creature->AddAura(Aur);
                    }
                }
            }

            BurningNova_Timer = urand(20000, 28000);
            Firenova_Timer.Reset(5000);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_kelidan_the_breaker(Creature *_Creature)
{
    return new boss_kelidan_the_breakerAI (_Creature);
}

/*######
## mob_shadowmoon_channeler
######*/

#define SPELL_SHADOW_BOLT       (HeroicMode ? 15472 : 12739)

#define SPELL_MARK_OF_SHADOW    30937

struct mob_shadowmoon_channelerAI : public ScriptedAI
{
    mob_shadowmoon_channelerAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    Timer ShadowBolt_Timer;
    Timer MarkOfShadow_Timer;

    void Reset()
    {
        ShadowBolt_Timer.Reset(urand(1000, 2000));
        MarkOfShadow_Timer.Reset(urand(5000, 7000));
        if (m_creature->IsNonMeleeSpellCast(false))
            m_creature->InterruptNonMeleeSpells(true);
        if (Creature* Kelidan = GetClosestCreatureWithEntry(me, ENTRY_KELIDAN, 500.0f))
            Kelidan->AI()->EnterEvadeMode();
    }

    void EnterCombat(Unit* who)
    {
        if(Creature *Kelidan = (Creature *)FindCreature(ENTRY_KELIDAN, 100, m_creature))
            ((boss_kelidan_the_breakerAI*)Kelidan->AI())->ChannelerEngaged(who);


        me->InterruptNonMeleeSpells(true);
        me->RemoveAurasDueToSpell(SPELL_CHANNELING);
        DoStartMovement(who);
    }

    void JustDied(Unit* Killer)
    {
        if(Creature *Kelidan = (Creature *)FindCreature(ENTRY_KELIDAN, 100, m_creature))
            ((boss_kelidan_the_breakerAI*)Kelidan->AI())->ChannelerDied(Killer);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (MarkOfShadow_Timer.Expired(diff))
        {
            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                AddSpellToCast(target,SPELL_MARK_OF_SHADOW);

            MarkOfShadow_Timer = 15000+rand()%5000;
        }

        if (ShadowBolt_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_SHADOW_BOLT);
            ShadowBolt_Timer = urand(5000, 6000);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_shadowmoon_channeler(Creature *_Creature)
{
    return new mob_shadowmoon_channelerAI (_Creature);
}

void AddSC_boss_kelidan_the_breaker()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_kelidan_the_breaker";
    newscript->GetAI = &GetAI_boss_kelidan_the_breaker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_shadowmoon_channeler";
    newscript->GetAI = &GetAI_mob_shadowmoon_channeler;
    newscript->RegisterSelf();
}

