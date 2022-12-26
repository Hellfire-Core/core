/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * Copyright (C) 2009-2017 MaNGOSOne <https://github.com/mangos/one>
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
SDName: Boss_Omar_The_Unscarred
SD%Complete: 98
SDComment:
SDCategory: Hellfire Citadel, Hellfire Ramparts
EndScriptData */

#include "scriptPCH.h"
#include "hellfire_ramparts.h"

#define SAY_AGGRO_1                 -1543009
#define SAY_AGGRO_2                 -1543010
#define SAY_AGGRO_3                 -1543011
#define SAY_SUMMON                  -1543012
#define SAY_CURSE                   -1543013
#define SAY_KILL_1                  -1543014
#define SAY_DIE                     -1543015
#define SAY_WIPE                    -1543016

#define SPELL_ORBITAL_STRIKE        30637
#define SPELL_SHADOW_WHIP           30638
#define SPELL_BANE_OF_AURA_TREACHERY (HeroicMode ? 37566 : 30695)
#define SPELL_DEMONIC_SHIELD        31901
#define SPELL_SHADOW_BOLT           (HeroicMode ? 39297 : 30686)
#define SPELL_SUMMON_FIENDISH_HOUND 30707

struct boss_omor_the_unscarredAI : public ScriptedAI
{
    boss_omor_the_unscarredAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    Timer OrbitalStrike_Timer;
    Timer ShadowWhip_Timer;
    Timer Aura_Timer;
    Timer DemonicShield_Timer;
    Timer Shadowbolt_Timer;
    Timer Summon_Timer;
    uint64 playerGUID;
    bool CanPullBack;

    void Reset()
    {
        DoScriptText(SAY_WIPE, me);

        OrbitalStrike_Timer.Reset(22000);
        ShadowWhip_Timer.Reset(2000);
        Aura_Timer.Reset(18000);
        DemonicShield_Timer.Reset(1000);
        Shadowbolt_Timer = 1;
        Summon_Timer.Reset(20000);
        playerGUID = 0;
        CanPullBack = false;

        if (pInstance)
            pInstance->SetData(DATA_OMOR, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(RAND(SAY_AGGRO_1, SAY_AGGRO_2, SAY_AGGRO_3), me);

        if (pInstance)
            pInstance->SetData(DATA_OMOR, IN_PROGRESS);
    }

    void KilledUnit(Unit* victim)
    {
        if (rand()%2)
            return;

        DoScriptText(SAY_KILL_1, me);
    }

    void JustSummoned(Creature* summoned)
    {
        DoScriptText(SAY_SUMMON, me);

        if (Unit* random = SelectUnit(SELECT_TARGET_RANDOM,0))
            summoned->AI()->AttackStart(random);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DIE, me);

        if (pInstance)
            pInstance->SetData(DATA_OMOR, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Summon_Timer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_SUMMON_FIENDISH_HOUND);
            Summon_Timer = 18000;
        }

        if (CanPullBack)
        {
            if (ShadowWhip_Timer.Expired(diff))
            {
                if (Unit* temp = Unit::GetUnit(*me,playerGUID))
                {
                    if (temp->HasUnitMovementFlag(MOVEFLAG_FALLINGFAR))
                    {
                        me->InterruptNonMeleeSpells(false);
                        DoCast(temp, SPELL_SHADOW_WHIP);
                    }
                    else 
                    {
                        if (!temp->HasUnitMovementFlag(MOVEFLAG_FALLINGFAR))
                        {
                            playerGUID = 0;
                            CanPullBack = false;
                        }
                    }
                }
                ShadowWhip_Timer = 2200;
            }
        }
        else if (OrbitalStrike_Timer.Expired(diff))
        {
            Unit *temp = SelectUnit(SELECT_TARGET_NEAREST, 0, 100, true);

            if (temp && temp->GetTypeId() == TYPEID_PLAYER && me->IsWithinMeleeRange(temp))
            {
                me->InterruptNonMeleeSpells(false);
                DoCast(temp, SPELL_ORBITAL_STRIKE);
                OrbitalStrike_Timer = 22000;
                playerGUID = temp->GetGUID();

                if (playerGUID)
                {
                    CanPullBack = true;
                    ShadowWhip_Timer = 3000;
                }
            }
        }

        if ((me->GetHealth()*100) / me->GetMaxHealth() < 20)
        {
            if (DemonicShield_Timer.Expired(diff))
            {
                AddSpellToCast(me, SPELL_DEMONIC_SHIELD);
                DemonicShield_Timer = 15000;
            }
        }

        if (Aura_Timer.Expired(diff))
        {
            DoScriptText(SAY_CURSE, me);

            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
            {
                AddSpellToCast(target, SPELL_BANE_OF_AURA_TREACHERY);
                Aura_Timer = 18000;
            }
        }

        if (Shadowbolt_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
            {
                if(!me->IsWithinMeleeRange(target))
                {
                    AddSpellToCast(target, SPELL_SHADOW_BOLT);
                    Shadowbolt_Timer = 3000;
                }
            }
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_omor_the_unscarredAI(Creature *_Creature)
{
    return new boss_omor_the_unscarredAI (_Creature);
}

void AddSC_boss_omor_the_unscarred()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_omor_the_unscarred";
    newscript->GetAI = &GetAI_boss_omor_the_unscarredAI;
    newscript->RegisterSelf();
}

