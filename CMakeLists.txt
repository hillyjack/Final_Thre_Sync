cmake_minimum_required(VERSION 3.9)
project(Final_Thre_Sync C)

set(CMAKE_C_STANDARD 99)
set(GCC_COVERAGE_LINK_FLAGS    "-pthread")
set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )

add_executable(Final_Thre_Sync main.c producer.c producer.h includes.h consumer.c consumer.h)
target_link_libraries(Final_Thre_Sync rt)