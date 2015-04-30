/* 
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Boss_DrekThar
SD%Complete: 50%
SDComment: Some spells listed on wowwiki but doesn't exist on wowhead
EndScriptData */

#include "precompiled.h"

#define YELL_AGGRO             -2100000

#define YELL_EVADE             -2100001
#define YELL_RESPAWN           -2100002

#define YELL_RANDOM1           -2100003
#define YELL_RANDOM2           -2100004
#define YELL_RANDOM3           -2100005
#define YELL_RANDOM4           -2100006
#define YELL_RANDOM5           -2100007


#define SPELL_WHIRLWIND        15589
#define SPELL_WHIRLWIND2       13736
#define SPELL_KNOCKDOWN        19128
#define SPELL_FRENZY           8269
#define SPELL_SWEEPING_STRIKES 18765 // not sure
#define SPELL_CLEAVE           20677 // not sure
#define SPELL_WINDFURY         35886 // not sure
#define SPELL_STORMPIKE        51876 // not sure

#define AV_DREKTHAR_NPC_COUNT   5

uint32 avDrekTharNpcId[AV_DREKTHAR_NPC_COUNT] =
{
    14772,
    14777,
    14776,
    14773,
    11946
};

struct boss_drektharAI : public ScriptedAI
{
    boss_drektharAI(Creature *c) : ScriptedAI(c)
    {
        m_creature->GetPosition(wLoc);
    }

    int32 WhirlwindTimer;
    int32 Whirlwind2Timer;
    int32 KnockdownTimer;
    int32 FrenzyTimer;
    int32 YellTimer;
    int32 CheckTimer;
    WorldLocation wLoc;

    void Reset()
    {
        WhirlwindTimer          = urand(0, 10000);
        Whirlwind2Timer         = urand(0, 15000);
        KnockdownTimer          = 12000;
        FrenzyTimer             = 6000;
        YellTimer               = urand(20000, 30000); //20 to 30 seconds
        CheckTimer              = 2000;
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(YELL_AGGRO, m_creature);

        // pull rest
        std::for_each(avDrekTharNpcId, avDrekTharNpcId + AV_DREKTHAR_NPC_COUNT,
                  [this, who] (uint32 a)->void
                  {
                        if (a == me->GetEntry())
                            return;

                        Creature * c = me->GetMap()->GetCreatureById(a);
                        if (c && c->isAlive() && c->IsAIEnabled && c->AI())
                            c->AI()->AttackStart(who);
                  });
    }

    void JustRespawned()
    {
        Reset();
        DoScriptText(YELL_RESPAWN, m_creature);
    }

    void EnterEvadeMode()
    {
        if (!me->isInCombat() || me->IsInEvadeMode())
            return;

        CreatureAI::EnterEvadeMode();

        // evade rest
        std::for_each(avDrekTharNpcId, avDrekTharNpcId + AV_DREKTHAR_NPC_COUNT,
                  [this] (uint32 a)->void
                  {
                        if (a == me->GetEntry())
                            return;

                        Creature * c = me->GetMap()->GetCreatureById(a);
                        if (c && c->isInCombat() && c->IsAIEnabled && c->AI())
                            c->AI()->EnterEvadeMode();
                  });
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        CheckTimer -= diff;
        if (CheckTimer < diff)
        {
            if(!m_creature->IsWithinDistInMap(&wLoc, 20.0f))
                EnterEvadeMode();

            me->SetSpeed(MOVE_WALK, 2.0f, true);
            me->SetSpeed(MOVE_RUN, 2.0f, true);

            CheckTimer += 2000;
        }
        

        WhirlwindTimer -= diff;
        if (WhirlwindTimer < diff)
        {
            AddSpellToCast(m_creature->getVictim(), SPELL_WHIRLWIND);
            WhirlwindTimer +=  urand(8000, 18000);
        }
        
        Whirlwind2Timer -= diff;
        if (Whirlwind2Timer < diff)
        {
            AddSpellToCast(m_creature->getVictim(), SPELL_WHIRLWIND2);
            Whirlwind2Timer += urand(7000, 25000);
        }
        
        KnockdownTimer -= diff;
        if (KnockdownTimer < diff)
        {
            AddSpellToCast(m_creature->getVictim(), SPELL_KNOCKDOWN);
            KnockdownTimer += urand(10000, 15000);
        }
        
        FrenzyTimer -= diff;
        if (FrenzyTimer < diff)
        {
            AddSpellToCast(m_creature->getVictim(), SPELL_FRENZY);
            FrenzyTimer += urand(20000, 25000);
        }
        
        YellTimer -= diff;
        if (YellTimer < diff)
        {
            DoScriptText(RAND(YELL_RANDOM1, YELL_RANDOM2, YELL_RANDOM3, YELL_RANDOM4, YELL_RANDOM5), m_creature);
            YellTimer += urand(20000, 30000); //20 to 30 seconds
        }
        

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

enum AVDrekTharOfficerSpells
{
    AV_DT_CHARGE    = 22911,
    AV_DT_CLEAVE    = 40504,
    AV_DT_DEMOSHOUT = 23511,
    AV_DT_WHIRLWIND = 13736,
    AV_DT_ENRAGE    = 8599
};

struct boss_drektharOfficerAI : public ScriptedAI
{
    boss_drektharOfficerAI(Creature *c) : ScriptedAI(c)
    {
        m_creature->GetPosition(wLoc);
    }

    int32 chargeTimer;
    int32 cleaveTimer;
    int32 demoShoutTimer;
    int32 whirlwindTimer;
    int32 CheckTimer;
    WorldLocation wLoc;

    void Reset()
    {
        chargeTimer             = urand(7500, 20000);
        cleaveTimer             = urand(5000, 10000);
        demoShoutTimer          = urand(2000, 4000);
        whirlwindTimer          = urand(9000, 13000);
        CheckTimer              = 2000;
    }

    void EnterCombat(Unit *who)
    {
        // pull rest
        std::for_each(avDrekTharNpcId, avDrekTharNpcId + AV_DREKTHAR_NPC_COUNT,
                  [this, who] (uint32 a)->void
                  {
                        if (a == me->GetEntry())
                            return;

                        Creature * c = me->GetMap()->GetCreatureById(a);
                        if (c && c->isAlive() && c->IsAIEnabled && c->AI())
                            c->AI()->AttackStart(who);
                  });
    }

    void JustRespawned()
    {
        Reset();
    }

    void EnterEvadeMode()
    {
        if (!me->isInCombat() || me->IsInEvadeMode())
            return;

        CreatureAI::EnterEvadeMode();

        // evade rest
        std::for_each(avDrekTharNpcId, avDrekTharNpcId + AV_DREKTHAR_NPC_COUNT,
                  [this] (uint32 a)->void
                  {
                        if (a == me->GetEntry())
                            return;

                        Creature * c = me->GetMap()->GetCreatureById(a);
                        if (c && c->isInCombat() && c->IsAIEnabled && c->AI())
                            c->AI()->EnterEvadeMode();
                  });
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        CheckTimer -= diff;
        if (CheckTimer < diff)
        {
            if (!m_creature->IsWithinDistInMap(&wLoc, 20.0f))
                EnterEvadeMode();

            me->SetSpeed(MOVE_WALK, 1.5f, true);
            me->SetSpeed(MOVE_RUN, 1.5f, true);

            CheckTimer += 2000;
        }
        
        chargeTimer -= diff;
        if (chargeTimer < diff)
        {
            Unit * target = SelectUnit(SELECT_TARGET_RANDOM, 0, 25.0f, true, 0, 8.0f);

            if (target)
                AddSpellToCast(target, AV_DT_CHARGE);

            chargeTimer += urand(7500, 20000);
        }
        
        cleaveTimer -= diff;
        if (cleaveTimer < diff)
        {
            AddSpellToCast(AV_DT_CLEAVE, CAST_TANK);
            cleaveTimer += urand(5000, 10000);
        }
        
        demoShoutTimer -= diff;
        if (demoShoutTimer < diff)
        {
            AddSpellToCast(AV_DT_DEMOSHOUT, CAST_NULL);
            demoShoutTimer += urand(14000, 25000);
        }
        
        whirlwindTimer -= diff;
        if (whirlwindTimer < diff)
        {
            AddSpellToCast(AV_DT_WHIRLWIND, CAST_SELF);
            whirlwindTimer += urand(9000, 13000);
        }
        
           

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_drekthar(Creature *_Creature)
{
    return new boss_drektharAI (_Creature);
}

CreatureAI* GetAI_boss_drektharOfficer(Creature *_Creature)
{
    return new boss_drektharOfficerAI (_Creature);
}

void AddSC_boss_drekthar()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_drekthar";
    newscript->GetAI = &GetAI_boss_drekthar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_drekthar_officer";
    newscript->GetAI = &GetAI_boss_drektharOfficer;
    newscript->RegisterSelf();
}
