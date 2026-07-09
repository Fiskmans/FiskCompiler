
#include <catch2/catch_all.hpp>

#include "precompiler/LineReader.h"

TEST_CASE("precompiler::line_reader::basic", "")
{
	fisk::precompiler::LineReader reader(std::make_unique<std::stringstream>("Hello"), "test.txt");

	REQUIRE((reader != nullptr));

	fisk::precompiler::SourceLine line = *reader;
	REQUIRE((*reader).myText == "Hello");
	REQUIRE((*reader).myPath == "test.txt");
	REQUIRE((*reader).myLine == 1);
	
	++reader;

	REQUIRE((reader == nullptr));
}

TEST_CASE("precompiler::line_reader::multiline_1", "")
{
	fisk::precompiler::LineReader reader(std::make_unique<std::stringstream>("Hello\nThere"), "test.txt");

	REQUIRE((reader != nullptr));
	REQUIRE((*reader).myText == "Hello\n");
	REQUIRE((*reader).myPath == "test.txt");
	REQUIRE((*reader).myLine == 1);

	++reader;

	REQUIRE((reader != nullptr));

	REQUIRE((reader != nullptr));
	REQUIRE((*reader).myText == "There");
	REQUIRE((*reader).myPath == "test.txt");
	REQUIRE((*reader).myLine == 2);

	++reader;

	REQUIRE((reader == nullptr));
}

TEST_CASE("precompiler::line_reader::multiline_2", "")
{
	fisk::precompiler::LineReader reader(std::make_unique<std::stringstream>("Hello\\\nThere\nGeneral"), "test.txt");

	REQUIRE((reader != nullptr));
	REQUIRE((*reader).myText == "Hello\\\n");
	REQUIRE((*reader).myPath == "test.txt");
	REQUIRE((*reader).myLine == 1);

	++reader;

	REQUIRE((reader != nullptr));
	REQUIRE((*reader).myText == "There\n");
	REQUIRE((*reader).myPath == "test.txt");
	REQUIRE((*reader).myLine == 2);

	++reader;

	REQUIRE((reader != nullptr));
	REQUIRE((*reader).myText == "General");
	REQUIRE((*reader).myPath == "test.txt");
	REQUIRE((*reader).myLine == 3);

	++reader;

	REQUIRE((reader == nullptr));
}