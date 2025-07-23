
list(APPEND Files FileReader.cpp)
list(APPEND Files LineReader.cpp)
list(APPEND Files UnpackingIterator.cpp)
list(APPEND Files SourceLine.cpp)
list(APPEND Files LineJoiner.cpp)

add_executable(catch_precompiler ${Files})

target_link_libraries(catch_precompiler PUBLIC precompiler)
target_link_libraries(catch_precompiler PRIVATE Catch2::Catch2WithMain)