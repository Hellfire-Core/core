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
SDName: Boss_Nethermancer_Sepethrea
SD%Complete: 99
SDComment: Check melee dmg and immunities
SDCategory: Tempest Keep, The Mechanar
EndScriptData */

#include "precompiled.h"
#include "def_mechanar.h"

#define SAY_AGGRO                       -1554013
#define SAY_SUMMON                      -1554014    // using this when resummoning adds, used on timer here to prevent double yell
#define SAY_DRAGONS_BREATH_1            -1554015
#define SAY_DRAGONS_BREATH_2            -1554016
#define SAY_SLAY1                       -1554017
#define SAY_SLAY2                       -1554018
#define SAY_DEATH                       -1554019

#define SPELL_SUMMON_RAGIN_FLAMES       35275
#define H_SPELL_SUMMON_RAGIN_FLAMES     39084

#define SPELL_FROST_ATTACK              45195
#define SPELL_ARCANE_BLAST              35314
#define SPELL_DRAGONS_BREATH            35250
#define SPELL_SOLARBURN                 (HeroicMode ? 38930 : 35267)

struct boss_nethermancer_sepethreaAI : public ScriptedAI
{
    boss_nethermancer_sepethreaAI(Creature *c) : ScriptedAI(c), summons(me)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance *pInstance;


    Timer arcane_blast_Timer;
    Timer dragons_breath_Timer;
    Timer yell_timer;
    Timer solarburn_timer;

    SummonList summons;

    void Reset()
    {
        arcane_blast_Timer.Reset(urand(12000, 18000));
        dragons_breath_Timer.Reset(urand(22000, 28000));
        yell_timer.Reset(5000);
        solarburn_timer.Reset(7000);

        pInstance->SetData(DATA_NETHERMANCER_EVENT, NOT_STARTED);

        summons.DespawnAll();
    }

    void EnterCombat(Unit *who)
    {
        pInstance->SetData(DATA_NETHERMANCER_EVENT, IN_PROGRESS);

        DoScriptText(SAY_AGGRO, me);
        AddSpellToCast(HeroicMode ? H_SPELL_SUMMON_RAGIN_FLAMES : SPELL_SUMMON_RAGIN_FLAMES, CAST_SELF);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2), me);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, me);
        pInstance->SetData(DATA_NETHERMANCER_EVENT, DONE);
        pInstance->SetData(DATA_BRIDGE_EVENT, DONE);
        summons.DespawnAll();
    }

    void JustSummoned(Creature * summoned)
    {
        if (summoned)
            summons.Summon(summoned);
    }

    void DamageMade(Unit* target, uint32 & , bool direct, uint8 school_mask)
    {
        if (direct && !me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED))
            if(roll_chance_i(40))
                me->CastSpell(target, SPELL_FROST_ATTACK, true);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        //Arcane Blast with knockback, reducing threat by 50%
        if (arcane_blast_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_ARCANE_BLAST, CAST_TANK);
            arcane_blast_Timer = urand(25000, 35000);
            me->getThreatManager().modifyThreatPercent(me->GetVictim(), -50.0f);
        }

        //Dragons Breath
        if (dragons_breath_Timer.Expired(diff))
        {
            AddSpellToCastWithScriptText(SPELL_DRAGONS_BREATH, CAST_TANK, RAND(SAY_DRAGONS_BREATH_1, SAY_DRAGONS_BREATH_2, 0, 0));
            dragons_breath_Timer = urand(30000, 35000);
        }

        if (yell_timer.Expired(diff))
        {
            DoScriptText(SAY_SUMMON, me);
            yell_timer = 0;
        }

        if (solarburn_timer.Expired(diff))
        {
            AddSpellToCast(SPELL_SOLARBURN);
            solarburn_timer = urand(12000, 15000);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_nethermancer_sepethrea(Creature *_Creature)
{
    return new boss_nethermancer_sepethreaAI (_Creature);
}

#define SPELL_INFERNO                   39346
#define SPELL_FIRE_TAIL                 35278

struct mob_ragin_flamesAI : public ScriptedAI
{
    mob_ragin_flamesAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    bool canMelee;
    Timer infernoTimer;
    uint64 currentTarget;

    void Reset()
    {
        infernoTimer.Reset(urand(8000, 13000));
        canMelee = true;

        me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_MAGIC, true);
        me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, true);
        me->SetSpeed(MOVE_RUN, HeroicMode ? 0.8f : 0.5f);

        SetAutocast(SPELL_FIRE_TAIL, HeroicMode ? 700 : 1200, true, CAST_SELF);
    }

    void ChangeTarget()
    {
        if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 500, true, me->getVictimGUID()))
        {
            Unit * prevTarUnit = me->GetMap()->GetUnit(currentTarget);
            if (prevTarUnit)
                me->AddThreat(prevTarUnit, -5000000.0f);

            currentTarget = target->GetGUID();
            me->AI()->AttackStart(target);
            me->AddThreat(target, 5000000.0f);
        }
    }

    void JustRespawned()
    {
        DoZoneInCombat();
        ChangeTarget();
    }

    void OnAuraApply(Aura* aur, Unit* caster, bool stackApply)
    {
        if (aur->GetId() == SPELL_INFERNO)
        {
            StopAutocast();
            canMelee = false;
        }
    }

    void OnAuraRemove(Aura* aur, bool stackApply)
    {
        if (aur->GetId() == SPELL_INFERNO)
        {
            StartAutocast();
            ChangeTarget();
            canMelee = true;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoSpecialThings(diff, DO_COMBAT_N_SPEED, 200.0f, HeroicMode ? 0.8f : 0.5f);

        if (infernoTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_INFERNO, CAST_SELF);
            infernoTimer = urand(16000,21000);
        }

        CastNextSpellIfAnyAndReady(diff);

        if(canMelee)
            DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_mob_ragin_flames(Creature *_Creature)
{
    return new mob_ragin_flamesAI (_Creature);
}

void AddSC_boss_nethermancer_sepethrea()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_nethermancer_sepethrea";
    newscript->GetAI = &GetAI_boss_nethermancer_sepethrea;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_ragin_flames";
    newscript->GetAI = &GetAI_mob_ragin_flames;
    newscript->RegisterSelf();
}
