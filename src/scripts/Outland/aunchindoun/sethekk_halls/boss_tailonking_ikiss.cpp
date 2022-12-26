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
SDName: Boss_Talon_King_Ikiss
SD%Complete: 80
SDComment: Heroic supported. Some details missing, but most are spell related.
SDCategory: Auchindoun, Sethekk Halls
EndScriptData */

#include "scriptPCH.h"
#include "def_sethekk_halls.h"

#define SAY_INTRO                   -1556007

#define SAY_AGGRO_1                 -1556008
#define SAY_AGGRO_2                 -1556009
#define SAY_AGGRO_3                 -1556010

#define SAY_SLAY_1                  -1556011
#define SAY_SLAY_2                  -1556012
#define SAY_DEATH                   -1556013
#define EMOTE_ARCANE_EXP            -1556015

#define SPELL_BLINK                 38194
#define SPELL_BLINK_TELEPORT        38203
#define SPELL_MANA_SHIELD           38151
#define SPELL_ARCANE_BUBBLE         9438
#define H_SPELL_SLOW                35032

#define SPELL_POLYMORPH             38245
#define H_SPELL_POLYMORPH           43309

#define SPELL_ARCANE_VOLLEY         35059
#define H_SPELL_ARCANE_VOLLEY       40424

#define SPELL_ARCANE_EXPLOSION      38197
#define H_SPELL_ARCANE_EXPLOSION    40425

struct boss_talon_king_ikissAI : public ScriptedAI
{
    boss_talon_king_ikissAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        m_creature->GetPosition(wLoc);
    }

    ScriptedInstance* pInstance;

    Timer ArcaneVolley_Timer;
    Timer Sheep_Timer;
    Timer Slow_Timer;

    WorldLocation wLoc;

    bool ManaShield;
    uint8 Blink;
    bool Intro;

    void Reset()
    {
        ArcaneVolley_Timer.Reset(5000);
        Sheep_Timer.Reset(8000);
        Slow_Timer.Reset(15000 + rand() % 15000);
        Blink = 0;
        Intro = false;
        ManaShield = false;

        if(pInstance)
            pInstance->SetData(DATA_IKISSEVENT, NOT_STARTED);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if( !m_creature->GetVictim() && who->isTargetableForAttack() && ( m_creature->IsHostileTo( who )) && who->isInAccessiblePlacefor(m_creature) )
        {
            if(!Intro && m_creature->IsWithinDistInMap(who, 100))
            {
                Intro = true;
                DoScriptText(SAY_INTRO, m_creature);
            }

            if (!m_creature->CanFly() && m_creature->GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
                return;

            float attackRadius = m_creature->GetAttackDistance(who);
            if( m_creature->IsWithinDistInMap(who, attackRadius) && m_creature->IsWithinLOSInMap(who) )
            {
                //who->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
                AttackStart(who);
            }
        }
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(RAND(SAY_AGGRO_1, SAY_AGGRO_2, SAY_AGGRO_3), m_creature);

        if (pInstance)
        {
            pInstance->SetData(DATA_IKISSEVENT, IN_PROGRESS);
            if (pInstance->GetData(DATA_DARKWEAVEREVENT) != DONE)
            {
                Player* moron = who->GetCharmerOrOwnerPlayerOrPlayerItself();
                if (moron)
                    me->Kill(moron);

                EnterEvadeMode();
            }
        }
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (pInstance)
            pInstance->SetData(DATA_IKISSEVENT, DONE);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), m_creature);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Blink & 0x1)
        {
            DoCast(m_creature,HeroicMode ? H_SPELL_ARCANE_EXPLOSION : SPELL_ARCANE_EXPLOSION);
            m_creature->CastSpell(m_creature,SPELL_ARCANE_BUBBLE,true);
            Blink++;
        }

        if (ArcaneVolley_Timer.Expired(diff))
        {
            DoCast(m_creature,HeroicMode ? H_SPELL_ARCANE_VOLLEY : SPELL_ARCANE_VOLLEY);
            ArcaneVolley_Timer = 10000+rand()%5000;
        }
        

        if (Sheep_Timer.Expired(diff))
        {
            Unit *target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0, 60, true, m_creature->getVictimGUID());
            //surely not tank (2.2.0 patchnotes), but not sure if it should be random or second aggro
            if (target)
                DoCast(target,HeroicMode ? H_SPELL_POLYMORPH : SPELL_POLYMORPH); //don't see any difference between them
            Sheep_Timer = 15000+rand()%2500;
        }


        //may not be correct time to cast
        if (!ManaShield && ((m_creature->GetHealth()*100) / m_creature->GetMaxHealth() < 20))
        {
            DoCast(m_creature,SPELL_MANA_SHIELD);
            ManaShield = true;
        }

        if (HeroicMode)
        {
            if (Slow_Timer.Expired(diff))
            {
                DoCast(m_creature, H_SPELL_SLOW);
                Slow_Timer = 15000 + rand() % 25000;
            }
        }

        if ((m_creature->HealthBelowPct(80) && Blink == 0) || (m_creature->HealthBelowPct(50) && Blink == 2) || (m_creature->HealthBelowPct(20) && Blink == 4))
        {
            DoScriptText(EMOTE_ARCANE_EXP, m_creature);

            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0, 60, true))
            {
                if (m_creature->IsNonMeleeSpellCast(false))
                    m_creature->InterruptNonMeleeSpells(false);

                //Spell doesn't work, but we use for visual effect at least
                DoCast(target,SPELL_BLINK);

                DoTeleportTo(target->GetPositionX(),target->GetPositionY(),target->GetPositionZ());

                DoCast(target,SPELL_BLINK_TELEPORT);
                Blink++;
            }
        }
        

        if (!(Blink & 0x1))
            DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_talon_king_ikiss(Creature *_Creature)
{
    return new boss_talon_king_ikissAI (_Creature);
}

void AddSC_boss_talon_king_ikiss()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_talon_king_ikiss";
    newscript->GetAI = &GetAI_boss_talon_king_ikiss;
    newscript->RegisterSelf();
}

