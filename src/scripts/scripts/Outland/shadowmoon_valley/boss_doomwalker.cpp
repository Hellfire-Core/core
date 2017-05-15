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
SDName: Boss_Doomwalker
SD%Complete: 100
SDComment:
SDCategory: Shadowmoon Valley
EndScriptData */

#include "precompiled.h"

#define SAY_AGGRO                   -1000387
#define SAY_EARTHQUAKE_1            -1000388
#define SAY_EARTHQUAKE_2            -1000389
#define SAY_OVERRUN_1               -1000390
#define SAY_OVERRUN_2               -1000391
#define SAY_SLAY_1                  -1000392
#define SAY_SLAY_2                  -1000393
#define SAY_SLAY_3                  -1000394
#define SAY_DEATH                   -1000395

#define SPELL_EARTHQUAKE            32686
#define SPELL_SUNDER_ARMOR          33661
#define SPELL_CHAIN_LIGHTNING       33665
#define SPELL_OVERRUN               32636
#define SPELL_ENRAGE                33653
#define SPELL_MARK_DEATH            37128

struct boss_doomwalkerAI : public ScriptedAI
{
    boss_doomwalkerAI(Creature *c) : ScriptedAI(c)
    {
        m_creature->GetPosition(wLoc);
    }

    Timer Chain_Timer;
    Timer Enrage_Timer;
    Timer Overrun_Timer;
    Timer Quake_Timer;
    Timer Armor_Timer;
    Timer Check_Timer;

    WorldLocation wLoc;

    bool InEnrage;

    void Reset()
    {
        Enrage_Timer.Reset(1);
        Armor_Timer.Reset(5000 + rand() % 8000);
        Chain_Timer.Reset(10000 + rand() % 20000);
        Quake_Timer.Reset(25000 + rand() % 10000);
        Overrun_Timer.Reset(30000 + rand() % 15000);

        InEnrage = false;
    }

    void KilledUnit(Unit* victim)
    {
        if(!victim->HasAura(SPELL_MARK_DEATH,0))
            m_creature->AddAura(SPELL_MARK_DEATH,victim);

        DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2, SAY_SLAY_3), m_creature);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, m_creature);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);
    }

    void MoveInLineOfSight(Unit* who)
    {
        if (who->HasAura(SPELL_MARK_DEATH, 0) && !who->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
            m_creature->Kill(who, false);

        ScriptedAI::MoveInLineOfSight(who);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(Check_Timer.Expired(diff))
        {
            if(!m_creature->IsWithinDistInMap(&wLoc, 80.0f))
                EnterEvadeMode();

            Check_Timer = 2000;
        }

        //Spell Enrage, when hp <= 20% gain enrage
        if (((m_creature->GetHealth()*100)/ m_creature->GetMaxHealth()) <= 20)
        {
            if(Enrage_Timer.Expired(diff))
            {
                m_creature->RemoveAurasDueToSpell(SPELL_ENRAGE);
                DoCast(m_creature,SPELL_ENRAGE, true);
                Enrage_Timer.Reset(600000);
                InEnrage = true;
            }
        }

        //Spell Overrun
        if (Overrun_Timer.Expired(diff))
        {
            DoScriptText(RAND(SAY_OVERRUN_1, SAY_OVERRUN_2), m_creature);

            DoCast(m_creature->getVictim(),SPELL_OVERRUN);

            DoResetThreat();

            Overrun_Timer = 25000 + rand()%15000;
        }

        //Spell Earthquake
        if (Quake_Timer.Expired(diff))
        {
            if (rand()%2)
                return;

            DoScriptText(RAND(SAY_EARTHQUAKE_1, SAY_EARTHQUAKE_2), m_creature);

            //remove enrage before casting earthquake because enrage + earthquake = 16000dmg over 8sec and all dead
            if (InEnrage)
                m_creature->RemoveAura(SPELL_ENRAGE, 0);

            DoCast(m_creature,SPELL_EARTHQUAKE);
            Enrage_Timer.Reset(8000);
            Quake_Timer = 30000 + rand()%25000;
        }

        //Spell Chain Lightning
        if (Chain_Timer.Expired(diff))
        {
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,1, GetSpellMaxRange(SPELL_CHAIN_LIGHTNING), true, m_creature->getVictimGUID());

            if (!target)
                target = m_creature->getVictim();

            if (target)
                DoCast(target,SPELL_CHAIN_LIGHTNING);

            Chain_Timer = 10000 + rand()%25000;
        }

        //Spell Sunder Armor
        if (Armor_Timer.Expired(diff))
        {
            DoCast(m_creature->getVictim(),SPELL_SUNDER_ARMOR);
            Armor_Timer = 10000 + rand()%15000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_doomwalker(Creature *_Creature)
{
    return new boss_doomwalkerAI (_Creature);
}

void AddSC_boss_doomwalker()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_doomwalker";
    newscript->GetAI = &GetAI_boss_doomwalker;
    newscript->RegisterSelf();
}
