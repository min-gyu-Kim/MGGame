
project(Server
    LANGUAGES CXX
    VERSION 0.1.0
)

find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE "main.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE MG::core Threads::Threads uring)

target_set_warnings(TARGET ${PROJECT_NAME})
target_set_ipo(TARGET ${PROJECT_NAME})

