cmake_minimum_required(VERSION 3.10)
set(CMAKE_SUPPRESS_REGENERATION true)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Override platform if generating for Visual Studio

string(REGEX MATCH "Visual Studio" IS_VISUAL_STUDIO ${CMAKE_GENERATOR})
if(IS_VISUAL_STUDIO)
  set(CMAKE_GENERATOR_PLATFORM x64)
endif()

include(CheckCXXCompilerFlag)
include(ExternalProject)

project(L)

# Configuration types

set(CMAKE_CONFIGURATION_TYPES "Debug;Development;Release" CACHE INTERNAL "Configs" FORCE)
set(DEV_DBG $<NOT:$<CONFIG:Release>>)
add_compile_definitions(
  L_DBG=$<CONFIG:Debug>
  L_DEV=$<CONFIG:Development>
  L_RLS=$<CONFIG:Release>
  L_LINUX=$<PLATFORM_ID:Linux>
  L_WINDOWS=$<PLATFORM_ID:Windows>
)
set(CMAKE_EXE_LINKER_FLAGS_DEVELOPMENT ${CMAKE_EXE_LINKER_FLAGS_DEBUG} CACHE INTERNAL "Configs")

# Compiler options

function(add_compile_options_safe)
  foreach(OPT ${ARGV})
    check_cxx_compiler_flag(${OPT} HAS_COMPILER_OPTION_${OPT})
    if(HAS_COMPILER_OPTION_${OPT})
      add_compile_options(${OPT})
    endif()
  endforeach()
endfunction()

if(MSVC)
  add_compile_definitions(
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    NOCRYPT
    _CRT_SECURE_NO_WARNINGS
    _WINSOCK_DEPRECATED_NO_WARNINGS
    _HAS_EXCEPTIONS=0
  )

  string(REPLACE "/EHsc" "/EHs-c-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  add_compile_options(
    /GR-
    /fp:fast
    /WX /W4
  )
  add_link_options(
    $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>
  )
else()
  add_compile_options(
    -fno-operator-names
    -fno-exceptions
    -fno-rtti
    -ffast-math
    -Werror -Wall
  )
  add_compile_options_safe(
    -Wno-class-memaccess
    -Wno-missing-braces
    -Wno-missing-field-initializers
    -Wno-type-limits
    -Wno-unused-function
  )
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

# Libraries

if(WIN32)
  link_libraries(ws2_32 $<${DEV_DBG}:dbghelp>)
else()
  link_libraries(X11 pthread)
endif()

# Vulkan

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
link_libraries(${Vulkan_LIBRARIES})

# Source files

file(GLOB_RECURSE CORE_SOURCES src/*.cpp src/*.inl src/*.h)
if(WIN32)
  list(FILTER CORE_SOURCES EXCLUDE REGEX .*_unix.cpp)
else()
  list(FILTER CORE_SOURCES EXCLUDE REGEX .*_win.cpp)
endif()
if(MSVC)
  list(APPEND CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resource.rc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/L.natvis
  )
endif()
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${CORE_SOURCES})

# Binary

add_executable(core ${CORE_SOURCES})
set_property(DIRECTORY . PROPERTY VS_STARTUP_PROJECT core)
set(L_GAME_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/smp" CACHE PATH "Game data directory")
set_target_properties(
  core PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${L_GAME_DIRECTORY}
  RUNTIME_OUTPUT_DIRECTORY_DEVELOPMENT ${L_GAME_DIRECTORY}
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${L_GAME_DIRECTORY}
  VS_DEBUGGER_WORKING_DIRECTORY ${L_GAME_DIRECTORY}
  OUTPUT_NAME_DEBUG Ldbg
  OUTPUT_NAME_DEVELOPMENT Ldev
  OUTPUT_NAME_RELEASE L
)

# Modules - function

function(add_module NAME)
  set(OPTARGS "")
  set(SINGLEARGS CONDITION)
  set(MULTIARGS
    SOURCES
    LINK_DIRS INCLUDE_DIRS SYSTEM_INCLUDE_DIRS
    LIBRARIES DBG_LIBRARIES RLS_LIBRARIES
    DEPENDENCIES MOD_DEPENDENCIES
  )
  cmake_parse_arguments(M "${OPTARGS}" "${SINGLEARGS}" "${MULTIARGS}" ${ARGN})
  if(NOT DEFINED M_CONDITION)
    set(M_CONDITION 1) # Always use module if no condition set
  endif()

  add_library(mod_${NAME} ${M_SOURCES})
  set_target_properties(
    mod_${NAME} PROPERTIES
    EXCLUDE_FROM_ALL TRUE
  )

  # Core definition and link
  target_compile_definitions(core PRIVATE $<${M_CONDITION}:L_USE_MODULE_${NAME}>)
  target_link_libraries(core PRIVATE $<${M_CONDITION}:mod_${NAME}>)

  # Module directories
  target_include_directories(mod_${NAME} PRIVATE ${M_INCLUDE_DIRS})
  target_include_directories(mod_${NAME} SYSTEM PRIVATE ${M_SYSTEM_INCLUDE_DIRS})

  # Libraries
  set(MOD_LIB_PATTERN "^(.*/)(.*)$")
  set(MOD_LIB_REPLACE "\\1${CMAKE_STATIC_LIBRARY_PREFIX}\\2${CMAKE_STATIC_LIBRARY_SUFFIX}")
  list(TRANSFORM M_LIBRARIES REPLACE ${MOD_LIB_PATTERN} ${MOD_LIB_REPLACE})
  list(TRANSFORM M_DBG_LIBRARIES REPLACE ${MOD_LIB_PATTERN} ${MOD_LIB_REPLACE})
  list(TRANSFORM M_RLS_LIBRARIES REPLACE ${MOD_LIB_PATTERN} ${MOD_LIB_REPLACE})
  target_link_libraries(mod_${NAME} ${M_LIBRARIES})
  foreach(LIB ${M_DBG_LIBRARIES})
    target_link_libraries(mod_${NAME} debug ${LIB})
  endforeach()
  foreach(LIB ${M_RLS_LIBRARIES})
    target_link_libraries(mod_${NAME} optimized ${LIB})
  endforeach()

  # Dependencies
  if(DEFINED M_DEPENDENCIES)
    add_dependencies(mod_${NAME} ${M_DEPENDENCIES})
  endif()
  foreach(MOD_DEP ${M_MOD_DEPENDENCIES})
    target_include_directories(mod_${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/mod/${MOD_DEP}/include)
    target_link_libraries(mod_${NAME} mod_${MOD_DEP})
  endforeach()
endfunction()

# Modules - iterate

set(MOD_INIT_GEN_PATH ${CMAKE_CURRENT_BINARY_DIR}/mod_init.gen)
file(WRITE ${MOD_INIT_GEN_PATH} "// This file was generated by CMake, do not modify manually\n")
file(GLOB MOD_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/mod/*)
foreach(MOD_DIR ${MOD_DIRS})
  get_filename_component(MOD_NAME ${MOD_DIR} NAME)
  option(L_ENABLE_${MOD_NAME} "Enable module ${MOD_NAME}" ON)
  if(L_ENABLE_${MOD_NAME})
    include(${MOD_DIR}/module.cmake)
    file(APPEND ${MOD_INIT_GEN_PATH}
      "#if L_USE_MODULE_${MOD_NAME}\n"
      "{\n"
      "#if L_PROFILING\n"
      "ScopeMarker marker_${MOD_NAME}(\"${MOD_NAME}\");\n"
      "#endif\n"
      "extern void ${MOD_NAME}_module_init();\n"
      "${MOD_NAME}_module_init();\n"
      "}\n"
      "#endif\n"
    )
  endif()
endforeach()
configure_file(${MOD_INIT_GEN_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/src/mod_init.gen)
