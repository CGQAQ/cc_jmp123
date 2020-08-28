# Testing

enable_testing()

find_package(GTest CONFIG REQUIRED)

add_executable(test_works test_works.cpp)
target_link_libraries(test_works PRIVATE GTest::gtest GTest::gtest_main #[[GTest::gmock GTest::gmock_main]])

add_executable(tests test_synthesis.cpp )
target_link_libraries(tests PRIVATE GTest::gtest GTest::gtest_main fmt libjmp123)
target_include_directories(tests PRIVATE ../)

add_test(AllTestsInMain test_works tests)