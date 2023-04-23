/* 
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Boss_Vanndar
SD%Complete:
SDComment: Some spells listed on wowwiki but doesn't exist on wowhead
EndScriptData */

#include "scriptPCH.h"

#define YELL_AGGRO              -1001008

#define YELL_EVADE              -1001009
#define YELL_RESPAWN1           -1001010
#define YELL_RESPAWN2           -1001011

#define YELL_RANDOM1            -1001012
#define YELL_RANDOM2            -1001013
#define YELL_RANDOM3            -1001014
#define YELL_RANDOM4            -1001015
#define YELL_RANDOM5            -1001016
#define YELL_RANDOM6            -1001017
#define YELL_RANDOM7            -1001018


#define SPELL_AVATAR            19135
#define SPELL_THUNDERCLAP       15588
#define SPELL_STORMBOLT         20685 // not sure

#define AV_VANDAR_NPC_COUNT     5

uint32 avVandarNpcIds[AV_VANDAR_NPC_COUNT] =
{
    14762,
    14763,
    14764,
    14765,
    11948
};

struct boss_vanndarAI : public ScriptedAI
{
    boss_vanndarAI(Creature *c) : ScriptedAI(c)
    {
        m_creature->GetPosition(wLoc);
    }

    Timer AvatarTimer;
    Timer ThunderclapTimer;
    Timer StormboltTimer;
    Timer YellTimer;
    Timer CheckTimer;
    WorldLocation wLoc;

    void Reset()
    {
        AvatarTimer.Reset(3000);
        ThunderclapTimer.Reset(4000);
        StormboltTimer.Reset(6000);
        YellTimer.Reset(urand(20000, 30000)); //20 to 30 seconds
        CheckTimer.Reset(2000);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(YELL_AGGRO, m_creature);

        // pull rest
        std::for_each(avVandarNpcIds, avVandarNpcIds + AV_VANDAR_NPC_COUNT,
                  [this, who] (uint32 a)->void
                  {
                        if (a == me->GetEntry())
                            return;

                        Creature * c = me->GetMap()->GetCreatureById(a);
                        if (c && c->IsAlive() && c->IsAIEnabled && c->AI())
                            c->AI()->AttackStart(who);
                  });
    }

    void JustRespawned()
    {
        Reset();
        DoScriptText(RAND(YELL_RESPAWN1, YELL_RESPAWN2), m_creature);
    }

    void EnterEvadeMode()
    {
        if (!me->IsInCombat() || me->IsInEvadeMode())
            return;

        CreatureAI::EnterEvadeMode();

        // evade rest
        std::for_each(avVandarNpcIds, avVandarNpcIds + AV_VANDAR_NPC_COUNT,
                  [this] (uint32 a)->void
                  {
                        if (a == me->GetEntry())
                            return;

                        Creature * c = me->GetMap()->GetCreatureById(a);
                        if (c && c->IsAIEnabled && c->AI())
                            c->AI()->EnterEvadeMode();
                  });
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        
        if (CheckTimer.Expired(diff))
        {
            if (!m_creature->IsWithinDistInMap(&wLoc, 20.0f))
                EnterEvadeMode();

            me->SetSpeed(MOVE_WALK, 2.0f, true);
            me->SetSpeed(MOVE_RUN, 2.0f, true);

            CheckTimer = 2000;
        }
        
        
        if (AvatarTimer.Expired(diff))
        {
            ForceSpellCast(m_creature->GetVictim(), SPELL_AVATAR);
            AvatarTimer = urand(15000, 20000);
        }
        
          
            
        if (ThunderclapTimer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_THUNDERCLAP);
            ThunderclapTimer = urand(5000, 15000);
        }
            
        
        if (StormboltTimer.Expired(diff))
        {
            Unit * victim = SelectUnit(SELECT_TARGET_RANDOM, 1, 30.0f, true);
            if (victim)
                AddSpellToCast(victim, SPELL_STORMBOLT);
            StormboltTimer = urand(10000, 25000);
        }
        
        
        if (YellTimer.Expired(diff))
        {
            DoScriptText(RAND(YELL_RANDOM1, YELL_RANDOM2, YELL_RANDOM3, YELL_RANDOM4, YELL_RANDOM5, YELL_RANDOM6, YELL_RANDOM7), m_creature);
            YellTimer = urand(20000, 30000); //20 to 30 seconds
        }
          

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

enum AVVanndarOfficerSpells
{
    AV_VO_CHARGE    = 22911,
    AV_VO_CLEAVE    = 40504,
    AV_VO_DEMOSHOUT = 23511,
    AV_VO_WHIRLWIND = 13736,
    AV_VO_ENRAGE    = 8599
};

struct boss_vanndarOfficerAI : public ScriptedAI
{
    boss_vanndarOfficerAI(Creature *c) : ScriptedAI(c)
    {
        m_creature->GetPosition(wLoc);
    }

    Timer chargeTimer;
    Timer cleaveTimer;
    Timer demoShoutTimer;
    Timer whirlwindTimer;
    Timer CheckTimer;
    WorldLocation wLoc;

    void Reset()
    {
        chargeTimer.Reset(urand(7500, 20000));
        cleaveTimer.Reset(urand(5000, 10000));
        demoShoutTimer.Reset(urand(2000, 4000));
        whirlwindTimer.Reset(urand(9000, 13000));
        CheckTimer.Reset(2000);
    }

    void EnterCombat(Unit *who)
    {
        // pull rest
        std::for_each(avVandarNpcIds, avVandarNpcIds + AV_VANDAR_NPC_COUNT,
                  [this, who] (uint32 a)->void
                  {
                        if (a == me->GetEntry())
                            return;

                        Creature * c = me->GetMap()->GetCreatureById(a);
                        if (c && c->IsAlive() && c->IsAIEnabled && c->AI())
                            c->AI()->AttackStart(who);
                  });
    }

    void JustRespawned()
    {
        Reset();
    }

    void EnterEvadeMode()
    {
        if (!me->IsInCombat() || me->IsInEvadeMode())
            return;

        CreatureAI::EnterEvadeMode();

        // evade rest
        std::for_each(avVandarNpcIds, avVandarNpcIds + AV_VANDAR_NPC_COUNT,
                  [this] (uint32 a)->void
                  {
                        if (a == me->GetEntry())
                            return;

                        Creature * c = me->GetMap()->GetCreatureById(a);
                        if (c && c->IsInCombat() && c->IsAIEnabled && c->AI())
                            c->AI()->EnterEvadeMode();
                  });
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        
        if (CheckTimer.Expired(diff))
        {
            if (!m_creature->IsWithinDistInMap(&wLoc, 20.0f))
                EnterEvadeMode();

            me->SetSpeed(MOVE_WALK, 1.5f, true);
            me->SetSpeed(MOVE_RUN, 1.5f, true);

            CheckTimer = 2000;
        }
        
        if (chargeTimer.Expired(diff))
        {
            Unit * target = SelectUnit(SELECT_TARGET_RANDOM, 0, 25.0f, true, 0, 8.0f);

            if (target)
                AddSpellToCast(target, AV_VO_CHARGE);

            chargeTimer = urand(7500, 20000);
        }
        
            
        if (cleaveTimer.Expired(diff))
        {
            AddSpellToCast(AV_VO_CLEAVE, CAST_TANK);
            cleaveTimer = urand(5000, 10000);
        }
        
           
        if (demoShoutTimer.Expired(diff))
        {
            AddSpellToCast(AV_VO_DEMOSHOUT, CAST_NULL);
            demoShoutTimer = urand(14000, 25000);
        }
        
        
        if (whirlwindTimer.Expired(diff))
        {
            AddSpellToCast(AV_VO_WHIRLWIND, CAST_SELF);
            whirlwindTimer = urand(9000, 13000);
        }
        


        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_vanndar(Creature *_Creature)
{
    return new boss_vanndarAI (_Creature);
}

CreatureAI* GetAI_boss_vanndarOfficer(Creature *_Creature)
{
    return new boss_vanndarOfficerAI (_Creature);
}

void AddSC_boss_vanndar()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_vanndar";
    newscript->GetAI = &GetAI_boss_vanndar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_vanndar_officer";
    newscript->GetAI = &GetAI_boss_vanndarOfficer;
    newscript->RegisterSelf();
}
