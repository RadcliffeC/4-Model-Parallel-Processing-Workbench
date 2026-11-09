cmake_minimum_required(VERSION 4.3)
project(4_Model_Parallel_Processing_Workbench)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(APPLE)
    execute_process(COMMAND brew --prefix libomp
                    OUTPUT_VARIABLE LIBOMP_PREFIX
                    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    if(LIBOMP_PREFIX)
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I${LIBOMP_PREFIX}/include")
        set(OpenMP_CXX_LIB_NAMES "omp")
        set(OpenMP_omp_LIBRARY "${LIBOMP_PREFIX}/lib/libomp.dylib")
    endif()
endif()

find_package(OpenMP REQUIRED)
find_package(Threads REQUIRED)

include_directories(Seq-Thread-OMP)

add_executable(4_Model_Parallel_Processing_Workbench
        Seq-Thread-OMP/input.cpp
        Seq-Thread-OMP/input.h
        Seq-Thread-OMP/main.cpp
        Seq-Thread-OMP/output.cpp
        Seq-Thread-OMP/output.h
        Seq-Thread-OMP/workbench.h
        Seq-Thread-OMP/workload.cpp
        Seq-Thread-OMP/workload.h
        OpenMP/openMPExecutor.cpp
        Utils/timer.cpp
        tests/openmp_test.cpp
        tests/timer_test.cpp
        )
target_link_libraries(4_Model_Parallel_Processing_Workbench PRIVATE OpenMP:OpenMP_CXX Threads::Threads)
target_compile_options(4_Model_Parallel_Processing_Workbench PRIVATE -Wall -Wextra)
