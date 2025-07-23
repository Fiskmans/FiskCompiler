cmake_minimum_required(VERSION 3.22)

project(fiskCompiler VERSION 0.0.0)

option(FISK_BUILD_TESTING "Build test executables" ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(BUILD_SHARED_LIBS OFF)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1
)

FetchContent_Declare(
  pattern_matcher
  GIT_REPOSITORY https://github.com/Fiskmans/PatternMatcher.git
  GIT_TAG        cb3198bb39ba2555ee2b34ba6ef4987dd315beec
)

FetchContent_MakeAvailable(pattern_matcher)

add_subdirectory(src)