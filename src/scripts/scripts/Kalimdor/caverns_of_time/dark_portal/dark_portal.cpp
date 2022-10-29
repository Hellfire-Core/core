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
SDName: Dark_Portal
SD%Complete: 90
SDComment: Still post-event needed and support for Time Keepers
SDCategory: Caverns of Time, The Dark Portal
EndScriptData */

/* ContentData
npc_medivh_bm
npc_time_rift
npc_saat
EndContentData */

#include "precompiled.h"
#include "def_dark_portal.h"

#define SAY_ENTER               -1269020        //intro speach by Medivh when entering instance
#define SAY_INTRO               -1269021
#define SAY_WEAK75              -1269022
#define SAY_WEAK50              -1269023
#define SAY_WEAK25              -1269024
#define SAY_DEATH               -1269025
#define SAY_WIN                 -1269026
#define SAY_ORCS_ENTER          -1269027
#define SAY_ORCS_ANSWER         -1269028

#define SPELL_CHANNEL           31556

#define SPELL_PORTAL_RUNE       32570                       //aura(portal on ground effect)

#define SPELL_BLACK_CRYSTAL     32563                       //aura
#define SPELL_PORTAL_CRYSTAL    32564                       //summon

#define SPELL_BANISH_PURPLE     32566                       //aura
#define SPELL_BANISH_GREEN      32567                       //aura

#define SPELL_CORRUPT           31326
#define SPELL_CORRUPT_AEONUS    37853

#define C_COUNCIL_ENFORCER      17023

#define C_RKEEP 21104
#define C_RLORD 17839
#define C_ASSAS 17835
#define C_WHELP 21818
#define C_CHRON 17892
#define C_EXECU 18994
#define C_VANQU 18995

#define C_ORCS 17023

static float OrcsLocationStart[6][4]=
{
   {-2098.4f  , 7120.17f , 34.58f , 6.17f},
   {-2097.82f , 7122.57f , 34.58f , 6.13f },
   {-2097.47f , 7124.29f , 34.58f , 6.14f },
   {-2096.87f , 7126.23f , 34.58f , 6.15f},
   {-2096.45f , 7128.42f , 34.58f , 6.17f},
   {-2095.79f , 7131.01f , 34.58f , 6.15f },
};

static float OrcsLocationEnd[24][4]=
{
   { -2045.55f,  7115.2f,   23.9073f,  6.01854f },
   { -2040.33f,  7120.68f,  22.9844f,  3.00261f },
   { -2043.49f,  7130.63f,  24.1012f,  6.06172f },
   { -2045.01f,  7117.83f,  23.8104f,  6.15595f },
   { -2044.26f,  7124.12f,  23.8552f,  6.13632f },
   { -2043.57f,  7127.51f,  23.8659f,  6.17559f },

   { -2056.24f,  7116.63f,  27.7814f,  6.18345f },
   { -2055.97f,  7119.47f,  27.8518f,  6.1756f }, 
   { -2055.88f,  7122.5f ,  28.0675f,  6.12847f }, 
   { -2056.07f,  7126.16f,  28.6046f,  6.14811f },
   { -2055.67f,  7128.68f,  28.9588f,  6.16381f },
   { -2054.81f,  7131.32f,  29.5189f,  6.15203f },
    
   { -2073.43f,  7115.83f,  30.5849f,  6.24629f },
   { -2072.95f,  7119.7f ,  30.5851f,  6.21095f },
   { -2072.41f,  7123.06f,  30.5881f,  6.22666f },
   { -2071.72f,  7126.01f,  30.5871f,  6.18739f },
   { -2070.7f ,  7128.53f,  30.5687f,  6.18738f },
   { -2069.5f ,  7132    ,  30.483f ,  6.13555f },
    
   { -2084.54f,  7117.77f,  34.5878f,  0.0322094f }, 
   { -2084.07f,  7120.62f,  34.5878f,  6.229f },
   { -2084.01f,  7123.15f,  34.5863f,  6.18973f },
   { -2083.62f,  7125.65f,  34.5627f,  6.18973f },
   { -2083.48f,  7128    ,  34.5495f,  6.20937f },
   { -2083.2f ,  7130.69f,  34.5279f,  6.20544f },
};

struct npc_medivh_bmAI : public ScriptedAI
{
    npc_medivh_bmAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance *pInstance;

    Timer SpellCorrupt_Timer;
    Timer DamageMelee_Timer;
    Timer Check_Timer;
    Timer Delay_Timer;

    bool Life75;
    bool Life50;
    bool Life25;
    bool Intro;
    bool Delay;

    // Orcs after event
    Timer Orcs_Wave_Start_Timer;
    Timer Orcs_Wave_End_Timer;
    Timer Orcs_Wave_Clear_Timer;
    Timer Medivh_Speech_Timer;
    Timer Orc_General_Speech_Timer;
    int currentOrcWave;
    std::list<uint64> OrcsGUID;
    std::list<uint64>::iterator OrcIterator; // for waves
    Unit* OrcGeneral; // he will say text

    void Reset()
    {
        SpellCorrupt_Timer = 0;
        DamageMelee_Timer = 0;
        Delay_Timer = 0;

        Life75 = true;
        Life50 = true;
        Life25 = true;

        Intro = false;
        Delay = false;

        if (pInstance && (pInstance->GetData(TYPE_MEDIVH) == IN_PROGRESS))
            m_creature->CastSpell(m_creature,SPELL_CHANNEL, true);
        else if (m_creature->HasAura(SPELL_CHANNEL,0))
            m_creature->RemoveAura(SPELL_CHANNEL,0);

        Orcs_Wave_Start_Timer = 0;
        Orcs_Wave_End_Timer = 0;
        Orcs_Wave_Clear_Timer = 0;
        Medivh_Speech_Timer = 0;
        Orc_General_Speech_Timer = 0;
        OrcsGUID.clear();
        OrcGeneral = NULL;
        currentOrcWave = 0;
    }

    void MoveInLineOfSight(Unit *who)
    {
        //say enter phrase when in 50yd distance
        if (!Intro && pInstance->GetData(TYPE_MEDIVH) != DONE && who->GetTypeId() == TYPEID_PLAYER  && m_creature->IsWithinDistInMap(who, 50.0f))
        {
            m_creature->CastSpell(m_creature,SPELL_PORTAL_RUNE,true);
            m_creature->CastSpell(m_creature,SPELL_CHANNEL,false);
            DoScriptText(SAY_ENTER, m_creature);
            Intro = true;
            Delay_Timer = 15000;
        }

        if (pInstance->GetData(TYPE_MEDIVH) != DONE && who->GetTypeId() == TYPEID_PLAYER  && !((Player*)who)->IsGameMaster() && m_creature->IsWithinDistInMap(who, 10.0f))
        {
            if (pInstance->GetData(TYPE_MEDIVH) == IN_PROGRESS)
                return;

            if(!Delay_Timer.GetInterval())
                DoScriptText(SAY_INTRO, m_creature);
            else
                Delay = true;

            pInstance->SetData(TYPE_MEDIVH,IN_PROGRESS);
            Check_Timer = 5000;
        }
        else if (who->GetTypeId() == TYPEID_UNIT  && who->GetVictim() && who->GetVictim() == m_creature && m_creature->IsWithinDistInMap(who, 15.0f))
        {
            if (pInstance->GetData(TYPE_MEDIVH) != IN_PROGRESS)
                return;

            uint32 entry = who->GetEntry();
            if (entry == C_ASSAS || entry == C_WHELP || entry == C_CHRON || entry == C_EXECU || entry == C_VANQU)
            {
                who->StopMoving();
                who->CastSpell(m_creature,SPELL_CORRUPT,false);
            }
            else if (entry == 20737 || entry == 17881)  //Aeonus
            {
                who->StopMoving();
                who->CastSpell(m_creature,SPELL_CORRUPT_AEONUS,false);
            }
        }
    }

    void EnterCombat(Unit *who) {}

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if (SpellCorrupt_Timer.GetInterval())
            return;

        if (spell->Id == SPELL_CORRUPT_AEONUS)
            SpellCorrupt_Timer = 1000;

        if (spell->Id == SPELL_CORRUPT)
            SpellCorrupt_Timer = 3000;
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (done_by != m_creature)
            damage = 0;

        if (DamageMelee_Timer.GetInterval() > 0)
            return;

        if (done_by->GetEntry() == C_RLORD || done_by->GetEntry() == C_RKEEP)
            DamageMelee_Timer = 5000;
        else
            DamageMelee_Timer = 1000;
    }

    void JustDied(Unit* Killer)
    {
        pInstance->SetData(TYPE_MEDIVH, FAIL);

        DoScriptText(SAY_DEATH, m_creature);
    }

    void UpdateAI(const uint32 diff)
    {
        if (Delay_Timer.Expired(diff))
        {
            if (Delay)
                DoScriptText(SAY_INTRO, m_creature);
            Delay_Timer = 0;
        }
            
        



        if (SpellCorrupt_Timer.Expired(diff))
        {
            pInstance->SetData(TYPE_MEDIVH, SPECIAL);

            if (m_creature->HasAura(SPELL_CORRUPT_AEONUS, 0))
                SpellCorrupt_Timer = 1000;
            else if (m_creature->HasAura(SPELL_CORRUPT, 0))
                SpellCorrupt_Timer = 3000;
            else
                SpellCorrupt_Timer = 0;
        }
        



        if (DamageMelee_Timer.Expired(diff))
        {
            pInstance->SetData(TYPE_MEDIVH, SPECIAL);
            DamageMelee_Timer = 0;
        }
   


        if (Check_Timer.Expired(diff))
        {
            uint32 pct = pInstance->GetData(DATA_SHIELD);

            Check_Timer = 5000;

            if (Life25 && pct <= 25)
            {
                DoScriptText(SAY_WEAK25, m_creature);
                Life25 = false;
                Check_Timer = 0;
            }
            else if (Life50 && pct <= 50)
            {
                DoScriptText(SAY_WEAK50, m_creature);
                Life50 = false;
            }
            else if (Life75 && pct <= 75)
            {
                DoScriptText(SAY_WEAK75, m_creature);
                Life75 = false;
            }

            //if we reach this it means event was running but at some point reset.
            if (pInstance->GetData(TYPE_MEDIVH) == NOT_STARTED)
            {
                m_creature->DealDamage(m_creature, m_creature->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                m_creature->RemoveCorpse();
                m_creature->Respawn();
                return;
            }

            if (pInstance->GetData(TYPE_MEDIVH) == DONE)
            {
                DoScriptText(SAY_WIN, m_creature);
                Check_Timer = 0;
                Orcs_Wave_Start_Timer = 3000;
            }
        }

        if (Orcs_Wave_Start_Timer.Expired(diff))
        {
            for (int orc_counter = 0; orc_counter < 6; ++orc_counter)
            {
                Unit *orc = m_creature->SummonCreature(C_ORCS, OrcsLocationStart[orc_counter][0],OrcsLocationStart[orc_counter][1],OrcsLocationStart[orc_counter][2],OrcsLocationStart[orc_counter][3], TEMPSUMMON_DEAD_DESPAWN,0);
                orc->MonsterMoveWithSpeed(OrcsLocationEnd[orc_counter + 6 * currentOrcWave][0], OrcsLocationEnd[orc_counter + 6 * currentOrcWave][1], OrcsLocationEnd[orc_counter + 6 * currentOrcWave][2], 500 + 1625 * (4 - currentOrcWave), true, true);
                orc->setFaction( m_creature->getFaction() );
                OrcsGUID.push_back(orc->GetGUID());

                if (!OrcGeneral && orc_counter == 1 && currentOrcWave == 0)
                    OrcGeneral = orc;
            }

            if (currentOrcWave >= 3)
            {
                Orcs_Wave_Start_Timer = 0;
                Medivh_Speech_Timer = 9000;
                OrcsGUID.reverse();
                OrcIterator = OrcsGUID.begin();
            }
            else 
            {
                Orcs_Wave_Start_Timer = 2500;
                currentOrcWave++;
            }
        }

        if (Medivh_Speech_Timer.Expired(diff))
        {
            DoScriptText(SAY_ORCS_ENTER, m_creature);
            Orc_General_Speech_Timer = 9000;
            Medivh_Speech_Timer = 0;
        }

        if (Orc_General_Speech_Timer.Expired(diff))
        {
            DoScriptText(SAY_ORCS_ANSWER, OrcGeneral);
            Orcs_Wave_End_Timer = 10000;
            Orc_General_Speech_Timer = 0;
        }

        if (Orcs_Wave_End_Timer.Expired(diff))
        {
            if (!OrcsGUID.empty() && OrcIterator != OrcsGUID.end())
            {
                int orc_counter = 5;
                while (orc_counter >= 0)
                {
                    if (Creature * orc = m_creature->GetCreature(*OrcIterator++))
                    {
                        orc->MonsterMoveWithSpeed(OrcsLocationStart[orc_counter][0],OrcsLocationStart[orc_counter][1],OrcsLocationStart[orc_counter][2], 500 + 1625 * (4 - currentOrcWave), true, true);
                    }
                    orc_counter--;
                }
            }

            if (currentOrcWave < 0)
            {
                Orcs_Wave_End_Timer = 0;
                Orcs_Wave_Clear_Timer = 6500;
            }
            else 
            {
                Orcs_Wave_End_Timer = 500;
                currentOrcWave--;
            }
        }

        if (Orcs_Wave_Clear_Timer.Expired(diff))
        {
            if (!OrcsGUID.empty())
            {
                for (std::list<uint64>::iterator orcGUID = OrcsGUID.begin(); orcGUID != OrcsGUID.end(); ++orcGUID)
                {
                    if (Creature * orc = m_creature->GetCreature(*orcGUID))
                    {
                        orc->SetVisibility(VISIBILITY_OFF);
                        orc->ForcedDespawn();
                    }
                }
            }

            OrcsGUID.clear();
            Orcs_Wave_Clear_Timer = 0;
        }
    }
};

CreatureAI* GetAI_npc_medivh_bm(Creature *_Creature)
{
    return new npc_medivh_bmAI (_Creature);
}

struct Wave
{
    uint32 PortalMob[4];                                    //spawns for portal waves (in order)
};

struct npc_time_riftAI : public ScriptedAI
{
    npc_time_riftAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        c->setActive(true);
    }

    ScriptedInstance *pInstance;

    Timer TimeRiftWave_Timer;
    uint8 mRiftWaveCount;
    uint8 mPortalCount;
    uint8 mWaveId;

    void Reset()
    {

        TimeRiftWave_Timer.Reset(15000);
        mRiftWaveCount = 0;

        mPortalCount = pInstance->GetData(DATA_PORTAL_COUNT);

        if (mPortalCount < 6)
            mWaveId = 0;
        else if (mPortalCount > 12)
            mWaveId = 2;
        else
            mWaveId = 1;
    }

    void EnterCombat(Unit *who) {}

    void JustDied(Unit* who)
    {
        m_creature->RemoveCorpse();
    }

    void DoSummonAtRift(uint32 creature_entry)
    {
        if (!creature_entry)
            return;

        if (pInstance->GetData(TYPE_MEDIVH) != IN_PROGRESS)
        {
            m_creature->InterruptNonMeleeSpells(true);
            m_creature->RemoveAllAuras();
            return;
        }

        float x,y,z;
        m_creature->GetRandomPoint(m_creature->GetPositionX(),m_creature->GetPositionY(),m_creature->GetPositionZ(),10.0f,x,y,z);

        //normalize Z-level if we can, if rift is not at ground level.
        m_creature->UpdateAllowedPositionZ(x, y, z);
        Unit *Summon = m_creature->SummonCreature(creature_entry,x,y,z,m_creature->GetOrientation(),
            TEMPSUMMON_CORPSE_TIMED_DESPAWN,30000);
    }

    void DoSelectSummon()
    {
        Wave PortalWaves[]=
        {
            C_ASSAS, C_WHELP, C_CHRON, 0,
            C_EXECU, C_CHRON, C_WHELP, C_ASSAS,
            C_EXECU, C_VANQU, C_CHRON, C_ASSAS
        };

        uint32 entry = 0;

        if ((mRiftWaveCount > 2 && mWaveId < 1) || mRiftWaveCount > 3)
        {
            mRiftWaveCount = 0;
        }

        entry = PortalWaves[mWaveId].PortalMob[mRiftWaveCount];
        debug_log("TSCR: npc_time_rift: summoning wave creature (Wave %u, Entry %u).",mRiftWaveCount,entry);

        ++mRiftWaveCount;

        if (entry == C_WHELP)
        {
            for (uint8 i = 0; i < 3; i++)
                DoSummonAtRift(entry);
        }
        else
            DoSummonAtRift(entry);
    }

    void UpdateAI(const uint32 diff)
    {
        mPortalCount = pInstance->GetData(DATA_PORTAL_COUNT);


        if (TimeRiftWave_Timer.Expired(diff))
        {
            DoSelectSummon();

            if (mPortalCount > 0 && mPortalCount < 13)
                TimeRiftWave_Timer = urand(12000, 17000);
            else if (mPortalCount > 12 && mPortalCount < 18)
                TimeRiftWave_Timer = urand(7000, 12000);
            else
                TimeRiftWave_Timer = 0;

        }


        if (m_creature->IsNonMeleeSpellCast(false))
            return;

        debug_log("TSCR: npc_time_rift: not casting anylonger, i need to die.");
        m_creature->setDeathState(JUST_DIED);

        mRiftWaveCount = 0;
        pInstance->SetData(TYPE_RIFT,SPECIAL);
    }
};

CreatureAI* GetAI_npc_time_rift(Creature *_Creature)
{
    return new npc_time_riftAI (_Creature);
}

struct rift_summonAI : public ScriptedAI
{
    rift_summonAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        c->setActive(true);
    }

    ScriptedInstance *pInstance;

    Timer Spell_Timer1;
    Timer Spell_Timer2;
    Timer Spell_Timer3;
    Timer Spell_Timer4;

    uint8 Type;
    bool aggro;
    bool frenzy;

    void Reset()
    {
        Unit* medivh = Unit::GetUnit(*m_creature ,pInstance->GetData64(DATA_MEDIVH));

        m_creature->setActive(true);
        m_creature->SetNoCallAssistance(true);

        if (medivh && m_creature->GetEntry() != C_RKEEP && m_creature->GetEntry() != C_RLORD)
            AttackStart(medivh);

        Type = urand(0,1);

        switch (m_creature->GetEntry())
        {
            case C_RKEEP:
            {
                if(Type)    //mage
                {
                    Spell_Timer1.Reset(1000);                                    //Frostbolt
                    Spell_Timer2.Reset(HeroicMode ? 18500 : 12500);              //Pyroblast
                    Spell_Timer3.Reset(HeroicMode ? urand(12000, 27000) : 8000); //Blast Wave
                    Spell_Timer4.Reset(HeroicMode ? 15000 : 0);                  //Polymorph
                }
                else      //warlock
                {
                    Spell_Timer1.Reset(7000);                            //Shadow Bolt Volley
                    Spell_Timer2.Reset(HeroicMode ? 6000 : 10000);       //Curse of Vulnerability
                    Spell_Timer3.Reset(urand(3000, 23000));              //Fear
                }
                frenzy = false;
                break;
            }
            case C_RLORD:
            {
                if(Type)    //protection type
                {
                    Spell_Timer1.Reset(urand(6000, 12000));      //sunder armor
                    Spell_Timer2.Reset(urand(5000, HeroicMode ? 20000 : 25000));     //thunderclap
                }
                else        //fury-arms
                {
                    Spell_Timer1.Reset(HeroicMode ? urand(6200, 18800) : urand(4800, 18800));    //knockdown
                    Spell_Timer2.Reset(HeroicMode ? urand(4900, 17700) : urand(6100, 18000));    //mortal strike
                    Spell_Timer3.Reset(HeroicMode ? urand(4600, 15700) : urand(7200, 11800));    //harmstring
                }
                break;
            }
            case C_ASSAS:
            {
                if(Type)    //combat
                {
                    Spell_Timer1.Reset(HeroicMode ? urand(500, 7300) : urand(1200, 11100));      //sinister strike
                    Spell_Timer2.Reset(HeroicMode ? urand(1000, 15800) : urand(1900, 10100));    //rupture
                    Spell_Timer3.Reset(HeroicMode ? urand(800, 7800) : 0);                       //crippling poison
                }
                else        //assasin
                {
                    Spell_Timer1.Reset(urand(1200, 12400));                      //kidney shot
                    Spell_Timer2.Reset(HeroicMode ? urand(1000, 6500) : 0);      //deadly poison
                    Spell_Timer3 = 1;                                       //backstab
                }
                break;
            }
            case C_WHELP:
                break;
            case C_CHRON:
            {
                if(Type)    //frost
                {
                    Spell_Timer1 = 1;    //frostbolt
                    Spell_Timer2.Reset(HeroicMode ? urand(3600, 12200) : urand(3700, 12900));    //frost nova
                }
                else        //arcane
                {
                    Spell_Timer1 = 1;                       //arcane bolt
                    Spell_Timer2.Reset(urand(8600, 18500));      //arcane explosion
                }
                break;
            }
            case C_EXECU:
                Spell_Timer1.Reset(HeroicMode ? urand(2000, 11700) : urand(7300, 14000));    //cleave
                Spell_Timer2.Reset(HeroicMode ? urand(2000, 3900) : 7200);                   //strike
                Spell_Timer3.Reset(HeroicMode ? urand(600, 10200) : 0);                      //harmstring
                break;
            case C_VANQU:
                Spell_Timer1.Reset(1000);                //scorch + shadow bolt
                Spell_Timer2.Reset(urand(5900, 6000));   //fire blast
                break;
            default:
                break;
        }

    }

    void EnterCombat(Unit *who)
    {
        if (who->GetTypeId() == TYPEID_UNIT)
            aggro = false;

        if (who->GetTypeId() == TYPEID_UNIT  && m_creature->GetEntry() != C_WHELP)
        {
            if (rand()%10 == 0)   //10% chance on yell
            {
                switch (rand()%9)
                {
                  case 0: m_creature->MonsterYell("The wizard will fall!", 0, m_creature->GetGUID()); break;
                  case 1: m_creature->MonsterYell("We will not be stopped!", 0, m_creature->GetGUID()); break;
                  case 2: m_creature->MonsterYell("Victory or death!", 0, m_creature->GetGUID()); break;
                  case 3: m_creature->MonsterYell("You are running out of time!", 0, m_creature->GetGUID()); break;
                  case 4: m_creature->MonsterYell("The rift must be protected!", 0, m_creature->GetGUID()); break;
                  case 5: m_creature->MonsterYell("Your efforts... are in vain.", 0, m_creature->GetGUID()); break;
                  case 6: m_creature->MonsterYell("We are not finished!", 0, m_creature->GetGUID()); break;
                  case 7: m_creature->MonsterYell("Death to the Last Guardian!", 0, m_creature->GetGUID()); break;
                  case 8: m_creature->MonsterYell("We will not fail!", 0, m_creature->GetGUID()); break;
                }
            }
        }
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        if (!aggro && done_by->GetTypeId() == TYPEID_PLAYER)
        {
            AttackStart(done_by);
            aggro = true;
        }
    }

    void JustDied(Unit* who) {}

    void UpdateAI(const uint32 diff)
    {
        if (m_creature->GetVictim() && m_creature->GetVictim()->GetTypeId() == TYPEID_PLAYER)
        {
            switch (m_creature->GetEntry())
            {
                case C_RKEEP:
                {
                    if (Type)    //mage
                    {
                        if (Spell_Timer1.Expired(diff))   //frostbolt
                        {
                            AddSpellToCast(m_creature->GetVictim(), HeroicMode?38534:36279);
                            Spell_Timer1 = urand(8000, HeroicMode ? 10000 : 16000);
                        }
                        

                        
                        if (Spell_Timer2.Expired(diff))    //pyroblast
                        {
                            Spell_Timer1 = 8000;

                            Unit* target = SelectUnit(SELECT_TARGET_NEAREST, 0, 70, true, m_creature->getVictimGUID());
                            if (!target)
                                target = m_creature->GetVictim();

                            if (target)
                                AddSpellToCast(target, HeroicMode?38535:36277);

                            Spell_Timer2 = HeroicMode ? urand(14000, 24000) : urand(12000, 17000);
                        }
                        

                        
                        if (Spell_Timer3.Expired(diff))    //blast wave
                        {
                            ForceSpellCast(m_creature, HeroicMode?38536:36278, DONT_INTERRUPT, true);
                            Spell_Timer3 = HeroicMode ? urand(15000, 25000) : 13000;
                        }
                        

                        
                        if (HeroicMode && Spell_Timer4.Expired(diff))    //polymorph
                        {
                            Unit* target = SelectUnit(SELECT_TARGET_NEAREST, 0, 70, true, m_creature->getVictimGUID());
                            if (target)
                                AddSpellToCast(target, 13323);
                            Spell_Timer4 = 30000;
                        }
                        
                    }
                    else       //warlock
                    {
                        
                        if (Spell_Timer1.Expired(diff))   //shadow bolt volley
                        {
                            AddSpellToCast(m_creature->GetVictim(), HeroicMode?38533:36275);
                            Spell_Timer1 = HeroicMode ? Spell_Timer2.GetTimeLeft() + 1500 : urand(10000, 25000);
                        }
                        

                      
                        if (Spell_Timer2.Expired(diff))    //curse of vulnerability
                        {
                            Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0,70,true);
                            if (target)
                                AddSpellToCast(target, 36276, true);
                            Spell_Timer2 = HeroicMode ? urand(9000, 14000) : Spell_Timer1.GetTimeLeft() + 2000;
                        }
                        

                        
                        if (Spell_Timer3.Expired(diff))    //fear
                        {
                            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 70, true);
                            if (target)
                                AddSpellToCast(target, 12542);
                            Spell_Timer3 = urand(15000, 25000);
                        }
                        

                        if (!frenzy && m_creature->GetHealth()*100 / m_creature->GetMaxHealth() < 30)
                        {
                            AddSpellToCast(m_creature, 8269, true);
                            frenzy = true;
                        }
                    }
                    break;
                }
                case C_RLORD:
                {
                    if(Type)    //protection type
                    {
                        if (Spell_Timer1.Expired(diff))   //sunder armor
                        {
                            AddSpellToCast(m_creature->GetVictim(), 16145, true);
                            Spell_Timer1 = urand(6000, 9000);
                        }
                        

                       
                        if (Spell_Timer2.Expired(diff))    //thunderclap
                        {
                            AddSpellToCast(m_creature, HeroicMode?38537:36214, true);
                            Spell_Timer2 = HeroicMode ? urand(12000, 17000) : urand(10000, 25000);
                        }
                        
                    }
                    else    //fury-arms
                    {
                        
                        if (Spell_Timer1.Expired(diff))   //knockback
                        {
                            AddSpellToCast(m_creature->GetVictim(), 11428, true);
                            Spell_Timer1 = HeroicMode ? urand(13300, 19100) : urand(18100, 38500);
                        }
                        

                        
                        if (Spell_Timer2.Expired(diff))    //mortal strike
                        {
                            AddSpellToCast(m_creature->GetVictim(), HeroicMode?35054:15708, true);
                            Spell_Timer2 = HeroicMode ? urand(10300, 14500) : urand(10800, 15800);
                        }
                        

                        
                        if (Spell_Timer3.Expired(diff))    //harmstring
                        {
                            AddSpellToCast(m_creature->GetVictim(), 9080, true);
                            Spell_Timer3 = HeroicMode ? urand(11600, 18100) : urand(15500, 26500);
                        }
                    }

                    if (!frenzy && m_creature->GetHealth()*100 / m_creature->GetMaxHealth() < 30)
                    {
                        AddSpellToCast(m_creature, 8269, true);
                        frenzy = true;
                    }
                    break;
                }
                case C_ASSAS:
                {
                    if (Type)    //combat
                    {
             
                        if (Spell_Timer1.Expired(diff))   //sinister strike
                        {
                            AddSpellToCast(m_creature->GetVictim(), HeroicMode?15667:14873, true);
                            Spell_Timer1 = HeroicMode ? urand(3500, 14500) : urand(4500, 15300);
                        }
                        
                        
                        if (Spell_Timer2.Expired(diff))    //rupture
                        {
                            AddSpellToCast(m_creature->GetVictim(), HeroicMode?15583:14874, true);
                            Spell_Timer2 = HeroicMode ? urand(10100, 20500) : urand(10400, 21600);
                        }
                        

                        
                        if (Spell_Timer3.Expired(diff))    //crippling poison
                        {
                            AddSpellToCast(m_creature->GetVictim(), 30981, true);
                            Spell_Timer3 = HeroicMode ? urand(12200, 62800) : 0;
                        }
                        
                    }
                    else        //assasin
                    {
                        
                        if (Spell_Timer1.Expired(diff))   //kidney shot
                        {
                            AddSpellToCast(m_creature->GetVictim(), 30832, true);
                            Spell_Timer1 = urand(20100, 24900);
                        }
                        else


                            
                        if (Spell_Timer2.Expired(diff))    //deadly poison
                        {
                            AddSpellToCast(m_creature->GetVictim(), 38520, true);
                            Spell_Timer2 = HeroicMode ? urand(12300, 24200) : 0;
                        }
                        

                        
                        if (Spell_Timer3.Expired(diff))    //backstab
                        {
                            AddSpellToCast(m_creature->GetVictim(), HeroicMode?15657:7159, true);
                            Spell_Timer3 = urand(4800, 7200);
                        }
                    }
                    break;
                }
                case C_WHELP:
                    break;
                case C_CHRON:
                {
                    if (m_creature->GetPower(POWER_MANA)*100/m_creature->GetMaxPower(POWER_MANA) > 15)
                    {
                        if (Type)    //frost
                        {
                            
                            if (Spell_Timer1.Expired(diff))   //frostbolt
                            {
                                AddSpellToCast(m_creature->GetVictim(), HeroicMode?12675:15497);
                                Spell_Timer1 = urand(2900, 5400);
                            }
                            

                            
                            if (Spell_Timer2.Expired(diff) && m_creature->IsWithinCombatRange(m_creature->GetVictim(), 10))    //frost nova
                            {
                                AddSpellToCast(m_creature, HeroicMode?15531:15063, true);
                                Spell_Timer2.Reset(HeroicMode ? urand(22200, 25700) : urand(33800, 39800));
                            }
                            
                        }
                        else    //arcane
                        {
                            
                            if (Spell_Timer1.Expired(diff))   //arcane bolt
                            {
                                AddSpellToCast(m_creature->GetVictim(), HeroicMode?15230:15124);
                                Spell_Timer1 = HeroicMode ? urand(1200, 3400) : urand(2900, 5400);
                            }
                            

                            if (Spell_Timer2.Expired(diff) && m_creature->IsWithinCombatRange(m_creature->GetVictim(), 10))    //arcane explosion
                            {
                                AddSpellToCast(m_creature, HeroicMode?33623:33860, true);
                                Spell_Timer2.Reset(HeroicMode ? urand(8000, 10100) : urand(9500, 10100));
                            }
                        }
                    }
                    break;
                }
                case C_EXECU:
                {
                    
                    if (Spell_Timer1.Expired(diff))   //cleave
                    {
                        AddSpellToCast(m_creature->GetVictim(), 15496, true);
                        Spell_Timer1 = HeroicMode ? urand(6000, 11700) : urand(7300, 14000);
                    }
                    

                    
                    if (Spell_Timer2.Expired(diff))    //strike
                    {
                        AddSpellToCast(m_creature->GetVictim(), HeroicMode?34920:15580, true);
                        Spell_Timer2 = HeroicMode ? urand(3900, 9700) : urand(9700, 20300);
                    }
                    

                    
                    if (Spell_Timer3.Expired(diff))    //harmstring
                    {
                        AddSpellToCast(m_creature->GetVictim(), 9080, true);
                        Spell_Timer3 = HeroicMode ? urand(10800, 15800) : 0;
                    }
                    

                    break;
                }
                case C_VANQU:
                {
                    if (m_creature->GetPower(POWER_MANA)*100/m_creature->GetMaxPower(POWER_MANA) > 15)
                    {
                        
                        if (Spell_Timer1.Expired(diff))   //scorch + shadow bolt
                        {
                            bool fire = urand(0,1);
                            if (fire)
                                AddSpellToCast(m_creature->GetVictim(), HeroicMode?36807:15241);
                            else
                                AddSpellToCast(m_creature->GetVictim(), HeroicMode?15472:12739);
                            Spell_Timer1 = urand(3500, 4500);
                        }
                        

                        
                        if (Spell_Timer2.Expired(diff))    //fire blast
                        {
                            AddSpellToCast(m_creature->GetVictim(), HeroicMode?38526:13341, true);
                            Spell_Timer2 = urand(5900, 6000);
                        }
                    }
                    break;
                }
                default:
                    break;
            }

            CastNextSpellIfAnyAndReady();
            DoMeleeAttackIfReady();
        }

        if (pInstance->GetData(TYPE_MEDIVH) == FAIL)
        {
            m_creature->Kill(m_creature, false);
            m_creature->RemoveCorpse();
        }
    }
};

CreatureAI* GetAI_rift_summon(Creature *_Creature)
{
    return new rift_summonAI (_Creature);
}

#define SAY_SAAT_WELCOME        -1269019

#define GOSSIP_ITEM_OBTAIN      "[PH] Obtain Chrono-Beacon"
#define SPELL_CHRONO_BEACON     34975
#define ITEM_CHRONO_BEACON      24289

bool GossipHello_npc_saat(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu(_Creature->GetGUID());

    if (player->GetQuestStatus(QUEST_OPENING_PORTAL) == QUEST_STATUS_INCOMPLETE && !player->HasItemCount(ITEM_CHRONO_BEACON,1))
    {
        player->ADD_GOSSIP_ITEM(0,GOSSIP_ITEM_OBTAIN,GOSSIP_SENDER_MAIN,GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(10000,_Creature->GetGUID());
        return true;
    }
    else if (player->GetQuestRewardStatus(QUEST_OPENING_PORTAL) && !player->HasItemCount(ITEM_CHRONO_BEACON,1))
    {
        player->ADD_GOSSIP_ITEM(0,GOSSIP_ITEM_OBTAIN,GOSSIP_SENDER_MAIN,GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(10001,_Creature->GetGUID());
        return true;
    }

    player->SEND_GOSSIP_MENU(10002,_Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_saat(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->CastSpell(player,SPELL_CHRONO_BEACON,false);
    }
    return true;
}

void AddSC_dark_portal()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_medivh_bm";
    newscript->GetAI = &GetAI_npc_medivh_bm;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_time_rift";
    newscript->GetAI = &GetAI_npc_time_rift;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "rift_summon";
    newscript->GetAI = &GetAI_rift_summon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_saat";
    newscript->pGossipHello = &GossipHello_npc_saat;
    newscript->pGossipSelect = &GossipSelect_npc_saat;
    newscript->RegisterSelf();
}
