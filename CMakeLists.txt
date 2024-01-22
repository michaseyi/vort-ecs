cmake_minimum_required(VERSION 3.2)
project(vort-ecs)

add_library(vort-ecs)
# add_executable(vort-ecs main.cpp)

file(GLOB_RECURSE VORT_ECS_SOURCES src/**.cpp)
target_sources(vort-ecs PRIVATE ${VORT_ECS_SOURCES})


set_target_properties(vort-ecs PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)


target_compile_options(vort-ecs PUBLIC -Wall -Wextra -pedantic)

# set_target_properties(vort-ecs PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# set_target_properties(vort-ecs PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
