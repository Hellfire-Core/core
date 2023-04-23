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
SDName: Boss_Warp_Splinter
SD%Complete: 80
SDComment: Includes Sapling (need some better control with these).
SDCategory: Tempest Keep, The Botanica
EndScriptData */

#include "scriptPCH.h"

/*#####
# mob_treant (Sapling)
#####*/

#define SPELL_HEAL_FATHER   6262

struct mob_treantAI  : public ScriptedAI
{
    mob_treantAI (Creature *c) : ScriptedAI(c), WarpGuid(0) {}

    uint64 WarpGuid;
    Timer lifeExpired;

    void Reset()
    {
        lifeExpired.Reset(20000);
    }

    void EnterCombat(Unit *who) {}

    void MoveInLineOfSight(Unit*) {}

    void MovementInform(uint32 type, uint32 id)
    {
        if (type == POINT_MOTION_TYPE && id == 1)
            me->GetMotionMaster()->MoveIdle();
    }

    void UpdateAI(const uint32 diff)
    {
        if (WarpGuid && lifeExpired.Expired(diff))
        {
            lifeExpired = 0;

            if (Unit *Warp = me->GetUnit(WarpGuid))
            {
                int32 CurrentHP_Treant = (int32)m_creature->GetHealth();
                Warp->CastCustomSpell(Warp, SPELL_HEAL_FATHER, &CurrentHP_Treant, 0, 0, true, 0, 0, m_creature->GetGUID());
                m_creature->DealDamage(m_creature, m_creature->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                return;
            }
        }

        if (!UpdateVictim())
            return;

        if (m_creature->getVictimGUID() != WarpGuid)
            DoMeleeAttackIfReady();
    }
};

/*#####
# boss_warp_splinter
#####*/

#define SAY_AGGRO           -1553007
#define SAY_SLAY_1          -1553008
#define SAY_SLAY_2          -1553009
#define SAY_SUMMON_1        -1553010
#define SAY_SUMMON_2        -1553011
#define SAY_DEATH           -1553012

#define WAR_STOMP           34716
#define SUMMON_TREANTS      34727                           // DBC: 34727, 34731, 34733, 34734, 34736, 34739, 34741 (with Ancestral Life spell 34742)   // won't work (guardian summon)
#define ARCANE_VOLLEY       (HeroicMode?39133:36705)

#define CREATURE_TREANT     19949

#define TREANT_SPAWN_DIST   50                              //50 yards from Warp Splinter's spawn point

static float treant_pos[6][3] =
{
    {24.301233f, 427.221100f, -27.060635f},
    {16.795492f, 359.678802f, -27.355425f},
    {53.493484f, 345.381470f, -26.196192f},
    {61.867096f, 439.362732f, -25.921030f},
    {109.861877f, 423.201630f, -27.356019f},
    {106.780159f, 355.582581f, -27.593357f}
};

struct boss_warp_splinterAI : public ScriptedAI
{
    boss_warp_splinterAI(Creature *c) : ScriptedAI(c), summons(me)
    {
        Treant_Spawn_Pos_X = c->GetPositionX();
        Treant_Spawn_Pos_Y = c->GetPositionY();
        m_creature->GetPosition(wLoc);
    }

    SummonList summons;
    Timer War_Stomp_Timer;
    Timer Summon_Treants_Timer;
    Timer Arcane_Volley_Timer;
    Timer Check_Timer;
    WorldLocation wLoc;

    float Treant_Spawn_Pos_X;
    float Treant_Spawn_Pos_Y;

    void Reset()
    {
        summons.DespawnAll();
        War_Stomp_Timer.Reset(25000 + rand() % 15000);
        Summon_Treants_Timer.Reset(45000);
        Arcane_Volley_Timer.Reset(8000 + rand() % 12000);

        m_creature->SetSpeed( MOVE_RUN, 0.7f, true);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), m_creature);
    }

    void JustSummoned(Creature* treant)
    {
        summons.Summon(treant);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, m_creature);
        summons.DespawnAll();
    }

    void SummonTreants()
    {
        for(int i = 0; i < 6; ++i)
        {
            float angle = (M_PI / 3) * i;

            float X = Treant_Spawn_Pos_X + TREANT_SPAWN_DIST * cos(angle);
            float Y = Treant_Spawn_Pos_Y + TREANT_SPAWN_DIST * sin(angle);
            float O = - m_creature->GetAngle(X,Y);

            if(Creature *pTreant = m_creature->SummonCreature(CREATURE_TREANT,treant_pos[i][0],treant_pos[i][1],treant_pos[i][2],O,TEMPSUMMON_CORPSE_DESPAWN,25000))
            {
                pTreant->setFaction(me->getFaction());
                pTreant->GetMotionMaster()->MoveFollow(m_creature, 5.0f, frand(0, 2 * M_PI));
                ((mob_treantAI*)pTreant->AI())->WarpGuid = m_creature->GetGUID();
            }
        }

        DoScriptText(RAND(SAY_SUMMON_1, SAY_SUMMON_2), m_creature);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim() )
            return;

        //Check for War Stomp
        if(War_Stomp_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),WAR_STOMP);
            War_Stomp_Timer = 25000 + rand()%15000;
        }

        //Check_Timer
        if(Check_Timer.Expired(diff))
        {
            if(!m_creature->IsWithinDistInMap(&wLoc, 30.0f))
                EnterEvadeMode();
            else
                DoZoneInCombat();

            Check_Timer = 1000;
        }

        //Check for Arcane Volley
        if(Arcane_Volley_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),ARCANE_VOLLEY);
            Arcane_Volley_Timer = 20000 + rand()%15000;
        }

        //Check for Summon Treants
        if(Summon_Treants_Timer.Expired(diff))
        {
            SummonTreants();
            Summon_Treants_Timer = 45000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_warp_splinter(Creature *_Creature)
{
    return new boss_warp_splinterAI (_Creature);
}

CreatureAI* GetAI_mob_treant(Creature *_Creature)
{
    return new mob_treantAI (_Creature);
}

void AddSC_boss_warp_splinter()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_warp_splinter";
    newscript->GetAI = &GetAI_boss_warp_splinter;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_warp_splinter_treant";
    newscript->GetAI = &GetAI_mob_treant;
    newscript->RegisterSelf();
}

