
#include <catch2/catch_all.hpp>

#include "precompiler/LineReader.h"

TEST_CASE("line_reader::basic", "")
{
	std::stringstream ss("Hello");

	fisk::precompiler::LineReader reader(ss);

	REQUIRE((reader != nullptr));
	REQUIRE(*reader == "Hello");
	
	++reader;

	REQUIRE((reader == nullptr));
}

TEST_CASE("line_reader::multiline_1", "")
{
	std::stringstream ss("Hello\nThere");

	fisk::precompiler::LineReader reader(ss);

	REQUIRE((reader != nullptr));
	REQUIRE(*reader == "Hello");

	++reader;

	REQUIRE((reader != nullptr));

	REQUIRE((reader != nullptr));
	REQUIRE(*reader == "There");

	++reader;

	REQUIRE((reader == nullptr));
}

TEST_CASE("line_reader::multiline_2", "")
{
	std::stringstream ss("Hello\\\nThere\nGeneral");

	fisk::precompiler::LineReader reader(ss);

	REQUIRE((reader != nullptr));
	REQUIRE(*reader == "Hello\\");

	++reader;

	REQUIRE((reader != nullptr));
	REQUIRE(*reader == "There");

	++reader;

	REQUIRE((reader != nullptr));
	REQUIRE(*reader == "General");

	++reader;

	REQUIRE((reader == nullptr));
}