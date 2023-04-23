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

/* ScriptData
SDName: Lord Ahune
SD%Complete: 80
SDComment: Lord Ahune in Midsummer Festival
SDCategory: Slave Pens
EndScriptData */

#include "scriptPCH.h"

#define DATA_AHUNEEVENT     1

enum // Ahune
{
    SPELL_GHOST_DISGUISE            = 46786,
    SPELL_SLIPPERY_FLOOR            = 45945,
    SPELL_AHUNE_SHIELD              = 45954,
    SPELL_AHUNE_RESURFACES          = 46402,
    SPELL_AHUNE_SELF_STUN           = 46416,
    SPELL_SUMMON_ICE_SPEAR_BUNNY    = 46359,
    SPELL_ICE_SPEAR_KNOWCKBACK      = 46360,

    CREATURE_CORE                   = 25865,
    CREATURE_HAILSTONE              = 25755,
    CREATURE_COLDWAVE               = 25756,
    CREATURE_FROSTWIND              = 25757,

    GO_ICE_SPEAR                    = 188077
};

struct boss_lord_ahune_coreAI : public Scripted_NoMovementAI
{
    boss_lord_ahune_coreAI(Creature *c) : Scripted_NoMovementAI(c) {}

    Timer   AhuneGhostHide_Timer;
    Timer   AhuneGhostShow_Timer;
    bool    DamageRecentlyTaken;
    bool    NeedToHideGhost;
    uint64  Ahune;

    void Reset()
    {
        DamageRecentlyTaken = false;
        NeedToHideGhost = false;
        AhuneGhostShow_Timer.Reset(3000);
        Ahune = 0;
    }

    void EnterCombat(Unit *who)
    {
    }

    void DamageTaken(Unit*, uint32 &)
    {
        DamageRecentlyTaken = true;
    }

    void JustDied(Unit *u)
    {
       if (Ahune)
            if (Unit *ahune = me->GetUnit(*me, Ahune))
            {
                ahune->RemoveAurasDueToSpell(SPELL_AHUNE_SELF_STUN);
                ahune->SetHealth(1, true);
            }
    }

    void SetAhuneVisibility(bool val)
    {
        if (Ahune)
            if (Unit *ahune = me->GetUnit(*me, Ahune))
                ahune->SetVisibility(val ? VISIBILITY_ON : VISIBILITY_OFF);
    }

    void UpdateAI(const uint32 diff)
    {
        if (NeedToHideGhost)
        {
            if (AhuneGhostHide_Timer.Expired(diff))
            {
                SetAhuneVisibility(false);
                NeedToHideGhost = false;
            }
        }

        if (DamageRecentlyTaken && !NeedToHideGhost)
        {
            if (AhuneGhostShow_Timer.Expired(diff))
            {
                SetAhuneVisibility(true);
                DamageRecentlyTaken = false;
                NeedToHideGhost = true;
                AhuneGhostHide_Timer.Reset(3000);
                AhuneGhostShow_Timer = 3000;
            }
        }
    }
};

CreatureAI* GetAI_boss_lord_ahune_core(Creature *_Creature)
{
    return new boss_lord_ahune_coreAI (_Creature);
}

struct boss_lord_ahuneAI : public Scripted_NoMovementAI
{
    ScriptedInstance* pInstance;
    SummonList Summons;

    uint8 Phase;            // 1, 2
    uint64 Core;            // GUID of core creature

    Timer Phase_Timer;
    uint8 PhaseCounter;
    Timer IceSpear_Timer;
    Timer Elementals_Timer;
    Timer Hailstone_Timer;
    Timer Death_Timer;

    boss_lord_ahuneAI(Creature *c) : Scripted_NoMovementAI(c), Summons(m_creature)
    {
        pInstance = (c->GetInstanceData());
    }

    void Reset()
    {
        if (pInstance)
            pInstance->SetData(DATA_AHUNEEVENT, NOT_STARTED);

        PhaseCounter = 0;
        Phase = 1;
        Core = 0;
        Phase_Timer.Reset(60000);
        Summons.DespawnAll();
        me->SetVisibility(VISIBILITY_ON);

        Elementals_Timer.Reset(5000);
        Hailstone_Timer.Reset(3000);
        IceSpear_Timer.Reset(5000);
        Death_Timer.Reset(1000);

        me->LowerPlayerDamageReq(me->GetHealth() / 2 + 1);
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(DATA_AHUNEEVENT, IN_PROGRESS);

        DoCast(me, SPELL_AHUNE_SHIELD, true);
        DoCast(me, SPELL_SLIPPERY_FLOOR, true);
        float x, y, z;
        me->GetPosition(x, y, z);

        if (Creature *core = me->SummonCreature(CREATURE_CORE, x, y, z, 0, TEMPSUMMON_MANUAL_DESPAWN, 0))
        {
            ((boss_lord_ahune_coreAI*)core->AI())->Ahune = me->GetGUID();
            core->SetVisibility(VISIBILITY_OFF);
            Core = core->GetGUID();
        }
    }

    void KilledUnit(Unit *victim)
    {

    }

    void UpdateHealth(Unit *core)
    {
        if (!me->GetVictim())
            return;

        if (me->GetHealth() > core->GetHealth())
            me->SetHealth(core->GetHealth());
        else
            core->SetHealth(me->GetHealth());
    }

    void JustSummoned(Creature* summoned)
    {
        if (summoned->AI() && me->GetVictim())
            summoned->AI()->AttackStart(me->GetVictim());

        Summons.Summon(summoned);
    }

    void SummonedCreatureDespawn(Creature *summon)
    {
        Summons.Despawn(summon);
    }

    void JustDied(Unit *victim)
    {
        if (pInstance)
            pInstance->SetData(DATA_AHUNEEVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (me->GetHealth() == 1)            // to ensure nice death animation
        {
            me->SetVisibility(VISIBILITY_ON);
            if (Death_Timer.Expired(diff))
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->DealDamage(me, 1);
            }
            return;
        }

        if (Phase_Timer.Expired(diff))
        {
            if (Phase == 1)
            {
                Phase = 2;
                if (Unit *core = me->GetUnit(*me, Core))
                {
                    UpdateHealth(core);
                    core->SetVisibility(VISIBILITY_ON);
                    me->SetVisibility(VISIBILITY_OFF);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    DoCast(me, SPELL_GHOST_DISGUISE, true);
                }
                Phase_Timer = 30000;
            }
            else
            {
                PhaseCounter++;
                Phase = 1;
                if (Unit *core = me->GetUnit(*me, Core))
                {
                    UpdateHealth(core);
                    core->SetVisibility(VISIBILITY_OFF);
                    me->RemoveAurasDueToSpell(SPELL_GHOST_DISGUISE);
                    me->RemoveAurasDueToSpell(SPELL_AHUNE_SELF_STUN);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    me->SetVisibility(VISIBILITY_ON);
                    DoCast(me, SPELL_AHUNE_SHIELD, true);
                    DoCast(me, SPELL_SLIPPERY_FLOOR, true);
                    DoCast(me, SPELL_AHUNE_RESURFACES, true);
                }
                Phase_Timer = 60000;
            }
        }

        if (Phase == 1)
        {
            float x, y, z;
            me->GetPosition(x, y, z);

            if (me->GetVisibility() != VISIBILITY_ON)
                me->SetVisibility(VISIBILITY_ON);

            if (Hailstone_Timer.Expired(diff))
            {
                me->SummonCreature(CREATURE_HAILSTONE, x, y, z, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                Hailstone_Timer = 60000;
            }

            if (Elementals_Timer.Expired(diff))
            {
                me->SummonCreature(CREATURE_COLDWAVE, x, y, z, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                me->SummonCreature(CREATURE_COLDWAVE, x, y, z, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                for (int i = 0; i < PhaseCounter; i++)
                    me->SummonCreature(CREATURE_FROSTWIND, x, y, z, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                Elementals_Timer = 8000;
            }

            if (IceSpear_Timer.Expired(diff))
            {
                if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                    DoCast(target, SPELL_SUMMON_ICE_SPEAR_BUNNY, true);
                IceSpear_Timer = 5500;
            }

            DoMeleeAttackIfReady();
        }

    }
};

CreatureAI* GetAI_boss_lord_ahune(Creature *_Creature)
{
    return new boss_lord_ahuneAI (_Creature);
}

struct npc_ice_spear_bunnyAI : public Scripted_NoMovementAI
{
    npc_ice_spear_bunnyAI(Creature *c) : Scripted_NoMovementAI(c)  {}

    Timer _Timer;
    uint64 IceSpear;
    bool Init;
    bool Knockback;
    bool Despawn;

    void Reset()
    {
        _Timer.Reset(1);
        IceSpear = 0;
        Init = false;
        Knockback = false;
        Despawn = false;
    }

    void KnockbackPlayers()
    {
        std::list<Unit*> PlayerList;
        uint32 knockbackRadius = 3;
        MaNGOS::AnyUnitInObjectRangeCheck  check(me, knockbackRadius);
        MaNGOS::UnitListSearcher<MaNGOS::AnyUnitInObjectRangeCheck > searcher(PlayerList, check);
        Cell::VisitWorldObjects(me, searcher, knockbackRadius);

        for (std::list<Unit*>::iterator i = PlayerList.begin(); i != PlayerList.end(); i++)
            (*i)->CastSpell((*i), SPELL_ICE_SPEAR_KNOWCKBACK, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!Init)
        {
            Init = true;
            float x, y, z;
            me->GetPosition(x,y,z);

            if (GameObject* iceSpear = me->SummonGameObject(GO_ICE_SPEAR, x, y, z, 0, 0, 0, 0, 0, 0))
                IceSpear = iceSpear->GetGUID();

        }

        if (_Timer.GetCurrent() > 3000 && !Knockback)
        {
            if (IceSpear)
                if (GameObject *iceSpear = me->GetMap()->GetGameObject(IceSpear))
                    iceSpear->Use(me);

            Knockback = true;
            KnockbackPlayers();
        }

        if (_Timer.GetCurrent() > 6000)
        {
            if (IceSpear)
                if (GameObject *iceSpear = me->GetMap()->GetGameObject(IceSpear))
                {
                    iceSpear->SendObjectDeSpawnAnim(iceSpear->GetGUID());
                    iceSpear->AddObjectToRemoveList();
                }

            me->setDeathState(JUST_DIED);
            me->RemoveCorpse();
        }

        _Timer.Update(diff);
    }

};

CreatureAI* GetAI_npc_ice_spear_bunny(Creature *_Creature)
{
    return new npc_ice_spear_bunnyAI (_Creature);
}

void AddSC_boss_lord_ahune()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_lord_ahune";
    newscript->GetAI = &GetAI_boss_lord_ahune;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_lord_ahune_core";
    newscript->GetAI = &GetAI_boss_lord_ahune_core;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_ice_spear_bunny";
    newscript->GetAI = &GetAI_npc_ice_spear_bunny;
    newscript->RegisterSelf();
}

