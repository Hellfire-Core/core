file(GLOB_RECURSE framework_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h)

source_group("Other"
  REGULAR_EXPRESSION .*
)

source_group("GameSystem"
  REGULAR_EXPRESSION GameSystem
)

source_group("Platform"
  REGULAR_EXPRESSION Platform
)

source_group("Policies"
  REGULAR_EXPRESSION Policies
)

source_group("Utilities"
  REGULAR_EXPRESSION Utilities
)

source_group("LinkedReference"
  REGULAR_EXPRESSION LinkedReference
)

source_group("Dynamic"
  REGULAR_EXPRESSION Dynamic
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TBB_INCLUDE_DIRS}
  ${ACE_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/src/shared
)

add_library(framework STATIC
  ${framework_SRCS}
)

SET_TARGET_PROPERTIES (framework PROPERTIES FOLDER "Game Libs")

target_link_libraries(framework
  ${TBB_LIBRARIES}
)

if(NOT ACE_USE_EXTERNAL)
  add_dependencies(framework ace)
endif()
