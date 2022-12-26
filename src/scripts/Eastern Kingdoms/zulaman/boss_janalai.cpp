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
SDName: Boss_Janalai
SD%Complete: 100
SDComment:
SDCategory: Zul'Aman
EndScriptData */

#include "scriptPCH.h"
#include "def_zulaman.h"
#include "GridNotifiers.h"

#define SAY_AGGRO                   -1568000
#define SAY_FIRE_BOMBS              -1568001
#define SAY_SUMMON_HATCHER          -1568002
#define SAY_ALL_EGGS                -1568003
#define SAY_BERSERK                 -1568004
#define SAY_SLAY_1                  -1568005
#define SAY_SLAY_2                  -1568006
#define SAY_DEATH                   -1568007
#define SAY_EVENT_STRANGERS         -1568008        // aka INTRO1
#define SAY_EVENT_FRIENDS           -1568009        // aka INTRO2

// Jan'alai
// --Spell
#define SPELL_FLAME_BREATH          43140
#define SPELL_FIRE_WALL             43113
#define SPELL_ENRAGE                44779
#define SPELL_SUMMON_PLAYERS        43097
#define SPELL_TELE_TO_CENTER        43098 // coord
#define SPELL_HATCH_ALL             43144
#define SPELL_BERSERK               45078
// -- Fire Bob Spells
#define SPELL_FIRE_BOMB_CHANNEL     42621 // last forever
#define SPELL_FIRE_BOMB_THROW       42628 // throw visual
#define SPELL_FIRE_BOMB_DUMMY       42629 // bomb visual
#define SPELL_FIRE_BOMB_DAMAGE      42630

// --Summons
#define MOB_AMANI_HATCHER           23818
#define MOB_HATCHLING               23598   // 42493
#define MOB_EGG                     23817
#define MOB_FIRE_BOMB               23920

// -- Hatcher Spells
#define SPELL_HATCH_EGG             43734   // 42471

// -- Hatchling Spells
#define SPELL_FLAMEBUFFET           43299

const int area_dx = 44;
const int area_dy = 51;

static float JanalainPos[1][3] =
{
    {-33.93f, 1149.27f, 19}
};

static float FireWallCoords[4][4] =
{
    {-10.13f, 1149.27f, 19, 3.1415f},
    {-33.93f, 1123.90f, 19, 0.5f*3.1415f},
    {-54.80f, 1150.08f, 19, 0},
    {-33.93f, 1175.68f, 19, 1.5f*3.1415f}
};

static float hatcherway[2][5][3] =
{
    {
        {-87.46f,1170.09f,6},
        {-74.41f,1154.75f,6},
        {-52.74f,1153.32f,19},
        {-33.37f,1172.46f,19},
        {-33.09f,1203.87f,19}
    },
    {
        {-86.57f,1132.85f,6},
        {-73.94f,1146.00f,6},
        {-52.29f,1146.51f,19},
        {-33.57f,1125.72f,19},
        {-34.29f,1095.22f,19}
    }
};

struct boss_janalaiAI : public ScriptedAI
{
    boss_janalaiAI(Creature *c) : ScriptedAI(c)
    {
        pInstance =(c->GetInstanceData());

        SpellEntry *TempSpell = (SpellEntry*)GetSpellStore()->LookupEntry(SPELL_HATCH_EGG);
        if(TempSpell && TempSpell->EffectImplicitTargetA[0] != 1)
        {
            TempSpell->EffectImplicitTargetA[0] = 1;
            TempSpell->EffectImplicitTargetB[0] = 0;
        }
        wLoc.coord_x = -33.93;
        wLoc.coord_y = 1149.27;
        wLoc.coord_z = 19;
        wLoc.mapid = c->GetMapId();
    }

    ScriptedInstance *pInstance;

    WorldLocation wLoc;

    Timer FireBreathTimer;
    Timer BombTimer;
    Timer BombSequenceTimer;
    uint32 BombCount;
    Timer HatcherTimer;
    Timer EnrageTimer;
    Timer ResetTimer;

    bool noeggs;
    bool enraged;
    bool isBombing;

    bool isFlameBreathing;

    uint64 FireBombGUIDs[40];

    Timer checkTimer;
    bool Intro;

    void Reset()
    {
        if(pInstance && pInstance->GetData(DATA_JANALAIEVENT) != DONE)
            pInstance->SetData(DATA_JANALAIEVENT, NOT_STARTED);

        FireBreathTimer.Reset(8000);
        BombTimer.Reset(30000);
        BombSequenceTimer.Reset(1000);
        BombCount = 0;
        HatcherTimer.Reset(10000);
        EnrageTimer.Reset(300000);
        ResetTimer.Reset(5000);

        noeggs = false;
        isBombing =false;
        enraged = false;

        isFlameBreathing = false;

        for(uint8 i = 0; i < 40; i++)
            FireBombGUIDs[i] = 0;

        HatchAllEggs(1);

        checkTimer.Reset(3000);
        Intro = false;
    }

    void MoveInLineOfSight(Unit *who)
    {
        if(!Intro && me->IsHostileTo(who) && who->IsWithinDist(me, 20, false))
        {
            Intro = true;
            DoScriptText(RAND(SAY_EVENT_FRIENDS, SAY_EVENT_STRANGERS), m_creature);
        }
        CreatureAI::MoveInLineOfSight(who);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, m_creature);

        if(pInstance)
            pInstance->SetData(DATA_JANALAIEVENT, DONE);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), m_creature);
    }

    void EnterCombat(Unit *who)
    {
        if(pInstance)
            pInstance->SetData(DATA_JANALAIEVENT, IN_PROGRESS);

        DoScriptText(SAY_AGGRO, m_creature);
//        DoZoneInCombat();
    }

    void DamageDeal(Unit* target, uint32 &damage)
    {
        if(isFlameBreathing)
        {
            if(!m_creature->HasInArc(M_PI/6, target))
                damage = 0;
        }
    }

    void FireWall()
    {
        uint8 WallNum;
        Creature* wall = NULL;
        for(uint8 i = 0; i < 4; i++)
        {
            if(i == 0 || i == 2)
                WallNum = 3;
            else
                WallNum = 2;

            for(uint8 j = 0; j < WallNum; j++)
            {
                if(WallNum == 3)
                    wall = m_creature->SummonCreature(MOB_FIRE_BOMB, FireWallCoords[i][0],FireWallCoords[i][1]+5*(j-1),FireWallCoords[i][2],FireWallCoords[i][3],TEMPSUMMON_TIMED_DESPAWN,15000);
                else
                    wall = m_creature->SummonCreature(MOB_FIRE_BOMB, FireWallCoords[i][0]-2+4*j,FireWallCoords[i][1],FireWallCoords[i][2],FireWallCoords[i][3],TEMPSUMMON_TIMED_DESPAWN,15000);
                if(wall) wall->CastSpell(wall, SPELL_FIRE_WALL, true);
            }
        }
    }

    void SpawnBombs()
    {
        float dx, dy;
        for( int i(0); i < 40; i++)
        {
            dx =(rand()%(area_dx))-(area_dx/2);
            dy =(rand()%(area_dy))-(area_dy/2);

            Creature* bomb = DoSpawnCreature(MOB_FIRE_BOMB, dx, dy, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 15000);
            if(bomb) FireBombGUIDs[i] = bomb->GetGUID();
        }
        BombCount = 0;
    }

    bool HatchAllEggs(uint32 action) //1: reset, 2: isHatching all
    {
        std::list<Creature*> templist;
        float x, y, z;
        m_creature->GetPosition(x, y, z);

        {
            MaNGOS::AllCreaturesOfEntryInRange check(m_creature, MOB_EGG, 100);
            MaNGOS::ObjectListSearcher<Creature, MaNGOS::AllCreaturesOfEntryInRange> searcher(templist, check);

            Cell::VisitGridObjects(me, searcher, 100);
        }

        //error_log("Eggs %d at middle", templist.size());
        if(!templist.size())
            return false;

        for(std::list<Creature*>::iterator i = templist.begin(); i != templist.end(); ++i)
        {
            if(action == 1)
               (*i)->SetDisplayId(10056);
            else if(action == 2 &&(*i)->GetDisplayId() != 11686)
               (*i)->CastSpell(*i, SPELL_HATCH_EGG, false);
        }
        return true;
    }

    void Boom()
    {
        std::list<Creature*> templist;
        float x, y, z;
        m_creature->GetPosition(x, y, z);

        {
            MaNGOS::AllCreaturesOfEntryInRange check(m_creature, MOB_FIRE_BOMB, 100);
            MaNGOS::ObjectListSearcher<Creature, MaNGOS::AllCreaturesOfEntryInRange> searcher(templist, check);

            Cell::VisitGridObjects(me, searcher, me->GetMap()->GetVisibilityDistance());
        }
        for(std::list<Creature*>::iterator i = templist.begin(); i != templist.end(); ++i)
        {
           (*i)->CastSpell(*i, SPELL_FIRE_BOMB_DAMAGE, true);
           (*i)->RemoveAllAuras();
        }
    }

    void HandleBombSequence()
    {
        if(BombCount < 40)
        {
            if(Unit *FireBomb = Unit::GetUnit((*m_creature), FireBombGUIDs[BombCount]))
            {
                FireBomb->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                DoCast(FireBomb, SPELL_FIRE_BOMB_THROW, true);
                FireBomb->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }
            BombCount++;
            if(BombCount == 40)
            {
                BombSequenceTimer = 5000;
            }else BombSequenceTimer = 100;
        }
        else
        {
            Boom();
            isBombing = false;
            BombTimer = 20000+rand()%20000;
            m_creature->RemoveAurasDueToSpell(SPELL_FIRE_BOMB_CHANNEL);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(isFlameBreathing)
        {
            if(!m_creature->IsNonMeleeSpellCast(false))
            {
                isFlameBreathing = false;
            }
            else 
            {
                if (EnrageTimer.Expired(diff))
                    EnrageTimer = 0;

                if (HatcherTimer.Expired(diff))
                    HatcherTimer = 0;
                return;
            }
        }

        if(isBombing)
        {
            if (BombSequenceTimer.Expired(diff))
                HandleBombSequence();

    
            if (EnrageTimer.Expired(diff))
                EnrageTimer = 0;
            if (HatcherTimer.Expired(diff))
                HatcherTimer = 0;
            return;
        }

        if(!UpdateVictim())
            return;


        if (checkTimer.Expired(diff))
        {
            if (!m_creature->IsWithinDistInMap(&wLoc, 23))
                EnterEvadeMode();
            else
                DoZoneInCombat();
            checkTimer = 3000;
        }
        

        //enrage if under 25% hp before 5 min.
        if(!enraged && m_creature->GetHealth() * 4 < m_creature->GetMaxHealth())
            EnrageTimer = 0;

        if (EnrageTimer.Expired(diff))
        {
            if(!enraged)
            {
                m_creature->CastSpell(m_creature, SPELL_ENRAGE, true);
                enraged = true;
                EnrageTimer = 300000;
            }
            else
            {
                DoScriptText(SAY_BERSERK, m_creature);
                m_creature->CastSpell(m_creature, SPELL_BERSERK, true);
                EnrageTimer = 300000;
            }
        }

        if (BombTimer.Expired(diff))
        {
            DoScriptText(SAY_FIRE_BOMBS, m_creature);

            m_creature->AttackStop();
            m_creature->GetMotionMaster()->Clear();
            DoTeleportTo(JanalainPos[0][0],JanalainPos[0][1],JanalainPos[0][2]);
            m_creature->StopMoving();
            m_creature->CastSpell(m_creature, SPELL_FIRE_BOMB_CHANNEL, false);
            //DoTeleportPlayer(m_creature, JanalainPos[0][0], JanalainPos[0][1],JanalainPos[0][2], 0);
            //m_creature->CastSpell(m_creature, SPELL_TELE_TO_CENTER, true);

            FireWall();
            SpawnBombs();
            isBombing = true;
            BombSequenceTimer = 100;

            //Teleport every Player into the middle
            Map *map = m_creature->GetMap();
            if(!map->IsDungeon()) return;
            Map::PlayerList const &PlayerList = map->GetPlayers();
            for(Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            {
                if (Player* i_pl = i->getSource())
                    if(i_pl->IsAlive() && !i_pl->IsInRange(m_creature, 0.0f, 20.0f))
                        DoTeleportPlayer(i_pl, JanalainPos[0][0]-5+rand()%10, JanalainPos[0][1]-5+rand()%10, JanalainPos[0][2], 0);
            }
            //m_creature->CastSpell(Temp, SPELL_SUMMON_PLAYERS, true); // core bug, spell does not work if too far
            return;
        }

        if(!noeggs)
        {

            if(100 * m_creature->GetHealth() < 35 * m_creature->GetMaxHealth())
            {
                DoScriptText(SAY_ALL_EGGS, m_creature);

                m_creature->AttackStop();
                m_creature->GetMotionMaster()->Clear();
                DoTeleportTo(JanalainPos[0][0],JanalainPos[0][1],JanalainPos[0][2]);
                m_creature->StopMoving();
                m_creature->CastSpell(m_creature, SPELL_HATCH_ALL, false);
                HatchAllEggs(2);
                noeggs = true;
            }
            else
            {
                if (HatcherTimer.Expired(diff) || !HatcherTimer.GetInterval())
                {
                    if (HatchAllEggs(0))
                    {
                        DoScriptText(SAY_SUMMON_HATCHER, m_creature);
                        m_creature->SummonCreature(MOB_AMANI_HATCHER, hatcherway[0][0][0], hatcherway[0][0][1], hatcherway[0][0][2], 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                        m_creature->SummonCreature(MOB_AMANI_HATCHER, hatcherway[1][0][0], hatcherway[1][0][1], hatcherway[1][0][2], 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                        HatcherTimer = 90000;
                    }
                    else
                        noeggs = true;
                }
            }
        }

        if (ResetTimer.Expired(diff))
        {
            float x, y, z, o;
            m_creature->GetHomePosition(x, y, z, o);
            if(m_creature->GetPositionZ() <= z-7)
            {
                EnterEvadeMode();
                return;
            }
            ResetTimer = 5000;
        }

        DoMeleeAttackIfReady();


        if (FireBreathTimer.Expired(diff))
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0, GetSpellMaxRange(SPELL_FLAME_BREATH), true))
            {
                m_creature->AttackStop();
                m_creature->GetMotionMaster()->Clear();
                m_creature->CastSpell(target, SPELL_FLAME_BREATH, false);
                m_creature->StopMoving();
                isFlameBreathing = true;
            }
            FireBreathTimer = 8000;
        }
    }
};

CreatureAI* GetAI_boss_janalaiAI(Creature *_Creature)
{
    return new boss_janalaiAI(_Creature);
}

struct mob_janalai_firebombAI : public ScriptedAI
{
    mob_janalai_firebombAI(Creature *c) : ScriptedAI(c){}

    void Reset() {}

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if(spell->Id == SPELL_FIRE_BOMB_THROW)
            m_creature->CastSpell(m_creature, SPELL_FIRE_BOMB_DUMMY, true);
    }

    void EnterCombat(Unit* who) {}

    void AttackStart(Unit* who) {}

    void MoveInLineOfSight(Unit* who) {}

    void UpdateAI(const uint32 diff) {}
};

CreatureAI* GetAI_mob_janalai_firebombAI(Creature *_Creature)
{
    return new mob_janalai_firebombAI(_Creature);
}

struct mob_amanishi_hatcherAI : public ScriptedAI
{
    mob_amanishi_hatcherAI(Creature *c) : ScriptedAI(c)
    {
        pInstance =(c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    uint32 waypoint;
    uint32 HatchNum;
    Timer WaitTimer;

    bool side;
    bool hasChangedSide;
    bool isHatching;

    void Reset()
    {
        side =(m_creature->GetPositionY() < 1150);
        waypoint = 0;
        isHatching = false;
        hasChangedSide = false;
        WaitTimer = 1;
        HatchNum = 0;
    }

    bool HatchEggs(uint32 num)
    {
        std::list<Creature*> templist;
        float x, y, z;
        m_creature->GetPosition(x, y, z);

        {
            MaNGOS::AllCreaturesOfEntryInRange check(m_creature, 23817, 50);
            MaNGOS::ObjectListSearcher<Creature, MaNGOS::AllCreaturesOfEntryInRange> searcher(templist, check);

            Cell::VisitGridObjects(me, searcher, 50);
        }

        //error_log("Eggs %d at %d", templist.size(), side);

        for(std::list<Creature*>::iterator i = templist.begin(); i != templist.end() && num > 0; ++i)
        {
            if((*i)->GetDisplayId() != 11686)
            {
               (*i)->CastSpell(*i, SPELL_HATCH_EGG, false);
                num--;
            }
        }

        if(num)
            return false;   // no more templist
        else
            return true;
    }

    void EnterCombat(Unit* who) {}
    void AttackStart(Unit*) {}
    void MoveInLineOfSight(Unit*) {}
    void MovementInform(uint32, uint32)
    {
        if(waypoint == 5)
        {
            isHatching = true;
            HatchNum = 1;
            WaitTimer = 5000;
        }
        else
            WaitTimer = 1;
    }

    void UpdateAI(const uint32 diff)
    {
        if(!pInstance || !(pInstance->GetData(DATA_JANALAIEVENT) == IN_PROGRESS))
        {
            m_creature->SetVisibility(VISIBILITY_OFF);
            m_creature->setDeathState(JUST_DIED);
            return;
        }

        if(!isHatching)
        {
            if(WaitTimer.GetInterval())
            {
                m_creature->GetMotionMaster()->Clear();
                m_creature->GetMotionMaster()->MovePoint(0,hatcherway[side][waypoint][0],hatcherway[side][waypoint][1],hatcherway[side][waypoint][2]);
                waypoint++;
                WaitTimer = 0;
            }
        }
        else
        {
            if (WaitTimer.Expired(diff))
            {
                if(HatchEggs(HatchNum))
                {
                    HatchNum++;
                    WaitTimer = 10000;
                }
                else if(!hasChangedSide)
                {
                    side = side ? 0 : 1;
                    isHatching = false;
                    waypoint = 3;
                    WaitTimer = 1;
                    hasChangedSide = true;
                }
                else
                {
                    m_creature->SetVisibility(VISIBILITY_OFF);
                    m_creature->setDeathState(JUST_DIED);
                }
            }
        }
    }
};

CreatureAI* GetAI_mob_amanishi_hatcherAI(Creature *_Creature)
{
    return new mob_amanishi_hatcherAI(_Creature);
}

struct mob_hatchlingAI : public ScriptedAI
{
    mob_hatchlingAI(Creature *c) : ScriptedAI(c)
    {
        pInstance =(c->GetInstanceData());
    }

    ScriptedInstance *pInstance;
    Timer BuffetTimer;

    void Reset()
    {
        BuffetTimer.Reset(7000);
        if(m_creature->GetPositionY() > 1150)
            m_creature->GetMotionMaster()->MovePoint(0, hatcherway[0][3][0]+rand()%4-2,1150+rand()%4-2,hatcherway[0][3][2]);
        else
            m_creature->GetMotionMaster()->MovePoint(0,hatcherway[1][3][0]+rand()%4-2,1150+rand()%4-2,hatcherway[1][3][2]);

        m_creature->SetLevitate(true);
    }

    void EnterCombat(Unit *who) {/*DoZoneInCombat();*/}

    void UpdateAI(const uint32 diff)
    {
        if(!pInstance || !(pInstance->GetData(DATA_JANALAIEVENT) == IN_PROGRESS))
        {
            m_creature->SetVisibility(VISIBILITY_OFF);
            m_creature->setDeathState(JUST_DIED);
            return;
        }

        if(!UpdateVictim())
            return;

        if (BuffetTimer.Expired(diff))
        {
            m_creature->CastSpell(m_creature->GetVictim(), SPELL_FLAMEBUFFET, false);
            BuffetTimer = 10000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_hatchlingAI(Creature *_Creature)
{
    return new mob_hatchlingAI(_Creature);
}

struct mob_eggAI : public ScriptedAI
{
    mob_eggAI(Creature *c) : ScriptedAI(c){}
    void Reset() {}
    void EnterCombat(Unit* who) {}
    void AttackStart(Unit* who) {}
    void MoveInLineOfSight(Unit* who) {}
    void UpdateAI(const uint32 diff) {}

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if(spell->Id == SPELL_HATCH_EGG)
        {
            DoSpawnCreature(MOB_HATCHLING, 0, 0, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60000);
            m_creature->SetDisplayId(11686);
        }
    }
};

CreatureAI* GetAI_mob_eggAI(Creature *_Creature)
{
    return new mob_eggAI(_Creature);
}

void AddSC_boss_janalai()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_janalai";
    newscript->GetAI = &GetAI_boss_janalaiAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_janalai_firebomb";
    newscript->GetAI = &GetAI_mob_janalai_firebombAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_janalai_hatcher";
    newscript->GetAI = &GetAI_mob_amanishi_hatcherAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_janalai_hatchling";
    newscript->GetAI = &GetAI_mob_hatchlingAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_janalai_egg";
    newscript->GetAI = &GetAI_mob_eggAI;
    newscript->RegisterSelf();
}

