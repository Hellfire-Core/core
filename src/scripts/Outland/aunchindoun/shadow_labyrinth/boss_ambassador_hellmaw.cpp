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
SDName: Boss_Ambassador_Hellmaw
SD%Complete: 99
SDComment: It appears that it's done now.
SDCategory: Auchindoun, Shadow Labyrinth
EndScriptData */

#include "scriptPCH.h"
#include "def_shadow_labyrinth.h"

#define SAY_INTRO       -1555000

#define SAY_AGGRO1      -1555001
#define SAY_AGGRO2      -1555002
#define SAY_AGGRO3      -1555003

#define SAY_HELP        -1555004

#define SAY_SLAY1       -1555005
#define SAY_SLAY2       -1555006

#define SAY_DEATH       -1555007

#define SPELL_BANISH            42354
#define SPELL_CORROSIVE_ACID    33551
#define SPELL_FEAR              33547
#define SPELL_ENRAGE            26662

#define PATH_PATROL              2100
#define PATH_FINAL               2101

struct boss_ambassador_hellmawAI : public ScriptedAI
{
    boss_ambassador_hellmawAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    Timer EventCheck_Timer;
    Timer Banish_Timer;
    Timer CorrosiveAcid_Timer;
    Timer Fear_Timer;
    Timer Enrage_Timer;
    Timer OnPath_Delay;
    bool Intro;
    bool IsBanished;
    bool patrol;

    void Reset()
    {
        EventCheck_Timer.Reset(5000);
        Banish_Timer = 1;
        CorrosiveAcid_Timer.Reset(25000);
        Fear_Timer.Reset(40000);
        Enrage_Timer.Reset(180000);
        Intro = false;
        IsBanished = false;

        if (pInstance)
        {
            if (pInstance->GetData(TYPE_HELLMAW) == NOT_STARTED)
            {
                DoCast(me, SPELL_BANISH, true);
                IsBanished = true;
            }
            else
                pInstance->SetData(TYPE_HELLMAW, SPECIAL); // not in_progress but also not not_started
        }
    }

    void DoIntro()
    {
        DoScriptText(SAY_INTRO, me);

        if (me->HasAura(SPELL_BANISH,0))
            me->RemoveAurasDueToSpell(SPELL_BANISH);

        IsBanished = false;
        Intro = true;
    }

    void EnterCombat(Unit *who)
    {
        if(IsBanished)
        {
            EnterEvadeMode();
            return;
        }

        me->GetMotionMaster()->Clear();
        DoScriptText(RAND(SAY_AGGRO1, SAY_AGGRO2, SAY_AGGRO3), me);
        if (pInstance)
            pInstance->SetData(TYPE_HELLMAW, IN_PROGRESS);
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2), me);
    }

    void JustDied(Unit *victim)
    {
        DoScriptText(SAY_DEATH, me);

        if (pInstance)
            pInstance->SetData(TYPE_HELLMAW, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!pInstance)
            return;

        if(IsBanished)
        {
            if (Banish_Timer.Expired(diff))
             {
                 DoCast(me,SPELL_BANISH, true);
                 Banish_Timer = 40000;
             }
             
        }

        if (!Intro)
        {
            if (EventCheck_Timer.Expired(diff))
            {
                if(pInstance->GetData(TYPE_RITUALIST) == DONE)
                {
                    OnPath_Delay = 0;
                    DoIntro();
                }
                EventCheck_Timer = 5000;
            }
        }

        if (!me->IsInCombat() && !IsBanished && !OnPath_Delay.GetInterval())
        {
            me->GetMotionMaster()->MovePath(PATH_PATROL, false);
            OnPath_Delay = 55000;
            patrol = false;
        }

        if (!me->IsInCombat() && !patrol && OnPath_Delay.Expired(diff))
        {
            me->GetMotionMaster()->MovePath(PATH_FINAL, true);
            patrol = true;
        }
        

        if (!UpdateVictim() )
            return;

        if (CorrosiveAcid_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_CORROSIVE_ACID);
            CorrosiveAcid_Timer = 25000;
        }
        

        if (Fear_Timer.Expired(diff))
        {
            DoCast(me,SPELL_FEAR);
            Fear_Timer = 25000;
        }


        if (HeroicMode)
        {
            if (Enrage_Timer.Expired(diff))
            {
                DoCast(me,SPELL_ENRAGE);
                Enrage_Timer = 5*MINUTE*1000;
            }
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_ambassador_hellmaw(Creature *_Creature)
{
    return new boss_ambassador_hellmawAI (_Creature);
}

void AddSC_boss_ambassador_hellmaw()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_ambassador_hellmaw";
    newscript->GetAI = &GetAI_boss_ambassador_hellmaw;
    newscript->RegisterSelf();
}

