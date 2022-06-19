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
SDName: Boss_Moroes
SD%Complete: 95
SDComment:
SDCategory: Karazhan
EndScriptData */

#include "precompiled.h"
#include "def_karazhan.h"

#define SAY_AGGRO           -1532011
#define SAY_SPECIAL_1       -1532012
#define SAY_SPECIAL_2       -1532013
#define SAY_KILL_1          -1532014
#define SAY_KILL_2          -1532015
#define SAY_KILL_3          -1532016
#define SAY_DEATH           -1532017

#define SPELL_VANISH        29448
#define SPELL_GARROTE       37066
#define SPELL_BLIND         34694
#define SPELL_GOUGE         29425
#define SPELL_FRENZY        37023

#define POS_Z               81.73

static float Locations[4][3]=
{
    {-10991.0f, -1884.33f, 0.614315f},
    {-10989.4f, -1885.88f, 0.904913f},
    {-10978.1f, -1887.07f, 2.035550f},
    {-10975.9f, -1885.81f, 2.253890f},
};

static const uint32 Adds[6]=
{
    17007,
    19872,
    19873,
    19874,
    19875,
    19876,
};

bool morocheckPosition(WorldObject* obj)
{
    if (!obj) return true;
    return (obj->GetPositionX() > -11030) && (obj->GetPositionY() > -1950);
}

struct boss_moroesAI : public ScriptedAI
{
    boss_moroesAI(Creature *c) : ScriptedAI(c)
    {
        for(int i = 0; i < 4; i++)
        {
            AddId[i] = 0;
        }
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    uint64 AddGUID[4];

    Timer Vanish_Timer;
    Timer Blind_Timer;
    Timer Gouge_Timer;
    Timer Wait_Timer;
    Timer NonAttackable_Timer;
    Timer CheckAdds_Timer;
    uint32 AddId[4];

    bool InVanish;
    bool NonAttackable;
    bool Enrage;

    void Reset()
    {
        Vanish_Timer.Reset(30000);
        Blind_Timer.Reset(35000);
        Gouge_Timer.Reset(23000);
        Wait_Timer.Reset(0);
        NonAttackable_Timer.Reset(0);
        CheckAdds_Timer.Reset(5000);
        Enrage = false;
        InVanish = false;
        NonAttackable = false;
        if(m_creature->GetHealth() > 0)
        {
            SpawnAdds();
        }

        if(m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
        {
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }

        if(pInstance && pInstance->GetData(DATA_MOROES_EVENT) != DONE)
            pInstance->SetData(DATA_MOROES_EVENT, NOT_STARTED);
    }


    void EnterCombat(Unit* who)
    {
        if (!pInstance)
            return;
        if (pInstance->GetData(DATA_MOROES_EVENT) != NOT_STARTED)
            return;

        pInstance->SetData(DATA_MOROES_EVENT, IN_PROGRESS);
        DoScriptText(SAY_AGGRO, m_creature);
        DoZoneInCombat();
        AddsAttack();
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_KILL_1, SAY_KILL_2, SAY_KILL_3), m_creature);
    }

    void JustDied(Unit* victim)
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (pInstance)
            pInstance->SetData(DATA_MOROES_EVENT, DONE);

        DeSpawnAdds();

        //remove aura from spell Garrote when Moroes dies
        Map *map = m_creature->GetMap();
        if (map->IsDungeon())
        {
            Map::PlayerList const &PlayerList = map->GetPlayers();

            if (PlayerList.isEmpty())
                return;

            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            {
                if (i->getSource()->isAlive() && i->getSource()->HasAura(SPELL_GARROTE,0))
                    i->getSource()->RemoveAurasDueToSpell(SPELL_GARROTE);
            }
        }
    }

    void SpawnAdds()
    {
        DeSpawnAdds();
        if(isAddlistEmpty())
        {
            Creature *pCreature = NULL;
            std::vector<uint32> AddList;


            for(uint8 i = 0; i < 6; ++i)
                AddList.push_back(Adds[i]);

            while(AddList.size() > 4)
                AddList.erase((AddList.begin())+(rand()%AddList.size()));

            uint8 i = 0;
            for(std::vector<uint32>::iterator itr = AddList.begin(); itr != AddList.end(); ++itr)
            {
                uint32 entry = *itr;

                pCreature = m_creature->SummonCreature(entry, Locations[i][0], Locations[i][1], POS_Z, Locations[i][2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                if (pCreature)
                {
                    AddGUID[i] = pCreature->GetGUID();
                    AddId[i] = entry;
                }
                ++i;
            }
        }else
        {
            for(int i = 0; i < 4; i++)
            {
                Creature *pCreature = m_creature->SummonCreature(AddId[i], Locations[i][0], Locations[i][1], POS_Z, Locations[i][2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                if (pCreature)
                {
                    AddGUID[i] = pCreature->GetGUID();
                }
            }
        }
    }

    bool isAddlistEmpty()
    {
        for(int i = 0; i < 4; i++)
        {
            if(AddId[i] == 0)
                return true;
        }
        return false;
    }

    void DeSpawnAdds()
    {
        for(uint8 i = 0; i < 4 ; ++i)
        {
            Creature* Temp = NULL;
            if (AddGUID[i])
            {
                Temp = Creature::GetCreature((*m_creature),AddGUID[i]);
                if (Temp && Temp->isAlive())
                {
                    (*Temp).GetMotionMaster()->Clear(true);
                    Temp->DealDamage(Temp, Temp->GetMaxHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                    Temp->RemoveCorpse();
                }

            }
        }
    }

    void AddsAttack()
    {
        for(uint8 i = 0; i < 4; ++i)
        {
            Creature* Temp = NULL;
            if (AddGUID[i])
            {
                Temp = Creature::GetCreature((*m_creature),AddGUID[i]);
                if (Temp && Temp->isAlive())
                {
                    Temp->AI()->AttackStart(m_creature->getVictim());
                    Temp->AI()->DoZoneInCombat();
                }
                else
                    EnterEvadeMode();
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim() )
            return;

        if(pInstance && !pInstance->GetData(DATA_MOROES_EVENT))
        {
            EnterEvadeMode();
            return;
        }

        if(!Enrage && m_creature->GetHealth()*100 / m_creature->GetMaxHealth() < 30)
        {
            DoCast(m_creature, SPELL_FRENZY);
            Enrage = true;
        }


        if (CheckAdds_Timer.Expired(diff))
        {
            for (uint8 i = 0; i < 4; ++i)
            {
                Creature* Temp = NULL;
                if (AddGUID[i])
                {
                    Temp = Unit::GetCreature((*m_creature),AddGUID[i]);
                    if (Temp && Temp->isAlive())
                    {
                        if (!Temp->getVictim())
                            Temp->AI()->AttackStart(m_creature->getVictim());
                        if (!morocheckPosition(Temp) || !morocheckPosition(Temp->getVictim()))
                            EnterEvadeMode();
                    }
                }
            }
            if (!morocheckPosition(m_creature) || !morocheckPosition(m_creature->getVictim()))
                EnterEvadeMode();
            CheckAdds_Timer = 5000;
        }
        

        if (!Enrage)
        {
            //Cast Vanish, then Garrote random victim
            if (Vanish_Timer.Expired(diff))
            {
                DoCast(m_creature, SPELL_VANISH);
                InVanish = true;
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                NonAttackable = true;
                NonAttackable_Timer = 12100;
                Vanish_Timer = 42000;
                Wait_Timer = 5000;
            }
            


            if (Gouge_Timer.Expired(diff))
            {
                DoCast(m_creature->getVictim(), SPELL_GOUGE);
                Gouge_Timer = urand(25000,35000);
            }
            
            if (Blind_Timer.Expired(diff))
            {
                if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_BLIND), true))
                    DoCast(target, SPELL_BLIND);

                Blind_Timer = 40000;
            }
            
        }

        if(InVanish)
        {
            if (Wait_Timer.Expired(diff))
            {
                DoScriptText(RAND(SAY_SPECIAL_1, SAY_SPECIAL_2), m_creature);

                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 50, true))
                   target->CastSpell(target, SPELL_GARROTE,true);

                InVanish = false;
            } 
        }

        if(!InVanish)
            DoMeleeAttackIfReady();

        if(NonAttackable)
        {
            if (NonAttackable_Timer.Expired(diff))
            {
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                NonAttackable = false;
            } 
        }
    }
};

struct boss_moroes_guestAI : public ScriptedAI
{
    ScriptedInstance* pInstance;

    uint64 GuestGUID[5];

    boss_moroes_guestAI(Creature* c) : ScriptedAI(c)
    {
        for(uint8 i = 0; i < 5; ++i)
            GuestGUID[i] = 0;

        pInstance = (c->GetInstanceData());
    }

    void Reset()
    {
        if(pInstance)
            pInstance->SetData(DATA_MOROES_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit* who)
    {
        if (!pInstance)
            return;
        Creature* Moroes = (Unit::GetCreature((*m_creature), pInstance->GetData64(DATA_MOROES)));
        if (Moroes)
        {
            Moroes->AI()->EnterCombat(who);
        }
    }

    void AcquireGUID()
    {
        if(!pInstance)
            return;

        GuestGUID[0] = pInstance->GetData64(DATA_MOROES);
        Creature* Moroes = (Unit::GetCreature((*m_creature), GuestGUID[0]));
        if(Moroes)
        {
            for(uint8 i = 0; i < 4; ++i)
            {
                uint64 GUID = ((boss_moroesAI*)Moroes->AI())->AddGUID[i];
                GuestGUID[i+1] = GUID;
            }
        }
    }

    Unit* SelectTarget()
    {
        uint8 count = 0;
        for (uint8 i = 0; i < 5; i++)
        {
            uint64 TempGUID = GuestGUID[urand(0, 4)];
            if (TempGUID)
            {
                Unit* pUnit = Unit::GetUnit((*m_creature), TempGUID);
                if (pUnit && pUnit->isAlive())
                    count++;
            }
        }

        count = urand(1, count);
        for (uint8 i = 0; i < 5; i++)
        {
            uint64 TempGUID = GuestGUID[urand(0, 4)];
            if (TempGUID)
            {
                Unit* pUnit = Unit::GetUnit((*m_creature), TempGUID);
                if (pUnit && pUnit->isAlive())
                {
                    count--;
                    if (count == 0)
                        return pUnit;
                }
            }
        }
        return m_creature;
    }

    void UpdateAI(const uint32 diff)
    {
        if(pInstance && !pInstance->GetData(DATA_MOROES_EVENT))
            EnterEvadeMode();

        DoMeleeAttackIfReady();
    }
};

#define SPELL_MANABURN       29405
#define SPELL_MINDFLY        29570
#define SPELL_SWPAIN         34441
#define SPELL_SHADOWFORM     29406

struct boss_baroness_dorothea_millstipeAI : public boss_moroes_guestAI
{
    //Shadow Priest
    boss_baroness_dorothea_millstipeAI(Creature *c) : boss_moroes_guestAI(c) {}

    Timer ManaBurn_Timer;
    Timer MindFlay_Timer;
    Timer ShadowWordPain_Timer;

    void Reset()
    {
        ManaBurn_Timer.Reset(7000);
        MindFlay_Timer.Reset(1000);
        ShadowWordPain_Timer.Reset(6000);

        DoCast(m_creature,SPELL_SHADOWFORM, true);

        boss_moroes_guestAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim() )
            return;

        boss_moroes_guestAI::UpdateAI(diff);

        if (MindFlay_Timer.Expired(diff))
        {
            DoCast(m_creature->getVictim(),SPELL_MINDFLY);
            MindFlay_Timer = 12000;                         //3sec channeled
        }

        
        if (ManaBurn_Timer.Expired(diff))
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0);
            if(target && (target->getPowerType() == POWER_MANA))
                DoCast(target,SPELL_MANABURN);
            ManaBurn_Timer = 5000;                          //3 sec cast
        }

        if (ShadowWordPain_Timer.Expired(diff))
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0);
            if(target)
            {
                DoCast(target,SPELL_SWPAIN);
                ShadowWordPain_Timer = 7000;
            }
        }
    }
};

#define SPELL_HAMMEROFJUSTICE       13005
#define SPELL_JUDGEMENTOFCOMMAND    29386
#define SPELL_SEALOFCOMMAND         29385

struct boss_baron_rafe_dreugerAI : public boss_moroes_guestAI
{
    //Retr Pally
    boss_baron_rafe_dreugerAI(Creature *c) : boss_moroes_guestAI(c){}

    Timer HammerOfJustice_Timer;
    Timer SealOfCommand_Timer;
    Timer JudgementOfCommand_Timer;

    void Reset()
    {
        HammerOfJustice_Timer.Reset(1000);
        SealOfCommand_Timer.Reset(7000);
        JudgementOfCommand_Timer = SealOfCommand_Timer.GetInterval() + 29000;

        boss_moroes_guestAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim() )
            return;

        boss_moroes_guestAI::UpdateAI(diff);

        
        if (SealOfCommand_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_SEALOFCOMMAND);
            SealOfCommand_Timer = 32000;
            JudgementOfCommand_Timer = 29000;
        }

        
        if (JudgementOfCommand_Timer.Expired(diff))
        {
            DoCast(m_creature->getVictim(),SPELL_JUDGEMENTOFCOMMAND);
            JudgementOfCommand_Timer = SealOfCommand_Timer.GetTimeLeft() + 29000;
        }

        
        if (HammerOfJustice_Timer.Expired(diff))
        {
            DoCast(m_creature->getVictim(),SPELL_HAMMEROFJUSTICE);
            HammerOfJustice_Timer = 12000;
        }
    }
};

#define SPELL_DISPELMAGIC           15090                   //Self or other guest+Moroes
#define SPELL_GREATERHEAL           29564                   //Self or other guest+Moroes
#define SPELL_HOLYFIRE              29563
#define SPELL_PWSHIELD              29408

struct boss_lady_catriona_von_indiAI : public boss_moroes_guestAI
{
    //Holy Priest
    boss_lady_catriona_von_indiAI(Creature *c) : boss_moroes_guestAI(c) {}

    Timer DispelMagic_Timer;
    Timer GreaterHeal_Timer;
    Timer HolyFire_Timer;
    Timer PowerWordShield_Timer;

    void Reset()
    {
        DispelMagic_Timer.Reset(11000);
        GreaterHeal_Timer.Reset(1500);
        HolyFire_Timer.Reset(5000);
        PowerWordShield_Timer.Reset(1000);

        AcquireGUID();

        boss_moroes_guestAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim() )
            return;

        boss_moroes_guestAI::UpdateAI(diff);

        
        if (PowerWordShield_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_PWSHIELD);
            PowerWordShield_Timer = 15000;
        }

        
        if (GreaterHeal_Timer.Expired(diff))
        {
            Unit* target = SelectTarget();

            DoCast(target, SPELL_GREATERHEAL);
            GreaterHeal_Timer = 17000;
        }

        
        if (HolyFire_Timer.Expired(diff))
        {
            DoCast(m_creature->getVictim(),SPELL_HOLYFIRE);
            HolyFire_Timer = 22000;
        }

        
        if (DispelMagic_Timer.Expired(diff))
        {
            if(rand()%2)
            {
                Unit* target = SelectTarget();

                DoCast(target, SPELL_DISPELMAGIC);
            }
            else
                DoCast(SelectUnit(SELECT_TARGET_RANDOM, 0), SPELL_DISPELMAGIC);

            DispelMagic_Timer = 25000;
        }
    }
};

#define SPELL_CLEANSE               29380                   //Self or other guest+Moroes
#define SPELL_GREATERBLESSOFMIGHT   29381                   //Self or other guest+Moroes
#define SPELL_HOLYLIGHT             29562                   //Self or other guest+Moroes
#define SPELL_DIVINESHIELD          41367

struct boss_lady_keira_berrybuckAI : public boss_moroes_guestAI
{
    //Holy Pally
    boss_lady_keira_berrybuckAI(Creature *c) : boss_moroes_guestAI(c)  {}

    Timer Cleanse_Timer;
    Timer GreaterBless_Timer;
    Timer HolyLight_Timer;
    Timer DivineShield_Timer;

    void Reset()
    {
        Cleanse_Timer.Reset(13000);
        GreaterBless_Timer.Reset(1000);
        HolyLight_Timer.Reset(7000);
        DivineShield_Timer.Reset(31000);

        AcquireGUID();

        boss_moroes_guestAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim() )
            return;

        boss_moroes_guestAI::UpdateAI(diff);

        
        if (DivineShield_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_DIVINESHIELD);
            DivineShield_Timer = 31000;
        }

        
        if (HolyLight_Timer.Expired(diff))
        {
            Unit* target = SelectTarget();

            DoCast(target, SPELL_HOLYLIGHT);
            HolyLight_Timer = 10000;
        }

        
        if (GreaterBless_Timer.Expired(diff))
        {
            Unit* target = SelectTarget();

            DoCast(target, SPELL_GREATERBLESSOFMIGHT);

            GreaterBless_Timer = 50000;
        }

        
        if (Cleanse_Timer.Expired(diff))
        {
            Unit* target = SelectTarget();

            DoCast(target, SPELL_CLEANSE);

            Cleanse_Timer = 10000;
        }
    }
};

#define SPELL_HAMSTRING         9080
#define SPELL_MORTALSTRIKE      29572
#define SPELL_WHIRLWIND         29573

struct boss_lord_robin_darisAI : public boss_moroes_guestAI
{
    //Arms Warr
    boss_lord_robin_darisAI(Creature *c) : boss_moroes_guestAI(c) {}

    Timer Hamstring_Timer;
    Timer MortalStrike_Timer;
    Timer WhirlWind_Timer;

    void Reset()
    {
        Hamstring_Timer.Reset(7000);
        MortalStrike_Timer.Reset(10000);
        WhirlWind_Timer.Reset(21000);

        boss_moroes_guestAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim() )
            return;

        boss_moroes_guestAI::UpdateAI(diff);

        
        if (Hamstring_Timer.Expired(diff))
        {
            DoCast(m_creature->getVictim(),SPELL_HAMSTRING);
            Hamstring_Timer = 12000;
        }

        
        if (MortalStrike_Timer.Expired(diff))
        {
            DoCast(m_creature->getVictim(), SPELL_MORTALSTRIKE);
            MortalStrike_Timer = 18000;
        }

        
        if (WhirlWind_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_WHIRLWIND);
            WhirlWind_Timer = 21000;
        }
    }
};

#define SPELL_DISARM            8379
#define SPELL_HEROICSTRIKE      29567
#define SPELL_SHIELDBASH        11972
#define SPELL_SHIELDWALL        29390

struct boss_lord_crispin_ferenceAI : public boss_moroes_guestAI
{
    //Arms Warr
    boss_lord_crispin_ferenceAI(Creature *c) : boss_moroes_guestAI(c) {}

    Timer Disarm_Timer;
    Timer HeroicStrike_Timer;
    Timer ShieldBash_Timer;
    Timer ShieldWall_Timer;

    void Reset()
    {
        Disarm_Timer.Reset(6000);
        HeroicStrike_Timer.Reset(10000);
        ShieldBash_Timer.Reset(8000);
        ShieldWall_Timer.Reset(4000);

        boss_moroes_guestAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim() )
            return;

        boss_moroes_guestAI::UpdateAI(diff);

        
        if (Disarm_Timer.Expired(diff))
        {
            DoCast(m_creature->getVictim(),SPELL_DISARM);
            Disarm_Timer = 12000;
        }

        
        if (HeroicStrike_Timer.Expired(diff))
        {
            DoCast(m_creature->getVictim(),SPELL_HEROICSTRIKE);
            HeroicStrike_Timer = 10000;
        }

        
        if (ShieldBash_Timer.Expired(diff))
        {
            DoCast(m_creature->getVictim(),SPELL_SHIELDBASH);
            ShieldBash_Timer = 13000;
        }

        
        if (ShieldWall_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_SHIELDWALL);
            ShieldWall_Timer = 21000;
        }
    }
};

CreatureAI* GetAI_boss_moroes(Creature *_Creature)
{
    return new boss_moroesAI (_Creature);
}

CreatureAI* GetAI_baroness_dorothea_millstipe(Creature *_Creature)
{
    return new boss_baroness_dorothea_millstipeAI (_Creature);
}

CreatureAI* GetAI_baron_rafe_dreuger(Creature *_Creature)
{
    return new boss_baron_rafe_dreugerAI (_Creature);
}

CreatureAI* GetAI_lady_catriona_von_indi(Creature *_Creature)
{
    return new boss_lady_catriona_von_indiAI (_Creature);
}

CreatureAI* GetAI_lady_keira_berrybuck(Creature *_Creature)
{
    return new boss_lady_keira_berrybuckAI (_Creature);
}

CreatureAI* GetAI_lord_robin_daris(Creature *_Creature)
{
    return new boss_lord_robin_darisAI (_Creature);
}

CreatureAI* GetAI_lord_crispin_ference(Creature *_Creature)
{
    return new boss_lord_crispin_ferenceAI (_Creature);
}

void AddSC_boss_moroes()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_moroes";
    newscript->GetAI = &GetAI_boss_moroes;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_baroness_dorothea_millstipe";
    newscript->GetAI = &GetAI_baroness_dorothea_millstipe;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_baron_rafe_dreuger";
    newscript->GetAI = &GetAI_baron_rafe_dreuger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_lady_catriona_von_indi";
    newscript->GetAI = &GetAI_lady_catriona_von_indi;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_lady_keira_berrybuck";
    newscript->GetAI = &GetAI_lady_keira_berrybuck;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_lord_robin_daris";
    newscript->GetAI = &GetAI_lord_robin_daris;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_lord_crispin_ference";
    newscript->GetAI = &GetAI_lord_crispin_ference;
    newscript->RegisterSelf();
}

