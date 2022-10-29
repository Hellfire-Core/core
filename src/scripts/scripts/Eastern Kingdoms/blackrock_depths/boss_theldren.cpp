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
#include "def_blackrock_depths.h"

#define BOSS_THELDREN                   16059

#define GO_ARENA_SPOILS                 211085

//Spells
#define SPELL_DRINK_HEALING_POTION      15503
//theldren
#define SPELL_BATTLE_SHOUT              11551
#define SPELL_DEMORALIZING_SHOUT        11556
#define SPELL_DISARM                    27581
#define SPELL_FRIGHTENING_SHOUT         19134
#define SPELL_HAMSTRING                 27584
#define SPELL_INTERCEPT                 20617
#define SPELL_MORTAL_STRIKE             27580
//malgen_longspear
#define SPELL_FREEZING_TRAP             27753
#define SPELL_CONCLUSSIVE_SHOT          27634
#define SPELL_AIMED_SHOT                30614
#define SPELL_MULTI_SHOT                20735
#define SPELL_SHOOT                     6660
#define SPELL_WING_CLIP                 44286
//Lefty
#define SPELL_KNOCKDOWN                 7095
#define SPELL_SNAP_KICK                 27620
#define SPELL_FFFEHT                    27673
//Rotfang
#define SPELL_EVISCERATE                31016
#define SPELL_GOUGE                     24698
#define SPELL_KICK                      11978
#define SPELL_KIDNEY_SHOT               27615
#define SPELL_SINISTER_STRIKE           26862
#define SPELL_SLOWING_POISON            14897
#define SPELL_VANISH                    44290
//Va'jashni
#define SPELL_DISPEL_MAGIC              988
#define SPELL_FLASH_HEAL                27608
#define SPELL_PW_SHIELD                 20697
#define SPELL_RENEW                     23895
#define SPELL_SW_PAIN                   10894
//Volida
#define SPELL_BLINK                     14514
#define SPELL_BLIZZARD                  27618
#define SPELL_CONE_OF_COLD              12557
#define SPELL_FROST_NOVA                15063
#define SPELL_FROSTBOLT                 36990
#define SPELL_ICE_BLOCK                 45439
//Snokh Blackspine
#define SPELL_FLAMESTRIKE               11829
#define SPELL_SCORCH                    13878
#define SPELL_BLAST_WAVE                38064
#define SPELL_PYROBLAST                 17273
#define SPELL_POLYMORPH                 13323
//Korv
#define SPELL_FROST_SHOCK               12548
#define SPELL_WAR_STOMP                 46026
#define SPELL_WINDFURY_TOTEM            27621
#define SPELL_EARTHBIND_TOTEM           15786
#define SPELL_LESSER_HEALING_WAVE       10468
#define SPELL_PURGE                     8012
//Rezznik
#define SPELL_RECOMBOBULATE             27677
#define SPELL_DARK_IRON_BOMB            19784
#define SPELL_GOBLIN_DRAGON_GUN         44272
#define SPELL_EXPLOSIVE_SHEEP           8209
#define SPELL_SUMMON_ADRAGONLING        27602

static uint32 AddEntryList[9]=
{
    16049,      //Lefty
    16050,      //Rotfang
    16052,      //Malgen Longspear
    16055,      //Vajashni
    16058,      //Volida
    16051,      //Snokh Blackspine
    16053,      //Korv
    16054,      //Rezznik
    16095       ///Gnashjaw     - PET
};

static float ArenaLocations[5][3]=
{
    {592.6309f, -179.561f, -53.90f},
    {594.6309f, -178.061f, -53.90f},
    {588.6309f, -182.561f, -53.90f},
    {586.6309f, -184.061f, -53.90f},
    {590.6309f, -181.061f, -53.90f}        // theldren

};
static float Orientation = 5.33f;

//////////////////////
//Theldren
//////////////////////
struct boss_theldrenAI : public ScriptedAI
{
    boss_theldrenAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = me->GetInstanceData();
        SelectAddEntry();
        for (uint8 i = 0; i < 4; ++i)
            AddGUID[i] = 0;
    }

    ScriptedInstance* pInstance;
    uint64 AddGUID[4];
    uint32 AddEntry[4];

    int32 BattleShout_Timer;
    int32 DemoralizingShout_Timer;
    int32 Disarm_Timer;
    int32 FrighteningShout_Timer;
    int32 Hamstring_Timer;
    int32 Intercept_Timer;
    int32 MortalStrike_Timer;
    bool DrinkHealingPotion_Used;

    void Reset()
    {
        if (pInstance && pInstance->GetData(TYPE_THELDREN)!= DONE)
            pInstance->SetData(TYPE_THELDREN, NOT_STARTED);

        BattleShout_Timer = 6000;
        DemoralizingShout_Timer = 3000;
        Disarm_Timer = 0;
        FrighteningShout_Timer = 2000;
        Hamstring_Timer = 5000;
        Intercept_Timer = 7000;
        MortalStrike_Timer = 6000;
        DrinkHealingPotion_Used = false;
        SpawnAdds();
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat();

        for (uint8 i = 0; i < 4; ++i)
        {
            Unit* Temp = Unit::GetUnit((*me),AddGUID[i]);
            if (Temp && Temp->IsAlive())
                ((Creature*)Temp)->AI()->AttackStart(me->GetVictim());
            else
            {
                EnterEvadeMode();
                break;
            }
        }
    }

    void JustDied(Unit* victim)
    {
        if (pInstance)
            pInstance->SetData(TYPE_THELDREN, DONE);
        victim->SummonGameObject(GO_ARENA_SPOILS, 596.664, -188.699, -54.1551, 5.67734, 0, 0, 0.298313, -0.954468, 0);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        BattleShout_Timer -= diff;
        if (BattleShout_Timer <= diff)
        {
            AddSpellToCast(me, SPELL_BATTLE_SHOUT);
            BattleShout_Timer += 10000;
        }
           

        DemoralizingShout_Timer -= diff;
        if (DemoralizingShout_Timer <= diff)
        {
            AddSpellToCast(me, SPELL_DEMORALIZING_SHOUT);
            DemoralizingShout_Timer += 120000;
        }
        

        Disarm_Timer -= diff;
        if (Disarm_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_DISARM);
            Disarm_Timer += 60000;
        }
        
            
        FrighteningShout_Timer -= diff;
        if (FrighteningShout_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_FRIGHTENING_SHOUT);
            FrighteningShout_Timer += 30000;
        }
           
        Hamstring_Timer -= diff;
        if (Hamstring_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_HAMSTRING);
            Hamstring_Timer += 30000;
        }
        
            
        Intercept_Timer -= diff;
        if (Intercept_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_INTERCEPT);
            Intercept_Timer += 25000;
        }
        
            
        MortalStrike_Timer -= diff;
        if (MortalStrike_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_MORTAL_STRIKE);
            MortalStrike_Timer += 15000;
        }
        
            

        if (!DrinkHealingPotion_Used && HealthBelowPct(50))
        {
            AddSpellToCast(me, SPELL_DRINK_HEALING_POTION);
            DrinkHealingPotion_Used = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

    void SelectAddEntry()
    {
        std::vector<uint32> AddList;

        for (uint8 i = 0; i < 8; ++i)
            AddList.push_back(AddEntryList[i]);

        while (AddList.size() > 4)
            AddList.erase(AddList.begin()+rand()%AddList.size());

        uint8 i = 0;
        for (std::vector<uint32>::iterator itr = AddList.begin(); itr != AddList.end(); ++itr, ++i)
            AddEntry[i] = *itr;
    }

    void SpawnAdds()
    {
        for (uint8 i = 0; i < 4; ++i)
        {
            Creature *pCreature = (Unit::GetCreature((*me), AddGUID[i]));
            if (!pCreature || !pCreature->IsAlive())
            {
                if (pCreature) pCreature->setDeathState(DEAD);
                pCreature = me->SummonCreature(AddEntry[i], ArenaLocations[i][0], ArenaLocations[i][1], ArenaLocations[i][2], Orientation, TEMPSUMMON_DEAD_DESPAWN, 0);
                if (pCreature) AddGUID[i] = pCreature->GetGUID();
            }
            else
            {
                pCreature->AI()->EnterEvadeMode();
                pCreature->Relocate(ArenaLocations[i][0], ArenaLocations[i][1], ArenaLocations[i][2], Orientation);
                pCreature->StopMoving();
            }
        }
    }
};

CreatureAI* GetAI_boss_theldren(Creature *creature)
{
    return new boss_theldrenAI (creature);
}

//////////////////////
///add malgen_longspear
//////////////////////
struct boss_malgen_longspearAI : public ScriptedAI
{
    boss_malgen_longspearAI(Creature *c) : ScriptedAI(c)
    {
        PetGUID = 0;
    }

    ScriptedInstance* pInstance;
    uint64 PetGUID;

    int32 FreezingTrap_Timer;
    int32 AimedShot_Timer;
    int32 ConclussiveShot_Timer;
    int32 MultiShot_Timer;
    int32 Shoot_Timer;
    int32 WingClip_Timer;
    bool DrinkHealingPotion_Used;

    void Reset()
    {
        FreezingTrap_Timer = 60000;
        AimedShot_Timer = 0;
        ConclussiveShot_Timer = 1000;
        MultiShot_Timer = 2000;
        Shoot_Timer = 0;
        WingClip_Timer = 9000;
        DrinkHealingPotion_Used = false;
        SpawnPet();
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat();

        Unit* Temp = Unit::GetUnit((*me),PetGUID);
        if (Temp && Temp->IsAlive())
            ((Creature*)Temp)->AI()->AttackStart(me->GetVictim());
        else
        {
            EnterEvadeMode();
        }
    }

    void JustDied(Unit* victim)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        DoStartNoMovement(me->GetVictim());

        FreezingTrap_Timer -= diff;
        if (FreezingTrap_Timer <= diff)
        {
            AddSpellToCast(me, SPELL_FREEZING_TRAP);
            FreezingTrap_Timer += 60000;
        }
        
            
        AimedShot_Timer -= diff;
        if (AimedShot_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_AIMED_SHOT);
            AimedShot_Timer += 10000;
        }
        
           
        ConclussiveShot_Timer -= diff;
        if (ConclussiveShot_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_CONCLUSSIVE_SHOT);
            ConclussiveShot_Timer += 8000;
        }
        
        MultiShot_Timer -= diff;
        if (MultiShot_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_MULTI_SHOT);
            MultiShot_Timer += 5000;
        }
        
        Shoot_Timer -= diff;
        if (Shoot_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_SHOOT);
            Shoot_Timer += 1500;
        }
        
        WingClip_Timer -= diff;
        if (WingClip_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_WING_CLIP, true);
            WingClip_Timer += 20000;
        }
        

        if (!DrinkHealingPotion_Used && HealthBelowPct(50))
        {
            AddSpellToCast(me, SPELL_DRINK_HEALING_POTION);
            DrinkHealingPotion_Used = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

    void SpawnPet()
    {
        Creature *pPet = (Unit::GetCreature((*me), PetGUID));
        if (!pPet || !pPet->IsAlive())
        {
            if (pPet) pPet->setDeathState(DEAD);
            pPet = me->SummonCreature(AddEntryList[8], me->GetPositionX(), me->GetPositionY()+2, me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);
            if (pPet) PetGUID = pPet->GetGUID();
        }
        else
        {
            pPet->AI()->EnterEvadeMode();
            pPet->Relocate(me->GetPositionX(), me->GetPositionY()+2, me->GetPositionZ(), me->GetOrientation());
            pPet->StopMoving();
        }
    }

};

CreatureAI* GetAI_boss_malgen_longspear(Creature *creature)
{
    return new boss_malgen_longspearAI (creature);
}

//////////////////////
///add lefty
//////////////////////
struct boss_leftyAI : public ScriptedAI
{
    boss_leftyAI(Creature *c) : ScriptedAI(c)
    {
    }

    ScriptedInstance* pInstance;
    uint64 PetGUID;

    int32 Knockdown_Timer;
    int32 SnapKick_Timer;
    int32 FFFEHT_Timer;
    int32 MultiShot_Timer;
    int32 Shoot_Timer;
    int32 WingClip_Timer;
    bool DrinkHealingPotion_Used;

    void Reset()
    {
        Knockdown_Timer = 6000;
        SnapKick_Timer = 0;
        FFFEHT_Timer = 1000;
        DrinkHealingPotion_Used = false;
    }

    void EnterCombat(Unit* who)
    {
    }

    void JustDied(Unit* victim)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        Knockdown_Timer -= diff;
        if (Knockdown_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_KNOCKDOWN);
            Knockdown_Timer += 30000;
        }
        

        SnapKick_Timer -= diff;
        if (SnapKick_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_SNAP_KICK);
            SnapKick_Timer += 15000;
        }
        

        FFFEHT_Timer -= diff;
        if (FFFEHT_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_FFFEHT);
            FFFEHT_Timer += 30000;
        }
          

        if(!DrinkHealingPotion_Used && HealthBelowPct(50))
        {
            AddSpellToCast(me, SPELL_DRINK_HEALING_POTION);
            DrinkHealingPotion_Used = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_lefty(Creature *_Creature)
{
    return new boss_leftyAI (_Creature);
}

//////////////////////
///add rotfang
//////////////////////
struct boss_rotfangAI : public ScriptedAI
{
    boss_rotfangAI(Creature *c) : ScriptedAI(c)
    {
    }

    int32 Eviscerate_Timer;
    int32 Gouge_Timer;
    int32 Kick_Timer;
    int32 KidneyShot_Timer;
    int32 SinisterStrike_Timer;
    int32 SlowingPoison_Timer;
    int32 Vanish_Timer;
    bool DrinkHealingPotion_Used;

    void Reset()
    {
        Eviscerate_Timer = 6000;
        Gouge_Timer = 3000;
        Kick_Timer = 0;
        KidneyShot_Timer = 2000;
        SinisterStrike_Timer = 5000;
        SlowingPoison_Timer = 7000;
        Vanish_Timer = 6000;
        DrinkHealingPotion_Used = false;
    }

    void EnterCombat(Unit* who)
    {}

    void JustDied(Unit* victim)
    {}

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        Eviscerate_Timer -= diff;
        if (Eviscerate_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_EVISCERATE);
            Eviscerate_Timer += 10000;
        }
        

        Gouge_Timer -= diff;
        if (Gouge_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_GOUGE);
            Gouge_Timer += 120000;
        }
        
        Kick_Timer -= diff;
        if (Kick_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_KICK);
            Kick_Timer += 60000;
        }
        
        KidneyShot_Timer -= diff;
        if (KidneyShot_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_KIDNEY_SHOT);
            KidneyShot_Timer += 30000;
        }
        
        SinisterStrike_Timer -= diff;
        if (SinisterStrike_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_SINISTER_STRIKE);
            SinisterStrike_Timer += 30000;
        }
        
        SlowingPoison_Timer -= diff;
        if (SlowingPoison_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_SLOWING_POISON);
            SlowingPoison_Timer += 25000;
        }
        
        Vanish_Timer -= diff;
        if (Vanish_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_VANISH);
            Vanish_Timer += 15000;
        }
       
            

        if(HealthBelowPct(50) && !DrinkHealingPotion_Used)
        {
            AddSpellToCast(me, SPELL_DRINK_HEALING_POTION);
            DrinkHealingPotion_Used = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_rotfang(Creature *_Creature)
{
    return new boss_rotfangAI (_Creature);
}

//////////////////////
///add Va'jashni
//////////////////////
struct boss_vajashniAI : public ScriptedAI
{
    boss_vajashniAI(Creature *c) : ScriptedAI(c)
    {
    }

    int32 DispelMagic_Timer;
    int32 FlashHeal_Timer;
    int32 PWShield_Timer;
    int32 Renew_Timer;
    int32 SWPain_Timer;
    bool DrinkHealingPotion_Used;

    void Reset()
    {
        DispelMagic_Timer = 2000;
        FlashHeal_Timer = 5000;
        PWShield_Timer = 0;
        Renew_Timer = 1000;
        SWPain_Timer = 0;
        DrinkHealingPotion_Used = false;
    }

    void EnterCombat(Unit* who)
    {}

    void JustDied(Unit* victim)
    {}

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        DoStartNoMovement(me->GetVictim());

        DispelMagic_Timer -= diff;
        if (DispelMagic_Timer <= diff)
        {
            if(rand()%2)
            {
                if(Unit* target = SelectLowestHpFriendly(50, 0))
                    AddSpellToCast(target, SPELL_DISPEL_MAGIC);
            }
            else if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_DISPEL_MAGIC), true))
            {
                AddSpellToCast(target, SPELL_DISPEL_MAGIC);
            }
            DispelMagic_Timer += 6000;
        }
        
        FlashHeal_Timer -= diff;
        if (FlashHeal_Timer <= diff)
        {
            if(Unit* target = SelectLowestHpFriendly(50, 1000))
            {
                AddSpellToCast(target, SPELL_FLASH_HEAL);
                FlashHeal_Timer += 5000;
            }
        }
        
        PWShield_Timer -= diff;
        if (PWShield_Timer <= diff)
        {
            if(Unit* target = SelectLowestHpFriendly(50, 0))
            {
                AddSpellToCast(target, SPELL_PW_SHIELD);
                PWShield_Timer += 20000;
            }
        }
        
        Renew_Timer -= diff;
        if (Renew_Timer <= diff)
        {
            if(Unit* target = SelectLowestHpFriendly(50, 0))
            {
                AddSpellToCast(target, SPELL_RENEW);
                Renew_Timer += 10000;
            }
        }
        
           
        SWPain_Timer -= diff;
        if (SWPain_Timer <= diff)
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_SW_PAIN), true))
            {
                AddSpellToCast(target, SPELL_SW_PAIN);
                SWPain_Timer += 7000;
            }
        }
        
          

        if(HealthBelowPct(50) && !DrinkHealingPotion_Used)
        {
            AddSpellToCast(me, SPELL_DRINK_HEALING_POTION);
            DrinkHealingPotion_Used = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_boss_vajashni(Creature *_Creature)
{
    return new boss_vajashniAI (_Creature);
}

//////////////////////
///add Volida
//////////////////////
struct boss_volidaAI : public ScriptedAI
{
    boss_volidaAI(Creature *c) : ScriptedAI(c)
    {
    }

    int32 Blink_Timer;
    int32 Blizzard_Timer;
    int32 ConeOfCold_Timer;
    int32 FrostNova_Timer;
    int32 FrostBolt_Timer;
    int32 IceBlock_Timer;
    bool DrinkHealingPotion_Used;

    void Reset()
    {
        Blink_Timer = 2000;
        Blizzard_Timer = 5000;
        ConeOfCold_Timer = 0;
        FrostNova_Timer = 1000;
        FrostBolt_Timer = 0;
        IceBlock_Timer = 0;
        DrinkHealingPotion_Used = false;
    }

    void EnterCombat(Unit* who)
    {}

    void JustDied(Unit* victim)
    {}

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        DoStartNoMovement(me->GetVictim());

        Blink_Timer -= diff;
        if (Blink_Timer <= diff)
        {
            AddSpellToCast(me, SPELL_BLINK);
            Blink_Timer += 30000;
        }
        
        Blizzard_Timer -= diff;
        if (Blizzard_Timer <= diff)
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_BLIZZARD), true))
            {
                AddSpellToCast(target, SPELL_BLIZZARD);
                Blizzard_Timer += 20000;
            }
        }
        
            
        ConeOfCold_Timer -= diff;
        if (ConeOfCold_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_CONE_OF_COLD);
            ConeOfCold_Timer += 15000;
        }
        
        FrostNova_Timer -= diff;
        if (FrostNova_Timer <= diff)
        {
            AddSpellToCast(me, SPELL_FROST_NOVA);
            FrostNova_Timer += 25000;
        }
        
        FrostBolt_Timer -= diff;
        if (FrostBolt_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_FROSTBOLT);
            FrostBolt_Timer += 3500;
        }
        
        IceBlock_Timer -= diff;
        if (HealthBelowPct(20) && IceBlock_Timer <= diff)
        {
            AddSpellToCast(me, SPELL_ICE_BLOCK);
            IceBlock_Timer += 30000;
        }
        

        if(HealthBelowPct(50) && !DrinkHealingPotion_Used)
        {
            AddSpellToCast(me, SPELL_DRINK_HEALING_POTION);
            DrinkHealingPotion_Used = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_boss_volida(Creature *_Creature)
{
    return new boss_volidaAI (_Creature);
}

//////////////////////
///add Snokh Blackspine
//////////////////////
struct boss_snokhAI : public ScriptedAI
{
    boss_snokhAI(Creature *c) : ScriptedAI(c)
    {
    }

    int32 Blink_Timer;
    int32 FlameStrike_Timer;
    int32 Scorch_Timer;
    int32 BlastWave_Timer;
    int32 Pyroblast_Timer;
    int32 Polymorph_Timer;
    bool DrinkHealingPotion_Used;

    void Reset()
    {
        Blink_Timer = 2000;
        FlameStrike_Timer = 1000;
        Scorch_Timer = 0;
        BlastWave_Timer = 1000;
        Pyroblast_Timer = 5000;
        Polymorph_Timer = 2000;
        DrinkHealingPotion_Used = false;
    }

    void EnterCombat(Unit* who)
    {}

    void JustDied(Unit* victim)
    {}

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        DoStartNoMovement(me->GetVictim());

        Blink_Timer -= diff;
        if (Blink_Timer <= diff)
        {
            AddSpellToCast(me, SPELL_BLINK);
            Blink_Timer += 30000;
        }
        
        FlameStrike_Timer -= diff;
        if (FlameStrike_Timer <= diff)
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_FLAMESTRIKE), true))
            {
                AddSpellToCast(target, SPELL_BLIZZARD);
                FlameStrike_Timer += 7000;
            }
        }
        

        Scorch_Timer -= diff;
        if (Scorch_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_SCORCH);
            Scorch_Timer += 5000;
        }
        
            
        BlastWave_Timer -= diff;
        if (BlastWave_Timer <= diff)
        {
            AddSpellToCast(me, SPELL_BLAST_WAVE);
            BlastWave_Timer += 15000;
        }
        
        Pyroblast_Timer -= diff;
        if (Pyroblast_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_PYROBLAST);
            Pyroblast_Timer += 25000;
        }
        
          
            Polymorph_Timer -= diff;
        if (Polymorph_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_POLYMORPH);
            Polymorph_Timer += 15000;
        }
        

        if(HealthBelowPct(50) && !DrinkHealingPotion_Used)
        {
            AddSpellToCast(me, SPELL_DRINK_HEALING_POTION);
            DrinkHealingPotion_Used = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_boss_snokh(Creature *_Creature)
{
    return new boss_snokhAI (_Creature);
}

//////////////////////
///add Korv
//////////////////////
struct boss_korvAI : public ScriptedAI
{
    boss_korvAI(Creature *c) : ScriptedAI(c)
    {
    }

    int32 FrostShock_Timer;
    int32 WarStamp_Timer;
    int32 WindfuryT_Timer;
    int32 EarthbindT_Timer;
    int32 LesserHealing_Timer;
    int32 Purge_Timer;
    bool DrinkHealingPotion_Used;

    void Reset()
    {
        FrostShock_Timer = 2000;
        WarStamp_Timer = 1000;
        WindfuryT_Timer = 0;
        EarthbindT_Timer = 0;
        LesserHealing_Timer = 5000;
        Purge_Timer = 2000;
        DrinkHealingPotion_Used = false;
    }

    void EnterCombat(Unit* who)
    {}

    void JustDied(Unit* victim)
    {}

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        FrostShock_Timer -= diff;
        if (FrostShock_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_FROST_SHOCK);
            FrostShock_Timer += 10000;
        }
        
        WarStamp_Timer -= diff;
        if (WarStamp_Timer <= diff)
        {
            AddSpellToCast(me, SPELL_WAR_STOMP);
            WarStamp_Timer += 15000;
        }
        
        Purge_Timer -= diff;
        if (Purge_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_PURGE);
            Purge_Timer += 7000;
        }
        
        LesserHealing_Timer -= diff;
        if (LesserHealing_Timer <= diff)
        {
            if(Unit* target = SelectLowestHpFriendly(50, 1000))
            {
                AddSpellToCast(target,SPELL_LESSER_HEALING_WAVE);
                LesserHealing_Timer += 6500;
            }
        }
        
            
        WindfuryT_Timer -= diff;
        if (WindfuryT_Timer <= diff)
        {
            AddSpellToCast(me, SPELL_WINDFURY_TOTEM);
            WindfuryT_Timer += 25000;
        }
        

        EarthbindT_Timer -= diff;
        if (EarthbindT_Timer <= diff)
        {
            AddSpellToCast(me, SPELL_EARTHBIND_TOTEM);
            EarthbindT_Timer += 25000;
        }
        

        if(HealthBelowPct(50) && !DrinkHealingPotion_Used)
        {
            AddSpellToCast(me, SPELL_DRINK_HEALING_POTION);
            DrinkHealingPotion_Used = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_korv(Creature *_Creature)
{
    return new boss_korvAI (_Creature);
}

//////////////////////
///add Rezznik
//////////////////////
struct boss_rezznikAI : public ScriptedAI
{
    boss_rezznikAI(Creature *c) : ScriptedAI(c)
    {
    }

    int32 Recombobulate_Timer;
    int32 DarkIronBomb_Timer;
    int32 GoblinGragonGun_Timer;
    int32 ExplosiveSheep_Timer;
    int32 SummonADragonling_Timer;
    bool DrinkHealingPotion_Used;

    void Reset()
    {
        Recombobulate_Timer = 1000;
        DarkIronBomb_Timer = 1000;
        GoblinGragonGun_Timer = 0;
        ExplosiveSheep_Timer = 0;
        SummonADragonling_Timer = 5000;
        DrinkHealingPotion_Used = false;
    }

    void EnterCombat(Unit* who)
    {}

    void JustDied(Unit* victim)
    {}

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        Recombobulate_Timer -= diff;
        if (Recombobulate_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_RECOMBOBULATE);
            Recombobulate_Timer += 11000;
        }
        
        DarkIronBomb_Timer -= diff;
        if (DarkIronBomb_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_DARK_IRON_BOMB);
            DarkIronBomb_Timer += 4000;
        }
        
        GoblinGragonGun_Timer -= diff;
        if (GoblinGragonGun_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_GOBLIN_DRAGON_GUN);
            GoblinGragonGun_Timer += 12000;
        }
        
        ExplosiveSheep_Timer -= diff;
        if (ExplosiveSheep_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_EXPLOSIVE_SHEEP);
            ExplosiveSheep_Timer += 20000;
        }
        
        SummonADragonling_Timer -= diff;
        if (SummonADragonling_Timer <= diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_SUMMON_ADRAGONLING);
            SummonADragonling_Timer += 3600000;
        }
        

        if(HealthBelowPct(50) && !DrinkHealingPotion_Used)
        {
            AddSpellToCast(me, SPELL_DRINK_HEALING_POTION);
            DrinkHealingPotion_Used = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_rezznik(Creature *_Creature)
{
    return new boss_rezznikAI (_Creature);
}

void AddSC_boss_theldren()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_theldren";
    newscript->GetAI = &GetAI_boss_theldren;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_malgen_longspear";
    newscript->GetAI = &GetAI_boss_malgen_longspear;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_lefty";
    newscript->GetAI = &GetAI_boss_lefty;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_rotfang";
    newscript->GetAI = &GetAI_boss_rotfang;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_vajashni";
    newscript->GetAI = &GetAI_boss_vajashni;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_volida";
    newscript->GetAI = &GetAI_boss_volida;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_snokh";
    newscript->GetAI = &GetAI_boss_snokh;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_korv";
    newscript->GetAI = &GetAI_boss_korv;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_rezznik";
    newscript->GetAI = &GetAI_boss_rezznik;
    newscript->RegisterSelf();
}
