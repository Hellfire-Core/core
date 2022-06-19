set(LIBRARY_NAME scripts)

file(GLOB_RECURSE SCRIPTS_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/dep/include/g3dlite
  ${CMAKE_SOURCE_DIR}/dep/include
  ${CMAKE_SOURCE_DIR}/src/game
  ${CMAKE_SOURCE_DIR}/src/game/movemap
  ${CMAKE_SOURCE_DIR}/src/game/vmap
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_SOURCE_DIR}/src/shared/Database
  ${CMAKE_SOURCE_DIR}/src/framework
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/src/shared
  ${MYSQL_INCLUDE_DIR}
  ${ACE_INCLUDE_DIR}
)

source_group("Scripts"
  REGULAR_EXPRESSION .*
)

source_group("Manager"
  REGULAR_EXPRESSION ScriptMgr
)

source_group("System"
  REGULAR_EXPRESSION ScriptLoader|system|sc_gossip|sc_instance|precompiled
)

source_group("Bosses"
  REGULAR_EXPRESSION boss*
)

source_group("Instances"
  REGULAR_EXPRESSION instance*
)

source_group("Areatrigger"
  REGULAR_EXPRESSION areatrigger*
)

source_group("Creatures and Npcs"
  REGULAR_EXPRESSION mob|npc|npcs|guard
)

source_group("Items"
  REGULAR_EXPRESSION item*
)

source_group("Game Objects"
  REGULAR_EXPRESSION go*
)

source_group("Spells"
  REGULAR_EXPRESSION spell*
)
source_group("AI"
  REGULAR_EXPRESSION escort_ai|follower_ai|simple_ai|sc_creature
)

add_library(${LIBRARY_NAME} STATIC
  ${SCRIPTS_SRCS}
)

SET_TARGET_PROPERTIES (${LIBRARY_NAME} PROPERTIES FOLDER "Game Libs")

target_link_libraries(scripts
  game
)

if(PCH)
  if(MSVC OR XCODE)
    if(MSVC)
      set(scripts_pch "${CMAKE_CURRENT_SOURCE_DIR}/include/precompiled.cpp")
    endif()
    add_native_precompiled_header(scripts ${CMAKE_CURRENT_SOURCE_DIR}/include/precompiled.h)
  elseif(CMAKE_COMPILER_IS_GNUCXX)
    add_precompiled_header(scripts ${CMAKE_CURRENT_SOURCE_DIR}/include/precompiled.h)
  endif()
endif()