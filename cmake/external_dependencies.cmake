include(FetchContent)

if(ENABLE_TEST)
    FetchContent_Declare(GTest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.16.0
    )

    FetchContent_MakeAvailable(GTest)
endif()

message(STATUS "FetchContent: GTest")
