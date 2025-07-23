
#include <catch2/catch_all.hpp>

#include "precompiler/Types.h"

TEST_CASE("precompiler::types::source_line", "[source]")
{
	{
		fisk::precompiler::SourceLine line
		{
			"file.txt",
			"Hello there",
			0
		};

		size_t column = 0;

		for (fisk::precompiler::SourceChar c : line)
		{
			REQUIRE(c.myFilePath == "file.txt");
			REQUIRE(c.myLine == 0);

			REQUIRE(c.myCharacter == "Hello there"[column]);
			REQUIRE(c.myColumn == column);

			column++;
		}
	}

	{
		fisk::precompiler::SourceLine line
		{
			"file2.txt",
			"Hello there",
			2
		};

		size_t column = 0;

		for (fisk::precompiler::SourceChar c : line)
		{
			REQUIRE(c.myFilePath == "file2.txt");
			REQUIRE(c.myLine == 2);

			REQUIRE(c.myCharacter == "Hello there"[column]);
			REQUIRE(c.myColumn == column);

			column++;
		}
	}
}