# Tests CMakeLists.txt

# Fetch Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Add test executable for algorithm5 helper functions
add_executable(test_algorithm5_helpers
    test_top_k_by_degree.cpp
)

# Link against gtest and algorithm includes
target_link_libraries(test_algorithm5_helpers
    PRIVATE
    gtest_main
    gtest
)

target_include_directories(test_algorithm5_helpers
    PRIVATE
    ${CMAKE_SOURCE_DIR}/algorithms
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(test_algorithm5_helpers)
