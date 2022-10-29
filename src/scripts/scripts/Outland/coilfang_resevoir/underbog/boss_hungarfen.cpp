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
SDName: Boss_Hungarfen
SD%Complete: 95
SDComment: Need confirmation if spell data are same in both modes. Summons should have faster rate in heroic
SDCategory: Coilfang Resevoir, Underbog
EndScriptData */

#include "precompiled.h"

#define SPELL_FOUL_SPORES   31673
#define SPELL_ACID_GEYSER   38739

struct boss_hungarfenAI : public ScriptedAI
{
    boss_hungarfenAI(Creature *c) : ScriptedAI(c)
    {
    }

    bool Root;
    Timer Mushroom_Timer;
    Timer AcidGeyser_Timer;

    void Reset()
    {
        Root = false;
        Mushroom_Timer.Reset(5000);                              // 1 mushroom after 5s, then one per 10s. This should be different in heroic mode
        AcidGeyser_Timer.Reset(10000);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if( (me->GetHealth()*100) / me->GetMaxHealth() <= 20 )
        {
            if( !Root )
            {
                DoCast(me,SPELL_FOUL_SPORES);
                Root = true;
            }
        }

        if (Mushroom_Timer.Expired(diff))
        {
            if( Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0) )
                me->SummonCreature(17990, target->GetPositionX()+(rand()%8), target->GetPositionY()+(rand()%8), target->GetPositionZ(), (rand()%5), TEMPSUMMON_TIMED_DESPAWN, 31000);
            else
                me->SummonCreature(17990, me->GetPositionX()+(rand()%8), me->GetPositionY()+(rand()%8), me->GetPositionZ(), (rand()%5), TEMPSUMMON_TIMED_DESPAWN, 31000);

            Mushroom_Timer = 10000;
        }

        if (AcidGeyser_Timer.Expired(diff))
        {
            if( Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0) )
                DoCast(target,SPELL_ACID_GEYSER);
            AcidGeyser_Timer = 10000+rand()%7500;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_hungarfen(Creature *_Creature)
{
    return new boss_hungarfenAI (_Creature);
}

#define SPELL_SPORE_CLOUD       34168
#define SPELL_PUTRID_MUSHROOM   31690
#define SPELL_GROW              31698

struct mob_underbog_mushroomAI : public ScriptedAI
{
    mob_underbog_mushroomAI(Creature *c) : ScriptedAI(c) {}

    bool Stop;
    Timer Grow_Timer;
    Timer Shrink_Timer;

    void Reset()
    {
        Stop = false;
        Grow_Timer.Reset(3000);
        Shrink_Timer.Reset(20000);

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

        DoCast(me,SPELL_PUTRID_MUSHROOM,true);
    }

    void MoveInLineOfSight(Unit *who) { return; }

    void AttackStart(Unit* who) { return; }

    void UpdateAI(const uint32 diff)
    {
        if( Stop )
            return;

        if (Grow_Timer.Expired(diff))
        {
            DoCast(me, SPELL_GROW);
            Grow_Timer = 3000;
        }

        if (Shrink_Timer.Expired(diff))
        {
            if (Shrink_Timer.GetInterval() == 20000)
            {
                // shrink
                me->RemoveAurasDueToSpell(SPELL_GROW);
                Shrink_Timer = 1000;
                Grow_Timer = 0;
                m_creature->CastSpell(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), SPELL_SPORE_CLOUD, false);
            }
            else
            {
                // disappear visually
                me->RemoveAurasDueToSpell(SPELL_PUTRID_MUSHROOM);
                Stop = true;
                Shrink_Timer = 0;
            }
        }
    }
};
CreatureAI* GetAI_mob_underbog_mushroom(Creature *_Creature)
{
    return new mob_underbog_mushroomAI (_Creature);
}

void AddSC_boss_hungarfen()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_hungarfen";
    newscript->GetAI = &GetAI_boss_hungarfen;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_underbog_mushroom";
    newscript->GetAI = &GetAI_mob_underbog_mushroom;
    newscript->RegisterSelf();
}

