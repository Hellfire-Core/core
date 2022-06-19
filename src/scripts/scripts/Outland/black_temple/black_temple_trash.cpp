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

/* ScriptData
SDName: Black_Temple_Trash
SD%Complete: 99.9% (68/68)
SDComment: Trash NPCs divided by to boss links
SDCategory: Black Temple
EndScriptData */

#include "precompiled.h"
#include "def_black_temple.h"

#define AGGRO_RANGE             35.0

/* ============================
*
*      HIGL  LORD  NAJENTUS
*
* ============================*/

/* Content Data
    * Aqueous Lord
    * Aqueous Spawn
    * Coilskar General
    * Coilskar Harpooner
    * Coilskar Sea-Caller
    * Coilskar Soothsayer
    * Coilskar Wrangler
    * Dragon Turtle
    * Leviathan
    */

/****************
* Aqueous Lord - id 22878
*****************/

enum AqueousLord
{
    SPELL_VILE_SLIME = 40099,
    SPELL_CRASHING_WAVE = 40100,

    NPC_AQUEOUS_SPAWN = 22883
};

struct mob_aqueous_lordAI : public ScriptedAI
{
    mob_aqueous_lordAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer VileSlime;
    Timer SummonTimer;
    Timer CrashingWave;

    void Reset()
    {
        ClearCastQueue();

        VileSlime.Reset(5000);
        SummonTimer.Reset(urand(5000, 10000));
        CrashingWave.Reset(15000);
    }
    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (VileSlime.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true))
                AddSpellToCast(target, SPELL_VILE_SLIME);
            VileSlime = urand(20000, 30000);
        }


        if (SummonTimer.Expired(diff))
        {
            WorldLocation temp;
            for (uint8 i = 0; i < 2; ++i)
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true))
                {
                    target->GetNearPoint(temp.coord_x, temp.coord_y, temp.coord_z, 15.0);
                    Creature* Spawn = me->SummonCreature(NPC_AQUEOUS_SPAWN, temp.coord_x, temp.coord_y, temp.coord_z, 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
                    Spawn->AI()->AttackStart(target);
                }
            }
            SummonTimer = urand(20000, 40000);
        }


        if (CrashingWave.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_CRASHING_WAVE);
            CrashingWave = 15000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

/****************
* Aqueous Spawn - id 22883
*****************/

enum AqeousSpawn
{
    SPELL_SLUDGE_NOVA = 40102,
    SPELL_MERGE = 40106,

    NPC_AQUEOUS_LORD = 22878
};

struct mob_aqueous_spawnAI : public ScriptedAI
{
    mob_aqueous_spawnAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Zcheck;
    Timer SludgeNova;
    Timer MergeTimer;
    bool merging;

    void Reset()
    {
        ClearCastQueue();

        Zcheck.Reset(1000);
        SludgeNova.Reset(5000);
        MergeTimer.Reset(urand(10000, 50000));
        merging = false;

    }
    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim() && !merging)
            return;

        // to avoid melting in textures on cb
        if (Zcheck.Expired(diff))
        {
            float x, y, z;
            me->GetPosition(x, y, z);
            me->GetMap()->CreatureRelocation(me, x, y, z + 0.8, me->GetOrientation());
            Zcheck = 1000;
        }


        if (SludgeNova.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30, true))
                AddSpellToCast(target, SPELL_SLUDGE_NOVA);
            SludgeNova = urand(15000, 18000);
        }



        if (!merging && MergeTimer.Expired(diff))
        {
            if (Unit* pLord = FindCreature(NPC_AQUEOUS_LORD, 80, me))
            {
                AddSpellToCast(pLord, SPELL_MERGE, false, true);
                merging = true;
            }
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

/****************
* Coilscar General - id 22873
*****************/

enum CoilscarGeneral
{
    SAY_FREE_FRIEND = -1650022,

    SPELL_BOOMING_VOICE = 40080,
    SPELL_FREE_FRIEND = 40081,
    SPELL_CLEAVE = 40504    // guessed
};

struct mob_coilskar_generalAI : public ScriptedAI
{
    mob_coilskar_generalAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer FreeFriend;
    Timer Cleave;
    Timer BoomingVoice;

    void Reset()
    {
        ClearCastQueue();

        FreeFriend.Reset(1000);
        Cleave.Reset(10000);
        BoomingVoice.Reset(40000);
    }

    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Cleave.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_CLEAVE);
            Cleave = urand(15000, 25000);
        }


        if (FreeFriend.Expired(diff))
        {
            std::list<Creature*> pList = FindFriendlyCC(100);
            if (!pList.empty())
            {
                Unit* target = *(pList.begin());

                if (target && target->HasAuraTypeWithFamilyFlags(SPELL_AURA_MOD_STUN, SPELLFAMILY_ROGUE, 0x80))    //ignore Sap
                {
                    pList.remove((Creature*)target);
                    if (!pList.empty())
                        Unit* target = *(pList.begin());
                    else
                        target = NULL;
                }

                if (target)
                {
                    DoScriptText(SAY_FREE_FRIEND, me);
                    AddSpellToCast(target, SPELL_FREE_FRIEND);
                    pList.remove((Creature*)target);

                    if (!me->HasAura(SPELL_BOOMING_VOICE, 0))
                    {
                        AddSpellToCast(me, SPELL_BOOMING_VOICE);
                        BoomingVoice = urand(30000, 60000);
                    }
                }
            }
            FreeFriend = urand(10000, 15000);
        }


        if (BoomingVoice.Expired(diff))     //make Booming Voice from time to time even if no creature in CC
        {
            AddSpellToCast(me, SPELL_BOOMING_VOICE);
            BoomingVoice = urand(40000, 60000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

/****************
* Coilscar Harpooner - id 22874
*****************/

enum CoilscarHarpooner
{
    SPELL_HARPOONERS_MARK = 40084,
    SPELL_HOOKED_NET = 40082,
    SPELL_SPEAR_THROW = 40083
};

struct mob_coilskar_harpoonerAI : public ScriptedAI
{
    mob_coilskar_harpoonerAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer SpearThrow;
    Timer HookedNet;
    Timer HarpoonersMark;

    uint64 MarkTargetGUID;

    void Reset()
    {
        ClearCastQueue();

        SpearThrow.Reset(urand(1000, 5000));
        HookedNet.Reset(urand(15000, 20000));
        HarpoonersMark.Reset(15000);
        MarkTargetGUID = 0;
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat(80.0f);
        if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 80, true))
        {
            if (!target->HasAura(40084, 0))
            {
                DoCast(target, SPELL_HARPOONERS_MARK, false);
                MarkTargetGUID = target->GetGUID();     //constant target for Mark per creature, selected at aggro
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (HarpoonersMark.Expired(diff))
        {
            if (Player* target = me->GetPlayer(MarkTargetGUID))
            {
                if (!target->HasAura(40084, 0))
                    DoCast(target, SPELL_HARPOONERS_MARK, true);
            }
            HarpoonersMark = 15000;     //check Mark each 15s
        }


        if (HookedNet.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30, true))
                AddSpellToCast(target, SPELL_HOOKED_NET);
            HookedNet = urand(10000, 20000);
        }


        if (SpearThrow.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40, true))
                AddSpellToCast(target, SPELL_SPEAR_THROW);
            SpearThrow = urand(5000, 15000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

/****************
* Coilskar Sea-Caller - id 22875
*****************/

enum CoilscarSeaCaller
{
    SPELL_FORKED_LIGHTNING = 40088,
    SPELL_HURRICANE = 40090,
    SPELL_SUMMON_GEYSER = 40091
};

struct mob_coilskar_seacallerAI : public ScriptedAI
{
    mob_coilskar_seacallerAI(Creature *c) : ScriptedAI(c)
    {
        SpellEntry* hurricaneSpell = (SpellEntry*)GetSpellStore()->LookupEntry(SPELL_HURRICANE);
        if (hurricaneSpell)
            hurricaneSpell->AttributesEx |= SPELL_ATTR_EX_CHANNELED_1;
    }

    Timer ForkedLightning;
    Timer Hurricane;
    Timer SummonGeyser;

    void Reset()
    {
        ClearCastQueue();

        ForkedLightning.Reset(urand(1000, 3000));
        Hurricane.Reset(urand(20000, 30000));
        SummonGeyser.Reset(urand(3000, 8000));
    }
    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (ForkedLightning.Expired(diff))
        {
            Unit* target = NULL;
            AddSpellToCast(target, SPELL_FORKED_LIGHTNING);
            ForkedLightning = urand(10000, 20000);
        }



        if (Hurricane.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40, true))
                AddSpellToCast(target, SPELL_HURRICANE);
            Hurricane = urand(40000, 60000);
        }


        if (SummonGeyser.Expired(diff))
        {
            AddSpellToCast(me, SPELL_SUMMON_GEYSER);
            SummonGeyser = urand(8000, 16000);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

/***
* Coilskar Geyser - id 23080
****/

enum CoilscarGeyser
{
    SPELL_GEYSER = 40089,

    MOB_COILSCAR_SEACALLER = 22875
};

struct mob_coilskar_geyserAI : public Scripted_NoMovementAI
{
    mob_coilskar_geyserAI(Creature *c) : Scripted_NoMovementAI(c) {}

    void Reset()
    {
        Unit* SeaCaller = FindCreature(MOB_COILSCAR_SEACALLER, 20, me);
        if (SeaCaller)
        {
            me->SetLevel(SeaCaller->getLevel());
            me->setFaction(SeaCaller->getFaction());
        }
        else
        {
            me->SetLevel(71);
            me->setFaction(14);
        }
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->CastSpell(me, SPELL_GEYSER, false);
    }
    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }
};

/****************
* Coilskar Soothsayer - id 22876
*****************/

enum CoilscarSoothsayer
{
    SPELL_HOLY_NOVA = 40096,
    SPELL_RESTORATION = 40097
};

struct mob_coilskar_soothsayerAI : public ScriptedAI
{
    mob_coilskar_soothsayerAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer HolyNova;
    Timer Restoration;

    void Reset()
    {
        ClearCastQueue();

        HolyNova.Reset(urand(5000, 15000));
        Restoration.Reset((8000, 12000));
    }
    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (HolyNova.Expired(diff))
        {
            DoCastAOE(SPELL_HOLY_NOVA);
            HolyNova = urand(9000, 14000);
        }


        if (Restoration.Expired(diff))
        {
            Unit* healTarget = SelectLowestHpFriendly(40.0f);
            if (healTarget)
                AddSpellToCast(healTarget, SPELL_RESTORATION);
            Restoration = urand(15000, 20000);     //check targets each 15-20s
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

/****************
* Coilscar Wrangler - id 22877
*****************/

enum CoilscarWrangler
{
    SPELL_CLEAVE_1 = 15284,
    SPELL_ELECTRIC_SPUR = 40076,
    SPELL_LIGHTNING_PROD = 40066,

    MOB_LEVIATHAN = 22884
};

struct mob_coilskar_wranglerAI : public ScriptedAI
{
    mob_coilskar_wranglerAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Cleave;
    Timer ElectricSpur;
    Timer LightningProd;

    void Reset()
    {
        ClearCastQueue();

        Cleave.Reset(urand(2000, 6000));
        ElectricSpur.Reset(urand(15000, 40000));
        LightningProd.Reset(urand(8000, 15000));
    }
    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Cleave.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_CLEAVE_1);
            Cleave = urand(10000, 15000);
        }


        if (ElectricSpur.Expired(diff))
        {
            Unit* target = NULL;
            AddSpellToCast(target, SPELL_ELECTRIC_SPUR, false);
            ElectricSpur = 0;
        }


        if (LightningProd.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_LIGHTNING_PROD);
            LightningProd = urand(10000, 20000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

/****************
* Dragon Turtle - id 22885
*****************/

enum DragonTurtle
{
    SPELL_SHELL_SHIELD = 40087,
    SPELL_WATER_SPIT = 40086
};

struct mob_dragon_turtleAI : public ScriptedAI
{
    mob_dragon_turtleAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    bool CanBeShielded;
    Timer ShellShield;
    Timer WaterSpit;

    void Reset()
    {
        ClearCastQueue();

        WaterSpit.Reset(urand(2000, 10000));
        ShellShield.Reset(3000);
        CanBeShielded = false;
    }
    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (ShellShield.Expired(diff))
        {
            if (!CanBeShielded)
            {
                if (me->GetHealth() * 100 / me->GetMaxHealth() < 50)
                    CanBeShielded = true;
                ShellShield = 1000;
            }
            else
            {
                AddSpellToCast(SPELL_SHELL_SHIELD, CAST_SELF);
                ShellShield = urand(20000, 30000);
            }
        }


        if (WaterSpit.Expired(diff))
        {
            if (!CanBeShielded)
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40, true))
                    AddSpellToCast(target, SPELL_WATER_SPIT);
                WaterSpit = 4000;
            }
            else
            {
                if (!urand(0, 2))    //30% on cast to random player
                {
                    if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40, true))
                        AddSpellToCast(target, SPELL_WATER_SPIT);
                }
                else
                    AddSpellToCast(me->getVictim(), SPELL_WATER_SPIT);
                WaterSpit = urand(2500, 3500);
            }
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

/****************
* Leviathan - id 22884
*****************/

enum Leviathan
{
    SPELL_DEBILITATING_SPRAY = 40079,
    SPELL_POISON_SPIT = 40078,
    SPELL_TAIL_SWEEP = 40077
};

struct mob_leviathanAI : public ScriptedAI
{
    mob_leviathanAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer DebilitatingSpray;
    Timer PoisonSpit;
    Timer TailSweep;

    void Reset()
    {
        ClearCastQueue();

        DebilitatingSpray.Reset(urand(3000, 9000));
        PoisonSpit.Reset(urand(6000, 15000));
        TailSweep.Reset(6000);
    }
    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (DebilitatingSpray.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 45, true))
                AddSpellToCast(target, SPELL_DEBILITATING_SPRAY);
            DebilitatingSpray = urand(10000, 20000);
        }


        if (PoisonSpit.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30, true))
                AddSpellToCast(target, SPELL_POISON_SPIT);
            PoisonSpit = urand(12000, 18000);
        }


        if (TailSweep.Expired(diff))
        {
            AddSpellToCast(SPELL_TAIL_SWEEP, CAST_NULL);
            TailSweep = urand(5000, 8000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

/*####
##  GetAI's
###########*/

CreatureAI* GetAI_mob_aqueous_lord(Creature *_Creature)
{
    return new mob_aqueous_lordAI(_Creature);
}

CreatureAI* GetAI_mob_aqueous_spawn(Creature *_Creature)
{
    return new mob_aqueous_spawnAI(_Creature);
}

CreatureAI* GetAI_mob_coilskar_general(Creature *_Creature)
{
    return new mob_coilskar_generalAI(_Creature);
}

CreatureAI* GetAI_mob_coilskar_harpooner(Creature *_Creature)
{
    return new mob_coilskar_harpoonerAI(_Creature);
}

CreatureAI* GetAI_mob_coilskar_seacaller(Creature *_Creature)
{
    return new mob_coilskar_seacallerAI(_Creature);
}

CreatureAI* GetAI_mob_coilskar_geyser(Creature *_Creature)
{
    return new mob_coilskar_geyserAI(_Creature);
}

CreatureAI* GetAI_mob_coilskar_soothsayer(Creature *_Creature)
{
    return new mob_coilskar_soothsayerAI(_Creature);
}

CreatureAI* GetAI_mob_coilskar_wrangler(Creature *_Creature)
{
    return new mob_coilskar_wranglerAI(_Creature);
}

CreatureAI* GetAI_mob_dragon_turtle(Creature *_Creature)
{
    return new mob_dragon_turtleAI(_Creature);
}

CreatureAI* GetAI_mob_leviathan(Creature *_Creature)
{
    return new mob_leviathanAI(_Creature);
}

/* ============================
*
*          SUPREMUS
*
* ============================*/

/* Content Data
    * Bonechewer Taskmaster
    * Bonechewer Worker
    * Dragonmaw Sky Stalker
    * Dragonmaw Wind Reaver
    * Dragonmaw Wyrmcaller
    * Illidari Fearbringer
    */

#define BONECHEWER_ID                       23028

#define SPELL_BONECHEWER_DISGRUNTLED        40851
#define SPELL_BONECHEWER_FURY               40845

#define SPELL_WORKER_THROW_PICK             40844

#define SPELL_SKYSTALKER_SHOOT              40873
#define SPELL_SKYSTALKER_IMMOLATION         40872

#define SPELL_WINDREAVER_DOOM_BOLT          40876
#define SPELL_WINDREAVER_FIREBALL           40877
#define SPELL_WINDREAVER_FREEZE             40875

#define SPELL_WYRMCALLER_CLEAVE             15284
#define SPELL_WYRMCALLER_FIXATE             40892
#define SPELL_WYRMCALLER_FIXATE_TRIGGER     40893
#define SPELL_WYRMCALLER_JAB                40895
#define AURA_WYRMCALLER_FIXATED             40893

#define SPELL_FEARBRINGER_ILLIDARI_FLAMES   40938
#define SPELL_FEARBRINGER_RAIN_OF_CHAOS     40946
#define SPELL_FEARBRINGER_WAR_STOMP         40936

#define EMOTE_CALL_FOR_HELP              -1564098
#define EMOTE_FIXATE                     -1564099

/*#####
##  mob Bonechewer Taskmaster - id 23028
###########*/

struct mob_bonechewer_taskmasterAI : public ScriptedAI
{
    mob_bonechewer_taskmasterAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer disgruntledTimer;
    bool disgruntledCast;

    void Reset()
    {
        disgruntledTimer.Reset(20000);
        disgruntledCast = false;
    }

    void EnterCombat(Unit *who)
    {
        DoZoneInCombat(80.0f);
        if (urand(0, 100) < 25)
        {
            me->CastSpell(me, SPELL_BONECHEWER_DISGRUNTLED, false);
            disgruntledCast = true;
        }
    }

    void WorkerDied()
    {
        me->CastSpell(me, SPELL_BONECHEWER_FURY, false);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (!disgruntledCast)
        {
            if (disgruntledTimer.Expired(diff) || me->GetHealth() * 100 / me->GetMaxHealth() < 75)
            {
                me->CastSpell(me, SPELL_BONECHEWER_DISGRUNTLED, false);
                disgruntledCast = true;
            }
        }

        DoMeleeAttackIfReady();
    }
};

/*#####
##  mob Bonechewer Worker - id 22963
###########*/

struct mob_bonechewer_workerAI : public ScriptedAI
{
    mob_bonechewer_workerAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer throwTimer;

    void Reset()
    {
        throwTimer.Reset(15000 + urand(0, 5000));
    }

    void EnterCombat(Unit *who)
    {
        DoZoneInCombat(80.0f);
        if (who)
        {
            if (urand(0, 100) < 20)
            {
                me->CastSpell(who, SPELL_WORKER_THROW_PICK, false);
            }
        }
    }

    void JustDied(Unit *victim)
    {
        std::list<Creature*> tmp = FindAllCreaturesWithEntry(BONECHEWER_ID, 20.0);

        if (tmp.empty())
            return;

        for (std::list<Creature*>::iterator itr = tmp.begin(); itr != tmp.end(); ++itr)
        {
            if ((*itr) && (*itr)->isAlive())
                ((mob_bonechewer_taskmasterAI*)(*itr)->AI())->WorkerDied();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (throwTimer.Expired(diff))
        {
            Unit * victim = SelectUnit(SELECT_TARGET_RANDOM, 0, 35, true);

            if (victim)
                me->CastSpell(victim, SPELL_WORKER_THROW_PICK, false);

            throwTimer = 15000 + urand(0, 5000);
        }

        DoMeleeAttackIfReady();
    }
};

/*#####
##  mob Dragonmaw Sky Stalker - id 23030
###########*/

struct mob_dragonmaw_skystalkerAI : public ScriptedAI
{
    mob_dragonmaw_skystalkerAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer immolationArrowTimer;
    Timer distCheckTimer;

    void Reset()
    {
        ClearCastQueue();

        immolationArrowTimer.Reset(10000 + urand(0, 5000));
        distCheckTimer.Reset(1000);
        SetAutocast(SPELL_SKYSTALKER_SHOOT, 2500, true);
    }

    void EnterCombat(Unit *who)
    {
        DoZoneInCombat(80.0f);
        me->GetMotionMaster()->MoveChase(who, 15.0f);
    }

    Unit * GetNewTarget()
    {
        Map::PlayerList const &PlayerList = ((InstanceMap*)me->GetMap())->GetPlayers();
        std::list<Unit*> rangedList;
        for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
        {
            Player* i_pl = i->getSource();
            if (i_pl && i_pl->isAlive() && !i_pl->isGameMaster() && !me->IsWithinDistInMap(i_pl, 15) && me->IsWithinDistInMap(i_pl, 40))
                rangedList.push_back((Unit*)i_pl);
        }

        if (rangedList.empty())
            return NULL;

        int n = urand(1, rangedList.size()) - 1;

        for (int i = 0; i < n; i++)
            rangedList.pop_front();

        if (!rangedList.empty())
            return rangedList.front();

        return NULL;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        Unit * victim = me->getVictim();

        if (!victim)
            return;

        if (distCheckTimer.Expired(diff))
        {
            if (!me->IsWithinDistInMap(victim, 37))
            {
                me->StopMoving();
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveChase(victim, 25);
                distCheckTimer = 3000;
            }
            else
            {
                if (me->IsWithinDistInMap(victim, 10))
                {
                    me->SetLevitate(false);

                    if (!me->IsNonMeleeSpellCast(false))
                    {
                        DoResetThreat();
                        victim = GetNewTarget();

                        if (!victim)
                        {
                            victim = SelectUnit(SELECT_TARGET_RANDOM, 0, 40, true);
                            me->StopMoving();
                            me->GetMotionMaster()->Clear();
                            me->GetMotionMaster()->MoveChase(victim, 25);
                        }
                        distCheckTimer = 3000;
                    }
                    else
                        distCheckTimer = 500;
                }
            }
        }

        if (immolationArrowTimer.Expired(diff))
        {
            ForceSpellCast(SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true), SPELL_SKYSTALKER_IMMOLATION);
            immolationArrowTimer = 10000 + urand(0, 5000);
        }

        CastNextSpellIfAnyAndReady(diff);
    }
};

/*#####
##  mob Dragonmaw Wind Reaver - id 23330
###########*/

struct mob_dragonmaw_windreaverAI : public ScriptedAI
{
    mob_dragonmaw_windreaverAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer doomBoltTimer;
    Timer freezeTimer;
    Timer distCheckTimer;

    void Reset()
    {
        ClearCastQueue();

        doomBoltTimer.Reset(5000 + urand(0, 5000));
        freezeTimer.Reset(10000 + urand(0, 5000));
        distCheckTimer.Reset(3000);
        SetAutocast(SPELL_WINDREAVER_FIREBALL, 2500, true);
    }

    void EnterCombat(Unit *who)
    {
        DoZoneInCombat(80.0f);
        me->StopMoving();
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveChase(who, 15);
    }

    Unit * GetNewTarget()
    {
        Map::PlayerList const &PlayerList = ((InstanceMap*)me->GetMap())->GetPlayers();
        std::list<Unit*> rangedList;
        for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
        {
            Player* i_pl = i->getSource();
            if (i_pl && i_pl->isAlive() && !i_pl->isGameMaster() && !me->IsWithinDistInMap(i_pl, 15) && me->IsWithinDistInMap(i_pl, 40))
                rangedList.push_back((Unit*)i_pl);
        }

        if (rangedList.empty())
            return NULL;

        int n = urand(0, rangedList.size() - 1);

        for (int i = 0; i < n; i++)
            rangedList.pop_front();

        if (!rangedList.empty())
            return rangedList.front();

        return NULL;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        Unit * victim = me->getVictim();

        if (!victim)
            return;

        if (distCheckTimer.Expired(diff))
        {
            if (!me->IsWithinDistInMap(victim, 37))
            {
                me->StopMoving();
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveChase(victim, 25);
            }
            else
            {
                if (me->IsWithinDistInMap(victim, 10))
                {
                    me->SetLevitate(false);
                    if (!me->IsNonMeleeSpellCast(false))
                    {
                        DoResetThreat();
                        victim = GetNewTarget();

                        if (!victim)
                        {
                            victim = SelectUnit(SELECT_TARGET_RANDOM, 0, 40, true);
                            me->StopMoving();
                            me->GetMotionMaster()->Clear();
                            me->GetMotionMaster()->MoveChase(victim, 25);
                        }
                        distCheckTimer = 3000;
                    }
                    else
                        distCheckTimer = 500;
                }
            }
            distCheckTimer = 3000;
        }


        if (doomBoltTimer.Expired(diff))
        {
            AddSpellToCast(SelectUnit(SELECT_TARGET_RANDOM, 0, 40, true), SPELL_WINDREAVER_DOOM_BOLT);
            doomBoltTimer = 5000 + urand(0, 5000);
        }


        if (freezeTimer.Expired(diff))
        {
            Unit* tmpTarget = GetClosestPlayer(me, 6.0f);
            if (tmpTarget)
            {
                me->CastSpell(tmpTarget, SPELL_WINDREAVER_FREEZE, false);
                me->StopMoving();
                me->GetMotionMaster()->Clear(false);
                me->GetMotionMaster()->MoveChase(tmpTarget, 15);
                distCheckTimer = 5000;
            }
            else
            {
                ForceSpellCast(SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true), SPELL_WINDREAVER_FREEZE);
            }

            freezeTimer = 10000 + urand(0, 5000);
        }


        CastNextSpellIfAnyAndReady(diff);
    }
};

/*#####
##  mob Dragonmaw Wyrmcaller - id 22960
###########*/

struct mob_dragonmaw_wyrmcallerAI : public ScriptedAI
{
    mob_dragonmaw_wyrmcallerAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer cleaveTimer;
    Timer fixateTimer;
    Timer jabTimer;

    void Reset()
    {
        cleaveTimer.Reset(5000 + urand(0, 10000));
        fixateTimer.Reset(15000 + urand(0, 10000));
        jabTimer.Reset(5000 + urand(0, 5000));
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(EMOTE_CALL_FOR_HELP, me);
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        Unit * victim = me->getVictim();

        if (!victim)
            return;

        if (cleaveTimer.Expired(diff))
        {
            me->CastSpell(victim, SPELL_WYRMCALLER_CLEAVE, false);
            cleaveTimer = 5000 + urand(0, 10000);
        }

        if (jabTimer.Expired(diff))
        {
            me->CastSpell(victim, SPELL_WYRMCALLER_JAB, false);
            jabTimer = 5000 + urand(0, 5000);
        }


        if (fixateTimer.Expired(diff))
        {
            victim = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true);
            std::list<Creature*> FriendlyList = FindAllFriendlyInGrid(100);
            std::vector<Creature*> Friendly;

            for (std::list<Creature*>::iterator i = FriendlyList.begin(); i != FriendlyList.end(); ++i)
                if ((*i)->isInCombat() && (*i)->IsAIEnabled)
                    Friendly.push_back(*i);

            Unit* target = *(Friendly.begin() + rand() % Friendly.size());

            if (victim && target)
            {
                target->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
                target->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
                victim->CastSpell(target, SPELL_WYRMCALLER_FIXATE_TRIGGER, true);
                if (target->GetTypeId() == TYPEID_UNIT)
                    target->MonsterTextEmote(EMOTE_FIXATE, victim->GetGUID(), false);
            }

            fixateTimer = 15000 + urand(0, 10000);
        }


        DoMeleeAttackIfReady();
    }
};

/*#####
##  mob Illidari Fearbringer - id 22954
###########*/

struct mob_illidari_fearbringerAI : public ScriptedAI
{
    mob_illidari_fearbringerAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer flamesTimer;
    Timer rainTimer;
    Timer stompTimer;
    Timer checkTimer;

    void Reset()
    {
        ClearCastQueue();

        checkTimer.Reset(2000);
        flamesTimer.Reset(5000 + urand(0, 10000));
        rainTimer.Reset(15000 + urand(0, 10000));
        stompTimer.Reset(10000 + urand(0, 10000));
    }

    void EnterCombat(Unit *who) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (checkTimer.Expired(diff))
        {
            me->SetSpeed(MOVE_RUN, 2.5);
            checkTimer = 2000;
        }

        if (flamesTimer.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_FEARBRINGER_ILLIDARI_FLAMES);
            flamesTimer = 10000 + urand(0, 10000);
        }


        if (rainTimer.Expired(diff))
        {
            AddSpellToCast(SelectUnit(SELECT_TARGET_RANDOM, 0, 50, true), SPELL_FEARBRINGER_RAIN_OF_CHAOS);
            rainTimer = 20000 + urand(0, 10000);
        }


        if (stompTimer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_FEARBRINGER_WAR_STOMP);

            stompTimer = 15000 + urand(0, 10000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

/*####
##  GetAI's
###########*/

CreatureAI* GetAI_mob_bonechewer_taskmaster(Creature *_Creature)
{
    return new mob_bonechewer_taskmasterAI(_Creature);
}

CreatureAI* GetAI_mob_bonechewer_worker(Creature *_Creature)
{
    return new mob_bonechewer_workerAI(_Creature);
}

CreatureAI* GetAI_mob_dragonmaw_skystalker(Creature *_Creature)
{
    return new mob_dragonmaw_skystalkerAI(_Creature);
}

CreatureAI* GetAI_mob_dragonmaw_windreaver(Creature *_Creature)
{
    return new mob_dragonmaw_windreaverAI(_Creature);
}

CreatureAI* GetAI_mob_dragonmaw_wyrmcaller(Creature *_Creature)
{
    return new mob_dragonmaw_wyrmcallerAI(_Creature);
}

CreatureAI* GetAI_mob_illidari_fearbringer(Creature *_Creature)
{
    return new mob_illidari_fearbringerAI(_Creature);
}

/* ============================
*
*      SHADE  OF  AKAMA - scripted in Shade of Akama script
*
* ============================*/

/* Content Data
    * Ashtongue Channeler
    * Ashtongue Defender
    * Ashtongue Elementalist
    * Ashtongue Rogue
    * Ashtongue Sorcerer
    * Ashtongue Spiritbinder
    */

/* ============================
*
*      SANCTUARY OF SHADOW (no boss here)
*
* ============================*/

/* Content Data
    * Ashtongue Battlelord
    * Ashtongue Feral Spirit
    * Ashtongue Mystic
    * Ashtongue Primalist
    * Ashtongue Stalker
    * Ashtongue Stormcaller
    * Illidari Boneslicer
    * Illidari Centurion
    * Illidari Defiler
    * Illidari Heartseeker
    * Illidari Nightlord
    * Storm Fury
    */

/****************
* Ashtongue Battlelord - id 22844
*****************/

#define SPELL_CONCUSSION_BLOW   32588
#define SPELL_CONCUSSION_THROW  41182
#define SPELL_FRENZY            34970

struct mob_ashtongue_battlelordAI : public ScriptedAI
{
    mob_ashtongue_battlelordAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Cleave;
    Timer ConcussionBlow;
    Timer ConcussionThrow;
    Timer Frenzy;

    void Reset()
    {
        ClearCastQueue();

        Cleave.Reset(urand(3000, 10000));
        ConcussionBlow.Reset(urand(10500, 25000));
        ConcussionThrow.Reset(urand(10500, 25000));
        Frenzy.Reset(5000);
    }
    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Cleave.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_CLEAVE_1);
            Cleave = 10000;
        }


        if (ConcussionBlow.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_CONCUSSION_BLOW);
            ConcussionBlow = 25000;
        }


        if (ConcussionThrow.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_TOPAGGRO, 1, 100, true))
            {
                AddSpellToCast(target, SPELL_CONCUSSION_THROW);
            }
            ConcussionThrow = 22000;
        }


        if (Frenzy.Expired(diff))
        {
            if (!me->HasAura(SPELL_FRENZY, 0))
                AddSpellToCast(me->getVictim(), SPELL_FRENZY);
            Frenzy = 30000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_ashtongue_battlelord(Creature *_Creature)
{
    return new mob_ashtongue_battlelordAI(_Creature);
}

/****************
* Ashtongue Feral Spirit - id 22849
*****************/

#define SPELL_CHARGE_RAGE       39575
#define SPELL_SPIRIT_BOND       39578

#define NPC_ASHTONGUE_PRIMALIST 22847

struct mob_ashtongue_feral_spiritAI : public ScriptedAI
{
    mob_ashtongue_feral_spiritAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(65.0f); }

    Timer ChargeRage;
    Timer SpiritBond;

    void Reset()
    {
        ClearCastQueue();

        ChargeRage.Reset(urand(10000, 30000));
        SpiritBond.Reset(urand(15000, 25000));
    }
    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (ChargeRage.Expired(diff))
        {
            AddSpellToCast(me, SPELL_CHARGE_RAGE);
            ChargeRage = urand(20000, 30000);
        }


        if (SpiritBond.Expired(diff))
        {
            AddSpellToCast(me, SPELL_SPIRIT_BOND);
            if (Creature* Primalist = GetClosestCreatureWithEntry(me, NPC_ASHTONGUE_PRIMALIST, 40.0f))
                Primalist->CastSpell(Primalist, SPELL_SPIRIT_BOND, true);
            SpiritBond = 30000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_ashtongue_feral_spirit(Creature *_Creature)
{
    return new mob_ashtongue_feral_spiritAI(_Creature);
}

/****************
* Ashtongue Mystic - id 22845 & Ashtongue Mystic Totems - id 22894, 22896, 22897
*****************/

#define SPELL_FROST_SHOCK               41116
#define SPELL_FLAME_SHOCK               41115
#define SPELL_BLOODLUST                 41185
#define SPELL_CHAIN_HEAL                41114
#define SPELL_SEARING_TOTEM             39588
#define SPELL_SUMMON_WINDFURY_TOTEM     39586
#define SPELL_CYCLONE_TOTEM             39589
// Totems
#define NPC_CYCLONE_TOTEM           22894
#define NPC_ASHTONGUE_SEARING_TOTEM 22896
#define NPC_SUMMONED_WINDFURY_TOTEM 22897
#define SPELL_CYCLON                39594
#define SPELL_ATTACK                39593
#define SPELL_WINDFURY_WEAPON       32911   //rank 3 AoE aura
#define AURA_WINDFURY               32912

struct totem_ashtongue_mysticAI : public Scripted_NoMovementAI
{
    totem_ashtongue_mysticAI(Creature *c) : Scripted_NoMovementAI(c) {}

    Timer SpellTimer;
    uint64 SummonerGUID;

    void Reset()
    {
        ClearCastQueue();

        DoZoneInCombat(80.0f);
        switch (me->GetEntry())
        {
            case NPC_CYCLONE_TOTEM:
                me->SetMaxHealth(urand(4100, 4200));
                SpellTimer.Reset(urand(2000, 11000));
                break;
            case NPC_ASHTONGUE_SEARING_TOTEM:
                me->SetMaxHealth(urand(4800, 4900));
                SpellTimer.Reset(1000);
                break;
            case NPC_SUMMONED_WINDFURY_TOTEM:
                me->SetMaxHealth(urand(1800, 1900));
                me->CastSpell((Unit*)NULL, SPELL_WINDFURY_WEAPON, true);
                break;
        }
        me->SetHealth(me->GetMaxHealth());
    }
    void IsSummonedBy(Unit* summoner)
    {
        if (summoner)
            SummonerGUID = summoner->GetGUID();
    }
    void JustDied(Unit* killer)
    {
        if (me->GetEntry() == NPC_SUMMONED_WINDFURY_TOTEM)
        {
            std::list<Creature*> pList;
            Hellground::AllFriendlyCreaturesInGrid u_check(me);
            Hellground::ObjectListSearcher<Creature, Hellground::AllFriendlyCreaturesInGrid> searcher(pList, u_check);
            Cell::VisitAllObjects(me, searcher, 60.0);  // range than of aura, in case mobs moved too far from totem when killed

            if (!pList.empty())
            {
                for (std::list<Creature*>::iterator iter = pList.begin(); iter != pList.end(); ++iter)
                {
                    if ((*iter)->HasAura(SPELL_WINDFURY_WEAPON, 0))
                    {
                        (*iter)->RemoveAurasDueToSpell(SPELL_WINDFURY_WEAPON);
                        (*iter)->RemoveAurasDueToSpell(AURA_WINDFURY);
                    }
                }
            }
        }
    }
    void UpdateAI(const uint32 diff)
    {
        if (SpellTimer.Expired(diff))
        {
            switch (me->GetEntry())
            {
                case NPC_CYCLONE_TOTEM:
                    if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40, true))
                        AddSpellToCast(target, SPELL_CYCLON);
                    SpellTimer = 11000;
                    break;
                case NPC_ASHTONGUE_SEARING_TOTEM:
                    AddSpellToCast(me, SPELL_ATTACK);
                    SpellTimer = 3000;
                    break;
            }
        }


        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_totem_ashtongue_mystic(Creature *_Creature)
{
    return new totem_ashtongue_mysticAI(_Creature);
}

struct mob_ashtongue_mysticAI : public ScriptedAI
{
    mob_ashtongue_mysticAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer FrostShock;
    Timer FlameShock;
    Timer ChainHeal;
    Timer SearingTotem;
    Timer WindfuryTotem;
    Timer CycloneTotem;
    Timer CheckTimer;

    void Reset()
    {
        ClearCastQueue();

        FrostShock.Reset(urand(5000, 22000));
        FlameShock.Reset(urand(10000, 30000));
        ChainHeal.Reset(5000);
        CheckTimer.Reset(2000);
        SearingTotem.Reset(urand(500, 30000));
        WindfuryTotem.Reset(urand(500, 30000));
        CycloneTotem.Reset(urand(500, 30000));
    }
    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
        DoCast(me, SPELL_BLOODLUST);
    }
    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (FrostShock.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 20, true))
                AddSpellToCast(target, SPELL_FROST_SHOCK);
            FrostShock = 22000;
        }


        if (FlameShock.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 20, true))
                AddSpellToCast(target, SPELL_FLAME_SHOCK);
            FlameShock = 30000;
        }


        if (ChainHeal.Expired(diff))
        {
            if (me->GetHealth() * 100 / me->GetMaxHealth() < 70)
            {
                ForceSpellCast(me, SPELL_CHAIN_HEAL);
                ChainHeal = 20000;
            }
            else if (Unit* healTarget = SelectLowestHpFriendly(40, 15000))
            {
                ForceSpellCast(healTarget, SPELL_CHAIN_HEAL);
                ChainHeal = 20000;
            }
            else
                ChainHeal = 3000;
        }


        if (SearingTotem.Expired(diff))
        {
            AddSpellToCast(me, SPELL_SEARING_TOTEM);
            SearingTotem = urand(8000, 15000);
        }


        if (WindfuryTotem.Expired(diff))
        {
            AddSpellToCast(me, SPELL_SUMMON_WINDFURY_TOTEM);
            WindfuryTotem = urand(8000, 15000);
        }


        if (CycloneTotem.Expired(diff))
        {
            AddSpellToCast(me, SPELL_CYCLONE_TOTEM);
            CycloneTotem = urand(8000, 15000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_ashtongue_mystic(Creature *_Creature)
{
    return new mob_ashtongue_mysticAI(_Creature);
}

/****************
* Ashtongue Primalist - id 22847
*****************/

#define SPELL_MULTI_SHOT            41187
#define SPELL_AP_SHOOT              41188
#define SPELL_WYVERN_STING          41186
#define SPELL_SWEEPING_WING_CLIP    39584

struct mob_ashtongue_primalistAI : public ScriptedAI
{
    mob_ashtongue_primalistAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer MultiShot;
    Timer Shoot;
    Timer WyvernSting;
    Timer SweepingWingClip;

    void Reset()
    {
        ClearCastQueue();

        MultiShot.Reset(urand(20000, 40000));
        Shoot.Reset(500);
        WyvernSting.Reset(urand(7000, 15000));
        SweepingWingClip.Reset(urand(20000, 37000));
    }

    void AttackStart(Unit* who)
    {
        ScriptedAI::AttackStartNoMove(who, CHECK_TYPE_SHOOTER);
    }

    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }
    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (MultiShot.Expired(diff))
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100.0f, true);
            if (target && !me->IsWithinDist(target, 5.0f))
            {
                ForceSpellCast(target, SPELL_MULTI_SHOT);
                MultiShot = 40000;
            }
            else
                MultiShot = 3000;
        }


        if (Shoot.Expired(diff))
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30.0f, true);
            if (target && !me->IsWithinDist(target, 5.0f))
                ForceSpellCast(target, SPELL_AP_SHOOT);
            Shoot = 1500;
        }


        if (WyvernSting.Expired(diff))
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 1, 35.0f, true);
            if (target && !me->IsWithinDist(target, 5.0f))
            {
                ForceSpellCast(target, SPELL_WYVERN_STING);
                WyvernSting = 15000;
            }
            else
                WyvernSting = 2000;
        }


        if (SweepingWingClip.Expired(diff))
        {
            if (me->IsWithinDistInMap(me->getVictim(), 5.0))
            {
                AddSpellToCast(me->getVictim(), SPELL_SWEEPING_WING_CLIP);

                float x, y, z;
                me->GetNearPoint(x, y, z, 0.0f, urand(10, 15), frand(0.0f, 2 * M_PI));
                me->GetMotionMaster()->MovePoint(1, x, y, z);
                SweepingWingClip = 37000;
            }
            else
                SweepingWingClip = 2500;
        }


        CheckShooterNoMovementInRange(diff, 30.0);
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_ashtongue_primalist(Creature *_Creature)
{
    return new mob_ashtongue_primalistAI(_Creature);
}

/****************
* Ashtongue Stalker - id 23374
*****************/

#define SPELL_BLIND                 34654
#define SPELL_INSTANT_POISON        41189
#define SPELL_MIND_NUMBING_POISON   41190
#define SPELL_STEALTH               34189
#define SPELL_DUAL_WIELD            29651

struct mob_ashtongue_stalkerAI : public ScriptedAI
{
    mob_ashtongue_stalkerAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Blind;
    Timer InstantPoison;
    Timer MindNumbingPoison;

    void Reset()
    {
        ClearCastQueue();

        DoCast(me, SPELL_STEALTH);
        DoCast(me, SPELL_DUAL_WIELD);
        Blind.Reset(urand(10000, 20000));
        InstantPoison.Reset(urand(5000, 10000));
        MindNumbingPoison.Reset(urand(5000, 10000));
    }
    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Blind.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_BLIND);
            Blind = 20000;
        }


        if (InstantPoison.Expired(diff))
        {
            if (Unit *pTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 0, 60.0f, true, POWER_RAGE))
            {
                AddSpellToCast(pTarget, SPELL_INSTANT_POISON);
                InstantPoison = 10000;
            }
            else if (Unit *pTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 0, 60.0f, true, POWER_ENERGY))
            {
                AddSpellToCast(pTarget, SPELL_INSTANT_POISON);
                InstantPoison = 10000;
            }
            else
                InstantPoison = 5000;
        }


        if (MindNumbingPoison.Expired(diff))
        {
            if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 60.0f, true, POWER_MANA))
            {
                AddSpellToCast(pTarget, SPELL_MIND_NUMBING_POISON);
                MindNumbingPoison = 10000;
            }
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_ashtongue_stalker(Creature *_Creature)
{
    return new mob_ashtongue_stalkerAI(_Creature);
}

/****************
* Ashtongue Stormcaller - id 22846
*****************/

#define SPELL_CHAIN_LIGHTNING           41183
#define SPELL_LIGHTNING_BOLT            41184
#define SPELL_LIGHTNING_SHIELD          41151

struct mob_ashtongue_stormcallerAI : public ScriptedAI
{
    mob_ashtongue_stormcallerAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer ChainLightning;
    Timer LightningBolt;
    Timer LightningShield;

    void Reset()
    {
        ClearCastQueue();

        ChainLightning.Reset(urand(6000, 25000));
        LightningBolt.Reset(urand(1500, 3000));
        LightningShield.Reset(25000);
    }
    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
        DoCast(me, SPELL_LIGHTNING_SHIELD);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (ChainLightning.Expired(diff))
        {
            ForceSpellCast(me->getVictim(), SPELL_CHAIN_LIGHTNING);
            ChainLightning = 10000;
        }


        if (LightningBolt.Expired(diff))
        {
            if (Unit *pTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 0, 60.0f, true, 0, 8.0f))
            {
                DoCast(pTarget, SPELL_LIGHTNING_BOLT);
                LightningBolt = 5000;
            }
        }


        if (LightningShield.Expired(diff))
        {
            DoCast(me, SPELL_LIGHTNING_SHIELD);
            LightningShield = 22000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_ashtongue_stormcaller(Creature *_Creature)
{
    return new mob_ashtongue_stormcallerAI(_Creature);
}

/****************
* Illidari Boneslicer - id 22869
*****************/

#define SPELL_CLOAK_OF_SHADOWS              39666
#define SPELL_GOUGE                         24698
#define SPELL_SHADOWSTEP                    41176
#define SPELL_WOUND_POISON                  39665

struct mob_illidari_boneslicerAI : public ScriptedAI
{
    mob_illidari_boneslicerAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer CloakOfShadows;
    Timer Gouge;
    Timer Shadowstep;
    Timer WoundPoison;

    void Reset()
    {
        ClearCastQueue();

        CloakOfShadows.Reset(15000);
        Gouge.Reset(urand(1000, 10000));;
        Shadowstep.Reset(urand(5000, 15000));
        WoundPoison.Reset(urand(1000, 3000));
    }
    void EnterCombat(Unit*)
    {
        DoCast(me, SPELL_CLOAK_OF_SHADOWS);
        DoZoneInCombat(80.0f);
    }

    void DamageMade(Unit* target, uint32 & damage, bool direct_damage, uint8 school_mask)
    {
        if (damage && direct_damage)
            ForceSpellCast(target, SPELL_WOUND_POISON);
    }
    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (CloakOfShadows.Expired(diff))
        {
            AddSpellToCast(me, SPELL_CLOAK_OF_SHADOWS);
            CloakOfShadows = 15000;
        }


        if (Gouge.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_GOUGE);
            Gouge = 10000;
        }


        if (Shadowstep.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_SHADOWSTEP);
            Shadowstep = 15000;
        }


        if (WoundPoison.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_WOUND_POISON);
            WoundPoison = 3000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_illidari_boneslicer(Creature *_Creature)
{
    return new mob_illidari_boneslicerAI(_Creature);
}

/****************
* Illidari Centurion - id 23337
*****************/

#define SPELL_SONIC_STRIKE          41168

struct mob_illidari_centurionAI : public ScriptedAI
{
    mob_illidari_centurionAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Cleave;
    Timer SonicStrike;

    void Reset()
    {
        ClearCastQueue();

        Cleave.Reset(urand(3000, 10000));
        SonicStrike.Reset(urand(5000, 15000));
    }
    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Cleave.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_CLEAVE_1);
            Cleave = 10000;
        }


        if (SonicStrike.Expired(diff))  //in cone in front of a caster, should "in front" be changed randomly?
        {
            AddSpellToCast(me, SPELL_SONIC_STRIKE);
            SonicStrike = 15000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_illidari_centurion(Creature *_Creature)
{
    return new mob_illidari_centurionAI(_Creature);
}

/****************
* Illidari Defiler - id 22853
*****************/

#define SPELL_FEL_IMMOLATE                  39670
#define SPELL_CURSE_OF_AGONY                39672
#define SPELL_BANISH                        39674
#define SPELL_RAIN_OF_CHAOS                 39671

struct mob_illidari_defilerAI : public ScriptedAI
{
    mob_illidari_defilerAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer FelImmolate;
    Timer CurseOfAgony;
    Timer Banish;
    Timer RainOfChaos;

    void Reset()
    {
        ClearCastQueue();

        FelImmolate.Reset(urand(15000, 25000));
        CurseOfAgony.Reset(urand(20000, 35000));
        Banish.Reset(urand(10000, 30000));
        RainOfChaos.Reset(urand(3000, 15000));
    }
    void EnterCombat(Unit*){ DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (FelImmolate.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_FEL_IMMOLATE);
            FelImmolate = 25000;
        }


        if (CurseOfAgony.Expired(diff))
        {
            AddSpellToCast(me, SPELL_CURSE_OF_AGONY);
            CurseOfAgony = 35000;
        }


        if (Banish.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30, true))
                AddSpellToCast(target, SPELL_BANISH);
            Banish = 30000;
        }


        if (RainOfChaos.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30, true))
                AddSpellToCast(target, SPELL_RAIN_OF_CHAOS);
            RainOfChaos = 15000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_illidari_defiler(Creature *_Creature)
{
    return new mob_illidari_defilerAI(_Creature);
}

/****************
* Illidari Heartseeker - id 23339
*****************/

#define SPELL_RAPID_SHOT                41173
#define SPELL_IH_SHOOT                  41169
#define SPELL_SKELETON_SHOT             41171
#define SPELL_CURSE_OF_THE_BLEAKHEART   6946  //41170 proper spell blocks 6947 proc

struct mob_illidari_heartseekerAI : public ScriptedAI
{
    mob_illidari_heartseekerAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer RapidShot;
    Timer Shoot;
    Timer SkeletonShot;
    Timer Curse;

    void Reset()
    {
        ClearCastQueue();

        RapidShot.Reset(urand(10000, 20000));
        Shoot.Reset(500);
        SkeletonShot.Reset(urand(15000, 35000));
        Curse.Reset(urand(1000, 20000));
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat(80.0f);
    }

    void AttackStart(Unit *who)
    {
        ScriptedAI::AttackStartNoMove(who, CHECK_TYPE_SHOOTER);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Shoot.Expired(diff))
        {
            //check if victim is in melee range, if so, start normal chasing
            if (me->IsWithinDistInMap(me->getVictim(), 8.0) && me->GetMotionMaster()->GetCurrentMovementGeneratorType() == IDLE_MOTION_TYPE)
                DoStartMovement(me->getVictim());

            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30.0, true, 0, 5.0))
            {
                AddSpellToCast(target, SPELL_IH_SHOOT);
                Shoot = 1800;
            }
        }


        if (RapidShot.Expired(diff))
        {
            AddSpellToCast(me, SPELL_RAPID_SHOT);
            RapidShot = 20000;
        }


        if (SkeletonShot.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40, true))
            {
                if (target->IsWithinDistInMap(me, 8.0))
                    SkeletonShot = 1000;
                else
                {
                    AddSpellToCast(target, SPELL_SKELETON_SHOT);
                    SkeletonShot = 35000;
                }
            }
        }


        if (Curse.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))    // workaround...
                target->CastSpell(target, SPELL_CURSE_OF_THE_BLEAKHEART, true, 0, 0, me->GetGUID());
            Curse = 20000;
        }


        CheckShooterNoMovementInRange(diff, 25.0);
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_illidari_heartseeker(Creature *_Creature)
{
    return new mob_illidari_heartseekerAI(_Creature);
}

/****************
* Illidari Nightlord - id 22855
*****************/

#define SPELL_CURSE_OF_MENDING          39647
#define SPELL_FEAR                      41150
#define SPELL_SHADOW_INFERNO            39645
#define SPELL_SUMMON_SHADOWFIENDS       39649

struct mob_illidari_nightlordAI : public ScriptedAI
{
    mob_illidari_nightlordAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer CurseOfMending;
    Timer Fear;
    Timer ShadowInferno;
    Timer Shadowfiends;

    void Reset()
    {
        ClearCastQueue();

        CurseOfMending.Reset(urand(10000, 20000));
        Fear.Reset(urand(3000, 15000));
        ShadowInferno.Reset(7000);
        Shadowfiends.Reset(urand(15000, 20000));
    }
    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (CurseOfMending.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 10, true))
                ForceSpellCast(target, SPELL_CURSE_OF_MENDING);
            CurseOfMending = 20000;
        }


        if (Fear.Expired(diff))
        {
            ForceSpellCast(me, SPELL_FEAR);
            Fear = urand(10000, 15000);
        }


        if (ShadowInferno.Expired(diff))
        {
            if (Fear.GetTimeLeft() < 8200)
                Fear.Delay(8000);

            if (CurseOfMending.GetTimeLeft() < 8200)
                CurseOfMending.Delay(8000);

            if (Shadowfiends.GetTimeLeft() < 8200)
                Shadowfiends.Delay(8000);

            ForceSpellCast(me, SPELL_SHADOW_INFERNO);
            ShadowInferno = 20000;
        }


        if (Shadowfiends.Expired(diff))
        {
            ForceSpellCast(me, SPELL_SUMMON_SHADOWFIENDS);
            Shadowfiends = 25000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_illidari_nightlord(Creature *_Creature)
{
    return new mob_illidari_nightlordAI(_Creature);
}

/****************
* Storm Fury - id 22848
*****************/

#define SPELL_STORM_BLINK           39581

struct mob_storm_furyAI : public ScriptedAI
{
    mob_storm_furyAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer StormBlink;

    void Reset()
    {
        ClearCastQueue();

        StormBlink.Reset(urand(15000, 25000));
    }
    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (StormBlink.Expired(diff))
        {
            AddSpellToCast(me, SPELL_STORM_BLINK);
            StormBlink = 25000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_storm_fury(Creature *_Creature)
{
    return new mob_storm_furyAI(_Creature);
}

/* ============================
*
*      TERON  GOREFIEND
*
* ============================*/

/* Content Data
    * Hand of Gorefiend
    * Shadowmoon Blood Mage
    * Shadowmoon Champion
    * Shadowmoon Deathshaper
    * Shadowmoon Grunt - no special AI only LoS aggro
    * Shadowmoon Houndmaster
    * Shadowmoon Reaver
    * Shadowmoon Riding Hound
    * Shadowmoon Soldier
    * Shadowmoon Weapon Master
    * Wrathbone Flayer
    */

/****************
* Hand of Gorefiend - id 23172
*****************/

#define SPELL_ENRAGE                38166

struct mob_hand_of_gorefiendAI : public ScriptedAI
{
    mob_hand_of_gorefiendAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Enrage;

    void Reset()
    {
        Enrage.Reset(10000);
    }
    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Enrage.Expired(diff))
        {
            DoCast(me, SPELL_ENRAGE);
            Enrage = 30000;
        }


        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_hand_of_gorefiend(Creature *_Creature)
{
    return new mob_hand_of_gorefiendAI(_Creature);
}

/****************
* Shadowmoon Blood Mage - id 22945
*****************/

#define SPELL_BLOOD_SIPHON              41068
#define SPELL_BLOODBOLT                 41072

// zle id ale wyglada podobnie
#define SPELL_GREEN_BEAM        38909
#define MOB_SKELETON            22953

struct mob_shadowmoon_blood_mageAI : public ScriptedAI
{
    mob_shadowmoon_blood_mageAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer BloodSiphon;
    Timer Bloodbolt;

    void Reset()
    {
        ClearCastQueue();

        BloodSiphon.Reset(urand(3000, 20000));
        Bloodbolt.Reset(urand(5000, 15000));
    }
    void EnterCombat(Unit *)
    {
        me->InterruptNonMeleeSpells(false, SPELL_GREEN_BEAM);
        DoZoneInCombat(80.0f);
    }

    void SpellHitTarget(Unit* target, const SpellEntry*)    //workaround
    {
        if (target->HasAura(41229, 1))
            target->RemoveAura(41229, 1);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (!me->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT))
            {
                if (Unit *skeleton = FindCreature(MOB_SKELETON, 20.0f, me))
                {
                    if (skeleton->isAlive())
                        DoCast(skeleton, SPELL_GREEN_BEAM);
                }
            }
            return;
        }

        if (BloodSiphon.Expired(diff))
        {
            AddSpellToCast(me, SPELL_BLOOD_SIPHON);
            BloodSiphon = 20000;
        }


        if (Bloodbolt.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_TOPAGGRO, 0, 40.0, true))
                ForceSpellCast(target, SPELL_BLOODBOLT);
            Bloodbolt = 15000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_shadowmoon_blood_mage(Creature *_Creature)
{
    return new mob_shadowmoon_blood_mageAI(_Creature);
}

/****************
* Shadowmoon Champion - id 22880 and Whirling Blade - id 23369
*****************/

#define SPELL_CHAOTIC_LIGHT                 41063
#define SPELL_WHIRLING_BLADE                41053
#define SPELL_WHIRLWIND                     41058

#define MOB_WHIRLING_BLADE                  23369

struct mob_shadowmoon_championAI : public ScriptedAI
{
    mob_shadowmoon_championAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer ChaoticLight;
    Timer WhirlingBlade;

    void Reset()
    {
        ClearCastQueue();

        ChaoticLight.Reset(urand(2000, 5000));
        WhirlingBlade = 0;
    }

    void EnterCombat(Unit *) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (ChaoticLight.Expired(diff))
        {
            AddSpellToCast(me, SPELL_CHAOTIC_LIGHT);
            ChaoticLight = urand(5000, 10000);
        }


        if (WhirlingBlade.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60.0, true))
                ForceSpellCast(target, SPELL_WHIRLING_BLADE);
            WhirlingBlade = 30000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_shadowmoon_champion(Creature *_Creature)
{
    return new mob_shadowmoon_championAI(_Creature);
}

struct mob_whirling_bladeAI : public Scripted_NoMovementAI
{
    mob_whirling_bladeAI(Creature *c) : Scripted_NoMovementAI(c) { }

    Timer Whirl;
    Timer DieTimer;

    void Reset()
    {
        ClearCastQueue();

        DoCast(me, SPELL_WHIRLWIND, true);
        Whirl.Reset(6000);
        DieTimer.Reset(14000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (Whirl.Expired(diff))
        {
            AddSpellToCast(me, SPELL_WHIRLWIND);
            Whirl = 6000;
        }

        if (DieTimer.Expired(diff))
        {
            me->Kill(me, false);
            me->RemoveCorpse();
            DieTimer = 2000;
        }


        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_mob_whirling_blade(Creature *_Creature)
{
    return new mob_whirling_bladeAI(_Creature);
}

/****************
* Shadowmoon Deathshaper - id 22882
*****************/

#define SPELL_DEATH_COIL                41070
#define SPELL_DEMON_ARMOR               13787
#define SPELL_RAISE_DEAD                41071
#define SPELL_SHADOWBOLT                41069

struct mob_shadowmoon_deathshaperAI : public ScriptedAI
{
    mob_shadowmoon_deathshaperAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Shadowbolt;
    Timer DeathCoil;
    Timer DemonArmor;
    Timer RaiseDeadCheck;

    std::list<uint64> UsedCorpsesGUID;

    void Reset()
    {
        ClearCastQueue();

        Shadowbolt.Reset(urand(500, 5000));
        DeathCoil.Reset(urand(5000, 20000));
        DemonArmor.Reset(1800000);
        RaiseDeadCheck.Reset(10000);

        UsedCorpsesGUID.clear();
    }

    void EnterCombat(Unit *)
    {
        me->InterruptNonMeleeSpells(false, SPELL_GREEN_BEAM);
        DoCast(me, SPELL_DEMON_ARMOR);
        DoZoneInCombat(80.0f);
    }

    uint64 SelectCorpseGUID()
    {
        std::list<Unit*> DeadList = FindAllDeadInRange(50);
        std::list<uint64> CorpseGUID;
        CorpseGUID.clear();

        // select all dead units GUIDs
        if (!DeadList.empty())
        {
            for (std::list<Unit*>::iterator i = DeadList.begin(); i != DeadList.end(); ++i)
            {
                CorpseGUID.push_back((*i)->GetGUID());  // this may be moved to searcher later on
            }
        }
        // remove GUIDs of once used corpses
        if (!CorpseGUID.empty())
        {
            for (std::list<uint64>::iterator i = CorpseGUID.begin(); i != CorpseGUID.end(); ++i)
            {
                if (!UsedCorpsesGUID.empty())
                {
                    for (std::list<uint64>::iterator iter = UsedCorpsesGUID.begin(); iter != UsedCorpsesGUID.end(); ++iter)
                    {
                        if ((*iter) == (*i))
                        {
                            CorpseGUID.remove((*iter));
                            if (CorpseGUID.size())
                            {
                                i = CorpseGUID.begin();
                                iter = UsedCorpsesGUID.begin();
                            }
                            else
                                break;
                        }
                    }
                }
                break;
            }
        }
        // now get random GUID
        if (!CorpseGUID.empty())
        {
            std::list<uint64>::iterator i = CorpseGUID.begin();
            advance(i, rand() % CorpseGUID.size());
            return (*i);
        }
        return 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (!me->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT))
            {
                if (Unit *skeleton = FindCreature(MOB_SKELETON, 20.0f, me))
                {
                    if (skeleton->isAlive())
                        DoCast(skeleton, SPELL_GREEN_BEAM);
                }
            }
            return;
        }

        if (Shadowbolt.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_TOPAGGRO, 0, 40.0, true))
                AddSpellToCast(target, SPELL_SHADOWBOLT);
            Shadowbolt = 5000;
        }


        if (DeathCoil.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30.0, true))
                AddSpellToCast(target, SPELL_DEATH_COIL);
            DeathCoil = 20000;
        }


        if (RaiseDeadCheck.Expired(diff))
        {
            if (uint64 targetGUID = SelectCorpseGUID())
            {
                if (Unit* target = Unit::GetUnit(*me, targetGUID))
                {
                    AddSpellToCast(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), SPELL_RAISE_DEAD, false);
                    UsedCorpsesGUID.push_back(targetGUID);
                }
            }
            RaiseDeadCheck = 10000;
        }


        if (DemonArmor.Expired(diff))
        {
            ForceSpellCast(me, SPELL_DEMON_ARMOR);
            DemonArmor = 1800000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_shadowmoon_deathshaper(Creature *_Creature)
{
    return new mob_shadowmoon_deathshaperAI(_Creature);
}

/****************
* Shadowmoon Grunt - id 23147
*****************/

struct mob_shadowmoon_gruntAI : public ScriptedAI
{
    mob_shadowmoon_gruntAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    void Reset() { }
    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_shadowmoon_grunt(Creature *_Creature)
{
    return new mob_shadowmoon_gruntAI(_Creature);
}

/****************
* Shadowmoon Houndmaster - id 23018
*****************/

#define SPELL_FREEZING_TRAP                 41085
#define SPELL_SHOOT_1                       41093
#define SPELL_SILENCING_SHOT                41084
#define SPELL_SUMMON_RIDING_WARHOUND        39906
#define SPELL_VOLLEY                        41091
#define SPELL_WING_CLIP                     32908
#define SPELL_FLARE                         41094

#define MOB_SHADOWMOON_RIDING_HOUND         23083

struct mob_shadowmoon_houndmasterAI : public ScriptedAI
{
    mob_shadowmoon_houndmasterAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Shoot;
    Timer FreezingTrap;
    Timer SilencingShot;
    Timer Volley;
    Timer WingClip;
    Timer Flare;
    Timer RangeCheck;

    void Reset()
    {
        ClearCastQueue();

        if (Creature* Hound = GetClosestCreatureWithEntry(me, MOB_SHADOWMOON_RIDING_HOUND, 80))
            Hound->DisappearAndDie();
        me->Mount(14334);
        me->UpdateVisibilityAndView();
        me->UpdateObjectVisibility();          // works nice for players; though still visual bug when: .die -> .respawn (need to get out of vision to see him mounted)
        me->GetMotionMaster()->Initialize();   // start moving in waypoints after exiting combat
        FreezingTrap.Reset(15000);
        SilencingShot.Reset(urand(5000, 15000));
        Volley.Reset(urand(10000, 25000));
        WingClip.Reset(urand(8000, 20000));
        Flare.Reset(urand(2000, 20000));
        RangeCheck.Reset(1000);
    }

    void EnterCombat(Unit * who)
    {
        me->Unmount();
        DoCast(me, SPELL_SUMMON_RIDING_WARHOUND);
        DoCast(me, SPELL_FREEZING_TRAP);
        DoZoneInCombat(80.0f);
        DoStartMovement(who);
    }


    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (RangeCheck.Expired(diff))
        {
            Unit* victim = me->getVictim();
            if (me->IsInRange(victim, 5.0f, 30.0f))
                DoStartNoMovement(victim, CHECK_TYPE_SHOOTER);
            else
                if (!me->IsInRange(victim, 5.0f, 30.0f))
                    DoStartMovement(victim);
            RangeCheck = 2000;
        }



        if (FreezingTrap.Expired(diff))
        {
            AddSpellToCast(SPELL_FREEZING_TRAP, CAST_NULL);
            FreezingTrap = 15000;
        }


        if (Volley.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40.0f, true, 0.0f, 10.0f))
            {
                AddSpellToCast(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), SPELL_VOLLEY);
                Volley = 25000;
            }
            else
                Volley = 3000;
        }


        if (Shoot.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_TOPAGGRO, 0, 100, true))
                AddSpellToCast(target, SPELL_SHOOT_1);

            Shoot = 2000;
        }


        if (SilencingShot.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 1, 35.0f, true, 0, 8.0f))
            {
                ForceSpellCast(target, SPELL_SILENCING_SHOT);
                SilencingShot = 15000;
            }
            else
                SilencingShot = 4000;
        }


        if (WingClip.Expired(diff))
        {
            if (me->IsWithinDistInMap(me->getVictim(), 5.0))
            {
                AddSpellToCast(me->getVictim(), SPELL_WING_CLIP);
                WingClip = 20000;
            }
            else
                WingClip = 2500;
        }


        if (Flare.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                AddSpellToCast(target, SPELL_FLARE);
            Flare = 20000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_shadowmoon_houndmaster(Creature *_Creature)
{
    return new mob_shadowmoon_houndmasterAI(_Creature);
}

/****************
* Shadowmoon Reaver - id 22879
*****************/

#define SPELL_SPELL_ABSORPTION              41034
#define SPELL_SHADOW_RESONANCE              41047

struct mob_shadowmoon_reaverAI : public ScriptedAI
{
    mob_shadowmoon_reaverAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer SpellAbsorption;
    Timer ShadowResonance;

    void Reset()
    {
        ClearCastQueue();

        SpellAbsorption.Reset(10000);
        ShadowResonance.Reset(urand(5000, 20000));
    }
    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (SpellAbsorption.Expired(diff))
        {
            AddSpellToCast(me, SPELL_SPELL_ABSORPTION);
            SpellAbsorption = 40000;
        }


        if (ShadowResonance.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 10.0f, true))
                AddSpellToCast(target, SPELL_SHADOW_RESONANCE);
            ShadowResonance = 20000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_shadowmoon_reaver(Creature *_Creature)
{
    return new mob_shadowmoon_reaverAI(_Creature);
}

/****************
* Shadowmoon Riding Hound - id 23083
*****************/

#define SPELL_CARNIVOROUS_BITE              41092
#define SPELL_CHARGE                        25821
#define SPELL_ENRAGE_1                      8599
#define MOB_SHADOWMOON_HOUNDMASTER          23018

struct mob_shadowmoon_riding_houndAI : public ScriptedAI
{
    mob_shadowmoon_riding_houndAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Charge;
    Timer Enrage;

    void Reset()
    {
        ClearCastQueue();

        Charge.Reset(urand(5000, 20000));
        Enrage.Reset(15000);
    }

    void JustRespawned()
    {
        if (Unit* hunter = GetClosestCreatureWithEntry(me, MOB_SHADOWMOON_HOUNDMASTER, 30.0f))
            AttackStart(hunter->getVictim());
    }

    void DamageMade(Unit* target, uint32 &damage, bool direct_damage, uint8 school_mask)
    {
        if (damage)
        {
            if (target->HasAura(SPELL_CARNIVOROUS_BITE, 0))   //do not stack again when 5 stacks and still more than 6 seconds of DoT to deal spell damage
            {
                Aura* Aur = target->GetAura(SPELL_CARNIVOROUS_BITE, 0);
                if (Aur && Aur->GetStackAmount() == 5 && Aur->GetAuraDuration() > 6000)
                {
                    Aur->UpdateAuraDuration();
                    return;
                }
            }
            ForceSpellCast(target, SPELL_CARNIVOROUS_BITE);
        }
    }
    void EnterCombat(Unit *) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Charge.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                AddSpellToCast(target, SPELL_CHARGE);
            Charge = 20000;
        }


        if (Enrage.Expired(diff))
        {
            AddSpellToCast(me, SPELL_ENRAGE_1);
            Enrage = 30000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_shadowmoon_riding_hound(Creature *_Creature)
{
    return new mob_shadowmoon_riding_houndAI(_Creature);
}

/****************
* Shadowmoon Soldier - id 23047
*****************/

#define SPELL_STRIKE                43298

struct mob_shadowmoon_soldierAI : public ScriptedAI
{
    mob_shadowmoon_soldierAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Strike;

    void Reset()
    {
        ClearCastQueue();

        Strike.Reset(urand(3000, 8000));
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type != POINT_MOTION_TYPE)
            return;

        switch (id)
        {
            case 0:
                me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
                break;
        }
    }

    void EnterCombat(Unit *) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Strike.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_STRIKE);
            Strike = urand(3000, 8000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_shadowmoon_soldier(Creature *_Creature)
{
    return new mob_shadowmoon_soldierAI(_Creature);
}

/****************
* Shadowmoon Weapon Master - id 23049
*****************/

static float fieldPositions[8][2] =
{
    { 448.26f, 195.74f },
    { 448.66f, 191.27f },
    { 449.34f, 187.12f },
    { 449.50f, 182.92f },
    { 442.48f, 195.18f },
    { 442.90f, 191.00f },
    { 443.34f, 186.51f },
    { 443.84f, 182.04f }
};

#define DISTANCE_TO_MOVE   40.00f
#define _HEIGHT           163.98f

#define WEAPON_X          454.35f
#define WEAPON_Y          190.09f

#define SPELL_BATTLE_AURA               41106
#define SPELL_BATTLE_STANCE             41099
#define SPELL_BERSERKER_AURA            41107
#define SPELL_BERSERKER_STANCE          41100
#define SPELL_DEFENSIVE_AURA            41105
#define SPELL_DEFENSIVE_STANCE          41101
#define SPELL_KNOCK_AWAY                18813
#define SPELL_MUTILATE                  41103
#define SPELL_SWM_SHIELD_WALL           41104
#define SPELL_WHIRLWIND_1               41097

#define AXE_MODEL                       45073
#define AXE_INFO                        33488898

#define BATTLE_STANCE_YELL  "Berserker stance! Attack them recklessly!"
#define DEFENSIVE_STANCE_YELL  "Defensive Stance! Shield yourself against their blows and strike back!"

enum Stances
{
    DEFENSIVE = 1,
    BERSERKER,
    BATTLE
};

struct mob_shadowmoon_weapon_masterAI : public ScriptedAI
{
    mob_shadowmoon_weapon_masterAI(Creature *c) : ScriptedAI(c)
    {
        m_nextMove = 0;
        m_nextId = -1;
        pInstance = (ScriptedInstance*)c->GetInstanceData();
        Reset();
    }

    ScriptedInstance *pInstance;
    std::vector<uint64> soldiersList;

    Timer m_nextMove;
    int32  m_nextId;
    uint8  Stance;

    Timer SpecialTimer;
    Timer KnockAway;

    void Reset()
    {
        ClearCastQueue();

        me->LoadEquipment(484, true);
        Stance = DEFENSIVE;
        KnockAway.Reset(urand(3000, 20000));
        SpecialTimer = 0;

        if (!soldiersList.empty())
        {
            m_nextMove.Reset(2000);
            m_nextId = 1;
        }
    }

    void SetWeaponModelAndDamage(uint8 Stance)
    {
        const CreatureInfo *cinfo = me->GetCreatureInfo();
        switch (Stance)
        {
            case DEFENSIVE:
                //main hand: 1h axe, off-hand: shield
                me->LoadEquipment(439, true);
                me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, cinfo->mindmg);
                me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, cinfo->maxdmg);
                me->UpdateDamagePhysical(BASE_ATTACK);
                break;
            case BERSERKER:
                // 2h sword
                me->LoadEquipment(484, true);
                me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, cinfo->mindmg);
                me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, cinfo->maxdmg);
                me->UpdateDamagePhysical(BASE_ATTACK);
                break;
            case BATTLE:
                //main hand: 1h axe
                me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, AXE_MODEL);
                me->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO, AXE_INFO);
                // off-hand: 1h axe
                me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 1, AXE_MODEL);
                me->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO + 2, AXE_INFO);

                me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, cinfo->mindmg);
                me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, cinfo->maxdmg);
                me->UpdateDamagePhysical(BASE_ATTACK);
                me->SetBaseWeaponDamage(OFF_ATTACK, MINDAMAGE, cinfo->mindmg);
                me->SetBaseWeaponDamage(OFF_ATTACK, MAXDAMAGE, cinfo->maxdmg);
                me->SetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, cinfo->mindmg);
                me->SetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, cinfo->maxdmg);
                me->SetAttackTime(OFF_ATTACK, (me->GetAttackTime(BASE_ATTACK) * 150) / 100);
                break;
            default:
                return;
        }
    }

    void DamageTaken(Unit* who, uint32& damage)
    {
        if (damage)
        {
            if (me->GetHealth() * 100 / me->GetMaxHealth() < 80 && Stance == DEFENSIVE)
            {
                me->RemoveAurasDueToSpell(SPELL_DEFENSIVE_STANCE);
                me->RemoveAurasDueToSpell(SPELL_DEFENSIVE_AURA);
                DoYell(BATTLE_STANCE_YELL, 0, me);
                if (me->HasAura(SPELL_SWM_SHIELD_WALL, 0))
                    me->RemoveAurasDueToSpell(SPELL_SWM_SHIELD_WALL);
                ForceSpellCast(me, SPELL_BERSERKER_STANCE, INTERRUPT_AND_CAST_INSTANTLY);
                ForceSpellCast(me, SPELL_BERSERKER_AURA, INTERRUPT_AND_CAST_INSTANTLY);
                Stance = BERSERKER;
                SetWeaponModelAndDamage(Stance);
                SpecialTimer = 0;
            }
            if (me->GetHealth() * 100 / me->GetMaxHealth() < 35 && Stance == BERSERKER)
            {
                me->RemoveAurasDueToSpell(SPELL_BERSERKER_STANCE);
                me->RemoveAurasDueToSpell(SPELL_BERSERKER_AURA);
                if (me->HasAura(SPELL_WHIRLWIND_1, 0))
                    me->RemoveAurasDueToSpell(SPELL_WHIRLWIND_1);
                ForceSpellCast(me, SPELL_BATTLE_STANCE, INTERRUPT_AND_CAST_INSTANTLY);
                ForceSpellCast(me, SPELL_BATTLE_AURA, INTERRUPT_AND_CAST_INSTANTLY);
                Stance = BATTLE;
                SetWeaponModelAndDamage(Stance);
                SpecialTimer = 0;
            }
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type != POINT_MOTION_TYPE)
            return;

        switch (id)
        {
            case 0:
            {
                m_nextMove = 2300;
                m_nextId = 1;
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_POINT_NOSHEATHE);
            }
            break;
            case 1:
            {
                m_nextMove = 5000;
                m_nextId = 0;
            }
            break;
            default:   break;
        }
    }

    void EnterCombat(Unit *)
    {
        DoZoneInCombat(80.0f);
        ForceSpellCast(me, SPELL_DEFENSIVE_STANCE, INTERRUPT_AND_CAST_INSTANTLY);
        ForceSpellCast(me, SPELL_DEFENSIVE_AURA, INTERRUPT_AND_CAST_INSTANTLY);
        DoYell(DEFENSIVE_STANCE_YELL, 0, me);
        me->SetArmor(me->GetArmor()*2.5);   // arbitrary
        SetWeaponModelAndDamage(Stance);
    }

    void UpdateAI(const uint32 diff)
    {
        if (soldiersList.empty())
        {
            if (pInstance)
            {
                if (pInstance->GetData(DATA_WEAPONMASTER_LIST_SIZE) == 8)
                {
                    for (int i = DATA_WEAPONMASTER_SOLDIER; i < DATA_WEAPONMASTER_SOLDIER + 8; ++i)
                    {
                        soldiersList.push_back(pInstance->GetData64(i));
                        if (Creature *soldier = me->GetMap()->GetCreature(pInstance->GetData64(i)))
                        {
                            soldier->Relocate(fieldPositions[i - DATA_WEAPONMASTER_SOLDIER][0], fieldPositions[i - DATA_WEAPONMASTER_SOLDIER][1], _HEIGHT, 0);
                            soldier->SetHomePosition(fieldPositions[i - DATA_WEAPONMASTER_SOLDIER][0], fieldPositions[i - DATA_WEAPONMASTER_SOLDIER][1], _HEIGHT, 0);
                        }
                    }
                    m_nextMove = 1000;
                    m_nextId = 0;
                }
            }
        }

        if (!UpdateVictim())
        {
            if (m_nextId >= 0)
            {

                if (m_nextMove.Expired(diff))
                {
                    m_nextMove = 0;
                    int id = m_nextId;
                    m_nextId = -1;

                    float x = 0;
                    float y = 0;

                    switch (id)
                    {
                        case 0:
                        {
                            for (int i = 0; i < 8; ++i)
                            {
                                if (Unit *soldier = me->GetUnit(*me, soldiersList[i]))
                                {
                                    x = soldier->GetPositionX() + DISTANCE_TO_MOVE * cos(0.0f);
                                    y = soldier->GetPositionY() + DISTANCE_TO_MOVE * sin(0.0f);
                                    soldier->GetMotionMaster()->MovePoint(0, x, y, _HEIGHT);
                                }
                            }
                            x = me->GetPositionX() + DISTANCE_TO_MOVE * cos(0.0f);
                            y = me->GetPositionY() + DISTANCE_TO_MOVE * sin(0.0f);
                        }
                        break;
                        case 1:
                        {
                            me->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
                            for (int i = 0; i < 8; ++i)
                            {
                                if (Unit *soldier = me->GetUnit(*me, soldiersList[i]))
                                    soldier->GetMotionMaster()->MovePoint(1, fieldPositions[i][0], fieldPositions[i][1], _HEIGHT);
                            }
                            x = WEAPON_X;
                            y = WEAPON_Y;
                        }
                        break;
                    }
                    me->GetMotionMaster()->MovePoint(id, x, y, _HEIGHT);

                }
            }
            return;
        }

        if (SpecialTimer.Expired(diff))
        {
            switch (Stance)
            {
                case DEFENSIVE:
                    AddSpellToCast(me, SPELL_SWM_SHIELD_WALL);
                    SpecialTimer = 40000;
                    break;
                case BERSERKER:
                    AddSpellToCast(me, SPELL_WHIRLWIND_1);
                    SpecialTimer = 25000;
                    break;
                case BATTLE:
                    AddSpellToCast(me->getVictim(), SPELL_MUTILATE);
                    SpecialTimer = 25000;
                    break;
            }
        }


        if (KnockAway.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_KNOCK_AWAY);
            KnockAway = 20000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_shadowmoon_weapon_master(Creature *_Creature)
{
    return new mob_shadowmoon_weapon_masterAI(_Creature);
}

/****************
* Wrathbone Flayer - id 22953
*****************/

#define SPELL_CLEAVE_2                      15496
#define SPELL_IGNORED                       39544

struct mob_wrathbone_flayerAI : public ScriptedAI
{
    mob_wrathbone_flayerAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Cleave;
    Timer Ignored;

    void Reset()
    {
        ClearCastQueue();

        Cleave.Reset(1500);
        Ignored.Reset(urand(3000, 10000));
    }

    void EnterCombat(Unit *) { DoZoneInCombat(80.0f); }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Cleave.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_CLEAVE_2);
            Cleave = 3000;
        }


        if (Ignored.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_IGNORED);
            Ignored = 10000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_wrathbone_flayer(Creature *_Creature)
{
    return new mob_wrathbone_flayerAI(_Creature);
}

/* ============================
*
*      GURTOGG  BLOODBOIL
*
* ============================*/

/* Content Data
    * Bonechewer Behemoth
    * Bonechewer Shield Disciple
    * Bonechewer Blade Fury
    * Bonechewer Blood Prophet
    * Mutant War Hound
    * Bonechewer Brawler
    * Bonechewer Combatant
    * Bonechewer Spectator
    */

/****************
* Bonechewer Behemoth - id 23196
*****************/

#define SPELL_BEHEMOTH_CHARGE                   41272
#define SPELL_FEL_STOMP                         41274
#define SPELL_FIERY_COMET                       41277
#define SPELL_FRENZY_1                          8269
#define SPELL_METEOR                            41276

enum Type
{
    MELEE_TYPE = 0,
    RANGED_TYPE
};

struct mob_bonechewer_behemothAI : public ScriptedAI
{
    mob_bonechewer_behemothAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint8 Type;
    Timer SpellTimer;
    Timer Frenzy;


    void Reset()
    {
        ClearCastQueue();

        Type = RAND(MELEE_TYPE, RANGED_TYPE);
        SpellTimer.Reset(Type ? urand(5000, 20000) : urand(3000, 15000));
        Frenzy.Reset(35000);
    }

    void EnterCombat(Unit *)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (SpellTimer.Expired(diff))
        {
            uint32 SpellId = 0;
            switch (Type)
            {
                case MELEE_TYPE:
                {
                    SpellId = RAND(SPELL_BEHEMOTH_CHARGE, SPELL_FEL_STOMP);
                    switch (SpellId)
                    {
                        case SPELL_BEHEMOTH_CHARGE:
                            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30.0f, true, 0, 5.0f))
                                AddSpellToCast(target, SPELL_BEHEMOTH_CHARGE);
                            else
                                SpellTimer = 2000;
                            break;
                        case SPELL_FEL_STOMP:
                            AddSpellToCast(me, SPELL_FEL_STOMP);
                            break;
                    }
                    break;
                }
                case RANGED_TYPE:
                {
                    SpellId = RAND(SPELL_FIERY_COMET, SPELL_METEOR);
                    switch (SpellId)
                    {
                        case SPELL_FIERY_COMET:
                            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40.0f, true, 0, 10.0f))
                                AddSpellToCast(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), SPELL_FIERY_COMET);
                            else
                                SpellTimer = 3000;
                            break;
                        case SPELL_METEOR:
                            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 120.0f, true))
                                AddSpellToCast(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), SPELL_METEOR);
                            else
                                SpellTimer = 2500;
                            break;
                    }
                    break;
                }
            }
            AddSpellToCast(me->getVictim(), SPELL_CLEAVE_2);
            SpellTimer = Type ? 20000 : 15000;
        }


        if (Frenzy.Expired(diff))
        {
            if (!me->HasAura(SPELL_FRENZY_1, 0))
                AddSpellToCast(me, SPELL_FRENZY_1);
            Frenzy = 10000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_bonechewer_behemoth(Creature *_Creature)
{
    return new mob_bonechewer_behemothAI(_Creature);
}

/****************
* Bonechewer Shield Disciple - id 23236
*****************/

#define SPELL_INTERVENE                 41198
#define SPELL_SHIELD_BASH               41197
#define SPELL_BSD_SHIELD_WALL           41196
#define SPELL_THROW_SHIELD              41213

#define MOB_BONECHEWER_BLADE_FURY       23235

struct mob_bonechewer_shield_discipleAI : public ScriptedAI
{
    mob_bonechewer_shield_discipleAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Intervene;
    Timer ShieldBash;
    Timer ThrowShield;


    void Reset()
    {
        ClearCastQueue();

        Intervene.Reset(urand(15000, 20000));
        ShieldBash.Reset(urand(4000, 15000));
        ThrowShield.Reset(urand(7000, 17000));
    }

    void EnterCombat(Unit *) { DoZoneInCombat(80.0f); }

    void DamageTaken(Unit* who, uint32& damage)
    {
        if (damage)
        {
            if (me->GetHealth() * 100 / me->GetMaxHealth() < 30 && !me->HasAura(SPELL_BSD_SHIELD_WALL, 0))
                ForceSpellCast(me, SPELL_BSD_SHIELD_WALL);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Intervene.Expired(diff))
        {
            Unit* InterveneTarget = GetClosestCreatureWithEntry(me, MOB_BONECHEWER_BLADE_FURY, 25.0f);
            if (InterveneTarget && !me->IsWithinDistInMap(InterveneTarget, 8.0) && InterveneTarget->isInCombat())
            {
                ForceSpellCast(InterveneTarget, SPELL_INTERVENE);
                Intervene = 20000;
            }
            else
                Intervene = 3000;
        }


        if (ShieldBash.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_SHIELD_BASH);
            ShieldBash = 15000;
        }


        if (ThrowShield.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_TOPAGGRO, 1, 45.0f, true, 0, 5.0f))
                AddSpellToCast(target, SPELL_THROW_SHIELD);
            ThrowShield = 17000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_bonechewer_shield_disciple(Creature *_Creature)
{
    return new mob_bonechewer_shield_discipleAI(_Creature);
}

/****************
* Bonechewer Blade Fury - id 23235
*****************/

#define SPELL_WHIRLWIND_2               41194

uint32 CCMechanics[8] =
{
    MECHANIC_FEAR,
    MECHANIC_ROOT,
    MECHANIC_SNARE,
    MECHANIC_STUN,
    MECHANIC_FREEZE,
    MECHANIC_KNOCKOUT,
    MECHANIC_POLYMORPH,
    MECHANIC_HORROR
};

struct mob_bonechewer_blade_furyAI : public ScriptedAI
{
    mob_bonechewer_blade_furyAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Whirlwind;
    Timer MoveTimer;

    void Reset()
    {
        ClearCastQueue();

        Whirlwind.Reset(urand(5000, 15000));
        MoveTimer = 0;
    }

    void EnterCombat(Unit *) { DoZoneInCombat(80.0f); }

    void SetCCImmunity(bool apply)
    {
        for (uint8 i = 0; i < 8; ++i)
            me->ApplySpellImmune(i, IMMUNITY_MECHANIC, CCMechanics[i], apply);
    }

    void OnAuraRemove(Aura* aur, bool stackRemove)
    {
        if (aur->GetId() == SPELL_WHIRLWIND_2)
        {
            SetCCImmunity(false);
            me->Relocate(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
            DoStartMovement(me->getVictim());
            MoveTimer = 0;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Whirlwind.Expired(diff))
        {
            AddSpellToCast(me, SPELL_WHIRLWIND_2);
            Whirlwind = 15000;
        }


        if (me->HasAura(SPELL_WHIRLWIND_2, 0))
        {
            if (MoveTimer.Expired(diff))
            {
                SetCCImmunity(true);
                float x, y, z = 0;
                me->Relocate(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                me->GetGroundPointAroundUnit(x, y, z, 10.0, 3.14*RAND(0, 1 / 6, 2 / 6, 3 / 6, 4 / 6, 5 / 6, 1));
                me->GetMotionMaster()->Clear(false);
                me->GetMotionMaster()->MovePoint(0, x, y, z);
                MoveTimer = 2500;
            }
        }
        else
            DoMeleeAttackIfReady();

        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_mob_bonechewer_blade_fury(Creature *_Creature)
{
    return new mob_bonechewer_blade_furyAI(_Creature);
}

/****************
* Bonechewer Blood Prophet - id 23237
*****************/

#define SPELL_BLOOD_DRAIN               41238
#define SPELL_BLOODBOLT_1               41229
#define SPELL_PROPHECY_OF_BLOOD_PL      41230
#define SPELL_PROPHECY_OF_BLOOD_MOB     41231

struct mob_bonechewer_blood_prophetAI : public ScriptedAI
{
    mob_bonechewer_blood_prophetAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer BloodDrain;
    Timer Bloodbolt;
    Timer ProphecyOfBlood;


    void Reset()
    {
        ClearCastQueue();

        BloodDrain.Reset(urand(5000, 25000));
        Bloodbolt.Reset(urand(1000, 10000));
        ProphecyOfBlood.Reset(12000);
    }

    void EnterCombat(Unit *) { DoZoneInCombat(80.0f); }

    void DamageTaken(Unit* who, uint32& damage)
    {
        if (damage)
        {
            if (me->GetHealth() * 100 / me->GetMaxHealth() < 35)
                ForceSpellCast(me, SPELL_FRENZY_1);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (BloodDrain.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_BLOOD_DRAIN);
            BloodDrain = 25000;
        }


        if (Bloodbolt.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                AddSpellToCast(target, SPELL_BLOODBOLT_1);
            Bloodbolt = 15000;
        }


        if (ProphecyOfBlood.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 20.0f, true))
                AddSpellToCast(target, SPELL_PROPHECY_OF_BLOOD_PL);
            std::list<Creature*> Friendly = FindAllFriendlyInGrid(20.0f);
            if (!Friendly.empty())
            {
                std::list<Creature*>::iterator i = Friendly.begin();
                advance(i, rand() % Friendly.size());
                if (Creature* Friend = (*i))
                    AddSpellToCast(Friend, SPELL_PROPHECY_OF_BLOOD_MOB);
            }
            ProphecyOfBlood = 15000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_bonechewer_blood_prophet(Creature *_Creature)
{
    return new mob_bonechewer_blood_prophetAI(_Creature);
}

/****************
* Mutant War Hound - id 23232
*****************/

#define SPELL_DISEASE_CLOUD             41290   //visual
#define SPELL_CLOUD_OF_DISEASE          41193


struct mob_mutated_war_houndAI : public ScriptedAI
{
    mob_mutated_war_houndAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    void Reset()
    {
        if (!me->HasAura(SPELL_DISEASE_CLOUD, 0))
            DoCast(me, SPELL_DISEASE_CLOUD);
    }

    void EnterCombat(Unit *) { DoZoneInCombat(80.0f); }

    void DamageTaken(Unit* who, uint32& damage)
    {
        if (damage && damage > me->GetHealth())
            me->CastSpell(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), SPELL_CLOUD_OF_DISEASE, false);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_mutated_war_hound(Creature *_Creature)
{
    return new mob_mutated_war_houndAI(_Creature);
}

/****************
* Bonechewer Brawler - id 23222
*****************/

#define SPELL_ENRAGE_2              41254

struct mob_bonechewer_brawlerAI : public ScriptedAI
{
    mob_bonechewer_brawlerAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Enrage;

    void Reset()
    {
        ClearCastQueue();

        Enrage.Reset(urand(15000, 25000));
    }

    void EnterCombat(Unit *) { DoZoneInCombat(80.0f); }

    void DamageTaken(Unit* who, uint32& damage)
    {
        if (damage)
        {
            if (me->GetHealth() * 100 / me->GetMaxHealth() < 30)
                ForceSpellCast(me, SPELL_FRENZY_1);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Enrage.Expired(diff))
        {
            AddSpellToCast(me, SPELL_ENRAGE_2);
            Enrage = 25000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_bonechewer_brawler(Creature *_Creature)
{
    return new mob_bonechewer_brawlerAI(_Creature);
}

/****************
* Bonechewer Combatant - id 23239
*****************/

#define SPELL_COMBAT_RAGE               41251

struct mob_bonechewer_combatantAI : public ScriptedAI
{
    mob_bonechewer_combatantAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer CombatRage;

    void Reset()
    {
        ClearCastQueue();

        CombatRage.Reset(urand(3000, 10000));
    }

    void EnterCombat(Unit *) { DoZoneInCombat(80.0f); }

    void DamageTaken(Unit* who, uint32& damage)
    {
        if (damage)
        {
            if (me->GetHealth() * 100 / me->GetMaxHealth() < 30)
                ForceSpellCast(me, SPELL_FRENZY_1);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (CombatRage.Expired(diff))
        {
            AddSpellToCast(me, SPELL_COMBAT_RAGE);
            CombatRage = 10000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_bonechewer_combatant(Creature *_Creature)
{
    return new mob_bonechewer_combatantAI(_Creature);
}

/****************
* Bonechewer Spectator - id 23223
*****************/

#define SPELL_CHARGE_1                  36140
#define SPELL_CLEAVE_3                  40505
#define SPELL_MORTAL_WOUND              25646
#define SPELL_STRIKE_1                  13446
#define SPELL_SUNDER_ARMOR              13444

struct mob_bonechewer_spectatorAI : public ScriptedAI
{
    mob_bonechewer_spectatorAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Charge;
    Timer Cleave;
    Timer MortalWound;
    Timer Strike;
    Timer SunderArmor;

    void Reset()
    {
        ClearCastQueue();

        Charge.Reset(urand(3000, 6000));
        Cleave.Reset(urand(5000, 15000));
        MortalWound.Reset(urand(2000, 6000));
        Strike.Reset(1500);
        SunderArmor.Reset(urand(3000, 10000));
    }

    void EnterCombat(Unit* who) { DoZoneInCombat(80.0f); }

    void DamageTaken(Unit* who, uint32& damage)
    {
        if (damage)
        {
            if (me->GetHealth() * 100 / me->GetMaxHealth() < 30)
                ForceSpellCast(me, SPELL_FRENZY_1);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Charge.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 25.0f, true, 0, 8.0f))
                AddSpellToCast(target, SPELL_CHARGE_1);
            Charge = urand(20000, 40000);
        }


        if (Cleave.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_CLEAVE_3);
            Cleave = 15000;
        }


        if (MortalWound.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_MORTAL_WOUND);
            MortalWound = urand(8000, 12000);
        }


        if (Strike.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_STRIKE_1);
            Strike = urand(4000, 6000);
        }


        if (SunderArmor.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_SUNDER_ARMOR);
            SunderArmor = 10000;
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_bonechewer_spectator(Creature *_Creature)
{
    return new mob_bonechewer_spectatorAI(_Creature);
}

/* ============================
*
*      RELIQUARY  OF  SOULS
*
* ============================*/

/* Content Data
    * Angered Soul Fragment
    * Hungering Soul Fragment
    * Suffering Soul Fragment
    */
/****************
* Angered Soul Fragment - id 23398
*****************/

#define SPELL_ANGER     41986

struct mob_angered_soul_fragmentAI : public ScriptedAI
{
    mob_angered_soul_fragmentAI(Creature *c) : ScriptedAI(c) {}

    Timer Anger;

    void Reset()
    {
        ClearCastQueue();

        Anger.Reset(urand(1500, 6000));
    }

    void EnterCombat(Unit *)
    {
        DoZoneInCombat(80.0f);
    }

    void JustDied(Unit* killer)
    {
        me->SetCorpseDelay(0);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Anger.Expired(diff))
        {
            AddSpellToCast(me, SPELL_ANGER);
            Anger = urand(6000, 16000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_angered_soul_fragment(Creature *_Creature)
{
    return new mob_angered_soul_fragmentAI(_Creature);
}

/****************
* Hungering Soul Fragment - id 23401
*****************/

#define SPELL_CONSUMING_STRIKES     41248

struct mob_hungering_soul_fragmentAI : public ScriptedAI
{
    mob_hungering_soul_fragmentAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
        DoCast(me, SPELL_CONSUMING_STRIKES);
    }

    void EnterCombat(Unit *)
    {
        DoZoneInCombat(80.0f);
    }

    void JustDied(Unit* killer)
    {
        me->SetCorpseDelay(0);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_hungering_soul_fragment(Creature *_Creature)
{
    return new mob_hungering_soul_fragmentAI(_Creature);
}

/****************
* Suffering Soul Fragment - id 23399
*****************/

#define SPELL_SOUL_BLAST        41245

struct mob_suffering_soul_fragmentAI : public ScriptedAI
{
    mob_suffering_soul_fragmentAI(Creature *c) : ScriptedAI(c) {}

    Timer SoulBlast;

    void Reset()
    {
        ClearCastQueue();

        SoulBlast.Reset(urand(2000, 5000));
    }

    void EnterCombat(Unit *)
    {
        DoZoneInCombat(80.0f);
    }

    void JustDied(Unit* killer)
    {
        me->SetCorpseDelay(0);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (SoulBlast.Expired(diff))
        {
            AddSpellToCast(me, SPELL_SOUL_BLAST);
            SoulBlast = urand(8000, 12000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_suffering_soul_fragment(Creature *_Creature)
{
    return new mob_suffering_soul_fragmentAI(_Creature);
}

/* ============================
*
*      MOTHER  SHAHRAZ
*
* ============================*/

/* Content Data
    * Priestess of Dementia
    * Priestess of Delight
    * Sister of Pain
    * Sister of Pleasure
    * Charming Courtesan
    * Spellbound Attendent
    * Enslaved Servant
    * Temple Concubine
    */

/****************
* Priestess of Dementia - id 22957
*****************/

enum PristessOfDementia
{
    NPC_IMAGE_OF_DEMENTIA = 23436,

    SPELL_CONFUSION = 41397,
    SPELL_DEMENTIA = 41404,
    SPELL_WHIRLWIND_3 = 41399
};

struct mob_pristess_of_dementiaAI : public ScriptedAI
{
    mob_pristess_of_dementiaAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Confusion;
    Timer Dementia;
    Timer ImageSummon;

    void Reset()
    {
        ClearCastQueue();

        Confusion.Reset(urand(5000, 15000));
        Dementia.Reset(3000);
        ImageSummon.Reset(15000);
    }

    void EnterCombat(Unit *)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Confusion.Expired(diff))
        {
            AddSpellToCast(me, SPELL_CONFUSION);
            Confusion = urand(15000, 25000);
        }


        if (Dementia.Expired(diff))
        {
            AddSpellToCast(me, SPELL_DEMENTIA);
            Dementia = urand(40000, 50000);
        }



        if (ImageSummon.Expired(diff))
        {
            float x, y, z;
            for (uint8 i = 0; i < 2; ++i)
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                {
                    target->GetNearPoint(x, y, z, 5.0f);
                    me->SummonCreature(NPC_IMAGE_OF_DEMENTIA, x, y, z, 0, TEMPSUMMON_TIMED_DESPAWN, 15000);
                }
            }
            ImageSummon = urand(25000, 35000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_pristess_of_dementia(Creature *_Creature)
{
    return new mob_pristess_of_dementiaAI(_Creature);
}

struct mob_image_of_dementiaAI : public ScriptedAI
{
    mob_image_of_dementiaAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer MoveTimer;

    void Reset()
    {
        me->CastSpell(me, SPELL_WHIRLWIND_3, false);
        MoveTimer.Reset(1000);
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (MoveTimer.Expired(diff))
        {
            float x, y, z = 0;
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 200.0f, true))
            {
                target->GetGroundPointAroundUnit(x, y, z, 10.0, 3.14*RAND(0, 1 / 6, 2 / 6, 3 / 6, 4 / 6, 5 / 6, 1));
                me->GetMotionMaster()->Clear(false);
                me->SetSpeed(MOVE_RUN, 2.5, true);
                me->GetMotionMaster()->MovePoint(0, x, y, z);
            }
            MoveTimer = urand(1500, 3000);
        }
    }
};

CreatureAI* GetAI_mob_image_of_dementia(Creature *_Creature)
{
    return new mob_image_of_dementiaAI(_Creature);
}

/****************
* Priestess of Delight - id 22962
*****************/

enum PristessOfDelight
{
    SPELL_CURSE_OF_VITALITY = 41351,
    SPELL_POLYMORPH = 41334
};

struct mob_pristess_of_delightAI : public ScriptedAI
{
    mob_pristess_of_delightAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer CurseOfVitality;
    Timer Polymorph;

    void Reset()
    {
        ClearCastQueue();

        CurseOfVitality.Reset(urand(1000, 3000));
        Polymorph.Reset(urand(5000, 10000));
    }

    void EnterCombat(Unit *)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Polymorph.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                AddSpellToCast(target, SPELL_POLYMORPH, false, true);
            Polymorph = urand(15000, 18000);
        }


        if (CurseOfVitality.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                AddSpellToCast(target, SPELL_CURSE_OF_VITALITY);
            CurseOfVitality = urand(5000, 8000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_pristess_of_delight(Creature *_Creature)
{
    return new mob_pristess_of_delightAI(_Creature);
}

/****************
* Sister of Pain - id 22956
*****************/

enum SisterOfPain
{
    NPC_SISTER_OF_PAIN = 22956,
    NPC_SISTER_OF_PLEASURE = 22964,

    SPELL_LASH_OF_PAIN = 41353,
    SPELL_PAINFUL_RAGE = 41369,
    SPELL_SHADOW_WORD_PAIN = 41355,
    SPELL_SHELL_OF_PAIN = 41371,
    SPELL_SHARED_BONDS = 41363
};

#define YELL_SISTER_OF_PAIN "Pain is inevitable, but the suffering is optional."

struct mob_sister_of_painAI : public ScriptedAI
{
    mob_sister_of_painAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer LashOfPain;
    Timer ShadowWordPain;
    Timer ShellOfPain;

    void Reset()
    {
        ClearCastQueue();

        LashOfPain.Reset(urand(3000, 5000));
        ShadowWordPain.Reset(urand(1000, 3000));
        ShellOfPain.Reset(urand(10000, 20000));
    }

    void EnterCombat(Unit *)
    {
        DoZoneInCombat(80.0f);

        if (Unit* Sister = FindCreature(NPC_SISTER_OF_PLEASURE, 30, me))
        {
            if (!me->HasAura(SPELL_SHARED_BONDS, 0))
                Sister->CastSpell(me, SPELL_SHARED_BONDS, false);
        }
    }

    void DamageTaken(Unit* who, uint32 &damage)
    {
        uint32 HPPercent = me->GetHealth() * 100 / me->GetMaxHealth();
        uint8 stack = 0;
        if (me->HasAura(SPELL_PAINFUL_RAGE, 0))
        {
            if (Aura* Rage = me->GetAura(SPELL_PAINFUL_RAGE, 0))
                stack = Rage->GetStackAmount();
        }
        if (damage)
        {
            if (HPPercent < (95 - 10 * stack))
                me->CastSpell(me, SPELL_PAINFUL_RAGE, true);
            if (urand(1, 1000) > 995)
                DoYell(YELL_SISTER_OF_PAIN, 0, who);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (LashOfPain.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_LASH_OF_PAIN);
            LashOfPain = urand(12000, 15000);
        }


        if (ShadowWordPain.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                AddSpellToCast(target, SPELL_SHADOW_WORD_PAIN);
            ShadowWordPain = urand(5000, 8000);
        }


        if (ShellOfPain.Expired(diff))
        {
            AddSpellToCast(me, SPELL_SHELL_OF_PAIN);
            ShellOfPain = urand(30000, 50000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_sister_of_pain(Creature *_Creature)
{
    return new mob_sister_of_painAI(_Creature);
}

/****************
* Sister of Pleasure - id 22964
*****************/

enum SisterOfPleasure
{
    SPELL_GREATER_HEAL = 41378,
    SPELL_HOLY_NOVA_1 = 41380,
    SPELL_SHELL_OF_LIFE = 41381
};

#define YELL_SISTER_OF_PLEASURE "When it comes to the needs of Illidan's troops, Mother Shahraz knows best."

struct mob_sister_of_pleasureAI : public ScriptedAI
{
    mob_sister_of_pleasureAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer GreaterHeal;
    Timer HolyNova;
    Timer ShellOfLife;
    bool cooldown;

    void Reset()
    {
        ClearCastQueue();

        GreaterHeal.Reset(2000);
        HolyNova.Reset(urand(5000, 10000));
        ShellOfLife.Reset(20000);
        cooldown = false;
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat(80.0f);
        DoYell(YELL_SISTER_OF_PLEASURE, 0, who);

        if (Unit* Sister = FindCreature(NPC_SISTER_OF_PAIN, 30, me))
        {
            if (!me->HasAura(SPELL_SHARED_BONDS, 0))
                Sister->CastSpell(me, SPELL_SHARED_BONDS, false);
        }
    }

    void DamageTaken(Unit* who, uint32 &damage)
    {
        uint32 HPPercent = me->GetHealth() * 100 / me->GetMaxHealth();
        if (damage)
        {
            if (HPPercent < 75 && !cooldown)
            {
                ForceSpellCast(me, SPELL_SHELL_OF_LIFE);
                cooldown = true;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (HolyNova.Expired(diff))
        {
            DoCastAOE(SPELL_HOLY_NOVA_1);
            HolyNova = urand(7000, 14000);
        }


        if (GreaterHeal.Expired(diff))
        {
            Unit* healTarget = SelectLowestHpFriendly(40.0f, 20000);
            if (healTarget)
                AddSpellToCast(healTarget, SPELL_GREATER_HEAL);
            GreaterHeal = urand(5000, 10000);     //check targets each 5-10s
        }


        if (cooldown && ShellOfLife.Expired(diff))
        {
            cooldown = false;
            ShellOfLife = urand(15000, 25000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_sister_of_pleasure(Creature *_Creature)
{
    return new mob_sister_of_pleasureAI(_Creature);
}

/****************
* Charming Courtesan - id 22955
*****************/

enum CharmingCourtesan
{
    SPELL_INFATUATION = 41345,
    SPELL_POISONOUS_THROW = 41346
};

#define YELL_CHARMING_COURTESAN "That which you desire lies this way, hero."

struct mob_charming_courtesanAI : public ScriptedAI
{
    mob_charming_courtesanAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Infatuation;
    Timer PoisonousThrow;

    void Reset()
    {
        ClearCastQueue();

        Infatuation.Reset(urand(10000, 15000));
        PoisonousThrow.Reset(urand(2000, 10000));
    }

    void EnterCombat(Unit* who)
    {
        if (urand(1, 100) > 85)
            DoYell(YELL_CHARMING_COURTESAN, 0, who);
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Infatuation.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                AddSpellToCast(target, SPELL_INFATUATION);
            Infatuation = urand(20000, 30000);
        }


        if (PoisonousThrow.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                AddSpellToCast(target, SPELL_INFATUATION);
            PoisonousThrow = urand(6000, 10000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_charming_courtesan(Creature *_Creature)
{
    return new mob_charming_courtesanAI(_Creature);
}

/****************
* Spellbound Attendent - id 22959
*****************/

enum SpellboundAttendent
{
    SPELL_KICK = 41395,
    SPELL_SLEEP = 41396
};

struct mob_spellbound_attendentAI : public ScriptedAI
{
    mob_spellbound_attendentAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Kick;
    Timer Sleep;
    bool cooldown;

    void Reset()
    {
        ClearCastQueue();

        Sleep.Reset(urand(3000, 8000));
        Kick.Reset(10000);
        cooldown = false;
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (me->getVictim() && me->getVictim()->IsNonMeleeSpellCast(false) && !cooldown)
        {
            ForceSpellCast(me->getVictim(), SPELL_KICK, INTERRUPT_AND_CAST_INSTANTLY);
            cooldown = true;
        }

        if (cooldown && Kick.Expired(diff))
        {
            cooldown = false;
            Kick = 10000;   //10s cooldown as rogue's ability
        }


        if (Sleep.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 1, 30.0f, true))
                AddSpellToCast(target, SPELL_SLEEP, false, true);
            Sleep = urand(10000, 18000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_spellbound_attendent(Creature *_Creature)
{
    return new mob_spellbound_attendentAI(_Creature);
}

/****************
* Enslaved Servant - id 22965
*****************/

enum EnslavedSevrant
{
    SPELL_UPPERCUT = 41388,
    SPELL_KIDNEY_SHOT = 41389
};

struct mob_enslaved_servantAI : public ScriptedAI
{
    mob_enslaved_servantAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Uppercut;
    Timer KidneyShot;

    void Reset()
    {
        ClearCastQueue();

        Uppercut.Reset(urand(5000, 15000));
        KidneyShot.Reset(urand(3000, 10000));
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (KidneyShot.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_KIDNEY_SHOT);
            KidneyShot = urand(20000, 30000);
        }


        if (Uppercut.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_UPPERCUT);
            Uppercut = urand(5000, 10000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_enslaved_servant(Creature *_Creature)
{
    return new mob_enslaved_servantAI(_Creature);
}

/****************
* Temple Concubine - id 22939
*****************/

enum TempleConcubine
{
    SPELL_LOVE_TAP = 41338
};

#define YELL_TEMPLE_CONCUBINE "Business... or pleasure?"

struct mob_temple_concubineAI : public ScriptedAI
{
    mob_temple_concubineAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer LoveTap;
    Timer Polymorph;

    void Reset()
    {
        ClearCastQueue();

        LoveTap.Reset(urand(2000, 15000));
        Polymorph.Reset(urand(5000, 10000));
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (LoveTap.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 10.0f, true))
                AddSpellToCast(target, SPELL_LOVE_TAP, false, true);
            LoveTap = urand(10000, 20000);
        }


        if (Polymorph.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30.0f, true))
            {
                AddSpellToCast(target, SPELL_POLYMORPH, false, true);
                if (urand(1, 10) > 3)
                    DoYell(YELL_TEMPLE_CONCUBINE, 0, target);
            }
            Polymorph = urand(15000, 25000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_temple_concubine(Creature *_Creature)
{
    return new mob_temple_concubineAI(_Creature);
}

/* ============================
*
*      ILLIDARI  COUNCIL
*
* ============================*/

/* Content Data
    * Illidari Archon
    * Illidari Assassin
    * Illidari Battle-mage
    * Illidari Blood Lord
    * Promenade Sentinel
    */

/****************
* Illidari Archon - id 23400
*****************/

enum IllidariArchon
{
    SPELL_HOLY_SMITE = 41370,
    SPELL_HEAL = 41372,
    SPELL_POWER_WORD_SHIELD = 41373,
    SPELL_SHADOWFORM = 29406,
    SPELL_MIND_FLAY = 37276,
    SPELL_POWER_WORD_DEATH = 41375,
    SPELL_MIND_BLAST = 41374,

    HOLY_TYPE = 100,
    SHADOW_TYPE = 101
};

struct mob_illidari_archonAI : public ScriptedAI
{
    mob_illidari_archonAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    uint32 type;
    Timer shieldCooldownTimer;
    Timer Heal;
    bool shieldCooldown;

    Timer MindFlay;
    Timer MindBlast;
    Timer wordDeathTimer;
    bool wordDeathCooldown;

    void Reset()
    {
        ClearCastQueue();

        type = RAND(HOLY_TYPE, SHADOW_TYPE);
        switch (type)
        {
            case HOLY_TYPE:
            {
                DoCast(me, SPELL_POWER_WORD_SHIELD);
                Heal.Reset(2500);
                shieldCooldownTimer.Reset(15000);
                shieldCooldown = true;
                SetAutocast(SPELL_HOLY_SMITE, 4800, true, CAST_TANK, 40, true);
                StartAutocast();
                break;
            }
            break;
            case SHADOW_TYPE:
            {
                StopAutocast();
                DoCast(me, SPELL_SHADOWFORM);
                MindFlay.Reset(6000);
                MindBlast.Reset(2000);
                wordDeathTimer.Reset(8000);
                wordDeathCooldown = false;
                break;
            }
            break;
        }
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        switch (type)
        {
            case HOLY_TYPE:
            {
                if (!shieldCooldown)
                {
                    if (Unit* shieldTarget = SelectLowestHpFriendly(40, 20000))
                    {
                        AddSpellToCast(shieldTarget, SPELL_POWER_WORD_SHIELD, false, true);
                        shieldCooldown = true;
                    }
                }
                if (shieldCooldownTimer.Expired(diff))
                {
                    shieldCooldown = false;
                    shieldCooldownTimer = 15000;   //15s cooldown as priest's ability
                }

                if (Heal.Expired(diff))
                {
                    if (Unit* healTarget = SelectLowestHpFriendly(40, 60000))
                        AddSpellToCast(healTarget, SPELL_HEAL, false, true);
                    Heal = 2600;
                }
            }
            break;
            case SHADOW_TYPE:
            {
                if (!wordDeathCooldown)
                {
                    if (me->getVictim()->GetHealth() * 100 / me->getVictim()->GetMaxHealth() < 30)
                    {
                        ClearCastQueue();
                        ForceSpellCast(me->getVictim(), SPELL_POWER_WORD_DEATH, INTERRUPT_AND_CAST);
                        wordDeathCooldown = true;
                        wordDeathTimer = 8000;
                    }
                }
                if (wordDeathCooldown && wordDeathTimer.Expired(diff))
                {
                    wordDeathCooldown = false;
                    wordDeathTimer = 8000;
                }

                if (MindFlay.Expired(diff))
                {
                    if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 20.0f, true))
                        AddSpellToCast(target, SPELL_MIND_FLAY);
                    MindFlay = urand(6000, 9000);
                }

                if (MindBlast.Expired(diff))
                {
                    if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                        AddSpellToCast(target, SPELL_MIND_BLAST);
                    MindBlast = urand(12000, 18000);
                }

            }
            break;
        }

        CastNextSpellIfAnyAndReady(diff);
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_illidari_archon(Creature *_Creature)
{
    return new mob_illidari_archonAI(_Creature);
}

/****************
* Illidari Assassin - id 23403
*****************/

enum IllidariAssassin
{
    SPELL_PARALYZING_POISON = 3609,
    SPELL_RIPOSTE_AURA = 41393,
    SPELL_VANISH = 39667,
    SPELL_AMBUSH = 41390
};

struct mob_illidari_assassinAI : public ScriptedAI
{
    mob_illidari_assassinAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer VanishEvent;
    Timer ParalyzingPoison;
    uint64 AmbushTagetGUID;
    bool ambushed;

    void Reset()
    {
        ClearCastQueue();

        DoCast(me, SPELL_RIPOSTE_AURA);
        VanishEvent.Reset(urand(6000, 8000));
        ParalyzingPoison.Reset(15000);
        AmbushTagetGUID = 0;
        ambushed = false;
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat(80.0f);
    }

    void OnAuraRemove(Aura* Aur, bool stack)
    {
        if (Aur->GetId() == SPELL_VANISH)
            AttackStart(me->getVictim());
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (VanishEvent.Expired(diff))
        {
            ClearCastQueue();
            ForceSpellCast(me, SPELL_VANISH, INTERRUPT_AND_CAST);
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30.0f, true))
            {
                AmbushTagetGUID = target->GetGUID();
                DoStartMovement(target);
                ambushed = false;
            }
            VanishEvent = urand(12000, 18000);
        }


        if (!ambushed && me->HasAura(SPELL_VANISH, 1))
        {
            if (Player* PlayerTarget = me->GetPlayer(AmbushTagetGUID))
            {
                if (me->IsWithinDistInMap(PlayerTarget, 14.0))
                {
                    ForceSpellCast(PlayerTarget, SPELL_AMBUSH);
                    ambushed = true;
                }
            }
            return;
        }
        else
            DoMeleeAttackIfReady();

        if (ParalyzingPoison.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_PARALYZING_POISON);
            ParalyzingPoison = urand(10000, 14000);
        }

        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_mob_illidari_assassin(Creature *_Creature)
{
    return new mob_illidari_assassinAI(_Creature);
}

/****************
* Illidari Battle-mage - id 23402
*****************/

enum IllidariBattlemage
{
    SPELL_BLIZZARD = 41382,
    SPELL_FLAMESTRIKE = 41379,
    SPELL_FIREBALL = 41383,
    SPELL_FROSTBOLT = 41384
};

struct mob_illidari_battle_mageAI : public ScriptedAI
{
    mob_illidari_battle_mageAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer Blizzard;
    Timer Flamestrike;
    Timer DirectTimer;

    void Reset()
    {
        ClearCastQueue();

        DirectTimer.Reset(urand(2500, 5000));
        Flamestrike.Reset(6000);
        Blizzard.Reset(12000);
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (DirectTimer.Expired(diff))
        {
            uint32 spellId = RAND(SPELL_FIREBALL, SPELL_FROSTBOLT);
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                AddSpellToCast(target, spellId, false, true);
            DirectTimer = urand(2500, 3500);
        }


        if (Flamestrike.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                AddSpellToCast(target, SPELL_FLAMESTRIKE, false, true);
            Flamestrike = urand(12000, 18000);
        }


        if (Blizzard.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 45.0f, true))
                AddSpellToCast(target, SPELL_BLIZZARD);
            DirectTimer = urand(10000, 13000);
            Blizzard = urand(18000, 25000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_illidari_battle_mage(Creature *_Creature)
{
    return new mob_illidari_battle_mageAI(_Creature);
}

/****************
* Illidari Blood Lord - id 23397
*****************/

enum IllidariBloodLord
{
    SPELL_HAMMER_OF_JUSTICE = 13005,
    SPELL_JUDGEMENT_OF_COMMAND = 41368,
    SPELL_DIVINE_SHIELD = 41367
};

#define YELL_ILLIDARI_BLOOD_LORD    "This world belongs to the Illidari"

struct mob_illidari_blood_lordAI : public ScriptedAI
{
    mob_illidari_blood_lordAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer HammerOfJustice;
    Timer JudgmentOfCommand;
    bool shielded;

    void Reset()
    {
        ClearCastQueue();

        HammerOfJustice.Reset(urand(6000, 8000));
        JudgmentOfCommand.Reset(urand(8000, 10000));
        shielded = false;
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat(80.0f);
        if (urand(1, 10) > 7)
            DoYell(YELL_ILLIDARI_BLOOD_LORD, 0, who);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (me->GetHealth() * 100 / me->GetMaxHealth() < 20 && !shielded)
        {
            ForceSpellCast(me, SPELL_DIVINE_SHIELD, INTERRUPT_AND_CAST, true);
            shielded = true;
        }

        if (HammerOfJustice.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_HAMMER_OF_JUSTICE);
            HammerOfJustice = urand(15000, 25000);
        }


        if (JudgmentOfCommand.Expired(diff))
        {
            AddSpellToCast(me->getVictim(), SPELL_JUDGEMENT_OF_COMMAND);
            JudgmentOfCommand = urand(3000, 8000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_illidari_blood_lord(Creature *_Creature)
{
    return new mob_illidari_blood_lordAI(_Creature);
}

/****************
* Promenade Sentinel - id 23394
*****************/

enum PromenadeSentinel
{
    SPELL_L1_ARCANE_CHARGE = 41359,
    SPELL_SPOTLIGHT = 41347,
    SPELL_L4_ARCANE_CHARGE_SUMMON = 41348,
    SPELL_L4_ARCANE_CHARGE = 41349,
    SPELL_L5_ARCANE_CHARGE = 41360
};

#define YELL_PROMENADE_SENTINEL    "Unauthorized entry detected. Engaging annihilation protocols."

struct mob_promenade_sentinelAI : public ScriptedAI
{
    mob_promenade_sentinelAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(AGGRO_RANGE); }

    Timer L5arcane;
    Timer L4arcane;

    void Reset()
    {
        ClearCastQueue();

        L5arcane.Reset(urand(7000, 10000));
        L4arcane.Reset(urand(1000, 3000));
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat(80.0f);
        DoYell(YELL_PROMENADE_SENTINEL, 0, who);
        DoCast(me, SPELL_L1_ARCANE_CHARGE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (L5arcane.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 45.0f, true, me->getVictimGUID()))
                AddSpellToCast(target, SPELL_L5_ARCANE_CHARGE, false, true);
            L5arcane = urand(15000, 25000);
        }


        if (L4arcane.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                AddSpellToCast(target, SPELL_L4_ARCANE_CHARGE_SUMMON);
            L4arcane = urand(10000, 15000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_promenade_sentinel(Creature *_Creature)
{
    return new mob_promenade_sentinelAI(_Creature);
}

struct mob_arcane_chargeAI : public Scripted_NoMovementAI
{
    mob_arcane_chargeAI(Creature *c) : Scripted_NoMovementAI(c) {}

    Timer Delay;

    void Reset()
    {
        DoCast(me, SPELL_SPOTLIGHT);
        Delay.Reset(3000);
    }
    void EnterCombat(Unit*) { DoZoneInCombat(80.0f); }
    void UpdateAI(const uint32 diff)
    {
        if (Delay.Expired(diff))
        {
            DoCast(me, SPELL_L4_ARCANE_CHARGE);
            Delay = 10000;
        }

    }
};

CreatureAI* GetAI_mob_arcane_charge(Creature *_Creature)
{
    return new mob_arcane_chargeAI(_Creature);
}

void AddSC_black_temple_trash()
{
    Script *newscript;

    //Najentus
    newscript = new Script;
    newscript->Name = "mob_aqueous_lord";
    newscript->GetAI = &GetAI_mob_aqueous_lord;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_aqueous_spawn";
    newscript->GetAI = &GetAI_mob_aqueous_spawn;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_coilskar_general";
    newscript->GetAI = &GetAI_mob_coilskar_general;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_coilskar_harpooner";
    newscript->GetAI = &GetAI_mob_coilskar_harpooner;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_coilskar_seacaller";
    newscript->GetAI = &GetAI_mob_coilskar_seacaller;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_coilskar_geyser";
    newscript->GetAI = &GetAI_mob_coilskar_geyser;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_coilskar_soothsayer";
    newscript->GetAI = &GetAI_mob_coilskar_soothsayer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_coilskar_wrangler";
    newscript->GetAI = &GetAI_mob_coilskar_wrangler;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_dragon_turtle";
    newscript->GetAI = &GetAI_mob_dragon_turtle;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_leviathan";
    newscript->GetAI = &GetAI_mob_leviathan;
    newscript->RegisterSelf();

    //Supremus
    newscript = new Script;
    newscript->Name = "mob_bonechewer_taskmaster";
    newscript->GetAI = &GetAI_mob_bonechewer_taskmaster;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_bonechewer_worker";
    newscript->GetAI = &GetAI_mob_bonechewer_worker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_dragonmaw_skystalker";
    newscript->GetAI = &GetAI_mob_dragonmaw_skystalker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_dragonmaw_windreaver";
    newscript->GetAI = &GetAI_mob_dragonmaw_windreaver;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_dragonmaw_wyrmcaller";
    newscript->GetAI = &GetAI_mob_dragonmaw_wyrmcaller;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_illidari_fearbringer";
    newscript->GetAI = &GetAI_mob_illidari_fearbringer;
    newscript->RegisterSelf();

    // Shade Of Akama
    newscript = new Script;
    newscript->Name = "mob_ashtongue_battlelord";
    newscript->GetAI = &GetAI_mob_ashtongue_battlelord;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_ashtongue_feral_spirit";
    newscript->GetAI = &GetAI_mob_ashtongue_feral_spirit;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_ashtongue_mystic";
    newscript->GetAI = &GetAI_mob_ashtongue_mystic;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "totem_ashtongue_mystic";
    newscript->GetAI = &GetAI_totem_ashtongue_mystic;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_ashtongue_primalist";
    newscript->GetAI = &GetAI_mob_ashtongue_primalist;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_ashtongue_stalker";
    newscript->GetAI = &GetAI_mob_ashtongue_stalker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_ashtongue_stormcaller";
    newscript->GetAI = &GetAI_mob_ashtongue_stormcaller;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_illidari_boneslicer";
    newscript->GetAI = &GetAI_mob_illidari_boneslicer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_illidari_centurion";
    newscript->GetAI = &GetAI_mob_illidari_centurion;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_illidari_defiler";
    newscript->GetAI = &GetAI_mob_illidari_defiler;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_illidari_heartseeker";
    newscript->GetAI = &GetAI_mob_illidari_heartseeker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_illidari_nightlord";
    newscript->GetAI = &GetAI_mob_illidari_nightlord;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_storm_fury";
    newscript->GetAI = &GetAI_mob_storm_fury;
    newscript->RegisterSelf();

    // Teron Gorefiend
    newscript = new Script;
    newscript->Name = "mob_hand_of_gorefiend";
    newscript->GetAI = &GetAI_mob_hand_of_gorefiend;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_shadowmoon_blood_mage";
    newscript->GetAI = &GetAI_mob_shadowmoon_blood_mage;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_shadowmoon_champion";
    newscript->GetAI = &GetAI_mob_shadowmoon_champion;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_whirling_blade";
    newscript->GetAI = &GetAI_mob_whirling_blade;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_shadowmoon_deathshaper";
    newscript->GetAI = &GetAI_mob_shadowmoon_deathshaper;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_shadowmoon_grunt";
    newscript->GetAI = &GetAI_mob_shadowmoon_grunt;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_shadowmoon_houndmaster";
    newscript->GetAI = &GetAI_mob_shadowmoon_houndmaster;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_shadowmoon_reaver";
    newscript->GetAI = &GetAI_mob_shadowmoon_reaver;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_shadowmoon_riding_hound";
    newscript->GetAI = &GetAI_mob_shadowmoon_riding_hound;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_shadowmoon_soldier";
    newscript->GetAI = &GetAI_mob_shadowmoon_soldier;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_shadowmoon_weapon_master";
    newscript->GetAI = &GetAI_mob_shadowmoon_weapon_master;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_wrathbone_flayer";
    newscript->GetAI = &GetAI_mob_wrathbone_flayer;
    newscript->RegisterSelf();

    // Gurtogg Bloodboil
    newscript = new Script;
    newscript->Name = "mob_bonechewer_behemoth";
    newscript->GetAI = &GetAI_mob_bonechewer_behemoth;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_bonechewer_shield_disciple";
    newscript->GetAI = &GetAI_mob_bonechewer_shield_disciple;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_bonechewer_blade_fury";
    newscript->GetAI = &GetAI_mob_bonechewer_blade_fury;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_bonechewer_blood_prophet";
    newscript->GetAI = &GetAI_mob_bonechewer_blood_prophet;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_mutated_war_hound";
    newscript->GetAI = &GetAI_mob_mutated_war_hound;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_bonechewer_brawler";
    newscript->GetAI = &GetAI_mob_bonechewer_brawler;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_bonechewer_combatant";
    newscript->GetAI = &GetAI_mob_bonechewer_combatant;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_bonechewer_spectator";
    newscript->GetAI = &GetAI_mob_bonechewer_spectator;
    newscript->RegisterSelf();

    // Reliquary of Souls
    newscript = new Script;
    newscript->Name = "mob_angered_soul_fragment";
    newscript->GetAI = &GetAI_mob_angered_soul_fragment;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_hungering_soul_fragment";
    newscript->GetAI = &GetAI_mob_hungering_soul_fragment;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_suffering_soul_fragment";
    newscript->GetAI = &GetAI_mob_suffering_soul_fragment;
    newscript->RegisterSelf();

    // Mother Shahraz
    newscript = new Script;
    newscript->Name = "mob_pristess_of_dementia";
    newscript->GetAI = &GetAI_mob_pristess_of_dementia;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_image_of_dementia";
    newscript->GetAI = &GetAI_mob_image_of_dementia;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_pristess_of_delight";
    newscript->GetAI = &GetAI_mob_pristess_of_delight;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_sister_of_pain";
    newscript->GetAI = &GetAI_mob_sister_of_pain;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_sister_of_pleasure";
    newscript->GetAI = &GetAI_mob_sister_of_pleasure;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_charming_courtesan";
    newscript->GetAI = &GetAI_mob_charming_courtesan;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_spellbound_attendent";
    newscript->GetAI = &GetAI_mob_spellbound_attendent;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_enslaved_servant";
    newscript->GetAI = &GetAI_mob_enslaved_servant;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_temple_concubine";
    newscript->GetAI = &GetAI_mob_temple_concubine;
    newscript->RegisterSelf();

    // Illidari Council
    newscript = new Script;
    newscript->Name = "mob_illidari_archon";
    newscript->GetAI = &GetAI_mob_illidari_archon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_illidari_assassin";
    newscript->GetAI = &GetAI_mob_illidari_assassin;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_illidari_battle_mage";
    newscript->GetAI = &GetAI_mob_illidari_battle_mage;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_illidari_blood_lord";
    newscript->GetAI = &GetAI_mob_illidari_blood_lord;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_promenade_sentinel";
    newscript->GetAI = &GetAI_mob_promenade_sentinel;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_arcane_charge";
    newscript->GetAI = &GetAI_mob_arcane_charge;
    newscript->RegisterSelf();
}
