set(FSP_SOURCES
  problems/fastfspeval.cpp
  FSPProblemFactory.cpp
  MHParamsSpecsFactory.cpp
  global.cpp
)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_library(flowshop_solver_lib STATIC ${FSP_SOURCES})
set_property(TARGET flowshop_solver_lib PROPERTY POSITION_INDEPENDENT_CODE ON)
add_library(flowshop_solver_lib_shared SHARED ${FSP_SOURCES})
set_property(TARGET flowshop_solver_lib_shared PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(flowshop_solver_lib stdc++fs Eigen3::Eigen)
target_link_libraries(flowshop_solver_lib_shared stdc++fs Eigen3::Eigen)

target_include_directories(flowshop_solver_lib PUBLIC ${CMAKE_SOURCE_DIR}/src/flowshop-solver)
target_include_directories(flowshop_solver_lib_shared PUBLIC ${CMAKE_SOURCE_DIR}/src/flowshop-solver)

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
