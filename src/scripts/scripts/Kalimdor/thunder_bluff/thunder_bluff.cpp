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
SDName: Thunder_Bluff
SD%Complete: 100
SDComment: Quest support: 925
SDCategory: Thunder Bluff
EndScriptData */

#include "precompiled.h"

/*#####
# npc_cairne_bloodhoof
######*/

#define SPELL_BERSERKER_CHARGE  16636
#define SPELL_CLEAVE            16044
#define SPELL_MORTAL_STRIKE     16856
#define SPELL_THUNDERCLAP       23931
#define SPELL_UPPERCUT          22916

#define GOSSIP_HCB "I know this is rather silly but a young ward who is a bit shy would like your hoofprint."
//TODO: verify abilities/timers
struct npc_cairne_bloodhoofAI : public ScriptedAI
{
    npc_cairne_bloodhoofAI(Creature* c) : ScriptedAI(c) {}

    Timer BerserkerCharge_Timer;
    Timer Cleave_Timer;
    Timer MortalStrike_Timer;
    Timer Thunderclap_Timer;
    Timer Uppercut_Timer;

    void Reset()
    {
        BerserkerCharge_Timer.Reset(30000);
        Cleave_Timer.Reset(5000);
        MortalStrike_Timer.Reset(10000);
        Thunderclap_Timer.Reset(15000);
        Uppercut_Timer.Reset(10000);
    }

    void EnterCombat(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (BerserkerCharge_Timer.Expired(diff))
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0,GetSpellMaxRange(SPELL_BERSERKER_CHARGE), true);
            if( target )
                DoCast(target,SPELL_BERSERKER_CHARGE);
            BerserkerCharge_Timer = 25000;
        }

        if (Uppercut_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_UPPERCUT);
            Uppercut_Timer = 20000;
        }

        if (Thunderclap_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_THUNDERCLAP);
            Thunderclap_Timer = 15000;
        }

        if (MortalStrike_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_MORTAL_STRIKE);
            MortalStrike_Timer = 15000;
        }

        if (Cleave_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CLEAVE);
            Cleave_Timer = 7000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_cairne_bloodhoof(Creature *_Creature)
{
    return new npc_cairne_bloodhoofAI (_Creature);
}

bool GossipHello_npc_cairne_bloodhoof(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if( player->GetQuestStatus(925) == QUEST_STATUS_INCOMPLETE )
        player->ADD_GOSSIP_ITEM( 0, GOSSIP_HCB, GOSSIP_SENDER_MAIN, GOSSIP_SENDER_INFO );

    player->SEND_GOSSIP_MENU(7013, _Creature->GetGUID() );

    return true;
}

bool GossipSelect_npc_cairne_bloodhoof(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_SENDER_INFO )
    {
        player->CastSpell(player, 23123, false);
        player->SEND_GOSSIP_MENU(7014, _Creature->GetGUID() );
    }
    return true;
}

void AddSC_thunder_bluff()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_cairne_bloodhoof";
    newscript->GetAI = &GetAI_npc_cairne_bloodhoof;
    newscript->pGossipHello = &GossipHello_npc_cairne_bloodhoof;
    newscript->pGossipSelect = &GossipSelect_npc_cairne_bloodhoof;
    newscript->RegisterSelf();
}

