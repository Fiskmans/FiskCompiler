
add_subdirectory(tokenizer)
add_subdirectory(tools)
add_subdirectory(executable)
add_subdirectory(common)
add_subdirectory(precompiler)
add_subdirectory(markup)

if(FISK_BUILD_TESTING)

	FetchContent_MakeAvailable(Catch2)

	add_subdirectory(tests)
endif()