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
SDName: Boss_Headless_Horseman
SD%Complete:
SDComment:
SDCategory: Scarlet Monastery
EndScriptData */

#include "precompiled.h"
#include "SpellMgr.h"
#include "def_scarlet_monastery.h"

#define SAY_ENTRANCE                -1189001
#define SAY_REJOINED                -1189002
#define SAY_LOST_HEAD               -1189003
#define SAY_CONFLAGRATION           -1189004
#define SAY_SPROUTING_PUMPKINS      -1189005
#define SAY_PLAYER_DEATH            -1189006
#define SAY_DEATH                   -1189007

uint32 RandomLaugh[] = {11965, 11975, 11976};

    // Entryes
#define HH_MOUNTED                  23682
#define HH_UNHORSED                 23800
#define HEAD                        23775
#define PULSING_PUMPKIN             23694
#define PUMPKIN_FIEND               23545
#define HELPER                      23686
#define WISP_INVIS                  24034

    //Spells
#define SPELL_CLEAVE                42587
#define SPELL_CONFLAGRATION         42380       //Phase 2, can't find real spell(Dim Fire?)
//#define SPELL_CONFL_SPEED         22587       //8% increase speed, value 22587 from SPELL_CONFLAGRATION mains that spell?
#define SPELL_SUMMON_PUMPKIN        42394

#define SPELL_WHIRLWIND             43116
#define SPELL_IMMUNE                42556
#define SPELL_BODY_REGEN            42403
#define SPELL_CONFUSE               43105

#define SPELL_FLYING_HEAD           42399       //visual flying head
#define SPELL_HEAD                  42413       //visual buff, "head"
#define SPELL_HEAD_IS_DEAD          42428       //at killing head, Phase 3

#define SPELL_PUMPKIN_AURA          42280
#define SPELL_PUMPKIN_AURA_GREEN    42294
#define SPELL_SQUASH_SOUL           42514
#define SPELL_SPROUTING             42281
#define SPELL_SPROUT_BODY           42285

    //Effects
#define SPELL_RHYME_BIG             42909
//#define SPELL_RHYME_SMALL         42910
#define SPELL_HEAD_SPEAKS           43129
#define SPELL_HEAD_LANDS            42400
#define SPELL_BODY_FLAME            42074
#define SPELL_HEAD_FLAME            42971
//#define SPELL_ENRAGE_VISUAL       42438       // he uses this spell?
#define SPELL_WISP_BLUE             42821
#define SPELL_WISP_FLIGHT_PORT      42818
//#define SPELL_WISP_INVIS          42823
#define SPELL_SMOKE                 42355
#define SPELL_DEATH                 42566       //not correct spell

struct Locations
{
    float x, y, z;
};

static Locations FlightPoint[]=
{
    {1754.00f,1346.00f,17.50f},
    {1765.00f,1347.00f,19.00f},
    {1784.00f,1346.80f,25.40f},
    {1803.30f,1347.60f,33.00f},
    {1824.00f,1350.00f,42.60f},
    {1838.80f,1353.20f,49.80f},
    {1852.00f,1357.60f,55.70f},
    {1861.30f,1364.00f,59.40f},
    {1866.30f,1374.80f,61.70f},
    {1864.00f,1387.30f,63.20f},
    {1854.80f,1399.40f,64.10f},
    {1844.00f,1406.90f,64.10f},
    {1824.30f,1411.40f,63.30f},
    {1801.00f,1412.30f,60.40f},
    {1782.00f,1410.10f,55.50f},
    {1770.50f,1405.20f,50.30f},
    {1765.20f,1400.70f,46.60f},
    {1761.40f,1393.40f,41.70f},
    {1759.10f,1386.70f,36.60f},
    {1757.80f,1378.20f,29.00f},
    {1758.00f,1367.00f,19.51f}
};

static Locations Spawn[]=
{
    {1776.27f, 1348.74f, 19.20f},    //spawn point for pumpkin shrine mob
    {1765.28f ,1347.46f, 17.55f}     //spawn point for smoke
};

struct Summon
{
    const std::string text;
};

static Summon Text[]=
{
    {"Horseman rise..."},
    {"Your time is nigh..."},
    {"You felt death once..."},
    {"Now, know demise!"}
};

struct mob_wisp_invisAI : public ScriptedAI
{
    mob_wisp_invisAI(Creature *c) : ScriptedAI(c)
    {
        Creaturetype = delay = spell = spell2 = 0;
        //that's hack but there are no info about range of this spells in dbc
        SpellEntry *wisp = (SpellEntry*)GetSpellStore()->LookupEntry(SPELL_WISP_BLUE);
        if (wisp)
            wisp->rangeIndex = 6; //100 yards
        SpellEntry *port = (SpellEntry*)GetSpellStore()->LookupEntry(SPELL_WISP_FLIGHT_PORT);
        if (port)
            port->rangeIndex = 6;
    }

    uint32 Creaturetype;
    uint32 delay;
    uint32 spell;
    uint32 spell2;
    void Reset() {}
    void EnterCombat(Unit *who) {}
    void SetType(uint32 _type)
    {
        Creaturetype = _type;
        switch (Creaturetype)
        {
            case 1:
                spell = SPELL_PUMPKIN_AURA_GREEN; break;
            case 2:
                delay = 15000;  spell = SPELL_BODY_FLAME; spell2 = SPELL_DEATH; break;
            case 3:
                delay = 15000;  spell = SPELL_SMOKE; break;
            case 4:
                delay = 7000;   spell2 = SPELL_WISP_BLUE; break;
        }
        if (spell)DoCast(m_creature, spell);
    }

    void SpellHit(Unit* caster, const SpellEntry *spell)
    {
        if (spell->Id == SPELL_WISP_FLIGHT_PORT && Creaturetype == 4)
            m_creature->SetDisplayId(2027);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!who || Creaturetype != 1 || !who->isTargetableForAttack())
            return;

        if (m_creature->GetDistance2d(who) < 0.1 && !who->HasAura(SPELL_SQUASH_SOUL, 0))
            DoCast(who, SPELL_SQUASH_SOUL);
    }

    void UpdateAI(const uint32 diff)
    {
        if (delay)    // delay = 51, diff = 51, delay - diff = 0, spell will be not cast, aura will be not removed, gratz
        {
            delay -= diff;
            if (delay <= diff)
            {
                m_creature->RemoveAurasDueToSpell(SPELL_SMOKE);
                if (spell2)
                    DoCast(m_creature, spell2);
                delay = 0;
            }
        }
    }
};

struct mob_headAI : public ScriptedAI
{
    mob_headAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    uint64 bodyGUID;
    ScriptedInstance *pInstance;

    uint32 Phase;
    Timer laugh;
    Timer wait;

    bool withbody;
    bool die;

    void Reset()
    {
        Phase = 0;
        bodyGUID = 0;
        die = false;
        withbody = true;
        wait.Reset(1000);
        laugh.Reset(15000 + rand() % 16 * 1000);
    }

    void EnterCombat(Unit *who) {}
    void SaySound(int32 textEntry, Unit *target = 0)
    {
        DoScriptText(textEntry, m_creature, target);
        //DoCast(m_creature,SPELL_HEAD_SPEAKS,true);
        Creature *speaker = DoSpawnCreature(HELPER, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 1000);
        if (speaker)
            speaker->CastSpell(speaker, SPELL_HEAD_SPEAKS, false);
        laugh = 3000;
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        if (withbody)
            return;

        switch (Phase)
        {
            case 1:
                if (((m_creature->GetHealth() - damage) * 100) / m_creature->GetMaxHealth() < 67)
                    Disappear();
                break;
            case 2:
                if (((m_creature->GetHealth() - damage) * 100) / m_creature->GetMaxHealth() < 34)
                    Disappear();
                break;
            case 3:
                if (damage >= m_creature->GetHealth())
                {
                    die = true;
                    withbody = true;
                    wait = 300;
                    m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    m_creature->StopMoving();
                    //m_creature->GetMotionMaster()->MoveIdle();
                    DoCast(m_creature, SPELL_HEAD_IS_DEAD);
                }break;
        }
    }
    void JustDied(Unit *killer)
    {
        if (!pInstance)
            return;

        if (Creature* horseman = Creature::GetCreature((*m_creature), pInstance->GetData64(DATA_HORSEMAN_EVENT)))
            horseman->DealDamage(horseman, horseman->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
    }

    void SpellHit(Unit *caster, const SpellEntry* spell)
    {
        if (!withbody)
            return;

        if (spell->Id == SPELL_FLYING_HEAD)
        {
            if (Phase < 3) ++Phase;
            else Phase = 3;
            withbody = false;
            if (!bodyGUID)
                bodyGUID = caster->GetGUID();
            m_creature->RemoveAllAuras();
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            DoCast(m_creature, SPELL_HEAD_LANDS, true);
            DoCast(m_creature, SPELL_HEAD, false);
            SaySound(SAY_LOST_HEAD);
            m_creature->GetMotionMaster()->Clear(false);
            m_creature->GetMotionMaster()->MoveFleeing(caster->getVictim());
        }
    }
    void Disappear();//we must set returned=true(this will prevent from "body calls head" while head flying to body), see function below
    void UpdateAI(const uint32 diff)
    {
        if (!withbody)
        {
            if (wait.Expired(diff))
            {
                wait = 1000;
                if (!m_creature->getVictim())
                    return;

                m_creature->GetMotionMaster()->Clear(false);
                m_creature->GetMotionMaster()->MoveFleeing(m_creature->getVictim());
            }


            if (laugh.Expired(diff))
            {
                laugh = 15000 + (rand() % 16) * 1000;
                DoPlaySoundToSet(m_creature, RandomLaugh[rand() % 3]);
                //DoCast(m_creature,SPELL_HEAD_SPEAKS,true); //this spell remove buff "head"
                Creature *speaker = DoSpawnCreature(HELPER, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 1000);
                if (speaker)
                    speaker->CastSpell(speaker, SPELL_HEAD_SPEAKS, false);
                DoTextEmote("laughs", NULL);
            }


        }
        else
        {
            if (die)
            {
                if (wait.Expired(diff))
                {
                    die = false;
                    Unit *body = Unit::GetUnit((*m_creature), bodyGUID);
                    if (body)
                        body->DealDamage(body, body->GetMaxHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                    m_creature->setDeathState(JUST_DIED);
                }
            }
        }
    }
};

struct boss_headless_horsemanAI : public ScriptedAI
{
    boss_headless_horsemanAI(Creature *c) : ScriptedAI(c)
    {
        SpellEntry *confl = (SpellEntry*)GetSpellStore()->LookupEntry(SPELL_CONFLAGRATION);
        if (confl)
        {
            confl->EffectApplyAuraName[0] = SPELL_AURA_PERIODIC_DAMAGE_PERCENT;
            confl->EffectBasePoints[0] = 10;
            confl->EffectBaseDice[0] = 10;
            confl->DmgMultiplier[0] = 1;
        }
        /*      SpellEntry *confl = (SpellEntry*)GetSpellStore()->LookupEntry(SPELL_CONFLAGRATION);
                if(confl)
                confl->EffectTriggerSpell[1] = 22587;

                SpellEntry *speed = (SpellEntry*)GetSpellStore()->LookupEntry(22587);
                if(speed)
                {
                speed->Effect[1] = SPELL_EFFECT_APPLY_AURA;
                speed->EffectApplyAuraName[1] = SPELL_AURA_MOD_CONFUSE;
                }*/
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    uint64 headGUID;
    uint64 playerGUID;

    uint32 Phase;
    uint32 id;
    uint32 count;
    Timer say_timer;

    Timer conflagrate;
    Timer summonadds;
    Timer cleave;
    Timer regen;
    Timer whirlwind;
    Timer laugh;
    Timer burn;

    bool withhead;
    bool returned;
    bool IsFlying;
    bool wp_reached;
    bool burned;

    void Reset()
    {
        Phase = 1;
        conflagrate.Reset(10000);
        summonadds.Reset(10000);
        laugh.Reset(16000 + rand() % 5 * 1000);
        cleave.Reset(2000);
        regen.Reset(1000);
        burn.Reset(6000);
        count = 0;
        say_timer.Reset(3000);

        withhead = true;
        returned = true;
        burned = false;
        IsFlying = false;
        DoCast(m_creature, SPELL_HEAD);
        if (headGUID)
        {
            Unit* Head = Unit::GetUnit((*m_creature), headGUID);
            if (Head)
            {
                Head->SetVisibility(VISIBILITY_OFF);
                Head->setDeathState(JUST_DIED);
            }
            headGUID = 0;
        }

        if (pInstance)
            pInstance->SetData(DATA_HORSEMAN_EVENT, 0);
    }

    void FlyMode()
    {
        m_creature->SetVisibility(VISIBILITY_OFF);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetLevitate(true);
        m_creature->SetSpeed(MOVE_WALK, 5.0f, true);
        wp_reached = false;
        count = 0;
        say_timer = 3000;
        id = 0;
        Phase = 0;
        IsFlying = false;
    }

    void MovementInform(uint32 type, uint32 i)
    {
        if (type != POINT_MOTION_TYPE || !IsFlying)
            return;

        if (i != id)
            return;

        wp_reached = true;

        switch (id)
        {
            case 0:
                m_creature->SetVisibility(VISIBILITY_ON); break;
            case 1:
            {
                Creature *smoke = m_creature->SummonCreature(HELPER, Spawn[1].x, Spawn[1].y, Spawn[1].z, 0, TEMPSUMMON_TIMED_DESPAWN, 20000);
                if (smoke)
                    ((mob_wisp_invisAI*)smoke->AI())->SetType(3);
                DoCast(m_creature, SPELL_RHYME_BIG);
                break;
            }
            case 6:
                if (pInstance)
                    pInstance->SetData(GAMEOBJECT_PUMPKIN_SHRINE, 0);   //hide gameobject
                break;
            case 19:
                m_creature->SetLevitate(false); break;
            case 20:
            {
                Phase = 1;
                IsFlying = false;
                wp_reached = false;
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                SaySound(SAY_ENTRANCE);
                Unit *plr = Unit::GetUnit((*m_creature), playerGUID);
                if (plr)
                    AttackStart(plr);
                break;
            }
        }
        ++id;
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(DATA_HORSEMAN_EVENT, IN_PROGRESS);
        DoZoneInCombat();
    }
    void AttackStart(Unit* who) { ScriptedAI::AttackStart(who); }
    void MoveInLineOfSight(Unit *who)
    {
        if (withhead && Phase != 0)
            ScriptedAI::MoveInLineOfSight(who);
    }
    void KilledUnit(Unit *plr)
    {
        if (plr->GetTypeId() == TYPEID_PLAYER)
        {
            if (withhead)
                SaySound(SAY_PLAYER_DEATH);
            else
            {          //maybe possible when player dies from conflagration
                Creature *Head = Unit::GetCreature((*m_creature), headGUID);
                if (Head)
                    ((mob_headAI*)Head->AI())->SaySound(SAY_PLAYER_DEATH);
            }
        }
    }

    void SaySound(int32 textEntry, Unit *target = 0)
    {
        DoScriptText(textEntry, m_creature, target);
        laugh = 4000;
    }

    Player* SelectRandomPlayer(float range = 0.0f, bool checkLoS = true)
    {
        Map *map = m_creature->GetMap();
        if (!map->IsDungeon()) return NULL;

        Map::PlayerList const &PlayerList = map->GetPlayers();
        Map::PlayerList::const_iterator i;
        if (PlayerList.isEmpty())
            return NULL;

        std::list<Player*> temp;
        std::list<Player*>::iterator j;

        for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            if ((m_creature->IsWithinLOSInMap(i->getSource()) || !checkLoS) && m_creature->getVictim() != i->getSource() &&
                m_creature->IsWithinDistInMap(i->getSource(), range) && i->getSource()->isAlive() && !i->getSource()->isGameMaster())
                temp.push_back(i->getSource());

        if (temp.size())
        {
            j = temp.begin();
            advance(j, rand() % temp.size());
            return (*j);
        }
        return NULL;
    }

    void SpellHitTarget(Unit* unit, const SpellEntry* spell)
    {
        if (spell->Id == SPELL_CONFLAGRATION)
            if (unit->HasAura(SPELL_CONFLAGRATION, 0))
                SaySound(SAY_CONFLAGRATION, unit);
    }

    void JustDied(Unit* killer)
    {
        m_creature->StopMoving();
        //m_creature->GetMotionMaster()->MoveIdle();    test
        SaySound(SAY_DEATH);
        Creature *flame = DoSpawnCreature(HELPER, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 60000);
        if (flame)
            flame->CastSpell(flame, SPELL_BODY_FLAME, false);
        Creature *wisp = DoSpawnCreature(WISP_INVIS, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 60000);
        if (wisp)
            ((mob_wisp_invisAI*)wisp->AI())->SetType(4);
        if (pInstance)
            pInstance->SetData(DATA_HORSEMAN_EVENT, DONE);
    }

    void SpellHit(Unit *caster, const SpellEntry* spell)
    {
        if (withhead)
            return;

        if (spell->Id == SPELL_FLYING_HEAD)
        {
            if (Phase < 3)
                Phase++;
            else
                Phase = 3;

            withhead = true;
            m_creature->RemoveAllAuras();
            m_creature->SetName("Headless Horseman");
            m_creature->SetHealth(m_creature->GetMaxHealth());
            SaySound(SAY_REJOINED);
            DoCast(m_creature, SPELL_HEAD);
            caster->GetMotionMaster()->Clear(false);
            caster->GetMotionMaster()->MoveFollow(m_creature, 6, rand() % 6);
            //DoResetThreat();//not sure if need
            std::list<HostileReference*>::iterator itr;
            for (itr = caster->getThreatManager().getThreatList().begin(); itr != caster->getThreatManager().getThreatList().end(); ++itr)
            {
                Unit* pUnit = Unit::GetUnit((*m_creature), (*itr)->getUnitGuid());
                if (pUnit && pUnit->isAlive() && pUnit != caster)
                    m_creature->AddThreat(pUnit, caster->getThreatManager().getThreat(pUnit));
            }
        }
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (damage >= m_creature->GetHealth() && withhead)
        {
            withhead = false;
            returned = false;
            damage = 0;
            m_creature->SetHealth(m_creature->GetMaxHealth() / 100);
            m_creature->RemoveAllAuras();
            m_creature->SetName("Headless Horseman, Unhorsed");

            if (!headGUID)
                headGUID = DoSpawnCreature(HEAD, rand() % 6, rand() % 6, 0, 0, TEMPSUMMON_DEAD_DESPAWN, 0)->GetGUID();
            Unit* Head = Unit::GetUnit((*m_creature), headGUID);
            if (Head && Head->isAlive())
            {
                Head->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                //Head->CastSpell(Head,SPELL_HEAD_INVIS,false);
                m_creature->InterruptNonMeleeSpells(false);
                DoCast(m_creature, SPELL_IMMUNE, true);
                DoCast(m_creature, SPELL_BODY_REGEN, true);
                m_creature->CastSpell(Head, SPELL_FLYING_HEAD, true);
                DoCast(m_creature, SPELL_CONFUSE, false);                     //test
                done_by->ProcDamageAndSpell(m_creature, PROC_FLAG_KILL_AND_GET_XP, PROC_FLAG_KILLED, PROC_EX_NONE, 0);
                whirlwind = 4000 + (rand() % 5) * 1000;
                regen = 0;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (withhead)
        {
            switch (Phase)
            {
                case 0: {
                    if (!IsFlying)
                    {
                        if (say_timer.Expired(diff))
                        {
                            say_timer = 3000;
                            Player *plr = SelectRandomPlayer(100.0f, false);
                            if (count < 3)
                            {
                                if (plr)
                                    plr->Say(Text[count].text, 0);
                            }
                            else
                            {
                                DoCast(m_creature, SPELL_RHYME_BIG);
                                if (plr)
                                {
                                    plr->Say(Text[count].text, 0);
                                    plr->HandleEmoteCommand(ANIM_EMOTE_SHOUT);
                                }
                                wp_reached = true;
                                IsFlying = true;
                                count = 0;
                                break;
                            }
                            ++count;
                        }
                    }
                    else
                    {
                        if (wp_reached)
                        {
                            wp_reached = false;
                            m_creature->GetMotionMaster()->Clear(false);
                            m_creature->GetMotionMaster()->MovePoint(id, FlightPoint[id].x, FlightPoint[id].y, FlightPoint[id].z);
                        }
                    }
                }
                        break;
                case 1:
                    if (burned)
                        break;


                    if (burn.Expired(diff))
                    {
                        Creature *flame = m_creature->SummonCreature(HELPER, Spawn[0].x, Spawn[0].y, Spawn[0].z, 0, TEMPSUMMON_TIMED_DESPAWN, 17000);
                        if (flame)
                            ((mob_wisp_invisAI*)flame->AI())->SetType(2);
                        burned = true;
                    }

                    break;
                case 2:
                    if (conflagrate.Expired(diff))
                    {
                        Unit *plr = SelectUnit(SELECT_TARGET_RANDOM, 1, 30, true, m_creature->getVictimGUID());
                        if (!plr)
                            plr = m_creature->getVictim();

                        if (plr)
                            m_creature->CastSpell(plr, SPELL_CONFLAGRATION, false);
                        conflagrate = 10000 + rand() % 7 * 1000;
                    }

                    break;
                case 3:
                    if (summonadds.Expired(diff))
                    {
                        m_creature->InterruptNonMeleeSpells(false);
                        DoCast(m_creature, SPELL_SUMMON_PUMPKIN);
                        SaySound(SAY_SPROUTING_PUMPKINS);
                        summonadds = 25000 + rand() % 11 * 1000;
                    }

                    break;
            }

            if (laugh.Expired(diff))
            {
                laugh = 11000 + rand() % 12 * 1000;
                DoTextEmote("laughs", NULL);
                DoPlaySoundToSet(m_creature, RandomLaugh[rand() % 3]);
            }


            if (UpdateVictim())
            {
                DoMeleeAttackIfReady();
                if (cleave.Expired(diff))
                {
                    DoCast(m_creature->getVictim(), SPELL_CLEAVE);
                    cleave = 2000 + rand() % 4000;       //1 cleave per 2.0-6.0sec
                }

            }

        }
        else
        {
            if (regen.Expired(diff))
            {
                regen = 1000;                   //"body calls head"
                if (m_creature->GetHealth() / m_creature->GetMaxHealth() == 1 && !returned)
                {
                    if (Phase > 1)
                        --Phase;
                    else
                        Phase = 1;
                    Creature* Head = Unit::GetCreature((*m_creature), headGUID);
                    if (Head && Head->isAlive())
                    {
                        ((mob_headAI*)Head->AI())->Phase = Phase;
                        ((mob_headAI*)Head->AI())->Disappear();
                    }
                    return;
                }
            }


            if (whirlwind.Expired(diff))
            {
                whirlwind = 4000 + rand() % 5 * 1000;
                if (rand() % 2)
                {
                    m_creature->RemoveAurasDueToSpell(SPELL_CONFUSE);
                    DoCast(m_creature, SPELL_WHIRLWIND, true);
                    DoCast(m_creature, SPELL_CONFUSE);
                }
                else
                    m_creature->RemoveAurasDueToSpell(SPELL_WHIRLWIND);
            }

        }
    }
};

void mob_headAI::Disappear()
{
    if (withbody)
        return;
    if (bodyGUID)
    {
        Creature *body = Unit::GetCreature((*m_creature), bodyGUID);
        if (body && body->isAlive())
        {
            withbody = true;
            m_creature->RemoveAllAuras();
            body->RemoveAurasDueToSpell(SPELL_IMMUNE);//hack, SpellHit doesn't calls if body has immune aura
            DoCast(body, SPELL_FLYING_HEAD);
            m_creature->SetHealth(m_creature->GetMaxHealth());
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            m_creature->GetMotionMaster()->MoveIdle();
            ((boss_headless_horsemanAI*)body->AI())->returned = true;
        }
    }
}

struct mob_pulsing_pumpkinAI : public ScriptedAI
{
    mob_pulsing_pumpkinAI(Creature *c) : ScriptedAI(c) {}

    bool sprouted;
    uint64 debuffGUID;

    void Reset()
    {
        float x, y, z;
        m_creature->GetPosition(x, y, z);   //this visual aura some under ground
        m_creature->Relocate(x, y, z + 0.35f);
        Despawn();
        Creature *debuff = DoSpawnCreature(HELPER, 0, 0, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 14500);
        if (debuff)
        {
            debuff->SetDisplayId(m_creature->GetDisplayId());
            debuff->CastSpell(debuff, SPELL_PUMPKIN_AURA_GREEN, false);
            ((mob_wisp_invisAI*)debuff->AI())->SetType(1);
            debuffGUID = debuff->GetGUID();
        }
        sprouted = false;
        DoCast(m_creature, SPELL_PUMPKIN_AURA, true);
        DoCast(m_creature, SPELL_SPROUTING);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
    }

    void EnterCombat(Unit *who) {}

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if (spell->Id == SPELL_SPROUTING)
        {
            sprouted = true;
            m_creature->RemoveAllAuras();
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
            DoCast(m_creature, SPELL_SPROUT_BODY, true);
            m_creature->UpdateEntry(PUMPKIN_FIEND);
            DoStartMovement(m_creature->getVictim());
        }
    }

    void Despawn()
    {
        if (!debuffGUID) return;
        Unit *debuff = Unit::GetUnit((*m_creature), debuffGUID);
        if (debuff)
            debuff->SetVisibility(VISIBILITY_OFF);
        debuffGUID = 0;
    }

    void JustDied(Unit *killer) { if (!sprouted) Despawn(); }

    void MoveInLineOfSight(Unit *who)
    {
        if (!who || !who->isTargetableForAttack() || !m_creature->IsHostileTo(who) || m_creature->getVictim())
            return;

        m_creature->AddThreat(who, 0.0f);
        if (sprouted)
            DoStartMovement(who);
    }

    void UpdateAI(const uint32 diff)
    {
        if (sprouted)
            if (UpdateVictim())
                DoMeleeAttackIfReady();
    }
};

bool GOUse_go_loosely_turned_soil(Player *plr, GameObject* soil)
{
    /*  if (soil->GetGoType() == GAMEOBJECT_TYPE_QUESTGIVER && plr->getLevel() > 64)
        {
        plr->PrepareQuestMenu(soil->GetGUID());
        plr->SendPreparedQuest(soil->GetGUID());
        }
        if (plr->GetQuestStatus(11405) == QUEST_STATUS_INCOMPLETE && plr->getLevel() > 64)
        { */
    plr->AreaExploredOrEventHappens(11405);
    Creature *horseman = soil->SummonCreature(HH_MOUNTED, FlightPoint[20].x, FlightPoint[20].y, FlightPoint[20].z, 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
    horseman->setActive(true);
    if (horseman)
    {
        ((boss_headless_horsemanAI*)horseman->AI())->playerGUID = plr->GetGUID();
        ((boss_headless_horsemanAI*)horseman->AI())->FlyMode();
    }
    //}
    return true;
}
bool GOReward_go_loosely_turned_soil(Player *plr, GameObject* soil, Quest const* quest)
{
    if (quest->GetQuestId() == 11392 || quest->GetQuestId() == 11405)
    {
        Creature *horseman = soil->SummonCreature(HH_MOUNTED, FlightPoint[20].x, FlightPoint[20].y, FlightPoint[20].z, 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
        horseman->setActive(true);
        if (horseman)
        {
            ((boss_headless_horsemanAI*)horseman->AI())->playerGUID = plr->GetGUID();
            ((boss_headless_horsemanAI*)horseman->AI())->FlyMode();
            soil->SetUInt32Value(GAMEOBJECT_FACTION, 14);
        }
    }
    return true;
}
CreatureAI* GetAI_mob_head(Creature *_Creature)
{
    return new mob_headAI(_Creature);
}

CreatureAI* GetAI_boss_headless_horseman(Creature *_Creature)
{
    return new boss_headless_horsemanAI(_Creature);
}

CreatureAI* GetAI_mob_pulsing_pumpkin(Creature *_Creature)
{
    return new mob_pulsing_pumpkinAI(_Creature);
}

CreatureAI* GetAI_mob_wisp_invis(Creature *_Creature)
{
    return new mob_wisp_invisAI(_Creature);
}

void AddSC_boss_headless_horseman()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_headless_horseman";
    newscript->GetAI = &GetAI_boss_headless_horseman;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_head";
    newscript->GetAI = &GetAI_mob_head;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_pulsing_pumpkin";
    newscript->GetAI = &GetAI_mob_pulsing_pumpkin;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_wisp_invis";
    newscript->GetAI = &GetAI_mob_wisp_invis;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_loosely_turned_soil";
    //newscript->pGOUse = &GOUse_go_loosely_turned_soil;
    newscript->pQuestRewardedGO = &GOReward_go_loosely_turned_soil;
    newscript->RegisterSelf();
}

