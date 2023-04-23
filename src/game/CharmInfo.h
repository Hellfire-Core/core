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

#ifndef _CHARMINFO_H
#define _CHARMINFO_H

#include "Common.h"
#include "Unit.h"

struct SpellEntry;

enum ReactStates
{
    REACT_PASSIVE    = 0,
    REACT_DEFENSIVE  = 1,
    REACT_AGGRESSIVE = 2
};

enum CommandStates
{
    COMMAND_STAY    = 0,
    COMMAND_FOLLOW  = 1,
    COMMAND_ATTACK  = 2,
    COMMAND_ABANDON = 3
};

struct CharmSpellEntry
{
    uint16 spellId;
    uint16 active;
};

enum ActiveStates
{
    ACT_ENABLED  = 0xC100,
    ACT_DISABLED = 0x8100,
    ACT_COMMAND  = 0x0700,
    ACT_REACTION = 0x0600,
    ACT_CAST     = 0x0100,
    ACT_PASSIVE  = 0x0000,
    ACT_DECIDE   = 0x0001
};

struct UnitActionBarEntry
{
    uint32 Type;
    uint32 SpellOrAction;
};

struct CharmInfo
{
    public:
        explicit CharmInfo(Unit* unit);
        ~CharmInfo();
        uint32 GetPetNumber() const { return m_petnumber; }
        void SetPetNumber(uint32 petnumber, bool statwindow);

        void SetCommandState(CommandStates st) { m_CommandState = st; }
        CommandStates GetCommandState() { return m_CommandState; }
        bool HasCommandState(CommandStates state) { return (m_CommandState == state); }

        void InitPossessCreateSpells();
        void InitCharmCreateSpells();
        void InitPetActionBar();
        void InitEmptyActionBar(bool withAttack = true);
                                                            //return true if successful
        bool AddSpellToActionBar(uint32 oldid, uint32 newid, ActiveStates newstate = ACT_DECIDE);
        void ToggleCreatureAutocast(uint32 spellid, bool apply);

        UnitActionBarEntry* GetActionBarEntry(uint8 index) { return &(PetActionBar[index]); }
        CharmSpellEntry* GetCharmSpell(uint8 index) { return &(m_charmspells[index]); }

        void HandleStayCommand();
        void HandleFollowCommand();
        void HandleAttackCommand(uint64 targetGUID);
        void HandleSpellActCommand(uint64 targetGUID, uint32 spellId);

    private:
        Unit* m_unit;
        UnitActionBarEntry PetActionBar[10];
        CharmSpellEntry m_charmspells[CREATURE_MAX_SPELLS];

        CommandStates   m_CommandState;

        uint32          m_petnumber;
        bool            m_barInit;

        //for restoration after charmed
        ReactStates     m_oldReactState;
};

#endif
