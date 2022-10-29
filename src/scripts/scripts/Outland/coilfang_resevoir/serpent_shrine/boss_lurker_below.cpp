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

#include "precompiled.h"
#include "def_serpent_shrine.h"
#include "../../../special/simple_ai.h"
#include "Spell.h"

#define EMOTE_SPOUT "takes a deep breath."

enum lurkerAdds
{
    MOB_COILFANG_GUARDIAN = 21873,
    MOB_COILFANG_AMBUSHER = 21865
};

static float addPos[9][4] =
{
    { MOB_COILFANG_AMBUSHER, 2.8553810f, -459.823914f, -19.182686f},   //MOVE_AMBUSHER_1 X, Y, Z
    { MOB_COILFANG_AMBUSHER, 12.400000f, -466.042267f, -19.182686f},   //MOVE_AMBUSHER_2 X, Y, Z
    { MOB_COILFANG_AMBUSHER, 51.366653f, -460.836060f, -19.182686f},   //MOVE_AMBUSHER_3 X, Y, Z
    { MOB_COILFANG_AMBUSHER, 62.597980f, -457.433044f, -19.182686f},   //MOVE_AMBUSHER_4 X, Y, Z
    { MOB_COILFANG_AMBUSHER, 77.607452f, -384.302765f, -19.182686f},   //MOVE_AMBUSHER_5 X, Y, Z
    { MOB_COILFANG_AMBUSHER, 63.897900f, -378.984924f, -19.182686f},   //MOVE_AMBUSHER_6 X, Y, Z
    { MOB_COILFANG_GUARDIAN, 34.447250f, -387.333618f, -19.182686f},   //MOVE_GUARDIAN_1 X, Y, Z
    { MOB_COILFANG_GUARDIAN, 14.388216f, -423.468018f, -19.625271f},   //MOVE_GUARDIAN_2 X, Y, Z
    { MOB_COILFANG_GUARDIAN, 42.471519f, -445.115295f, -19.769423f}    //MOVE_GUARDIAN_3 X, Y, Z
};

enum lurkerSpells
{
    SPELL_SPOUT_VISUAL       = 37429,
    SPELL_SPOUT_BREATH       = 37431,
    SPELL_SPOUT_EFFECT       = 37433,
    SPELL_GEYSER             = 37478,
    SPELL_WHIRL              = 37660,
    SPELL_WATERBOLT          = 37138,
    SPELL_SUBMERGE           = 37550,
    SPELL_EMERGE             = 20568 // wrong spellid

};

#define SPOUT_WIDTH 1.2f

enum LurkerEvents
{
    LURKER_EVENT_SPOUT_EMOTE    = 1,
    LURKER_EVENT_SPOUT          = 2,
    LURKER_EVENT_STOP_SPOUT     = 3,
    LURKER_EVENT_WHIRL          = 4,
    LURKER_EVENT_GEYSER         = 5,
    LURKER_EVENT_SUBMERGE       = 6,
    LURKER_EVENT_REEMERGING     = 7,
    LURKER_EVENT_REEMERGE       = 8,
    LURKER_EVENT_SPAWN_ADDS     = 9
};

struct boss_the_lurker_belowAI : public BossAI
{
    boss_the_lurker_belowAI(Creature *c) : BossAI(c, DATA_THELURKERBELOW) 
    {
        me->AddUnitMovementFlag(MOVEFLAG_SWIMMING);
        me->SetLevitate(true);
    }

    bool m_rotating;
    bool m_submerged;
    bool m_emoting;


    void Reset()
    {
        ClearCastQueue();
        events.Reset();

        instance->SetData(DATA_THELURKERBELOWEVENT, NOT_STARTED);
        instance->SetData(DATA_STRANGE_POOL, NOT_STARTED);


        // Set reactstate to: Defensive
        me->SetReactState(REACT_DEFENSIVE);
        me->SetVisibility(VISIBILITY_OFF);


        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);

        // Bools
        m_rotating = false;
        m_submerged = true;
        m_emoting = false;
        
        

        summons.DespawnAll();
        ForceSpellCast(me, SPELL_SUBMERGE, INTERRUPT_AND_CAST_INSTANTLY);
    }

    void EnterCombat(Unit *who)
    {
        instance->SetData(DATA_THELURKERBELOWEVENT, IN_PROGRESS);
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void AttackStart(Unit *pWho)
    {
        if (!pWho || !me->HasReactState(REACT_AGGRESSIVE))
            return;

        if (m_creature->Attack(pWho, true))
            {} //DoStartNoMovement(pWho);
    }

    void MoveInLineOfSight(Unit *pWho)
    {
        if (me->GetVisibility() == VISIBILITY_OFF || me->IsInCombat())
            return;

        AttackStart(pWho);
    }

    void JustDied(Unit* Killer)
    {
        instance->SetData(DATA_THELURKERBELOWEVENT, DONE);
        me->AddUnitMovementFlag(MOVEFLAG_SWIMMING); 
        me->SetLevitate(true); // just to be sure
        DoTeleportTo(37.6721f, -417.457f, -21.62f, me->GetOrientation());
    }

    void SpellHitTarget(Unit *target, const SpellEntry *spell)
    {
    }


    void SummonAdds()
    {
        for (uint8 i = 0; i < 9; i++)
            me->SummonCreature(addPos[i][0], addPos[i][1], addPos[i][2], addPos[i][3], 0, TEMPSUMMON_DEAD_DESPAWN, 2000);
    }

    void MovementInform(uint32 type, uint32 data)
    {
    }

    bool SelectTarget()
    {
        std::list<HostileReference*>& m_threatlist = me->getThreatManager().getThreatList();
        if (m_threatlist.empty())
            return false;

        for (std::list<HostileReference*>::iterator itr = m_threatlist.begin(); itr != m_threatlist.end(); ++itr)
        {
            Unit* pUnit = Unit::GetUnit((*me), (*itr)->getUnitGuid());
            if (pUnit && pUnit->IsAlive())
            {
                if (me->GetDistance(pUnit) > 1.5f)
                    (*itr)->setThreat(0.0f);
            }
        }
        return UpdateVictim();
    }

    void DoMeleeAttackIfReady()
    {
        if (!SelectTarget())
        {
            EnterEvadeMode();
            return;
        }

        if (m_submerged || m_rotating || m_emoting)
            return;

        if (me->GetDistance(me->GetVictim()) <= 1.5f)
        {
            me->Attack(me->GetVictim(), true);
            UnitAI::DoMeleeAttackIfReady();
            return;
        }
        else if (me->isAttackReady())
        {
            Unit *pTarget = SelectCastTarget(SPELL_WATERBOLT, CAST_RANDOM);
            if (!pTarget)
                return;
            m_creature->SetSelection(pTarget->GetGUID());
            m_creature->CastSpell(pTarget, SPELL_WATERBOLT, false);
            //ForceSpellCast(SPELL_WATERBOLT, CAST_RANDOM, INTERRUPT_AND_CAST_INSTANTLY);
            me->resetAttackTimer();
        }

    }
    

    void UpdateAI(const uint32 diff)
    {
        if (instance->GetData(DATA_STRANGE_POOL) != IN_PROGRESS)
            return;

        if (me->GetVisibility() == VISIBILITY_OFF)
        {
            me->SetReactState(REACT_AGGRESSIVE);
            me->SetVisibility(VISIBILITY_ON);
            events.Reset();
            events.ScheduleEvent(LURKER_EVENT_REEMERGING, 1000);
            DoZoneInCombat();
            me->SetIgnoreVictimSelection(true);
        }
        events.Update(diff);
        DoSpecialThings(diff, DO_PULSE_COMBAT);

        if (m_rotating)
            me->SetSelection(0); // another !IMPORTANTE! hack! he sets target on someone who he hits with spell, so this is a hack, very important hack!  also i have no idea what is causing him to target the spout victim when he hits...



        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case LURKER_EVENT_SPOUT_EMOTE:
                {
                    ClearCastQueue();
                    me->MonsterTextEmote(EMOTE_SPOUT, 0, true);
                    float fix = me->GetOrientation();
                    me->SetSelection(0);
                    ForceSpellCast(SPELL_SPOUT_BREATH, CAST_NULL, INTERRUPT_AND_CAST_INSTANTLY);
                    me->SetOrientation(fix);
                    me->SendHeartBeat(); // send this orientation change to players
                    m_emoting = true;
                    me->SetIgnoreVictimSelection(true);

                    events.RescheduleEvent(LURKER_EVENT_WHIRL, 15100); // 100 ms after spout we whirl
                    events.RescheduleEvent(LURKER_EVENT_GEYSER, urand(15100, 25000));

                    events.ScheduleEvent(LURKER_EVENT_SPOUT, 3000);
                    break;
                }
                case LURKER_EVENT_SPOUT:
                {
                    m_emoting = false;
                    m_rotating = true;
                    ForceSpellCast(SPELL_SPOUT_VISUAL, CAST_NULL, INTERRUPT_AND_CAST_INSTANTLY);
                    me->GetMotionMaster()->MoveRotate(12000, RAND(ROTATE_DIRECTION_LEFT, ROTATE_DIRECTION_RIGHT));
                    
                    events.ScheduleEvent(LURKER_EVENT_SPOUT_EMOTE, 45000);
                    events.ScheduleEvent(LURKER_EVENT_STOP_SPOUT, 12000);
                    break;
                }
                case LURKER_EVENT_STOP_SPOUT:
                {
                    me->RemoveAurasDueToSpell(SPELL_SPOUT_VISUAL);
                    m_rotating = false;
                    me->SetIgnoreVictimSelection(false);
                    break;
                }
                case LURKER_EVENT_WHIRL:
                {
                    ForceSpellCast(SPELL_WHIRL, CAST_NULL);
                    events.RescheduleEvent(LURKER_EVENT_WHIRL, 18000);
                    break;
                }
                case LURKER_EVENT_GEYSER:
                {
                    AddSpellToCast(SPELL_GEYSER, CAST_RANDOM);
                    events.ScheduleEvent(LURKER_EVENT_GEYSER, urand(5000, 10000));
                    break;
                }
                case LURKER_EVENT_SUBMERGE:
                {
                    ForceSpellCast(me, SPELL_SUBMERGE, INTERRUPT_AND_CAST_INSTANTLY);

                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
                    me->RemoveAllAuras();

                    m_submerged = true;

                    events.Reset();
                    events.ScheduleEvent(LURKER_EVENT_SPAWN_ADDS, 5000);
                    events.ScheduleEvent(LURKER_EVENT_REEMERGING, 55000);
                    break;
                }
                case LURKER_EVENT_SPAWN_ADDS:
                {
                    SummonAdds();
                    break;
                }
                case LURKER_EVENT_REEMERGING:
                {
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
                    me->RemoveAurasDueToSpell(SPELL_SUBMERGE);
                    me->UpdateVisibilityAndView();
                    ForceSpellCast(SPELL_EMERGE, CAST_NULL, INTERRUPT_AND_CAST_INSTANTLY);
                    me->UpdateVisibilityAndView();
                    events.Reset();
                    events.ScheduleEvent(LURKER_EVENT_REEMERGE, 5000);
                    events.ScheduleEvent(LURKER_EVENT_WHIRL, 5250); // whirl right after reemerging, 250 ms to finish animation and shit
                    ClearCastQueue(); // prevent casting waterbolt on random target even if someone is close
                    break;
                }
                case LURKER_EVENT_REEMERGE:
                {
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);

                    m_submerged = false;

                    events.ScheduleEvent(LURKER_EVENT_SPOUT_EMOTE, 45000);;
                    events.ScheduleEvent(LURKER_EVENT_GEYSER, urand(1000, 10000));
                    events.ScheduleEvent(LURKER_EVENT_SUBMERGE, 90000);
                    break;
                }

            }
        }


        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

enum guardianSpells
{
    SPELL_HARMSTRING   = 9080,
    SPELL_ARCING_SMASH = 28168
};

struct mob_coilfang_guardianAI : public ScriptedAI
{
    mob_coilfang_guardianAI(Creature *c) : ScriptedAI(c) { }

    Timer m_harmstringTimer;
    Timer m_arcingTimer;

    void Reset()
    {
        ClearCastQueue();

        m_harmstringTimer.Reset(urand(5000, 15000));
        m_arcingTimer.Reset(urand(15000, 20000));
    }

    void JustRespawned()
    {
        DoZoneInCombat(200.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_harmstringTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_HARMSTRING);
            m_harmstringTimer = 10500;
        }


        if (m_arcingTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_ARCING_SMASH);
            m_arcingTimer = urand(10000, 20000);
        }
        

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

enum ambusherSpells
{
    SPELL_SPREAD_SHOT = 37790,
    SPELL_NORMAL_SHOT = 37770
};

struct mob_coilfang_ambusherAI : public Scripted_NoMovementAI
{
    mob_coilfang_ambusherAI(Creature *c) : Scripted_NoMovementAI(c) { }

    Timer m_spreadTimer;
    Timer m_shootTimer;

    void Reset()
    {
        ClearCastQueue();

        m_spreadTimer.Reset(urand(10000, 20000));
        m_shootTimer.Reset(2000);
    }

    void JustRespawned()
    {
        DoZoneInCombat(200.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_spreadTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_SPREAD_SHOT, CAST_RANDOM);
            m_spreadTimer = urand(10000, 20000);
        }


        if (m_shootTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_NORMAL_SHOT, CAST_RANDOM);
            m_shootTimer = 2000;
        }
        
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_coilfang_guardian(Creature* pCreature)
{
    return new mob_coilfang_guardianAI (pCreature);
}

CreatureAI* GetAI_mob_coilfang_ambusher(Creature* pCreature)
{
    return new mob_coilfang_ambusherAI (pCreature);
}

CreatureAI* GetAI_boss_the_lurker_below(Creature* pCreature)
{
    return new boss_the_lurker_belowAI (pCreature);
}

void AddSC_boss_the_lurker_below()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_the_lurker_below";
    newscript->GetAI = &GetAI_boss_the_lurker_below;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_coilfang_guardian";
    newscript->GetAI = &GetAI_mob_coilfang_guardian;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_coilfang_ambusher";
    newscript->GetAI = &GetAI_mob_coilfang_ambusher;
    newscript->RegisterSelf();
}
