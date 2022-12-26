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
SDName: Boss_Warlord_Kalithres
SD%Complete: 95
SDComment:
SDCategory: Coilfang Resevoir, The Steamvault
EndScriptData */

#include "scriptPCH.h"
#include "def_steam_vault.h"

#define SAY_INTRO                   -1545016
#define SAY_REGEN                   -1545017
#define SAY_AGGRO1                  -1545018
#define SAY_AGGRO2                  -1545019
#define SAY_AGGRO3                  -1545020
#define SAY_SLAY1                   -1545021
#define SAY_SLAY2                   -1545022
#define SAY_DEATH                   -1545023

#define SPELL_SPELL_REFLECTION      31534
#define SPELL_IMPALE                39061
//#define SPELL_WARLORDS_RAGE         37081
#define SPELL_WARLORDS_RAGE_NAGA    31543
#define SPELL_WARLORDS_RAGE_PROC    36453

struct boss_warlord_kalithreshAI : public ScriptedAI
{
    boss_warlord_kalithreshAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        c->GetPosition(pos);
    }

    ScriptedInstance *pInstance;

    Timer Reflection_Timer;
    Timer Impale_Timer;
    Timer Rage_Timer;
    uint64 CurrentDistiller;
    Timer checkTimer;

    WorldLocation pos;

    void Reset()
    {
        ClearCastQueue();

        Reflection_Timer.Reset(10000);
        Impale_Timer.Reset(urand(7000,14000));
        Rage_Timer.Reset(urand(15000, 25000));
        CurrentDistiller = 0;
        checkTimer.Reset(3000);

        std::list<Creature*> naga_distillers = FindAllCreaturesWithEntry(17954, 100);
        for(std::list<Creature*>::iterator it = naga_distillers.begin(); it != naga_distillers.end(); it++)
        {
            (*it)->Respawn();
            (*it)->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            (*it)->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
            (*it)->GetUnitStateMgr().PushAction(UNIT_ACTION_STUN);
        }

        if (pInstance)
            pInstance->SetData(TYPE_WARLORD_KALITHRESH, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(RAND(SAY_AGGRO1, SAY_AGGRO2, SAY_AGGRO3), me);
        me->SetHealth(me->GetMaxHealth());
        if (pInstance)
            pInstance->SetData(TYPE_WARLORD_KALITHRESH, IN_PROGRESS);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2), me);
    }

    void OnAuraRemove(Aura* aur, bool)
    {
        if (aur->GetId() == SPELL_WARLORDS_RAGE_NAGA)
        {
            if (CurrentDistiller)
            {
                if (Unit* distiller = me->GetUnit(CurrentDistiller))
                {
                    if (distiller && distiller->IsAlive())
                    {
                        distiller->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        distiller->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
                        distiller->SetHealth(distiller->GetMaxHealth());
                        CurrentDistiller = 0;
                        DoCast(me, SPELL_WARLORDS_RAGE_PROC, true);
                    }
                    else return;
                }
            }
        }

        //if (aur->GetId() == SPELL_WARLORDS_RAGE)
            // return;
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type != POINT_MOTION_TYPE || !id)
            return;

        if (CurrentDistiller)
        {
            if (Unit* distiller = me->GetUnit(CurrentDistiller))
            {
                me->SetFacingToObject(distiller);
                DoScriptText(SAY_REGEN, me);
                distiller->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                distiller->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
                distiller->CastSpell(me, SPELL_WARLORDS_RAGE_NAGA, true);
            }
        }
    }

    void JustDied(Unit* Killer)
    {
        // suicide all distillers cause combat stop
        std::list<Creature*> naga_distillers = FindAllCreaturesWithEntry(17954, 100);
        for(std::list<Creature*>::iterator it = naga_distillers.begin(); it != naga_distillers.end(); it++)
        {
            (*it)->Kill(*it);
        }

        DoScriptText(SAY_DEATH, me);
        if (pInstance)
            pInstance->SetData(TYPE_WARLORD_KALITHRESH, DONE);
    }

    void DamageTaken(Unit* done_by, uint32& damage)
    {
        if (!done_by->IsWithinDistInMap(&pos, 105.0f))
            damage = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim() )
            return;

        if (checkTimer.Expired(diff))
        {
            if (!me->IsWithinDistInMap(&pos, 105.0f))
            {
                EnterEvadeMode();
                return;
            }
            checkTimer = 3000;
        }
        

        if (Rage_Timer.Expired(diff))
        {
            Creature* distiller = GetClosestCreatureWithEntry(me, 17954, 100);

            if (distiller)
            {
                float x, y, z;
                distiller->GetNearPoint(x, y, z, INTERACTION_DISTANCE);
                me->GetMotionMaster()->MovePoint(1, x, y, z);
                CurrentDistiller = distiller->GetGUID();
            }

            Rage_Timer = urand(15000,25000);
        }

        //Reflection_Timer
        if (Reflection_Timer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_SPELL_REFLECTION);
            Reflection_Timer = urand(15000,25000);
        }

        //Impale_Timer
        if (Impale_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
                AddSpellToCast(target,SPELL_IMPALE);

            Impale_Timer = urand(7500, 12500);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_warlord_kalithresh(Creature *_Creature)
{
    return new boss_warlord_kalithreshAI (_Creature);
}

void AddSC_boss_warlord_kalithresh()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_warlord_kalithresh";
    newscript->GetAI = &GetAI_boss_warlord_kalithresh;
    newscript->RegisterSelf();
}

