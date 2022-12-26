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

enum
{
    SPELL_HEAD_BUTT		    = 11650,
    SPELL_PIERCE_ARMOUR	    = 6016,
    SPELL_ARCANE_EXPLOSION  = 37106,
    SPELL_AVOIDANCE         = 35698,
    SPELL_FEL_FLAMESTRIKE   = 39139,
    SPELL_DEATH_BLAST       = 40736,
    SPELL_BLAZING_SPEED     = 31643,

    NPC_HOGGER          = 66716,
    NPC_GRUFF           = 66717,
    NPC_RIVERPAW_ELITE  = 66718,
    NPC_IMP             = 66719,
};

struct npc_hogger_theAI : public ScriptedAI
{
    npc_hogger_theAI(Creature* c) : ScriptedAI(c), summons(c) {};

    Timer pierceTimer;
    Timer stunTimer;
    Timer spawnTimer;
    Timer felFireTimer;
    Timer checkTimer;
    uint8 pierceCounter;
    uint64 stunTargetGUID;
    uint32 combatTime;
    uint8 phase; // 1,3 - tank and spank, 2 - ads
    uint8 adsWave;
    SummonList summons;

    void Reset()
    {
        combatTime = 0;
        checkTimer.Reset(3000);
        pierceTimer.Reset(3000);
        stunTimer.Reset(5000);
        spawnTimer.Reset(3000);
        felFireTimer.Reset(30000);
        pierceCounter = 0;
        stunTargetGUID = 0;
        phase = 1;
        adsWave = 0;
        summons.DespawnAll();
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
        m_creature->SetVisibility(VISIBILITY_ON);
    }

    void EnterCombat(Unit* who)
    {
        combatTime = 0;
        m_creature->Yell("Heroes must die!!",0,0);
    }

    void SummonAds()
    {
        switch (adsWave)
        {
        case 0:
        case 2:
        case 4:
            SummonMobs(15, NPC_IMP);
            break;
        case 1:
            SummonMobs(1, NPC_GRUFF);
            SummonMobs(5, NPC_RIVERPAW_ELITE);
            break;
        case 3:
            SummonMobs(10, NPC_RIVERPAW_ELITE);
            break;
        case 5:
            phase = 3;
            m_creature->SetHealth(m_creature->GetMaxHealth());
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
            m_creature->SetVisibility(VISIBILITY_ON);
            DoCast(NULL, SPELL_ARCANE_EXPLOSION, true);
            SummonMobs(5, NPC_RIVERPAW_ELITE);
            return;
        }

        adsWave++;
        spawnTimer = 3000;
    }

    void SummonMobs(uint8 count, uint32 entry)
    {
        for (uint8 i = 0; i < count; i++)
        {
            Position pos;
            m_creature->GetHomePosition(pos.m_positionX, pos.m_positionY, pos.m_positionZ, pos.o);
            m_creature->GetValidPointInAngle(pos, frand(1.0f, 20.0f), frand(0.0f, 2 * M_PI), false);
            if (Creature* imp = m_creature->SummonCreature(entry, pos.m_positionX, pos.m_positionY, pos.m_positionZ, pos.o, TEMPSUMMON_CORPSE_DESPAWN, 1000))
            {
                summons.Summon(imp);
                if (Unit* target = imp->SelectNearbyTarget(40.0f))
                    imp->CombatStart(target);
                int32 pct = -30;
                if (entry == NPC_RIVERPAW_ELITE)
                    pct = -95;
                else if (entry == NPC_IMP)
                    pct = -70;
                imp->CastCustomSpell(imp, SPELL_AVOIDANCE, &pct, NULL, NULL, true);
            }
        }
    }

    void SummonedCreatureDespawn(Creature* spawn)
    {
        summons.Despawn(spawn);
    }

    void JustDied(Unit* killer)
    {
        m_creature->Yell("Imbecile mortals...", 0, 0);
        std::ostringstream str;
        str << "was killed after " << combatTime << " ms";
        m_creature->TextEmote(str.str().c_str(), 0);
    }

    void UpdateAI(const uint32 diff)
    {
        combatTime += diff;
        if (!UpdateVictim())
            return;
        
        if (checkTimer.Expired(diff))
        {
            //manual dozoneincombat
            Player* plr = me->GetVictim()->GetCharmerOrOwnerPlayerOrPlayerItself();
            Group* grp = NULL;
            if (plr)
                grp = plr->GetGroup();
            if (grp)
            {
                for (GroupReference *itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
                {
                    plr = itr->getSource();
                    if (!plr || plr->IsGameMaster() || !plr->IsAlive())
                        continue;

                    WorldLocation center = m_creature->GetHomePosition();
                    if (plr->IsWithinDistInMap(&center, 40.0f))
                    {
                        me->SetInCombatWith(plr);
                        plr->SetInCombatWith(me);
                        me->AddThreat(plr, 0.0f);
                    }
                    else
                    {
                        m_creature->Yell("Going somewhere??", 0, 0);
                        m_creature->Kill(plr);
                    }
                }
            }
            checkTimer = 3000;
        }

        Unit* target = m_creature->GetVictim();

        if (phase == 1 || phase == 3)
        {
            if (stunTimer.Expired(diff))
            {
                if (!stunTargetGUID)
                {
                    m_creature->AddAura(SPELL_HEAD_BUTT, target);
                    stunTargetGUID = target->GetGUID();
                    stunTimer = 2900;
                    m_creature->Yell("Bash...!", 0, 0);
                }
                else
                {
                    if (m_creature->getVictimGUID() == stunTargetGUID && target->HasAura(SPELL_HEAD_BUTT))
                    {
                        m_creature->Kill(target);
                        m_creature->Yell("And crush!", 0, 0);
                    }
                    stunTargetGUID = 0;
                    stunTimer = 7100;
                    return; // target could be dead, avoid problems
                }
            }

            if (pierceTimer.Expired(diff))
            {
                DoCast(target, SPELL_PIERCE_ARMOUR);
                pierceTimer = 6000;
            }
            if (!m_creature->IsWithinMeleeRange(m_creature->GetVictim()) && !m_creature->HasAura(SPELL_BLAZING_SPEED))
            {
                DoCast(m_creature, SPELL_BLAZING_SPEED, true);
            }
            DoMeleeAttackIfReady();

            if (phase == 1 && m_creature->HealthBelowPct(10))
            {
                phase = 2;

                m_creature->Yell("Enough of this!", 0, 0);
                DoCast(m_creature->GetVictim(), SPELL_ARCANE_EXPLOSION, true);
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
                m_creature->SetVisibility(VISIBILITY_OFF);
            }
        }

        if (phase == 2)
        {
            if (summons.empty() && spawnTimer.Expired(diff))
                SummonAds();
            if (felFireTimer.Expired(diff))
            {
                Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 50.0f);
                if (target)
                    DoCast(target, SPELL_FEL_FLAMESTRIKE, true);
                if (felFireTimer.GetInterval() > 6000)
                    felFireTimer = felFireTimer.GetInterval() - 2000;
                else
                    felFireTimer = 6000;
            }
        }
    }
};

CreatureAI* GetAI_npc_hogger_the(Creature* c)
{
    return new npc_hogger_theAI(c);
}

struct npc_gruff_ai : public ScriptedAI
{
    npc_gruff_ai(Creature* c) : ScriptedAI(c) {}

    Timer castTimer;
    bool triggered;

    void Reset()
    {
        m_creature->ApplySpellImmune(0, IMMUNITY_DISPEL, DISPEL_POISON, true);
        castTimer.Reset(2000);
        triggered = false;
    }

    void JustDied(Unit* killer)
    {
        m_creature->Yell("Ouch, me died...", 0, 0);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;
        
        if (castTimer.Expired(diff))
        {
            if (m_creature->HasAuraType(SPELL_AURA_HASTE_SPELLS))
            {
                if (!triggered)
                {
                    m_creature->Yell("Demonish? I speak that language very well! That was not a good idea!", 0, 0);
                    triggered = true;
                }
                DoCast(m_creature->GetVictim(), SPELL_DEATH_BLAST, true);
                castTimer = 1000;
            }
            else
            {
                DoCast(m_creature->GetVictim(), SPELL_ARCANE_EXPLOSION);
                castTimer = 6000;
            }
        }

        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_gruff(Creature* c)
{
    return new npc_gruff_ai(c);
}

void AddSC_hogger_the()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_event_hogger_the";
    newscript->GetAI = &GetAI_npc_hogger_the;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_event_gruff_swiftbite";
    newscript->GetAI = &GetAI_npc_gruff;
    newscript->RegisterSelf();
}
