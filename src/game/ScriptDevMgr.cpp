#include "Common.h"
#include "Policies/SingletonImp.h"
#include "Config/Config.h"
#include "Database/DatabaseEnv.h"
#include "DBCStores.h"
#include "ObjectMgr.h"
#include "ProgressBar.h"
#include "SystemConfig.h"
#include "ScriptDevMgr.h"
#include "ace/Singleton.h"
#include "ScriptedCreature.h"

#ifdef BUILD_SCRIPTDEV
#include "system/ScriptLoader.h"
#endif

INSTANTIATE_SINGLETON_1(ScriptDevMgr);

// Utility macros to refer to the script registry.
#define SCR_REG_MAP(T) ScriptRegistry<T>::ScriptMap
#define SCR_REG_LST(T) ScriptRegistry<T>::ScriptPointerList

// Utility macros for looping over scripts.
#define FOR_SCRIPTS(T,C,E) \
    if (SCR_REG_LST(T).empty()) \
        return; \
    for (SCR_REG_MAP(T)::iterator C = SCR_REG_LST(T).begin(); \
        C != SCR_REG_LST(T).end(); ++C)
#define FOR_SCRIPTS_RET(T,C,E,R) \
    if (SCR_REG_LST(T).empty()) \
        return R; \
    for (SCR_REG_MAP(T)::iterator C = SCR_REG_LST(T).begin(); \
        C != SCR_REG_LST(T).end(); ++C)
#define FOREACH_SCRIPT(T) \
    FOR_SCRIPTS(T, itr, end) \
    itr->second

// Utility macros for finding specific scripts.
#define GET_SCRIPT(T,I,V) \
    T* V = ScriptRegistry<T>::GetScriptById(I); \
    if (!V) \
        return;
#define GET_SCRIPT_RET(T,I,V,R) \
    T* V = ScriptRegistry<T>::GetScriptById(I); \
    if (!V) \
        return R;

ScriptDevMgr::ScriptDevMgr()
{
}

ScriptDevMgr::~ScriptDevMgr()
{
#define SCR_CLEAR(T) \
        FOR_SCRIPTS(T, itr, end) \
            delete itr->second; \
        SCR_REG_LST(T).clear();

    // Clear scripts for every script type.
    /*SCR_CLEAR(SpellHandlerScript);
    SCR_CLEAR(AuraHandlerScript);
    SCR_CLEAR(ServerScript);
    SCR_CLEAR(WorldScript);
    SCR_CLEAR(GroupScript);
    SCR_CLEAR(FormulaScript);
    SCR_CLEAR(WorldMapScript);
    SCR_CLEAR(InstanceMapScript);
    SCR_CLEAR(BattlegroundMapScript);
    SCR_CLEAR(ItemScript);
    SCR_CLEAR(CreatureScript);
    SCR_CLEAR(GameObjectScript);
    SCR_CLEAR(AreaTriggerScript);
    SCR_CLEAR(BattlegroundScript);
    SCR_CLEAR(OutdoorPvPScript);
    SCR_CLEAR(CommandScript);
    SCR_CLEAR(WeatherScript);
    SCR_CLEAR(AuctionHouseScript);
    SCR_CLEAR(ConditionScript);
    SCR_CLEAR(DynamicObjectScript);
    SCR_CLEAR(TransportScript);*/
    SCR_CLEAR(PlayerScript);


#undef SCR_CLEAR
}

/* #############################################
   #                PlayerScripts
   #
   ############################################# */

void ScriptDevMgr::OnPlayerLogin(Player* player)
{
    FOREACH_SCRIPT(PlayerScript)->OnLogin(player);
}

void ScriptDevMgr::OnPlayerLevelChanged(Player* player, uint8 oldLevel, uint8 newLevel)
{
    FOREACH_SCRIPT(PlayerScript)->OnLevelChanged(player, oldLevel, newLevel);
}

void ScriptDevMgr::OnGivePlayerXP(Player* player, uint32& amount, Unit* victim)
{
    FOREACH_SCRIPT(PlayerScript)->OnGiveXP(player, amount, victim);
}

PlayerScript::PlayerScript(const char* name)
    : ScriptObject(name)
{
    ScriptDevMgr::ScriptRegistry<PlayerScript>::AddScript(this);
}

template<class TScript>
void ScriptDevMgr::ScriptRegistry<TScript>::AddScript(TScript* const script)
{
    ASSERT(script);

    // See if the script is using the same memory as another script. If this happens, it means that
    // someone forgot to allocate new memory for a script.
    typedef typename ScriptMap::iterator ScriptMapIterator;
    for (ScriptMapIterator it = ScriptPointerList.begin(); it != ScriptPointerList.end(); ++it)
    {
        if (it->second == script)
        {
            sLog.outString("Script '%s' forgot to allocate memory, so this script and/or the script before that can't work.",
                script->ToString());

            return;
        }
    }
    // Get an ID for the script. An ID only exists if it's a script that is assigned in the database
    // through a script name (or similar).
    uint32 id = sScriptMgr.GetScriptId(script->ToString());
    if (id)
    {
        // Try to find an existing script.
        bool existing = false;
        typedef typename ScriptMap::iterator ScriptMapIterator;
        for (ScriptMapIterator it = ScriptPointerList.begin(); it != ScriptPointerList.end(); ++it)
        {
            // If the script names match...
            if (it->second->GetName() == script->GetName())
            {
                // ... It exists.
                existing = true;
                break;
            }
        }

        // If the script isn't assigned -> assign it!
        if (!existing)
        {
            ScriptPointerList[id] = script;
        }
        else
        {
            // If the script is already assigned -> delete it!
            sLog.outString("Script '%s' already assigned with the same script name, so the script can't work.",
                script->ToString());

            delete script;
        }
    }
    else if (script->IsDatabaseBound())
    {
        // The script uses a script name from database, but isn't assigned to anything.
        if (script->GetName().find("example") == std::string::npos)
            sLog.outString("Script named '%s' does not have a script name assigned in database.",
                script->ToString());

        delete script;
    }
    else
    {
        // We're dealing with a code-only script; just add it.
        ScriptPointerList[_scriptIdCounter++] = script;
    }
}

// Instantiate static members of ScriptMgr::ScriptRegistry.
template<class TScript> std::map<uint32, TScript*> ScriptDevMgr::ScriptRegistry<TScript>::ScriptPointerList;
template<class TScript> uint32 ScriptDevMgr::ScriptRegistry<TScript>::_scriptIdCounter;

// Specialize for each script type class like so:
/*template class ScriptDevMgr::ScriptRegistry<SpellHandlerScript>;
template class ScriptDevMgr::ScriptRegistry<AuraHandlerScript>;
template class ScriptDevMgr::ScriptRegistry<ServerScript>;
template class ScriptDevMgr::ScriptRegistry<WorldScript>;*/
template class ScriptDevMgr::ScriptRegistry<PlayerScript>;
/*template class ScriptDevMgr::ScriptRegistry<GroupScript>;
template class ScriptDevMgr::ScriptRegistry<FormulaScript>;
template class ScriptDevMgr::ScriptRegistry<WorldMapScript>;
template class ScriptDevMgr::ScriptRegistry<InstanceMapScript>;
template class ScriptDevMgr::ScriptRegistry<BattlegroundMapScript>;
template class ScriptDevMgr::ScriptRegistry<ItemScript>;
template class ScriptDevMgr::ScriptRegistry<CreatureScript>;
template class ScriptDevMgr::ScriptRegistry<GameObjectScript>;
template class ScriptDevMgr::ScriptRegistry<AreaTriggerScript>;
template class ScriptDevMgr::ScriptRegistry<BattlegroundScript>;
template class ScriptDevMgr::ScriptRegistry<OutdoorPvPScript>;
template class ScriptDevMgr::ScriptRegistry<CommandScript>;
template class ScriptDevMgr::ScriptRegistry<WeatherScript>;
template class ScriptDevMgr::ScriptRegistry<AuctionHouseScript>;
template class ScriptDevMgr::ScriptRegistry<ConditionScript>;
template class ScriptDevMgr::ScriptRegistry<DynamicObjectScript>;
template class ScriptDevMgr::ScriptRegistry<TransportScript>;
template class ScriptDevMgr::ScriptRegistry<MovementHandlerScript>;
template class ScriptDevMgr::ScriptRegistry<BGScript>; */


// Undefine utility macros.
#undef GET_SCRIPT_RET
#undef GET_SCRIPT
#undef FOREACH_SCRIPT
#undef FOR_SCRIPTS_RET
#undef FOR_SCRIPTS
#undef SCR_REG_LST
#undef SCR_REG_MAP