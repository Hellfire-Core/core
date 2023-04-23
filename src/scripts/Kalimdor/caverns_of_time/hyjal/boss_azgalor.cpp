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
#include "def_hyjal.h"
#include "hyjal_trash.h"

#define SPELL_RAIN_OF_FIRE 31340
#define SPELL_DOOM 31347
#define SPELL_HOWL_OF_AZGALOR 31344
#define SPELL_CLEAVE 31345
#define SPELL_BERSERK 26662

#define SAY_ONDEATH "Your time is almost... up"
#define SOUND_ONDEATH 11002

#define SAY_ONSLAY1 "Reesh, hokta!"
#define SAY_ONSLAY2 "Don't fight it"
#define SAY_ONSLAY3 "No one is going to save you"
#define SOUND_ONSLAY1 11001
#define SOUND_ONSLAY2 11048
#define SOUND_ONSLAY3 11047

#define SAY_DOOM1 "Just a taste... of what awaits you"
#define SAY_DOOM2 "Suffer you despicable insect!"
#define SOUND_DOOM1 11046
#define SOUND_DOOM2 11000

#define SAY_ONAGGRO "Abandon all hope! The legion has returned to finish what was begun so many years ago. This time there will be no escape!"
#define SOUND_ONAGGRO 10999

struct boss_azgalorAI : public hyjal_trashAI
{
    boss_azgalorAI(Creature *c) : hyjal_trashAI(c)
    {
        pInstance = (c->GetInstanceData());
        go = false;
        pos = 0;
        SpellEntry *TempSpell = (SpellEntry*)GetSpellStore()->LookupEntry(SPELL_HOWL_OF_AZGALOR);
        if(TempSpell)
            TempSpell->EffectRadiusIndex[0] = 12;//100yards instead of 50000?!
    }

    Timer RainTimer;
    Timer DoomTimer;
    Timer HowlTimer;
    Timer CleaveTimer;
    Timer EnrageTimer;
    Timer CheckTimer;
    bool enraged;

    bool go;
    uint32 pos;

    void Reset()
    {
        damageTaken = 0;
        RainTimer.Reset(20000);
        DoomTimer.Reset(50000);
        HowlTimer.Reset(30000);
        CleaveTimer.Reset(10000);
        EnrageTimer.Reset(600000);
        CheckTimer.Reset(3000);
        enraged = false;

        if(pInstance && IsEvent)
            pInstance->SetData(DATA_AZGALOREVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if(pInstance && IsEvent)
            pInstance->SetData(DATA_AZGALOREVENT, IN_PROGRESS);

        DoPlaySoundToSet(m_creature, SOUND_ONAGGRO);
        DoYell(SAY_ONAGGRO, LANG_UNIVERSAL, NULL);
    }

    void KilledUnit(Unit *victim)
    {
        switch(rand()%3)
        {
            case 0:
                DoPlaySoundToSet(m_creature, SOUND_ONSLAY1);
                DoYell(SAY_ONSLAY1, LANG_UNIVERSAL, NULL);
                break;
            case 1:
                DoPlaySoundToSet(m_creature, SOUND_ONSLAY2);
                DoYell(SAY_ONSLAY2, LANG_UNIVERSAL, NULL);
                break;
            case 2:
                DoPlaySoundToSet(m_creature, SOUND_ONSLAY3);
                DoYell(SAY_ONSLAY3, LANG_UNIVERSAL, NULL);
                break;
        }
    }

    void WaypointReached(uint32 i)
    {
        pos = i;
        if (i == 7 && pInstance)
        {
            Unit* target = Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_THRALL));
            if (target && target->IsAlive())
            {
                m_creature->AddThreat(target,0.0);
                AttackStart(target);
            }
            else
            {
                if(target = m_creature->SelectNearbyTarget(200.0))
                    AttackStart(target);
            }
        }
    }

    void JustDied(Unit *Killer)
    {
        hyjal_trashAI::JustDied(Killer);
        if(pInstance && IsEvent)
            pInstance->SetData(DATA_AZGALOREVENT, DONE);

        DoPlaySoundToSet(m_creature, SOUND_ONDEATH);
    }

    void UpdateAI(const uint32 diff)
    {
        if (IsEvent)
        {
            //Must update npc_escortAI
            npc_escortAI::UpdateAI(diff);
            if(!go)
            {
                go = true;
                if(pInstance)
                {
                    AddWaypoint(0, 5492.91,    -2404.61,    1462.63);
                    AddWaypoint(1, 5531.76,    -2460.87,    1469.55);
                    AddWaypoint(2, 5554.58,    -2514.66,    1476.12);
                    AddWaypoint(3, 5554.16,    -2567.23,    1479.90);
                    AddWaypoint(4, 5540.67,    -2625.99,    1480.89);
                    AddWaypoint(5, 5508.16,    -2659.2,    1480.15);
                    AddWaypoint(6, 5489.62,    -2704.05,    1482.18);
                    AddWaypoint(7, 5457.04,    -2726.26,    1485.10);
                    Start(false, true);
                    SetDespawnAtEnd(false);
                }
            }
        }

        //Return since we have no target
        if (!UpdateVictim() )
            return;

        
        if (CheckTimer.Expired(diff))
        {
            DoZoneInCombat();
            m_creature->SetSpeed(MOVE_RUN, 3.0);
            CheckTimer = 1000;
        }
        

        
        if (RainTimer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40, true))
            {
                DoCast(target, SPELL_RAIN_OF_FIRE);
                RainTimer = urand(20000, 35000);
            }
        }
        

        
        //only set timer when target exist, cause with exclude defined we return NULL that now can be acceptable spell target
        if (DoomTimer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true, m_creature->getVictimGUID()))
            {
                DoCast(target, SPELL_DOOM);//never on tank
                DoomTimer = urand(45000, 50000);
            }
        }
        

        
        if (HowlTimer.Expired(diff))
        {
            DoCast(m_creature, SPELL_HOWL_OF_AZGALOR);
            HowlTimer = 30000;
        }
        


        if (CleaveTimer.Expired(diff))
        {
            if(Unit *target = m_creature->GetVictim())
            {
                DoCast(target, SPELL_CLEAVE);
                CleaveTimer = urand(10000, 15000);
            }
        }
        

        
        if (!enraged && EnrageTimer.Expired(diff))
        {
            m_creature->InterruptNonMeleeSpells(false);
            DoCast(m_creature, SPELL_BERSERK, true);
            enraged = true;
            EnrageTimer = 600000;
        }
        

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_azgalor(Creature *_Creature)
{
    return new boss_azgalorAI (_Creature);
}

#define SPELL_THRASH 12787
#define SPELL_CRIPPLE 31406
#define SPELL_WARSTOMP 31408

struct mob_lesser_doomguardAI : public hyjal_trashAI
{
    mob_lesser_doomguardAI(Creature *c) : hyjal_trashAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    Timer CrippleTimer;
    Timer WarstompTimer;
    Timer CheckTimer;

    ScriptedInstance* pInstance;

    void Reset()
    {
        CrippleTimer.Reset(50000);
        WarstompTimer.Reset(10000);
        CheckTimer.Reset(2000);
    }

    void WaypointReached(uint32){}

    void JustRespawned()
    {
        DoCast(m_creature, SPELL_THRASH, true);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!m_creature->IsInCombat() && m_creature->IsWithinDistInMap(who, 50) && m_creature->IsHostileTo(who))
        {
            m_creature->AddThreat(who,0.0);
            m_creature->Attack(who,false);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        
        if (CheckTimer.Expired(diff))
        {
            if(pInstance)
            {
                Creature *pAzgalor = pInstance->GetCreature(pInstance->GetData64(DATA_AZGALOR));
                if(!pAzgalor || !pAzgalor->IsAlive())
                {
                    m_creature->setDeathState(JUST_DIED);
                    m_creature->RemoveCorpse();
                    return;
                }
            }
            CheckTimer = 2000;
        }
        

        //Return since we have no target
        if (!UpdateVictim() )
            return;

        
        if (WarstompTimer.Expired(diff))
        {
            DoCast(m_creature, SPELL_WARSTOMP);
            WarstompTimer = urand(10000, 25000);
        }
        

        if (CrippleTimer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0,100,true))
            {
                DoCast(target, SPELL_CRIPPLE);
                CrippleTimer = urand(25000, 30000);
            }
        }
        

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_lesser_doomguard(Creature *_Creature)
{
    return new mob_lesser_doomguardAI (_Creature);
}

void AddSC_boss_azgalor()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_azgalor";
    newscript->GetAI = &GetAI_boss_azgalor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_lesser_doomguard";
    newscript->GetAI = &GetAI_mob_lesser_doomguard;
    newscript->RegisterSelf();
}
