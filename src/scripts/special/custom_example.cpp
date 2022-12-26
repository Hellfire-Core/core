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
SDName: Custom_Example
SD%Complete: 100
SDComment: Short custom scripting example
SDCategory: Script Examples
EndScriptData */

#include "scriptPCH.h"
#include "ObjectMgr.h"
#include "ScriptDevMgr.h"

// **** This script is designed as an example for others to build on ****
// **** Please modify whatever you'd like to as this script is only for developement ****

// **** Script Info ****
// This script is written in a way that it can be used for both friendly and hostile monsters
// Its primary purpose is to show just how much you can really do with scripts
// I recommend trying it out on both an agressive NPC and on friendly npc

// **** Quick Info ****
// Functions with Handled Function marked above them are functions that are called automatically by the core
// Functions that are marked Custom Function are functions I've created to simplify code

#define SPELL_BUFF      25661
#define SPELL_ONE       12555
#define SPELL_ONE_ALT   24099
#define SPELL_TWO       10017
#define SPELL_THREE     26027
#define SPELL_ENRAGE    23537
#define SPELL_BESERK    32309

#define SAY_AGGRO       "Let the games begin."
#define SAY_RANDOM_0    "I see endless suffering. I see torment. I see rage. I see everything."
#define SAY_RANDOM_1    "Muahahahaha"
#define SAY_RANDOM_2    "These mortal infedels my lord, they have invaded your sanctum and seek to steal your secrets."
#define SAY_RANDOM_3    "You are already dead."
#define SAY_RANDOM_4    "Where to go? What to do? So many choices that all end in pain, end in death."
#define SAY_BESERK      "$N, I sentance you to death!"
#define SAY_PHASE       "The suffering has just begun!"

#define GOSSIP_ITEM     "I'm looking for a fight"
#define SAY_DANCE       "I always thought I was a good dancer"
#define SAY_SALUTE      "Move out Soldier!"

struct custom_exampleAI : public ScriptedAI
{
    //*** HANDLED FUNCTION ***
    //This is the constructor, called only once when the creature is first created
    custom_exampleAI(Creature *c) : ScriptedAI(c) {}

    //*** CUSTOM VARIABLES ****
    //These variables are for use only by this individual script.
    //Nothing else will ever call them but us.

    Timer Say_Timer;                                       //Timer for random chat
    Timer Rebuff_Timer;                                    //Timer for rebuffing
    Timer Spell_1_Timer;                                   //Timer for spell 1 when in combat
    Timer Spell_2_Timer;                                   //Timer for spell 1 when in combat
    Timer Spell_3_Timer;                                   //Timer for spell 1 when in combat
    Timer Beserk_Timer;                                    //Timer until we go into Beserk (enraged) mode
    uint32 Phase;                                          //The current battle phase we are in
    Timer Phase_Timer;                                     //Timer until phase transition

    //*** HANDLED FUNCTION ***
    //This is called whenever the core decides we need to evade
    void Reset()
    {
        Phase = 1;                                          //Start in phase 1
        Phase_Timer.Reset(60000);                                //60 seconds
        Spell_1_Timer.Reset(5000);                               //5 seconds
        Spell_2_Timer.Reset(37000);                              //37 seconds
        Spell_3_Timer.Reset(19000);                              //19 seconds
        Beserk_Timer.Reset(120000);                              //2 minutes
    }

    //*** HANDLED FUNCTION ***
    //Attack Start is called whenever someone hits us.
    void EnterCombat(Unit *who)
    {
        //Say some stuff
        DoSay(SAY_AGGRO,LANG_UNIVERSAL,NULL);
        DoPlaySoundToSet(m_creature,8280);
    }

    //*** HANDLED FUNCTION ***
    //Update AI is called Every single map update (roughly once every 100ms if a player is within the grid)
    void UpdateAI(const uint32 diff)
    {
        //Out of combat timers
        if (!m_creature->GetVictim())
        {
            //Random Say timer
            if (Say_Timer.Expired(diff))
            {
                //Random switch between 5 outcomes
                switch (rand()%5)
                {
                    case 0:
                        DoYell(SAY_RANDOM_0,LANG_UNIVERSAL,NULL);
                        DoPlaySoundToSet(m_creature,8831);  //8831 is the index of the sound we are playing. You find these numbers in SoundEntries.dbc
                        break;

                    case 1:
                        DoYell(SAY_RANDOM_1,LANG_UNIVERSAL,NULL);
                        DoPlaySoundToSet(m_creature,8818);
                        break;

                    case 2:
                        DoYell(SAY_RANDOM_2,LANG_UNIVERSAL,NULL);
                        DoPlaySoundToSet(m_creature,8041);
                        break;

                    case 3:
                        DoYell(SAY_RANDOM_3,LANG_UNIVERSAL,NULL);
                        DoPlaySoundToSet(m_creature,8581);
                        break;

                    case 4:
                        DoYell(SAY_RANDOM_4,LANG_UNIVERSAL,NULL);
                        DoPlaySoundToSet(m_creature,8791);
                        break;
                }

                Say_Timer = 45000;                          //Say something agian in 45 seconds
            }

            //Rebuff timer
            if (Rebuff_Timer.Expired(diff))
            {
                DoCast(m_creature,SPELL_BUFF);
                Rebuff_Timer = 900000;                      //Rebuff agian in 15 minutes
            }
        }

        //Return since we have no target
        if (!UpdateVictim())
            return;

        //Spell 1 timer
        if (Spell_1_Timer.Expired(diff))
        {
            //Cast spell one on our current target.
            if (rand()%50 > 10)
                DoCast(m_creature->GetVictim(),SPELL_ONE_ALT);
            else
                if (m_creature->IsWithinDistInMap(m_creature->GetVictim(), 25))
                    DoCast(m_creature->GetVictim(),SPELL_ONE);

            Spell_1_Timer = 5000;
        }

        //Spell 2 timer
        if (Spell_2_Timer.Expired(diff))
        {
            //Cast spell one on our current target.
            DoCast(m_creature->GetVictim(),SPELL_TWO);

            Spell_2_Timer = 37000;
        }

        //Spell 3 timer
        if (Phase > 1)
            if (Spell_3_Timer.Expired(diff))
        {
            //Cast spell one on our current target.
            DoCast(m_creature->GetVictim(),SPELL_THREE);

            Spell_3_Timer = 19000;
        }

        //Beserk timer
        if (Phase > 1)
            if (Beserk_Timer.Expired(diff))
        {
            //Say our line then cast uber death spell
            DoPlaySoundToSet(m_creature,8588);
            DoYell(SAY_BESERK,LANG_UNIVERSAL,m_creature->GetVictim());
            DoCast(m_creature->GetVictim(),SPELL_BESERK);

            //Cast our beserk spell agian in 12 seconds if we didn't kill everyone
            Beserk_Timer = 12000;
        }

        //Phase timer
        if (Phase == 1)
            if (Phase_Timer.Expired(diff))
            {
                //Go to next phase
                Phase++;
                DoYell(SAY_PHASE,LANG_UNIVERSAL,NULL);
                DoCast(m_creature,SPELL_ENRAGE);
            }

        DoMeleeAttackIfReady();
    }
};

//This is the GetAI method used by all scripts that involve AI
//It is called every time a new creature using this script is created
CreatureAI* GetAI_custom_example(Creature *_Creature)
{
    return new custom_exampleAI (_Creature);
}

//This function is called when the player clicks an option on the gossip menu
void SendDefaultMenu_custom_example(Player *player, Creature *_Creature, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)               //Fight time
    {
        //Set our faction to hostile twoards all
        _Creature->setFaction(24);
        _Creature->Attack(player, true);
        player->PlayerTalkClass->CloseGossip();
    }
}

//This function is called when the player clicks an option on the gossip menu
bool GossipSelect_custom_example(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (sender == GOSSIP_SENDER_MAIN)
        SendDefaultMenu_custom_example(player, _Creature, action);

    return true;
}

//This function is called when the player opens the gossip menu
bool GossipHello_custom_example(Player *player, Creature *_Creature)
{
    player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM        , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    player->PlayerTalkClass->SendGossipMenu(907,_Creature->GetGUID());

    return true;
}

//Our Recive emote function
bool ReceiveEmote_custom_example(Player *player, Creature *_Creature, uint32 emote)
{
    _Creature->HandleEmoteCommand(emote);

    if (emote == TEXTEMOTE_DANCE)
        ((custom_exampleAI*)_Creature->AI())->DoSay(SAY_DANCE,LANG_UNIVERSAL,NULL);

    if (emote == TEXTEMOTE_SALUTE)
        ((custom_exampleAI*)_Creature->AI())->DoSay(SAY_SALUTE,LANG_UNIVERSAL,NULL);

    return true;
}

void LearnSkillRecipesHelper(Player *player, uint32 skill_id)
{
    uint32 classmask = player->getClassMask();

    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        SkillLineAbilityEntry const *skillLine = sSkillLineAbilityStore.LookupEntry(j);
        if (!skillLine)
            continue;

        // wrong skill
        if (skillLine->skillId != skill_id)
            continue;

        // not high rank
        if (skillLine->forward_spellid)
            continue;

        // skip racial skills
        if (skillLine->racemask != 0)
            continue;

        // skip wrong class skills
        if (skillLine->classmask && (skillLine->classmask & classmask) == 0)
            continue;

        SpellEntry const* spellEntry = sSpellStore.LookupEntry(skillLine->spellId);
        if (!spellEntry || !SpellMgr::IsSpellValid(spellEntry, player, false))
            continue;

        player->LearnSpell(skillLine->spellId);
    }
}

bool LearnAllRecipesInProfession(Player *pPlayer, SkillType skill)
{
    ChatHandler handler(pPlayer->GetSession());
    char* skill_name;

    SkillLineEntry const *SkillInfo = sSkillLineStore.LookupEntry(skill);
    skill_name = SkillInfo->name[sWorld.GetDefaultDbcLocale()];

    if (!SkillInfo)
    {
        sLog.outDebug("Profession NPC: received non-valid skill ID");
        return false;
    }

    pPlayer->SetSkill(SkillInfo->id, 300, 300);
    LearnSkillRecipesHelper(pPlayer, SkillInfo->id);
    pPlayer->GetSession()->SendNotification("All recipes for %s learned", skill_name);
    return true;
}

void CompleteLearnProfession(Player *pPlayer, Creature *pCreature, SkillType skill)
{
    if (pPlayer->GetFreePrimaryProfessionPoints() == 0 && !(skill == SKILL_COOKING || skill == SKILL_FIRST_AID))
    {
        pPlayer->GetSession()->SendNotification("You already know two primary professions.");
    }
    else
    {
        if (!LearnAllRecipesInProfession(pPlayer, skill))
            pPlayer->GetSession()->SendNotification("Internal error.");
    }
}

bool GossipHello_Paymaster(Player *player, Creature *_Creature)
{
    _Creature->prepareGossipMenu(player); // why to rewrite other function? just add new line if nessessary

    player->ADD_GOSSIP_ITEM(0, "5,000 Gold", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    player->ADD_GOSSIP_ITEM(0, "Profession Trainer", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    player->ADD_GOSSIP_ITEM(0, "I want to be Aldor.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
    player->ADD_GOSSIP_ITEM(0, "Let's go Scryer.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
    player->SEND_GOSSIP_MENU(8850, _Creature->GetGUID());

    return true;
}

bool GossipSelect_Paymaster(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    if (sender == GOSSIP_SENDER_MAIN)
    {
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                player->CLOSE_GOSSIP_MENU();
                player->SetMoney(50000000);
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "Alchemy", 2, 1);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "Blacksmithing", 2, 2);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "Leatherworking", 2, 3);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "Tailoring", 2, 4);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "Engineering", 2, 5);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "Enchanting", 2, 6);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "Jewelcrafting", 2, 7);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "Herbalism", 2, 9);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "Skinning", 2, 10);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "Mining", 2, 11);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "First Aid", 2, 12);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "Fishing", 2, 13);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_2, "Cooking", 2, 14);
                player->SEND_GOSSIP_MENU(1301, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 3:
            {
                player->CLOSE_GOSSIP_MENU();
                FactionEntry const *scryer = sFactionStore.LookupEntry(934);
                player->GetReputationMgr().SetReputation(scryer, 0);
                FactionEntry const *aldor = sFactionStore.LookupEntry(932);
                player->GetReputationMgr().SetReputation(aldor, 84000);
                break;
            }
            case GOSSIP_ACTION_INFO_DEF + 4:
            {
                player->CLOSE_GOSSIP_MENU();
                FactionEntry const *aldor = sFactionStore.LookupEntry(932);
                player->GetReputationMgr().SetReputation(aldor, 0);
                FactionEntry const *scryer = sFactionStore.LookupEntry(934);
                player->GetReputationMgr().SetReputation(scryer, 84000);
                break;
            }
        }
    }
    else
    {
        switch (action)
        {
            case 1:
                if (!player->HasSkill(SKILL_ALCHEMY))
                    CompleteLearnProfession(player, creature, SKILL_ALCHEMY);
                break;
            case 2:
                if (!player->HasSkill(SKILL_BLACKSMITHING))
                    CompleteLearnProfession(player, creature, SKILL_BLACKSMITHING);
                break;
            case 3:
                if (!player->HasSkill(SKILL_LEATHERWORKING))
                    CompleteLearnProfession(player, creature, SKILL_LEATHERWORKING);
                break;
            case 4:
                if (!player->HasSkill(SKILL_TAILORING))
                    CompleteLearnProfession(player, creature, SKILL_TAILORING);
                break;
            case 5:
                if (!player->HasSkill(SKILL_ENGINERING))
                    CompleteLearnProfession(player, creature, SKILL_ENGINERING);
                break;
            case 6:
                if (!player->HasSkill(SKILL_ENCHANTING))
                    CompleteLearnProfession(player, creature, SKILL_ENCHANTING);
                break;
            case 7:
                if (!player->HasSkill(SKILL_JEWELCRAFTING))
                    CompleteLearnProfession(player, creature, SKILL_JEWELCRAFTING);
                break;
            case 9:
                if (!player->HasSkill(SKILL_HERBALISM))
                    CompleteLearnProfession(player, creature, SKILL_HERBALISM);
                break;
            case 10:
                if (!player->HasSkill(SKILL_SKINNING))
                    CompleteLearnProfession(player, creature, SKILL_SKINNING);
                break;
            case 11:
                if (!player->HasSkill(SKILL_MINING))
                    CompleteLearnProfession(player, creature, SKILL_MINING);
                break;
            case 12:
                if (!player->HasSkill(SKILL_FIRST_AID))
                    CompleteLearnProfession(player, creature, SKILL_FIRST_AID);
                break;
            case 13:
                if (!player->HasSkill(SKILL_FISHING))
                    CompleteLearnProfession(player, creature, SKILL_FISHING);
                break;
            case 14:
                if (!player->HasSkill(SKILL_COOKING))
                    CompleteLearnProfession(player, creature, SKILL_COOKING);
                break;
        }
        player->CLOSE_GOSSIP_MENU();
    }
    
    // TODO: returning false should force core to handle casual options normal way, it does not
    return true;
}

bool GossipHello_Donjon(Player* player, Creature* creature)
{
    creature->prepareGossipMenu(player);

    player->ADD_GOSSIP_ITEM(0, "I want to go to Serpentshine Cavern (Raid).", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    player->ADD_GOSSIP_ITEM(0, "I want to go to Tempest Keep (Raid).", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    player->SEND_GOSSIP_MENU(42730, creature->GetGUID());

    return true;
}

bool GossipSelect_Donjon(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            player->CLOSE_GOSSIP_MENU();
            player->TeleportTo(530, 820.025f, 6864.93f, -66.7556f, 6.28127f);
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            player->CLOSE_GOSSIP_MENU();
            player->TeleportTo(530, 3099.36f, 1518.73f, 190.3f, 4.72592f);
            break;
    }

    return true;
}

bool GossipHello_Beastmaster(Player* player, Creature* creature)
{
    creature->prepareGossipMenu(player);

    if (player->GetClass() == CLASS_HUNTER)
    {
        //player->ADD_GOSSIP_ITEM(0, "Stable", 0, GOSSIP_OPTION_STABLEPET);
        player->ADD_GOSSIP_ITEM(0, "New Pet", 0, GOSSIP_ACTION_INFO_DEF + 1);
        player->ADD_GOSSIP_ITEM(0, "Untrain Pet", 0, GOSSIP_OPTION_UNLEARNPETSKILLS);
        player->SEND_GOSSIP_MENU(12562, creature->GetGUID());
    }
    else
        player->SEND_GOSSIP_MENU(12539, creature->GetGUID());

    return true;
}

bool GossipSelect_Beastmaster(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    if (sender == 0)
    {
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                player->ADD_GOSSIP_ITEM(0, "Bat", 2, 26017);
                player->ADD_GOSSIP_ITEM(0, "Boar", 2, 26020);
                player->ADD_GOSSIP_ITEM(0, "Carrion Bird", 2, 26019);
                player->ADD_GOSSIP_ITEM(0, "Cat", 2, 26021);
                player->ADD_GOSSIP_ITEM(0, "Dragonhawk", 2, 26024);
                player->ADD_GOSSIP_ITEM(0, "Nether Ray", 2, 26027);
                player->ADD_GOSSIP_ITEM(0, "Owl", 2, 26028);
                player->ADD_GOSSIP_ITEM(0, "Raptor", 2, 26029);
                player->ADD_GOSSIP_ITEM(0, "Ravager", 2, 26030);
                player->ADD_GOSSIP_ITEM(0, "Scorpid", 2, 26031);
                player->ADD_GOSSIP_ITEM(0, "Serpent", 2, 26032);
                player->ADD_GOSSIP_ITEM(0, "Spider", 2, 26033);
                player->ADD_GOSSIP_ITEM(0, "Warp Stalker", 2, 26037);
                player->ADD_GOSSIP_ITEM(0, "Wind Serpent", 2, 26038);
                player->ADD_GOSSIP_ITEM(0, "Wolf", 2, 26016);
                player->SEND_GOSSIP_MENU(12561, creature->GetGUID());
                break;
            case GOSSIP_OPTION_STABLEPET:
                player->CLOSE_GOSSIP_MENU();
                player->GetSession()->SendStablePet(creature->GetGUID());
                break;
            case GOSSIP_OPTION_UNLEARNPETSKILLS:
                player->CLOSE_GOSSIP_MENU();
                player->SendPetSkillWipeConfirm();
                break;
        }
    }
    else
    {
        player->CLOSE_GOSSIP_MENU();

        CreatureInfo const* pInfo = sObjectMgr.GetCreatureTemplate(action);
        if (!pInfo || pInfo->type != CREATURE_TYPE_BEAST)
            return false;

        if (Pet* pPet = player->GetPet())
            player->RemovePet(pPet, PET_SAVE_AS_DELETED);

        if (Creature* pSummon = creature->SummonCreature(action, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 30000))
        {
            Pet* pet = new Pet(HUNTER_PET);

            if (!pet)
                return false;

            if (!pet->CreateBaseAtCreature(pSummon))
            {
                delete pet;
                return false;
            }

            pet->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, player->GetGUID());
            pet->SetUInt64Value(UNIT_FIELD_CREATEDBY, player->GetGUID());
            pet->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, player->getFaction());

            if (!pet->InitStatsForLevel(pSummon->GetLevel()))
            {
                delete pet;
                return false;
            }

            // prepare visual effect for levelup
            pet->SetUInt32Value(UNIT_FIELD_LEVEL, pSummon->GetLevel() - 1);

            pet->GetCharmInfo()->SetPetNumber(sObjectMgr.GeneratePetNumber(), true);
            // this enables pet details window (Shift+P)

            pet->InitPetCreateSpells();
            pet->SetHealth(pet->GetMaxHealth());

            Map * pMap = pet->GetMap();
            pMap->Add((Creature*)pet);

            // visual effect for levelup
            pet->SetUInt32Value(UNIT_FIELD_LEVEL, pSummon->GetLevel());

            player->SetPet(pet);
            pet->SavePetToDB(PET_SAVE_AS_CURRENT);
            player->DelayedPetSpellInitialize();
            pSummon->AddObjectToRemoveList();
        }
    }
    
    // TODO: returning false should force core to handle casual options normal way, it does not
    return true;
}

class DoubleXP : public PlayerScript
{
public:
    DoubleXP() : PlayerScript("DoubleXP") {}

    void OnLogin(Player* p) override
    {
        ChatHandler(p).PSendSysMessage("Both Scripting Structures are working Side by Side!");
    }

    void OnGiveXP(Player* p, uint32& amount, Unit* victim) override
    {
        p->Say("Hook Works as intended!", LANG_UNIVERSAL);
    }

};


//This is the actual function called only once durring InitScripts()
//It must define all handled functions that are to be run in this script
//For example if you want this Script to handle Emotes you must include
//newscript->ReciveEmote = My_Emote_Function;
void AddSC_custom_example()
{

    Script *newscript;

    /*
    newscript = new Script;
    newscript->Name="custom_example";
    newscript->GetAI = &GetAI_custom_example;
    newscript->pGossipHello = &GossipHello_custom_example;
    newscript->pGossipSelect = &GossipSelect_custom_example;
    newscript->pReceiveEmote = &ReceiveEmote_custom_example;
    newscript->RegisterSelf();
    */

    newscript = new Script;
    newscript->Name = "ptr_paymaster";
    newscript->pGossipHello = &GossipHello_Paymaster;
    newscript->pGossipSelect = &GossipSelect_Paymaster;
    newscript->RegisterSelf(false);

    newscript = new Script;
    newscript->Name = "ptr_donjon";
    newscript->pGossipHello = &GossipHello_Donjon;
    newscript->pGossipSelect = &GossipSelect_Donjon;
    newscript->RegisterSelf(false);

    newscript = new Script;
    newscript->Name = "ptr_beastmaster";
    newscript->pGossipHello = &GossipHello_Beastmaster;
    newscript->pGossipSelect = &GossipSelect_Beastmaster;
    newscript->RegisterSelf(false);

   // new DoubleXP();
}

