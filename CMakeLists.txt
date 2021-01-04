######################################################################################
### 0) Set your application properties
######################################################################################
# check cmake version compatibility
cmake_minimum_required(VERSION 3.2)

# set a language for the entire project.
enable_language(CXX)

# sends the -std=c++17 flag to the gcc compiler
set(CMAKE_CXX_STANDARD 17)

# Set the project name
SET(PROJ_NAME "IG with AOS for Flowshop")

# Here define the name and the version of your package
set(PACKAGE_NAME ${PROJ_NAME} CACHE STRING "IG with AOS for Flowshop" FORCE)
set(PACKAGE_VERSION "1" CACHE STRING "1" FORCE)

# Define project
project(${PROJ_NAME})

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Define path to find Paradiseo cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

add_definitions(-DDATA_FOLDER="${CMAKE_SOURCE_DIR}/data")
add_definitions(-DDEPRECATED_MESSAGES="")

#set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lpthread -g -fPIC -v -Wall -Wextra -Wno-logical-op-parentheses -Wno-deprecated-register -Wno-pragma-once-outside-header")

#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ ")

# set(CMAKE_CXX_CLANG_TIDY
#   clang-tidy;
#   -header-filter=.;
#   -checks=-*,clang-diagnostic-*,clang-analyzer-*,-clang-diagnostic-c++98-compat,performance-*,bugprone-*,modernize-*;)


#####################################################################################
### include required modules & utilities
#####################################################################################
include(CMakeBackwardCompatibilityCXX)
include(FindDoxygen)
include(CheckLibraryExists)
include(ExternalProject)

#####################################################################################
### Manage the build type
#####################################################################################

# the user should choose the build type on windows environments,excepted under cygwin (default=none)
#set(CMAKE_DEFAULT_BUILD_TYPE "Debug" CACHE STRING "Variable that stores the default CMake build type" FORCE)
# set(CMAKE_BUILD_TYPE "Debug")

find_program(MEMORYCHECK_COMMAND
    NAMES purify valgrind
    PATHS
    "/usr/local/bin /usr/bin [HKEY_LOCAL_MACHINE\\SOFTWARE\\Rational Software\\Purify\\Setup;InstallFolder]"
    DOC "Path to the memory checking command, used for memory error detection.")

if(CMAKE_BUILD_TYPE MATCHES Debug)
#  add_definitions(-DCMAKE_VERBOSE_MAKEFILE=ON)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

if(CMAKE_BUILD_TYPE MATCHES Release)
    add_definitions(-DEIGEN_NO_DEBUG)
endif(CMAKE_BUILD_TYPE MATCHES Release)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include(cmake/Conan.cmake)
run_conan()

######################################################################################
### 2) include the sources
######################################################################################
# Prepare environment to find Paradiseo lib files

#set(PARADISEO_PROJECT_FOLDER "${CMAKE_SOURCE_DIR}/lib/ParadisEO-2.0")
#buld_external_project(paradiseo paradiseo ${PARADISEO_PROJECT_FOLDER} -DCMAKE_INSTALL_PREFIX:PATH=build)
#

#set(PARADISEO_SRC ${CMAKE_SOURCE_DIR}/lib/paradiseo)
#set(PARADISEO_ROOT ${CMAKE_SOURCE_DIR}/build/paradiseo)
#ExternalProject_Add(
#  ParadisEO
#  GIT_REPOSITORY "https://scm.gforge.inria.fr/anonscm/git/paradiseo/paradiseo.git"
#  GIT_TAG "master"
#  GIT_SHALLOW 1
#  GIT_PROGRESS 1
#  UPDATE_COMMAND ""
#  PATCH_COMMAND ""
#  SOURCE_DIR ${PARADISEO_SRC}
#  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PARADISEO_ROOT}
#  TEST_COMMAND ""
#)

#set(PARADISEO_INCLUDES_TO_FIND eo mo moeo)
#foreach(FIND_PARADISEO_COMPONENT ${PARADISEO_INCLUDES_TO_FIND})
#    set(PARADISEO_INCLUDE_DIR ${PARADISEO_INCLUDE_DIR} "${CMAKE_SOURCE_DIR}/lib/paradiseo/${FIND_PARADISEO_COMPONENT}/src")
#endforeach()
#set(PARADISEO_LIBRARIES_TO_FIND eo eoutils cma es ga moeo)
#foreach(FIND_PARADISEO_COMPONENT ${PARADISEO_LIBRARIES_TO_FIND})
#    add_library(${FIND_PARADISEO_COMPONENT} IMPORTED STATIC)
#    set_property(TARGET ${FIND_PARADISEO_COMPONENT} PROPERTY IMPORTED_LOCATION "${PARADISEO_ROOT}/lib64/lib${FIND_PARADISEO_COMPONENT}.a")
#    #set(PARADISEO_LIBRARIES ${PARADISEO_LIBRARIES} "${PARADISEO_ROOT}/lib64/lib${FIND_PARADISEO_COMPONENT}.a")
#    list(APPEND PARADISEO_LIBRARIES ${FIND_PARADISEO_COMPONENT})
#    add_dependencies(${FIND_PARADISEO_COMPONENT} ParadisEO)
#endforeach()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/module")
find_package(Paradiseo COMPONENTS moeo eoutils eo)
include_directories(SYSTEM ${PARADISEO_INCLUDE_DIR})

find_package(Eigen3 3.3 REQUIRED NO_MODULE)

######################################################################################
### 3) Link the libraries for your executable
######################################################################################

set(FSP_SOURCES
  src/flowshop-solver/FSPProblemFactory.cpp
  src/flowshop-solver/MHParamsSpecsFactory.cpp
  src/flowshop-solver/global.cpp
)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_library(flowshop_solver_lib STATIC ${FSP_SOURCES})
set_property(TARGET flowshop_solver_lib PROPERTY POSITION_INDEPENDENT_CODE ON)
add_library(flowshop_solver_lib_shared SHARED ${FSP_SOURCES})
set_property(TARGET flowshop_solver_lib_shared PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(flowshop_solver_lib Eigen3::Eigen)
target_link_libraries(flowshop_solver_lib_shared Eigen3::Eigen)

target_include_directories(flowshop_solver_lib PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(flowshop_solver_lib_shared PUBLIC ${CMAKE_SOURCE_DIR}/src)

set_target_properties(
  flowshop_solver_lib PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  COMPILE_FLAGS "${WARNING_FLAGS}"
)

# file(GLOB FSP_MAIN *.hpp)
# file(GLOB FSP_PROBLEMS problems/*.hpp)
# file(GLOB FSP_HEURISTICS heuristics/*.hpp)
# file(GLOB FSP_AOS aos/*.hpp)
# set(CMAKE_INSTALL_PREFIX "")
# install (FILES ${FSP_MAIN}
#          DESTINATION include/flowshop-solver/)
# install (FILES ${FSP_PROBLEMS}
#         DESTINATION include/flowshop-solver/problems)
# install (FILES ${FSP_HEURISTICS}
#          DESTINATION include/flowshop-solver/heuristics)
# install (FILES ${FSP_AOS}
#          DESTINATION include/flowshop-solver/aos)
# install(TARGETS flowshop_solver_lib flowshop_solver_lib_shared
#         RUNTIME DESTINATION bin/flowshop-solver/
#         LIBRARY DESTINATION lib/flowshop-solver/
#         ARCHIVE DESTINATION lib/static/flowshop-solver/)

######################################################################################
# Link the main executable
######################################################################################
## If you want generate main executable in a separate directory at build
## directory, uncomment line bellow
## add_subdirectory(${APP_DIR})
## Comment out line above if you  want generate main executable in build
## directory, AND uncomment line bellow.

add_subdirectory(${CMAKE_SOURCE_DIR}/main)

######################################################################################
# Create tests and docs
######################################################################################
# add_subdirectory(doc)

enable_testing()
add_subdirectory(${CMAKE_SOURCE_DIR}/test)
