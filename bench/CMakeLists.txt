find_package(Threads REQUIRED)

#~~~~~~~~~~~~~~~~


INCLUDE_DIRECTORIES(${PARADISEO_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)

file(GLOB_RECURSE ALL_BENCH_CPP *.cpp)

foreach(ONE_BENCH_CPP ${ALL_BENCH_CPP})

   get_filename_component(ONE_BENCH_EXEC ${ONE_BENCH_CPP} NAME_WE)

   # Avoid name collision 
   set(TARGET_NAME Bench_${ONE_BENCH_EXEC})

   add_executable(${TARGET_NAME} ${ONE_BENCH_CPP})
   set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${ONE_BENCH_EXEC} CXX_STANDARD 14) 
   target_link_libraries(${TARGET_NAME} benchmark
       flowshop_solver_lib
       ${PARADISEO_LIBRARIES}
       ${CMAKE_THREAD_LIBS_INIT})

   # If you want to run benchmarks with the "make test" command, uncomment me
   add_test(${TARGET_NAME} ${ONE_BENCH_EXEC})
endforeach()
