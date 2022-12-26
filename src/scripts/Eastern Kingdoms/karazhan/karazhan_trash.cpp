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
#include "def_karazhan.h"

#define SPELL_DANCE_VIBE            29521
#define SPELL_SEARING_PAIN          29492
#define SPELL_IMMOLATE              29928
#define SPELL_THROW                 29582
#define SPELL_IMPALE                29583
#define SPELL_GOBLIN_DRAGON_GUN     29513
#define SPELL_THROW_DYNAMITE        29579
#define SPELL_PUNCH                 29581
#define SPELL_CURSE_OF_AGONY        29930
#define SPELL_HEAL                  29580
#define SPELL_HOLY_NOVA             29514

#define GO_CHAIR                    183776


struct mob_phantom_guestAI : public ScriptedAI
{
    mob_phantom_guestAI(Creature* c) : ScriptedAI(c) 
    {
        Type = urand(0, 4);   
    }

    uint32 Type;
    Timer MainTimer;
    Timer SecondaryTimer;

    void Reset()
    {
        me->CastSpell(me, SPELL_DANCE_VIBE, true);

        MainTimer = 0;
        SecondaryTimer.Reset(urand(5000, 20000));

        if(GameObject *chair = FindGameObject(GO_CHAIR, 5.0, me))
            chair->Use(me);
    }

    void AttackStart(Unit *who)
    {
        if(Type == 0 || Type == 1)
            ScriptedAI::AttackStartNoMove(who, Type == 0 ? CHECK_TYPE_CASTER : CHECK_TYPE_SHOOTER);
        else
            ScriptedAI::AttackStart(who);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        
        if (MainTimer.Expired(diff))
        {
            switch(Type)
            {
            case 0:
                AddSpellToCast(SPELL_SEARING_PAIN, CAST_TANK);
                MainTimer = 3500;
                break;
            case 1:
                AddSpellToCast(SPELL_THROW, CAST_TANK);
                MainTimer = 2000;
                break;
            case 2:
                AddSpellToCast(SPELL_GOBLIN_DRAGON_GUN, CAST_SELF);
                MainTimer = 20000;
                break;
            case 3:
                AddSpellToCast(SPELL_PUNCH, CAST_TANK);
                MainTimer = urand(10000, 30000);
                break;
            case 4:
                AddSpellToCast(SPELL_HEAL, CAST_LOWEST_HP_FRIENDLY);
                MainTimer = urand(5000, 20000);
                break;
            }
        } 
        
        
        if (SecondaryTimer.Expired(diff))
        {
            switch(Type)
            {
            case 0:
                AddSpellToCast(SPELL_IMMOLATE, CAST_RANDOM);
                SecondaryTimer = urand(7000, 30000);
                break;
            case 1:
                AddSpellToCast(SPELL_IMPALE, CAST_RANDOM);
                SecondaryTimer = urand(5000, 30000);
                break;
            case 2:
                AddSpellToCast(SPELL_THROW_DYNAMITE, CAST_RANDOM);
                SecondaryTimer = urand(15000, 40000);
                break;
            case 3:
                AddSpellToCast(SPELL_CURSE_OF_AGONY, CAST_RANDOM);
                SecondaryTimer = urand(10000, 30000);
                break;
            case 4:
                AddSpellToCast(SPELL_HOLY_NOVA, CAST_SELF);
                SecondaryTimer = urand(10000, 30000);
                break;
            }
        }
        

        if(Type == 0)
            CheckCasterNoMovementInRange(diff, 30.0);
        else if(Type == 1)
            CheckShooterNoMovementInRange(diff, 30.0);
        CastNextSpellIfAnyAndReady(diff);
        DoMeleeAttackIfReady();
    }    
};

CreatureAI* GetAI_mob_phantom_guest(Creature *_Creature)
{
    return new mob_phantom_guestAI(_Creature);
}

#define SPELL_DUAL_WIELD    674
#define SPELL_SHOT          29575
#define SPELL_MULTI_SHOT    29576

#define SENTRY_SAY_AGGRO1   "What is this?"
#define SENTRY_SAY_AGGRO2   "Stop them!"
#define SENTRY_SAY_AGGRO3   "Invaders in the tower!"
#define SENTRY_SAY_DEATH1   "I have failed..." 
#define SENTRY_SAY_RANDOM   "It's great assigment, yeah, but \"all looking and no touching\" gets old after a while."

struct mob_spectral_sentryAI : public ScriptedAI
{
    mob_spectral_sentryAI(Creature* c) : ScriptedAI(c) {}

    Timer ShotTimer;
    Timer MultiShotTimer;
    Timer RandomSayTimer;

    void Reset()
    {
        me->CastSpell(me, SPELL_DUAL_WIELD, true);

        ShotTimer = 1;
        MultiShotTimer.Reset(8000);
        RandomSayTimer.Reset(urand(40000, 80000));
    }
    
    void EnterCombat(Unit *who)
    {
        if(roll_chance_i(70))
            me->Say(RAND<const char*>(SENTRY_SAY_AGGRO1, SENTRY_SAY_AGGRO2, SENTRY_SAY_AGGRO3), 0, 0);
    }

    void AttackStart(Unit *who)
    {
        ScriptedAI::AttackStartNoMove(who, CHECK_TYPE_SHOOTER);
    }

    void JustDied(Unit *)
    {
        if(roll_chance_i(30))
            me->Say(SENTRY_SAY_DEATH1, 0, 0);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
        {
            if (RandomSayTimer.Expired(diff))
            {
                if(roll_chance_i(30))
                    me->Say(SENTRY_SAY_RANDOM, 0, 0);
                RandomSayTimer = urand(40000, 80000);
            }
            
            return;
        }

        
        if (ShotTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_SHOT, CAST_TANK);
            ShotTimer = 2000;
        } 
        
        if (MultiShotTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_MULTI_SHOT, CAST_RANDOM);
            MultiShotTimer = 8000;
        }
        

        CheckShooterNoMovementInRange(diff, 20.0);
        CastNextSpellIfAnyAndReady(diff);
        DoMeleeAttackIfReady();
    }    
};

CreatureAI* GetAI_mob_spectral_sentry(Creature *_Creature)
{
    return new mob_spectral_sentryAI(_Creature);
}

#define SPELL_RETURN_FIRE1  29793
#define SPELL_RETURN_FIRE2  29794
#define SPELL_RETURN_FIRE3  29788
#define SPELL_FIST_OF_STONE 29840
#define NPC_ASTRAL_SPARK    17283


struct mob_arcane_protectorAI : public ScriptedAI
{
    mob_arcane_protectorAI(Creature* c) : ScriptedAI(c) {}

    Timer SkillTimer;

    void Reset()
    {
        me->ApplySpellImmune(1, IMMUNITY_STATE, SPELL_AURA_PERIODIC_LEECH, true);
        me->ApplySpellImmune(2, IMMUNITY_STATE, SPELL_AURA_PERIODIC_MANA_LEECH, true);
        me->ApplySpellImmune(3, IMMUNITY_DISPEL, DISPEL_POISON, true);
        SkillTimer.Reset(urand(10000, 20000));
    }
    
    void EnterCombat(Unit *who)
    {
        me->CastSpell(me, RAND(SPELL_RETURN_FIRE1, SPELL_RETURN_FIRE2, SPELL_RETURN_FIRE3), false); 
    }

    void OnAuraApply(Aura *aur, Unit*, bool stack)
    {
        if(aur->GetEffIndex() == 0)
        {
            switch(aur->GetId())
            {
            case SPELL_RETURN_FIRE1:
                me->Say("Activating defence mode EL-2S.", 0, 0);
                break;
            case SPELL_RETURN_FIRE2:
                me->Say("Activating defence mode EL-5R.", 0, 0);
                break;
            case SPELL_RETURN_FIRE3:
                me->Say("Activating defence mode EL-7M.", 0, 0);
                break;
            }
        }
    }

    void JustDied(Unit *)
    {
        if(roll_chance_i(30))
            me->Say(RAND<const char*>("You will not make it out alive!",
                                      "This... changes nothing. Eternal damnation awaits you!",
                                      "Others will take my place"), 0, 0);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        
        if (SkillTimer.Expired(diff))
        {
            if(roll_chance_i(50))
                me->SummonCreature(NPC_ASTRAL_SPARK, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(),
                        TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
            else
                me->CastSpell(me, SPELL_FIST_OF_STONE, false);
            SkillTimer = urand(15000, 30000);
        }
        

        CastNextSpellIfAnyAndReady(diff);
        DoMeleeAttackIfReady();
    }    
};

CreatureAI* GetAI_mob_arcane_protector(Creature *_Creature)
{
    return new mob_arcane_protectorAI(_Creature);
}

#define SPELL_WARP_BREACH_AOE       29919
#define SPELL_WARP_BREACH_VISUAL    37079

struct mob_mana_warpAI : public ScriptedAI
{
    mob_mana_warpAI(Creature* c) : ScriptedAI(c) {}

    Timer Exploded;

    void Reset()
    {
        Exploded = 0;
    }
    
    void DamageTaken(Unit* pDone_by, uint32& uiDamage)
    {
        if(me->IsNonMeleeSpellCast(true) && uiDamage > me->GetHealth())
            uiDamage = me->GetHealth() - 1;
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(!Exploded.GetInterval() && HealthBelowPct(10))
        {
            me->CastSpell(me, SPELL_WARP_BREACH_AOE, false);
            me->CastSpell(me, SPELL_WARP_BREACH_VISUAL, true);
            Exploded.Reset(1000);
        }
        if (Exploded.Expired(diff))
            me->Kill(me);

        DoMeleeAttackIfReady();
    }    
};

CreatureAI* GetAI_mob_mana_warp(Creature *_Creature)
{
    return new mob_mana_warpAI(_Creature);
}

bool Spell_charge(const Aura* aura, bool apply)
{
    if(!apply)
    {
        if(Unit* caster = aura->GetCaster())
            caster->CastSpell(aura->GetTarget(), 29321, true);      // trigger fear after charge
    }
    return true;
}

#define SPELL_SEARING_PAIN      29492
#define SPELL_IMMOLATE          29928
#define SPELL_CURSE_OF_AGONY    29930

struct mob_shadow_pillagerAI : public ScriptedAI
{
    mob_shadow_pillagerAI(Creature* c) : ScriptedAI(c) {}

    Timer DotTimer;

    void Reset()
    {
        SetAutocast(SPELL_SEARING_PAIN, 2500, true);
        DotTimer.Reset(urand(2000, 6000));
    }
    
    void AttackStart(Unit *who)
    {
        ScriptedAI::AttackStartNoMove(who, CHECK_TYPE_CASTER);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;
        
        
        if (DotTimer.Expired(diff))
        {
            AddSpellToCast(roll_chance_i(50) ? SPELL_IMMOLATE : SPELL_CURSE_OF_AGONY, CAST_RANDOM);
            DotTimer = urand(2000, 8000);
        } 
        

        CheckCasterNoMovementInRange(diff, 40.0);
        CastNextSpellIfAnyAndReady(diff);
    }    
};

CreatureAI* GetAI_mob_shadow_pillager(Creature *_Creature)
{
    return new mob_shadow_pillagerAI(_Creature);
}


#define SPELL_FIREBOLT    30180

struct mob_homunculusAI : public ScriptedAI
{
    mob_homunculusAI(Creature* c) : ScriptedAI(c) {}

    void Reset()
    {
        SetAutocast(SPELL_FIREBOLT, 2000, true);
    }
    
    void AttackStart(Unit *who)
    {
        ScriptedAI::AttackStartNoMove(who, CHECK_TYPE_CASTER);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        CheckCasterNoMovementInRange(diff, 40.0);
        CastNextSpellIfAnyAndReady(diff);
    }    
};

CreatureAI* GetAI_mob_homunculus(Creature *_Creature)
{
    return new mob_homunculusAI(_Creature);
}

enum arcaneanomalyspells
{
    SPELL_AA_MANASHIELD = 29880,
    SPELL_AA_LOOSEMANA  = 29882,
    SPELL_AA_BLINK      = 29883,
    SPELL_AA_VOLLEY     = 29885
};

struct mob_arcane_anomalyAI : public ScriptedAI
{
    mob_arcane_anomalyAI(Creature* c) : ScriptedAI(c) {}


    Timer blinkTimer;
    Timer volleyTimer;
    bool shield;
    void Reset()
    {
        blinkTimer = urand(6000, 8000);
        volleyTimer = urand(8000, 10000);
        shield = false;
    }

    void DamageTaken(Unit* enemy, uint32& Damage)
    {
        if (m_creature->HasAura(SPELL_AA_MANASHIELD))
        {
            Damage = 0;
            return;
        }
        if (shield) // shield ended dont cast again
            return;

        m_creature->CastSpell(m_creature, SPELL_AA_MANASHIELD, true);
        shield = true;
        if (Damage >= m_creature->GetHealth())
            Damage = m_creature->GetHealth() - 1;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (blinkTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_AA_BLINK, CAST_RANDOM);
            blinkTimer = urand(10000, 12000);
        }

        if (volleyTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_AA_VOLLEY);
            volleyTimer = urand(10000, 15000);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

    void JustDied(Unit* who)
    {
        m_creature->CastSpell(m_creature, SPELL_AA_LOOSEMANA, true);
    }
};

CreatureAI* GetAI_mob_arcane_anomaly(Creature *_Creature)
{
    return new mob_arcane_anomalyAI(_Creature);
}

enum downstairs
{
    SPELL_KNOCKDOWN         = 18812,
    SPELL_PIERCE_ARMOUR     = 6016,
    SPELL_HEALING_TOUCH     = 29339,
    SPELL_FRENZY_WHIP       = 29340,

    SPELL_DEMORALIZING_ROAR = 29584,
    SPELL_STEALTH_DETECTION = 12418,

    SPELL_ALLURING_AURA     = 29485,
    SPELL_HOSTESS_TRANSFORM = 29472,
    SPELL_BEWITCHING_AURA   = 29486,
    SPELL_HOSTESS_WAIL      = 29477,
    SPELL_HOSTESS_SILENCE   = 29505,

    SPELL_NIGHT_TRANSFORM   = 29491,
    SPELL_NIGHT_SHADOWBOLT  = 29487,
    SPELL_NIGHT_IMPENDING   = 29488,

    SPELL_CONCUBINE_TRANSFORM = 29489,
    SPELL_CONCUBINE_SEDUCTION = 29490,
    SPELL_CONCUBINE_TEMPTATION = 29494,
    SPELL_CONCUBINE_JEALOUSY = 29497,

    NPC_HOSTESS         = 16459,
    NPC_NIGHT_MISTRESS  = 16460,
    NPC_CONCUBINE       = 16461,
};

struct mob_demon_ladiesAI : public ScriptedAI
{
    mob_demon_ladiesAI(Creature* c) : ScriptedAI(c) {}

    bool changed;
    Timer mainCastTimer;

    void Reset()
    {
        changed = false;
        mainCastTimer.Reset(urand(5000,7000));
    }

    void EnterCombat(Unit*)
    {
        if (m_creature->GetEntry() == NPC_HOSTESS)
            DoCast(m_creature, SPELL_ALLURING_AURA, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!changed && m_creature->HealthBelowPct(50))
        {
            changed = true;
            m_creature->RemoveAllAuras();
            switch (m_creature->GetEntry())
            {
            case NPC_HOSTESS:
                DoCast(m_creature, SPELL_BEWITCHING_AURA, true);
                DoCast(m_creature, SPELL_HOSTESS_TRANSFORM, true);
                break;
            case NPC_NIGHT_MISTRESS:
                DoCast(m_creature, SPELL_NIGHT_TRANSFORM, true);
                break;
            case NPC_CONCUBINE:
                DoCast(m_creature, SPELL_CONCUBINE_TRANSFORM, true);
                break;
            }
        }
        if (!UpdateVictim())
            return;

        if (mainCastTimer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 0, true))
            {
                switch (m_creature->GetEntry())
                {
                case NPC_HOSTESS:
                    DoCast(target, changed ? SPELL_HOSTESS_SILENCE : SPELL_HOSTESS_WAIL, false);
                    mainCastTimer = urand(8000, 15000);
                    break;
                case NPC_NIGHT_MISTRESS:
                    DoCast(target, changed ? SPELL_NIGHT_IMPENDING : SPELL_NIGHT_SHADOWBOLT, false);
                    mainCastTimer = urand(5000, 7000);
                    break;
                case NPC_CONCUBINE:
                    DoCast(target, changed ? SPELL_CONCUBINE_JEALOUSY : SPELL_CONCUBINE_TEMPTATION, false);
                    mainCastTimer = urand(8000, 12000);
                    break;
                }
            }
            else
                mainCastTimer = 1000;
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_demon_ladies(Creature* c)
{
    return new mob_demon_ladiesAI(c);
}

struct mob_spectral_stable_handAI : public ScriptedAI
{
    mob_spectral_stable_handAI(Creature* c) : ScriptedAI(c) {}

    Timer KnockdownTimer;
    Timer PierceTimer;
    Timer HealTimer;
    Timer FrenzyTimer;

    

    void Reset()
    {
        KnockdownTimer.Reset(urand(7000,10000));
        PierceTimer.Reset(urand(4000,6000));
        HealTimer.Reset(urand(15000,25000));
        FrenzyTimer.Reset(urand(15000,25000));
    }

    Unit* findHorse()
    {
        struct horsefinder
        {
            bool operator()(Unit* u)
            {
                if (u->GetTypeId() != TYPEID_UNIT || !u->IsAlive() || !u->IsInCombat())
                    return false;
                return (u->GetEntry() == 15547 || u->GetEntry() == 15548);
            }
        } u_check;
        std::list<Creature*> unitList;

        MaNGOS::ObjectListSearcher<Creature, horsefinder> searcher(unitList, u_check);
        Cell::VisitGridObjects(me, searcher, 100);

        if (unitList.size() == 0)
            return NULL;
        std::list<Creature*>::iterator i = unitList.begin();
        advance(i, urand(0, unitList.size() - 1));
        return (*i);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (KnockdownTimer.Expired(diff))
        {
            DoCastVictim(SPELL_KNOCKDOWN);
            KnockdownTimer = urand(12000,18000);
        }

        if (PierceTimer.Expired(diff))
        {
            DoCastVictim(SPELL_PIERCE_ARMOUR);
            PierceTimer = urand(30000, 45000);
        }

        if (HealTimer.Expired(diff))
        {
            if (Unit* horse = findHorse())
                DoCast(horse, SPELL_HEALING_TOUCH);
            HealTimer = urand(20000, 35000);
        }

        if (FrenzyTimer.Expired(diff))
        {
            if (Unit* horse = findHorse())
                DoCast(horse, SPELL_FRENZY_WHIP);
            FrenzyTimer = urand(20000, 35000);
        }

        DoMeleeAttackIfReady();
    }

    void JustDied(Unit*)
    {
        DoScriptText(RAND(-40, -41), m_creature);
    }
};

CreatureAI* GetAI_mob_spectral_stable_hand(Creature* c)
{
    return new mob_spectral_stable_handAI(c);
}

struct mob_phantom_valetAI : public ScriptedAI
{
    mob_phantom_valetAI(Creature* c) : ScriptedAI(c) {}

    Timer RoarTimer;

    void Reset()
    {
        RoarTimer.Reset(9000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->HasAura(SPELL_STEALTH_DETECTION))
            DoCast(m_creature, SPELL_STEALTH_DETECTION, true);
        if (!UpdateVictim())
            return;

        if (RoarTimer.Expired(diff))
        {
            DoCastVictim(SPELL_DEMORALIZING_ROAR);
            RoarTimer = 30000;
        }
    }

    void JustDied(Unit*)
    {
        DoScriptText(-51, m_creature);
    }
};

CreatureAI* GetAI_mob_phantom_valet(Creature* c)
{
    return new mob_phantom_valetAI(c);
}


void AddSC_karazhan_trash()
{
    Script* newscript;
    newscript = new Script;
    newscript->Name = "spell_charge_29320";
    newscript->pEffectAuraDummy = &Spell_charge;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_phantom_guest";
    newscript->GetAI = &GetAI_mob_phantom_guest;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_spectral_sentry";
    newscript->GetAI = &GetAI_mob_spectral_sentry;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_arcane_protector";
    newscript->GetAI = &GetAI_mob_arcane_protector;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_mana_warp";
    newscript->GetAI = &GetAI_mob_mana_warp;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_shadow_pillager";
    newscript->GetAI = &GetAI_mob_shadow_pillager;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_homunculus";
    newscript->GetAI = &GetAI_mob_homunculus;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_arcane_anomaly";
    newscript->GetAI = &GetAI_mob_arcane_anomaly;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_demon_ladies";
    newscript->GetAI = &GetAI_mob_demon_ladies;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_spectral_stable_hand";
    newscript->GetAI = &GetAI_mob_spectral_stable_hand;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_phantom_valet";
    newscript->GetAI = &GetAI_mob_phantom_valet;
    newscript->RegisterSelf();
}
