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
SDName: Boss_Mandokir
SD%Complete: 90
SDComment: Implement ghosts that offer resurrection. Ohgan function needs improvements (?).
SDCategory: Zul'Gurub
EndScriptData */

#include "precompiled.h"
#include "def_zulgurub.h"

struct boss_mandokirAI : public ScriptedAI
{
    boss_mandokirAI(Creature *c) : ScriptedAI(c), KillCount(0), WatchedTarget(0)
    {
        pInstance = c->GetInstanceData();
    }

    enum Events
    {
        EVENT_CAST_CHARGE,
        EVENT_CAST_CLEAVE,
        EVENT_CAST_FEAR,
        EVENT_CAST_WHIRLWIND,
        EVENT_CAST_MORTAL_STRIKE,
        EVENT_WATCH_BEGIN,              // every 30s notify player and begin casting gaze debuff
        EVENT_SAVE_POSITION,            // after 2s save player's position
        EVENT_WATCH_END,                // after 8s check whether player moved and if so, deal damage
        EVENT_ENRAGE
    };

    enum Spells
    {
        SPELL_CHARGE            = 24315,
        SPELL_GUILLOTINE        = 24316,
        SPELL_CLEAVE            = 20691,
        SPELL_FEAR              = 29321,
        SPELL_WHIRLWIND         = 24236,
        SPELL_MORTAL_STRIKE     = 24573,
        SPELL_ENRAGE            = 23537,
        SPELL_WATCH             = 24314,
        SPELL_LEVEL_UP          = 24312,
        SPELL_SWIFT_RAPTOR      = 23243
    };

    enum Texts
    {
        SAY_AGGRO               = -1309015,
        SAY_DING_KILL           = -1309016,
        SAY_GRATS_JINDO         = -1309017,
        SAY_WATCH               = -1309018
    };

    ScriptedInstance *pInstance;
    EventMap events;
    uint32 KillCount;
    uint64 WatchedTarget;   // watched player GUID
    Position tPos;          // watched target position

    void Reset()
    {
        KillCount = 0;
        WatchedTarget = 0;

        DoCast(m_creature, SPELL_SWIFT_RAPTOR);
        pInstance->SetData(DATA_MANDOKIREVENT, NOT_STARTED);

        events.Reset()
              .ScheduleEvent(EVENT_CAST_MORTAL_STRIKE, 1000)
              .ScheduleEvent(EVENT_CAST_FEAR, 2000)
              .ScheduleEvent(EVENT_CAST_CLEAVE, 7000)
              .ScheduleEvent(EVENT_CAST_WHIRLWIND, 20000)
              .ScheduleEvent(EVENT_WATCH_BEGIN, 30000);
    }

    void KilledUnit(Unit* victim)
    {
        if (victim->GetTypeId() == TYPEID_PLAYER)
        {
            if (++KillCount >= 3)
            {
                DoScriptText(SAY_DING_KILL, me);

                if (pInstance)
                    if (Unit* Jindo = Unit::GetUnit(*me, pInstance->GetData64(DATA_JINDO)))
                        if (Jindo->IsAlive())
                            DoScriptText(SAY_GRATS_JINDO, Jindo);

                DoCast(me, SPELL_LEVEL_UP, true);
                KillCount = 0;
            }
        }
    }

    void EnterCombat(Unit *who)
    {
        if (me->HasAuraType(SPELL_AURA_MOUNTED))
            me->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

        if (Creature* Ohgan = me->SummonCreature(14988, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 35000))
            Ohgan->AI()->AttackStart(who);

        DoScriptText(SAY_AGGRO, m_creature);
        pInstance->SetData(DATA_MANDOKIREVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        pInstance->SetData(DATA_MANDOKIREVENT, DONE);
    }

    void DoAction(const int32 param)
    {
        if (param == EVENT_ENRAGE)
            DoCast(me, SPELL_ENRAGE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CAST_CLEAVE:
                {
                    DoCast(me->GetVictim(), SPELL_CLEAVE);
                    events.ScheduleEvent(EVENT_CAST_CLEAVE, 7000);
                    break;
                }
                case EVENT_CAST_WHIRLWIND:
                {
                    DoCast(me, SPELL_WHIRLWIND);
                    events.ScheduleEvent(EVENT_CAST_WHIRLWIND, 18000);
                    break;
                }
                case EVENT_CAST_FEAR:
                {
                    if (FindAllPlayersInRange(NOMINAL_MELEE_RANGE).size() >= 2)
                        DoCast(me->GetVictim(), SPELL_FEAR);

                    events.ScheduleEvent(EVENT_CAST_FEAR, 4000);
                    break;
                }
                case EVENT_CAST_MORTAL_STRIKE:
                {
                    if (me->GetVictim() && me->GetVictim()->HealthBelowPct(50))
                    {
                        DoCast(m_creature->GetVictim(), SPELL_MORTAL_STRIKE);
                        events.ScheduleEvent(EVENT_CAST_MORTAL_STRIKE, 15000);
                    }
                    else
                        events.ScheduleEvent(EVENT_CAST_MORTAL_STRIKE, 3000);
                    break;
                }
                case EVENT_WATCH_BEGIN:
                {
                    if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_WATCH), true))
                    {
                        WatchedTarget = target->GetGUID();
                        DoScriptText(SAY_WATCH, me, target);
                        DoCast(target, SPELL_WATCH);

                        events.ScheduleEvent(EVENT_SAVE_POSITION, 2000)
                              .ScheduleEvent(EVENT_WATCH_END, 8000)
                              .CancelEvent(EVENT_CAST_CLEAVE)
                              .CancelEvent(EVENT_CAST_WHIRLWIND)
                              .CancelEvent(EVENT_CAST_FEAR)
                              .CancelEvent(EVENT_CAST_MORTAL_STRIKE);
                    }
                    else
                    {
                        WatchedTarget = 0;
                        events.ScheduleEvent(EVENT_WATCH_BEGIN, 3000);
                    }
                    break;
                }
                case EVENT_SAVE_POSITION:
                {
                    if (Unit* target = me->GetUnit(WatchedTarget))
                        target->GetPosition(tPos);
                    break;
                }
                case EVENT_WATCH_END:
                {
                    if (Unit* target = me->GetUnit(WatchedTarget))
                    {
                        Position currentPos;
                        target->GetPosition(currentPos);

                        if (tPos != currentPos && target->IsAlive() && target->IsInCombat())
                        {
                            if (me->IsWithinMeleeRange(target))
                            {
                                DoCast(target, SPELL_GUILLOTINE);
                            }
                            else
                            {
                                DoCast(target, SPELL_CHARGE);
                                AttackStart(target);
                            }
                        }
                    }

                    events.ScheduleEvent(EVENT_WATCH_BEGIN, 22000)
                          .ScheduleEvent(EVENT_CAST_CLEAVE, 3000)
                          .ScheduleEvent(EVENT_CAST_FEAR, 6000)
                          .ScheduleEvent(EVENT_CAST_MORTAL_STRIKE, 14000)
                          .ScheduleEvent(EVENT_CAST_WHIRLWIND, 18000);
                    break;
                }
            }
        }

        DoMeleeAttackIfReady();
    }
};

#define SPELL_SUNDERARMOR       24317

struct mob_ohganAI : public ScriptedAI
{
    mob_ohganAI(Creature *c) : ScriptedAI(c) {}

    Timer SunderArmor_Timer;

    void Reset()
    {
        SunderArmor_Timer.Reset(5000);
    }

    void EnterCombat(Unit *who) {}

    void JustDied(Unit* Killer)
    {
        if (!me->GetInstanceData())
            return;

        if (uint64 MandokirGUID = me->GetInstanceData()->GetData64(DATA_MANDOKIR))
            if (Unit* Mandokir = me->GetUnit(MandokirGUID))
                if (Mandokir->IsAlive() && Mandokir->IsInCombat())
                    dynamic_cast<boss_mandokirAI*>(Mandokir->ToCreature()->AI())->DoAction(boss_mandokirAI::EVENT_ENRAGE);
    }

    void UpdateAI (const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (SunderArmor_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_SUNDERARMOR);
            SunderArmor_Timer = 10000 + rand()%5000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_mandokir(Creature *_Creature)
{
    return new boss_mandokirAI (_Creature);
}

CreatureAI* GetAI_mob_ohgan(Creature *_Creature)
{
    return new mob_ohganAI (_Creature);
}

void AddSC_boss_mandokir()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_mandokir";
    newscript->GetAI = &GetAI_boss_mandokir;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_ohgan";
    newscript->GetAI = &GetAI_mob_ohgan;
    newscript->RegisterSelf();
}

