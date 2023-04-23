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

#ifndef INSTANCE_KARAZHAN
#define INSTANCE_KARAZHAN

#include "def_karazhan.h"

#define ENCOUNTERS      15

/*
0  - Attumen + Midnight (optional)
1  - Moroes
2  - Maiden of Virtue (optional)
3  - Hyakiss the Lurker /  Rokad the Ravager  / Shadikith the Glider  (optional)
4  - Opera Event
5  - Curator
6  - Shade of Aran (optional)
7  - Terestian Illhoof (optional)
8  - Netherspite (optional)
9  - Chess Event
10 - Prince Malchezzar
11 - Nightbane  (optional)
12 - Dust Covered Chest

13 - Chess Event Team
14 - Chess Event damage done
*/

struct instance_karazhan : public ScriptedInstance
{
    instance_karazhan(Map* map);

    uint32 Encounters[ENCOUNTERS];

    uint32 OperaEvent;
    uint32 OzDeathCount;

    uint64 CurtainGUID;
    uint64 StageDoorLeftGUID;
    uint64 StageDoorRightGUID;
    uint64 KilrekGUID;
    uint64 TerestianGUID;
    uint64 NightbaneGUID;
    uint64 MoroesGUID;
    uint64 LibraryDoor;                                     // Door at Shade of Aran
    uint64 MassiveDoor;                                     // Door at Netherspite
    uint64 GamesmansDoor;                                   // Door before Chess
    uint64 GamesmansExitDoor;                               // Door after Chess
    uint64 NetherspaceDoor;                                 // Door at Malchezaar
    uint64 SideEntranceDoor;                                // Door after side entrance
    uint64 ServentAccessDoor;                               // Door on shortcut from Guest Chambers to Broken Stairs
    uint64 MastersTerraceDoor[2];
    uint64 ImageGUID;
    uint64 AranGUID;
    uint64 MedivhGUID;
    uint64 BarnesGUID;
    uint64 BlizzardGUID;
    uint64 AnimalBossGUID[3];

    Timer CheckTimer;
    Timer AnimalBossCheck;
    std::list<uint64> animalBossTrashList;
    std::list<uint64> forChessList;

    bool needRespawn;

    void Initialize();

    bool IsEncounterInProgress() const;

    uint32 GetData(uint32 identifier);

    void OnCreatureCreate(Creature *creature, uint32 entry);

    uint64 GetData64(uint32 data);

    void SetData(uint32 type, uint32 data);

    void SetData64(uint32 identifier, uint64 data);

    void OnObjectCreate(GameObject* go);

    std::string GetSaveData();

    void Load(const char* in);

    void HandleInitCreatureState(Creature * mob);

    void Update(uint32 diff);
};

#endif
