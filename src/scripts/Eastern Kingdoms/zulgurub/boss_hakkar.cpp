/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Boss_Hakkar
SD%Complete: 95
SDComment: Blood siphon spell buggy cause of Core Issue.
SDCategory: Zul'Gurub
EndScriptData */

#include "scriptPCH.h"
#include "def_zulgurub.h"

#define SAY_AGGRO                   -1309020
#define SAY_FLEEING                 -1309021
#define SAY_MINION_DESTROY          -1309022                //where does it belong?
#define SAY_PROTECT_ALTAR           -1309023                //where does it belong?

#define SPELL_BLOODSIPHON            24322
#define SPELL_CORRUPTEDBLOOD         24328
#define SPELL_CAUSEINSANITY          24327                  //Not working disabled.
#define SPELL_WILLOFHAKKAR           24178
#define SPELL_ENRAGE                 24318

// The Aspects of all High Priests
#define SPELL_ASPECT_OF_JEKLIK       24687
#define SPELL_ASPECT_OF_VENOXIS      24688
#define SPELL_ASPECT_OF_MARLI        24686
#define SPELL_ASPECT_OF_THEKAL       24689
#define SPELL_ASPECT_OF_ARLOKK       24690

struct boss_hakkarAI : public ScriptedAI
{
    boss_hakkarAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    int32 BloodSiphon_Timer;
    int32 CorruptedBlood_Timer;
    int32 CauseInsanity_Timer;
    int32 WillOfHakkar_Timer;
    int32 Enrage_Timer;

    int32 CheckJeklik_Timer;
    int32 CheckVenoxis_Timer;
    int32 CheckMarli_Timer;
    int32 CheckThekal_Timer;
    int32 CheckArlokk_Timer;

    int32 AspectOfJeklik_Timer;
    int32 AspectOfVenoxis_Timer;
    int32 AspectOfMarli_Timer;
    int32 AspectOfThekal_Timer;
    int32 AspectOfArlokk_Timer;

    bool Enraged;

    void Reset()
    {
        BloodSiphon_Timer = 90000;
        CorruptedBlood_Timer = 25000;
        CauseInsanity_Timer = 17000;
        WillOfHakkar_Timer = 17000;
        Enrage_Timer = 600000;

        CheckJeklik_Timer = 1000;
        CheckVenoxis_Timer = 2000;
        CheckMarli_Timer = 3000;
        CheckThekal_Timer = 4000;
        CheckArlokk_Timer = 5000;

        AspectOfJeklik_Timer = 4000;
        AspectOfVenoxis_Timer = 7000;
        AspectOfMarli_Timer = 12000;
        AspectOfThekal_Timer = 8000;
        AspectOfArlokk_Timer = 18000;

        Enraged = false;

        pInstance->SetData(DATA_HAKKAREVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);
        pInstance->SetData(DATA_HAKKAREVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        pInstance->SetData(DATA_HAKKAREVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        BloodSiphon_Timer -= diff;
        if (BloodSiphon_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_BLOODSIPHON);
            BloodSiphon_Timer += 90000;
        }
        

        CorruptedBlood_Timer -= diff;
        if (CorruptedBlood_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_CORRUPTEDBLOOD);
            CorruptedBlood_Timer += 30000 + rand()%15000;
        }
        

        /*
        CauseInsanity_Timer -= diff;
        if (CauseInsanity_Timer <= diff)
        {
        if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
        DoCast(target,SPELL_CAUSEINSANITY);

        CauseInsanity_Timer += 35000 + rand()%8000;
        }*/

        WillOfHakkar_Timer -= diff;
        if (WillOfHakkar_Timer <= diff)
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0,GetSpellMaxRange(SPELL_WILLOFHAKKAR), true))
                DoCast(target,SPELL_WILLOFHAKKAR);

            WillOfHakkar_Timer += 25000 + rand()%10000;
        }
        

        Enrage_Timer -= diff;
        if (!Enraged && Enrage_Timer <= diff)
        {
            DoCast(m_creature, SPELL_ENRAGE);
            Enraged = true;
        }
        

        CheckJeklik_Timer -= diff;
        if(CheckJeklik_Timer <= diff)
        {
            if(pInstance)
            {
                if(pInstance->GetData(DATA_JEKLIKEVENT) != DONE)
                {
                    AspectOfJeklik_Timer -= diff;
                    if (AspectOfJeklik_Timer <= diff)
                    {
                        DoCast(m_creature->GetVictim(),SPELL_ASPECT_OF_JEKLIK);
                        AspectOfJeklik_Timer += 10000 + rand()%4000;
                    }
                }
            }
            CheckJeklik_Timer += 1000;
        }
        
        CheckVenoxis_Timer -= diff;
        //Checking if Venoxis is dead. If not we cast his Aspect
        if(CheckVenoxis_Timer <= diff)
        {
            if(pInstance)
            {
                if(pInstance->GetData(DATA_VENOXISEVENT) != DONE)
                {
                    AspectOfVenoxis_Timer -= diff;
                    if (AspectOfVenoxis_Timer <= diff)
                    {
                        DoCast(m_creature->GetVictim(),SPELL_ASPECT_OF_VENOXIS);
                        AspectOfVenoxis_Timer += 8000;
                    }
                }
            }
            CheckVenoxis_Timer += 1000;
        }
        

        CheckMarli_Timer -= diff;
        //Checking if Marli is dead. If not we cast her Aspect
        if(CheckMarli_Timer <= diff)
        {
            if(pInstance)
            {
                if (pInstance->GetData(DATA_MARLIEVENT) != DONE)
                {
                    AspectOfMarli_Timer -= diff;
                    if (AspectOfMarli_Timer <= diff)
                    {
                        DoCast(m_creature->GetVictim(),SPELL_ASPECT_OF_MARLI);
                        AspectOfMarli_Timer += 10000;
                    }

                }
            }
            CheckMarli_Timer += 1000;
        }
        

        CheckThekal_Timer -= diff;
        //Checking if Thekal is dead. If not we cast his Aspect
        if(CheckThekal_Timer <= diff)
        {
            if(pInstance)
            {
                if(pInstance->GetData(DATA_THEKALEVENT) != DONE)
                {
                    AspectOfThekal_Timer -= diff;
                    if (AspectOfThekal_Timer <= diff)
                    {
                        DoCast(m_creature,SPELL_ASPECT_OF_THEKAL);
                        AspectOfThekal_Timer += 15000;
                    }
                }
            }
            CheckThekal_Timer += 1000;
        }
        

        CheckArlokk_Timer -= diff;
        //Checking if Arlokk is dead. If yes we cast her Aspect
        if(CheckArlokk_Timer <= diff)
        {
            if(pInstance)
            {
                if(pInstance->GetData(DATA_ARLOKKEVENT) != DONE)
                {
                    AspectOfArlokk_Timer -= diff;
                    if (AspectOfArlokk_Timer <= diff)
                    {
                        DoCast(m_creature,SPELL_ASPECT_OF_ARLOKK);
                        DoResetThreat();

                        AspectOfArlokk_Timer += 10000 + rand()%5000;
                    }
                }
            }
            CheckArlokk_Timer += 1000;
        }
        

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_hakkar(Creature *_Creature)
{
    return new boss_hakkarAI (_Creature);
}

void AddSC_boss_hakkar()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_hakkar";
    newscript->GetAI = &GetAI_boss_hakkar;
    newscript->RegisterSelf();
}

