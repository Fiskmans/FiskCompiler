

list(APPEND SOURCE_FILES Types.h)
list(APPEND SOURCE_FILES Types.cpp)
list(APPEND SOURCE_FILES FileReader.h)
list(APPEND SOURCE_FILES FileReader.cpp)
list(APPEND SOURCE_FILES IncludeFinder.h)
list(APPEND SOURCE_FILES IncludeFinder.cpp)
list(APPEND SOURCE_FILES IncludeFilter.h)
list(APPEND SOURCE_FILES IncludeFilter.cpp)
list(APPEND SOURCE_FILES LineReader.h)
list(APPEND SOURCE_FILES LineReader.cpp)
list(APPEND SOURCE_FILES Precompiler.h)
list(APPEND SOURCE_FILES Precompiler.cpp)

add_library(precompiler "${SOURCE_FILES}")

target_link_libraries(precompiler PUBLIC pattern_matcher)

target_include_directories(precompiler PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(precompiler PUBLIC ${CMAKE_BINARY_DIR}/src)