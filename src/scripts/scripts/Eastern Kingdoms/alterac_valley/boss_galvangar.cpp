/* 
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Boss_Galvangar
SD%Complete:
SDComment: timers should be adjusted
EndScriptData */

#include "precompiled.h"

#define YELL_AGGRO                 -1001021
#define YELL_EVADE                 -1001022

#define SPELL_CLEAVE               15284
#define SPELL_FRIGHTENING_SHOUT    19134
#define SPELL_WHIRLWIND            13736
#define SPELL_MORTAL_STRIKE        16856


struct boss_galvangarAI : public ScriptedAI
{
    boss_galvangarAI(Creature *c) : ScriptedAI(c)
    {
        m_creature->GetPosition(wLoc);
    }


    Timer CleaveTimer;
    Timer FrighteningShoutTimer;
    Timer WhirlwindTimer;
    Timer MortalStrikeTimer;
    Timer CheckTimer;
    WorldLocation wLoc;


    void Reset()
    {
        CleaveTimer.Reset(4000);
        FrighteningShoutTimer.Reset(10000);
        WhirlwindTimer.Reset(5000);
        MortalStrikeTimer.Reset(2000);
        CheckTimer.Reset(2000);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(YELL_AGGRO, m_creature);
    }

    void JustRespawned()
    {
        Reset();
    }

    void KilledUnit(Unit* victim){}

    void JustDied(Unit* Killer){}

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (CheckTimer.Expired(diff))
        {
            if (!m_creature->IsWithinDistInMap(&wLoc, 20.0f))
            {
                EnterEvadeMode();
                return;
            }
            CheckTimer = 2000;
        }
        

        if (CleaveTimer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_CLEAVE);
            CleaveTimer =  urand(4000, 12000);
        }

        if (FrighteningShoutTimer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_FRIGHTENING_SHOUT);
            FrighteningShoutTimer = urand(14000, 24000);
        }

        if (MortalStrikeTimer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_MORTAL_STRIKE);
            MortalStrikeTimer = 6000;
        }

        if (WhirlwindTimer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_WHIRLWIND);
            WhirlwindTimer = 10000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_galvangar(Creature *_Creature)
{
    return new boss_galvangarAI (_Creature);
}

void AddSC_boss_galvangar()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_galvangar";
    newscript->GetAI = &GetAI_boss_galvangar;
    newscript->RegisterSelf();
}
