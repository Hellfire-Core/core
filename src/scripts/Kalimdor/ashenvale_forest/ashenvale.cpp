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
SDName: Ashenvale
SD%Complete: 70
SDComment: Quest support: 6544, 6482
SDCategory: Ashenvale Forest
EndScriptData */

/* ContentData
npc_torek
npc_ruul_snowhoof
EndContentData */

#include "scriptPCH.h"
#include "ScriptedEscortAI.h"

/*####
# npc_torek
####*/

#define SAY_READY                   -1000106
#define SAY_MOVE                    -1000107
#define SAY_PREPARE                 -1000108
#define SAY_WIN                     -1000109
#define SAY_END                     -1000110

#define SPELL_REND                  11977
#define SPELL_THUNDERCLAP           8078

#define QUEST_TOREK_ASSULT          6544

#define ENTRY_SPLINTERTREE_RAIDER   12859
#define ENTRY_DURIEL                12860
#define ENTRY_SILVERWING_SENTINEL   12896
#define ENTRY_SILVERWING_WARRIOR    12897

struct npc_torekAI : public npc_escortAI
{
    npc_torekAI(Creature *c) : npc_escortAI(c) {}

    int32 Rend_Timer;
    int32 Thunderclap_Timer;
    bool Completed;

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch (i)
        {
        case 1:
            DoScriptText(SAY_MOVE, m_creature, pPlayer);
            break;
        case 8:
            DoScriptText(SAY_PREPARE, m_creature, pPlayer);
            break;
        case 19:
            //TODO: verify location and creatures amount.
            m_creature->SummonCreature(ENTRY_DURIEL,1776.73,-2049.06,109.83,1.54,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,25000);
            m_creature->SummonCreature(ENTRY_SILVERWING_SENTINEL,1774.64,-2049.41,109.83,1.40,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,25000);
            m_creature->SummonCreature(ENTRY_SILVERWING_WARRIOR,1778.73,-2049.50,109.83,1.67,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,25000);
            break;
        case 20:
            DoScriptText(SAY_WIN, m_creature, pPlayer);
            Completed = true;
            pPlayer->GroupEventHappens(QUEST_TOREK_ASSULT,m_creature);
            break;
        case 21:
            DoScriptText(SAY_END, m_creature, pPlayer);
            break;
        }
    }

    void Reset()
    {
        Rend_Timer = 5000;
        Thunderclap_Timer = 8000;
        Completed = false;
    }

    void EnterCombat(Unit* who) {}

    void JustSummoned(Creature* summoned)
    {
        summoned->AI()->AttackStart(m_creature);
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        Rend_Timer -= diff;
        if (Rend_Timer <= diff)
        {
            DoCast(m_creature->GetVictim(),SPELL_REND);
            Rend_Timer += 20000;
        }
        

        Thunderclap_Timer -= diff;
        if (Thunderclap_Timer <= diff)
        {
            DoCast(m_creature,SPELL_THUNDERCLAP);
            Thunderclap_Timer += 30000;
        }
        
    }
};

bool QuestAccept_npc_torek(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (pPlayer && quest->GetQuestId() == QUEST_TOREK_ASSULT)
    {
        //TODO: find companions, make them follow Torek, at any time (possibly done by mangos/database in future?)
        DoScriptText(SAY_READY, pCreature, pPlayer);
        pCreature->setFaction(113);

        if (npc_escortAI* pEscortAI = CAST_AI(npc_torekAI, pCreature->AI()))
            pEscortAI->Start(true, true, pPlayer->GetGUID(), quest);
    }

    return true;
}

CreatureAI* GetAI_npc_torek(Creature *_Creature)
{
    npc_torekAI* thisAI = new npc_torekAI(_Creature);

    thisAI->AddWaypoint(0, 1782.63, -2241.11, 109.73, 5000);
    thisAI->AddWaypoint(1, 1788.88, -2240.17, 111.71);
    thisAI->AddWaypoint(2, 1797.49, -2238.11, 112.31);
    thisAI->AddWaypoint(3, 1803.83, -2232.77, 111.22);
    thisAI->AddWaypoint(4, 1806.65, -2217.83, 107.36);
    thisAI->AddWaypoint(5, 1811.81, -2208.01, 107.45);
    thisAI->AddWaypoint(6, 1820.85, -2190.82, 100.49);
    thisAI->AddWaypoint(7, 1829.60, -2177.49, 96.44);
    thisAI->AddWaypoint(8, 1837.98, -2164.19, 96.71);       //prepare
    thisAI->AddWaypoint(9, 1839.99, -2149.29, 96.78);
    thisAI->AddWaypoint(10, 1835.14, -2134.98, 96.80);
    thisAI->AddWaypoint(11, 1823.57, -2118.27, 97.43);
    thisAI->AddWaypoint(12, 1814.99, -2110.35, 98.38);
    thisAI->AddWaypoint(13, 1806.60, -2103.09, 99.19);
    thisAI->AddWaypoint(14, 1798.27, -2095.77, 100.04);
    thisAI->AddWaypoint(15, 1783.59, -2079.92, 100.81);
    thisAI->AddWaypoint(16, 1776.79, -2069.48, 101.77);
    thisAI->AddWaypoint(17, 1776.82, -2054.59, 109.82);
    thisAI->AddWaypoint(18, 1776.88, -2047.56, 109.83);
    thisAI->AddWaypoint(19, 1776.86, -2036.55, 109.83);
    thisAI->AddWaypoint(20, 1776.90, -2024.56, 109.83);     //win
    thisAI->AddWaypoint(21, 1776.87, -2028.31, 109.83,60000);//stay
    thisAI->AddWaypoint(22, 1776.90, -2028.30, 109.83);

    return (CreatureAI*)thisAI;
}

/*####
# npc_ruul_snowhoof
####*/

#define QUEST_FREEDOM_TO_RUUL    6482
#define GO_CAGE                  178147

struct npc_ruul_snowhoofAI : public npc_escortAI
{
    npc_ruul_snowhoofAI(Creature *c) : npc_escortAI(c) {}

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();
        if (!pPlayer)
            return;

        switch(i)
        {
        case 0: 
        {
                m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
                GameObject* Cage = FindGameObject(GO_CAGE, 20, m_creature);
                if(Cage)
                    Cage->SetGoState(GO_STATE_ACTIVE);
                break;
        }
        case 13:
                m_creature->SummonCreature(3922, 3449.218018, -587.825073, 174.978867, 4.714445, TEMPSUMMON_DEAD_DESPAWN, 60000);
                m_creature->SummonCreature(3921, 3446.384521, -587.830872, 175.186279, 4.714445, TEMPSUMMON_DEAD_DESPAWN, 60000);
                m_creature->SummonCreature(3926, 3444.218994, -587.835327, 175.380600, 4.714445, TEMPSUMMON_DEAD_DESPAWN, 60000);
                break;
        case 19:
                m_creature->SummonCreature(3922, 3508.344482, -492.024261, 186.929031, 4.145029, TEMPSUMMON_DEAD_DESPAWN, 60000);
                m_creature->SummonCreature(3921, 3506.265625, -490.531006, 186.740128, 4.239277, TEMPSUMMON_DEAD_DESPAWN, 60000);
                m_creature->SummonCreature(3926, 3503.682373, -489.393799, 186.629684, 4.349232, TEMPSUMMON_DEAD_DESPAWN, 60000);
                break;

        case 21:
                pPlayer->GroupEventHappens(QUEST_FREEDOM_TO_RUUL, m_creature);
                break;
        }
    }

    void EnterCombat(Unit* who) {}

    void Reset()
    {
        if (GameObject* Cage = FindGameObject(GO_CAGE, 20, m_creature))
            Cage->SetGoState(GO_STATE_READY);
    }
};

bool QuestAccept_npc_ruul_snowhoof(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_FREEDOM_TO_RUUL)
    {
        pCreature->setFaction(113);

        if (npc_escortAI* pEscortAI = CAST_AI(npc_ruul_snowhoofAI, (pCreature->AI())))
            pEscortAI->Start(true, false, pPlayer->GetGUID(), quest);
    }
    return true;
}

CreatureAI* GetAI_npc_ruul_snowhoofAI(Creature *_Creature)
{
    npc_ruul_snowhoofAI* ruul_snowhoofAI = new npc_ruul_snowhoofAI(_Creature);

    ruul_snowhoofAI->AddWaypoint(0, 3347.250089, -694.700989, 159.925995);
    ruul_snowhoofAI->AddWaypoint(1, 3341.527039, -694.725891, 161.124542, 4000);
    ruul_snowhoofAI->AddWaypoint(2, 3338.351074, -686.088138, 163.444000);
    ruul_snowhoofAI->AddWaypoint(3, 3352.744873, -677.721741, 162.316269);
    ruul_snowhoofAI->AddWaypoint(4, 3370.291016, -669.366943, 160.751358);
    ruul_snowhoofAI->AddWaypoint(5, 3381.479492, -659.449097, 162.545303);
    ruul_snowhoofAI->AddWaypoint(6, 3389.554199, -648.500000, 163.651825);
    ruul_snowhoofAI->AddWaypoint(7, 3396.645020, -641.508911, 164.216019);
    ruul_snowhoofAI->AddWaypoint(8, 3410.498535, -634.299622, 165.773453);
    ruul_snowhoofAI->AddWaypoint(9, 3418.461426, -631.791992, 166.477615);
    ruul_snowhoofAI->AddWaypoint(10, 3429.500000, -631.588745, 166.921265);
    ruul_snowhoofAI->AddWaypoint(11,3434.950195, -629.245483, 168.333969);
    ruul_snowhoofAI->AddWaypoint(12,3438.927979, -618.503235, 171.503143);
    ruul_snowhoofAI->AddWaypoint(13,3444.217529, -609.293640, 173.077972, 1000); // Ambush 1
    ruul_snowhoofAI->AddWaypoint(14,3460.505127, -593.794189, 174.342255);
    ruul_snowhoofAI->AddWaypoint(15,3480.283203, -578.210327, 176.652313);
    ruul_snowhoofAI->AddWaypoint(16,3492.912842, -562.335449, 181.396301);
    ruul_snowhoofAI->AddWaypoint(17,3495.230957, -550.977600, 184.652267);
    ruul_snowhoofAI->AddWaypoint(18,3496.247070, -529.194214, 188.172028);
    ruul_snowhoofAI->AddWaypoint(19,3497.619385, -510.411499, 188.345322, 1000); // Ambush 2
    ruul_snowhoofAI->AddWaypoint(20,3498.498047, -497.787506, 185.806274);
    ruul_snowhoofAI->AddWaypoint(21,3484.218750, -489.717529, 182.389862, 4000); // End

    return (CreatureAI*)ruul_snowhoofAI;
}

/*####
# npc_muglash
####*/

enum eEnums
{
    SAY_MUG_START1          = -1800054,
    SAY_MUG_START2          = -1800055,
    SAY_MUG_BRAZIER         = -1800056,
    SAY_MUG_BRAZIER_WAIT    = -1800057,
    SAY_MUG_ON_GUARD        = -1800058,
    SAY_MUG_REST            = -1800059,
    SAY_MUG_DONE            = -1800060,
    SAY_MUG_GRATITUDE       = -1800061,
    SAY_MUG_PATROL          = -1800062,
    SAY_MUG_RETURN          = -1800063,

    QUEST_VORSHA            = 6641,

    GO_NAGA_BRAZIER         = 178247,

    NPC_WRATH_RIDER         = 3713,
    NPC_WRATH_SORCERESS     = 3717,
    NPC_WRATH_RAZORTAIL     = 3712,

    NPC_WRATH_PRIESTESS     = 3944,
    NPC_WRATH_MYRMIDON      = 3711,
    NPC_WRATH_SEAWITCH      = 3715,

    NPC_VORSHA              = 12940,
    NPC_MUGLASH             = 12717
};

static float m_afFirstNagaCoord[3][3]=
{
    {3603.504150f, 1122.631104f, 1.635f},                      // rider
    {3589.293945f, 1148.664063f, 5.565f},                      // sorceress
    {3609.925537f, 1168.759521f, -1.168f}                      // razortail
};

static float m_afSecondNagaCoord[3][3]=
{
    {3609.925537f, 1168.759521f, -1.168f},                     // witch
    {3645.652100f, 1139.425415f, 1.322f},                      // priest
    {3583.602051f, 1128.405762f, 2.347f}                       // myrmidon
};

static float m_fVorshaCoord[]={3633.056885f, 1172.924072f, -5.388f};

struct npc_muglashAI : public npc_escortAI
{
    npc_muglashAI(Creature* pCreature) : npc_escortAI(pCreature) { }

    uint32 m_uiWaveId;
    int32 m_uiEventTimer;
    int32 m_uiPausedCheckTimer;
    uint64 m_uiBrazierGUID;
    bool m_bIsBrazierExtinguished;

    void JustSummoned(Creature* pSummoned)
    {
        pSummoned->SetHomePosition(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0);
        pSummoned->SetAggroRange(50.0f);
        pSummoned->AI()->AttackStart(m_creature);
    }

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();

        switch(i)
        {
            case 0:
                if (pPlayer)
                    DoScriptText(SAY_MUG_START2, m_creature, pPlayer);
                break;
            case 24:
                if (pPlayer)
                    DoScriptText(SAY_MUG_BRAZIER, m_creature, pPlayer);

                if (GameObject* pGo = FindGameObject(GO_NAGA_BRAZIER, INTERACTION_DISTANCE*2, m_creature))
                {
                    m_uiBrazierGUID = pGo->GetGUID();
                    
                    pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);
                    SetEscortPaused(true);
                }
                break;
            case 25:
                DoScriptText(SAY_MUG_GRATITUDE, m_creature);

                if (pPlayer)
                    pPlayer->GroupEventHappens(QUEST_VORSHA, m_creature);
                break;
            case 26:
                DoScriptText(SAY_MUG_PATROL, m_creature);
                break;
            case 27:
                if(m_uiBrazierGUID)
                {
                    if (GameObject* pGo = GameObject::GetGameObject(*m_creature, m_uiBrazierGUID))
                    {
                        pGo->SetLootState(GO_JUST_DEACTIVATED);
                        pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);
                        pGo->SetGoState(GO_STATE_READY);
                    }
                }
                DoScriptText(SAY_MUG_RETURN, m_creature);
                break;
        }
    }

    void Aggro(Unit* pWho)
    {
        if (HasEscortState(STATE_ESCORT_PAUSED))
        {
            if (urand(0, 1))
            DoScriptText(SAY_MUG_ON_GUARD, m_creature);
            return;
        }
    }

    void Reset()
    {
        if(!HasEscortState(STATE_ESCORT_ESCORTING))
            m_uiBrazierGUID = 0;
        if(!HasEscortState(STATE_ESCORT_PAUSED))
        {
            m_uiPausedCheckTimer = 120000;   //after 2 minutes pausing event ends with fail
            m_uiEventTimer = 10000;
            m_uiWaveId = 0;
            m_bIsBrazierExtinguished = false;
        }
    }

    void JustDied(Unit* pKiller)
    {
        m_creature->Respawn();
        if (Player* pPlayer = GetPlayerForEscort())
        {
            if (Group* pGroup = pPlayer->GetGroup())
            {
                for (GroupReference* pRef = pGroup->GetFirstMember(); pRef != NULL; pRef = pRef->next())
                {
                    if (Player* pMember = pRef->getSource())
                    {
                        if (pMember->GetQuestStatus(QUEST_VORSHA) == QUEST_STATUS_INCOMPLETE)
                            pMember->FailQuest(QUEST_VORSHA);
                    }
                }
            }
            else
            {
                if (pPlayer->GetQuestStatus(QUEST_VORSHA) == QUEST_STATUS_INCOMPLETE)
                    pPlayer->FailQuest(QUEST_VORSHA);
            }
            if(m_uiBrazierGUID)
            {
                if (GameObject* pGo = GameObject::GetGameObject(*pPlayer, m_uiBrazierGUID))
                {
                    pGo->SetLootState(GO_JUST_DEACTIVATED);
                    pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);
                    pGo->SetGoState(GO_STATE_READY);
                }
            }
        }
    }

    void DoWaveSummon()
    {
        switch(m_uiWaveId)
        {
            case 1:
                m_creature->SummonCreature(NPC_WRATH_RIDER,     m_afFirstNagaCoord[0][0], m_afFirstNagaCoord[0][1], m_afFirstNagaCoord[0][2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                m_creature->SummonCreature(NPC_WRATH_SORCERESS, m_afFirstNagaCoord[1][0], m_afFirstNagaCoord[1][1], m_afFirstNagaCoord[1][2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                m_creature->SummonCreature(NPC_WRATH_RAZORTAIL, m_afFirstNagaCoord[2][0], m_afFirstNagaCoord[2][1], m_afFirstNagaCoord[2][2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                break;
            case 2:
                m_creature->SummonCreature(NPC_WRATH_PRIESTESS, m_afSecondNagaCoord[0][0], m_afSecondNagaCoord[0][1], m_afSecondNagaCoord[0][2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                m_creature->SummonCreature(NPC_WRATH_MYRMIDON,  m_afSecondNagaCoord[1][0], m_afSecondNagaCoord[1][1], m_afSecondNagaCoord[1][2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                m_creature->SummonCreature(NPC_WRATH_SEAWITCH,  m_afSecondNagaCoord[2][0], m_afSecondNagaCoord[2][1], m_afSecondNagaCoord[2][2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                break;
            case 3:
                m_creature->SummonCreature(NPC_VORSHA, m_fVorshaCoord[0], m_fVorshaCoord[1], m_fVorshaCoord[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                break;
            case 4:
                SetEscortPaused(false);
                DoScriptText(SAY_MUG_DONE, m_creature);
                break;
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        npc_escortAI::UpdateAI(uiDiff);

        if (!m_creature->GetVictim())
        {
            if (HasEscortState(STATE_ESCORT_PAUSED))
            {
                m_uiPausedCheckTimer -= uiDiff;
                if(m_uiPausedCheckTimer <= uiDiff)
                {
                    SetEscortPaused(false);
                    m_creature->Kill(m_creature, false);
                    m_creature->RemoveCorpse();
                }
                

                if(m_bIsBrazierExtinguished)
                {
                    m_uiEventTimer -= uiDiff;
                    if (m_uiEventTimer <= uiDiff)
                    {
                        ++m_uiWaveId;
                        DoWaveSummon();
                        if(m_uiWaveId == 3)
                            m_uiEventTimer += 2000;
                        else
                            m_uiEventTimer += 10000;
                    }
                }
            }
            return;
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_muglash(Creature* pCreature)
{
    return new npc_muglashAI(pCreature);
}

bool QuestAccept_npc_muglash(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_VORSHA)
    {
        if (npc_muglashAI* pEscortAI = CAST_AI(npc_muglashAI, pCreature->AI()))
        {
            DoScriptText(SAY_MUG_START1, pCreature);
            pCreature->setFaction(113);

            pEscortAI->Start(true, true, pPlayer->GetGUID(), pQuest, true);
        }
    }
    return true;
}

bool GOUse_go_naga_brazier(Player* pPlayer, GameObject* pGo)
{
    if (Creature* pCreature = GetClosestCreatureWithEntry(pGo, NPC_MUGLASH, INTERACTION_DISTANCE*2))
    {
        if (npc_muglashAI* pEscortAI = CAST_AI(npc_muglashAI, pCreature->AI()))
        {
            if (pEscortAI->m_bIsBrazierExtinguished)
                return false;
            DoScriptText(SAY_MUG_BRAZIER_WAIT, pCreature);
            pEscortAI->m_bIsBrazierExtinguished = true;
            return false;
        }
    }
    return false;
}

#define NPC_EARTHEN_RING_GUIDE      25324

bool ItemUse_item_Totemic_Beacon(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    float x,y,z;
    player->GetNearPoint(x,y,z, 0.0f, 3.0f, frand(0, 2*M_PI));
    player->SummonCreature(NPC_EARTHEN_RING_GUIDE, x,y,z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 120000);
    return false;
}

#define NPC_ICECALLERBRIATHA        25949

struct npc_Heretic_EmisaryAI : public ScriptedAI
{
    npc_Heretic_EmisaryAI(Creature* c) : ScriptedAI(c) {}

    int32 TalkTimer;
    uint32 Phase;
    int32 Check;
    uint64 player;
    bool EventStarted;

    void Reset()
    {
        Phase = 0;

        TalkTimer = 5000;
        EventStarted = false;
    }

    void MoveInLineOfSight(Unit * unit)
    {
        if(EventStarted)
            return;

        if(unit->GetTypeId() == TYPEID_PLAYER && unit->HasAura(46337, 0) && ((Player*)unit)->GetQuestStatus(11891) == QUEST_STATUS_INCOMPLETE)
        {
            EventStarted = true;
            Phase = 0;
            player = unit->GetGUID();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!me->GetVictim())
        {
            if (EventStarted)
            {
                TalkTimer -= diff;
                if (TalkTimer <= diff)
                {
                    Player * Player_;
                    Creature * Briatha = GetClosestCreatureWithEntry(me, NPC_ICECALLERBRIATHA, 20);

                    if (Briatha && Briatha->IsAlive()) 
                    {
                        switch(Phase)
                        {
                        case 0:
                            Briatha->Say("These stones should be the last of them. Our coordination with Neptulon's forces will be impeccable.", LANG_UNIVERSAL, 0);
                            Phase++;
                            break;
                        case 1:
                            me->Say("Yess. The Tidehunter will be pleased at this development. The Firelord's hold will weaken.", LANG_UNIVERSAL, 0);
                            Phase++;
                            break;
                        case 2:
                            Briatha->Say("And your own preparations? Will the Frost Lord have a path to the portal?", LANG_UNIVERSAL, 0);
                            Phase++;
                            break;
                        case 3:
                            me->Say("Skar'this has informed us well. We have worked our way into the slave pens and await your cryomancerss.", LANG_UNIVERSAL, 0);
                            Phase++;
                            break;
                        case 4:
                            Briatha->Say("The ritual in Coilfang will bring Ahune through once he is fully prepared, and the resulting clash between Firelord and Frostlord will rend the foundations of this world. Our ultimate goals are in reach at last...", LANG_UNIVERSAL, 0);
                            Phase = 0;
                            if(Player_ = (Player*)(me->GetUnit(player)))
                                if(Player_->HasAura(46337, 0))
                                    Player_->AreaExploredOrEventHappens(11891);
                            EventStarted = false;
                            break;
                        }
                        TalkTimer += 5000;
                    }
                    else
                        EventStarted = false;
                }
            }

            return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_Heretic_Emisary(Creature *_Creature)
{
    return new npc_Heretic_EmisaryAI(_Creature);
}

const float feero_adds[][3] = {
    { 3519.252197f,219.959412f,10.116269f },
    { 3521.238037f,216.784348f,9.499805f },
    { 3521.570068f,210.839874f,9.394688f },
    { 3520.265625f,205.178207f,10.311738f },
    { 3809.975830f,132.909821f,9.805327f },
    { 3806.964844f,125.293358f,9.614912f },
    { 3804.356201f,122.136726f,9.610646f },
    { 4253.566406f,132.063339f,40.726017f },
    { 4256.975098f,126.932564f,40.685898f },
    { 4261.208496f,132.154388f,41.457546f }
};

enum efeero
{
    QUEST_SUPPLIES_TO_AUBERDINE = 976,
    SPEECH_FEERO_START = -1000010,
    NPC_DARK_STRAND_ASSASIN = 3879,
    NPC_FORSAKEN_SCOUT = 3893,
    NPC_ALIGAR = 3898,
    NPC_BALIZAR = 3899,
    NPC_CAEDAKAR = 3900
};

struct npc_feero_ironhandAI : public npc_escortAI
{
    npc_feero_ironhandAI(Creature *c) : npc_escortAI(c) {}

    void Reset()
    {
    }

    void SummonAdd(uint32 id, uint8 pos)
    {
        Creature* summon = m_creature->SummonCreature(id, feero_adds[pos][0], feero_adds[pos][1], feero_adds[pos][2],
            0, TEMPSUMMON_DEAD_DESPAWN, 10000);
        if (summon)
        {
            summon->AI()->AttackStart(m_creature);
            if (pos == 8)
                DoScriptText(SPEECH_FEERO_START - 6, summon);
            if (pos == 4)
                DoScriptText(SPEECH_FEERO_START - 3, summon);
        }
    }

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();
        if (!pPlayer)
            return;

        switch (i)
        {
        case 9:
            DoScriptText(SPEECH_FEERO_START, m_creature);
            SummonAdd(NPC_DARK_STRAND_ASSASIN, 0);
            SummonAdd(NPC_DARK_STRAND_ASSASIN, 1);
            SummonAdd(NPC_DARK_STRAND_ASSASIN, 2);
            SummonAdd(NPC_DARK_STRAND_ASSASIN, 3);
            break;
        case 10:
            DoScriptText(SPEECH_FEERO_START - 1, m_creature);
            break;
        case 18:
            DoScriptText(SPEECH_FEERO_START - 2, m_creature);
            SummonAdd(NPC_FORSAKEN_SCOUT, 4);
            SummonAdd(NPC_FORSAKEN_SCOUT, 5);
            SummonAdd(NPC_FORSAKEN_SCOUT, 6);
            break;
        case 19:
            DoScriptText(SPEECH_FEERO_START - 4, m_creature);
            break;
        case 33:
            DoScriptText(SPEECH_FEERO_START - 5, m_creature);
            SummonAdd(NPC_CAEDAKAR, 7);
            SummonAdd(NPC_BALIZAR, 8);
            SummonAdd(NPC_ALIGAR, 9);
            DoScriptText(SPEECH_FEERO_START - 7, m_creature);
            break;
        case 34:
            DoScriptText(SPEECH_FEERO_START - 8, m_creature);
            pPlayer->GroupEventHappens(QUEST_SUPPLIES_TO_AUBERDINE, m_creature);
            break;
        case 35:
            m_creature->DisappearAndDie();
            break;
        }
    }
    
};

bool QuestAccept_npc_feero_ironhand(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_SUPPLIES_TO_AUBERDINE)
    {
        pCreature->setFaction(113);

        if (npc_escortAI* pEscortAI = CAST_AI(npc_feero_ironhandAI, (pCreature->AI())))
            pEscortAI->Start(true, true, pPlayer->GetGUID(), quest);
    }
    return true;
}
CreatureAI* GetAI_npc_feero_ironhand(Creature* c)
{
    return new npc_feero_ironhandAI(c);
}


void AddSC_ashenvale()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_torek";
    newscript->GetAI = &GetAI_npc_torek;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_torek;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_ruul_snowhoof";
    newscript->GetAI = &GetAI_npc_ruul_snowhoofAI;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_ruul_snowhoof;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_muglash";
    newscript->GetAI = &GetAI_npc_muglash;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_muglash;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_naga_brazier";
    newscript->pGOUse = &GOUse_go_naga_brazier;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="item_Totemic_Beacon";
    newscript->pItemUse = &ItemUse_item_Totemic_Beacon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_Heretic_Emisary";
    newscript->GetAI = &GetAI_npc_Heretic_Emisary;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_feero_ironhand";
    newscript->GetAI = &GetAI_npc_feero_ironhand;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_feero_ironhand;
    newscript->RegisterSelf();
}
