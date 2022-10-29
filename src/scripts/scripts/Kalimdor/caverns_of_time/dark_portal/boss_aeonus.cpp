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
SDName: Boss_Aeonus
SD%Complete: 99
SDComment: Some timers may not be proper
SDCategory: Caverns of Time, The Dark Portal
EndScriptData */

#include "precompiled.h"
#include "def_dark_portal.h"

#define SAY_ENTER         -1269012
#define SAY_AGGRO         -1269013
#define SAY_BANISH        -1269014
#define SAY_SLAY1         -1269015
#define SAY_SLAY2         -1269016
#define SAY_DEATH         -1269017
#define EMOTE_FRENZY      -1269018

#define SPELL_CLEAVE        40504
#define SPELL_TIME_STOP     31422
#define SPELL_ENRAGE        37605
#define SPELL_SAND_BREATH   31473
#define H_SPELL_SAND_BREATH 39049

struct boss_aeonusAI : public ScriptedAI
{
    boss_aeonusAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    Timer Say_Timer;
    Timer Cleave_Timer;
    Timer SandBreath_Timer;
    Timer TimeStop_Timer;
    Timer Frenzy_Timer;

    void Reset()
    {
        Say_Timer.Reset(20000);
        Cleave_Timer.Reset(5000);
        SandBreath_Timer.Reset(30000);
        TimeStop_Timer.Reset(40000);
        Frenzy_Timer.Reset(120000);
        m_creature->setActive(true);

        SayIntro();
    }

    void SayIntro()
    {
        DoScriptText(SAY_ENTER, m_creature);
    }

    void EnterCombat(Unit *who) {}

    void MoveInLineOfSight(Unit *who)
    {
        //Despawn Time Keeper
        if (who->GetTypeId() == TYPEID_UNIT && who->GetEntry() == C_TIME_KEEPER)
        {
            if (me->IsAlive() && m_creature->IsWithinDistInMap(who,20.0f))
            {
                DoScriptText(SAY_BANISH, m_creature);
                who->ToCreature()->ForcedDespawn();
            }
        }

        ScriptedAI::MoveInLineOfSight(who);
    }

    void JustDied(Unit *victim)
    {
        if (pInstance->GetData(TYPE_MEDIVH) != FAIL)
            DoScriptText(SAY_DEATH, m_creature);

        pInstance->SetData(TYPE_RIFT, DONE);
        pInstance->SetData(TYPE_MEDIVH, DONE);//FIXME: later should be removed
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2), m_creature);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        
        if (Say_Timer.Expired(diff))
        {
            DoScriptText(SAY_AGGRO, m_creature);
            Say_Timer = 0;
        }
        

        if (Cleave_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_CLEAVE);
            Cleave_Timer = 6000+rand()%4000;
        }
        


        if (SandBreath_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), HeroicMode ? H_SPELL_SAND_BREATH : SPELL_SAND_BREATH);
            SandBreath_Timer = 30000;
        }
        


        if (TimeStop_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_TIME_STOP);
            TimeStop_Timer = 40000;
        }
        


        if (Frenzy_Timer.Expired(diff))
        {
            AddSpellToCastWithScriptText(m_creature, SPELL_ENRAGE, EMOTE_FRENZY);
            Frenzy_Timer = 120000;
        }
        

        //if event failed, remove boss from instance
        if (pInstance->GetData(TYPE_MEDIVH) == FAIL)
        {
            m_creature->Kill(m_creature, false);
            m_creature->RemoveCorpse();
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_aeonus(Creature *_Creature)
{
    return new boss_aeonusAI (_Creature);
}

void AddSC_boss_aeonus()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_aeonus";
    newscript->GetAI = &GetAI_boss_aeonus;
    newscript->RegisterSelf();
}

