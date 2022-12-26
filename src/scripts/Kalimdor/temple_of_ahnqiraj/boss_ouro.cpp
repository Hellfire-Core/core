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
SDName: Boss_Ouro
SD%Complete: 85
SDComment: No model for submerging. Currently just invisible.
SDCategory: Temple of Ahn'Qiraj
EndScriptData */

#include "scriptPCH.h"
#include "def_temple_of_ahnqiraj.h"

#define SPELL_SWEEP             26103
#define SPELL_SANDBLAST         26102
#define SPELL_GROUND_RUPTURE    26100
#define SPELL_BIRTH             26262                       //The Birth Animation

#define SPELL_DIRTMOUND_PASSIVE 26092

struct boss_ouroAI : public ScriptedAI
{
    boss_ouroAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    Timer Sweep_Timer;
    Timer SandBlast_Timer;
    Timer Submerge_Timer;
    Timer Back_Timer;
    Timer ChangeTarget_Timer;
    Timer Spawn_Timer;

    bool Enrage;
    bool Submerged;
    bool InCombat;

    void Reset()
    {
        Sweep_Timer.Reset(5000 + rand() % 5000);
        SandBlast_Timer.Reset(20000 + rand() % 15000);
        Submerge_Timer.Reset(90000 + rand() % 60000);
        Back_Timer.Reset(30000 + rand() % 15000);
        ChangeTarget_Timer.Reset(5000 + rand() % 3000);
        Spawn_Timer.Reset(10000 + rand() % 10000);

        Enrage = false;
        Submerged = false;

        if (pInstance)
            pInstance->SetData(DATA_OURO, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoCast(m_creature->GetVictim(), SPELL_BIRTH);
        if (pInstance)
            pInstance->SetData(DATA_OURO, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_OURO, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        //Sweep_Timer
        if (!Submerged && Sweep_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_SWEEP);
            Sweep_Timer = 15000 + rand()%15000;
        }

        //SandBlast_Timer
        if (!Submerged && SandBlast_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_SANDBLAST);
            SandBlast_Timer = 20000 + rand()%15000;
        }

        //Submerge_Timer
        if (!Submerged && Submerge_Timer.Expired(diff))
        {
            //Cast
            m_creature->HandleEmoteCommand(EMOTE_ONESHOT_SUBMERGE);
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            m_creature->setFaction(35);
            DoCast(m_creature, SPELL_DIRTMOUND_PASSIVE);

            Submerged = true;
            Back_Timer = 30000 + rand()%15000;
        }

        //ChangeTarget_Timer
        if (Submerged && ChangeTarget_Timer.Expired(diff))
        {
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0);

            if(target)
                DoTeleportTo(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());

            ChangeTarget_Timer = 10000 + rand()%10000;
        }

        //Back_Timer
        if (Submerged && Back_Timer.Expired(diff))
        {
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            m_creature->setFaction(14);

            DoCast(m_creature->GetVictim(), SPELL_GROUND_RUPTURE);

            Submerged = false;
            Submerge_Timer = 60000 + rand()%60000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_ouro(Creature *_Creature)
{
    return new boss_ouroAI (_Creature);
}

void AddSC_boss_ouro()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_ouro";
    newscript->GetAI = &GetAI_boss_ouro;
    newscript->RegisterSelf();
}

