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
SDName: Eversong_Woods
SD%Complete: 100
SDComment: Quest support: 8346, 8483, 8488, 8490
SDCategory: Eversong Woods
EndScriptData */

/* ContentData
mobs_mana_tapped
npc_prospector_anvilward
npc_apprentice_mirveda
npc_infused_crystal
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## mobs_mana_tapped
######*/

struct mobs_mana_tappedAI : public ScriptedAI
{
    mobs_mana_tappedAI(Creature *c) : ScriptedAI(c) {}

    void Reset() { }

    void EnterCombat(Unit *who) { }

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if( caster->GetTypeId() == TYPEID_PLAYER)
            if( ((Player*)caster)->GetQuestStatus(8346) == QUEST_STATUS_INCOMPLETE && !((Player*)caster)->GetReqKillOrCastCurrentCount(8346, m_creature->GetEntry()) && spell->Id == 28734)
                ((Player*)caster)->CastCreatureOrGO(15468, m_creature->GetGUID(), spell->Id);
        return;
    }
};
CreatureAI* GetAI_mobs_mana_tapped(Creature *_Creature)
{
    return new mobs_mana_tappedAI (_Creature);
}

/*######
## npc_prospector_anvilward
######*/

#define GOSSIP_HELLO    "I need a moment of your time, sir."
#define GOSSIP_SELECT   "Why... yes, of course. I've something to show you right inside this building, Mr. Anvilward."

#define SAY_PR_1 -1000281
#define SAY_PR_2 -1000282

#define QUEST_THE_DWARVEN_SPY 8483

struct npc_prospector_anvilwardAI : public npc_escortAI
{
    // CreatureAI functions
    npc_prospector_anvilwardAI(Creature *c) : npc_escortAI(c) {}

    // Pure Virtual Functions
    void WaypointReached(uint32 i)
    {
       Player* pPlayer = GetPlayerForEscort();

        if(!pPlayer)
            return;

        switch (i)
        {
            case 0: DoScriptText(SAY_PR_1, m_creature, pPlayer); break;
            case 5: DoScriptText(SAY_PR_2, m_creature, pPlayer); break;
            case 6: m_creature->setFaction(24); break;
        }
    }

    void EnterCombat(Unit* who) { }

    void Reset()
    {
        me->RestoreFaction();
    }

    void JustDied(Unit* killer)
    {
        me->RestoreFaction();
    }
};

CreatureAI* GetAI_npc_prospector_anvilward(Creature *_Creature)
{
    npc_prospector_anvilwardAI* thisAI = new npc_prospector_anvilwardAI(_Creature);

    thisAI->AddWaypoint(0, 9294.78, -6682.51, 22.42);
    thisAI->AddWaypoint(1, 9298.27, -6667.99, 22.42);
    thisAI->AddWaypoint(2, 9309.63, -6658.84, 22.43);
    thisAI->AddWaypoint(3, 9304.43, -6649.31, 26.46);
    thisAI->AddWaypoint(4, 9298.83, -6648.00, 28.61);
    thisAI->AddWaypoint(5, 9291.06, -6653.46, 31.83, 2500);
    thisAI->AddWaypoint(6, 9289.08, -6660.17, 31.85, 5000);
    thisAI->AddWaypoint(7, 9291.06, -6653.46, 31.83);

    return (CreatureAI*)thisAI;
}

bool GossipHello_npc_prospector_anvilward(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_THE_DWARVEN_SPY) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    pPlayer->SEND_GOSSIP_MENU(8239, pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_prospector_anvilward(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM( 0, GOSSIP_SELECT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            pPlayer->SEND_GOSSIP_MENU(8240, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->CLOSE_GOSSIP_MENU();
            if (npc_escortAI* pEscortAI = CAST_AI(npc_prospector_anvilwardAI, pCreature->AI()))
                pEscortAI->Start(true, false, pPlayer->GetGUID());
            break;
    }
    return true;
}

/*######
## Quest 9686 Second Trial
######*/

#define QUEST_SECOND_TRIAL 9686

#define MASTER_KELERUN_BLOODMOURN 17807

#define CHAMPION_BLOODWRATH 17809
#define CHAMPION_LIGHTREND  17810
#define CHAMPION_SWIFTBLADE 17811
#define CHAMPION_SUNSTRIKER 17812

#define HARBINGER_OF_THE_SECOND_TRIAL 182052

#define SPELL_FLASH_OF_LIGHT 19939
#define TIMER_FLASH_OF_LIGHT 3225

#define SPELL_SEAL_OF_JUSTICE 20164
#define TIMER_SEAL_OF_JUSTICE 10000

#define SPELL_JUDGEMENT_OF_LIGHT 20271
#define TIMER_JUDGEMENT_OF_LIGHT 10000

#define SPELL_SEAL_OF_COMMAND 20375
#define TIMER_SEAL_OF_COMMAND 20000

#define FACTION_HOSTILE    45
#define FACTION_FRIENDLY   7

#define TEXT_SECOND_TRIAL_1 -1645006
#define TEXT_SECOND_TRIAL_2 -1645007
#define TEXT_SECOND_TRIAL_3 -1645008
#define TEXT_SECOND_TRIAL_4 -1645009

struct Locations
{
    float x, y, z, o;
};

static Locations SpawnPosition[]=
{
    {5.3f,   -11.8f,  0.361f, 4.2f},
    {11.2f,  -29.17f, 0.361f, 2.7f},
    {-5.7f,  -34.85f, 0.361f, 1.09f},
    {-11.9f, -18,     0.361f, 5.87f}
};

static uint32 PaladinEntry[]= {CHAMPION_BLOODWRATH, CHAMPION_LIGHTREND, CHAMPION_SWIFTBLADE, CHAMPION_SUNSTRIKER};

/*######
## npc_second_trial_controller
######*/

struct master_kelerun_bloodmournAI : public ScriptedAI
{
    master_kelerun_bloodmournAI(Creature *c) : ScriptedAI(c) {}

    uint8  questPhase; // 0-none, 1-waiting, 2345-pallies, 6-complete
    int32  timer;

    uint64 paladinGuid[4];

    void Reset()
    {
        questPhase = 0;
        timer = 60000;
        uint64 paladinGuid[] = {0,0,0,0};
    }

    void EnterCombat(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        if (questPhase == 6) // last pally killed
        {
            Reset();
            return;
        }
        else if (questPhase == 1) // Quest accepted but object not yet activated
        {
            timer -= diff;
            if (timer <= 0)
                Reset();
        }
        else if (questPhase > 1 && timer) // activate paladins
        {
            timer -= diff;
            if (timer <= 0)
            {
                Creature* paladinSpawn;
                paladinSpawn = (Unit::GetCreature((*m_creature), paladinGuid[questPhase - 2]));
                if (paladinSpawn)
                {
                    paladinSpawn->AI()->DoAction(0);

                    switch(questPhase)
                    {
                        case 2:
                            DoScriptText(TEXT_SECOND_TRIAL_1,m_creature);
                            break;
                        case 3:
                            DoScriptText(TEXT_SECOND_TRIAL_2,m_creature);
                            break;
                        case 4:
                            DoScriptText(TEXT_SECOND_TRIAL_3,m_creature);
                            break;
                        case 5:
                            DoScriptText(TEXT_SECOND_TRIAL_4,m_creature);
                            break;
                    }
                }
                else
                    Reset();

                timer = 0;
            }
            
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

    void StartEvent()
    {
        if (questPhase == 1)
        {
            for (int i = 0; i<4; i++)
            {
                Creature* Summoned;
                Summoned = DoSpawnCreature(PaladinEntry[i], SpawnPosition[i].x, SpawnPosition[i].y, SpawnPosition[i].z, SpawnPosition[i].o, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 180000);

                if(Summoned)
                {
                    paladinGuid[i] = Summoned->GetGUID();
                    Summoned->setFaction(FACTION_FRIENDLY);
                }
            }

            timer = 500;
            questPhase++;
        }
    }

    void SecondTrialKill()
    {
        if (questPhase > 1 && questPhase < 6)
        {
            ++questPhase;
            timer = 500;
        }
    }
};


bool GossipHello_master_kelerun_bloodmourn(Player *player, Creature *_Creature)
{
    if (((master_kelerun_bloodmournAI*)_Creature->AI())->questPhase == 0)
    {
        player->PrepareQuestMenu(_Creature->GetGUID());
        player->SendPreparedQuest(_Creature->GetGUID());
    }

    player->SEND_GOSSIP_MENU(_Creature->GetEntry(), _Creature->GetGUID());
    return true;
}

bool QuestAccept_master_kelerun_bloodmourn(Player *player, Creature *creature, Quest const *quest )
{
    // One Player exclusive quest, wait for user go activation
    if (quest->GetQuestId() == QUEST_SECOND_TRIAL)
        ((master_kelerun_bloodmournAI*)creature->AI())->questPhase = 1;

    return true;
}

CreatureAI* GetAI_master_kelerun_bloodmourn(Creature *_Creature)
{
    return new master_kelerun_bloodmournAI(_Creature);
}

/*######
## npc_second_trial_paladin
######*/

struct npc_secondTrialAI : public ScriptedAI
{
    npc_secondTrialAI(Creature *c) : ScriptedAI(c) {}

    int32  timer;

    bool spellFlashLight;
    bool spellJustice;
    bool spellJudLight;
    bool spellCommand;

    int32 timerFlashLight;
    int32 timerJustice;
    int32 timerJudLight;
    int32 timerCommand;


    void Reset()
    {
        timer = 2000;

        m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, PLAYER_STATE_KNEEL);
        m_creature->setFaction(FACTION_FRIENDLY);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);

        spellFlashLight = false;
        spellJustice = false;
        spellJudLight = false;
        spellCommand = false;

        switch (m_creature->GetEntry())
        {
        case CHAMPION_BLOODWRATH:
            spellFlashLight = true;
            timerFlashLight = TIMER_FLASH_OF_LIGHT;
            break;
        case CHAMPION_LIGHTREND:
            spellJustice = true;
            timerJustice = 500;
            break;
        case CHAMPION_SWIFTBLADE:
            spellJudLight = false;  // Misses Script Effect // http://www.wowhead.com/?spell=20271
            timerJudLight = 500;
            break;
        case CHAMPION_SUNSTRIKER:
            spellFlashLight = true;
            spellJudLight = false;  // Misses Script Effect // http://www.wowhead.com/?spell=20271
            spellCommand = false;  // Misses Dummy // http://www.wowhead.com/?spell=20375
            timerFlashLight = TIMER_FLASH_OF_LIGHT;
            timerJudLight = 500;
            timerCommand = 1500;
            break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        // healer
        if (spellFlashLight && m_creature->GetHealth() * 100 / m_creature->GetMaxHealth() < 70)
        {
            timerFlashLight -= diff;
            if (timerFlashLight <= diff)
            {
                DoCast(m_creature, SPELL_FLASH_OF_LIGHT);
                timerFlashLight += TIMER_FLASH_OF_LIGHT + rand() % (TIMER_FLASH_OF_LIGHT);
            }
        }

        if (spellJustice)
        {
            timerJustice -= diff;
            if (timerJustice <= diff)
            {
                DoCast(m_creature, SPELL_SEAL_OF_JUSTICE);
                timerJustice += TIMER_SEAL_OF_JUSTICE + rand() % (TIMER_SEAL_OF_JUSTICE);
            }
        }

        if (spellJudLight)
        {
            timerJudLight -= diff;
            if (timerJudLight <= diff)
            {
                DoCast(m_creature, SPELL_JUDGEMENT_OF_LIGHT);
                timerJudLight += TIMER_JUDGEMENT_OF_LIGHT + rand() % (TIMER_JUDGEMENT_OF_LIGHT);
            }
        }

        if (spellCommand)
        {
            timerCommand -= diff;
            if (timerCommand <= diff)
            {
                DoCast(m_creature, TIMER_SEAL_OF_COMMAND);
                timerCommand += TIMER_SEAL_OF_COMMAND + rand() % (TIMER_SEAL_OF_COMMAND);
            }
        }

        DoMeleeAttackIfReady();
    }

    void DoAction(const int32 param)
    {
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
        m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, PLAYER_STATE_NONE);
        m_creature->setFaction(FACTION_HOSTILE);

        Unit *target = m_creature->SelectNearbyTarget(30);
        if (target && target->GetTypeId() == TYPEID_PLAYER)
        {
            m_creature->AddThreat(target, 5000000.0f);
            AttackStart(target);
        }
    }

    void KilledUnit(Unit* Killed)
    {
        if (Killed->GetTypeId() == TYPEID_PLAYER &&
            Killed->ToPlayer()->GetQuestStatus(QUEST_SECOND_TRIAL) == QUEST_STATUS_INCOMPLETE)
            Killed->ToPlayer()->FailQuest(QUEST_SECOND_TRIAL);
    }

    void JustDied(Unit* Killer)
    {
        if (Killer->GetTypeId() == TYPEID_PLAYER)
        {
            if (Creature* Summoner = GetClosestCreatureWithEntry(me, MASTER_KELERUN_BLOODMOURN, 30))
                ((master_kelerun_bloodmournAI*)Summoner->AI())->SecondTrialKill();

            if (m_creature->GetEntry() == CHAMPION_SUNSTRIKER &&
                Killer->ToPlayer()->GetQuestStatus(QUEST_SECOND_TRIAL) == QUEST_STATUS_INCOMPLETE)
                Killer->ToPlayer()->CompleteQuest(QUEST_SECOND_TRIAL);
        }
    }
};

CreatureAI* GetAI_npc_secondTrial(Creature *_Creature)
{
    return new npc_secondTrialAI (_Creature);
}

/*######
## go_second_trial
######*/

bool GOUse_go_second_trial(Player *player, GameObject* _GO)
{
    // find spawn :: master_kelerun_bloodmourn
    if (Creature* event_controller = GetClosestCreatureWithEntry(_GO, MASTER_KELERUN_BLOODMOURN, 30))
    {
        ((master_kelerun_bloodmournAI*)event_controller->AI())->StartEvent();
    }
    return true;
}

/*######
## npc_apprentice_mirveda
######*/

#define QUEST_UNEXPECTED_RESULT 8488
#define MOB_GHARZUL     15958
#define MOB_ANGERSHADE  15656

struct npc_apprentice_mirvedaAI : public ScriptedAI
{
    npc_apprentice_mirvedaAI(Creature* c) : ScriptedAI(c), Summons(m_creature) {}

    uint32 KillCount;
    uint64 PlayerGUID;
    bool Summon;
    SummonList Summons;

    void Reset()
    {
        KillCount = 0;
        PlayerGUID = 0;
        Summons.DespawnAll();
        Summon = false;
    }

    void JustSummoned(Creature *summoned)
    {
        summoned->AI()->AttackStart(m_creature);
        Summons.Summon(summoned);
    }

    void SummonedCreatureDespawn(Creature* summoned)
    {
        Summons.Despawn(summoned);
        ++KillCount;
    }

    void JustDied(Unit* killer)
    {
        if (PlayerGUID)
        {
            Player* player = Unit::GetPlayer(PlayerGUID);
            if (player)
                player->FailQuest(QUEST_UNEXPECTED_RESULT);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(KillCount >= 3)
        {
            if (PlayerGUID)
            {
                Player* player = Unit::GetPlayer(PlayerGUID);
                if(player)
                    player->CompleteQuest(QUEST_UNEXPECTED_RESULT);
                Reset();
            }
        }

        if(Summon)
        {
            Creature* cre = NULL;
            cre = m_creature->SummonCreature(MOB_GHARZUL, 8725, -7143.32, 35.22, 0, TEMPSUMMON_CORPSE_DESPAWN, 4000);
            if (cre) cre->AI()->AttackStart(me);
            cre = m_creature->SummonCreature(MOB_ANGERSHADE, 8725, -7143.32, 35.22, 0, TEMPSUMMON_CORPSE_DESPAWN, 4000);
            if (cre) cre->AI()->AttackStart(me);
            cre = m_creature->SummonCreature(MOB_ANGERSHADE, 8725, -7143.32, 35.22, 0, TEMPSUMMON_CORPSE_DESPAWN, 4000);
            if (cre) cre->AI()->AttackStart(me);
            Summon = false;
        }
    }
};

bool QuestAccept_npc_apprentice_mirveda(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_UNEXPECTED_RESULT)
    {
        ((npc_apprentice_mirvedaAI*)creature->AI())->Summon = true;
        ((npc_apprentice_mirvedaAI*)creature->AI())->PlayerGUID = player->GetGUID();
    }
    return true;
}

CreatureAI* GetAI_npc_apprentice_mirvedaAI(Creature *_Creature)
{
    return new npc_apprentice_mirvedaAI (_Creature);
}

/*######
## npc_infused_crystal
######*/

#define MOB_ENRAGED_WRAITH  17086
#define EMOTE   -1000283
#define QUEST_POWERING_OUR_DEFENSES 8490
#define POWERING_OUR_DEFENSES_CREDIT 16364

struct Location
{
    float x, y, z;
};

static Location SpawnLocations[]=
{
    {8270.68f, -7188.53f, 139.619f},
    {8284.27f, -7187.78f, 139.603f},
    {8297.43f, -7193.53f, 139.603f},
    {8303.5f, -7201.96f, 139.577f},
    {8273.22f, -7241.82f, 139.382f},
    {8254.89f, -7222.12f, 139.603f},
    {8278.51f, -7242.13f, 139.162f},
    {8267.97f, -7239.17f, 139.517f}
};

struct npc_infused_crystalAI : public Scripted_NoMovementAI
{
    npc_infused_crystalAI(Creature* c) : Scripted_NoMovementAI(c) {}

    int32 EndTimer;
    int32 WaveTimer;
    bool Completed;
    bool Progress;
    uint64 PlayerGUID;

    void Reset()
    {
        EndTimer = 0;
        Completed = false;
        Progress = false;
        PlayerGUID = 0;
        WaveTimer = 0;
    }

    void MoveInLineOfSight(Unit* who)
    {
        if( who->GetTypeId() == TYPEID_PLAYER && !m_creature->canStartAttack(who) && !Progress)
        {
            if( ((Player*)who)->GetQuestStatus(QUEST_POWERING_OUR_DEFENSES) == QUEST_STATUS_INCOMPLETE )
            {
                float Radius = 10.0;
                if( m_creature->IsWithinDistInMap(who, Radius) )
                {
                    PlayerGUID = who->GetGUID();
                    WaveTimer = 1000;
                    EndTimer = 60000;
                    Progress = true;
                }
            }
        }
    }

    void JustSummoned(Creature *summoned)
    {
        summoned->AI()->AttackStart(m_creature);
    }

    void JustDied(Unit* killer)
    {
        if (PlayerGUID && !Completed)
        {
            Player* player = Unit::GetPlayer(PlayerGUID);
            if (player)
                player->FailQuest(QUEST_POWERING_OUR_DEFENSES);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        EndTimer -= diff;
        if(EndTimer <= diff && Progress)
        {
            Completed = true;
            if (PlayerGUID)
            {
                Player* player = Unit::GetPlayer(PlayerGUID);
                if(player)
                {
                    DoScriptText(EMOTE, m_creature);
                    player->KilledMonster(POWERING_OUR_DEFENSES_CREDIT, 0);
                }
            }
            m_creature->DisappearAndDie();
        }
        

        WaveTimer -= diff;
        if(WaveTimer <= diff && !Completed && Progress)
        {
            uint32 ran1 = rand()%8;
            uint32 ran2 = rand()%8;
            uint32 ran3 = rand()%8;
            m_creature->SummonCreature(MOB_ENRAGED_WRAITH, SpawnLocations[ran1].x, SpawnLocations[ran1].y, SpawnLocations[ran1].z, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 10000);
            m_creature->SummonCreature(MOB_ENRAGED_WRAITH, SpawnLocations[ran2].x, SpawnLocations[ran2].y, SpawnLocations[ran2].z, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 10000);
            m_creature->SummonCreature(MOB_ENRAGED_WRAITH, SpawnLocations[ran3].x, SpawnLocations[ran3].y, SpawnLocations[ran3].z, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 10000);
            WaveTimer += 30000;
        }
    }
};

CreatureAI* GetAI_npc_infused_crystalAI(Creature *_Creature)
{
    return new npc_infused_crystalAI (_Creature);
}

void AddSC_eversong_woods()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="mobs_mana_tapped";
    newscript->GetAI = &GetAI_mobs_mana_tapped;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name= "npc_prospector_anvilward";
    newscript->GetAI = &GetAI_npc_prospector_anvilward;
    newscript->pGossipHello =  &GossipHello_npc_prospector_anvilward;
    newscript->pGossipSelect = &GossipSelect_npc_prospector_anvilward;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_second_trial_controller";
    newscript->GetAI = &GetAI_master_kelerun_bloodmourn;
    newscript->pGossipHello = &GossipHello_master_kelerun_bloodmourn;
    newscript->pQuestAcceptNPC = &QuestAccept_master_kelerun_bloodmourn;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_second_trial_paladin";
    newscript->GetAI = &GetAI_npc_secondTrial;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_second_trial";
    newscript->pGOUse =  &GOUse_go_second_trial;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_apprentice_mirveda";
    newscript->GetAI = &GetAI_npc_apprentice_mirvedaAI;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_apprentice_mirveda;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_infused_crystal";
    newscript->GetAI = &GetAI_npc_infused_crystalAI;
    newscript->RegisterSelf();
}

