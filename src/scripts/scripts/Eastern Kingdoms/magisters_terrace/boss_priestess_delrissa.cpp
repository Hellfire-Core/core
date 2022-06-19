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
SDName: Boss_Priestess_Delrissa
SD%Complete: 85
SDComment: Verify timers, long and tiring debugging :)
SDCategory: Magister's Terrace
EndScriptData */

#include "precompiled.h"
#include "def_magisters_terrace.h"

struct Speech
{
    int32 id;
};

static Speech LackeyDeath[]=
{
    {-1585013},
    {-1585014},
    {-1585015},
    {-1585016},
};

static Speech PlayerDeath[]=
{
    {-1585017},
    {-1585018},
    {-1585019},
    {-1585020},
    {-1585021},
};

#define SAY_AGGRO                   -1585012
#define SAY_DEATH                   -1585022

#define SPELL_DISPEL_MAGIC          27609
#define SPELL_FLASH_HEAL            17843
#define SPELL_PHYSIC_SCREAM         27610
#define SPELL_WEAKENED_SOUL         6788
#define SPELL_SHIELD                (HeroicMode?46193:44175)
#define SPELL_SW_PAIN               (HeroicMode?15654:14032)
#define SPELL_RENEW                 (HeroicMode?46192:44174)
#define SPELL_MEDALION_OF_IMMUNITY  46227

#define ORIENT                      4.98f
#define POS_Z                       -19.9215f

static float LackeyLocations[4][2]=
{
    {123.77f,  17.6007f},
    {131.731f, 15.0827f},
    {121.563f, 15.6213f},
    {129.988f, 17.2355f},
};

static const uint32 AddEntry[8]=
{
    24557,                                                  //Kagani Nightstrike
    24558,                                                  //Elris Duskhallow
    24554,                                                  //Eramas Brightblaze
    24561,                                                  //Yazzaj
    24559,                                                  //Warlord Salaris
    24555,                                                  //Garaxxas
    24553,                                                  //Apoko
    24556,                                                  //Zelfan
};

struct Add
{
    Add(uint32 _entry, uint64 _guid)
    {
        entry = _entry;
        guid  = _guid;
    }

    uint32 entry;
    uint64 guid;
};

struct boss_priestess_delrissaAI : public ScriptedAI
{
    boss_priestess_delrissaAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        me->GetPosition(wLoc);
        Adds.clear();
    }

    ScriptedInstance* pInstance;
    WorldLocation wLoc;

    std::vector<Add> Adds;

    uint8 LackeysKilled;
    uint8 PlayersKilled;

    bool canFear;
    bool canUseMedalion;
    bool aggroSpeach;

    Timer Heal_Timer;
    Timer Renew_Timer;
    Timer Scream_Cooldown;
    Timer Medalion_Cooldown;
    Timer Shield_Timer;
    Timer SWPain_Timer;
    Timer Dispel_Timer;
    Timer Check_Timer;
    Timer ResetThreatTimer;

    void Reset()
    {
        m_creature->SetCorpseDelay(60*60*1000);
        LackeysKilled = 0;
        PlayersKilled = 0;

        canFear = true;
        canUseMedalion = true;
        aggroSpeach = false;

        Heal_Timer.Reset(15000);
        Renew_Timer.Reset(10000);
        Scream_Cooldown.Reset(HeroicMode ? 15000 : 30000);
        Medalion_Cooldown.Reset(60000);
        Shield_Timer.Reset(2000);
        SWPain_Timer.Reset(5000);
        Dispel_Timer.Reset(7500);
        Check_Timer.Reset(2000);
        ResetThreatTimer.Reset(urand(3000, 8000));
        me->setActive(true);

        CheckAdds();

        if(pInstance)
        {
            pInstance->SetData(DATA_DELRISSA_DEATH_COUNT, 0);
            pInstance->SetData(DATA_DELRISSA_EVENT, NOT_STARTED);
        }
    }

    void EnterEvadeMode()
    {
        ScriptedAI::EnterEvadeMode();

        for(uint8 i = 0; i < Adds.size(); ++i)
        {
            if(Creature* pAdd = m_creature->GetCreature(Adds[i].guid))
                pAdd->AI()->EnterEvadeMode();
        }
    }

    void EnterCombat(Unit* who)
    {
        if(!aggroSpeach)
        {
            DoScriptText(SAY_AGGRO, me);
            aggroSpeach= true;
        }

        for(uint8 i = 0; i < Adds.size(); ++i)
        {
            if(Creature* pAdd = m_creature->GetCreature(Adds[i].guid))
            {
                pAdd->AI()->AttackStart(who);
                pAdd->AddThreat(who, 1.0f);
            }
        }
    }

    void SummonAdds()
    {
        std::set<uint32> AddList;

        while(AddList.size() < 4)
        {
            uint8 i = urand(0, 7);
            if(AddList.find(AddEntry[i]) == AddList.end())
                AddList.insert(AddEntry[i]);
        }

        uint8 i = 0;

        for(std::set<uint32>::iterator iter = AddList.begin(); iter != AddList.end(); ++iter)
        {
            Creature* pAdd = m_creature->SummonCreature((*iter), LackeyLocations[i][0], LackeyLocations[i][1], POS_Z, ORIENT, TEMPSUMMON_DEAD_DESPAWN, 0);
            if(pAdd)
            {
                Add nAdd((*iter), pAdd->GetGUID());
                Adds.push_back(nAdd);
                ++i;
            }
        }
    }

    void CheckAdds()
    {
        if(Adds.empty())
        {
            SummonAdds();
            return;
        }
        for(uint8 i = 0; i < Adds.size(); ++i)
        {
            Creature* pAdd = m_creature->GetCreature(Adds[i].guid);
            if(pAdd && pAdd->isAlive())
            {
                pAdd->AI()->EnterEvadeMode();
                pAdd->GetMotionMaster()->MovePoint(0,LackeyLocations[i][0], LackeyLocations[i][1], POS_Z);
            }
            if(!pAdd || (pAdd && pAdd->isDead()))
            {
                if(pAdd)
                    pAdd->RemoveCorpse();//looks stupid if mob is alive but has a dead corpse in front of him :)
                Creature* pAdd = m_creature->SummonCreature(Adds[i].entry, LackeyLocations[i][0], LackeyLocations[i][1], POS_Z, ORIENT, TEMPSUMMON_DEAD_DESPAWN, 0);
                if(pAdd)
                    Adds[i].guid = pAdd->GetGUID();
            }
        }
    }

    void KilledUnit(Unit* victim)
    {
        if(victim->GetTypeId() != TYPEID_PLAYER)
            return;

        if(m_creature->isAlive())
            DoScriptText(PlayerDeath[PlayersKilled].id, m_creature);
        if( PlayersKilled < 4 )
            ++PlayersKilled;
    }

    void KilledLackey()
    {
        if(m_creature->isAlive())
        {
            DoScriptText(LackeyDeath[LackeysKilled].id, m_creature);
            if(LackeysKilled < 3)
                ++LackeysKilled;
        }
        CheckLootable();
    }

    void JustDied(Unit* killer)
    {
        if(pInstance)
            pInstance->SetData(DATA_DELRISSA_DEATH_COUNT, 1);
        DoScriptText(SAY_DEATH, m_creature);
        CheckLootable();
    }

    void CheckLootable()
    {
        if(pInstance && pInstance->GetData(DATA_DELRISSA_DEATH_COUNT) >= 5) // Delrissa also counts
        {
            m_creature->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
            pInstance->SetData(DATA_DELRISSA_EVENT, DONE);
        }
        else
            m_creature->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
    }

    Unit* SelectLowestHpFriendlyMissingBuff(float range, uint32 spellid)
    {
        std::list<Creature*> missingBuff = FindFriendlyMissingBuff(range, spellid);
        float healthPct = 100;
        float newHealthPct;
        Unit* target = NULL;

        for(std::list<Creature*>::iterator it = missingBuff.begin(); it != missingBuff.end(); ++it)
        {
            Creature *c = *it;
            newHealthPct = ((float)c->GetHealth()) / c->GetMaxHealth();
            if(c->isAlive() && !c->HasAura(spellid, 0) && newHealthPct < healthPct)
            {
                healthPct = newHealthPct;
                target = c;
            }
        }
        return target;
    }

    void RegenMana()
    {
        uint32 maxMana = me->GetMaxPower(POWER_MANA);
        uint32 Mana = me->GetPower(POWER_MANA);
        me->SetPower(POWER_MANA, Mana+(HeroicMode?0.04:0.03)*maxMana);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(me->getVictim()->isCrowdControlled())
            DoModifyThreatPercent(me->getVictim(), -100);


        if (Check_Timer.Expired(diff))
        {
            DoZoneInCombat();
            RegenMana();
            if(fabs(me->GetPositionZ() - wLoc.coord_z) > 10.0)
            {
                EnterEvadeMode();
                return;
            }
            if(!me->IsWithinDistInMap(&wLoc, 80.0))
                DoTeleportTo(wLoc.coord_x, wLoc.coord_y, wLoc.coord_z);
            if(HeroicMode && canUseMedalion)
            {
                if(me->isCrowdControlled())
                {
                    DoCast(me, SPELL_MEDALION_OF_IMMUNITY, true);
                    canUseMedalion = false;
                }
            }
            if(canFear && me->IsWithinMeleeRange(me->getVictim()))
            {
                ForceSpellCast(SPELL_PHYSIC_SCREAM, CAST_SELF, INTERRUPT_AND_CAST);
                canFear = false;
            }
            else
            Check_Timer = 2000;
        }



        if (ResetThreatTimer.Expired(diff))
        {
            DoResetThreat();
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 200, true))
            {
                AttackStart(target);
                me->AddThreat(target, 200000);
            }
            ResetThreatTimer = urand(8000, 20000);
        }

        if(!canFear)
        {

            if (Scream_Cooldown.Expired(diff))
            {
                canFear = true;
                Scream_Cooldown = HeroicMode?15000:30000;
            }
        }

        if(HeroicMode && !canUseMedalion)
        {

            if (Medalion_Cooldown.Expired(diff))
            {
                canUseMedalion = true;
                Medalion_Cooldown = 60000;
            }

        }


        if (Heal_Timer.Expired(diff))
        {

            if (Unit* target = SelectLowestHpFriendly(40, 200))
            {
                AddSpellToCast(target, SPELL_FLASH_HEAL, false, true);
                if (target->GetHealth() * 100 / target->GetMaxHealth() < 50)
                    Heal_Timer = 3000;
                else
                    Heal_Timer = urand(4000, 10000);
                return;
            }
                Heal_Timer = 2000;
        }


        if (Renew_Timer.Expired(diff))
        {

            if(Unit *target = SelectLowestHpFriendlyMissingBuff(40, SPELL_RENEW))
            {
                AddSpellToCast(target, SPELL_RENEW);
                if(target->GetHealth() * 100 / target->GetMaxHealth() < 40)
                    Renew_Timer = 4000;
                else
                    Renew_Timer = 8000;
                return;
            }
            Renew_Timer = 1000;
        }


        if (Shield_Timer.Expired(diff))
        {
            if(Unit *target = SelectLowestHpFriendlyMissingBuff(40, SPELL_WEAKENED_SOUL))
            {
                ForceSpellCast(target, SPELL_SHIELD, INTERRUPT_AND_CAST);
                Shield_Timer = 15000;
                return;
            }
        Shield_Timer = 1000;
        }


        if (Dispel_Timer.Expired(diff))
        {
            Unit* target = NULL;

            std::list<Creature*> friendlyCC;
            switch(rand()%3)
            {
                case 0:
                    target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30, true);
                    if(target)
                        break;
                case 1:
                    friendlyCC = FindFriendlyCC(30);
                    if(!friendlyCC.empty())
                    {
                        target = *friendlyCC.begin();
                        break;
                    }
                case 2:
                    target = SelectLowestHpFriendly(30);
                    break;
            }

            if(target)
            {
                AddSpellToCast(target, SPELL_DISPEL_MAGIC);
                Dispel_Timer = 10000;
                return;
            }
            Dispel_Timer = 1000;
        }


        if (SWPain_Timer.Expired(diff))
        {

            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30, true, -SPELL_SW_PAIN))
            {
                if(!target->isCrowdControlled())
                {
                    AddSpellToCast(target,SPELL_SW_PAIN);
                    SWPain_Timer = 10000;
                    return;
                }
                SWPain_Timer = 1000;
            }
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

#define SPELL_HEALING_POTION        15503

struct boss_priestess_guestAI : public ScriptedAI
{
    boss_priestess_guestAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
        c->GetPosition(HomePos);
    }

    ScriptedInstance* pInstance;
    WorldLocation HomePos;

    bool resetThreat;
    bool canUseMedalion;
    bool usedPotion;
    Timer ResetThreatTimer;
    Timer Check_Timer;
    Timer Medalion_Cooldown;
    float targetRange;

    void Reset()
    {
        usedPotion = false;
        resetThreat = true;
        canUseMedalion = true;
        ResetThreatTimer.Reset(urand(3000, 8000));             // These guys like to switch targets often, and are not meant to be tanked.
        Check_Timer.Reset(2000);
        Medalion_Cooldown.Reset(60000);
        targetRange = 100;
        me->setActive(true);

        if(pInstance)
        {
            if(Creature *boss = m_creature->GetCreature(pInstance->GetData64(DATA_DELRISSA)))
            {
                me->SetOrientation(boss->GetOrientation());
                if(boss->isDead())
                    boss->Respawn();
            }
        }
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat();
        if(pInstance)
        {
            if(Creature* Delrissa = m_creature->GetCreature(pInstance->GetData64(DATA_DELRISSA)))
                ((boss_priestess_delrissaAI*)Delrissa->AI())->EnterCombat(who);
        }
    }

    void JustDied(Unit* killer)
    {
        if(pInstance)
        {
            pInstance->SetData(DATA_DELRISSA_DEATH_COUNT, 1);
            if(Creature* Delrissa = m_creature->GetCreature(pInstance->GetData64(DATA_DELRISSA)))
                ((boss_priestess_delrissaAI*)Delrissa->AI())->KilledLackey();
        }
    }

    void KilledUnit(Unit* victim)
    {
        if(pInstance)
        {
            if(Creature* Delrissa = m_creature->GetCreature(pInstance->GetData64(DATA_DELRISSA)))
                Delrissa->AI()->KilledUnit(victim);
        }
    }

    void TeleportPlayers()
    {
        Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
        {
            if (Player* player = i->getSource())
            {
                if (player->isGameMaster())
                    continue;
                player->TeleportTo(me->GetMapId(), 127.0f, 0.0f, -20.5f, player->GetOrientation(), TELE_TO_NOT_LEAVE_COMBAT);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(HealthBelowPct(25) && !usedPotion)
        {
            ClearCastQueue();
            AddSpellToCast(m_creature, SPELL_HEALING_POTION);
            usedPotion = true;
        }


        if (Check_Timer.Expired(diff))
        {
            if(HeroicMode && canUseMedalion)
            {
                if(me->isCrowdControlled())
                {
                    DoCast(me, SPELL_MEDALION_OF_IMMUNITY, true);
                    canUseMedalion = false;
                }
            }
            if(!me->IsWithinDistInMap(&HomePos, 80.0))
            {
                DoTeleportTo(HomePos.coord_x, HomePos.coord_y, HomePos.coord_z);
                TeleportPlayers();
            }
            if(me->GetPositionZ() < -22.5)
                DoTeleportTo(me->GetPositionX(), me->GetPositionY(), -19.5);
            Check_Timer = 2000;
        }


        if(HeroicMode && !canUseMedalion)
        {

            if (Medalion_Cooldown.Expired(diff))
            {
                canUseMedalion = true;
                Medalion_Cooldown = 60000;
            }

        }

        if(resetThreat)
        {
            if (ResetThreatTimer.Expired(diff))
            {
                DoResetThreat();
                if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, targetRange, true))
                {
                    AttackStart(target);
                    me->AddThreat(target, 200000);
                }
                ResetThreatTimer = urand(8000, 20000);
            }

        }
    }
};

#define SPELL_GOUGE              12540
#define SPELL_KICK               27613
#define SPELL_KIDNEY_SHOT        27615
#define SPELL_VANISH             44290
#define SPELL_BACKSTAB           (HeroicMode?15582:15657)
#define SPELL_EVISCERATE         (HeroicMode?46189:27611)
#define SPELL_DUALWIELD          29651
#define SPELL_CRIPPLING_POISON   44289

struct boss_kagani_nightstrikeAI : public boss_priestess_guestAI
{
    //Rogue
    boss_kagani_nightstrikeAI(Creature *c) : boss_priestess_guestAI(c) {}

    Timer Gouge_Timer;
    Timer Kick_Cooldown;
    Timer Vanish_Timer;
    Timer Eviscerate_Timer;
    Timer Backstab_Timer;
    Timer Check_Timer;

    bool canKick;
    bool InVanish;

    void Reset()
    {
        boss_priestess_guestAI::Reset();
        Gouge_Timer.Reset(5500);
        Kick_Cooldown.Reset(7000);
        Vanish_Timer.Reset((5000, 15000));
        Eviscerate_Timer.Reset(urand(6000, 12000));
        Backstab_Timer.Reset(3000);
        Check_Timer = 0;
        canKick = true;
        InVanish = false;
        resetThreat = false;

        DoCast(m_creature, SPELL_DUALWIELD);
    }

    void DamageMade(Unit* target, uint32 & damage, bool direct_damage, uint8 school_mask)
    {
        if(target->HasAura(SPELL_CRIPPLING_POISON, 0))
            return;

        if(damage && direct_damage && roll_chance_f(50))
            DoCast(target, SPELL_CRIPPLING_POISON, true);
    }

    void OnAuraRemove(Aura *aur, bool stackRemove)
    {
        if(aur->GetId() == 26888)
        {
            AddSpellToCast(SPELL_KIDNEY_SHOT, CAST_TANK);
            InVanish = false;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(InVanish)
        {
            if (Check_Timer.Expired(diff))
            {
                if(me->IsWithinMeleeRange(me->getVictim()))
                {
                    if(!me->getVictim()->HasInArc(M_PI, me))
                        AddSpellToCast(me->getVictim(), SPELL_BACKSTAB);
                    me->RemoveAurasDueToSpell(SPELL_VANISH);
                    Gouge_Timer = urand(10000, 15000);
                }
                Check_Timer = 2000;
            }


            CastNextSpellIfAnyAndReady();
            return;
        }

        boss_priestess_guestAI::UpdateAI(diff);


        if (Vanish_Timer.Expired(diff))
        {
            ForceSpellCast(me, SPELL_VANISH, INTERRUPT_AND_CAST);
            DoResetThreat();
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true))
            {
                float x, y, z;
                me->GetNearPoint(x, y, z, 0, 5.0, frand(0, 2*M_PI));
                AttackStart(target);
                me->AddThreat(target, 10000);
                me->GetMotionMaster()->MovePoint(0, x, y, z);
            }
            InVanish = true;
            Check_Timer = 2000;
            Vanish_Timer = urand(20000, 30000);
        }


        if(!canKick)
        {
            if (Kick_Cooldown.Expired(diff))
            {
                canKick = true;
                Kick_Cooldown = urand(15000, 18000);
            }

        }

        if(canKick && (me->getVictim()->IsNonMeleeSpellCast(true) || roll_chance_f(15.0)))
        {
            if(me->IsWithinMeleeRange(me->getVictim()))
            {
                ForceSpellCast(SPELL_KICK, CAST_TANK);
                canKick = false;
            }
        }


        if (Backstab_Timer.Expired(diff))
        {
            if(!m_creature->getVictim()->HasInArc(M_PI, m_creature))
                ForceSpellCast(SPELL_BACKSTAB, CAST_TANK, INTERRUPT_AND_CAST);
            Backstab_Timer = 3000;
        }



        if (Gouge_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_GOUGE, CAST_TANK);
            DoModifyThreatPercent(m_creature->getVictim(),-100);
            Gouge_Timer = urand(12000, 25000);
        }


        if (Eviscerate_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_EVISCERATE, CAST_TANK);
            Eviscerate_Timer = urand(4000, 10000);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

#define SPELL_CURSE_OF_AGONY         (HeroicMode?46190:14875)
#define SPELL_IMMOLATE               (HeroicMode?46191:44267)
#define SPELL_SHADOW_BOLT            (HeroicMode?15232:12471)
#define SPELL_SEED_OF_CORRUPTION     44141
#define SPELL_DEATH_COIL             44142
#define SPELL_FEAR                   38595

#define SPELL_SUMMON_IMP             44163
#define SPELL_IMP_FIREBALL           44164

#define NPC_FIZZLE                   24656

struct boss_ellris_duskhallowAI : public boss_priestess_guestAI
{
    //Warlock
    boss_ellris_duskhallowAI(Creature *c) : boss_priestess_guestAI(c) { }

    Timer Check_Timer;
    Timer Autocast_Timer;
    Timer SummonImp_Timer;
    Timer Seed_of_Corruption_Timer;
    Timer Curse_of_Agony_Timer;
    Timer Fear_Timer;

    void Reset()
    {
        Check_Timer.Reset(2000);
        Autocast_Timer.Reset(3000);
        SummonImp_Timer.Reset(5000);
        Seed_of_Corruption_Timer.Reset(2000);
        Curse_of_Agony_Timer.Reset(1000);
        Fear_Timer.Reset(10000);

        boss_priestess_guestAI::Reset();
    }

    void AttackStart(Unit* who)
    {
        ScriptedAI::AttackStartNoMove(who, CHECK_TYPE_CASTER);
    }

    void JustDied(Unit* killer)
    {
        boss_priestess_guestAI::JustDied(killer);
    }

    void RegenMana()
    {
        uint32 maxMana = me->GetMaxPower(POWER_MANA);
        uint32 Mana = me->GetPower(POWER_MANA);
        me->SetPower(POWER_MANA, Mana+0.01*maxMana);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!me->isInCombat())
        {

            if (SummonImp_Timer.Expired(diff))
            {
                // check if still having pet nearby;]
                Unit* Fizzle = FindCreature(NPC_FIZZLE, 60.0, me);

                if(!Fizzle)
                    DoCast(m_creature, SPELL_SUMMON_IMP, false);

                if(Fizzle && !Fizzle->isAlive())
                {
                    ((Creature*)Fizzle)->RemoveCorpse();
                    DoCast(m_creature, SPELL_SUMMON_IMP, false);
                }
                SummonImp_Timer = 15000;
            }
        }

        if(!UpdateVictim())
            return;

        boss_priestess_guestAI::UpdateAI(diff);

        if(me->getVictim()->isCrowdControlled())
            DoModifyThreatPercent(me->getVictim(), -100);

        if (Check_Timer.Expired(diff))
        {
            RegenMana();
            Check_Timer = 2000;
        }

        if (Autocast_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_SHADOW_BOLT, CAST_TANK);
            if(roll_chance_f(20.0))
                AddSpellToCast(SPELL_IMMOLATE, CAST_TANK);
            Autocast_Timer = 3000;
        }

        if (Seed_of_Corruption_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_SEED_OF_CORRUPTION, CAST_RANDOM);
            Seed_of_Corruption_Timer = 10000;
        }

        if (Curse_of_Agony_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_CURSE_OF_AGONY, CAST_TANK);
            Curse_of_Agony_Timer = urand(12000, 15000);
        }

        if (Fear_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_FEAR, CAST_RANDOM);
            Fear_Timer = urand(8000, 12000);
        }

        CheckCasterNoMovementInRange(diff);
        CastNextSpellIfAnyAndReady(diff);

        if(me->GetPower(POWER_MANA)*100/me->GetMaxPower(POWER_MANA) <= 5)
            DoMeleeAttackIfReady();
    }
};

struct mob_fizzleAI : public ScriptedAI
{
    mob_fizzleAI(Creature *c) : ScriptedAI(c) { }

    Timer Autocast_Timer;
    Timer Check_Timer;

    void Reset()
    {
        Autocast_Timer = 0;
        Check_Timer.Reset(2000);
    }

    void AttackStart(Unit* who)
    {
        ScriptedAI::AttackStartNoMove(who, CHECK_TYPE_CASTER);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(me->getVictim()->isCrowdControlled())
            DoModifyThreatPercent(me->getVictim(), -100);


        if (Autocast_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_IMP_FIREBALL, CAST_TANK);
            Autocast_Timer = 1900;
        }


        CheckCasterNoMovementInRange(diff);
        CastNextSpellIfAnyAndReady();
        if(me->GetPower(POWER_MANA)*100/me->GetMaxPower(POWER_MANA) <= 5)
            DoMeleeAttackIfReady();
    }
};

#define SPELL_KNOCKDOWN            (HeroicMode?46183:11428)
#define SPELL_SNAP_KICK            46182
#define SPELL_FISTS_OF_ARCANE_FURY 44120
#define SPELL_HEALING_POTION         15503

struct boss_eramas_brightblazeAI : public boss_priestess_guestAI
{
    //Monk
    boss_eramas_brightblazeAI(Creature *c) : boss_priestess_guestAI(c) {}

    Timer Knockdown_Timer;
    Timer Snap_Kick_Timer;
    Timer ChacraDrain_Timer;

    void Reset()
    {
        Knockdown_Timer.Reset(urand(10000, 20000));
        Snap_Kick_Timer.Reset(urand(4000, 8000));
        ChacraDrain_Timer.Reset(urand(10000, 20000));

        boss_priestess_guestAI::Reset();
    }

    void DamageMade(Unit* target, uint32 & damage, bool direct_damage, uint8 school_mask)
    {
        if(damage && direct_damage && roll_chance_f(20))
            AddSpellToCast(target, SPELL_FISTS_OF_ARCANE_FURY, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim() )
            return;

        boss_priestess_guestAI::UpdateAI(diff);

        if(me->getVictim()->isCrowdControlled())
            DoResetThreat();


        if (Knockdown_Timer.Expired(diff))
        {
            if(me->IsWithinMeleeRange(me->getVictim()))
            {
                AddSpellToCast(SPELL_KNOCKDOWN, CAST_TANK);
                Knockdown_Timer = 10000;
            }
        }


        if (Snap_Kick_Timer.Expired(diff))
        {
            if(me->IsWithinMeleeRange(me->getVictim()))
            {
                AddSpellToCast(SPELL_SNAP_KICK, CAST_TANK);
                Snap_Kick_Timer = 12000;
            }
        }


        if (ChacraDrain_Timer.Expired(diff))
        {
            if(HeroicMode)
                AddSpellToCast(SPELL_HEALING_POTION, CAST_SELF);
            ChacraDrain_Timer = urand(10000, 20000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

#define SPELL_POLYMORPH             13323
#define SPELL_ICE_BLOCK             27619
#define SPELL_FROST_NOVA            44177
#define SPELL_BLINK                 14514
#define SPELL_BLIZZARD              (HeroicMode?46195:44178)
#define SPELL_ICE_LANCE             (HeroicMode?46194:44176)
#define SPELL_CONE_OF_COLD          (HeroicMode?38384:12611)
#define SPELL_FROSTBOLT             (HeroicMode?15530:15043)

struct boss_yazzaiAI : public boss_priestess_guestAI
{
    //Mage
    boss_yazzaiAI(Creature *c) : boss_priestess_guestAI(c)  {}

    bool canFroze;
    bool canCoC;
    bool canBlink;
    bool hasIceBlocked;

    Timer FrostNova_Cooldown;
    Timer ConeOfCold_Cooldown;
    Timer Blink_Cooldown;

    Timer Check_Timer;
    Timer Autocast_Timer;
    Timer MeleeCheck_Timer;
    Timer Polymorph_Timer;
    Timer Ice_Block_Timer;
    Timer Blizzard_Timer;
    Timer Ice_Lance_Timer;

    void Reset()
    {
        hasIceBlocked = false;
        canFroze = true;
        canCoC = true;
        canBlink = true;
        FrostNova_Cooldown.Reset(25000);
        ConeOfCold_Cooldown.Reset(10000);
        Blink_Cooldown.Reset(15000);
        Polymorph_Timer = 0;
        MeleeCheck_Timer.Reset(2000);
        Check_Timer.Reset(5000);
        Autocast_Timer = 0;
        Blizzard_Timer.Reset(urand(3000, 10000));

        boss_priestess_guestAI::Reset();
    }

    void AttackStart(Unit* who)
    {
        ScriptedAI::AttackStartNoMove(who, CHECK_TYPE_CASTER);
    }

    void RegenMana()
    {
        uint32 maxMana = me->GetMaxPower(POWER_MANA);
        uint32 Mana = me->GetPower(POWER_MANA);
        me->SetPower(POWER_MANA, Mana+(HeroicMode?0.01:0.005)*maxMana);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim() )
            return;

        boss_priestess_guestAI::UpdateAI(diff);

        if(me->getVictim() && me->getVictim()->isCrowdControlled())
            DoModifyThreatPercent(me->getVictim(), -100);


        if (Check_Timer.Expired(diff))
        {
            RegenMana();
            Check_Timer = 2000;
        }


        if (Autocast_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_FROSTBOLT, CAST_TANK);
            Autocast_Timer = 3000;
        }


        if(!canFroze)
        {

            if (FrostNova_Cooldown.Expired(diff))
            {
                canFroze = true;
                FrostNova_Cooldown = 25000;
            }
        }
        if(!canCoC)
        {

            if (ConeOfCold_Cooldown.Expired(diff))
            {
                canCoC = true;
                ConeOfCold_Cooldown = 10000;
            }
        }
        if(!canBlink)
        {

            if (Blink_Cooldown.Expired(diff))
            {
                ClearCastQueue();
                SetAutocast(SPELL_FROSTBOLT, 3000, true);
                canBlink = true;
                Blink_Cooldown = 15000;
            }
        }

        ;
        if (MeleeCheck_Timer.Expired(diff))
        {
            if(me->IsWithinMeleeRange(me->getVictim()))
            {
                if(canFroze)
                {
                    AddSpellToCast(SPELL_FROST_NOVA, CAST_SELF, true);
                    if(!canBlink)
                    {
                        float x, y, z;
                        me->GetNearPoint(x, y, z, 0, 7.0, frand(0, 2*M_PI));
                        me->UpdateAllowedPositionZ(x, y, z);
                        me->GetMotionMaster()->MovePoint(1, x, y, z);
                    }
                    SetAutocast(SPELL_ICE_LANCE, 1400, true);
                    canFroze = false;
                }
                if(canBlink)
                {
                    ForceSpellCast(SPELL_BLINK, CAST_SELF);
                    MeleeCheck_Timer = 3000;
                    canBlink = false;
                    return;
                }
                MeleeCheck_Timer = 2000;
            }

            if(me->IsWithinDistInMap(me->getVictim(), 10.0))
            {
                if(canCoC)
                {
                    AddSpellToCast(SPELL_CONE_OF_COLD, CAST_TANK);
                    canCoC = false;
                }
            }
        }



        if (Polymorph_Timer.Expired(diff))
        {
            Polymorph_Timer = 1000;
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30, true))
            {
                AddSpellToCast(target, SPELL_POLYMORPH);
                DoModifyThreatPercent(target,-100);
                Polymorph_Timer = urand(15000, 25000);
            }
        }


        if(HealthBelowPct(35) && !hasIceBlocked)
        {
            ForceSpellCast(SPELL_ICE_BLOCK, CAST_SELF, INTERRUPT_AND_CAST_INSTANTLY);
            canBlink = true;
            hasIceBlocked = true;
        }


        if (Blizzard_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_BLIZZARD, CAST_RANDOM);
            Blizzard_Timer = urand(16000, 24000);
        }


        CheckCasterNoMovementInRange(diff, 30);
        CastNextSpellIfAnyAndReady(diff);
        if(me->GetPower(POWER_MANA)*100/me->GetMaxPower(POWER_MANA) <= 5)
            DoMeleeAttackIfReady();
    }
};

#define SPELL_BATTLE_SHOUT           27578
#define SPELL_INTERCEPT              27577
#define SPELL_DISARM                 27581
#define SPELL_PIERCING_HOWL          23600
#define SPELL_PSYCHIC_SCREAM         27610
#define SPELL_HAMSTRING              27584
#define SPELL_MORTAL_STRIKE          44268

struct boss_warlord_salarisAI : public boss_priestess_guestAI
{
    //Warrior
    boss_warlord_salarisAI(Creature *c) : boss_priestess_guestAI(c) {}

    Timer BattleShout_Timer;
    Timer Intercept_Timer;
    Timer Disarm_Timer;
    Timer Piercing_Howl_Timer;
    Timer Frightening_Shout_Timer;
    Timer Hamstring_Timer;
    Timer Mortal_Strike_Timer;

    void Reset()
    {
        boss_priestess_guestAI::Reset();
        BattleShout_Timer.Reset(110000);
        Intercept_Timer.Reset(1000);
        Disarm_Timer.Reset(urand(10000, 15000));
        Hamstring_Timer.Reset(urand(4000, 5000));
        Mortal_Strike_Timer.Reset(urand(8000, 12000));
        Piercing_Howl_Timer.Reset(urand(5000, 8000));
        Frightening_Shout_Timer.Reset(urand(15000, 24000));
        resetThreat = false;
    }

    void EnterCombat(Unit* who)
    {
        ForceSpellCast(SPELL_BATTLE_SHOUT, CAST_NULL);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        boss_priestess_guestAI::UpdateAI(diff);

        if(me->getVictim() && me->getVictim()->isCrowdControlled())
            DoModifyThreatPercent(me->getVictim(), -100);


        if (BattleShout_Timer.Expired(diff))
        {
            ForceSpellCast(SPELL_BATTLE_SHOUT, CAST_NULL);
            BattleShout_Timer = 110000;
        }


        if (Intercept_Timer.Expired(diff))
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 25.0, true, 0, 8.0);
            if(target)
            {
                DoResetThreat();
                AttackStart(target);
                me->AddThreat(target, 10000);
                ForceSpellCast(target, SPELL_INTERCEPT, INTERRUPT_AND_CAST);
                Intercept_Timer = urand(15000, 25000);
            }
            else
                Intercept_Timer = 2000;
        }


        if (Disarm_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_DISARM, CAST_TANK);
            Disarm_Timer = 60000;
        }



        if (Hamstring_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_HAMSTRING, CAST_TANK);
            Hamstring_Timer = urand(5000, 10000);
        }



        if (Mortal_Strike_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_MORTAL_STRIKE, CAST_TANK);
            Mortal_Strike_Timer = urand(10000, 15000);
        }



        if (Piercing_Howl_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_PIERCING_HOWL, CAST_NULL);
            Piercing_Howl_Timer = urand(20000, 35000);
        }



        if (Frightening_Shout_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_PSYCHIC_SCREAM, CAST_NULL);
            Frightening_Shout_Timer = urand(15000, 40000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

#define SPELL_SHOOT                 (HeroicMode?22907:15620)
#define SPELL_MULTI_SHOT            (HeroicMode?44285:31942)
#define SPELL_AIMED_SHOT            44271
#define SPELL_CONCUSSIVE_SHOT       27634
#define SPELL_WING_CLIP             44286
#define SPELL_FREEZING_TRAP         44136

#define NPC_SLIVER                  24552

struct boss_garaxxasAI : public boss_priestess_guestAI
{
    //Hunter
    boss_garaxxasAI(Creature *c) : boss_priestess_guestAI(c) {}

    Timer GetSliver_Timer;
    Timer Aimed_Shot_Timer;
    Timer Shoot_Timer;
    Timer Concussive_Shot_Timer;
    Timer Multi_Shot_Timer;
    Timer Wing_Clip_Cooldown;
    bool canWingClip;
    Timer Freezing_Trap_Cooldown;
    bool canSetTrap;

    void Reset()
    {
        targetRange = 30;
        Aimed_Shot_Timer.Reset(500);
        Shoot_Timer.Reset(2500);
        Concussive_Shot_Timer.Reset(urand(6000, 8000));
        Multi_Shot_Timer.Reset(urand(16000, 20000));
        Wing_Clip_Cooldown.Reset(10000);
        canWingClip = true;
        Freezing_Trap_Cooldown.Reset(30000);
        canSetTrap = true;

        boss_priestess_guestAI::Reset();
    }

    void AttackStart(Unit* who)
    {
        ScriptedAI::AttackStartNoMove(who, CHECK_TYPE_SHOOTER);
    }

    void JustDied(Unit* killer)
    {
        boss_priestess_guestAI::JustDied(killer);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!me->isInCombat())
        {

            if (GetSliver_Timer.Expired(diff))
            {
                // check if still having pet ;]
                Unit* Sliver = FindCreature(NPC_SLIVER, 60.0, me);

                if(!Sliver)
                {
                    float x, y, z;
                    me->GetNearPoint( x, y, z, 0, 3.0, frand(0, 2*M_PI));
                    Creature* Sliver = m_creature->SummonCreature(NPC_SLIVER, x, y, z, me->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
                }

                if(Sliver && !Sliver->isAlive())
                {
                    ((Creature*)Sliver)->RemoveCorpse();
                    float x, y, z;
                    me->GetNearPoint( x, y, z, 0, 3.0, frand(0, 2*M_PI));
                    Creature* Sliver = m_creature->SummonCreature(NPC_SLIVER, x, y, z, me->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
                }
                GetSliver_Timer = 15000;
            }

        }

        if(!UpdateVictim() )
            return;

        boss_priestess_guestAI::UpdateAI(diff);

        if(me->getVictim() && me->getVictim()->isCrowdControlled())
            DoModifyThreatPercent(me->getVictim(), -100);

        if(!canSetTrap)
        {

            if (Freezing_Trap_Cooldown.Expired(diff))
            {
                canSetTrap = true;
                Freezing_Trap_Cooldown = 30000;
            }
        }

        if(me->IsWithinDistInMap(me->getVictim(), 3.0) && canSetTrap)
        {
            ForceSpellCast(SPELL_FREEZING_TRAP, CAST_SELF, INTERRUPT_AND_CAST_INSTANTLY);
            float x, y, z;
            // float dist = me->GetDistance2d(me->getVictim()); <- unused, so why it's here?
            float angle = me->GetAngle(me->getVictim());
            me->GetPosition(x, y, z);
            x = x - 5.5 * cos(angle);
            y = y - 5.5 * sin(angle);
            me->UpdateAllowedPositionZ(x, y, z);
            me->GetMotionMaster()->MovePoint(1, x, y, z);
            canSetTrap = false;
        }

        if(!canWingClip)
        {

            if (Wing_Clip_Cooldown.Expired(diff))
            {
                Wing_Clip_Cooldown = 10000;
                canWingClip = true;
            }
        }

        if(me->IsWithinDistInMap(me->getVictim(), 6.0) && canWingClip)
        {
            ForceSpellCast(SPELL_WING_CLIP, CAST_TANK, INTERRUPT_AND_CAST);
            float x, y, z;
            me->GetNearPoint(x, y, z, 0, 9.0, frand(0, 2*M_PI));
            me->UpdateAllowedPositionZ(x, y, z);
            me->GetMotionMaster()->MovePoint(2, x, y, z);
            canWingClip = false;
        }


        if (Concussive_Shot_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_CONCUSSIVE_SHOT, CAST_RANDOM);
            Concussive_Shot_Timer = urand(8000, 12000);
        }


        if (Multi_Shot_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_MULTI_SHOT, CAST_RANDOM);
            Multi_Shot_Timer = urand(8000, 12000);
        }


        if (Aimed_Shot_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_AIMED_SHOT, CAST_TANK);
            Aimed_Shot_Timer = urand(12000, 18000);
        }


        if (Shoot_Timer.Expired(diff))
        {
            if(me->GetMotionMaster()->GetCurrentMovementGeneratorType() == IDLE_MOTION_TYPE)
                AddSpellToCast(me->getVictim(), SPELL_SHOOT);
            Shoot_Timer = urand(3000, 5000);
        }


        CheckShooterNoMovementInRange(diff, 30.0);
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

struct mob_sliverAI : public ScriptedAI
{
    mob_sliverAI(Creature *c) : ScriptedAI(c) { }

    void Reset() { }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(me->getVictim() && me->getVictim()->isCrowdControlled())
            DoModifyThreatPercent(me->getVictim(), -100);

        DoMeleeAttackIfReady();
    }
};

#define SPELL_WAR_STOMP              46026
#define SPELL_PURGE                  27626
#define SPELL_LESSER_HEALING_WAVE    44256
#define SPELL_FROST_SHOCK            21401
#define SPELL_WINDFURY_TOTEM         39586  // 27621 is not well supported, lets use NPCs totem spell: AI in file black_temple_trash
#define SPELL_FIRE_NOVA_TOTEM        44257
#define SPELL_EARTHBIND_TOTEM        15786

#define NPC_WINDFURY_TOTEM            22897
#define SPELL_WINDFURY_WEAPON         32911
#define AURA_WINDFURY                 32912

struct boss_apokoAI : public boss_priestess_guestAI
{
    //Shaman
    boss_apokoAI(Creature *c) : boss_priestess_guestAI(c), summons(c) {}

    Timer Totem_Timer;
    uint8  Totem_Amount;
    Timer War_Stomp_Timer;
    Timer Purge_Timer;
    Timer Healing_Wave_Cooldown;
    Timer Frost_Shock_Timer;
    uint32 TotemSpell;
    bool canHeal;
    SummonList summons;

    void Reset()
    {
        summons.DespawnAll();
        Totem_Timer.Reset(urand(3000, 5000));
        War_Stomp_Timer.Reset(urand(2000, 10000));
        Healing_Wave_Cooldown.Reset(5000);
        Purge_Timer.Reset(urand(8000, 15000));
        Frost_Shock_Timer.Reset(urand(5000, 10000));
        TotemSpell = RAND(SPELL_WINDFURY_TOTEM, SPELL_FIRE_NOVA_TOTEM, SPELL_EARTHBIND_TOTEM);
        canHeal = true;

        boss_priestess_guestAI::Reset();
    }

    void JustSummoned(Creature* summon)
    {
        summon->SetMaxHealth(5);
        summons.Summon(summon);
    }

    void RegenMana()
    {
        uint32 maxMana = me->GetMaxPower(POWER_MANA);
        uint32 Mana = me->GetPower(POWER_MANA);
        me->SetPower(POWER_MANA, Mana+0.05*maxMana);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim() )
            return;

        boss_priestess_guestAI::UpdateAI(diff);

        if(canHeal)
        {
            if(Unit* healTarget = SelectLowestHpFriendly(40.0f, 10000))
                ForceSpellCast(healTarget, SPELL_LESSER_HEALING_WAVE, INTERRUPT_AND_CAST);
            canHeal = false;
        }
        else
        {
            if (Healing_Wave_Cooldown.Expired(diff))
            {
                canHeal = true;
                Healing_Wave_Cooldown = (HeroicMode?urand(5000, 8000):urand(6000, 10000));
            }
        }


        if (Totem_Timer.Expired(diff))
        {
            // do not summon same totem twice in row
            uint32 tempSpell = RAND(SPELL_WINDFURY_TOTEM, SPELL_FIRE_NOVA_TOTEM, SPELL_EARTHBIND_TOTEM);
            while(TotemSpell == tempSpell)
            {
                tempSpell = RAND(SPELL_WINDFURY_TOTEM, SPELL_FIRE_NOVA_TOTEM, SPELL_EARTHBIND_TOTEM);
            }
            TotemSpell = tempSpell;
            AddSpellToCast(TotemSpell, CAST_SELF);
            RegenMana();
            Totem_Timer = urand(3000, 8000);
        }


        if (War_Stomp_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_WAR_STOMP, CAST_NULL);
            War_Stomp_Timer = urand(8000, 15000);
        }


        if (Purge_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_PURGE, CAST_RANDOM);
            Purge_Timer = urand(8000, 15000);
        }



        if (Frost_Shock_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_FROST_SHOCK, CAST_TANK);
            Frost_Shock_Timer = urand(6000, 15000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

#define SPELL_GOBLIN_DRAGON_GUN        (HeroicMode?46185:44272)
#define SPELL_ROCKET_LAUNCH            (HeroicMode?46187:44137)
#define SPELL_FEL_IRON_BOMB            (HeroicMode?46184:46024)
#define SPELL_RECOMBOBULATE            44274
#define SPELL_HIGH_EXPLOSIVE_SHEEP     44276
#define SPELL_SHEEP_EXPLOSION          44279

#define CREATURE_EXPLOSIVE_SHEEP        24715

struct boss_zelfanAI : public boss_priestess_guestAI
{
    //Engineer
    boss_zelfanAI(Creature *c) : boss_priestess_guestAI(c) {}

    Timer Goblin_Dragon_Gun_Timer;
    Timer Rocket_Launch_Timer;
    Timer Recombobulate_Timer;
    Timer High_Explosive_Sheep_Timer;
    Timer Fel_Iron_Bomb_Timer;

    void Reset()
    {
        Goblin_Dragon_Gun_Timer.Reset(urand(5000, 20000));
        Rocket_Launch_Timer.Reset(1000);
        Recombobulate_Timer.Reset(urand(3000, 6000));
        High_Explosive_Sheep_Timer.Reset((HeroicMode ? 1000 : urand(8000, 15000)));
        Fel_Iron_Bomb_Timer.Reset(urand(4000, 15000));

        boss_priestess_guestAI::Reset();
    }

    void JustSummoned(Creature* summon)
    {
        if(summon->GetEntry() != CREATURE_EXPLOSIVE_SHEEP)
            return;
        if(Unit* target = SelectUnit(SELECT_TARGET_NEAREST, 0, 100.0, true))
        {
            summon->AI()->AttackStart(target);
            summon->AddThreat(target, 50000);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim() )
            return;

        boss_priestess_guestAI::UpdateAI(diff);

        if(me->getVictim() && me->getVictim()->isCrowdControlled())
            DoModifyThreatPercent(me->getVictim(), -100);


        if (Goblin_Dragon_Gun_Timer.Expired(diff))
        {
            if (me->IsWithinDistInMap(me->getVictim(), 5))
            {
                ForceSpellCast(SPELL_GOBLIN_DRAGON_GUN, CAST_TANK, INTERRUPT_AND_CAST);
                Goblin_Dragon_Gun_Timer = urand(10000, 15000);
            }
            else
                Goblin_Dragon_Gun_Timer = 2000;
        }


        if (Rocket_Launch_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_ROCKET_LAUNCH, CAST_RANDOM);
            Rocket_Launch_Timer = urand(8000, 12000);
        }


        if (Fel_Iron_Bomb_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_FEL_IRON_BOMB, CAST_RANDOM);
            Fel_Iron_Bomb_Timer = HeroicMode?urand(3500, 7000):urand(4000, 10000);
            ResetThreatTimer = 0;
        }



        if (Recombobulate_Timer.Expired(diff))
        {
            Recombobulate_Timer = 3000;
            std::list<Creature*> CC_list = FindFriendlyCC(30.0);
            if(!CC_list.empty())
            {
                for(std::list<Creature*>::iterator i = CC_list.begin(); i != CC_list.end(); ++i)
                {
                    if((*i)->IsPolymorphed())
                    {
                        ForceSpellCast((*i), SPELL_RECOMBOBULATE, INTERRUPT_AND_CAST);
                        Recombobulate_Timer = urand(15000, 30000);
                        break;
                    }
                }
            }
        }


        if (High_Explosive_Sheep_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_HIGH_EXPLOSIVE_SHEEP, CAST_SELF);
            High_Explosive_Sheep_Timer = HeroicMode?urand(15000, 20000):urand(22000, 32000);
        }


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

struct mob_high_explosive_sheepAI : public ScriptedAI
{
    mob_high_explosive_sheepAI(Creature *c) : ScriptedAI(c) {}

    Timer SelfDestro_Timer;
    Timer Check_Timer;

    void Reset()
    {
        me->SetWalk(true);
        SelfDestro_Timer.Reset(60000);
        Check_Timer.Reset(500);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;


        if (Check_Timer.Expired(diff))
        {
            if(me->IsWithinMeleeRange(me->getVictim()))
                DoCast(me, SPELL_SHEEP_EXPLOSION);
            Check_Timer = 500;
        }



        if (SelfDestro_Timer.Expired(diff))
            DoCast(me, SPELL_SHEEP_EXPLOSION);

    }
};

CreatureAI* GetAI_boss_priestess_delrissa(Creature *_Creature)
{
    return new boss_priestess_delrissaAI (_Creature);
}

CreatureAI* GetAI_boss_kagani_nightstrike(Creature *_Creature)
{
    return new boss_kagani_nightstrikeAI (_Creature);
}

CreatureAI* GetAI_ellris_duskhallow(Creature *_Creature)
{
    return new boss_ellris_duskhallowAI (_Creature);
}

CreatureAI* GetAI_mob_fizzle(Creature *_Creature)
{
    return new mob_fizzleAI (_Creature);
};

CreatureAI* GetAI_eramas_brightblaze(Creature *_Creature)
{
    return new boss_eramas_brightblazeAI (_Creature);
}

CreatureAI* GetAI_yazzai(Creature *_Creature)
{
    return new boss_yazzaiAI (_Creature);
}

CreatureAI* GetAI_warlord_salaris(Creature *_Creature)
{
    return new boss_warlord_salarisAI (_Creature);
}

CreatureAI* GetAI_garaxxas(Creature *_Creature)
{
    return new boss_garaxxasAI (_Creature);
}

CreatureAI* GetAI_mob_sliver(Creature *_Creature)
{
    return new mob_sliverAI (_Creature);
};

CreatureAI* GetAI_apoko(Creature *_Creature)
{
    return new boss_apokoAI (_Creature);
}

CreatureAI* GetAI_zelfan(Creature *_Creature)
{
    return new boss_zelfanAI (_Creature);
}

CreatureAI* GetAI_mob_high_explosive_sheep(Creature *_Creature)
{
    return new mob_high_explosive_sheepAI (_Creature);
};

void AddSC_boss_priestess_delrissa()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_priestess_delrissa";
    newscript->GetAI = &GetAI_boss_priestess_delrissa;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_kagani_nightstrike";
    newscript->GetAI = &GetAI_boss_kagani_nightstrike;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_ellris_duskhallow";
    newscript->GetAI = &GetAI_ellris_duskhallow;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_fizzle";
    newscript->GetAI = &GetAI_mob_fizzle;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_eramas_brightblaze";
    newscript->GetAI = &GetAI_eramas_brightblaze;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_yazzai";
    newscript->GetAI = &GetAI_yazzai;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_warlord_salaris";
    newscript->GetAI = &GetAI_warlord_salaris;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_garaxxas";
    newscript->GetAI = &GetAI_garaxxas;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_sliver";
    newscript->GetAI = &GetAI_mob_sliver;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_apoko";
    newscript->GetAI = &GetAI_apoko;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_zelfan";
    newscript->GetAI = &GetAI_zelfan;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_high_explosive_sheep";
    newscript->GetAI = &GetAI_mob_high_explosive_sheep;
    newscript->RegisterSelf();
}

