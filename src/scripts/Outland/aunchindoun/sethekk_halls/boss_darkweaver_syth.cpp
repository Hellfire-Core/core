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
SDName: Boss_Darkweaver_Syth
SD%Complete: 85
SDComment: Shock spells/times need more work. Heroic partly implemented.
SDCategory: Auchindoun, Sethekk Halls
EndScriptData */

#include "scriptPCH.h"
#include "def_sethekk_halls.h"

#define SAY_SUMMON                  -1556000

#define SAY_AGGRO_1                 -1556001
#define SAY_AGGRO_2                 -1556002
#define SAY_AGGRO_3                 -1556003

#define SAY_SLAY_1                  -1556004
#define SAY_SLAY_2                  -1556005

#define SAY_DEATH                   -1556006

#define SAY_LAKKA                  -1900253
#define NPC_LAKKA                   18956

#define SPELL_FROST_SHOCK           21401 //37865
#define SPELL_FLAME_SHOCK           34354
#define SPELL_SHADOW_SHOCK          30138
#define SPELL_ARCANE_SHOCK          37132

#define SPELL_CHAIN_LIGHTNING       15659 //15305

#define NPC_ELEMENTAL               19203
#define NUM_ELEMENTALS              4

#define SPELL_FLAME_BUFFET          (HeroicMode?38141:33526)
#define SPELL_ARCANE_BUFFET         (HeroicMode?38138:33527)
#define SPELL_FROST_BUFFET          (HeroicMode?38142:33528)
#define SPELL_SHADOW_BUFFET         (HeroicMode?38143:33529)

struct boss_darkweaver_sythAI : public ScriptedAI
{
    boss_darkweaver_sythAI(Creature *c) : ScriptedAI(c), summons(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance *pInstance;

    SummonList summons;

    Timer flameshock_timer;
    Timer arcaneshock_timer;
    Timer frostshock_timer;
    Timer shadowshock_timer;
    Timer chainlightning_timer;

    bool summon90;
    bool summon50;
    bool summon10;

    void Reset()
    {
        summons.DespawnAll();

        flameshock_timer.Reset(2000);
        arcaneshock_timer.Reset(4000);
        frostshock_timer.Reset(6000);
        shadowshock_timer.Reset(8000);
        chainlightning_timer.Reset(15000);

        summon90 = false;
        summon50 = false;
        summon10 = false;

        if(pInstance)
            pInstance->SetData(DATA_DARKWEAVEREVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(RAND(SAY_AGGRO_1, SAY_AGGRO_2, SAY_AGGRO_3), m_creature);
        if(pInstance)
            pInstance->SetData(DATA_DARKWEAVEREVENT, IN_PROGRESS);
    }

    void JustDied(Unit* Killer)
    {
        summons.DespawnAll();

        DoScriptText(SAY_DEATH, m_creature);

        if (Creature* lakka = GetClosestCreatureWithEntry(me, NPC_LAKKA, 25.0f))
            DoScriptText(SAY_LAKKA, lakka);

        if(pInstance)
            pInstance->SetData(DATA_DARKWEAVEREVENT, DONE);
    }

    void KilledUnit(Unit* victim)
    {
        if (rand()%2)
            return;

        DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), m_creature);
    }

    void JustSummoned(Creature *summoned)
    {
        if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0, 60, true))
            summoned->AI()->AttackStart(target);

        summons.Summon(summoned);
    }

    void SythSummoning()
    {
        DoScriptText(SAY_SUMMON, m_creature);

        if (m_creature->IsNonMeleeSpellCast(false))
            m_creature->InterruptNonMeleeSpells(false);

        float px, py, pz;

        for (int id = NUM_ELEMENTALS; id--; )
        {
            me->GetNearPoint(px, py, pz, 0.0f, 8.0f, 0.5f * id * M_PI);

            if (Creature *elemental = me->SummonCreature(NPC_ELEMENTAL + id, px, py, pz, me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN, 0))
                summons.Summon(elemental);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (((m_creature->GetHealth()*100) / m_creature->GetMaxHealth() < 90) && !summon90)
        {
            SythSummoning();
            summon90 = true;
        }

        if (((m_creature->GetHealth()*100) / m_creature->GetMaxHealth() < 50) && !summon50)
        {
            SythSummoning();
            summon50 = true;
        }

        if (((m_creature->GetHealth()*100) / m_creature->GetMaxHealth() < 10) && !summon10)
        {
            SythSummoning();
            summon10 = true;
        }

        if (flameshock_timer.Expired(diff))
        {
            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))
                DoCast(target, SPELL_FLAME_SHOCK);

            flameshock_timer = 10000 + rand() % 5000;
        }

        if (arcaneshock_timer.Expired(diff))
        {
            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))
                DoCast(target,SPELL_ARCANE_SHOCK);

            arcaneshock_timer = 10000 + rand()%5000;
        } 

        if (frostshock_timer.Expired(diff))
        {
            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))
                DoCast(target, SPELL_FROST_SHOCK);

            frostshock_timer = 10000 + rand() % 5000;
        }

        if (shadowshock_timer.Expired(diff))
        {
            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0, 60, true))
                DoCast(target,SPELL_SHADOW_SHOCK);

            shadowshock_timer = 10000 + rand()%5000;
        } 

        if (chainlightning_timer.Expired(diff))
        {
            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0, 60, true))
                DoCast(target,SPELL_CHAIN_LIGHTNING);

            chainlightning_timer = 25000;
        } 

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_darkweaver_syth(Creature *_Creature)
{
    return new boss_darkweaver_sythAI (_Creature);
}

/* ELEMENTALS */

struct mob_syth_fireAI : public ScriptedAI
{
    mob_syth_fireAI(Creature *c) : ScriptedAI(c)

    {
    }

    Timer flameshock_timer;
    Timer flamebuffet_timer;

    void Reset()
    {
        m_creature->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FIRE, true);
        flameshock_timer.Reset(2500);
        flamebuffet_timer.Reset(5000);
    }

    void EnterCombat(Unit *who) { }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (flameshock_timer.Expired(diff))
        {
            if( Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0, 60, true) )
                DoCast(target,SPELL_FLAME_SHOCK);

            flameshock_timer = 5000;
        }
        

        if (flamebuffet_timer.Expired(diff))
        {
            if( Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0, 60, true) )
                DoCast(target,SPELL_FLAME_BUFFET);

            flamebuffet_timer = 5000;
        }


        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_syth_fire(Creature *_Creature)
{
    return new mob_syth_fireAI (_Creature);
}

struct mob_syth_arcaneAI : public ScriptedAI
{
    mob_syth_arcaneAI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer arcaneshock_timer;
    Timer arcanebuffet_timer;

    void Reset()
    {
        m_creature->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_ARCANE, true);
        arcaneshock_timer.Reset(2500);
        arcanebuffet_timer.Reset(5000);
    }

    void EnterCombat(Unit *who) { }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (arcaneshock_timer.Expired(diff))
        {
            if( Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0, 60, true) )
                DoCast(target,SPELL_ARCANE_SHOCK);

            arcaneshock_timer = 5000;
        }


        if (arcanebuffet_timer.Expired(diff))
        {
            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))
                DoCast(target, SPELL_ARCANE_BUFFET);

            arcanebuffet_timer = 5000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_syth_arcane(Creature *_Creature)
{
    return new mob_syth_arcaneAI (_Creature);
}

struct mob_syth_frostAI : public ScriptedAI
{
    mob_syth_frostAI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer frostshock_timer;
    Timer frostbuffet_timer;

    void Reset()
    {
        m_creature->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FROST, true);
        frostshock_timer.Reset(2500);
        frostbuffet_timer.Reset(5000);
    }

    void EnterCombat(Unit *who) { }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (frostshock_timer.Expired(diff))
        {
            if( Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0, 60, true) )
                DoCast(target,SPELL_FROST_SHOCK);

            frostshock_timer = 5000;
        }
        

        if (frostbuffet_timer.Expired(diff))
        {
            if( Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0, 60, true) )
                DoCast(target,SPELL_FROST_BUFFET);

            frostbuffet_timer = 5000;
        }
        

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_syth_frost(Creature *_Creature)
{
    return new mob_syth_frostAI (_Creature);
}

struct mob_syth_shadowAI : public ScriptedAI
{
    mob_syth_shadowAI(Creature *c) : ScriptedAI(c)

    {
        HeroicMode = m_creature->GetMap()->IsHeroic();
    }

    Timer shadowshock_timer;
    Timer shadowbuffet_timer;
    bool HeroicMode;

    void Reset()
    {
        m_creature->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_SHADOW, true);
        shadowshock_timer.Reset(2500);
        shadowbuffet_timer.Reset(5000);
    }

    void EnterCombat(Unit *who) { }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (shadowshock_timer.Expired(diff))
        {
            if( Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0, 60, true) )
                DoCast(target,SPELL_SHADOW_SHOCK);

            shadowshock_timer = 5000;
        }
        

        if (shadowbuffet_timer.Expired(diff))
        {
            if( Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0, 60, true) )
                DoCast(target,SPELL_SHADOW_BUFFET);

            shadowbuffet_timer = 5000;
        }


        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_syth_shadow(Creature *_Creature)
{
    return new mob_syth_shadowAI (_Creature);
}

void AddSC_boss_darkweaver_syth()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_darkweaver_syth";
    newscript->GetAI = &GetAI_boss_darkweaver_syth;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_syth_fire";
    newscript->GetAI = &GetAI_mob_syth_fire;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_syth_arcane";
    newscript->GetAI = &GetAI_mob_syth_arcane;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_syth_frost";
    newscript->GetAI = &GetAI_mob_syth_frost;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_syth_shadow";
    newscript->GetAI = &GetAI_mob_syth_shadow;
    newscript->RegisterSelf();
}

