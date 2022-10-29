/*
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: boss_Akilzon
SD%Complete: 95%
SDComment: Boss fight completed. TODO: Akil'zon Gauntlet event
EndScriptData */

#include "precompiled.h"
#include "def_zulaman.h"
#include "Weather.h"

enum Akilzon
{
    SAY_EVENT1                  = -1568024,
    SAY_EVENT2                  = -1568025,
    SAY_AGGRO                   = -1568026,
    SAY_SUMMON                  = -1568027,
    SAY_SUMMON_ALT              = -1568028,
    SAY_ENRAGE                  = -1568029,
    SAY_SLAY1                   = -1568030,
    SAY_SLAY2                   = -1568031,
    SAY_DEATH                   = -1568032,
    EMOTE_STORM                 = -1568033,

    SPELL_STATIC_DISRUPTION     = 43622,
    SPELL_CALL_LIGHTNING        = 43661,
    SPELL_GUST_OF_WIND          = 43621,
    SPELL_ELECTRICAL_STORM      = 43648,
    SPELL_BERSERK               = 45078,
    SPELL_EAGLE_SWOOP           = 44732,

    NPC_SOARING_EAGLE           = 24858
};

//SAY_EVENT1 (-1568024) and SAY_EVENT2 (-1568025) to be used later

struct boss_akilzonAI : public ScriptedAI
{
    boss_akilzonAI(Creature *c) : ScriptedAI(c), summons(me)
    {
        pInstance = c->GetInstanceData();
        m_creature->GetPosition(wLoc);
    }
    ScriptedInstance *pInstance;

    std::list<Creature*> BirdsList;
    SummonList summons;

    Timer StaticDisruption_Timer;
    Timer GustOfWind_Timer;
    Timer CallLighting_Timer;
    Timer ElectricalStorm_Timer;
    Timer SummonEagles_Timer;
    Timer Enrage_Timer;

    bool isRaining;

    Timer checkTimer;
    WorldLocation wLoc;

    void Reset()
    {
        ClearCastQueue();

        if(pInstance && pInstance->GetData(DATA_AKILZONEVENT) != DONE)
            pInstance->SetData(DATA_AKILZONEVENT, NOT_STARTED);

        StaticDisruption_Timer.Reset(urand(5000, 10000));
        GustOfWind_Timer.Reset(urand(8000, 15000));
        CallLighting_Timer.Reset(urand(8000, 12000));
        ElectricalStorm_Timer.Reset(60000);
        Enrage_Timer.Reset(480000); //8 minutes to enrage
        SummonEagles_Timer.Reset(99999);

        summons.DespawnAll();
        BirdsList.clear();

        isRaining = false;

        SetWeather(WEATHER_STATE_FINE, 0.0f);

        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_INTERRUPT_CAST, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_FEAR, true);
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_SILENCE, true);
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_STUN, true);

        checkTimer = 3000;
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);
        DoZoneInCombat();
        if(pInstance)
            pInstance->SetData(DATA_AKILZONEVENT, IN_PROGRESS);
    }

    void JustDied(Unit* Killer)
    {
        summons.DespawnAll();
        DoScriptText(SAY_DEATH, m_creature);

        if(pInstance)
            pInstance->SetData(DATA_AKILZONEVENT, DONE);
    }

    void JustSummoned(Creature* eagle)
    {
        summons.Summon(eagle);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(urand(0, 1) ? SAY_SLAY1 : SAY_SLAY2, m_creature);
    }

    void DoSummonEagles()
    {
        BirdsList = FindAllCreaturesWithEntry(NPC_SOARING_EAGLE, 200.0);
        uint32 count;
        if(BirdsList.empty())
            count = urand(6, 12);
        else
            count = 12 - BirdsList.size();

        for(uint32 i = 0; i < count; ++i)
        {
            float x, y, z;
            m_creature->GetRandomPoint(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ()+15.0f, 30.0f, x, y, z);

            Creature* Eagle = m_creature->SummonCreature(NPC_SOARING_EAGLE, x, y, z, m_creature->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1000);
            if(Eagle)
            {
                Eagle->setFaction(me->getFaction());
                Eagle->SetLevitate(true);
                Eagle->GetMotionMaster()->MoveIdle();
            }
        }
    }

    void SetWeather(uint32 weather, float grade)
    {
        Map *map = m_creature->GetMap();
        if (!map->IsDungeon()) return;

        WorldPacket data(SMSG_WEATHER, (4+4+4));
        data << uint32(weather) << (float)grade << uint8(0);

        map->BroadcastPacket(me, &data);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (checkTimer.Expired(diff))
        {
            if (!m_creature->IsWithinDistInMap(&wLoc, 110.0f))
                EnterEvadeMode();
            else
            {
                m_creature->SetSpeed(MOVE_RUN, 2.0);
                DoZoneInCombat();
            }
            checkTimer = 1000;
        }




        if (Enrage_Timer.Expired(diff))
        {
            DoScriptText(SAY_ENRAGE, m_creature);
            m_creature->CastSpell(m_creature, SPELL_BERSERK, true);
            Enrage_Timer = 600000;
        }




        if (StaticDisruption_Timer.Expired(diff))
        {
            if(ElectricalStorm_Timer.GetTimeLeft() < 3000)
                StaticDisruption_Timer = 6000;
            else
            {
                Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_STATIC_DISRUPTION), true, m_creature->getVictimGUID());
                if(!target)
                    target = m_creature->GetVictim();
                AddSpellToCast(target, SPELL_STATIC_DISRUPTION, false, true);
                StaticDisruption_Timer = urand(7000, 14000);
            }
        }


        if (GustOfWind_Timer.Expired(diff))
        {
            //we dont want to start a storm with player in the air
            if(ElectricalStorm_Timer.GetTimeLeft() < 8000)
                GustOfWind_Timer = 18000;
            else
            {
                if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_GUST_OF_WIND), true, m_creature->getVictimGUID()))
                    AddSpellToCast(target, SPELL_GUST_OF_WIND);
                GustOfWind_Timer = urand(8000, 14000);
            }
        }



        if (CallLighting_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_CALL_LIGHTNING);
            CallLighting_Timer = RAND(urand(10000, 15000), urand(30000, 45000));
        }


        if (!isRaining && ElectricalStorm_Timer.GetTimeLeft() < urand(8000, 12000))
        {
            SetWeather(WEATHER_STATE_HEAVY_RAIN, 0.9999f);
            isRaining = true;
        }

        if (isRaining && ElectricalStorm_Timer.GetTimeLeft() > 50000)
        {
            SetWeather(WEATHER_STATE_FINE, 0.0f);
            SummonEagles_Timer.Reset(13000);
            isRaining = false;
        }


        if (ElectricalStorm_Timer.Expired(diff))
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_ELECTRICAL_STORM), true);

            if(!target)
            {
                EnterEvadeMode();
                return;
            }
            // throw player to air and cast electrical storm on (should be handled by proper script effect targeting?)
            DoScriptText(EMOTE_STORM, m_creature, 0, true);
            // temporary test, normally used in spell_linked_spell, should be cast before players flying
            target->CastSpell(target, 44007, true);
            m_creature->CastSpell(target, SPELL_ELECTRICAL_STORM, false);

            ElectricalStorm_Timer = 60000;
            StaticDisruption_Timer.Reset(10000);
        }



        if (SummonEagles_Timer.Expired(diff))
        {
            DoScriptText(urand(0,1) ? SAY_SUMMON : SAY_SUMMON_ALT, m_creature);
            DoSummonEagles();
            SummonEagles_Timer = 999999;
        }


        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

struct mob_soaring_eagleAI : public ScriptedAI
{
    mob_soaring_eagleAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
        Reset();
    }

    ScriptedInstance* pInstance;

    Timer EagleSwoop_Timer;
    Timer Return_Timer;
    bool canMoveRandom;
    bool canCast;

    void Reset()
    {
        DoZoneInCombat();
        EagleSwoop_Timer.Reset(urand(2000, 6000));
        me->SetWalk(false);
        me->SetLevitate(true);
        me->SetSpeed(MOVE_FLIGHT, 1.5);
        Return_Timer.Reset(200);
        canMoveRandom = true;
        canCast = true;
    }

    void MovementInform(uint32 uiType, uint32 uiPointId)
    {
        if (uiType != POINT_MOTION_TYPE)
            return;
        me->GetMotionMaster()->MoveIdle();
        canCast = true;
    }

    void DoMoveToRandom()
    {
        if (!pInstance)
            return;

        if (Creature* Akil = me->GetMap()->GetCreature(pInstance->GetData64(DATA_AKILZONEVENT)))
        {
            float x, y, z;
            Akil->GetPosition(x, y, z);
            Akil->GetRandomPoint(x, y, z, 40.0f, x, y, z);
            m_creature->GetMotionMaster()->MovePoint(1, x, y, z+frand(12, 16));
            canMoveRandom = false;
        }
    }

    void AttackStart(Unit *pWho)
    {
        m_creature->Attack(pWho, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (canMoveRandom)
            if (Return_Timer.Expired(diff))
            {
                DoMoveToRandom();
                Return_Timer = 800;
            }

        if (!canCast)
            return;


        if (EagleSwoop_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true))
            {
                DoCast(target, SPELL_EAGLE_SWOOP);

                canMoveRandom = true;
                canCast = false;
            }
            EagleSwoop_Timer = urand(4000, 6000);
        }
    }
};

//Soaring Eagle
CreatureAI* GetAI_mob_soaring_eagle(Creature *_Creature)
{
    return new mob_soaring_eagleAI(_Creature);
}

CreatureAI* GetAI_boss_akilzon(Creature *_Creature)
{
    return new boss_akilzonAI(_Creature);
}

void AddSC_boss_akilzon()
{
    Script *newscript = NULL;

    newscript = new Script;
    newscript->Name="boss_akilzon";
    newscript->GetAI = &GetAI_boss_akilzon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_akilzon_eagle";
    newscript->GetAI = &GetAI_mob_soaring_eagle;
    newscript->RegisterSelf();
}

