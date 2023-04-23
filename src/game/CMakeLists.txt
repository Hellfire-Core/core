file(GLOB_RECURSE game_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h)
if((${CMAKE_VERSION} VERSION_LESS "3.16") OR USE_PCH_OLD)
  list(APPEND game_SRCS
      pchdef.cpp
      pchdef.h
  )
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
endif()

if(WIN32)
  #allow exceptions in destructors
  if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:implicitNoexcept-")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D__ACE_INLINE__")
  elseif(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
  endif()
endif()
# Used in game ScriptMgr
if(USE_SCRIPTS)
  list(APPEND game_SRCS
      ${CMAKE_SOURCE_DIR}/src/scripts/ScriptLoader.cpp
      ${CMAKE_SOURCE_DIR}/src/scripts/ScriptLoader.h
  )
else()
  list(APPEND game_SRCS
      ${CMAKE_SOURCE_DIR}/src/scripts/ScriptLoader_noscripts.cpp
      ${CMAKE_SOURCE_DIR}/src/scripts/ScriptLoader.h
  )
endif()
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/AI
  ${CMAKE_CURRENT_SOURCE_DIR}/Anticheat
  ${CMAKE_CURRENT_SOURCE_DIR}/Anticheat/WardenAnticheat
  ${CMAKE_CURRENT_SOURCE_DIR}/AuctionHouse
  ${CMAKE_CURRENT_SOURCE_DIR}/Battlegrounds
  ${CMAKE_CURRENT_SOURCE_DIR}/Chat
  ${CMAKE_CURRENT_SOURCE_DIR}/Commands
  ${CMAKE_CURRENT_SOURCE_DIR}/Database
  ${CMAKE_CURRENT_SOURCE_DIR}/Group
  ${CMAKE_CURRENT_SOURCE_DIR}/Guild
  ${CMAKE_CURRENT_SOURCE_DIR}/Handlers
  ${CMAKE_CURRENT_SOURCE_DIR}/LFG
  ${CMAKE_CURRENT_SOURCE_DIR}/Mail
  ${CMAKE_CURRENT_SOURCE_DIR}/Maps
  ${CMAKE_CURRENT_SOURCE_DIR}/Maps/Pool
  ${CMAKE_CURRENT_SOURCE_DIR}/Movement
  ${CMAKE_CURRENT_SOURCE_DIR}/Movement/spline
  ${CMAKE_CURRENT_SOURCE_DIR}/Objects
  ${CMAKE_CURRENT_SOURCE_DIR}/OutdoorPvP
  ${CMAKE_CURRENT_SOURCE_DIR}/PacketBroadcast
  ${CMAKE_CURRENT_SOURCE_DIR}/PlayerBots
  ${CMAKE_CURRENT_SOURCE_DIR}/Protocol
  ${CMAKE_CURRENT_SOURCE_DIR}/Spells
  ${CMAKE_CURRENT_SOURCE_DIR}/Threat
  ${CMAKE_CURRENT_SOURCE_DIR}/Transports
  ${CMAKE_CURRENT_SOURCE_DIR}/vmap
  ${CMAKE_SOURCE_DIR}/src/scripts
  ${CMAKE_SOURCE_DIR}/src/scripts/PrecompiledHeaders
  ${CMAKE_SOURCE_DIR}/dep/include/g3dlite
  ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour
  ${CMAKE_SOURCE_DIR}/dep/recastnavigation
  ${CMAKE_SOURCE_DIR}/dep/include
  ${CMAKE_SOURCE_DIR}/src/game/PlayerBots
  ${CMAKE_SOURCE_DIR}/src/shared/Auth
  ${CMAKE_SOURCE_DIR}/src/shared/Database
  ${CMAKE_SOURCE_DIR}/src/framework
  ${CMAKE_SOURCE_DIR}/src/framework/Network
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/src/shared
  ${MYSQL_INCLUDE_DIR}
  ${TBB_INCLUDE_DIRS}
  ${ACE_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR}
  ${ZLIB_INCLUDE_DIRS}
)

if(WIN32)
  include_directories(
    ${CMAKE_SOURCE_DIR}/dep/windows/include
  )
endif()

source_group("Object"
  REGULAR_EXPRESSION .*
)

source_group("World/Handlers"
  REGULAR_EXPRESSION Mgr|Handler|Manager|Cell|Channel|Chat|Gossip|Grid|Instance|Map|Path|Pool|Script|Skill|Transports|Update|Weather|World
)

source_group("Motion generators"
  REGULAR_EXPRESSION MoveMap|MoveMapSharedDefines|Movement|Holder|Motion|Traveller
)

source_group("Server"
  REGULAR_EXPRESSION Socket|Session|Opcodes|DBC
  FILES
    SharedDefines.h
)

source_group("Chat Commands"
  REGULAR_EXPRESSION Level[0-9]
  FILES
    debugcmds.cpp
)

source_group("Tool"
  REGULAR_EXPRESSION DatabaseCleaner|Language|PlayerDump
)

source_group("PlayerBots"
  REGULAR_EXPRESSION Bot
)

source_group("References"
  REGULAR_EXPRESSION Reference|RefManager|ThreatManager
)

source_group("Anticheat"
  REGULAR_EXPRESSION Anticheat
)

add_library(game STATIC
  ${game_SRCS}
)

SET_TARGET_PROPERTIES (game PROPERTIES FOLDER "Game Libs")

target_link_libraries(game
  shared
  Detour
  ${ACE_LIBRARIES} 
  ${OPENSSL_LIBRARIES} 
)

#add_dependencies(game revision.h)

# Generate precompiled header
if((${CMAKE_VERSION} VERSION_LESS "3.16") OR USE_PCH_OLD)
  if(USE_PCH)
    if(MSVC OR XCODE)
      if(MSVC)
        set(game_pch "${CMAKE_CURRENT_SOURCE_DIR}/pchdef.cpp")
      endif()
      add_native_precompiled_header(game ${CMAKE_CURRENT_SOURCE_DIR}/pchdef.h)
    elseif(CMAKE_COMPILER_IS_GNUCXX)
      add_precompiled_header(game ${CMAKE_CURRENT_SOURCE_DIR}/pchdef.h)
    endif()
  endif()
else()
  if(USE_PCH)
      target_precompile_headers(game PRIVATE
      Protocol/WorldSocket.h
      ../shared/Common.h

      Maps/MapManager.h
      ../shared/Log.h
      ObjectAccessor.h
      ObjectGuid.h
      Protocol/Opcodes.h
      SharedDefines.h
      Guild/GuildMgr.h
      ObjectMgr.h
      ScriptMgr.h
      )
  endif()
endif()
