set(FSP_SOURCES
  aos/probability_matching.hpp
  aos/adaptive_operator_selection.hpp
  aos/frrmab.hpp
  aos/lin_ucb.hpp
  aos/beta_distribution.hpp
  aos/thompson_sampling.hpp
  continuators/myMovedSolutionStat.hpp
  heuristics/acceptCritTemperature.hpp
  heuristics/dummyAspiration.hpp
  heuristics/fastnehheuristic.hpp
  heuristics/FSPOrderHeuristics.hpp
  heuristics/IGexplorer.hpp
  heuristics/IGexplorerWithRepl.hpp
  heuristics/moFirstBestTSexplorer.hpp
  heuristics/moFirstBestTS.hpp
  heuristics/moFirstTSexplorer.hpp
  heuristics/moFirstTS.hpp
  heuristics/NEHInit.hpp
  heuristics/OpPerturbDestConst.hpp
  heuristics/plateauExplorerPerturb.hpp
  heuristics/randomNeutralWalkExplorer.hpp
  heuristics/sa.hpp
  heuristics/hc.hpp
  heuristics/ts.hpp
  heuristics/ihc.hpp
  heuristics/isa.hpp
  heuristics/ils.hpp
  heuristics/aco.hpp
  heuristics/op_cooling_schedule.hpp
  problems/fastfspeval.cpp
  problems/fastfspeval.hpp
  problems/FSPData.cpp
  problems/FSPData.hpp
  problems/FSPEvalFunc.hpp
  problems/FSP.hpp
  problems/FSPProblem.hpp
  problems/NIFSPEvalFunc.hpp
  problems/NWFSPEvalFunc.hpp
  problems/Problem.hpp
  moHiResTimeContinuator.hpp
  fla_methods.hpp
  fspproblemfactory.cpp
  fspproblemfactory.hpp
  specsdata.cpp
  specsdata.hpp
  global.cpp
  global.hpp
  MHParamsSpecs.hpp
  MHParamsValues.hpp
  MHSolve.hpp
  ParamSpec.hpp
  heuristics.hpp
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
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  COMPILE_FLAGS "${WARNING_FLAGS}"
)

file(GLOB FSP_MAIN *.hpp)
file(GLOB FSP_PROBLEMS problems/*.hpp)
file(GLOB FSP_HEURISTICS heuristics/*.hpp)
file(GLOB FSP_AOS aos/*.hpp)

set(CMAKE_INSTALL_PREFIX "")
install (FILES ${FSP_MAIN}
         DESTINATION include/flowshop-solver/)
install (FILES ${FSP_PROBLEMS}
        DESTINATION include/flowshop-solver/problems)
install (FILES ${FSP_HEURISTICS}
         DESTINATION include/flowshop-solver/heuristics)
install (FILES ${FSP_AOS}
         DESTINATION include/flowshop-solver/aos)
install(TARGETS flowshop_solver_lib flowshop_solver_lib_shared
        RUNTIME DESTINATION bin/flowshop-solver/
        LIBRARY DESTINATION lib/flowshop-solver/
        ARCHIVE DESTINATION lib/static/flowshop-solver/)
