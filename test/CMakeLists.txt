#
# test/CMakeLists.txt
# CTest integration of googletest

find_package(GTest REQUIRED)

set(GTEST_EXE testParallelRngManager)
set(GTEST_SRCS test_parallel_rng.cpp)
add_executable(${GTEST_EXE} ${GTEST_SRCS})
target_link_libraries(${GTEST_EXE} ${PROJECT_NAME}) 
target_link_libraries(${GTEST_EXE} GTest::GTest GTest::Main)

add_test(NAME GTest COMMAND ${GTEST_EXE})
