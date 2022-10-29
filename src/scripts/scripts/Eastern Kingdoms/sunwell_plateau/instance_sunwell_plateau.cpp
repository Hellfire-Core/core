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
SDName: Instance_Sunwell_Plateau
SD%Complete: 75
SDComment: Muru testing
SDCategory: Sunwell_Plateau
EndScriptData */

#include "precompiled.h"
#include "def_sunwell_plateau.h"

#define ENCOUNTERS 7

uint32 GauntletNPC[6] =
{
    25484,
    25837,
    25506,
    25483,
    25373,
    25486
};

/* Sunwell Plateau:
0 - Kalecgos and Sathrovarr
1 - Brutallus & Madrigosa intro
2 - Brutallus
3 - Felmyst
4 - Eredar Twins (Alythess and Sacrolash)
5 - M'uru
6 - Kil'Jaeden
7 - Trash gauntlet event
*/

struct instance_sunwell_plateau : public ScriptedInstance
{
    instance_sunwell_plateau(Map *map) : ScriptedInstance(map), m_gbk(map) {Initialize();};

    uint32 Encounters[ENCOUNTERS];

    /** Creatures **/
    uint64 Kalecgos_Dragon;
    uint64 Kalecgos_Human;
    uint64 Sathrovarr;
    uint64 Brutallus;
    uint64 Madrigosa;
    uint64 BrutallusTrigger;
    uint64 Felmyst;
    uint64 Alythess;
    uint64 Sacrolash;
    uint64 Muru;
    uint64 KilJaeden;
    uint64 KilJaedenController;
    uint64 Anveena;
    uint64 KalecgosKJ;

    /** GameObjects **/
    uint64 ForceField;                                      // Kalecgos Encounter
    uint64 Collision_1;                                     // Kalecgos Encounter
    uint64 Collision_2;                                     // Kalecgos Encounter
    uint64 FireBarrier;                                     // Brutallus Encounter
    uint64 IceBarrier;                                      // Brutallus Encounter
    uint64 Gate[4];
    uint64 Archonisus;                                      // Archonisus, the second gate

    /*** Misc ***/
    uint32 KalecgosPhase;
    uint32 GauntletProgress;
    uint32 EredarTwinsIntro;
    uint32 HandOfDeceiverCount;

    uint32 EredarTwinsAliveInfo[2];
    GBK_handler m_gbk;

    void Initialize()
    {
        /*** Creatures ***/
        Kalecgos_Dragon         = 0;
        Kalecgos_Human          = 0;
        Sathrovarr              = 0;
        Brutallus               = 0;
        Madrigosa               = 0;
        BrutallusTrigger        = 0;
        Felmyst                 = 0;
        Alythess                = 0;
        Sacrolash               = 0;
        Muru                    = 0;
        HandOfDeceiverCount     = 1;

        /*** GameObjects ***/
        ForceField  = 0;
        Collision_1 = 0;
        Collision_2 = 0;
        FireBarrier = 0;
        IceBarrier = 0;
        for (uint8 i = 0; i < 3; ++i)
            Gate[i]  = 0;

        EredarTwinsAliveInfo[0] = 0;
        EredarTwinsAliveInfo[1] = 0;

        EredarTwinsIntro = NOT_STARTED;

        /*** Encounters ***/
        for(uint8 i = 0; i < ENCOUNTERS; ++i)
            Encounters[i] = NOT_STARTED;
        GauntletProgress = NOT_STARTED;

        requiredEncounterToMobs.clear();
    }

    bool IsEncounterInProgress() const
    {
        for(uint8 i = 0; i < ENCOUNTERS; ++i)
            if(Encounters[i] == IN_PROGRESS)
            {
                if(i == 1)
                    continue;
                return true;
            }
        return false;
    }

    Player* GetPlayerInMap()
    {
        Map::PlayerList const& players = instance->GetPlayers();

        if (!players.isEmpty())
        {
            for(Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                Player* plr = itr->getSource();
                if (plr && !plr->HasAura(45839,0))
                    return plr;
            }
        }

        debug_log("TSCR: Instance Sunwell Plateau: GetPlayerInMap, but PlayerList is empty!");
        return NULL;
    }

    void HandleGameObject(uint64 guid, uint32 state)
    {
        if (!guid)
            return;
        
        if (GameObject *go = instance->GetGameObject(guid))
            go->SetGoState(GOState(state));
    }

    void RespawnGauntletGroups()
    {
        for (uint8 i = 0; i < 6; ++i)
        {
            std::list<uint64> GauntletList = instance->GetCreaturesGUIDList(GauntletNPC[i]);
            if(!GauntletList.empty())
            {
                for(std::list<uint64>::iterator itr = GauntletList.begin(); itr != GauntletList.end(); ++itr)
                {
                    if(Creature* c = instance->GetCreature(*itr))
                        c->Respawn();
                }
            }
        }
    }

    uint32 GetEncounterForEntry(uint32 entry)
    {
        switch(entry)
        {
            case 25315:
                return DATA_KILJAEDEN_EVENT;
            case 25741:
            case 25840:
                return DATA_MURU_EVENT;
            case 25166:
            case 25165:
                return DATA_EREDAR_TWINS_EVENT;
            case 25038:
                return DATA_FELMYST_EVENT;
            case 24882:
                return DATA_BRUTALLUS_EVENT;
            case 24850:
            case 24892:
                return DATA_KALECGOS_EVENT;
            default:
                return 0;
        }
    }

    uint32 GetRequiredEncounterForEntry(uint32 entry)
    {
        switch (entry)
        {
            // Kil'jeaden
            case 25315:
                return DATA_MURU_EVENT;
            // M'uru
            case 25741:
            case 25840:
                return DATA_EREDAR_TWINS_EVENT;
            // Eredar Twins
            case 25166:
            case 25165:
                return DATA_FELMYST_EVENT;
            default:
                return 0;
        }
    }

    void OnCreatureCreate(Creature* creature, uint32 entry)
    {
        switch(entry)
        {
            case 24850: Kalecgos_Dragon     = creature->GetGUID(); break;
            case 24891: Kalecgos_Human      = creature->GetGUID(); break;
            case 24892:
                Sathrovarr = creature->GetGUID();
                if(GetData(DATA_KALECGOS_EVENT) == DONE)
                    creature->NearTeleportTo(1705, 927, 53.07, creature->GetOrientation());
                break;
            case 24882: Brutallus           = creature->GetGUID(); break;
            case 25166: Alythess            = creature->GetGUID(); break;
            case 25165: Sacrolash           = creature->GetGUID(); break;
            case 25741: Muru                = creature->GetGUID(); break;
            case 25038: Felmyst             = creature->GetGUID(); break;
            case 24895:
                Madrigosa = creature->GetGUID();
                if(GetData(DATA_BRUTALLUS_INTRO_EVENT) == DONE)
                {
                    creature->GetMap()->CreatureRelocation(creature, 1476.3, 649, 21.5, creature->GetOrientation());
                    // spawn Felmyst when needed
                    if(GetData(DATA_BRUTALLUS_EVENT) == DONE)
                    {
                        // summon Felmyst
                        if(!Felmyst && GetData(DATA_FELMYST_EVENT) != DONE)
                        {
                            creature->CastSpell(creature, 45069, true);
                            if(Unit* Fel = FindCreature(45069, 20, creature))
                                Felmyst = Fel->GetGUID();
                        }
                        // truly kill Madrigosa when Felmyst spawned
                        creature->Kill(creature, false);
                        creature->RemoveCorpse();
                        break;
                    }
                    else
                    {
                        creature->SetLevitate(false);
                        creature->setFaction(35);
                        creature->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                        creature->SetFlag(UNIT_DYNAMIC_FLAGS, (UNIT_DYNFLAG_DEAD | UNIT_FLAG_SPAWNING | UNIT_FLAG_PACIFIED));
                    }
                }
                else
                    creature->SetReactState(REACT_PASSIVE);
                break;
            case 19871: BrutallusTrigger    = creature->GetGUID(); break;
        }

        HandleInitCreatureState(creature);
    }

    void OnObjectCreate(GameObject* gobj)
    {
        switch(gobj->GetEntry())
        {
            case 188421: ForceField     = gobj->GetGUID(); break;
            case 188523: Collision_1    = gobj->GetGUID(); break;
            case 188524: Collision_2    = gobj->GetGUID(); break;
            case 188075: 
                FireBarrier    = gobj->GetGUID();
                if(GetData(DATA_FELMYST_EVENT) == DONE)
                    HandleGameObject(FireBarrier, GO_STATE_ACTIVE);
                break;
            case 188119: IceBarrier     = gobj->GetGUID();
                if (GetData(DATA_BRUTALLUS_INTRO_EVENT) == DONE)
                    HandleGameObject(IceBarrier, GO_STATE_ACTIVE);
                break;
            // Eredar Twins Up - door 4
            case 187770:
                Gate[0] = gobj->GetGUID();
                if(GetData(DATA_EREDAR_TWINS_EVENT) == DONE)
                    HandleGameObject(Gate[0], GO_STATE_ACTIVE);
                break;
            case 187990: // door 7
                if (gobj->GetPositionZ() > 60.0f) // M'uru - entrance
                {
                    Gate[1] = gobj->GetGUID();
                    if (GetData(DATA_EREDAR_TWINS_EVENT) == DONE)
                        HandleGameObject(Gate[1], GO_STATE_ACTIVE);
                }
                else    // Eredar Twins Down
                {
                    Gate[2] = gobj->GetGUID();
                    if(GetData(DATA_EREDAR_TWINS_EVENT) == DONE)
                        HandleGameObject(Gate[2], GO_STATE_ACTIVE);
                }
                break;
            case 188118: // door 8 - Muru ramp to Kil'jaeden
                Gate[3] = gobj->GetGUID();
                if(GetData(DATA_MURU_EVENT) == DONE)
                    HandleGameObject(Gate[3], GO_STATE_ACTIVE);
                break;
            case 187765:
                Archonisus = gobj->GetGUID();
                break;
        }
    }

    uint32 GetData(uint32 id)
    {
        switch(id)
        {
            case DATA_KALECGOS_EVENT:           return Encounters[0];
            case DATA_BRUTALLUS_INTRO_EVENT:    return Encounters[1];
            case DATA_BRUTALLUS_EVENT:          return Encounters[2];
            case DATA_FELMYST_EVENT:            return Encounters[3];
            case DATA_EREDAR_TWINS_EVENT:       return Encounters[4];
            case DATA_MURU_EVENT:               return Encounters[5];
            case DATA_KILJAEDEN_EVENT:          return Encounters[6];
            case DATA_TRASH_GAUNTLET_EVENT:     return GauntletProgress;
            case DATA_KALECGOS_PHASE:           return KalecgosPhase;
            case DATA_ALYTHESS:                 return EredarTwinsAliveInfo[0];
            case DATA_SACROLASH:                return EredarTwinsAliveInfo[1];
            case DATA_EREDAR_TWINS_INTRO:       return EredarTwinsIntro;
            case DATA_HAND_OF_DECEIVER_COUNT:   return HandOfDeceiverCount;
        }

        return 0;
    }

    uint64 GetData64(uint32 id)
    {
        switch(id)
        {
            case DATA_KALECGOS_DRAGON:      return Kalecgos_Dragon;
            case DATA_KALECGOS_HUMAN:       return Kalecgos_Human;
            case DATA_SATHROVARR:           return Sathrovarr;
            case DATA_BRUTALLUS:            return Brutallus;
            case DATA_MADRIGOSA:            return Madrigosa;
            case DATA_BRUTALLUS_TRIGGER:    return BrutallusTrigger;
            case DATA_FELMYST:              return Felmyst;
            case DATA_ALYTHESS:             return Alythess;
            case DATA_SACROLASH:            return Sacrolash;
            case DATA_MURU:                 return Muru;
            case DATA_PLAYER_GUID:
                if (Player* Target = GetPlayerInMap())
                    return Target->GetGUID();
                break;
        }

        return 0;
    }
    
    GBK_Encounters EncounterForGBK(uint32 enc)
    {
        switch (enc)
        {
        case DATA_KALECGOS_EVENT:           return GBK_KALECGOS;
        case DATA_BRUTALLUS_EVENT:          return GBK_BRUTALLUS;
        case DATA_FELMYST_EVENT:            return GBK_FELMYST;
        case DATA_EREDAR_TWINS_EVENT:       return GBK_HOT_EREDAR_CHICKS;
        case DATA_MURU_EVENT:               return GBK_MURU;
        case DATA_KILJAEDEN_EVENT:          return GBK_KILJAEDEN;
        }
        return GBK_NONE;
    }

    void SetData(uint32 id, uint32 data)
    {
        switch(id)
        {
            case DATA_KALECGOS_EVENT:
                if(Encounters[0] != DONE)
                {
                    if(data == IN_PROGRESS)
                    {
                        HandleGameObject(ForceField, GO_STATE_READY);
                        HandleGameObject(Collision_1, GO_STATE_READY);
                        HandleGameObject(Collision_2, GO_STATE_READY);
                    }
                    else
                    {
                        HandleGameObject(ForceField, GO_STATE_ACTIVE);
                        HandleGameObject(Collision_1, GO_STATE_ACTIVE);
                        HandleGameObject(Collision_2, GO_STATE_ACTIVE);
                    }
                    Encounters[0] = data;
                }
                break;
            case DATA_BRUTALLUS_INTRO_EVENT:
                if(Encounters[1] != DONE)
                    Encounters[1] = data;
                switch(data)
                {
                    case IN_PROGRESS:
                        HandleGameObject(IceBarrier, GO_STATE_READY);
                        break;
                    case DONE:
                        HandleGameObject(IceBarrier, GO_STATE_ACTIVE);
                        break;
                }
                break;
            case DATA_BRUTALLUS_EVENT:
                if(Encounters[2] != DONE)
                    Encounters[2] = data;
                break;
            case DATA_FELMYST_EVENT:
                if(data == DONE)
                    HandleGameObject(FireBarrier, GO_STATE_ACTIVE);
                if(Encounters[3] != DONE)
                    Encounters[3] = data;
                break;
            case DATA_EREDAR_TWINS_EVENT:
                if(Encounters[4] != DONE)
                {
                    if(data == IN_PROGRESS)
                        HandleGameObject(Gate[0], GO_STATE_READY);
                    else
                        HandleGameObject(Gate[0], GO_STATE_ACTIVE);
                    Encounters[4] = data;
                }
                if(data == DONE)
                {
                    HandleGameObject(Gate[0], GO_STATE_ACTIVE);
                    HandleGameObject(Gate[2], GO_STATE_ACTIVE);
                    if(Player* pl = GetPlayerInMap())
                    {
                        if(Unit* muru = pl->GetUnit(Muru))
                        {
                            muru->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                            muru->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
                            muru->SetVisibility(VISIBILITY_ON);
                        }
                    }
                }
                break;
            case DATA_MURU_EVENT:
                if(Encounters[5] != DONE)
                {
                    switch(data)
                    {
                        case IN_PROGRESS:
                            HandleGameObject(Gate[1], GO_STATE_READY);
                            HandleGameObject(Gate[3], GO_STATE_READY);
                            break;
                        case NOT_STARTED:
                            HandleGameObject(Gate[3], GO_STATE_READY);
                            HandleGameObject(Gate[1], GO_STATE_ACTIVE);
                            break;
                    }
                    Encounters[5] = data;
                }
                if(data == DONE)
                {
                    HandleGameObject(Gate[1], GO_STATE_ACTIVE);
                    HandleGameObject(Gate[3], GO_STATE_ACTIVE);

                    // prepare KJ testing data here
                }
                break;
            case DATA_KILJAEDEN_EVENT:
                if(Encounters[6] != DONE)
                    Encounters[6] = data;
                break;
            case DATA_TRASH_GAUNTLET_EVENT:
                GauntletProgress = data;
                if(data == FAIL)
                {
                    RespawnGauntletGroups();
                    data = NOT_STARTED;
                }
                break;
            case DATA_KALECGOS_PHASE:
                KalecgosPhase = data;
                break;
            case DATA_ALYTHESS:
                EredarTwinsAliveInfo[0] = data;
                if (data == DONE && IsEncounterInProgress())
                {
                    if (Creature *pSacrolash = GetCreature(GetData64(DATA_SACROLASH)))
                        pSacrolash->AI()->DoAction(SISTER_DEATH);
                }
                return;
            case DATA_SACROLASH:
                EredarTwinsAliveInfo[1] = data;
                if (data == DONE && IsEncounterInProgress())
                {
                    if (Creature *pAlythess = GetCreature(GetData64(DATA_ALYTHESS)))
                        pAlythess->AI()->DoAction(SISTER_DEATH);
                }
                return;
            case DATA_EREDAR_TWINS_INTRO:
                EredarTwinsIntro = data;
                if (data == DONE)
                {
                    if (Creature *pAlythess = GetCreature(GetData64(DATA_ALYTHESS)))
                        pAlythess->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
                    if (Creature *pSacrolash = GetCreature(GetData64(DATA_SACROLASH)))
                        pSacrolash->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
                }
                break;
            case DATA_HAND_OF_DECEIVER_COUNT:
                HandOfDeceiverCount = data;
                break;
        }

        GBK_Encounters gbkEnc = EncounterForGBK(id);
        if (gbkEnc != GBK_NONE)
        {
            if (data == DONE)
                m_gbk.StopCombat(gbkEnc, true);
            else if (data == NOT_STARTED)
                m_gbk.StopCombat(gbkEnc, false);
            else if (data == IN_PROGRESS)
                m_gbk.StartCombat(gbkEnc);
        }

        if(data == DONE || data == FAIL)
            SaveToDB();

        HandleRequiredEncounter(id);
    }

    void SetData64(uint32 id, uint64 guid)
    {
    }

    void Update(uint32 diff)
    {
    }

    std::string GetSaveData()
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream stream;
        stream << Encounters[0] << " ";
        stream << Encounters[1] << " ";
        stream << Encounters[2] << " ";
        stream << Encounters[3] << " ";
        stream << Encounters[4] << " ";
        stream << Encounters[5] << " ";
        stream << Encounters[6];

        OUT_SAVE_INST_DATA_COMPLETE;

        return stream.str();
    }

    void Load(const char* in)
    {
        if(!in)
        {
            OUT_LOAD_INST_DATA_FAIL;
            return;
        }

        OUT_LOAD_INST_DATA(in);
        std::istringstream stream(in);
        stream >> Encounters[0] >> Encounters[1] >> Encounters[2] >> Encounters[3]
            >> Encounters[4] >> Encounters[5] >> Encounters[6];
        for(uint8 i = 0; i < ENCOUNTERS; ++i)
            if(Encounters[i] == IN_PROGRESS)                // Do not load an encounter as "In Progress" - reset it instead.
                Encounters[i] = NOT_STARTED;
        OUT_LOAD_INST_DATA_COMPLETE;
    }

    void OnPlayerDealDamage(Player* plr, uint32 amount)
    {
        m_gbk.DamageDone(plr->GetGUIDLow(), amount);
    }

    void OnPlayerHealDamage(Player* plr, uint32 amount)
    {
        m_gbk.HealingDone(plr->GetGUIDLow(), amount);
    }

    void OnPlayerDeath(Player* plr)
    {
        m_gbk.PlayerDied(plr->GetGUIDLow());
    }
};

InstanceData* GetInstanceData_instance_sunwell_plateau(Map* map)
{
    return new instance_sunwell_plateau(map);
}

void AddSC_instance_sunwell_plateau()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_sunwell_plateau";
    newscript->GetInstanceData = &GetInstanceData_instance_sunwell_plateau;
    newscript->RegisterSelf();
}

