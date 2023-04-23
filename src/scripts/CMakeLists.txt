set(LIBRARY_NAME scripts)

file(GLOB_RECURSE SCRIPTS_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h)

set(SCRIPTS_SRCS
  ${SCRIPTS_SRCS}
)

if((${CMAKE_VERSION} VERSION_LESS "3.16") OR USE_PCH_OLD)
  list(APPEND SCRIPTS_SRCS
        PrecompiledHeaders/scriptPCH.cpp
    )
  # Enable precompiled headers when using the GCC compiler.
  if(USE_PCH)
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
  endif()
endif()

if(WIN32)
  if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
  endif()
endif()

# Used in game ScriptMgr, not need here
list(REMOVE_ITEM SCRIPTS_SRCS ScriptLoader.cpp)
list(REMOVE_ITEM SCRIPTS_SRCS ScriptLoader.h)

foreach(SRC ${SCRIPTS_SRCS})
  get_filename_component(PTH ${SRC} PATH)
  if(PTH)
    if(NOT XCODE) # FIXME: Xcode Generator has bug with nested dirs
      string(REPLACE "/" "\\\\" PTH ${PTH})
    endif()
    source_group(${PTH} FILES ${SRC})
  endif()
endforeach(SRC)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/dep/include/g3dlite
  ${CMAKE_SOURCE_DIR}/dep/include
  ${CMAKE_SOURCE_DIR}/src/game
  ${CMAKE_SOURCE_DIR}/src/game/AI
  ${CMAKE_SOURCE_DIR}/src/game/Anticheat
  ${CMAKE_SOURCE_DIR}/src/game/Anticheat/WardenAnticheat
  ${CMAKE_SOURCE_DIR}/src/game/AuctionHouse
  ${CMAKE_SOURCE_DIR}/src/game/Battlegrounds
  ${CMAKE_SOURCE_DIR}/src/game/Chat
  ${CMAKE_SOURCE_DIR}/src/game/Commands
  ${CMAKE_SOURCE_DIR}/src/game/Database
  ${CMAKE_SOURCE_DIR}/src/game/Group
  ${CMAKE_SOURCE_DIR}/src/game/Guild
  ${CMAKE_SOURCE_DIR}/src/game/Handlers
  ${CMAKE_SOURCE_DIR}/src/game/LFG
  ${CMAKE_SOURCE_DIR}/src/game/Mail
  ${CMAKE_SOURCE_DIR}/src/game/Maps
  ${CMAKE_SOURCE_DIR}/src/game/Maps/Pool
  ${CMAKE_SOURCE_DIR}/src/game/Movement
  ${CMAKE_SOURCE_DIR}/src/game/Movement/spline
  ${CMAKE_SOURCE_DIR}/src/game/Objects
  ${CMAKE_SOURCE_DIR}/src/game/OutdoorPvP
  ${CMAKE_SOURCE_DIR}/src/game/PlayerBots
  ${CMAKE_SOURCE_DIR}/src/game/Protocol
  ${CMAKE_SOURCE_DIR}/src/game/Spells
  ${CMAKE_SOURCE_DIR}/src/game/Threat
  ${CMAKE_SOURCE_DIR}/src/game/Transports
  ${CMAKE_SOURCE_DIR}/src/game/vmap
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_SOURCE_DIR}/src/shared/Database
  ${CMAKE_SOURCE_DIR}/src/framework
  ${CMAKE_SOURCE_DIR}/src/framework/Network
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/src/shared
  ${TBB_INCLUDE_DIRS}
  ${ACE_INCLUDE_DIR}
  ${MYSQL_INCLUDE_DIR}
)

add_library(${LIBRARY_NAME} STATIC
  ${SCRIPTS_SRCS}
)

SET_TARGET_PROPERTIES (${LIBRARY_NAME} PROPERTIES FOLDER "Game Libs")

target_link_libraries(scripts
  game
)

#add_dependencies(${LIBRARY_NAME} revision.h)

if((${CMAKE_VERSION} VERSION_LESS "3.16") OR USE_PCH_OLD)
  if(USE_PCH)
    if(MSVC OR XCODE)
      if(MSVC)
        set(scripts_pch "${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders/scriptPCH.cpp")
      endif()
      add_native_precompiled_header(scripts ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders/scriptPCH.h)
    elseif(CMAKE_COMPILER_IS_GNUCXX)
      add_precompiled_header(scripts ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders/scriptPCH.h)
    endif()
  endif()
else()
  set(PCHS
      ../game/ScriptMgr.h
      ../game/Objects/Object.h
      ../game/ObjectMgr.h
      ../game/Objects/Unit.h
      ../game/Spells/Spell.h
      ../game/Objects/Creature.h
      ../game/AI/CreatureAI.h
      ../game/Objects/GameObject.h
      ../game/AI/ScriptedInstance.h
      ../game/Spells/SpellAuras.h
      ../game/Objects/TemporarySummon.h
      ../game/Chat/Chat.h
      ../game/Battlegrounds/BattleGroundAV.h
      ../game/World.h
      ../game/Weather.h
      ../game/AI/TotemAI.h
      PrecompiledHeaders/scriptPCH.h
      )

  if(WIN32)
      list(APPEND PCHS <windows.h>)
  endif()

  # Generate precompiled header
  if(USE_PCH)
      target_precompile_headers(${LIBRARY_NAME} PRIVATE
          ${PCHS}
          )
  endif()
endif()

