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
SDName: Boss_Prince_Malchezzar
SD%Complete: 100
SDComment:
SDCategory: Karazhan
EndScriptData */

#include "scriptPCH.h"
#include "def_karazhan.h"

#define SAY_AGGRO           -1532091
#define SAY_AXE_TOSS1       -1532092
#define SAY_AXE_TOSS2       -1532093
#define SAY_SPECIAL1        -1532094
#define SAY_SPECIAL2        -1532095
#define SAY_SPECIAL3        -1532096
#define SAY_SLAY1           -1532097
#define SAY_SLAY2           -1532098
#define SAY_SLAY3           -1532099
#define SAY_SUMMON1         -1532100
#define SAY_SUMMON2         -1532101
#define SAY_DEATH           -1532102

// 19 Coordinates for Infernal spawns
struct InfernalPoint
{
    float x,y;
};

#define INFERNAL_Z  275.5f

static InfernalPoint InfernalPoints[] =
{
    {-10922.8f, -1985.2f},
    {-10916.2f, -1996.2f},
    {-10932.2f, -2008.1f},
    {-10948.8f, -2022.1f},
    {-10958.7f, -1997.7f},
    {-10971.5f, -1997.5f},
    {-10990.8f, -1995.1f},
    {-10989.8f, -1976.5f},
    {-10971.6f, -1973.0f},
    {-10955.5f, -1974.0f},
    {-10939.6f, -1969.8f},
    {-10958.0f, -1952.2f},
    {-10941.7f, -1954.8f},
    {-10943.1f, -1988.5f},
    {-10948.8f, -2005.1f},
    {-10984.0f, -2019.3f},
    {-10932.8f, -1979.6f},
    {-10932.8f, -1979.6f},
    {-10935.7f, -1996.0f}
};

#define TOTAL_INFERNAL_POINTS 19

//Enfeeble is supposed to reduce hp to 1 and then heal player back to full when it ends
//Along with reducing healing and regen while enfeebled to 0%
//This spell effect will only reduce healing

#define SPELL_ENFEEBLE          30843                       //Enfeeble during phase 1 and 2
#define SPELL_ENFEEBLE_EFFECT   41624

#define SPELL_SHADOWNOVA        30852                       //Shadownova used during all phases
#define SPELL_SW_PAIN           30854                       //Shadow word pain during phase 1 and 3 (different targeting rules though)
#define SPELL_THRASH_PASSIVE    12787                       //Extra attack chance during phase 2
#define SPELL_SUNDER_ARMOR      30901                       //Sunder armor during phase 2
#define SPELL_THRASH_AURA       3417                        //Passive proc chance for thrash
#define SPELL_EQUIP_AXES        30857                       //Visual for axe equiping
#define SPELL_AMPLIFY_DAMAGE    39095                       //Amplifiy during phase 3
#define SPELL_HELLFIRE          30859                       //Infenals' hellfire aura
#define SPELL_CLEAVE            30131                       //Same as Nightbane.
#define NETHERSPITE_INFERNAL    17646                       //The netherspite infernal creature
#define MALCHEZARS_AXE          17650                       //Malchezar's axes (creatures), summoned during phase 3

#define INFERNAL_MODEL_INVISIBLE 11686                      //Infernal Effects
#define SPELL_INFERNAL_RELAY     30834
#define NPC_RELAY                17645

#define AXE_EQUIP_MODEL          40066                      //Axes info
#define AXE_EQUIP_INFO           33448898

//---------Infernal code first
struct netherspite_infernalAI : public Scripted_NoMovementAI
{
    netherspite_infernalAI(Creature *c) : Scripted_NoMovementAI(c) ,
        malchezaarGUID(0), HellfireTimer(0)
    {
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CASTING_SPEED, true);
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_HASTE_SPELLS, true);
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_STUN, true);
    }

    Timer HellfireTimer;
    uint64 malchezaarGUID;

    void Reset() {}
    void EnterCombat(Unit*) {}
    void MoveInLineOfSight(Unit*) {}
    void AttackStart(Unit*) {}

    void UpdateAI(const uint32 diff)
    {
        if (HellfireTimer.Expired(diff))
        {
            DoCast(m_creature, SPELL_HELLFIRE);
            HellfireTimer = 0;
        }
    }

    void KilledUnit(Unit *who)
    {
        if (Creature *malchezaar = Unit::GetCreature(*m_creature, malchezaarGUID))
            malchezaar->AI()->KilledUnit(who);
    }

    void SpellHit(Unit *who, const SpellEntry *spell)
    {
        if (spell->Id == SPELL_INFERNAL_RELAY)
        {
            m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, m_creature->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            HellfireTimer = 4000;
        }
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (done_by->GetGUID() != malchezaarGUID)
            damage = 0;
    }
};

struct boss_malchezaarAI : public ScriptedAI
{
    boss_malchezaarAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        m_creature->GetPosition(wLoc);
    }

    ScriptedInstance *pInstance;
    Timer EnfeebleTimer;
    Timer EnfeebleResetTimer;
    Timer ShadowNovaTimer;
    Timer SWPainTimer;
    Timer SunderArmorTimer;
    Timer AmplifyDamageTimer;
    Timer InfernalTimer;
    Timer AxesTargetSwitchTimer;
    Timer InfernalCleanupTimer;
    Timer CheckTimer;
    Timer Cleave_Timer;

    WorldLocation wLoc;

    std::vector<uint64> infernals;
    std::vector<InfernalPoint> positions;

    uint64 axes[2];
    uint64 enfeeble_targets[5];
    uint64 enfeeble_health[5];

    uint32 phase;

    bool Enabled;

    void Reset()
    {
        m_creature->SetCanDualWield(false);
        AxesCleanup();
        ClearWeapons();
        InfernalCleanup();
        for(int i =0; i < 5; ++i)
            enfeeble_targets[i] = 0;

        
        EnfeebleTimer.Reset(30000);
        EnfeebleResetTimer = 0;
        ShadowNovaTimer = 0;
        SWPainTimer.Reset(20000);
        SunderArmorTimer = 0;
        AmplifyDamageTimer.Reset(5000);
        Cleave_Timer.Reset(8000);
        InfernalTimer.Reset(45000);
        InfernalCleanupTimer.Reset(47000);
        AxesTargetSwitchTimer.Reset(7500 + rand() % 12500);
        CheckTimer.Reset(3000);
        phase = 1;

        if(pInstance)
        {
            if (GameObject* Door = GameObject::GetGameObject((*m_creature), pInstance->GetData64(DATA_GAMEOBJECT_NETHER_DOOR)))
                Door->SetGoState(GO_STATE_ACTIVE);

            if (pInstance->GetData(DATA_MALCHEZZAR_EVENT) != DONE)
                pInstance->SetData(DATA_MALCHEZZAR_EVENT, NOT_STARTED);
        }

        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        Enabled = false;
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2, SAY_SLAY3), m_creature);
    }

    void JustDied(Unit *victim)
    {
        DoScriptText(SAY_DEATH, m_creature);

        AxesCleanup();
        ClearWeapons();
        InfernalCleanup();

        if (pInstance)
        {
            if (GameObject* Door = GameObject::GetGameObject((*m_creature),pInstance->GetData64(DATA_GAMEOBJECT_NETHER_DOOR)))
                Door->SetGoState(GO_STATE_ACTIVE);

            pInstance->SetData(DATA_MALCHEZZAR_EVENT, DONE);
        }
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);

        if (pInstance)
        {
            if (GameObject* Door = GameObject::GetGameObject((*m_creature),pInstance->GetData64(DATA_GAMEOBJECT_NETHER_DOOR)))
                Door->SetGoState(GO_STATE_READY);

            pInstance->SetData(DATA_MALCHEZZAR_EVENT, IN_PROGRESS);
        }

        positions.clear();

        for (int i = 0; i < TOTAL_INFERNAL_POINTS; ++i)
            positions.push_back(InfernalPoints[i]);
    }

    void InfernalCleanup()
    {
        //Infernal Cleanup
        for(std::vector<uint64>::iterator itr = infernals.begin(); itr != infernals.end(); ++itr)
        {
            Unit *pInfernal = Unit::GetUnit(*m_creature, *itr);
            if (pInfernal && pInfernal->IsAlive())
            {
                pInfernal->SetVisibility(VISIBILITY_OFF);
                pInfernal->setDeathState(JUST_DIED);
            }
        }
        infernals.clear();
    }

    void AxesCleanup()
    {
        for (int i = 0; i < 2; ++i)
        {
            if (Unit *axe = Unit::GetUnit(*m_creature, axes[i]))
                if (axe->IsAlive())
                    axe->Kill(axe);

            axes[i] = 0;
        }
    }

    void ClearWeapons()
    {
        m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, 0);
        m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO, 0);

        m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY+1, 0);
        m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO+2, 0);
    }

    void EnfeebleHealthEffect()
    {
        const SpellEntry *info = GetSpellStore()->LookupEntry(SPELL_ENFEEBLE_EFFECT);
        if (!info)
            return;

        std::list<HostileReference *> t_list = m_creature->getThreatManager().getThreatList();
        std::vector<Unit *> targets;

        if (t_list.size() < 2)
            return;

        //begin + 1 , so we don't target the one with the highest threat
        std::list<HostileReference *>::iterator itr = t_list.begin();
        for (; itr != t_list.end(); ++itr)                   //store the threat list in a different container
        {
            if ((*itr)->getUnitGuid() == m_creature->getVictimGUID())
                continue;
            Unit *target = Unit::GetUnit(*m_creature, (*itr)->getUnitGuid());
                                                            //only on alive players
            if (target && target->IsAlive() && target->GetTypeId() == TYPEID_PLAYER)
                targets.push_back(target);
        }

        //cut down to size if we have more than 5 targets
        while (targets.size() > 5)
            targets.erase(targets.begin()+rand()%targets.size());

        int i = 0;
        for (std::vector<Unit *>::iterator itr = targets.begin(); itr != targets.end(); ++itr, ++i)
        {
            if (Unit *target = *itr)
            {
                enfeeble_targets[i] = target->GetGUID();
                enfeeble_health[i] = target->GetHealth();

                target->CastSpell(target, SPELL_ENFEEBLE, true);
                target->SetHealth(1);
            }
        }
    }

    void EnfeebleResetHealth()
    {
        for (int i = 0; i < 5; ++i)
        {
            Unit *target = Unit::GetUnit(*m_creature, enfeeble_targets[i]);
            if (target && target->IsAlive())
                target->SetHealth(enfeeble_health[i]);

            enfeeble_targets[i] = 0;
            enfeeble_health[i] = 0;
        }
    }

    void SummonedCreatureDespawn(Creature* who)
    {
        if (who->GetEntry() == NETHERSPITE_INFERNAL)
        {
            InfernalPoint ip;
            ip.x = who->GetPositionX();
            ip.y = who->GetPositionY();
            positions.push_back(ip);
        }
    }

    void SummonInfernal(const uint32 diff)
    {
        InfernalPoint point;
        float posX, posY, posZ;

        if ((m_creature->GetMapId() != 532) || positions.empty())
        {
            m_creature->GetRandomPoint(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 60, posX, posY, posZ);
        }
        else
        {
            std::vector<InfernalPoint>::iterator itr = positions.begin()+rand()%positions.size();
            point = *itr;
            positions.erase(itr);
            posX = point.x;
            posY = point.y;
            posZ = INFERNAL_Z;
        }

        Creature *Infernal = m_creature->SummonCreature(NETHERSPITE_INFERNAL, posX, posY, posZ, 0, TEMPSUMMON_TIMED_DESPAWN, 240000);

        if (Infernal)
        {
            Infernal->SetUInt32Value(UNIT_FIELD_DISPLAYID, INFERNAL_MODEL_INVISIBLE);
            Infernal->setFaction(m_creature->getFaction());
            ((netherspite_infernalAI*)Infernal->AI())->malchezaarGUID=m_creature->GetGUID();

            infernals.push_back(Infernal->GetGUID());
            Creature* relay = m_creature->GetMap()->GetCreatureById(NPC_RELAY);
            if (relay)
                relay->CastSpell(Infernal, SPELL_INFERNAL_RELAY, false);
            else
                DoCast(Infernal, SPELL_INFERNAL_RELAY);
        }

        DoScriptText(RAND(SAY_SUMMON1, SAY_SUMMON2), m_creature);
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (!done_by->IsWithinDistInMap(&wLoc, 95.0f))
            damage = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!Enabled && pInstance->GetData(DATA_OPERA_EVENT) == DONE)
        {
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            Enabled = true;
        }

        if (!UpdateVictim())
            return;

        if (CheckTimer.Expired(diff))
        {
            if (!m_creature->IsWithinDistInMap(&wLoc, 95.0f))
                DoResetThreat();
            else
                DoZoneInCombat();

            CheckTimer = 3000;
        }

        if (EnfeebleResetTimer.Expired(diff))                  //Let's not forget to reset that
        {
            EnfeebleResetHealth();
            EnfeebleResetTimer = 0;
        }

        if (m_creature->HasUnitState(UNIT_STAT_STUNNED))     //While shifting to phase 2 Malchezaar stuns himself
            return;

        if (m_creature->GetSelection() != m_creature->getVictimGUID())
            m_creature->SetSelection(m_creature->getVictimGUID());

        if (phase == 1)
        {
            if (HealthBelowPct(60))
            {
                m_creature->InterruptNonMeleeSpells(false);

                phase = 2;

                //animation
                DoCast(m_creature, SPELL_EQUIP_AXES);

                //text
                DoScriptText(SAY_AXE_TOSS1, m_creature);

                //passive thrash aura
                m_creature->CastSpell(m_creature, SPELL_THRASH_AURA, true);

                //models
                m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, AXE_EQUIP_MODEL);
                m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO, AXE_EQUIP_INFO);

                m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY+1, AXE_EQUIP_MODEL);
                m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO+2, AXE_EQUIP_INFO);

                m_creature->SetAttackTime(OFF_ATTACK, (m_creature->GetAttackTime(BASE_ATTACK)*150)/100);

                SunderArmorTimer.Reset(15000);
                m_creature->SetCanDualWield(true);
            }
        }
        else if (phase == 2)
        {
            if (HealthBelowPct(30))
            {
                InfernalTimer = 15000;

                phase = 3;

                ClearWeapons();

                //remove thrash
                m_creature->RemoveAurasDueToSpell(SPELL_THRASH_AURA);

                DoScriptText(SAY_AXE_TOSS2, m_creature);

                Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0);
                for (uint32 i = 0; i < 2; ++i)
                {
                    Creature *axe = m_creature->SummonCreature(MALCHEZARS_AXE, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0, TEMPSUMMON_DEAD_DESPAWN, 1000);
                    if (axe)
                    {
                        axe->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, AXE_EQUIP_MODEL);
                        axe->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO, AXE_EQUIP_INFO);

                        axe->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        axe->setFaction(m_creature->getFaction());
                        axes[i] = axe->GetGUID();

                        if (target)
                        {
                            axe->AI()->AttackStart(target);
                            // axe->getThreatManager().tauntApply(target); //Taunt Apply and fade out does not work properly
                                                            // So we'll use a hack to add a lot of threat to our target
                            axe->AddThreat(target, 10000000.0f);
                        }
                    }
                }

                if (!ShadowNovaTimer.GetInterval())
                    ShadowNovaTimer.Reset(EnfeebleTimer.GetTimeLeft() + 5000);

                return;
            }

            if (SunderArmorTimer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(), SPELL_SUNDER_ARMOR);
                SunderArmorTimer = urand(10000,15000);
            }

            if (Cleave_Timer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(), SPELL_CLEAVE);
                Cleave_Timer = 6000 + rand() % 6000;
            }
        }
        else // phase 3
        {
            if (AxesTargetSwitchTimer.Expired(diff))
            {
                AxesTargetSwitchTimer = 7500 + rand() % 12500;

                if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                {
                    for (int i = 0; i < 2; ++i)
                    {
                        if (Unit *axe = Unit::GetUnit(*m_creature, axes[i]))
                        {
                            float threat = 1000000.0f;

                            if (axe->GetVictim() && DoGetThreat(axe->GetVictim()))
                            {
                                threat = axe->getThreatManager().getThreat(axe->GetVictim());
                                axe->getThreatManager().modifyThreatPercent(axe->GetVictim(), -100);
                            }

                            if (target)
                                axe->AddThreat(target, threat);
                            //axe->getThreatManager().tauntFadeOut(axe->GetVictim());
                            //axe->getThreatManager().tauntApply(target);
                        }
                    }
                }
            }

            if (AmplifyDamageTimer.Expired(diff))
            {
                if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_AMPLIFY_DAMAGE), true))
                    DoCast(target, SPELL_AMPLIFY_DAMAGE);

                AmplifyDamageTimer = 20000 + rand()%10000;
            }
        }

        //Time for global and double timers
        if (InfernalTimer.Expired(diff))
        {
            SummonInfernal(diff);
            InfernalTimer = phase == 3 ? 15000 : 45000;    //15 secs in phase 3, 45 otherwise
        }

        if (phase != 2)
        {
            
            if (SWPainTimer.Expired(diff))
            {
                Unit* target = nullptr;
                if (phase == 1)
                    target = m_creature->GetVictim();       // the tank
                else                                        //anyone but the tank
                    target = SelectUnit(SELECT_TARGET_RANDOM, 1, GetSpellMaxRange(SPELL_SW_PAIN), true, m_creature->getVictimGUID());

                if (target)
                    DoCast(target, SPELL_SW_PAIN);

                SWPainTimer = 20000;
            }
        }

        if (ShadowNovaTimer.Expired(diff))
        {
            DoCast(m_creature, SPELL_SHADOWNOVA);
            ShadowNovaTimer = (phase == 3) ? 30000 : 0;
        }

        if (phase != 3)
        {
            if (EnfeebleTimer.Expired(diff))
            {
                EnfeebleHealthEffect();
                ShadowNovaTimer.Reset(5000);
                EnfeebleResetTimer.Reset(9000);
                EnfeebleTimer = 30000;
            }
        }

        if (phase == 2)
            DoMeleeAttacksIfReady();
        else
            DoMeleeAttackIfReady();
    }

    void DoMeleeAttacksIfReady()
    {
        if (m_creature->IsWithinMeleeRange(m_creature->GetVictim()) && !m_creature->IsNonMeleeSpellCast(false))
        {
            //Check for base attack
            if (m_creature->isAttackReady() && m_creature->GetVictim())
            {
                m_creature->AttackerStateUpdate(m_creature->GetVictim());
                m_creature->resetAttackTimer();
            }
            //Check for offhand attack
            if (m_creature->isAttackReady(OFF_ATTACK) && m_creature->GetVictim())
            {
                m_creature->AttackerStateUpdate(m_creature->GetVictim(), OFF_ATTACK);
                m_creature->resetAttackTimer(OFF_ATTACK);
            }
        }
    }


};

CreatureAI* GetAI_netherspite_infernal(Creature *_Creature)
{
    return new netherspite_infernalAI (_Creature);
}

CreatureAI* GetAI_boss_malchezaar(Creature *_Creature)
{
    return new boss_malchezaarAI (_Creature);
}

void AddSC_boss_malchezaar()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_malchezaar";
    newscript->GetAI = &GetAI_boss_malchezaar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="netherspite_infernal";
    newscript->GetAI = &GetAI_netherspite_infernal;
    newscript->RegisterSelf();
}

