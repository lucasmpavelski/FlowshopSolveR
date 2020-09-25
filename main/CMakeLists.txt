######################################################################################
### 1) Include the sources (besides PARADISEO_INCLUDE_DIR and 'inc')
######################################################################################

######################################################################################
### 2) Generate executables
######################################################################################

add_executable(fsp_solver fsp_solver.cpp)
target_link_libraries(fsp_solver flowshop_solver_lib ${PARADISEO_LIBRARIES} pthread)
