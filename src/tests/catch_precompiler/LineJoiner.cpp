
#include <catch2/catch_all.hpp>

#include "precompiler/LineJoiner.h"

void RequireSame(fisk::precompiler::SourceChar aLeft, fisk::precompiler::SourceChar aRight)
{
	REQUIRE(aLeft.myCharacter == aRight.myCharacter);
	REQUIRE(aLeft.myColumn == aRight.myColumn);
	REQUIRE(aLeft.myFilePath == aRight.myFilePath);
	REQUIRE(aLeft.myLine == aRight.myLine);
}

void Test(std::string aContent, std::string aFile, std::vector<std::vector<fisk::precompiler::SourceLine>> aExpected)
{
	fisk::precompiler::LineJoiner joiner(std::make_shared<fisk::precompiler::LineReader>(std::make_shared<std::stringstream>(aContent), aFile));

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

		REQUIRE(l == std::end(left));
		REQUIRE(r == std::end(right));

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

TEST_CASE("precompiler::line_joiner::basic", "")
{
	Test("Hello \nthere", 
		"1_basic.txt", 
		{ 
			{
				{ 
					"1_basic.txt",
					"Hello \n",
					1
				}
			},
			{
				{
					"1_basic.txt",
					"there",
					2
				}
			}
		});
}

TEST_CASE("precompiler::line_joiner::joined", "")
{
	Test("Hello \\\nthere", 
		"2_joined.txt",
		{
			{
				{
					"2_joined.txt",
					"Hello ",
					1
				},
				{
					"2_joined.txt",
					"there",
					2
				}
			}
		});
}

TEST_CASE("precompiler::line_joiner::empty", "")
{
	Test("Hello\n\nthere",
		"3_empty.txt", 
		{
			{
				{
					"3_empty.txt",
					"Hello\n",
					1
				}
			},
			{
				{
					"3_empty.txt",
					"\n",
					2
				}
			},
			{
				{
					"3_empty.txt",
					"there",
					3
				}
			}
		});
	Test(
		"Hello\n\\\nthere",
		"3_empty_2.txt", 
		{
			{
				{
					"3_empty_2.txt",
					"Hello\n",
					1
				}
			},
			{
				{
					"3_empty_2.txt",
					"there",
					3
				}
			}
		 });
	Test(
		"Hello\\\n\\\nthere",
		"3_empty_3.txt", 
		{
			{
				{
					"3_empty_3.txt",
					"Hello",
					1
				},
				{
					"3_empty_3.txt",
					"there",
					3
				}
			}
		 });
}

TEST_CASE("precompiler::line_joiner::whitespace", "")
{
	Test("Hello \\ \t\b\r\v\n\nthere",
		"3_empty.txt", 
		{
			{
				{
					"3_empty.txt",
					"Hello ",
					1
				},
				{
					"3_empty.txt",
					"\n",
					2
				}
			},
			{
				{
					"3_empty.txt",
					"there",
					3
				}
			}
		});
}