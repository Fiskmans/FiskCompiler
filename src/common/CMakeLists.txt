
list(APPEND SOURCE_FILES CompilerContext.cpp)
list(APPEND SOURCE_FILES CompilerContext.h)
list(APPEND SOURCE_FILES HelpPrinter.cpp)
list(APPEND SOURCE_FILES HelpPrinter.h)
list(APPEND SOURCE_FILES FeatureSwitch.cpp)
list(APPEND SOURCE_FILES FeatureSwitch.h)
list(APPEND SOURCE_FILES IteratorRange.h)

add_library(common "${SOURCE_FILES}")

target_include_directories(common PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(common PUBLIC ${CMAKE_BINARY_DIR}/src)