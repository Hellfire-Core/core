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

#include "scriptPCH.h"

enum
{
    ENTRY_SHAMBLER = 12224,
    ENTRY_MESHLOK = 12237,
    SPELL_HEAL = 7984,
    SPELL_KNOCK = 16790
};

struct mob_cavern_shamblerAI : public ScriptedAI
{
    mob_cavern_shamblerAI(Creature *c) : ScriptedAI(c) { rarespawn = (urand(0, 100) > 80); } // 20% chances to be meshlok

    bool rarespawn;
    Timer healTimer;
    Timer knockTimer;
    void Reset()
    {
        if (rarespawn && m_creature->GetEntry() == ENTRY_SHAMBLER)
            m_creature->UpdateEntry(ENTRY_MESHLOK);
        healTimer.Reset(10000);
        knockTimer.Reset(5000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (healTimer.Expired(diff))
        {
            DoCast(m_creature, SPELL_HEAL);
            healTimer = 12000;
        }
        
        if (knockTimer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_KNOCK);
            knockTimer = 7000;
        }
        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_mob_cavern_shambler(Creature *_Creature)
{
    return new mob_cavern_shamblerAI(_Creature);
}

void AddSC_maraudon()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "mob_cavern_shambler";
    newscript->GetAI = &GetAI_mob_cavern_shambler;
    newscript->RegisterSelf();
}

