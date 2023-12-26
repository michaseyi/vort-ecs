cmake_minimum_required(VERSION 3.2)
project(vort-ecs)

add_library(vort-ecs)

file(GLOB_RECURSE VORT_ECS_SOURCES src/**.cpp)
target_sources(vort-ecs PRIVATE ${VORT_ECS_SOURCES})
