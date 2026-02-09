cmake_minimum_required(VERSION 3.10)

project(halloc)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(halloc "src/halloc.c")
target_include_directories(halloc PUBLIC "${CMAKE_SOURCE_DIR}/include")

find_package(Threads REQUIRED)
target_link_libraries(halloc PUBLIC Threads::Threads)
target_compile_options(halloc PRIVATE -Wall -Wextra -Wpedantic)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
   add_executable(halloc-app src/main.c)
   target_link_libraries(halloc-app PRIVATE halloc)
endif()
