/* 
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

#include "scriptPCH.h"

#define SPELL_HAMSTRING     9080
#define SPELL_HEAD_CRACK    16172
#define YELL_AGGRO          -678
#define YELL_OOC1           -679
#define YELL_OOC2           -680
#define YELL_OOC3           -681
#define YELL_OOC4           -683
#define YELL_OOC5           -684
#define YELL_OOC6           -685


struct mob_coilfang_slavehandlerAI : public ScriptedAI
{
    mob_coilfang_slavehandlerAI(Creature *c) : ScriptedAI(c) {}

    Timer hamstringtimer;
    Timer headcracktimer;
    Timer yelltimer;

    void Reset()
    {
        hamstringtimer.Reset(urand(5800, 6200));
        headcracktimer.Reset(11100);
        yelltimer.Reset(urand(60000, 120000));
    }

    void JustDied(Unit* )
    {
        StartRun();
    }

    void StartRun()
    {
        std::list<Creature*> alist= FindAllCreaturesWithEntry(17963, 100);
        std::list<Creature*> blist= FindAllCreaturesWithEntry(17964, 100);
        for(std::list<Creature*>::iterator itr = blist.begin(); itr != blist.end(); itr++)
            alist.push_front(*itr);
        for(std::list<Creature*>::iterator itr = alist.begin(); itr != alist.end(); itr++)
        {
            if ((*itr)->GetFormation() == me->GetFormation())
            {
                if ((*itr)->IsAlive())
                {
                    (*itr)->RemoveAllAuras();
                    (*itr)->DeleteThreatList();
                    (*itr)->CombatStop();

                    if (me->GetPositionY() > -150.0f)
                        (*itr)->GetMotionMaster()->MovePoint(1, 115.0f, -98.0f, -1.5f);
                    else
                        (*itr)->GetMotionMaster()->MovePoint(2, -102.0f, -121.0f, -2.1f);

                    (*itr)->ForcedDespawn(7000);
                } 
            }
        }
    }

    void EnterCombat(Unit* attacker)
    {
        me->Yell(YELL_AGGRO,0,0);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!me->IsInCombat())
            if (yelltimer.Expired(diff))
            {
                me->Yell(RAND(YELL_OOC1,YELL_OOC2,YELL_OOC3,YELL_OOC4,YELL_OOC5,YELL_OOC6),0,0);
                yelltimer = urand(60000,120000);
            }


        if(!UpdateVictim())
            return;

        if (hamstringtimer.Expired(diff))
        {
            AddSpellToCast(SPELL_HAMSTRING);
            hamstringtimer = 9000;
        }
        

        if (headcracktimer.Expired(diff))
        {
            AddSpellToCast(SPELL_HEAD_CRACK,CAST_RANDOM);
            headcracktimer = urand(20000,25000);
        }
        

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady(diff);
    }
};

CreatureAI* GetAI_mob_coilfang_slavehandler(Creature *_creature)
{
    return new mob_coilfang_slavehandlerAI (_creature);
}

// SV
#define SPELL_DISARM        6713
#define SPELL_GEYSER        10987
#define SPELL_FRENZY        8269

struct npc_coilfang_slavemasterAI : public ScriptedAI
{
    npc_coilfang_slavemasterAI(Creature *c) : ScriptedAI(c) {}

    Timer hamstringtimer;
    Timer headcracktimer;
    Timer yelltimer;

    bool frenzy;

    void Reset()
    {
        hamstringtimer.Reset(urand(5800, 6200));
        headcracktimer.Reset(11100);
        yelltimer.Reset(urand(60000, 120000));
        frenzy = false;
    }

    void JustDied(Unit* )
    {
        StartRun();
    }

    void StartRun()
    {
        std::list<Creature*> alist= FindAllCreaturesWithEntry(17799, 100);
        for(std::list<Creature*>::iterator itr = alist.begin(); itr != alist.end(); itr++)
        {
            if ((*itr)->GetFormation() == me->GetFormation())
            {
                if ((*itr)->IsAlive())
                {
                    (*itr)->RemoveAllAuras();
                    (*itr)->DeleteThreatList();
                    (*itr)->CombatStop();
                    (*itr)->GetMotionMaster()->MovePoint(1, -149.0f, -295.0f, -7.6f);
                    (*itr)->ForcedDespawn(7000);
                } 
            }
        }
    }

    void EnterCombat(Unit* attacker)
    {
        me->Yell(YELL_AGGRO,0,0);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!me->IsInCombat())
            if (yelltimer.Expired(diff))
            {
                me->Yell(RAND(YELL_OOC1,YELL_OOC2,YELL_OOC3,YELL_OOC4,YELL_OOC5,YELL_OOC6),0,0);
                yelltimer = urand(60000,120000);
            }


        if(!UpdateVictim())
            return;

        if (hamstringtimer.Expired(diff))
        {
            AddSpellToCast(SPELL_DISARM);
            hamstringtimer = 9000;
        }
        

        if (headcracktimer.Expired(diff))
        {
            AddSpellToCast(SPELL_GEYSER);
            headcracktimer = urand(20000,25000);
        }
        

        if (HealthBelowPct(20.0f) && !frenzy)
        {
            ForceSpellCast(me, SPELL_FRENZY, INTERRUPT_AND_CAST_INSTANTLY);
            frenzy = true;
        }

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady(diff);
    }
};

CreatureAI* GetAI_npc_coilfang_slavemaster(Creature *_creature)
{
    return new npc_coilfang_slavemasterAI (_creature);
}


void AddSC_slave_pens_trash()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="mob_coilfang_slavehandler";
    newscript->GetAI = &GetAI_mob_coilfang_slavehandler;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_coilfang_slavemaster";
    newscript->GetAI = &GetAI_npc_coilfang_slavemaster;
    newscript->RegisterSelf();
}
