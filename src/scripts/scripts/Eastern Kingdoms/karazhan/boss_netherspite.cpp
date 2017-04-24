/*
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
 SDName: Boss_Netherspite
 SD%Complete: 90
 SDComment: Not sure about timing and portals placing
 SDCategory: Karazhan
 EndScriptData */

#include "precompiled.h"
#include "def_karazhan.h"

#define EMOTE_PHASE_PORTAL          -1532089
#define EMOTE_PHASE_BANISH          -1532090

enum
{
    SPELL_NETHERBURN_AURA       = 30522,
    SPELL_VOIDZONE              = 37063,
    SPELL_NETHER_INFUSION       = 38688,
    SPELL_NETHERBREATH          = 38523,
    SPELL_BANISH_VISUAL         = 39833,
    SPELL_BANISH_ROOT           = 42716,
    SPELL_EMPOWERMENT           = 38549,
    SPELL_NETHERSPITE_ROAR      = 38684,
    SPELL_VOID_ZONE_EFFECT      = 46264,

    NETHER_PATROL_PATH          = 15689,
};

const float PortalCoord[3][3] =
{
    {-11195.353516, -1613.237183, 278.237258}, // Left side
    {-11137.846680, -1685.607422, 278.239258}, // Right side
    {-11094.493164, -1591.969238, 279.949188}  // Back side
};

enum Netherspite_Portal
{
    RED_PORTAL,     // Perseverence
    GREEN_PORTAL,   // Serenity
    BLUE_PORTAL     // Dominance
};

const uint32 PortalID[3]        = {17369,17367,17368};
const uint32 PortalVisual[3]    = {30487,30490,30491};
const uint32 PortalBeam[3]      = {30465,30464,30463};
const uint32 PlayerBuff[3]      = {30421,30422,30423};
const uint32 NetherBuff[3]      = {30466,30467,30468};
const uint32 PlayerDebuff[3]    = {38637,38638,38639};

struct boss_netherspiteAI : public ScriptedAI
{
    boss_netherspiteAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    bool PortalPhase;
    bool Berserk;
    Timer PhaseTimer; // timer for phase switching
    Timer VoidZoneTimer;
    Timer NetherInfusionTimer; // berserking timer
    Timer NetherbreathTimer;
    Timer EmpowermentTimer;
    Timer ExhaustCheckTimer;
    Timer PortalTimer; // timer for beam checking
    Timer PortalPhaseIntro;
    uint64 PortalGUID[3]; // guid's of portals
    uint64 BeamTarget[3]; // guid's of portals' current targets
    bool PortalCasting[3];

    void Reset()
    {
        ClearCastQueue();

        Berserk = false;
        NetherInfusionTimer.Reset(540000);
        VoidZoneTimer.Reset(15000);
        NetherbreathTimer.Reset(3000);
        ExhaustCheckTimer.Reset(1000);
        PortalPhaseIntro.Reset(0);
        HandleDoors(true);
        DestroyPortals();

        for(int i=0; i<3; ++i)
        {
            PortalGUID[i] = 0;
            BeamTarget[i] = 0;
            PortalCasting[i] = false;
        }

        m_creature->GetMotionMaster()->MovePath(NETHER_PATROL_PATH, true);

        if(pInstance && pInstance->GetData(DATA_NETHERSPITE_EVENT) != DONE)
            pInstance->SetData(DATA_NETHERSPITE_EVENT, NOT_STARTED);
    }

    void SummonPortals()
    {
        uint8 r = rand()%4;
        uint8 pos[3];
        pos[RED_PORTAL] = (r%2 ? (r>1 ? 2: 1): 0);
        pos[GREEN_PORTAL] = (r%2 ? 0: (r>1 ? 2: 1));
        pos[BLUE_PORTAL] = (r>1 ? 1: 2); // Blue Portal not on the left side (0)

        for(int i=0; i<3; ++i)
        {
            if(Creature *portal = m_creature->SummonCreature(PortalID[i],PortalCoord[pos[i]][0],PortalCoord[pos[i]][1],PortalCoord[pos[i]][2],0,TEMPSUMMON_TIMED_DESPAWN,60000))
            {
                PortalGUID[i] = portal->GetGUID();
                BeamTarget[i] = me->GetGUID();
                portal->CastSpell(portal, PortalVisual[i], true);
                portal->CastSpell(me, PortalBeam[i], false);
                PortalCasting[i] = true;
            }
        }
    }

    void DestroyPortals()
    {
        for(int i=0; i<3; ++i)
        {
            if(Creature *portal = Unit::GetCreature(*m_creature, PortalGUID[i]))
            {
                portal->SetVisibility(VISIBILITY_OFF);
                portal->DealDamage(portal, portal->GetMaxHealth());
                portal->RemoveFromWorld();
            }

            PortalGUID[i] = 0;
            BeamTarget[i] = 0;
        }
    }

    void UpdatePortals() // Here we handle the beams' behavior
    {
        for(int j=0; j<3; ++j) // j = color
        {
            if(Creature *portal = Unit::GetCreature(*m_creature, PortalGUID[j]))
            {
                // the one who's been cast upon before
                Unit *current = Unit::GetUnit(*portal, BeamTarget[j]);
                // temporary store for the best suitable beam reciever
                Unit *target = m_creature;

                if(Map* map = m_creature->GetMap())
                {
                    Map::PlayerList const& players = map->GetPlayers();

                    // get the best suitable target
                    for(Map::PlayerList::const_iterator i = players.begin(); i!=players.end(); ++i)
                    {
                        Player* p = i->getSource();
                        if (p && p->isAlive() && !p->HasAura(PlayerDebuff[j]) // alive
                            && (!target || target->GetExactDistance2d(portal->GetPositionX(), portal->GetPositionY()) > p->GetExactDistance2d(portal->GetPositionX(), portal->GetPositionY())) // closer than current best
                            && !p->HasAura(PlayerDebuff[j], 0) // not exhausted
                            //&& !p->HasAura(PlayerBuff[(j+1)%3],0) // not on another beam
                            //&& !p->HasAura(PlayerBuff[(j+2)%3],0)
                            && BeamTarget[(j + 1) % 3] != p->GetGUID()
                            && BeamTarget[(j + 2) % 3] != p->GetGUID()
                            && p->isBetween(m_creature, portal)) // on the beam
                            target = p;
                    }
                }

                // buff the target
                if (target->GetTypeId() == TYPEID_PLAYER)
                {
                    target->AddAura(PlayerBuff[j], target);
                    target->RemoveAurasDueToSpell(PlayerBuff[(j + 1) % 3]);
                    target->RemoveAurasDueToSpell(PlayerBuff[(j + 2) % 3]);
                }
                else
                    target->AddAura(NetherBuff[j], target);    

                if(target != current)
                {
                    portal->InterruptNonMeleeSpells(true);

                    // debuff added when aura expires by spell_linked_spell
                    //if (current->GetTypeId() == TYPEID_PLAYER)
                    //    current->CastSpell(current, PlayerDebuff[j], true);

                    //portal->CastSpell(target, PortalBeam[j], false);
                    PortalCasting[j] = false; // cast next update
                    BeamTarget[j] = target->GetGUID();
                }

                // aggro target if Red Beam
                if(j == RED_PORTAL && m_creature->getVictim() != target && target->GetTypeId() == TYPEID_PLAYER)
                    m_creature->getThreatManager().addThreat(target, 100000.0f+DoGetThreat(m_creature->getVictim()));
            }
        }
    }

    void SwitchToPortalPhase()
    {
        PortalPhaseIntro.Reset(3000);
        m_creature->RemoveAurasDueToSpell(SPELL_BANISH_ROOT);
        m_creature->RemoveAurasDueToSpell(SPELL_BANISH_VISUAL);
        SummonPortals();
        PhaseTimer.Reset(60000);
        PortalPhase = true;
        PortalTimer.Reset(10000);
        EmpowermentTimer.Reset(13000);
        DoScriptText(EMOTE_PHASE_PORTAL,m_creature);
        AttackStart(m_creature->getVictim());
        DoResetThreat();
    }

    void SwitchToBanishPhase()
    {
        m_creature->RemoveAurasDueToSpell(SPELL_EMPOWERMENT);
        m_creature->RemoveAurasDueToSpell(SPELL_NETHERBURN_AURA);
        DoCast(m_creature,SPELL_BANISH_VISUAL,true);
        DoCast(m_creature,SPELL_BANISH_ROOT,true);
        DestroyPortals();
        PhaseTimer.Reset(30000);
        PortalPhase = false;
        DoScriptText(EMOTE_PHASE_BANISH,m_creature);

        for(int i=0; i<3; ++i)
            m_creature->RemoveAurasDueToSpell(NetherBuff[i]);
    }

    void HandleDoors(bool open) // Massive Door switcher
    {
        if(GameObject *Door = GameObject::GetGameObject((*m_creature),pInstance->GetData64(DATA_GAMEOBJECT_MASSIVE_DOOR)))
            Door->SetUInt32Value(GAMEOBJECT_STATE, open ? 0 : 1);
    }

    void EnterCombat(Unit *who)
    {
        HandleDoors(false);
        SwitchToPortalPhase();

        m_creature->GetMotionMaster()->Clear();
        DoStartMovement(who);

        if (pInstance)
            pInstance->SetData(DATA_NETHERSPITE_EVENT, IN_PROGRESS);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!m_creature->isInCombat() && m_creature->IsWithinDistInMap(who, 25.0) && m_creature->IsHostileTo(who))
            AttackStart(who);
    }

    void JustDied(Unit* killer)
    {
        HandleDoors(true);
        DestroyPortals();
        if (pInstance)
            pInstance->SetData(DATA_NETHERSPITE_EVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        DoSpecialThings(diff, DO_EVERYTHING, 125.0f, 1.5f);

        if (VoidZoneTimer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM,1,GetSpellMaxRange(SPELL_VOIDZONE),true, m_creature->getVictimGUID()))
                AddSpellToCast(target,SPELL_VOIDZONE,true);

            VoidZoneTimer = 15000;
        }

        if (!Berserk && NetherInfusionTimer.Expired(diff))
        {
            m_creature->AddAura(SPELL_NETHER_INFUSION, m_creature);
            ForceSpellCast(m_creature, SPELL_NETHERSPITE_ROAR, INTERRUPT_AND_CAST_INSTANTLY);
            Berserk = true;
        }

        if(PortalPhase) // PORTAL PHASE
        {
            for (uint8 j = 0; j < 3; j++)
            {
                if (!PortalCasting[j]) //first tick after target change
                {
                    Creature* portal = Unit::GetCreature(*m_creature, PortalGUID[j]);
                    Unit* target = m_creature->GetUnit(BeamTarget[j]);
                    if (target && portal)
                        portal->CastSpell(target, PortalBeam[j], false);
                    PortalCasting[j] = true;
                }
            }

            if (PortalTimer.Expired(diff))
            {
                UpdatePortals();
                PortalTimer = 1000;
            }

            if (EmpowermentTimer.Expired(diff))
            {
                ForceSpellCast(m_creature, SPELL_EMPOWERMENT);
                m_creature->AddAura(SPELL_NETHERBURN_AURA, m_creature);
                EmpowermentTimer = 90000;
            }

            if (PhaseTimer.Expired(diff))
            {
                if(!m_creature->IsNonMeleeSpellCast(false))
                {
                    SwitchToBanishPhase();
                    return;
                }
            }

            // check target
            if (BeamTarget[0] && BeamTarget[0] != me->getVictimGUID() && BeamTarget[0] != m_creature->GetGUID() && m_creature->GetUnit(BeamTarget[0]))
                AttackStart(m_creature->GetUnit(BeamTarget[0]));

            if (PortalPhaseIntro.Expired(diff))
                PortalPhaseIntro = 0;
            if (!PortalPhaseIntro.GetInterval())
                DoMeleeAttackIfReady();
        }
        else // BANISH PHASE
        {
            if(m_creature->GetMotionMaster()->GetCurrentMovementGeneratorType() != IDLE_MOTION_TYPE)
            {
                m_creature->GetMotionMaster()->Clear();
                m_creature->GetMotionMaster()->MoveIdle();
            }

            if (NetherbreathTimer.Expired(diff))
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 200, true))
                    AddSpellToCast(target, SPELL_NETHERBREATH, false, true);

                NetherbreathTimer = 5000+rand()%2000;
            }

            if (PhaseTimer.Expired(diff))
            {
                if(!m_creature->IsNonMeleeSpellCast(false))
                {
                    SwitchToPortalPhase();
                    return;
                }
            }
        }

        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_boss_netherspite(Creature *_Creature)
{
    return new boss_netherspiteAI(_Creature);
}

/**************
* Void Zone - id 16697
***************/

struct mob_void_zoneAI : public Scripted_NoMovementAI
{
    mob_void_zoneAI(Creature* c) : Scripted_NoMovementAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;
    Timer checkTimer;
    Timer dieTimer;

    void Reset()
    {
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        checkTimer.Reset(500);
        dieTimer.Reset(25000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (checkTimer.Expired(diff))
        {
            if (pInstance && pInstance->GetData(DATA_NETHERSPITE_EVENT) == DONE)
            {
                m_creature->Kill(m_creature, false);
                m_creature->RemoveCorpse();
            }

            const int32 dmg = frand(1000, 1500);    //workaround here, no proper spell known
            m_creature->CastCustomSpell(NULL, SPELL_VOID_ZONE_EFFECT, &dmg, NULL, NULL, false);
            checkTimer = 2000;
        }

        if (dieTimer.Expired(diff))
        {
            m_creature->Kill(m_creature, false);
            m_creature->RemoveCorpse();
            dieTimer = 25000;
        }
    }
};

CreatureAI* GetAI_mob_void_zone(Creature *_Creature)
{
    return new mob_void_zoneAI(_Creature);
}

void AddSC_boss_netherspite()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_netherspite";
    newscript->GetAI = GetAI_boss_netherspite;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_void_zone";
    newscript->GetAI = GetAI_mob_void_zone;
    newscript->RegisterSelf();
}
