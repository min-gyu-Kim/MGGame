file(GLOB_RECURSE CORE_TEST_SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(core_test ${CORE_TEST_SRCS})
target_link_libraries(core_test PRIVATE MG::core GTest::gtest)

gtest_discover_tests(core_test)
