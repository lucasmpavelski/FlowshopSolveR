######################################################################################
### 1) Include the sources (besides PARADISEO_INCLUDE_DIR and 'inc')
######################################################################################

######################################################################################
### 2) Generate executables
######################################################################################

add_executable(flowshop-solver flowshop-solver.cpp)
target_link_libraries(flowshop-solver flowshop_solver_lib ${PARADISEO_LIBRARIES} pthread)

add_executable(random-search random-search.cpp)
target_link_libraries(random-search flowshop_solver_lib ${PARADISEO_LIBRARIES} pthread)

add_executable(fsp_solver fsp_solver.cpp)
target_link_libraries(fsp_solver flowshop_solver_lib ${PARADISEO_LIBRARIES} pthread)

add_executable(refine refine.cpp)
target_link_libraries(refine Eigen3::Eigen flowshop_solver_lib ${PARADISEO_LIBRARIES} pthread)

#add_executable(hppso_myriam hppso_myriam.cpp)
#target_link_libraries(hppso_myriam utils_lib hppso_lib flowshop_solver_lib ${PARADISEO_LIBRARIES})

#add_executable(dde dde.cpp)
#target_link_libraries(dde flowshop_solver_lib ${PARADISEO_LIBRARIES} pthread)

#add_executable(ils2 ils2.cpp)
#target_link_libraries(ils2 flowshop_solver_lib ${PARADISEO_LIBRARIES} pthread)

#add_executable(hppso_run hppso_run.cpp)
#target_link_libraries(hppso_run pso_tunner_lib ${PARADISEO_LIBRARIES})
# flowshop_solver_lib
