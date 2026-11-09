cmake_minimum_required(VERSION 3.20)

project(ParallelProcessingWorkbench LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR})

add_library(timer
        Utils/timer.cpp
)

add_library(workloads
        Seq-Thread-OMP/workload.cpp
)

add_library(openmp
        OpenMP/openMPExecutor.cpp
)

find_package(OpenMP REQUIRED COMPONENTS CXX)

target_link_libraries(openmp PUBLIC
        workloads
        OpenMP::OpenMP_CXX
)

add_executable(openmp_test
        tests/openmp_test.cpp
)

target_link_libraries(openmp_test PRIVATE
        openmp
        timer
)

add_executable(timer_test
        tests/timer_test.cpp
)

target_link_libraries(timer_test PRIVATE timer)