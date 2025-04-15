if(WIN32)
    project(StressClient
        LANGUAGES CXX
        VERSION 0.1.0
    )

    add_executable(${PROJECT_NAME})
    target_sources(${PROJECT_NAME} PRIVATE "main.cpp" "StressAgent.cpp")

    target_set_warnings(TARGET ${PROJECT_NAME})
    target_set_ipo(TARGET ${PROJECT_NAME})
endif()

