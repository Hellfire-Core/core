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
SDName: Boss_Maiden_of_Virtue
SD%Complete: 100
SDComment:
SDCategory: Karazhan
EndScriptData */

#include "scriptPCH.h"
#include "def_karazhan.h"

#define SAY_AGGRO               -1532018
#define SAY_SLAY1               -1532019
#define SAY_SLAY2               -1532020
#define SAY_SLAY3               -1532021
#define SAY_REPENTANCE1         -1532022
#define SAY_REPENTANCE2         -1532023
#define SAY_DEATH               -1532024

#define SPELL_REPENTANCE        29511
#define SPELL_HOLYFIRE          29522
#define SPELL_HOLYWRATH         32445
#define SPELL_HOLYGROUND        29512
#define SPELL_BERSERK           26662

struct boss_maiden_of_virtueAI : public ScriptedAI
{
    boss_maiden_of_virtueAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        m_creature->GetPosition(wLoc);
    }

    ScriptedInstance *pInstance;

    Timer Repentance_Timer;
    Timer Holyfire_Timer;
    Timer Holywrath_Timer;
    Timer Holyground_Timer;
    Timer Enrage_Timer;
    Timer CheckTimer;

    WorldLocation wLoc;

    bool Enraged;

    void Reset()
    {
        Repentance_Timer.Reset(30000 + (rand() % 15000));
        Holyfire_Timer.Reset(8000 + (rand() % 17000));
        Holywrath_Timer.Reset(20000 + (rand() % 10000));
        Holyground_Timer.Reset(3000);
        Enrage_Timer.Reset(600000);
        CheckTimer.Reset(3000);

        if(pInstance && pInstance->GetData(DATA_MAIDENOFVIRTUE_EVENT) != DONE)
            pInstance->SetData(DATA_MAIDENOFVIRTUE_EVENT, NOT_STARTED);

        Enraged = false;
    }

    void KilledUnit(Unit* Victim)
    {
        if(rand()%2)
            return;

        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2, SAY_SLAY3), m_creature);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (pInstance)
            pInstance->SetData(DATA_MAIDENOFVIRTUE_EVENT, DONE);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);
        if (pInstance)
            pInstance->SetData(DATA_MAIDENOFVIRTUE_EVENT, IN_PROGRESS);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim() )
            return;

        
        if (CheckTimer.Expired(diff))
        {
            if(!m_creature->IsWithinDistInMap(&wLoc, 30.0f))
                EnterEvadeMode();
            else
                DoZoneInCombat();

            CheckTimer = 3000;
        }
        

        if (!Enraged && Enrage_Timer.Expired(diff))
        {
            DoCast(m_creature, SPELL_BERSERK,true);
            Enraged = true;
        }
        
        if (Holyground_Timer.Expired(diff))
        {
            DoCast(m_creature, SPELL_HOLYGROUND, true);     //Triggered so it doesn't interrupt her at all
            Holyground_Timer = 3000;
        }
        
        
        if (Repentance_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_REPENTANCE);

            DoScriptText(RAND(SAY_REPENTANCE1, SAY_REPENTANCE2), m_creature);

            Repentance_Timer = 30000 + rand()%15000;        //A little randomness on that spell
            Holyfire_Timer = 6000;
        }
        
          

        if (Holyfire_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0,GetSpellMaxRange(SPELL_HOLYFIRE), true))
                DoCast(target,SPELL_HOLYFIRE);

                Holyfire_Timer = 8000 + rand()%17000; //Anywhere from 8 to 25 seconds, good luck having several of those in a row!
        }
        
           

        if (Holywrath_Timer.Expired(diff))
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0,GetSpellMaxRange(SPELL_HOLYWRATH), true))
                DoCast(target,SPELL_HOLYWRATH);

            Holywrath_Timer = 20000+(rand()%10000);     //20-30 secs sounds nice

        }
        

        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_boss_maiden_of_virtue(Creature *_Creature)
{
    return new boss_maiden_of_virtueAI (_Creature);
}

void AddSC_boss_maiden_of_virtue()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_maiden_of_virtue";
    newscript->GetAI = &GetAI_boss_maiden_of_virtue;
    newscript->RegisterSelf();
}

