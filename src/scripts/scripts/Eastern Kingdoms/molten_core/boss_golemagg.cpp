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
SDName: Boss_Golemagg
SD%Complete: 90
SDComment:
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "def_molten_core.h"

#define EMOTE_AEGIS     -1409002

enum Golemagg
{
    SPELL_MAGMASPLASH   = 13879,
    SPELL_PYROBLAST     = 20228,
    SPELL_EARTHQUAKE    = 19798,
    SPELL_ENRAGE        = 19953,
    SPELL_TRUST         = 20553
};

enum CoreRager
{
    SPELL_MANGLE        = 19820,
    SPELL_AEGIS         = 20620              //This is self cast whenever we are below 50%
};

struct boss_golemaggAI : public ScriptedAI
{
    boss_golemaggAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }
    ScriptedInstance *pInstance;

    Timer Pyroblast_Timer;
    Timer EarthQuake_Timer;
    Timer Enrage_Timer;
    Timer Buff_Timer;

    void Reset()
    {
        Pyroblast_Timer.Reset(7000);        // These times are probably wrong
        EarthQuake_Timer.Reset(3000);
        Buff_Timer.Reset(2500);
        Enrage_Timer.Reset(60000);          // fixme: is this correct?

        m_creature->CastSpell(m_creature,SPELL_MAGMASPLASH,true);

        if (pInstance && pInstance->GetData(DATA_GOLEMAGG_THE_INCINERATOR_EVENT) != DONE)
            pInstance->SetData(DATA_GOLEMAGG_THE_INCINERATOR_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(DATA_GOLEMAGG_THE_INCINERATOR_EVENT, IN_PROGRESS);
    }

    void JustDied(Unit* Killer)
    {
        if(pInstance)
            pInstance->SetData(DATA_GOLEMAGG_THE_INCINERATOR_EVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Pyroblast_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
                DoCast(target,SPELL_PYROBLAST);

            Pyroblast_Timer = 7000;
        }

        if (HealthBelowPct(11))
        {
            if (Enrage_Timer.Expired(diff))
            {
                DoCast(m_creature,SPELL_ENRAGE);
                Enrage_Timer = 62000;
            }

            if (EarthQuake_Timer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(),SPELL_EARTHQUAKE);
                EarthQuake_Timer = 3000;
            }

            if (Buff_Timer.Expired(diff))
            {
                //DoCast(m_creature, SPELL_TRUST); //fixme: should be cast on adds
                Buff_Timer = 2500;
            }
        }

        DoMeleeAttackIfReady();
    }
};

struct mob_core_ragerAI : public ScriptedAI
{
    mob_core_ragerAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    Timer Mangle_Timer;
    Timer Check_Timer;
    ScriptedInstance *pInstance;

    void Reset()
    {
        Mangle_Timer.Reset(7000);                                //These times are probably wrong
        Check_Timer.Reset(1000);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Mangle_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_MANGLE);
            Mangle_Timer = 10000;
        }

        if (HealthBelowPct(50))
        {
            DoCast(m_creature,SPELL_AEGIS);
            DoScriptText(EMOTE_AEGIS, m_creature);
        }

        if (Check_Timer.Expired(diff))
        {
            if(pInstance)
            {
                Unit *pGolemagg = Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_GOLEMAGG));

                if(!pGolemagg || !pGolemagg->IsAlive())
                    me->DisappearAndDie();
            }

            Check_Timer = 1000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_golemagg(Creature *_Creature)
{
    return new boss_golemaggAI (_Creature);
}

CreatureAI* GetAI_mob_core_rager(Creature *_Creature)
{
    return new mob_core_ragerAI (_Creature);
}

void AddSC_boss_golemagg()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_golemagg";
    newscript->GetAI = &GetAI_boss_golemagg;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_core_rager";
    newscript->GetAI = &GetAI_mob_core_rager;
    newscript->RegisterSelf();
}

