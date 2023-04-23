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
SDName: Boss_Skeram
SD%Complete: 75
SDComment: Mind Control buggy.
SDCategory: Temple of Ahn'Qiraj
EndScriptData */

#include "scriptPCH.h"
#include "def_temple_of_ahnqiraj.h"
#include "Group.h"

enum Skeram
{
    SAY_AGGRO1                  = -1531000,
    SAY_AGGRO2                  = -1531001,
    SAY_AGGRO3                  = -1531002,
    SAY_SLAY1                   = -1531003,
    SAY_SLAY2                   = -1531004,
    SAY_SLAY3                   = -1531005,
    SAY_SPLIT                   = -1531006,
    SAY_DEATH                   = -1531007,

    SPELL_ARCANE_EXPLOSION      = 26192,
    SPELL_EARTH_SHOCK           = 26194,
    SPELL_TRUE_FULFILLMENT      = 785,
    SPELL_BLINK                 = 28391,
};

static const std::vector<Position> teleportPos =
{
    {-8340.782227f, 2083.814453f, 125.648788f, 0},
    {-8341.546875f, 2118.504639f, 133.058151f, 0},
    {-8318.822266f, 2058.231201f, 133.058151f, 0},
};

struct boss_skeramAI : public ScriptedAI
{
    boss_skeramAI(Creature *c) : ScriptedAI(c), summons(m_creature)
    {
        pInstance = c->GetInstanceData();
        IsImage = m_creature->IsTemporarySummon();
    }

    ScriptedInstance *pInstance;
    SummonList summons;

    Timer ArcaneExplosion_Timer;
    Timer EarthShock_Timer;
    Timer Fulfillment_Timer;
    Timer Blink_Timer;
    Timer Invisible_Timer;

    bool IsImage;
    uint8 HpCheck;

    void Reset()
    {
        summons.DespawnAll();

        ArcaneExplosion_Timer.Reset(urand(6000, 12000));
        EarthShock_Timer.Reset(5000);
        Fulfillment_Timer.Reset(15000);
        Blink_Timer.Reset(urand(30000, 45000));
        Invisible_Timer.Reset(500);

        HpCheck = 75;

        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetVisibility(VISIBILITY_ON);

        if (!IsImage)
            pInstance->SetData(DATA_THE_PROPHET_SKERAM, NOT_STARTED);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2, SAY_SLAY3), m_creature);
    }

    void JustDied(Unit* Killer)
    {
        if (IsImage)
            return;

        DoScriptText(SAY_DEATH, m_creature);

        if (pInstance)
            pInstance->SetData(DATA_THE_PROPHET_SKERAM, DONE);
    }

    void EnterCombat(Unit *who)
    {
        if (IsImage)
            return;

        DoScriptText(RAND(SAY_AGGRO1, SAY_AGGRO2, SAY_AGGRO3), m_creature);

        if (pInstance)
            pInstance->SetData(DATA_THE_PROPHET_SKERAM, IN_PROGRESS);
    }

    void DoSplit()
    {
        DoScriptText(SAY_SPLIT, m_creature);
        ClearGroupTargetIcons();

        std::vector<Position> pos(teleportPos);
        std::random_shuffle(pos.begin(), pos.end());

        m_creature->RemoveAllAuras();
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetVisibility(VISIBILITY_OFF);
        m_creature->NearTeleportTo(pos[0].m_positionX, pos[0].m_positionY, pos[0].m_positionZ, pos[0].o);

        DoResetThreat();
        DoStopAttack();
        Invisible_Timer = 500;
        Blink_Timer.Reset(urand(10000, 30000));

        for (uint8 id = 1; id <= 2; ++id)
        {
            if (Creature* Image = m_creature->SummonCreature(15263, pos[id].m_positionX, pos[id].m_positionY, pos[id].m_positionZ, pos[id].o, TEMPSUMMON_CORPSE_DESPAWN, 30000))
            {
                summons.Summon(Image);
                Image->SetMaxHealth(m_creature->GetMaxHealth() / 5);
                Image->SetHealth(m_creature->GetHealth() / 5);
                Image->AI()->AttackStart(SelectUnit(SELECT_TARGET_RANDOM));
            }
            else
                return;
        }
    }

    void ClearGroupTargetIcons()
    {
        if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM))
            if (target->ToPlayer())
                if (Group* group = target->ToPlayer()->GetGroup())
                    for (uint8 icon = 0; icon < TARGET_ICON_COUNT; ++icon)
                        group->SetTargetIcon(icon, 0);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (ArcaneExplosion_Timer.Expired(diff))
        {
            if (FindAllPlayersInRange(NOMINAL_MELEE_RANGE).size() >= 5)
                DoCast(m_creature->GetVictim(), SPELL_ARCANE_EXPLOSION);
            ArcaneExplosion_Timer = urand(8000, 18000);
        }

        if (!IsImage && Fulfillment_Timer.Expired(diff))
        {
            DoCast(SelectUnit(SELECT_TARGET_RANDOM, 0, 300.0f, true, me->getVictimGUID()), SPELL_TRUE_FULFILLMENT);
            Fulfillment_Timer = urand(20000, 30000);
        }

        if (Blink_Timer.Expired(diff))
        {
            const Position& pos = teleportPos[rand() % 3];
            m_creature->NearTeleportTo(pos.m_positionX, pos.m_positionY, pos.m_positionZ, pos.o);

            if (m_creature->GetVisibility() != VISIBILITY_ON)
                m_creature->SetVisibility(VISIBILITY_ON);

            DoResetThreat();
            Blink_Timer = urand(10000, 30000);
        }

        if (!IsImage && HealthBelowPct(HpCheck))
        {
            DoSplit();
            HpCheck -= 25;
        }

        if (Invisible_Timer.Expired(diff))
        {
            //Making Skeram visible after telporting
            m_creature->SetVisibility(VISIBILITY_ON);
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            Invisible_Timer = 0;
        }

        //If we are within range melee the target
        if (m_creature->IsWithinMeleeRange(m_creature->GetVictim()))
            DoMeleeAttackIfReady();
        else if (EarthShock_Timer.Expired(diff))
        {
            DoCast(SelectUnit(SELECT_TARGET_TOPAGGRO), SPELL_EARTH_SHOCK);
            EarthShock_Timer = 1000;
        }
    }
};

CreatureAI* GetAI_boss_skeram(Creature *_Creature)
{
    return new boss_skeramAI(_Creature);
}

void AddSC_boss_skeram()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_skeram";
    newscript->GetAI = &GetAI_boss_skeram;
    newscript->RegisterSelf();
}

