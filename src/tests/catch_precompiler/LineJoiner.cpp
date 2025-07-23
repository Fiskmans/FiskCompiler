
#include <catch2/catch_all.hpp>

#include "precompiler/LineJoiner.h"

void RequireSame(fisk::precompiler::SourceChar aLeft, fisk::precompiler::SourceChar aRight)
{
	REQUIRE(aLeft.myCharacter == aRight.myCharacter);
	REQUIRE(aLeft.myColumn == aRight.myColumn);
	REQUIRE(aLeft.myFilePath == aRight.myFilePath);
	REQUIRE(aLeft.myLine == aRight.myLine);
}

void Test(std::string aFile, std::vector<std::vector<fisk::precompiler::SourceLine>> aExpected)
{
	fisk::precompiler::LineJoiner joiner(std::make_unique<fisk::precompiler::FileReader>(aFile));

	auto at = std::begin(aExpected);

	REQUIRE((joiner != nullptr));
	REQUIRE((at != std::end(aExpected)));

	while (true)
	{
		auto left = *joiner;
		auto right = *at;

		auto l = std::begin(left);
		fisk::precompiler::UnpackingIterator r = std::begin(right);

		while (true)
		{
			if (l == std::end(left))
				break;

			if (r == std::end(right))
				break;

			RequireSame(*l, *r);
			++l;
			++r;
		}

		REQUIRE((l == std::end(left)));
		REQUIRE((r == std::end(right)));

		++joiner;
		++at;

		if (joiner == nullptr)
			break;

		if (at == std::end(aExpected))
			break;
	}

	REQUIRE((joiner == nullptr));
	REQUIRE((at == std::end(aExpected)));
}

TEST_CASE("precompiler::file_joiner::basic", "")
{
	Test("test/precompiler/line_joiner/1_basic.txt", { 
			{
				{ 
					"test/precompiler/line_joiner/1_basic.txt",
					"Hello ",
					1
				}
			},
			{
				{
					"test/precompiler/line_joiner/1_basic.txt",
					"there",
					2
				}
			}
		});
}

TEST_CASE("precompiler::file_joiner::joined", "")
{
	Test("test/precompiler/line_joiner/2_joined.txt", {
			{
				{
					"test/precompiler/line_joiner/2_joined.txt",
					"Hello ",
					1
				},
				{
					"test/precompiler/line_joiner/2_joined.txt",
					"there",
					2
				}
			}
		 });
}

TEST_CASE("precompiler::file_joiner::empty", "")
{
	Test("test/precompiler/line_joiner/3_empty.txt", {
			{
				{
					"test/precompiler/line_joiner/3_empty.txt",
					"Hello",
					1
				}
			},
			{},
			{
				{
					"test/precompiler/line_joiner/3_empty.txt",
					"there",
					3
				}
			}
		 });
	Test("test/precompiler/line_joiner/3_empty_2.txt", {
			{
				{
					"test/precompiler/line_joiner/3_empty_2.txt",
					"Hello",
					1
				}
			},
			{
				{
					"test/precompiler/line_joiner/3_empty_2.txt",
					"there",
					3
				}
			}
		 });
	Test("test/precompiler/line_joiner/3_empty_3.txt", {
			{
				{
					"test/precompiler/line_joiner/3_empty_3.txt",
					"Hello",
					1
				}
			},
			{
				{
					"test/precompiler/line_joiner/3_empty_3.txt",
					"there",
					3
				}
			}
		 });
}