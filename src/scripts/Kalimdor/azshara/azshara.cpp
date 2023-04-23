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
SDName: Azshara
SD%Complete: 90
SDComment: Quest support: 2744, 3141, 9364, 10994
SDCategory: Azshara
EndScriptData */

/* ContentData
mobs_spitelashes
npc_loramus_thalipedes
mob_rizzle_sprysprocket
mob_depth_charge
mob_spirit_of_azuregos
EndContentData */

#include "scriptPCH.h"

/*######
## mobs_spitelashes
######*/

struct mobs_spitelashesAI : public ScriptedAI
{
    mobs_spitelashesAI(Creature *c) : ScriptedAI(c) {}

    uint32 morphtimer;
    bool spellhit;

    void Reset()
    {
        morphtimer = 0;
        spellhit = false;
    }

    void EnterCombat(Unit *who) { }

    void SpellHit(Unit *Hitter, const SpellEntry *Spellkind)
    {
        if( !spellhit &&
            Hitter->GetTypeId() == TYPEID_PLAYER &&
            ((Player*)Hitter)->GetQuestStatus(9364) == QUEST_STATUS_INCOMPLETE &&
            (Spellkind->Id==118 || Spellkind->Id== 12824 || Spellkind->Id== 12825 || Spellkind->Id== 12826) )
        {
            spellhit=true;
            DoCast(m_creature,29124);                       //become a sheep
        }
    }

    void UpdateAI(const uint32 diff)
    {
        // we mustn't remove the creature in the same round in which we cast the summon spell, otherwise there will be no summons
        if( spellhit && morphtimer>=5000 )
        {
            m_creature->DealDamage(m_creature, m_creature->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            m_creature->RemoveCorpse();                     //you don't see any corpse on off.
            EnterEvadeMode();                               //spellhit will be set to false
            return;
        }
        // walk 5 seconds before summoning
        if( spellhit && morphtimer<5000 )
        {
            morphtimer+=diff;
            if( morphtimer>=5000 )
            {
                DoCast(m_creature,28406);                   //summon copies
                DoCast(m_creature,6924);                    //visual explosion
            }
        }
        if (!UpdateVictim() )
            return;

        //TODO: add abilities for the different creatures
        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_mobs_spitelashes(Creature *_Creature)
{
    return new mobs_spitelashesAI (_Creature);
}

/*######
## npc_loramus_thalipedes
######*/

bool GossipHello_npc_loramus_thalipedes(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestStatus(2744) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, "Can you help me?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    if (player->GetQuestStatus(3141) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, "Tell me your story", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_loramus_thalipedes(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(2744);
            break;

        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, "Please continue", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 21);
            player->SEND_GOSSIP_MENU(1813, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+21:
            player->ADD_GOSSIP_ITEM( 0, "I do not understand", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 22);
            player->SEND_GOSSIP_MENU(1814, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+22:
            player->ADD_GOSSIP_ITEM( 0, "Indeed", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 23);
            player->SEND_GOSSIP_MENU(1815, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+23:
            player->ADD_GOSSIP_ITEM( 0, "I will do this with or your help, Loramus", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 24);
            player->SEND_GOSSIP_MENU(1816, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+24:
            player->ADD_GOSSIP_ITEM( 0, "Yes", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 25);
            player->SEND_GOSSIP_MENU(1817, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+25:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(3141);
            break;
    }
    return true;
}

/*####
# mob_rizzle_sprysprocket
####*/

#define MOB_DEPTH_CHARGE 23025
#define SPELL_RIZZLE_BLACKJACK 39865
#define SPELL_RIZZLE_ESCAPE 39871
#define SPELL_RIZZLE_FROST_GRENADE 40525
#define SPELL_DEPTH_CHARGE_TRAP 38576
#define SPELL_PERIODIC_DEPTH_CHARGE 39912
#define SPELL_GIVE_SOUTHFURY_MOONSTONE 39886

#define SAY_RIZZLE_START -1000245
#define SAY_RIZZLE_GRENADE -1000246
#define SAY_RIZZLE_FINAL -1000247

#define GOSSIP_GET_MOONSTONE "Hand over the Southfury moonstone and I'll let you go."
#define MSG_ESCAPE_NOTICE "takes the Southfury moonstone and escapes into the river. Follow her!"

float WPs[58][4] =
{
//pos_x   pos_y     pos_z    orien
{3691.97f, -3962.41f, 35.9118f, 3.67f},
{3675.02f, -3960.49f, 35.9118f, 3.67f},
{3653.19f, -3958.33f, 33.9118f, 3.59f},
{3621.12f, -3958.51f, 29.9118f, 3.48f},
{3604.86f, -3963.0f,  29.9118f, 3.48f},
{3569.94f, -3970.25f, 29.9118f, 3.44f},
{3541.03f, -3975.64f, 29.9118f, 3.41f},
{3510.84f, -3978.71f, 29.9118f, 3.41f},
{3472.7f,  -3997.07f, 29.9118f, 3.35f},
{3439.15f, -4014.55f, 29.9118f, 3.29f},
{3412.8f,  -4025.87f, 29.9118f, 3.25f},
{3384.95f, -4038.04f, 29.9118f, 3.24f},
{3346.77f, -4052.93f, 29.9118f, 3.22f},
{3299.56f, -4071.59f, 29.9118f, 3.20f},
{3261.22f, -4080.38f, 30.9118f, 3.19f},
{3220.68f, -4083.09f, 31.9118f, 3.18f},
{3187.11f, -4070.45f, 33.9118f, 3.16f},
{3162.78f, -4062.75f, 33.9118f, 3.15f},
{3136.09f, -4050.32f, 33.9118f, 3.07f},
{3119.47f, -4044.51f, 36.0363f, 3.07f},
{3098.95f, -4019.8f,  33.9118f, 3.07f},
{3073.07f, -4011.42f, 33.9118f, 3.07f},
{3051.71f, -3993.37f, 33.9118f, 3.02f},
{3027.52f, -3978.6f,  33.9118f, 3.00f},
{3003.78f, -3960.14f, 33.9118f, 2.98f},
{2977.99f, -3941.98f, 31.9118f, 2.96f},
{2964.57f, -3932.07f, 30.9118f, 2.96f},
{2947.9f,  -3921.31f, 29.9118f, 2.96f},
{2924.91f, -3910.8f,  29.9118f, 2.94f},
{2903.04f, -3896.42f, 29.9118f, 2.93f},
{2884.75f, -3874.03f, 29.9118f, 2.90f},
{2868.19f, -3851.48f, 29.9118f, 2.82f},
{2854.62f, -3819.72f, 29.9118f, 2.80f},
{2825.53f, -3790.4f,  29.9118f, 2.744f},
{2804.31f, -3773.05f, 29.9118f, 2.71f},
{2769.78f, -3763.57f, 29.9118f, 2.70f},
{2727.23f, -3745.92f, 30.9118f, 2.69f},
{2680.12f, -3737.49f, 30.9118f, 2.67f},
{2647.62f, -3739.94f, 30.9118f, 2.66f},
{2616.6f,  -3745.75f, 30.9118f, 2.64f},
{2589.38f, -3731.97f, 30.9118f, 2.61f},
{2562.94f, -3722.35f, 31.9118f, 2.56f},
{2521.05f, -3716.6f,  31.9118f, 2.55f},
{2485.26f, -3706.67f, 31.9118f, 2.51f},
{2458.93f, -3696.67f, 31.9118f, 2.51f},
{2432.0f,  -3692.03f, 31.9118f, 2.46f},
{2399.59f, -3681.97f, 31.9118f, 2.45f},
{2357.75f, -3666.6f,  31.9118f, 2.44f},
{2311.99f, -3656.88f, 31.9118f, 2.94f},
{2263.41f, -3649.55f, 31.9118f, 3.02f},
{2209.05f, -3641.76f, 31.9118f, 2.99f},
{2164.83f, -3637.64f, 31.9118f, 3.15f},
{2122.42f, -3639.0f,  31.9118f, 3.21f},
{2075.73f, -3643.59f, 31.9118f, 3.22f},
{2033.59f, -3649.52f, 31.9118f, 3.42f},
{1985.22f, -3662.99f, 31.9118f, 3.42f},
{1927.09f, -3679.56f, 33.9118f, 3.42f},
{1873.57f, -3695.32f, 33.9118f, 3.44f}
};

struct mob_rizzle_sprysprocketAI : public ScriptedAI
{
    mob_rizzle_sprysprocketAI(Creature *c) : ScriptedAI(c) {}

    int32 Teleport_Timer;
    Timer Check_Timer;
    Timer Grenade_Timer;
    int32 Must_Die_Timer;
    uint32 CurrWP;

    uint64 PlayerGUID;

    bool Must_Die;
    bool Escape;
    bool ContinueWP;
    bool Reached;

    void Reset()
    {
        Teleport_Timer = 3500;
        Check_Timer.Reset(1000);
        Grenade_Timer.Reset(30000);
        Must_Die_Timer = 3000;
        CurrWP = 0;

        PlayerGUID = 0;

        Must_Die = false;
        Escape = false;
        ContinueWP = false;
        Reached = false;
    }

    void Despawn()
    {
        m_creature->DealDamage(m_creature, m_creature->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
        m_creature->RemoveCorpse();
    }

    void EnterEvadeMode()
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (Must_Die)
        {
            Must_Die_Timer -= diff;
            if (Must_Die_Timer <= diff)
            {
                Despawn();
                return;
            }
        }
        if(!Escape)
        {
            if(!PlayerGUID)
                return;

            Teleport_Timer -= diff;
            if(Teleport_Timer <= diff)
            {
                DoCast(m_creature, SPELL_RIZZLE_ESCAPE, true);

                //begin swimming and summon depth charges
                me->TextEmote(MSG_ESCAPE_NOTICE, PlayerGUID);
                DoCast(m_creature, SPELL_PERIODIC_DEPTH_CHARGE);
                m_creature->SetLevitate(true);
                m_creature->SetSpeed(MOVE_RUN, 0.85f, true);
                m_creature->SetSpeed(MOVE_FLIGHT, 0.85f, true);
                m_creature->GetMotionMaster()->MovementExpired();
                m_creature->GetMotionMaster()->MovePoint(CurrWP, WPs[CurrWP][0], WPs[CurrWP][1], WPs[CurrWP][2]);
                Escape = true;
            } 

            return;
        }

        if(ContinueWP)
        {
            m_creature->GetMotionMaster()->MovePoint(CurrWP, WPs[CurrWP][0], WPs[CurrWP][1], WPs[CurrWP][2]);
            ContinueWP = false;
        }


        if(Grenade_Timer.Expired(diff))
        {
            Player *player = (Player *)Unit::GetUnit((*m_creature), PlayerGUID);
            if(player && Reached == false)
            {
               DoScriptText(SAY_RIZZLE_GRENADE, m_creature, player);
               DoCast(player, SPELL_RIZZLE_FROST_GRENADE, true);
            }

            Grenade_Timer = 30000;
        } 

        if(Check_Timer.Expired(diff))
        {
            Unit *player = m_creature->GetUnit(PlayerGUID);
            if(!player)
            {
                Despawn();
                return;
            }

            if (((me->GetDistance(player) < 5) || (me->GetDistance(player) < 15 && me->GetPositionX() - 3 < player->GetPositionX())) && !Reached)
            {
                DoScriptText(SAY_RIZZLE_FINAL, m_creature);
                m_creature->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                m_creature->setFaction(35);
                m_creature->RemoveAurasDueToSpell(SPELL_PERIODIC_DEPTH_CHARGE);
                me->CombatStop();
                player->CombatStop();
                Reached = true;
            }

            Check_Timer = 500;
        } 

    }

    void AttackStart(Unit *who)
    {
        if (!who || PlayerGUID)
            return;

        if(who->GetTypeId() == TYPEID_PLAYER && ((Player *)who)->GetQuestStatus(10994) == QUEST_STATUS_INCOMPLETE)
        {
            PlayerGUID = who->GetGUID();
            DoScriptText(SAY_RIZZLE_START, m_creature);
            DoCast(who, SPELL_RIZZLE_BLACKJACK, false);
            return;
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if ((type != POINT_MOTION_TYPE) || Reached == true)
            return;

        if(id == 57)
        {
            Despawn();
            return;
        }

        ++CurrWP;
        ContinueWP = true;
    }

};

bool GossipHello_mob_rizzle_sprysprocket(Player *player, Creature *_Creature)
{
    if(player->GetQuestStatus(10994) != QUEST_STATUS_INCOMPLETE)
        return true;
    player->ADD_GOSSIP_ITEM( 0, GOSSIP_GET_MOONSTONE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    player->SEND_GOSSIP_MENU(10811,_Creature->GetGUID());
    return true;
}

bool GossipSelect_mob_rizzle_sprysprocket(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1 && player->GetQuestStatus(10994) == QUEST_STATUS_INCOMPLETE)
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->CastSpell(player, SPELL_GIVE_SOUTHFURY_MOONSTONE, true);
        ((mob_rizzle_sprysprocketAI*)_Creature->AI())->Must_Die_Timer = 3000;
        ((mob_rizzle_sprysprocketAI*)_Creature->AI())->Must_Die = true;
    }
    return true;
}

CreatureAI* GetAI_mob_rizzle_sprysprocket(Creature *_Creature)
{
    return new mob_rizzle_sprysprocketAI (_Creature);
}

/*####
# mob_depth_charge
####*/

struct mob_depth_chargeAI : public ScriptedAI
{
    mob_depth_chargeAI(Creature *c) : ScriptedAI(c) {}

    bool we_must_die;
    int32 must_die_timer;

    void Reset()
    {
        m_creature->SetLevitate(true);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        we_must_die = false;
        must_die_timer = 1000;
    }

    void UpdateAI(const uint32 diff)
    {
        if (we_must_die)
        {
            must_die_timer -= diff;
            if (must_die_timer <= diff)
            {
                m_creature->DealDamage(m_creature, m_creature->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                m_creature->RemoveCorpse();
            }
            return;
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!who)
            return;

        if(who->GetTypeId() == TYPEID_PLAYER && m_creature->IsWithinDistInMap(who, 5))
        {
            DoCast(who, SPELL_DEPTH_CHARGE_TRAP);
            we_must_die = true;
            return;
        }
    }

    void AttackStart(Unit *who)
    {
        return;
    }

    void EnterCombat(Unit* who)
    {
        return;
    }
};

CreatureAI* GetAI_mob_depth_charge(Creature *_Creature)
{
    return new mob_depth_chargeAI (_Creature);
}

/*########
# mob_spirit_of_azuregos
#########*/

#define GOSSIP_ITEM_AZUREGOS1 "How did you know? I mean, yes... Yes I am looking for that shard. Do you have it?"
#define GOSSIP_ITEM_AZUREGOS2 "Alright. Where?"
#define GOSSIP_ITEM_AZUREGOS3 "By Bronzebeard's... um, beard! What are you talking about?"
#define GOSSIP_ITEM_AZUREGOS4 "Fish? You gave a piece of what could be the key to saving all life on Kalimdor to a fish?"
#define GOSSIP_ITEM_AZUREGOS5 "A minnow? The oceans are filled with minnows! There could be a hundred million million minnows out there!"
#define GOSSIP_ITEM_AZUREGOS6 "..."
#define GOSSIP_ITEM_AZUREGOS7 "You put the piece on a minnow and placed the minnow somewhere in the waters of the sea between here and the Eastern Kingdoms? And this minnow has special powers?"
#define GOSSIP_ITEM_AZUREGOS8 "You're insane."
#define GOSSIP_ITEM_AZUREGOS9 "I'm all ears."
#define GOSSIP_ITEM_AZUREGOS10 "Come again."
#define GOSSIP_ITEM_AZUREGOS11 "Ok, let me get this straight. You put the scepter entrusted to your Flight by Anachronos on a minnow of your own making and now you expect me to build an... an arcanite buoy or something... to force your minnow out of hiding? AND potentially incur the wrath of an Elemental Lord? Did I miss anything? Perhaps I am to do this without any clothes on, during a solar eclipse, on a leap year?"
#define GOSSIP_ITEM_AZUREGOS12 "FINE! And how, dare I ask, am I supposed to acquire an arcanite buoy?"
#define GOSSIP_ITEM_AZUREGOS13 "But..."
#define azuregos_bye_bye       "I said GOOD DAY!"

bool GossipHello_mob_spirit_of_azuregos(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );
    if(player->GetQuestRewardStatus(8555) && !player->HasItemCount(20949,1))
    {
        player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_AZUREGOS1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1 );
        player->SEND_GOSSIP_MENU(25005, _Creature->GetGUID());
    }
    else
    {
        player->SEND_GOSSIP_MENU(25006, _Creature->GetGUID());
    }
return true;
}

bool GossipSelect_mob_spirit_of_azuregos(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
  switch (action)
  {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_AZUREGOS2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(25007, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_AZUREGOS3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->SEND_GOSSIP_MENU(25008, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_AZUREGOS4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->SEND_GOSSIP_MENU(25009, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_AZUREGOS5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            player->SEND_GOSSIP_MENU(25010, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_AZUREGOS6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
            player->SEND_GOSSIP_MENU(25011, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_AZUREGOS7, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+7);
            player->SEND_GOSSIP_MENU(25012, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+7:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_AZUREGOS8, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+8);
            player->SEND_GOSSIP_MENU(25013, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+8:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_AZUREGOS9, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+9);
            player->SEND_GOSSIP_MENU(25014, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+9:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_AZUREGOS10, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+10);
            player->SEND_GOSSIP_MENU(25015, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+10:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_AZUREGOS11, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+11);
            player->SEND_GOSSIP_MENU(25016, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+11:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_AZUREGOS12, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+12);
            player->SEND_GOSSIP_MENU(25017, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+12:
        {
            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 20949, 1);
            if (msg == EQUIP_ERR_OK)
            {
                Item* item = player->StoreNewItem(dest, 20949, true);
                player->SendNewItem(item,1,true,false,true);
            }
        player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_AZUREGOS13, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+13);
        player->SEND_GOSSIP_MENU(25018, _Creature->GetGUID());
        break;
        }
        case GOSSIP_ACTION_INFO_DEF+13:
            player->CLOSE_GOSSIP_MENU();
            _Creature->Say(azuregos_bye_bye,LANG_UNIVERSAL,player->GetGUID() );
            break;
  }
return true;
}

/*######
## npc_duke_hydraxis
######*/

#define GOSSIP_AQ "Aqual Quintessence"
#define GOSSIP_ET "Eternal Quintessence"

bool GossipHello_npc_duke_hydraxis(Player *player, Creature *_Creature)
{
    if( _Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if( player->GetQuestStatus(6824) == QUEST_STATUS_COMPLETE)
    {
        if (player->GetReputationMgr().GetRank(749) == REP_HONORED)
            player->ADD_GOSSIP_ITEM(0, GOSSIP_AQ, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        else if (player->GetReputationMgr().GetRank(749) >= REP_REVERED)
            player->ADD_GOSSIP_ITEM(0, GOSSIP_ET, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    }

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_duke_hydraxis(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_ACTION_INFO_DEF )
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->CastSpell(player, 21357, false);
    }
    else if ( action == GOSSIP_ACTION_INFO_DEF + 1 )
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->CastSpell(player, 28439, false);
    }

    return true;
}

void AddSC_azshara()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="mobs_spitelashes";
    newscript->GetAI = &GetAI_mobs_spitelashes;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_loramus_thalipedes";
    newscript->pGossipHello =  &GossipHello_npc_loramus_thalipedes;
    newscript->pGossipSelect = &GossipSelect_npc_loramus_thalipedes;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_rizzle_sprysprocket";
    newscript->GetAI = &GetAI_mob_rizzle_sprysprocket;
    newscript->pGossipHello =  &GossipHello_mob_rizzle_sprysprocket;
    newscript->pGossipSelect = &GossipSelect_mob_rizzle_sprysprocket;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_depth_charge";
    newscript->GetAI = &GetAI_mob_depth_charge;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_spirit_of_azuregos";
    newscript->pGossipHello =  &GossipHello_mob_spirit_of_azuregos;
    newscript->pGossipSelect = &GossipSelect_mob_spirit_of_azuregos;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_duke_hydraxis";
    newscript->pGossipHello = &GossipHello_npc_duke_hydraxis;
    newscript->pGossipSelect = &GossipSelect_npc_duke_hydraxis;
    newscript->RegisterSelf();
}

