/*
 * Copyright (C) 2005-2008 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2017 MaNGOSOne <https://github.com/mangos/one>
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

#include "QuestDef.h"
#include "GossipDef.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Formulas.h"

GossipMenu::GossipMenu()
{
    m_gItems.reserve(16);                                   // can be set for max from most often sizes to speedup push_back and less memory use
}

GossipMenu::~GossipMenu()
{
    ClearMenu();
}

void GossipMenu::AddMenuItem(uint8 Icon, const std::string& Message, uint32 dtSender, uint32 dtAction, const std::string& BoxMessage, uint32 BoxMoney, bool Coded)
{
    ASSERT(m_gItems.size() <= GOSSIP_MAX_MENU_ITEMS );

    GossipMenuItem gItem;

    gItem.m_gIcon       = Icon;
    gItem.m_gMessage    = Message;
    gItem.m_gCoded      = Coded;
    gItem.m_gSender     = dtSender;
    gItem.m_gAction     = dtAction;
    gItem.m_gBoxMessage = BoxMessage;
    gItem.m_gBoxMoney   = BoxMoney;

    m_gItems.push_back(gItem);
}

void GossipMenu::AddMenuItem(uint8 Icon, const std::string& Message, bool Coded)
{
    AddMenuItem(Icon, Message, 0, 0, "", 0, Coded);
}

void GossipMenu::AddMenuItem(uint8 Icon, char const* Message, bool Coded)
{
    AddMenuItem(Icon, std::string(Message ? Message : ""),Coded);
}

void GossipMenu::AddMenuItem(uint8 Icon, char const* Message, uint32 dtSender, uint32 dtAction, char const* BoxMessage, uint32 BoxMoney, bool Coded)
{
    AddMenuItem(Icon, std::string(Message ? Message : ""), dtSender, dtAction, std::string(BoxMessage ? BoxMessage : ""), BoxMoney, Coded);
}

uint32 GossipMenu::MenuItemSender(unsigned int ItemId)
{
    if (ItemId >= m_gItems.size()) return 0;

    return m_gItems[ ItemId ].m_gSender;
}

uint32 GossipMenu::MenuItemAction(unsigned int ItemId)
{
    if (ItemId >= m_gItems.size()) return 0;

    return m_gItems[ ItemId ].m_gAction;
}

bool GossipMenu::MenuItemCoded(unsigned int ItemId)
{
    if (ItemId >= m_gItems.size()) return 0;

    return m_gItems[ ItemId ].m_gCoded;
}

void GossipMenu::ClearMenu()
{
    m_gItems.clear();
}

PlayerMenu::PlayerMenu(WorldSession *session) : pSession(session)
{
}

PlayerMenu::~PlayerMenu()
{
    ClearMenus();
}

void PlayerMenu::ClearMenus()
{
    mGossipMenu.ClearMenu();
    mQuestMenu.ClearMenu();
}

uint32 PlayerMenu::GossipOptionSender(unsigned int Selection)
{
    return mGossipMenu.MenuItemSender(Selection);
}

uint32 PlayerMenu::GossipOptionAction(unsigned int Selection)
{
    return mGossipMenu.MenuItemAction(Selection);
}

bool PlayerMenu::GossipOptionCoded(unsigned int Selection)
{
    return mGossipMenu.MenuItemCoded(Selection);
}

void PlayerMenu::SendGossipMenu(uint32 TitleTextId, uint64 npcGUID)
{
    WorldPacket data(SMSG_GOSSIP_MESSAGE, (100));         // guess size
    data << uint64(npcGUID);
    data << uint32(0);                                      // new 2.4.0
    data << uint32(TitleTextId);
    data << uint32(mGossipMenu.MenuItemCount());          // max count 0x0F

    for (unsigned int iI = 0; iI < mGossipMenu.MenuItemCount(); iI++)
    {
        GossipMenuItem const& gItem = mGossipMenu.GetItem(iI);
        data << uint32(iI);
        data << uint8(gItem.m_gIcon);
        data << uint8(gItem.m_gCoded);                    // makes pop up box password
        data << uint32(gItem.m_gBoxMoney);                  // money required to open menu, 2.0.3
        data << gItem.m_gMessage;                           // text for gossip item
        data << gItem.m_gBoxMessage;                        // accept text (related to money) pop up box, 2.0.3
    }

    data << uint32(mQuestMenu.MenuItemCount());           // max count 0x20

    for (uint16 iI = 0; iI < mQuestMenu.MenuItemCount(); iI++)
    {
        QuestMenuItem const& qItem = mQuestMenu.GetItem(iI);
        uint32 questID = qItem.m_qId;
        Quest const* pQuest = sObjectMgr.GetQuestTemplate(questID);

        data << uint32(questID);
        data << uint32(qItem.m_qIcon);
        data << int32(pQuest->GetQuestLevel());
        data << pQuest->GetName();
    }

    pSession->SendPacket(&data);
    //sLog.outDebug("WORLD: Sent SMSG_GOSSIP_MESSAGE NPCGuid=%u",GUID_LOPART(npcGUID));
}

void PlayerMenu::CloseGossip()
{
    WorldPacket data(SMSG_GOSSIP_COMPLETE, 0);
    pSession->SendPacket(&data);

    //sLog.outDebug("WORLD: Sent SMSG_GOSSIP_COMPLETE");
}

void PlayerMenu::SendPointOfInterest(float X, float Y, uint32 Icon, uint32 Flags, uint32 Data, char const * locName)
{
    WorldPacket data(SMSG_GOSSIP_POI, (4+4+4+4+4+10));    // guess size
    data << Flags;
    data << X << Y;
    data << uint32(Icon);
    data << uint32(Data);
    data << locName;

    pSession->SendPacket(&data);
    //sLog.outDebug("WORLD: Sent SMSG_GOSSIP_POI");
}

void PlayerMenu::SendTalking(uint32 textID)
{
    GossipText const* pGossip = sObjectMgr.GetGossipText(textID);

    WorldPacket data(SMSG_NPC_TEXT_UPDATE, 100);          // guess size
    data << textID;                                         // can be < 0

    if (!pGossip)
    {
        for (uint8 i = 0; i < 8; ++i)
        {
            data << float(0);
            data << "Greetings $N";
            data << "Greetings $N";
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
        }
    }
    else
    {
        for (uint8 i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; ++i)
        {
            data << pGossip->Options[i].Probability;

            if (pGossip->Options[i].Text_0.empty())
                data << pGossip->Options[i].Text_1;
            else
                data << pGossip->Options[i].Text_0;

            if (pGossip->Options[i].Text_1.empty())
                data << pGossip->Options[i].Text_0;
            else
                data << pGossip->Options[i].Text_1;

            data << pGossip->Options[i].Language;

            for(uint8 j = 0; j < 3; ++j)
            {
                data << pGossip->Options[i].Emotes[j]._Delay;
                data << pGossip->Options[i].Emotes[j]._Emote;
            }
        }
    }
    pSession->SendPacket(&data);

    sLog.outDebug( "WORLD: Sent SMSG_NPC_TEXT_UPDATE ");
}

void PlayerMenu::SendTalking(char const * title, char const * text)
{
    WorldPacket data(SMSG_NPC_TEXT_UPDATE, 50);           // guess size
    data << uint32(0);
    for (uint32 i = 0; i < 8; ++i)
    {
        data << float(0);
        data << title;
        data << text;
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
    }

    pSession->SendPacket(&data);

    sLog.outDebug("WORLD: Sent SMSG_NPC_TEXT_UPDATE ");
}

/*********************************************************/
/***                    QUEST SYSTEM                   ***/
/*********************************************************/

QuestMenu::QuestMenu()
{
    m_qItems.reserve(16);                                   // can be set for max from most often sizes to speedup push_back and less memory use
}

QuestMenu::~QuestMenu()
{
    ClearMenu();
}

void QuestMenu::AddMenuItem(uint32 QuestId, uint8 Icon)
{
    Quest const* qinfo = sObjectMgr.GetQuestTemplate(QuestId);
    if (!qinfo) return;

    ASSERT(m_qItems.size() <= GOSSIP_MAX_MENU_ITEMS );

    QuestMenuItem qItem;

    qItem.m_qId        = QuestId;
    qItem.m_qIcon      = Icon;

    m_qItems.push_back(qItem);
}

bool QuestMenu::HasItem(uint32 questid)
{
    for (QuestMenuItemList::iterator i = m_qItems.begin(); i != m_qItems.end(); ++i)
    {
        if (i->m_qId==questid)
        {
            return true;
        }
    }
    return false;
}

void QuestMenu::ClearMenu()
{
    m_qItems.clear();
}

void PlayerMenu::SendQuestGiverQuestList(QEmote eEmote, const std::string& Title, uint64 npcGUID)
{
    WorldPacket data(SMSG_QUESTGIVER_QUEST_LIST, 100);    // guess size
    data << uint64(npcGUID);
    data << Title;
    data << uint32(eEmote._Delay);                         // player emote
    data << uint32(eEmote._Emote);                         // NPC emote
    data << uint8 (mQuestMenu.MenuItemCount());

    for (uint16 iI = 0; iI < mQuestMenu.MenuItemCount(); iI++)
    {
        QuestMenuItem const& qmi = mQuestMenu.GetItem(iI);

        uint32 questID = qmi.m_qId;

        if (Quest const *pQuest = sObjectMgr.GetQuestTemplate(questID))
        {
            data << uint32(questID);
            data << uint32(qmi.m_qIcon);
            data << int32(pQuest->GetQuestLevel());
            data << pQuest->GetName();
        }
    }
    pSession->SendPacket(&data);
    sLog.outDebug("WORLD: Sent SMSG_QUESTGIVER_QUEST_LIST NPC Guid=%u", GUID_LOPART(npcGUID));
}

void PlayerMenu::SendQuestGiverStatus(uint8 questStatus, uint64 npcGUID)
{
    WorldPacket data(SMSG_QUESTGIVER_STATUS, 9);
    data << uint64(npcGUID);
    data << uint8(questStatus);

    pSession->SendPacket(&data);
    sLog.outDebug("WORLD: Sent SMSG_QUESTGIVER_STATUS NPC Guid=%u, status=%u", GUID_LOPART(npcGUID), questStatus);
}

void PlayerMenu::SendQuestGiverQuestDetails(Quest const *pQuest, uint64 npcGUID, bool ActivateAccept)
{
    WorldPacket data(SMSG_QUESTGIVER_QUEST_DETAILS, 100);   // guess size

    data << uint64(npcGUID);
    data << uint32(pQuest->GetQuestId());
    data << pQuest->GetName();;
    data << pQuest->GetDetails();
    data << pQuest->GetObjectives();
    data << uint32(ActivateAccept);
    data << uint32(pQuest->GetSuggestedPlayers());

    if (pQuest->HasFlag(QUEST_FLAGS_HIDDEN_REWARDS))
    {
        data << uint32(0);                                  // Rewarded chosen items hidden
        data << uint32(0);                                  // Rewarded items hidden
        data << uint32(0);                                  // Rewarded money hidden
    }
    else
    {
        ItemPrototype const* IProto;

        data << uint32(pQuest->GetRewChoiceItemsCount());
        for (uint32 i=0; i < QUEST_REWARD_CHOICES_COUNT; i++)
        {
            if (!pQuest->RewChoiceItemId[i]) continue;
            data << uint32(pQuest->RewChoiceItemId[i]);
            data << uint32(pQuest->RewChoiceItemCount[i]);
            IProto = ObjectMgr::GetItemPrototype(pQuest->RewChoiceItemId[i]);
            if (IProto)
                data << uint32(IProto->DisplayInfoID);
            else
                data << uint32(0x00);
        }

        data << uint32(pQuest->GetRewItemsCount());
        for (uint32 i=0; i < QUEST_REWARDS_COUNT; i++)
        {
            if (!pQuest->RewItemId[i]) continue;
            data << uint32(pQuest->RewItemId[i]);
            data << uint32(pQuest->RewItemCount[i]);
            IProto = ObjectMgr::GetItemPrototype(pQuest->RewItemId[i]);
            if (IProto)
                data << uint32(IProto->DisplayInfoID);
            else
                data << uint32(0);
        }

        data << uint32(pQuest->GetRewOrReqMoney());
    }

    // rewarded honor points. Multiply with 10 to satisfy client
    data << uint32(10*MaNGOS::Honor::hk_honor_at_level(pSession->GetPlayer()->GetLevel(), pQuest->GetRewHonorableKills()));
    data << uint32(pQuest->GetRewSpell());                  // reward spell, this spell will display (icon) (cast if RewSpellCast==0)
    data << uint32(pQuest->GetRewSpellCast());              // cast spell
    data << uint32(pQuest->GetCharTitleId());               // CharTitleId, new 2.4.0, player gets this title (id from CharTitles)

    data << uint32(QUEST_EMOTE_COUNT);
    for (uint32 i=0; i < QUEST_EMOTE_COUNT; i++)
    {
        data << uint32(pQuest->DetailsEmote[i]);
        data << uint32(0);                                  // DetailsEmoteDelay
    }
    pSession->SendPacket(&data);

    sLog.outDebug("WORLD: Sent SMSG_QUESTGIVER_QUEST_DETAILS NPCGuid=%u, questid=%u", GUID_LOPART(npcGUID), pQuest->GetQuestId());
}

void PlayerMenu::SendQuestQueryResponse(Quest const *pQuest)
{

    WorldPacket data(SMSG_QUEST_QUERY_RESPONSE, 100);     // guess size

    data << uint32(pQuest->GetQuestId());
    data << uint32(pQuest->GetQuestMethod());               // Accepted values: 0, 1 or 2. 0==IsAutoComplete() (skip objectives/details)
    data << int32(pQuest->GetQuestLevel());                 // may be 0, -1, static data, in other cases must be used dynamic level: Player::GetQuestOrPlayerLevel
    data << uint32(pQuest->GetZoneOrSort());                // zone or sort to display in quest log

    data << uint32(pQuest->GetType());
    data << uint32(pQuest->GetSuggestedPlayers());

    data << uint32(pQuest->GetRepObjectiveFaction());       // shown in quest log as part of quest objective
    data << uint32(pQuest->GetRepObjectiveValue());         // shown in quest log as part of quest objective

    data << uint32(0);                                      // RequiredOpositeRepFaction
    data << uint32(0);                                      // RequiredOpositeRepValue, required faction value with another (oposite) faction (objective)

    data << uint32(pQuest->GetNextQuestInChain());          // client will request this quest from NPC, if not 0

    if (pQuest->HasFlag(QUEST_FLAGS_HIDDEN_REWARDS))
        data << uint32(0);                                  // Hide money rewarded
    else
        data << uint32(pQuest->GetRewOrReqMoney());

    data << uint32(pQuest->GetRewMoneyMaxLevel());          // used in XP calculation at client
    data << uint32(pQuest->GetRewSpell());                  // reward spell, this spell will display (icon) (cast if RewSpellCast==0)
    data << uint32(pQuest->GetRewSpellCast());              // cast spell

    // rewarded honor points
    data << uint32(MaNGOS::Honor::hk_honor_at_level(pSession->GetPlayer()->GetLevel(), pQuest->GetRewHonorableKills()));
    data << uint32(pQuest->GetSrcItemId());
    data << uint32(pQuest->GetFlags() & 0xFFFF);
    data << uint32(pQuest->GetCharTitleId());               // CharTitleId, new 2.4.0, player gets this title (id from CharTitles)

    int iI;

    if (pQuest->HasFlag(QUEST_FLAGS_HIDDEN_REWARDS))
    {
        for (iI = 0; iI < QUEST_REWARDS_COUNT; iI++)
            data << uint32(0) << uint32(0);
        for (iI = 0; iI < QUEST_REWARD_CHOICES_COUNT; iI++)
            data << uint32(0) << uint32(0);
    }
    else
    {
        for (iI = 0; iI < QUEST_REWARDS_COUNT; iI++)
        {
            data << uint32(pQuest->RewItemId[iI]);
            data << uint32(pQuest->RewItemCount[iI]);
        }
        for (iI = 0; iI < QUEST_REWARD_CHOICES_COUNT; iI++)
        {
            data << uint32(pQuest->RewChoiceItemId[iI]);
            data << uint32(pQuest->RewChoiceItemCount[iI]);
        }
    }

    data << pQuest->GetPointMapId();
    data << pQuest->GetPointX();
    data << pQuest->GetPointY();
    data << pQuest->GetPointOpt();

    data << pQuest->GetName();
    data << pQuest->GetObjectives();
    data << pQuest->GetDetails();
    data << pQuest->GetEndText();

    for (iI = 0; iI < QUEST_OBJECTIVES_COUNT; iI++)
    {
        if (pQuest->ReqCreatureOrGOId[iI] < 0)
        {
            // client expected gameobject template id in form (id|0x80000000)
            data << uint32((pQuest->ReqCreatureOrGOId[iI]*(-1))|0x80000000);
        }
        else
        {
            data << uint32(pQuest->ReqCreatureOrGOId[iI]);
        }
        data << uint32(pQuest->ReqCreatureOrGOCount[iI]);
        data << uint32(pQuest->ReqItemId[iI]);
        data << uint32(pQuest->ReqItemCount[iI]);
    }

    for (iI = 0; iI < QUEST_OBJECTIVES_COUNT; iI++)
        data << pQuest->ObjectiveText[iI];

    pSession->SendPacket(&data);
    sLog.outDebug("WORLD: Sent SMSG_QUEST_QUERY_RESPONSE questid=%u",pQuest->GetQuestId());
}

void PlayerMenu::SendQuestGiverOfferReward(Quest const* pQuest, uint64 npcGUID, bool EnbleNext)
{
    WorldPacket data(SMSG_QUESTGIVER_OFFER_REWARD, 50);   // guess size

    data << npcGUID;
    data << pQuest->GetQuestId();
    data << pQuest->GetName();
    data << pQuest->GetOfferRewardText();

    data << uint32(EnbleNext);
    data << uint32(0);                                      // unk

    uint32 EmoteCount = 0;
    for (uint32 i = 0; i < QUEST_EMOTE_COUNT; i++)
    {
        if (pQuest->OfferRewardEmote[i] <= 0)
            break;
        ++EmoteCount;
    }

    data << EmoteCount;                                     // Emote Count
    for (uint32 i = 0; i < EmoteCount; i++)
    {
        data << uint32(0);                                  // Delay Emote
        data << pQuest->OfferRewardEmote[i];
    }

    ItemPrototype const *pItem;

    data << uint32(pQuest->GetRewChoiceItemsCount());
    for (uint32 i=0; i < pQuest->GetRewChoiceItemsCount(); i++)
    {
        pItem = ObjectMgr::GetItemPrototype(pQuest->RewChoiceItemId[i]);

        data << uint32(pQuest->RewChoiceItemId[i]);
        data << uint32(pQuest->RewChoiceItemCount[i]);

        if (pItem)
            data << uint32(pItem->DisplayInfoID);
        else
            data << uint32(0);
    }

    data << uint32(pQuest->GetRewItemsCount());
    for (uint16 i=0; i < pQuest->GetRewItemsCount(); i++)
    {
        pItem = ObjectMgr::GetItemPrototype(pQuest->RewItemId[i]);
        data << uint32(pQuest->RewItemId[i]);
        data << uint32(pQuest->RewItemCount[i]);

        if (pItem)
            data << uint32(pItem->DisplayInfoID);
        else
            data << uint32(0);
    }

    data << uint32(pQuest->GetRewOrReqMoney());

    // rewarded honor points. Multiply with 10 to satisfy client
    data << uint32(10*MaNGOS::Honor::hk_honor_at_level(pSession->GetPlayer()->GetLevel(), pQuest->GetRewHonorableKills()));
    data << uint32(0x08);                                   // unused by client?
    data << uint32(pQuest->GetRewSpell());                  // reward spell, this spell will display (icon) (cast if RewSpellCast==0)
    data << uint32(pQuest->GetRewSpellCast());              // cast spell
    data << uint32(0x00);                                   // unk, NOT honor
    pSession->SendPacket(&data);
    sLog.outDebug("WORLD: Sent SMSG_QUESTGIVER_OFFER_REWARD NPCGuid=%u, questid=%u",GUID_LOPART(npcGUID),pQuest->GetQuestId());
}

void PlayerMenu::SendQuestGiverRequestItems(Quest const *pQuest, uint64 npcGUID, bool Completable, bool CloseOnCancel)
{
    // We can always call to RequestItems, but this packet only goes out if there are actually
    // items.  Otherwise, we'll skip straight to the OfferReward

    // We may wish a better check, perhaps checking the real quest requirements
    if (pQuest->GetRequestItemsText().empty())
    {
        SendQuestGiverOfferReward(pQuest, npcGUID, true);
        return;
    }

    WorldPacket data(SMSG_QUESTGIVER_REQUEST_ITEMS, 50);  // guess size
    data << npcGUID;
    data << pQuest->GetQuestId();
    data << pQuest->GetName();
    data << pQuest->GetRequestItemsText();

    data << uint32(0x00);                                   // unknown

    if (Completable)
        data << pQuest->GetCompleteEmote();
    else
        data << pQuest->GetIncompleteEmote();

    // Close Window after cancel
    if (CloseOnCancel)
        data << uint32(0x01);
    else
        data << uint32(0x00);

    data << uint32(0x00);                                   // unknown

    // Required Money
    data << uint32(pQuest->GetRewOrReqMoney() < 0 ? -pQuest->GetRewOrReqMoney() : 0);

    data << uint32(pQuest->GetReqItemsCount());
    ItemPrototype const *pItem;
    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        if (!pQuest->ReqItemId[i]) continue;
        pItem = ObjectMgr::GetItemPrototype(pQuest->ReqItemId[i]);
        data << uint32(pQuest->ReqItemId[i]);
        data << uint32(pQuest->ReqItemCount[i]);

        if (pItem)
            data << uint32(pItem->DisplayInfoID);
        else
            data << uint32(0);
    }

    if (!Completable)
        data << uint32(0x00);
    else
        data << uint32(0x03);

    data << uint32(0x04);
    data << uint32(0x08);
    data << uint32(0x10);

    pSession->SendPacket(&data);
    sLog.outDebug("WORLD: Sent SMSG_QUESTGIVER_REQUEST_ITEMS NPCGuid=%u, questid=%u", GUID_LOPART(npcGUID), pQuest->GetQuestId());
}
