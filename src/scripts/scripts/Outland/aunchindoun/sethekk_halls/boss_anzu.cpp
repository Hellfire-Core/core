/*
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
#include "def_sethekk_halls.h"

enum Anzu
{
    SPELL_SPELL_BOMB                = 40303,
    SPELL_CYCLONE_OF_FEATHERS       = 40321,
    SPELL_PARALYZING_SCREECH        = 40184,
    SPELL_BANISH                    = 42354,   // probably completely wrong spell

    SPELL_PROTECTION_OF_THE_HAWK    = 40237,
    SPELL_SPITE_OF_THE_EAGLE        = 40240,
    SPELL_SPEED_OF_THE_FALCON       = 40241,

    NPC_HAWK_SPIRIT                 = 23134,
    NPC_EAGLE_SPIRIT                = 23136,
    NPC_FALCON_SPIRIT               = 23135,
    NPC_BROOD_OF_ANZU               = 23132,

    GO_RAVENS_CLAW                  = 185554
};

uint32 AnzuSpirits[] = { NPC_HAWK_SPIRIT, NPC_EAGLE_SPIRIT, NPC_FALCON_SPIRIT };

float AnzuSpiritLoc[][3] = {
    { -113, 293, 27 },
    { -77, 315, 27 },
    { -62, 288, 27 }
};

struct boss_anzuAI : public ScriptedAI
{
    boss_anzuAI(Creature* c) : ScriptedAI(c), summons(c)
    {
        pInstance = c->GetInstanceData();
    }

    SummonList summons;
    ScriptedInstance* pInstance;

    Timer Banish_Timer;
    Timer SpellBomb_Timer;
    Timer CycloneOfFeathers_Timer;
    Timer ParalyzingScreech_Timer;
    uint8 BanishedTimes;
    uint8 BroodCount;

    void Reset()
    {
        ClearCastQueue();
        summons.DespawnAll();

        Banish_Timer = 0;
        SpellBomb_Timer.Reset(22000);
        CycloneOfFeathers_Timer.Reset(5000);
        ParalyzingScreech_Timer.Reset(14000);
        BanishedTimes = 2;

        if (pInstance)
            pInstance->SetData(DATA_ANZUEVENT, NOT_STARTED);
    }

    void IsSummonedBy(Unit *summoner)
    {
        if (GameObject* go = FindGameObject(GO_RAVENS_CLAW, 20, me))
            go->Delete();
    }

    void JustSummoned(Creature *summon)
    {
        if (summon->GetEntry() == NPC_BROOD_OF_ANZU)
        {
            summon->AI()->AttackStart(me->GetVictim());
            BroodCount++;
        }
        summons.Summon(summon);
    }

    void SummonedCreatureDespawn(Creature *summon)
    {
        if (summon->GetEntry() == NPC_BROOD_OF_ANZU && BroodCount > 0)
            BroodCount--;
        summons.Despawn(summon);
    }

    void SummonSpirits()
    {
        for (uint8 i = 0; i < 3; i++)
            me->SummonCreature(AnzuSpirits[i], AnzuSpiritLoc[i][0], AnzuSpiritLoc[i][1], AnzuSpiritLoc[i][2], 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
    }

    void SummonBrood()
    {
        for (uint8 i = 0; i < 5; i++)
            DoSummon(NPC_BROOD_OF_ANZU, me, 5, 0, TEMPSUMMON_CORPSE_DESPAWN);
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(DATA_ANZUEVENT, IN_PROGRESS);
        SummonSpirits();
    }

    void JustDied(Unit* Killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_ANZUEVENT, DONE);
        summons.DespawnAll();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Banish_Timer.GetInterval())
        {
            if (BroodCount == 0 || Banish_Timer.Expired(diff))
            {
                Banish_Timer = 0;
                me->RemoveAurasDueToSpell(SPELL_BANISH);
            }
        }
        else
        {
            if (ParalyzingScreech_Timer.Expired(diff))
            {
                AddSpellToCast(me, SPELL_PARALYZING_SCREECH);
                ParalyzingScreech_Timer = 26000;
            }

            if (SpellBomb_Timer.Expired(diff))
            {
                if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                    AddSpellToCast(target, SPELL_SPELL_BOMB);
                SpellBomb_Timer = 30000;
            }

            if (CycloneOfFeathers_Timer.Expired(diff))
            {
                if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 1, 45.0f, true, me->getVictimGUID()))
                    AddSpellToCast(target, SPELL_CYCLONE_OF_FEATHERS);
                CycloneOfFeathers_Timer = 21000;
            }

            if (HealthBelowPct(33 * BanishedTimes))
            {
                BanishedTimes--;
                Banish_Timer = 45000;
                ForceSpellCast(me, SPELL_BANISH, INTERRUPT_AND_CAST, true);
                SummonBrood();
            }
        }

        CastNextSpellIfAnyAndReady();

        if (!Banish_Timer.GetInterval())
            DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_anzu(Creature *_Creature)
{
    return new boss_anzuAI(_Creature);
}

struct npc_anzu_spiritAI : public Scripted_NoMovementAI
{
    npc_anzu_spiritAI(Creature* c, uint32 spell) : Scripted_NoMovementAI(c)
    {
        Spell = spell;
    }

    uint32 Spell;
    Timer _Timer;

    void Reset() {
        _Timer.Reset(5000);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_PL_SPELL_TARGET);
    }

    bool isDruidHotSpell(const SpellEntry *spellProto)
    {
        return spellProto->SpellFamilyName == SPELLFAMILY_DRUID && (spellProto->SpellFamilyFlags & 0x1000000050LL);
    }

    void OnAuraApply(Aura *aur, Unit *caster, bool stackApply)
    {
        if (isDruidHotSpell(aur->GetSpellProto()))
        {
            DoCast(me, Spell);
            _Timer = 5000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (_Timer.Expired(diff))
        {
            const Unit::AuraList& auras = me->GetAurasByType(SPELL_AURA_PERIODIC_HEAL);
            for (Unit::AuraList::const_iterator i = auras.begin(); i != auras.end(); ++i)
            {
                if (isDruidHotSpell((*i)->GetSpellProto()))
                {
                    DoCast(me, Spell);
                    break;
                }
            }
            _Timer = 5000;
        }
    }
};

CreatureAI* GetAI_npc_eagle_spirit(Creature *_Creature)
{
    return new npc_anzu_spiritAI(_Creature, SPELL_SPITE_OF_THE_EAGLE);
}

CreatureAI* GetAI_npc_hawk_spirit(Creature *_Creature)
{
    return new npc_anzu_spiritAI(_Creature, SPELL_PROTECTION_OF_THE_HAWK);
}

CreatureAI* GetAI_npc_falcon_spirit(Creature *_Creature)
{
    return new npc_anzu_spiritAI(_Creature, SPELL_SPEED_OF_THE_FALCON);
}

void AddSC_boss_anzu()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_anzu";
    newscript->GetAI = &GetAI_boss_anzu;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_eagle_spirit";
    newscript->GetAI = &GetAI_npc_eagle_spirit;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_falcon_spirit";
    newscript->GetAI = &GetAI_npc_falcon_spirit;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_hawk_spirit";
    newscript->GetAI = &GetAI_npc_hawk_spirit;
    newscript->RegisterSelf();
}
