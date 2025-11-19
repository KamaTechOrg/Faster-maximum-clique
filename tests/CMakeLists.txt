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

# Collect all test files
set(TEST_SOURCES
    test_top_k.cpp
    test_candidate_ops.cpp
    test_intersect.cpp
    test_k_core.cpp
    test_ordering.cpp
    test_heuristics.cpp
    test_search.cpp
    test_filter_N.cpp
)

# Create test executable
add_executable(lazymc_tests ${TEST_SOURCES})

# Link against gtest and lazymc
target_link_libraries(lazymc_tests
    PRIVATE
    gtest_main
    gtest
    lazymc
)

target_include_directories(lazymc_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(lazymc_tests)
