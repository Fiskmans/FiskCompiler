
list(APPEND SOURCE_FILES concepts)
list(APPEND SOURCE_FILES filesystem)
list(APPEND SOURCE_FILES fstream)
list(APPEND SOURCE_FILES iomanip)
list(APPEND SOURCE_FILES iostream)
list(APPEND SOURCE_FILES memory)
list(APPEND SOURCE_FILES optional)
list(APPEND SOURCE_FILES stack)
list(APPEND SOURCE_FILES string)
list(APPEND SOURCE_FILES string_view)
list(APPEND SOURCE_FILES unordered_map)
list(APPEND SOURCE_FILES vector)

add_library(std "${SOURCE_FILES}")

target_include_directories(std PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(std PUBLIC ${CMAKE_BINARY_DIR}/src)