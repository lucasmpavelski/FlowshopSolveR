# 1) Include the sources
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(${PARADISEO_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/flowshop-solver)
INCLUDE_DIRECTORIES(${CATCH_INCLUDE_DIR})

# 2) Define targets: executable and dependencies
ADD_EXECUTABLE(test-fsp test-fsp.cpp)
ADD_EXECUTABLE(test-heuristic test-heuristic.cpp)
ADD_EXECUTABLE(test-solve test-solve.cpp)
ADD_EXECUTABLE(test-fla test-fla.cpp)
ADD_EXECUTABLE(test-jsp test-jsp.cpp)
ADD_EXECUTABLE(test-aco test-aco.cpp)
#ADD_EXECUTABLE(test-comparators test-comparators.cpp)
#ADD_EXECUTABLE(test-or-tools test-or-tools.cpp)
add_executable(test-all test-all.cpp)
add_executable(test-mh-params-specs test-mh-params-specs.cpp)
add_executable(test-aos test-aos.cpp)

ADD_DEFINITIONS(-DTEST_FIXTURES_FOLDER="${CMAKE_SOURCE_DIR}/test/")

target_link_libraries(test-all flowshop_solver_lib ${GTEST_LIBRARIES} ${PARADISEO_LIBRARIES} pthread)
TARGET_LINK_LIBRARIES(test-fsp flowshop_solver_lib ${GTEST_LIBRARIES} ${PARADISEO_LIBRARIES})
TARGET_LINK_LIBRARIES(test-heuristic flowshop_solver_lib ${PARADISEO_LIBRARIES})
TARGET_LINK_LIBRARIES(test-solve flowshop_solver_lib ${PARADISEO_LIBRARIES})
TARGET_LINK_LIBRARIES(test-fla flowshop_solver_lib ${GTEST_LIBRARIES} ${PARADISEO_LIBRARIES} pthread)
TARGET_LINK_LIBRARIES(test-jsp flowshop_solver_lib ${PARADISEO_LIBRARIES})
TARGET_LINK_LIBRARIES(test-aco flowshop_solver_lib ${PARADISEO_LIBRARIES})
TARGET_LINK_LIBRARIES(test-mh-params-specs flowshop_solver_lib ${GTEST_LIBRARIES} ${PARADISEO_LIBRARIES} pthread)
#TARGET_LINK_LIBRARIES(test-comparators pso_tunner_lib ${PARADISEO_LIBRARIES})
TARGET_LINK_LIBRARIES(test-aos flowshop_solver_lib ${GTEST_LIBRARIES} ${PARADISEO_LIBRARIES} pthread)

#add_dependencies(test-fsp ParadisEO)
#add_dependencies(test-heuristic ParadisEO)

add_test(TestAllSolvers test-all)
add_test(TestMHParamsSpecs test-mh-params-specs)
add_test(TestFLA test-fla)

