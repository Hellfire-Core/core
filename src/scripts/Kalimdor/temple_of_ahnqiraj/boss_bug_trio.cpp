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
SDName: boss_kri, boss_yauj, boss_vem : The Bug Trio
SD%Complete: 100
SDComment:
SDCategory: Temple of Ahn'Qiraj
EndScriptData */

#include "scriptPCH.h"
#include "def_temple_of_ahnqiraj.h"

#define SPELL_CLEAVE        26350
#define SPELL_TOXIC_VOLLEY  25812
#define SPELL_POISON_CLOUD  38718                           //Only Spell with right dmg.
#define SPELL_ENRAGE        34624                           //Changed cause 25790 is cast on gamers too. Same prob with old explosion of twin emperors.

#define SPELL_CHARGE        26561
#define SPELL_KNOCKBACK     26027

#define SPELL_HEAL      25807
#define SPELL_FEAR      19408

struct boss_kriAI : public ScriptedAI
{
    boss_kriAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    Timer Cleave_Timer;
    Timer ToxicVolley_Timer;
    Timer Check_Timer;

    bool VemDead;
    bool Death;

    void Reset()
    {
        Cleave_Timer.Reset(4000 + rand() % 4000);
        ToxicVolley_Timer.Reset(6000 + rand() % 6000);
        Check_Timer.Reset(2000);

        VemDead = false;
        Death = false;

        if (pInstance)
            pInstance->SetData(DATA_BUG_TRIO, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(DATA_BUG_TRIO, IN_PROGRESS);
    }

    void JustDied(Unit* killer)
    {
        if(pInstance)
        {
            if(pInstance->GetData(DATA_BUG_TRIO_DEATH) < 2)
                                                            // Unlootable if death
                m_creature->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

            pInstance->SetData(DATA_BUG_TRIO, DONE);
        }
    }
    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        //Cleave_Timer
        if (Cleave_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CLEAVE);
            Cleave_Timer = 5000 + rand()%7000;
        }

        //ToxicVolley_Timer
        if (ToxicVolley_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_TOXIC_VOLLEY);
            ToxicVolley_Timer = 10000 + rand()%5000;
        }

        if (m_creature->GetHealth() <= m_creature->GetMaxHealth() * 0.05 && !Death)
        {
            DoCast(m_creature->GetVictim(),SPELL_POISON_CLOUD);
            Death = true;
        }

        if(!VemDead)
        {
            //Checking if Vem is dead. If yes we will enrage.
            if (Check_Timer.Expired(diff))
            {
                if(pInstance && pInstance->GetData(DATA_VEM))
                {
                    DoCast(m_creature, SPELL_ENRAGE);
                    VemDead = true;
                }
                Check_Timer = 1000;
            }
        }

        DoMeleeAttackIfReady();
    }
};

struct boss_vemAI : public ScriptedAI
{
    boss_vemAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    Timer Charge_Timer;
    Timer KnockBack_Timer;
    Timer Enrage_Timer;

    bool Enraged;

    void Reset()
    {
        Charge_Timer.Reset(15000 + rand() % 12000);
        KnockBack_Timer.Reset(8000 + rand() % 12000);
        Enrage_Timer.Reset(120000);

        Enraged = false;

        if (pInstance)
            pInstance->SetData(DATA_BUG_TRIO, NOT_STARTED);
    }

    void JustDied(Unit* Killer)
    {
        if(pInstance)
        {
            pInstance->SetData(DATA_VEM, 1);
            if(pInstance->GetData(DATA_BUG_TRIO_DEATH) < 2)
                                                            // Unlootable if death
                m_creature->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

            pInstance->SetData(DATA_BUG_TRIO, DONE);
        }
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(DATA_BUG_TRIO, IN_PROGRESS);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        //Charge_Timer
        if (Charge_Timer.Expired(diff))
        {
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0);
            if(target)
            {
                DoCast(target, SPELL_CHARGE);
                //m_creature->SendMonsterMove(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, true,1);
                AttackStart(target);
            }

            Charge_Timer = 8000 + rand()%8000;
        }

        //KnockBack_Timer
        if (KnockBack_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_KNOCKBACK);
            if(DoGetThreat(m_creature->GetVictim()))
                DoModifyThreatPercent(m_creature->GetVictim(),-80);
            KnockBack_Timer = 15000 + rand()%10000;
        }

        //Enrage_Timer
        if (!Enraged && Enrage_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_ENRAGE);
            Enraged = true;
        }

        DoMeleeAttackIfReady();
    }
};

struct boss_yaujAI : public ScriptedAI
{
    boss_yaujAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    Timer Heal_Timer;
    Timer Fear_Timer;
    Timer Check_Timer;

    bool VemDead;

    void Reset()
    {
        Heal_Timer.Reset(25000 + rand() % 15000);
        Fear_Timer.Reset(12000 + rand() % 12000);
        Check_Timer.Reset(2000);

        VemDead = false;

        if (pInstance)
            pInstance->SetData(DATA_BUG_TRIO, NOT_STARTED);
    }

    void JustDied(Unit* Killer)
    {
        if(pInstance)
        {
            if(pInstance->GetData(DATA_BUG_TRIO_DEATH) < 2)
                                                            // Unlootable if death
                m_creature->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

            pInstance->SetData(DATA_BUG_TRIO, DONE);
        }

        for(int i = 0; i < 10;i++)
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0);
            Creature* Summoned = m_creature->SummonCreature(15621,m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(),0,TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN,90000);
            if(Summoned && target)
                ((CreatureAI*)Summoned->AI())->AttackStart(target);
        }
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(DATA_BUG_TRIO, IN_PROGRESS);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        //Fear_Timer
        if (Fear_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FEAR);
            DoResetThreat();
            Fear_Timer = 20000;
        }

        //Casting Heal to other twins or herself.
        if (Heal_Timer.Expired(diff))
        {
            if(pInstance)
            {
                Unit *pKri = Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_KRI));
                Unit *pVem = Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_VEM));

                switch(rand()%3)
                {
                    case 0:
                        if(pKri)
                            DoCast(pKri, SPELL_HEAL);
                        break;
                    case 1:
                        if(pVem)
                            DoCast(pVem, SPELL_HEAL);
                        break;
                    case 2:
                        DoCast(m_creature, SPELL_HEAL);
                        break;
                }
            }

            Heal_Timer = 15000+rand()%15000;
        }

        //Checking if Vem is dead. If yes we will enrage.
        if (Check_Timer.Expired(diff))
        {
            if (!VemDead)
            {
                if(pInstance)
                {
                    if(pInstance->GetData(DATA_VEM))
                    {
                        DoCast(m_creature, SPELL_ENRAGE);
                        VemDead = true;
                    }
                }
            }
            Check_Timer = 1000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_yauj(Creature *_Creature)
{
    return new boss_yaujAI (_Creature);
}

CreatureAI* GetAI_boss_vem(Creature *_Creature)
{
    return new boss_vemAI (_Creature);
}

CreatureAI* GetAI_boss_kri(Creature *_Creature)
{
    return new boss_kriAI (_Creature);
}

void AddSC_bug_trio()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_kri";
    newscript->GetAI = &GetAI_boss_kri;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_vem";
    newscript->GetAI = &GetAI_boss_vem;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_yauj";
    newscript->GetAI = &GetAI_boss_yauj;
    newscript->RegisterSelf();
}

