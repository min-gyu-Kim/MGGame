cmake_minimum_required(VERSION 3.20)

project(
    MGGame
    VERSION 0.1.0
    DESCRIPTION "MG Game Project"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_TEST "Enable tests(unit test, logic test, performance test...)" ON)
option(STRICT_WARNING "Strict warning" ON)
option(SANITIZERS "" OFF)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(external_dependencies)
include(target_set_ipo)
include(target_set_warnings)
include(turn_on_sanitizers)

if(ENABLE_TEST)
    enable_testing()
    include(GoogleTest)
endif(ENABLE_TEST)

if(SANITIZERS)
    turn_on_sanitizers()
endif(SANITIZERS)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(experimental)

add_subdirectory(libs)
add_subdirectory(tests)
add_subdirectory(app)
