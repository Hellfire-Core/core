/*
 * Copyright (C) 2009 TrinityCore <http://www.trinitycore.org/>
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
SDName: Boss_Kiljaeden
SD%Complete: 70
SDComment: Phase4, Phase5, Shadow Spike, Armageddon
SDCategory: Sunwell_Plateau
EndScriptData */

//TODO rewrite Amagedon
//TODO Remove blue visual from Orbs on reset and if it is used

#include "scriptPCH.h"
#include "boss_kiljaeden.h"
#include "def_sunwell_plateau.h"
#include <math.h>

class AllOrbsInGrid
{
public:
    AllOrbsInGrid() {}
    bool operator() (GameObject* go)
    {
        if (go->GetEntry() == GAMEOBJECT_ORB_OF_THE_BLUE_DRAGONFLIGHT)
            return true;
        return false;
    }
};

bool GOUse_go_orb_of_the_blue_flight(Player *plr, GameObject* go)
{
    if (go->GetUInt32Value(GAMEOBJECT_FACTION) == 35)
    {
        ScriptedInstance* pInstance = (go->GetInstanceData());
        float x, y, z, dx, dy, dz;
        Unit* dragon = go->SummonCreature(CREATURE_POWER_OF_THE_BLUE_DRAGONFLIGHT, plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 121000);
        if (dragon)
        {
            plr->CastSpell(dragon, SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT, true);
            plr->CastSpell(plr, SPELL_POSSESS_DRAKE_IMMUNE, true);
        }
        go->SetUInt32Value(GAMEOBJECT_FACTION, 0);
        Unit* Kalec = pInstance->instance->GetCreatureById(CREATURE_KALECGOS);

        if (!Kalec)
            return true;

        go->GetPosition(x, y, z);
        for (uint8 i = 0; i < 4; ++i)
        {
            DynamicObject* Dyn = Kalec->GetDynObject(SPELL_RING_OF_BLUE_FLAMES);
            if (Dyn)
            {
                Dyn->GetPosition(dx, dy, dz);
                if (x == dx && dy == y && dz == z)
                {
                    Dyn->RemoveFromWorld();
                    break;
                }
            }
        }
        go->Refresh();
    }
    return true;
}

//AI for Kalecgos
struct boss_kalecgos_kjAI : public ScriptedAI
{
    boss_kalecgos_kjAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;
    Timer FelmystOutroTimer;

    GameObject* Orb[4];
    uint8 OrbsEmpowered;
    uint8 EmpowerCount;

    bool Searched;


    void Reset()
    {
        if (pInstance->GetData(DATA_EREDAR_TWINS_EVENT) == DONE)
            me->SetVisibility(VISIBILITY_OFF);

        for (uint8 i = 0; i < 4; ++i)
            Orb[i] = NULL;
        FindOrbs();

        FelmystOutroTimer = 0;
        OrbsEmpowered = 0;
        EmpowerCount = 0;
        m_creature->SetLevitate(true);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->setActive(true);
        Searched = false;
    }


    void FindOrbs()
    {
     // std::list<GameObject*> orbList;
        std::list<GameObject*> orbList2;
        AllOrbsInGrid check;
        MaNGOS::AllGameObjectsInRange objects(me, 100.0f);
    //  MaNGOS::ObjectListSearcher<GameObject, MaNGOS::AllGameObjectsInRange> searcher(orbList, objects);
        MaNGOS::ObjectListSearcher<GameObject, AllOrbsInGrid> searcher(orbList2, check);
        Cell::VisitGridObjects(me, searcher, me->GetMap()->GetVisibilityDistance()); 

        if (orbList2.empty())
             return;
        uint8 i = 0;
        for (std::list<GameObject*>::iterator itr = orbList2.begin(); itr != orbList2.end(); ++itr, ++i)
        {
            Orb[i] = GameObject::GetGameObject(*me, (*itr)->GetGUID());

        }
    }

    void ResetOrbs()
    {
        me->RemoveDynObject(SPELL_RING_OF_BLUE_FLAMES);
        for (uint8 i = 0; i < 4; ++i)
            if (Orb[i])
                Orb[i]->SetUInt32Value(GAMEOBJECT_FACTION, 0);
    }

    void EmpowerOrb(bool all)
    {
        if (!Orb[OrbsEmpowered])
            return;
        
        if (all)
        {
            me->RemoveDynObject(SPELL_RING_OF_BLUE_FLAMES);
            for (uint8 i = 0; i < 4; ++i)
            {
                if (!Orb[i]) return;
                Orb[i]->CastSpell(me, SPELL_RING_OF_BLUE_FLAMES);
                Orb[i]->SetUInt32Value(GAMEOBJECT_FACTION, 35);
                Orb[i]->setActive(true);
                Orb[i]->Refresh();
            }
        }
        else
        {
            uint8 random = urand(0, 3);
            float x, y, z, dx, dy, dz;
            Orb[random]->GetPosition(x, y, z);
            for (uint8 i = 0; i < 4; ++i)
            {
                DynamicObject* Dyn = me->GetDynObject(SPELL_RING_OF_BLUE_FLAMES);
                if (Dyn)
                {
                    Dyn->GetPosition(dx, dy, dz);
                    if (x == dx && dy == y && dz == z)
                    {
                        Dyn->RemoveFromWorld();
                        break;
                    }
                }
            }
            Orb[random]->CastSpell(me, SPELL_RING_OF_BLUE_FLAMES);
            Orb[random]->SetUInt32Value(GAMEOBJECT_FACTION, 35);
            Orb[random]->setActive(true);
            Orb[random]->Refresh();
            ++OrbsEmpowered;
        }
        ++EmpowerCount;

        switch (EmpowerCount)
        {
            case 1: DoScriptText(SAY_KALEC_ORB_READY1, m_creature); break;
            case 2: DoScriptText(SAY_KALEC_ORB_READY2, m_creature); break;
            case 3: DoScriptText(SAY_KALEC_ORB_READY3, m_creature); break;
            case 4: DoScriptText(SAY_KALEC_ORB_READY4, m_creature); break;
        }
    }

    void MovementInform(uint32 Type, uint32 Id)
    {
        if (Type == POINT_MOTION_TYPE)
        {
            switch (Id)
            {
                case 50: // felmyst outro speach
                    DoScriptText(YELL_KALECGOS, me);
                    FelmystOutroTimer.Reset(10000);
                    break;
                case 60: // on starting phase 2
                    me->DisappearAndDie();
                    break;
                default:
                    break;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (FelmystOutroTimer.Expired(diff))
        {
            me->SetSpeed(MOVE_FLIGHT, 2.5);
            me->GetMotionMaster()->MovePoint(60, 1547, 531, 161);
            FelmystOutroTimer = 0;
        }

        if (!Searched)
        {
            FindOrbs();
            Searched = true;
        }

        if (OrbsEmpowered == 4) OrbsEmpowered = 0;
    }



};

CreatureAI* GetAI_boss_kalecgos_kj(Creature *_Creature)
{
    return new boss_kalecgos_kjAI(_Creature);
}

//AI for Kil'jaeden
struct boss_kiljaedenAI : public Scripted_NoMovementAI
{
    boss_kiljaedenAI(Creature* c) : Scripted_NoMovementAI(c), Summons(m_creature)
    {
        pInstance = (c->GetInstanceData());
        me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
    }

    ScriptedInstance* pInstance;
    SummonList Summons;

    uint8 Phase;
    uint8 SpikesLeft; // he releases 9 spikes while casting Shadow Spikes
    uint8 SpeechPhase;

    Timer _Timer[TIMER_KJ_MAX];
    Timer WaitTimer;
    Timer StunTimer;
    Timer Emerging;   // we don't want to damage players when emerging, we do this when the fight starts

    /* Boolean */
    bool IsKalecJoined;
    bool IsInDarkness;
    bool IsEmerging;
    bool IsCastingSpikes;

    void Reset()
    {
        Summons.DespawnAll();
        //Phase 2 Timer
        _Timer[TIMER_KALEC_JOIN].Reset(26000);
        _Timer[TIMER_SOUL_FLAY].Reset(3000);
        _Timer[TIMER_LEGION_LIGHTNING].Reset(10000);
        _Timer[TIMER_FIRE_BLOOM].Reset(13000);
        _Timer[TIMER_SUMMON_SHILEDORB].Reset(urand(10000,16000));

        //other phases - inactive
        _Timer[TIMER_SHADOW_SPIKE].Reset(0);
        _Timer[TIMER_FLAME_DART].Reset(0);
        _Timer[TIMER_DARKNESS].Reset(0);
        _Timer[TIMER_ORBS_EMPOWER].Reset(0);
        _Timer[TIMER_ARMAGEDDON].Reset(0);
        _Timer[TIMER_ANVEENA_SPEECH].Reset(0);

        WaitTimer.Reset(1);
        StunTimer.Reset(5000);

        Phase = PHASE_DECEIVERS;
        SpeechPhase = 0;

        Emerging.Reset(10000);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);

        IsKalecJoined   = false;
        IsInDarkness    = false;
        IsEmerging      = true;

        IsCastingSpikes = false;
        SpikesLeft      = 9;
    }

    void MoveInLineOfSight(Unit* who)
    {
        if (who->GetTypeId() != TYPEID_PLAYER)
            return;
        Creature* caster = me->SummonCreature(12999, me->GetPositionX(), me->GetPositionY(), 28.540001, 0, TEMPSUMMON_TIMED_DESPAWN, 1); // we use invisible trigger to avoid animation break
        if (caster)                                                                                                                      // on KJ; can't use controller because he has no skin
            if (me->GetExactDist2d(who->GetPositionX(), who->GetPositionY()) <= 12.0f)                                                   // which causes wrong-cast-height problem (0yd kb)
            {
                if (!IsEmerging)
                {
                    me->DealDamage(who, 475, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_FIRE);
                    me->SendSpellNonMeleeDamageLog(who, SPELL_SUNWELL_KNOCKBACK, 475, SPELL_SCHOOL_MASK_FIRE, 0, 0, false, 0);
                }
                caster->CastSpell(who, SPELL_SUNWELL_KNOCKBACK, false, 0, 0, me->GetGUID());
            }

    }

    void SpellHitTarget(Unit* target, const SpellEntry* spell)
    {    }

    void JustSummoned(Creature* summoned)
    {
        if (summoned->GetEntry() == CREATURE_ARMAGEDDON_TARGET)
        {
            summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
        }
        else
        {
            summoned->SetLevel(m_creature->GetLevel());
        }
        summoned->setFaction(m_creature->getFaction());
        Summons.Summon(summoned);
    }

    void JustDied(Unit* killer)
    {
        DoScriptText(SAY_KJ_DEATH, m_creature);

        if (pInstance)
        {
            pInstance->SetData(DATA_KILJAEDEN_EVENT, DONE);
            Creature* Control = pInstance->instance->GetCreatureById(CREATURE_KILJAEDEN_CONTROLLER);
            if (Control)
                Control->Kill(Control);
        }
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_KJ_SLAY1, SAY_KJ_SLAY2), m_creature);
    }

    void EnterEvadeMode()
    {
        Scripted_NoMovementAI::EnterEvadeMode();
        Summons.DespawnAll();
        
        if (Creature* Kalec = pInstance->instance->GetCreatureById(CREATURE_KALECGOS))
            CAST_AI(boss_kalecgos_kjAI, Kalec->AI())->ResetOrbs();

        // Reset the controller
        if (pInstance)
        {
            Creature* Control = pInstance->instance->GetCreatureById(CREATURE_KILJAEDEN_CONTROLLER);
            if (Control)
                ((Scripted_NoMovementAI*)Control->AI())->EnterEvadeMode();
        }
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat();
    }

    void CastSinisterReflection()
    {
        DoScriptText(RAND(SAY_KJ_REFLECTION1, SAY_KJ_REFLECTION2), m_creature);
        float x, y, z;
        Unit* target;

        target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true);
        target->GetPosition(x, y, z);
        for (uint8 i = 0; i < 4; i++)
        {
            Creature* SinisterReflection = m_creature->SummonCreature(CREATURE_SINISTER_REFLECTION, x - sinf(M_PI/2*i), y - cosf(M_PI/2*i), z, 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
            if (SinisterReflection)
            {
                SinisterReflection->setFaction(me->getFaction());
                SinisterReflection->SetFacingToObject(target);
                SinisterReflection->Attack(target, false);

            }
            if (i == 3)
                target->CastSpell(SinisterReflection, SPELL_SINISTER_REFLECTION, true);
        }


    }

    void UpdateAI(const uint32 diff)
    {
        if (Emerging.Expired(diff))
        {
            Emerging = 0;
            IsEmerging = false;
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
            DoScriptText(SAY_KJ_EMERGE, me);
        }

        if (Phase == PHASE_SACRIFICE)
        {
            if (StunTimer.Expired(diff))
            {
                m_creature->ClearUnitState(UNIT_STAT_STUNNED);
                StunTimer = 0;
            }
        }

        if (Phase < PHASE_NORMAL || IsEmerging || !UpdateVictim())
            return;

        if (WaitTimer.GetTimeLeft())
        {
            WaitTimer.Update(diff);
            return;
        }


        for (uint8 t = 0; t < TIMER_KJ_MAX; t++)
        {
            if (_Timer[t].Expired(diff))
            {
                switch (t)
                {
                    case TIMER_KALEC_JOIN:
                    {
                        if (Creature* Kalec = pInstance->instance->GetCreatureById(CREATURE_KALECGOS))
                        {
                            DoScriptText(SAY_KALECGOS_JOIN, Kalec);
                            IsKalecJoined = true;
                            _Timer[TIMER_KALEC_JOIN] = 0;
                        }
                        break;
                    }
                    case TIMER_SOUL_FLAY:
                    {
                        if (me->IsNonMeleeSpellCast(false))
                            break;

                        Unit* target = SelectUnit(SELECT_TARGET_TOPAGGRO, 0,
                            [this](Unit* u) {return !u->IsImmunedToDamage(SPELL_SCHOOL_MASK_SHADOW); });
                        if (target)
                            AddSpellToCast(target, SPELL_SOUL_FLAY);

                        _Timer[TIMER_SOUL_FLAY] = 4000;
                        break;
                    }
                    case TIMER_LEGION_LIGHTNING:
                    {
                        Unit* randomPlayer = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true);
                        if (randomPlayer)
                            AddSpellToCast(randomPlayer, SPELL_LEGION_LIGHTNING, false, true);
                        else
                            error_log("try to cast SPELL_LEGION_LIGHTNING on invalid target");

                        _Timer[TIMER_LEGION_LIGHTNING] = (Phase == PHASE_SACRIFICE) ? 18000 : urand(13000, 17000); // 18 seconds in PHASE_SACRIFICE
                        _Timer[TIMER_SOUL_FLAY].Reset(3500);
                        break;
                    }
                    case TIMER_FIRE_BLOOM:
                    {
                        Unit* randomPlayer = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true);
                        if (randomPlayer)
                            AddSpellToCast(randomPlayer, SPELL_FIRE_BLOOM);
                        _Timer[TIMER_FIRE_BLOOM] = (Phase == PHASE_SACRIFICE) ? 25000 : urand(35000, 40000); // 25 seconds in PHASE_SACRIFICE
                        break;
                    }
                    case TIMER_SUMMON_SHILEDORB:
                    {
                        for (uint8 i = 1; i < Phase; ++i)
                        {
                            float sx, sy;
                            sx = ShieldOrbLocations[0][0] + ShieldOrbLocations[i][1]*sin(ShieldOrbLocations[i][0]);
                            sy = ShieldOrbLocations[0][1] + ShieldOrbLocations[i][1]*cos(ShieldOrbLocations[i][0]);
                            Creature* shieldorb = m_creature->SummonCreature(CREATURE_SHIELD_ORB, sx, sy, SHIELD_ORB_Z, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 45000);
                            if (shieldorb)
                                shieldorb->AI()->DoAction(i);
                        }
                        _Timer[TIMER_SUMMON_SHILEDORB] = (Phase == PHASE_NORMAL) ? urand(30000,38000) : 0;
                        
                        break;
                    }
                    /*$$$$$$$$$$$$$$$$$$$$$$$$$$$
                              Phase 3
                    $$$$$$$$$$$$$$$$$$$$$$$$$$$$*/


                    case TIMER_SHADOW_SPIKE:
                    {
                        if (!IsCastingSpikes)
                        {
                            AddSpellToCast(SPELL_SHADOW_SPIKE, CAST_NULL);
                            _Timer[TIMER_SHADOW_SPIKE].Reset(2500);
                            IsCastingSpikes = true;
                        }
                        else
                        {
                            if (SpikesLeft)
                            {
                                Unit* random = SelectUnit(SELECT_TARGET_RANDOM, 0, 100.0f, true);
                                Position pos;
                                if (random)
                                {
                                    SendDebug("spike on %s", random->GetName());
                                    random->GetPosition(&pos);
                                    me->SummonCreature(CREATURE_SPIKE_TARGET1, pos.m_positionX, pos.m_positionY, pos.m_positionZ, 0, TEMPSUMMON_TIMED_DESPAWN, 2800);
                                }
                                SpikesLeft--;
                                _Timer[TIMER_SHADOW_SPIKE] = 3000;
                                me->SetSelection(0);
                            }
                            else
                            {
                                SpikesLeft = 9;
                                _Timer[TIMER_SHADOW_SPIKE] = 0;
                                IsCastingSpikes = false;
                                _Timer[TIMER_DARKNESS].Reset(45000);
                            }

                        }
                        break;
                    }
                    case TIMER_FLAME_DART:
                    {
                        AddSpellToCast(SPELL_FLAME_DART, CAST_NULL);
                        _Timer[TIMER_FLAME_DART] = urand(18000, 22000);
                        break;
                    }
                    case TIMER_DARKNESS:
                    {
                        ClearCastQueue();
                        // Begins to channel for 8 seconds, then deals 50'000 damage to all raid members.
                        if (!IsInDarkness)
                        {
                            SendDebug("Channeling darkness");
                            me->InterruptNonMeleeSpells(true);
                            DoScriptText(EMOTE_KJ_DARKNESS, m_creature);
                            DoCast(m_creature, SPELL_DARKNESS_OF_A_THOUSAND_SOULS, false);
                            WaitTimer.Reset(8000);
                            _Timer[TIMER_DARKNESS] = 1;
                            IsInDarkness = true;
                            _Timer[TIMER_ARMAGEDDON] = 0;
                        }
                        else
                        {
                            _Timer[TIMER_DARKNESS] = (Phase == PHASE_SACRIFICE) ? urand(17000,22000) :urand(35000,40000); // from end of one to begining of another
                            IsInDarkness = false;
                            DoCastAOE(SPELL_DARKNESS_OF_A_THOUSAND_SOULS_DAMAGE, true);

                            DoScriptText(RAND(SAY_KJ_DARKNESS1, SAY_KJ_DARKNESS2, SAY_KJ_DARKNESS3), m_creature);
                            SendDebug("Casting aoe darkness");
                            _Timer[TIMER_SOUL_FLAY].Delay(3000);
                            if (Phase >= PHASE_ARMAGEDDON)
                                _Timer[TIMER_ARMAGEDDON].Reset(3000);
                            if (Phase != PHASE_SACRIFICE)
                                _Timer[TIMER_SUMMON_SHILEDORB].Reset(5000);
                        }
                        
                        break;
                    }
                    case TIMER_ORBS_EMPOWER:
                    {
                        Creature* Kalec = pInstance->instance->GetCreatureById(CREATURE_KALECGOS);
                        if (Kalec)
                            ((boss_kalecgos_kjAI*)Kalec->AI())->EmpowerOrb(Phase == PHASE_SACRIFICE);

                        _Timer[TIMER_ORBS_EMPOWER] = 0;
                        if (Phase == PHASE_SACRIFICE)
                            _Timer[TIMER_ARMAGEDDON].Reset(10000);
                        break; 
                    }
                    case TIMER_ARMAGEDDON:
                    {
                        Unit* target = NULL;
                        while (!target)
                        {
                            target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true);
                            if (target->HasAura(SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT))
                                target = NULL;
                        }
                        if (target)
                        {
                            float x, y, z;
                            target->GetPosition(x, y, z);
                            m_creature->SummonCreature(CREATURE_ARMAGEDDON_TARGET, x, y, z, 0, TEMPSUMMON_TIMED_DESPAWN, 15000);
                        }
                        _Timer[TIMER_ARMAGEDDON] = (Phase == PHASE_ARMAGEDDON) ? 3500 : 2200;
                        break;
                    }
                    case TIMER_ANVEENA_SPEECH:
                    {

                        if (SpeechPhase == 9)
                        {
                            Creature* Anveena = pInstance->instance->GetCreatureById(CREATURE_ANVEENA);
                            if (Anveena)
                            {
                                Anveena->CastSpell(m_creature, SPELL_SACRIFICE_OF_ANVEENA, false);
                                Anveena->Kill(Anveena);
                            }
                            StunTimer.Reset(5000);// He shouldn't cast spells for ~5 seconds after Anveena's sacrifice.
                            m_creature->addUnitState(UNIT_STAT_STUNNED);
                        }

                        Unit* speaker = pInstance->instance->GetCreatureById(Sacrifice[SpeechPhase].creature);
                        if (speaker)
                            DoScriptText(Sacrifice[SpeechPhase].textid, speaker);
                        _Timer[TIMER_ANVEENA_SPEECH] = Sacrifice[SpeechPhase].timer;
                        SpeechPhase++;
                        break;
                    }
                }
            }
        }

        //Phase 3
        if (Phase == PHASE_NORMAL && !me->IsNonMeleeSpellCast(true) && ((m_creature->GetHealth() * 100 / m_creature->GetMaxHealth()) < 85))
        {
            // nothing should be cast before shadow spike ends
            ClearCastQueue();
            
            _Timer[TIMER_SOUL_FLAY].Delay(34000);
            _Timer[TIMER_LEGION_LIGHTNING].Reset(47000);
            _Timer[TIMER_FIRE_BLOOM].Reset(67000);
            _Timer[TIMER_SUMMON_SHILEDORB].Reset(urand(45000, 60000));
            _Timer[TIMER_SHADOW_SPIKE].Reset(4000);
            _Timer[TIMER_FLAME_DART].Reset(57000);
            _Timer[TIMER_DARKNESS].Reset(45000);
            _Timer[TIMER_ORBS_EMPOWER].Reset(35000);
            CastSinisterReflection();

            DoScriptText(SAY_KJ_PHASE3, m_creature);
            Phase = PHASE_DARKNESS;
            SendDebug("Entering phase 3");
        }

        //Phase 4
        if (Phase == PHASE_DARKNESS && !me->IsNonMeleeSpellCast(true) && ((m_creature->GetHealth() * 100 / m_creature->GetMaxHealth()) < 55))
        {
            ClearCastQueue();
            
            _Timer[TIMER_SOUL_FLAY].Delay(34000);
            _Timer[TIMER_LEGION_LIGHTNING].Reset(47000);
            _Timer[TIMER_FIRE_BLOOM].Reset(67000);
            _Timer[TIMER_SUMMON_SHILEDORB].Reset(urand(45000, 60000));
            _Timer[TIMER_SHADOW_SPIKE].Reset(4000);
            _Timer[TIMER_FLAME_DART].Reset(57000);
            _Timer[TIMER_DARKNESS].Reset(45000);
            _Timer[TIMER_ORBS_EMPOWER].Reset(35000);
            _Timer[TIMER_ARMAGEDDON].Reset(50000);
            CastSinisterReflection();

            DoScriptText(SAY_KJ_PHASE4, m_creature);
            Phase = PHASE_ARMAGEDDON;
            SendDebug("Entering phase 4");
        }

        //Phase 5 specific spells all we can
        if (Phase == PHASE_ARMAGEDDON && !me->IsNonMeleeSpellCast(true) && ((m_creature->GetHealth() * 100 / m_creature->GetMaxHealth()) < 25))
        {
            ClearCastQueue();
            _Timer[TIMER_SOUL_FLAY].Delay(34000);
            _Timer[TIMER_LEGION_LIGHTNING].Reset(47000);
            _Timer[TIMER_FIRE_BLOOM].Reset(67000);
            _Timer[TIMER_SUMMON_SHILEDORB].Reset(0);
            _Timer[TIMER_SHADOW_SPIKE].Reset(4000);
            _Timer[TIMER_FLAME_DART].Reset(57000);
            _Timer[TIMER_DARKNESS].Reset(45000);
            _Timer[TIMER_ORBS_EMPOWER].Reset(35000);
            _Timer[TIMER_ARMAGEDDON].Reset(0);
            _Timer[TIMER_ANVEENA_SPEECH].Reset(1);
            CastSinisterReflection();

            Phase = PHASE_SACRIFICE;
            SendDebug("Entering phase 5");
        }

        CastNextSpellIfAnyAndReady();

    }

    void GetDebugInfo(ChatHandler& reader)
    {
        std::ostringstream str;
        str << "KJD Debugai, phase " << (int)Phase << "\n";
        for (uint8 i = 0; i < 10; i++)
            str << "Timer " << (int)i << " : " << (int)_Timer[i].GetTimeLeft()  << "\n";
        str << "WaitTimer : " << (int)WaitTimer.GetTimeLeft();
        reader.SendSysMessage(str.str().c_str());
    }
};

CreatureAI* GetAI_boss_kiljaeden(Creature *_Creature)
{
    return new boss_kiljaedenAI(_Creature);
}

//AI for Kil'jaeden Event Controller
struct mob_kiljaeden_controllerAI : public Scripted_NoMovementAI
{
    mob_kiljaeden_controllerAI(Creature* c) : Scripted_NoMovementAI(c), Summons(m_creature)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;
    Creature* KalecKJ;
    SummonList Summons;

    bool SummonedAnveena;
    bool KiljaedenDeath;

    Timer RandomSayTimer;
    Timer CheckDeceivers;

    uint32 Phase;

    void Reset()
    {
        KalecKJ = pInstance->instance->GetCreatureById(CREATURE_KALECGOS);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
        m_creature->addUnitState(UNIT_STAT_STUNNED);

        pInstance->SetData(DATA_KILJAEDEN_EVENT, NOT_STARTED);
        pInstance->SetData(DATA_HAND_OF_DECEIVER_COUNT, 1);
        std::list<uint64> hands = m_creature->GetMap()->GetCreaturesGUIDList(CREATURE_HAND_OF_THE_DECEIVER, GET_FIRST_CREATURE_GUID, 3);
        for (std::list<uint64>::iterator itr = hands.begin(); itr != hands.end(); itr++)
        {
            if (Creature* oneHand = m_creature->GetMap()->GetCreature(*itr))
                oneHand->Respawn();
        }
        Phase = PHASE_DECEIVERS;
        //if(KalecKJ)((boss_kalecgos_kjAI*)KalecKJ->AI())->ResetOrbs();
        //DeceiverDeathTimer = 0;
        SummonedAnveena = false;
        KiljaedenDeath = false;
        RandomSayTimer.Reset(30000);
        CheckDeceivers.Reset(1000);
        Summons.DespawnAll();
        me->SetReactState(REACT_AGGRESSIVE);
        DoCastAOE(SPELL_KILL_DRAKES);
    }

    void EnterCombat(Unit* who)
    {
        pInstance->SetData(DATA_KILJAEDEN_EVENT, IN_PROGRESS);
        DoZoneInCombat(1000.0f);
    }

    void JustSummoned(Creature* summoned)
    {
        switch (summoned->GetEntry())
        {
            case CREATURE_ANVEENA:
                summoned->SetLevitate(true);
                summoned->CastSpell(summoned, SPELL_ANVEENA_PRISON, true);
                summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                break;
            case CREATURE_KILJAEDEN:
                summoned->CastSpell(summoned, SPELL_REBIRTH, false);
                ((boss_kiljaedenAI*)summoned->AI())->Phase = PHASE_NORMAL;
                summoned->AddThreat(m_creature->GetVictim(), 0.0f);
                summoned->AI()->DoZoneInCombat();
                break;
        }
        Summons.Summon(summoned);
    }

    void DoRandomSay(uint32 diff)
    {    // do not yell when any encounter in progress
        if (pInstance->IsEncounterInProgress())
            return;

        if (!pInstance->IsEncounterInProgress() && pInstance->GetData(DATA_MURU_EVENT) != DONE && RandomSayTimer.Expired(diff))
        {
            DoScriptText(RAND(SAY_KJ_OFFCOMBAT1, SAY_KJ_OFFCOMBAT2, SAY_KJ_OFFCOMBAT3, SAY_KJ_OFFCOMBAT4, SAY_KJ_OFFCOMBAT5), m_creature);
            RandomSayTimer = 60000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!pInstance)
            return;

        DoRandomSay(diff);

        if (!SummonedAnveena)
        {
            Unit* Anveena = DoSpawnCreature(CREATURE_ANVEENA, 0, 0, 40, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
            if (Anveena)
                Anveena->setHover(true);
            DoCast(m_creature, SPELL_ANVEENA_ENERGY_DRAIN);
            SummonedAnveena = true;
        }

        if (CheckDeceivers.Expired(diff))
        {
            CheckDeceivers = 1000;
            if (pInstance->GetData(DATA_HAND_OF_DECEIVER_COUNT) == 0 && pInstance->GetData(DATA_KILJAEDEN_EVENT) == IN_PROGRESS && Phase == PHASE_DECEIVERS)
            {
                m_creature->RemoveAurasDueToSpell(SPELL_ANVEENA_ENERGY_DRAIN);
                Phase = PHASE_NORMAL;
                DoSpawnCreature(CREATURE_KILJAEDEN, 0, 0, 0, 3.886, TEMPSUMMON_MANUAL_DESPAWN, 0);
                CheckDeceivers = 0;
            }
        }

        if (me->getThreatManager().isThreatListEmpty() && me->IsInCombat())
            EnterEvadeMode(); // we use this instead of UpdateVictim()


    }
};

CreatureAI* GetAI_mob_kiljaeden_controller(Creature *_Creature)
{
    return new mob_kiljaeden_controllerAI(_Creature);
}

//AI for Hand of the Deceiver
struct mob_hand_of_the_deceiverAI : public ScriptedAI
{
    mob_hand_of_the_deceiverAI(Creature* c) : ScriptedAI(c), Summons(m_creature)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    Timer ShadowBoltVolleyTimer;
    Timer FelfirePortalTimer;
    SummonList Summons;

    void Reset()
    {
        if (!me->IsInEvadeMode())
            me->CastSpell(me, SPELL_SHADOW_CHANNELING, false);

        ShadowBoltVolleyTimer.Reset(1000 + urand(0, 3000)); // So they don't all cast it in the same moment.
        FelfirePortalTimer.Reset(20000);
        Summons.DespawnAll();
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->setFaction(m_creature->getFaction());
        summoned->SetLevel(m_creature->GetLevel());
        summoned->AI()->DoZoneInCombat();
        Summons.Summon(summoned);
    }

    void JustReachedHome()
    {
        me->CastSpell(me, SPELL_SHADOW_CHANNELING, false);
    }

    void EnterCombat(Unit* who)
    {
        if (pInstance)
        {
            Creature* Control = pInstance->instance->GetCreatureById(CREATURE_KILJAEDEN_CONTROLLER);
            if (Control)
                Control->AI()->EnterCombat(who);
        }
        m_creature->InterruptNonMeleeSpells(true);
    }

    void EnterEvadeMode()
    {        
        if (!_EnterEvadeMode())
            return;

        float x, y, z, o;
        m_creature->GetRespawnCoord(x, y, z, &o);
        m_creature->SetHomePosition(x, y, z, o);
        me->GetMotionMaster()->MoveTargetedHome();
        Reset();
    }

    void JustDied(Unit* killer)
    {
        if (!pInstance)
            return;

        if (!me->GetMap()->GetCreatureById(CREATURE_HAND_OF_THE_DECEIVER, GET_ALIVE_CREATURE_GUID))
        {
            pInstance->SetData(DATA_HAND_OF_DECEIVER_COUNT, 0);
        }
    }


    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        // Gain Shadow Infusion at 20% health
        if (((m_creature->GetHealth() * 100 / m_creature->GetMaxHealth()) < 20) && !m_creature->HasAura(SPELL_SHADOW_INFUSION, 0))
            ForceSpellCast(me, SPELL_SHADOW_INFUSION, INTERRUPT_AND_CAST_INSTANTLY, true);

        // Shadow Bolt Volley - Shoots Shadow Bolts at all enemies within 30 yards, for ~2k Shadow damage.
        if (ShadowBoltVolleyTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_SHADOW_BOLT_VOLLEY, CAST_TANK);
            ShadowBoltVolleyTimer = urand(1000, 5000);
        }

        // Felfire Portal - Creatres a portal, that spawns Volatile Felfire Fiends, which do suicide bombing.
        if (FelfirePortalTimer.Expired(diff))
        {
            DoSpawnCreature(CREATURE_FELFIRE_PORTAL, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 20000);
            FelfirePortalTimer = 20000;
        }

        
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_hand_of_the_deceiver(Creature *_Creature)
{
    return new mob_hand_of_the_deceiverAI(_Creature);
}

//AI for Felfire Portal
struct mob_felfire_portalAI : public Scripted_NoMovementAI
{
    mob_felfire_portalAI(Creature* c) : Scripted_NoMovementAI(c) {}

    Timer SpawnFiendTimer;
    uint64 myOwnerGUID;

    void Reset()
    {
        myOwnerGUID =0;
        SpawnFiendTimer = 3000;
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
    }
    
    void IsSummonedBy(Unit* owner)
    {
        myOwnerGUID = owner->GetGUID();
    }

    void JustSummoned(Creature* summoned)
    {
        Creature* owner = me->GetCreature(myOwnerGUID);
            owner->AI()->JustSummoned(summoned);
    }
    
    void UpdateAI(const uint32 diff)
    {
        if (SpawnFiendTimer.Expired(diff))
        {
            Creature* Fiend = DoSpawnCreature(CREATURE_VOLATILE_FELFIRE_FIEND, 0, 0, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 20000);
            SpawnFiendTimer = 3000;
        }
    }
};

CreatureAI* GetAI_mob_felfire_portal(Creature *_Creature)
{
    return new mob_felfire_portalAI(_Creature);
}

//AI for Felfire Fiend
struct mob_volatile_felfire_fiendAI : public ScriptedAI
{
    mob_volatile_felfire_fiendAI(Creature* c) : ScriptedAI(c) {}

    Timer ExplodeTimer;
    Timer WaitTimer;

    bool LockedTarget;

    void Reset()
    {
        WaitTimer.Reset(1500);
        ExplodeTimer.Reset(5000);
        LockedTarget = false;
        me->SetReactState(REACT_PASSIVE);
    }


    void UpdateAI(const uint32 diff)
    {
        if (WaitTimer.Expired(diff))
        {
            WaitTimer = 0;
            me->SetReactState(REACT_AGGRESSIVE);
            DoZoneInCombat(100.0f);
            Unit* random = SelectUnit(SELECT_TARGET_RANDOM, 0, 100.0f, true);
            if (random)
                AttackStart(random);
        }

        if (WaitTimer.GetTimeLeft())
            return;
        
        if (!me->GetVictim())
        {
            Unit* random = SelectUnit(SELECT_TARGET_RANDOM, 0, 100.0f, true);
            if (random)
                AttackStart(random);
            return;
        }

        if (ExplodeTimer.Expired(diff) || me->GetDistance(me->GetVictim()) < 5.0f) // Explode if it's close enough to it's target
        {
            me->CastCustomSpell(SPELL_FELFIRE_FISSION, SPELLVALUE_BASE_POINT0, 4000, m_creature->GetVictim(), true);
            me->DisappearAndDie();
        }
    }
};

CreatureAI* GetAI_mob_volatile_felfire_fiend(Creature *_Creature)
{
    return new mob_volatile_felfire_fiendAI(_Creature);
}

//AI for Armageddon target
struct mob_armageddonAI : public Scripted_NoMovementAI
{
    mob_armageddonAI(Creature* c) : Scripted_NoMovementAI(c) {}

    uint8 Spell;
    uint32 Timer;

    void Reset()
    {
        Spell = 0;
        Timer = 0;
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
        me->SetFloatValue(OBJECT_FIELD_SCALE_X, 4.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (Timer <= diff)
        {
            switch (Spell)
            {
                case 0:
                    DoCast(m_creature, SPELL_ARMAGEDDON_VISUAL, true);
                    ++Spell;
                    break;
                case 1:
                    DoCast(m_creature, SPELL_ARMAGEDDON_VISUAL2, true);
                    Timer = 8000;
                    ++Spell;
                    break;
                case 2:
                    m_creature->CastSpell(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(),
                        SPELL_ARMAGEDDON_TRIGGER, true);
                    ++Spell;
                    Timer = 5000;
                    break;
                case 3:
                    m_creature->DealDamage(m_creature, m_creature->GetMaxHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                    m_creature->RemoveCorpse();
                    break;
            }
        }
        else Timer -= diff;
    }
};

CreatureAI* GetAI_mob_armageddon(Creature *_Creature)
{
    return new mob_armageddonAI(_Creature);
}

//AI for Shield Orbs
struct mob_shield_orbAI : public ScriptedAI
{
    mob_shield_orbAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    bool PointReached;
    bool Clockwise;
    Timer _Timer;
    Timer CheckTimer;
    ScriptedInstance* pInstance;
    float x, y, r, c, mx, my;

    void Reset()
    {
        me->SetIgnoreVictimSelection(true);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
        me->SetSelection(0);
        m_creature->SetLevitate(true);
        PointReached = true;
        _Timer.Reset(urand(500,1000));
        CheckTimer.Reset(1000);
        mx = ShieldOrbLocations[0][0];
        my = ShieldOrbLocations[0][1];
        Clockwise = true;
    }

    void DoAction(const int32 act)
    {
        c = ShieldOrbLocations[act][0];
        r = ShieldOrbLocations[act][1];
        if (act == 1 || act == 3)
            Clockwise = false;
    }

    void UpdateAI(const uint32 diff)
    {
        if (me->GetSelection())
            me->SetSelection(0);
        if (PointReached)
        {
            y = my + r * cos(c);
            x = mx + r * sin(c);
            PointReached = false;
            m_creature->GetMotionMaster()->MovePoint(1, x, y, SHIELD_ORB_Z);
            if (Clockwise)
                c += M_PI / 50;
            else
                c -= M_PI / 50;

            if (c > 2 * M_PI)
                c -= 2 * M_PI;
            if (c < 0)
                c += 2 * M_PI;
        }

        

        if (_Timer.Expired(diff))
        {
            ForceSpellCast(SPELL_SHADOW_BOLT, CAST_RANDOM, INTERRUPT_AND_CAST_INSTANTLY, true);
            _Timer = 500; // 2 per second
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type != POINT_MOTION_TYPE)
            return;

        PointReached = true;
    }

};

CreatureAI* GetAI_mob_shield_orb(Creature *_Creature)
{
    return new mob_shield_orbAI(_Creature);
}

//AI for Sinister Reflection
struct mob_sinster_reflectionAI : public ScriptedAI
{
    mob_sinster_reflectionAI(Creature* c) : ScriptedAI(c) {}

    uint8 Class;
    Timer _Timer[3];
    Timer Wait;

    void Reset()
    {
        _Timer[0].Reset(1);
        _Timer[1].Reset(1);
        _Timer[2].Reset(1);
        Wait.Reset(5000);
        Class = 0;
        m_creature->addUnitState(UNIT_STAT_CANNOT_AUTOATTACK);
        m_creature->addUnitState(UNIT_STAT_NOT_MOVE);
    }

    void UpdateAI(const uint32 diff)
    {
        
        if (Class == 0 && me->GetVictim())
        {
            me->CastSpell(me, SPELL_SINISTER_REFLECTION_ENLARGE, true);
            Class = m_creature->GetVictim()->GetClass();
            switch (Class)
            {
                case CLASS_DRUID:
                    break;
                case CLASS_HUNTER:
                    break;
                case CLASS_MAGE:
                    break;
                case CLASS_WARLOCK:
                    break;
                case CLASS_WARRIOR:
                    m_creature->SetCanDualWield(true);
                    break;
                case CLASS_PALADIN:
                    break;
                case CLASS_PRIEST:
                    break;
                case CLASS_SHAMAN:
                    m_creature->SetCanDualWield(true);
                    break;
                case CLASS_ROGUE:
                    m_creature->SetCanDualWield(true);
                    break;
            }
        }

        if (Wait.Expired(diff))
        {
            m_creature->ClearUnitState(UNIT_STAT_CANNOT_AUTOATTACK);
            m_creature->ClearUnitState(UNIT_STAT_NOT_MOVE);
            Wait = 0;
        }

        if (Wait.GetTimeLeft())
            return;

        if (!UpdateVictim())
            return;

        switch (Class)
        {
            case CLASS_DRUID:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_MOONFIRE, false);
                    _Timer[1] = 3000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_HUNTER:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_MULTI_SHOT, false);
                    _Timer[1] = 9000;
                }
                if (_Timer[2].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_SHOOT, false);
                    _Timer[2] = 5000;
                }
                if (m_creature->IsWithinMeleeRange(m_creature->GetVictim(), 6))
                {
                    if (_Timer[3].Expired(diff))
                    {
                        DoCast(m_creature->GetVictim(), SPELL_SR_MULTI_SHOT, false);
                        _Timer[3] = 7000;
                    }
                    DoMeleeAttackIfReady();
                }
                break;
            case CLASS_MAGE:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_FIREBALL, false);
                    _Timer[1] = 3000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_WARLOCK:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_SHADOW_BOLT, false);
                    _Timer[1] = 4000;
                }
                if (_Timer[2].Expired(diff))
                {
                    if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true))
                        DoCast(target, SPELL_SR_CURSE_OF_AGONY, true);
                    _Timer[2] = 3000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_WARRIOR:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_WHIRLWIND, false);
                    _Timer[1] = 10000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_PALADIN:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_HAMMER_OF_JUSTICE, false);
                    _Timer[1] = 7000;
                }
                if (_Timer[2].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_HOLY_SHOCK, false);
                    _Timer[2] = 3000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_PRIEST:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_HOLY_SMITE, false);
                    _Timer[1] = 5000;
                }
                if (_Timer[2].Expired(diff) && (me->GetHealth()*2 < me->GetMaxHealth()))
                {
                    DoCast(m_creature, SPELL_SR_RENEW, false);
                    _Timer[2] = 7000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_SHAMAN:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_EARTH_SHOCK, false);
                    _Timer[1] = 5000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_ROGUE:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_HEMORRHAGE, true);
                    _Timer[1] = 5000;
                }
                DoMeleeAttackIfReady();
                break;
        }
    }

};

CreatureAI* GetAI_mob_sinster_reflection(Creature *_Creature)
{
    return new mob_sinster_reflectionAI(_Creature);
}

void AddSC_boss_kiljaeden()
{
    Script* newscript;

    newscript = new Script;
    newscript->pGOUse = &GOUse_go_orb_of_the_blue_flight;
    newscript->Name = "go_orb_of_the_blue_flight";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_boss_kalecgos_kj;
    newscript->Name = "boss_kalecgos_kj";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_boss_kiljaeden;
    newscript->Name = "boss_kiljaeden";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_kiljaeden_controller;
    newscript->Name = "mob_kiljaeden_controller";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_hand_of_the_deceiver;
    newscript->Name = "mob_hand_of_the_deceiver";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_felfire_portal;
    newscript->Name = "mob_felfire_portal";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_volatile_felfire_fiend;
    newscript->Name = "mob_volatile_felfire_fiend";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_armageddon;
    newscript->Name = "mob_armageddon";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_shield_orb;
    newscript->Name = "mob_shield_orb";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_sinster_reflection;
    newscript->Name = "mob_sinster_reflection";
    newscript->RegisterSelf();
};
