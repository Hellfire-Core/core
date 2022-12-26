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
SDName: Boss_Temporus
SD%Complete: 95
SDComment: Some timers may need to be adjusted
SDCategory: Caverns of Time, The Dark Portal
EndScriptData */

#include "scriptPCH.h"
#include "def_dark_portal.h"

#define SAY_ENTER               -1269000
#define SAY_AGGRO               -1269001
#define SAY_BANISH              -1269002
#define SAY_SLAY1               -1269003
#define SAY_SLAY2               -1269004
#define SAY_DEATH               -1269005

#define SPELL_HASTE             31458
#define SPELL_MORTAL_WOUND      31464
#define SPELL_WING_BUFFET       31475
#define H_SPELL_WING_BUFFET     38593
#define SPELL_REFLECT           38592

struct boss_temporusAI : public ScriptedAI
{
    boss_temporusAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;
    bool canApplyWound;

    Timer MortalWound_Timer;
    Timer WingBuffet_Timer;
    Timer Haste_Timer;
    Timer SpellReflection_Timer;

    void Reset()
    {
        MortalWound_Timer.Reset(5000);
        canApplyWound = false;
        WingBuffet_Timer.Reset(10000);
        Haste_Timer.Reset(20000);
        SpellReflection_Timer.Reset(40000);
        m_creature->setActive(true);

        SayIntro();
    }

    void SayIntro()
    {
        DoScriptText(SAY_ENTER, m_creature);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2), m_creature);
    }

    void JustDied(Unit *victim)
    {
        if (pInstance->GetData(TYPE_MEDIVH) != FAIL)
            DoScriptText(SAY_DEATH, m_creature);

        pInstance->SetData(TYPE_RIFT,SPECIAL);
        pInstance->SetData(TYPE_TEMPORUS,DONE);
    }

    void MoveInLineOfSight(Unit *who)
    {
        //Despawn Time Keeper
        if (who->GetTypeId() == TYPEID_UNIT && who->GetEntry() == C_TIME_KEEPER)
        {
            if (me->IsAlive() && m_creature->IsWithinDistInMap(who,20.0f))
            {
                DoScriptText(SAY_BANISH, m_creature);
                who->ToCreature()->ForcedDespawn();
            }
        }

        ScriptedAI::MoveInLineOfSight(who);
    }

    void DamageMade(Unit* target, uint32 & damage, bool direct_damage, uint8 school_mask)
    {
        if (canApplyWound)
            me->CastSpell(target, SPELL_MORTAL_WOUND, true);

        canApplyWound = false;
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;


        if (Haste_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_HASTE, CAST_SELF);
            Haste_Timer = urand(20000, 25000);
        }
        

        
        if (WingBuffet_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature, HeroicMode ? H_SPELL_WING_BUFFET : SPELL_WING_BUFFET);
            WingBuffet_Timer = urand(15000, 25000);
        }
        

        
        if (MortalWound_Timer.Expired(diff))
        {
            canApplyWound = true;

            if (m_creature->HasAura(SPELL_HASTE, 0))
                MortalWound_Timer = urand(2000, 3000);
            else
                MortalWound_Timer = urand(6000, 9000);
        }
        

     
        if (HeroicMode && SpellReflection_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature, SPELL_REFLECT);
            SpellReflection_Timer = urand(40000, 50000);
        }
        

        //if event failed, remove boss from instance
        if (pInstance->GetData(TYPE_MEDIVH) == FAIL)
        {
            m_creature->Kill(m_creature, false);
            m_creature->RemoveCorpse();
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_temporus(Creature *_Creature)
{
    return new boss_temporusAI (_Creature);
}

void AddSC_boss_temporus()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_temporus";
    newscript->GetAI = &GetAI_boss_temporus;
    newscript->RegisterSelf();
}
