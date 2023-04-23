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
#include "def_blackrock_depths.h"


#define BOSS_COREN_DIREBREW             23872
#define NPC_ILSA_DIREBREW               26764
#define NPC_URSULA_DIREBREW             26822
#define NPC_DARK_IRON_ANTAGONIST        23795
#define COREN_TEXT              "You'll pay for this insult!"

#define SPELL_SUMMON_MINION_KNOCKBACK   50313 //wrong?
#define SPELL_SUMMON_MINION             47375
#define SPELL_DIREBREW_CHARGE           47718
#define SPELL_DIREBREWS_DISARM          47310
#define SPELL_DISARM_GROW               47409
#define SPELL_SUMMON_MOLE_MACHINE       43563


//////////////////////
//Coren Direbrew
//////////////////////
struct boss_coren_direbrewAI : public ScriptedAI
{
    boss_coren_direbrewAI(Creature *c) : ScriptedAI(c) { }

    int32 Disarm_Timer;
    int32 Summon_Timer;
    int32 Drink_Timer;
    int32 Ilsa_Timer;
    int32 Ursula_Timer;

    void Reset()
    {
        Disarm_Timer = 20000;
        Summon_Timer = 15000;
        Ilsa_Timer = 0;
        Ursula_Timer = 0;
        
        //me->setFaction(35);

        std::list<Creature*> antagonistList;
        MaNGOS::AllCreaturesOfEntryInRange check(me, NPC_DARK_IRON_ANTAGONIST, 100);
        MaNGOS::ObjectListSearcher<Creature, MaNGOS::AllCreaturesOfEntryInRange> searcher(antagonistList, check);

        Cell::VisitGridObjects(me, searcher, 100);

        if(antagonistList.size())
        {
            for(std::list<Creature*>::iterator i = antagonistList.begin(); i != antagonistList.end(); ++i)
            {
                //(*i)->setFaction(35);
                (*i)->Respawn();
            }
        }

    }

    void SummonedCreatureDespawn(Creature * creature)
    {
        switch(creature->GetEntry())
        {
            case NPC_ILSA_DIREBREW:
                Ilsa_Timer = 5000;
                break;
            case NPC_URSULA_DIREBREW:
                Ursula_Timer = 10000;
                break;
            default:
                break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        Disarm_Timer -= diff;
        if(Disarm_Timer <= diff)
        {
            AddSpellToCast(SPELL_DISARM_GROW, CAST_SELF, true);
            AddSpellToCast(SPELL_DISARM_GROW, CAST_SELF, true);
            AddSpellToCast(SPELL_DISARM_GROW, CAST_SELF, true);
            AddSpellToCast(SPELL_DIREBREWS_DISARM, CAST_NULL);
            Disarm_Timer += 30000;
        }
      
        Summon_Timer -= diff;
        if(Summon_Timer <= diff)
        {
            if(Unit * target = SelectUnit(SELECT_TARGET_RANDOM, 0, 45, true))
            {
                AddSpellToCast(target, SPELL_SUMMON_MOLE_MACHINE);
                //me->SummonGameObject(188478, me->GetVictim()->GetPositionX(), me->GetVictim()->GetPositionY(), me->GetVictim()->GetPositionZ(), 0,0 ,0 ,0 ,0, 4);
                //me->GetVictim()->KnockBackFrom(me, 4, 7);
                //AddSpellToCast(me, SPELL_SUMMON_MINION_KNOCKBACK);
                AddSpellToCast(target, SPELL_SUMMON_MINION, true);
                Summon_Timer += 15000;
            }
        }


        if(float(me->GetHealth())/float(me->GetMaxHealth()) < 0.66f)
        {
            Ilsa_Timer -= diff;
            if (Ilsa_Timer <= diff)
            {
                Creature * Ilsa = GetClosestCreatureWithEntry(me, NPC_ILSA_DIREBREW, 100);
                if (!Ilsa)
                {
                    me->SummonCreature(NPC_ILSA_DIREBREW, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
                }
                Ilsa_Timer += 300000;
            }
        }

        if(float(me->GetHealth())/float(me->GetMaxHealth()) < 0.33f)
        {
            Ursula_Timer -= diff;
            if(Ursula_Timer <= diff)
            {
                Creature * Ursula = GetClosestCreatureWithEntry(me, NPC_URSULA_DIREBREW, 100);
                if (!Ursula)
                {
                    me->SummonCreature(NPC_URSULA_DIREBREW, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
                }
                Ursula_Timer += 300000;
            }
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_coren_direbrew(Creature *creature)
{
    return new boss_coren_direbrewAI (creature);
}



//////////////////////
//Trigger
//////////////////////
struct direbrew_starter_triggerAI : public ScriptedAI
{
    direbrew_starter_triggerAI(Creature *c) : ScriptedAI(c) { }

    uint32 Start_Timer;

    void Reset()
    {
        Start_Timer = 1000;
    }

    void UpdateAI(const uint32 diff)
    {
        Start_Timer -= diff;
        if(Start_Timer <= diff)
        {
            Creature * Coren = GetClosestCreatureWithEntry(me, BOSS_COREN_DIREBREW, 20);
            if (Coren && Coren->IsAlive())
            {
                Coren->Say(COREN_TEXT, LANG_UNIVERSAL, 0);
                Coren->setFaction(54);

                std::list<Creature*> antagonistList;
                MaNGOS::AllCreaturesOfEntryInRange check(me, NPC_DARK_IRON_ANTAGONIST, 100);
                MaNGOS::ObjectListSearcher<Creature, MaNGOS::AllCreaturesOfEntryInRange> searcher(antagonistList, check);

                Cell::VisitGridObjects(me, searcher, 100);

                if(antagonistList.size())
                {
                    for(std::list<Creature*>::iterator i = antagonistList.begin(); i != antagonistList.end(); ++i)
                    {
                        (*i)->setFaction(54);
                    }
                }

                me->ForcedDespawn(0);
            }
        }
    }
};

CreatureAI* GetAI_direbrew_starter_trigger(Creature *creature)
{
    return new direbrew_starter_triggerAI (creature);
}

void AddSC_boss_coren_direbrew()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="direbrew_starter_trigger";
    newscript->GetAI = &GetAI_direbrew_starter_trigger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_coren_direbrew";
    newscript->GetAI = &GetAI_boss_coren_direbrew;
    newscript->RegisterSelf();
}
