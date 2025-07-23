

#include <catch2/catch_all.hpp>

#include "precompiler/Types.h"

TEST_CASE("precompiler::types::unpacking_iterator", "[iterator]")
{
	std::vector<std::vector<int>> range =
	{
		{1,2,3},
		{4},
		{},
		{5,6},
		{}
	};

	fisk::precompiler::UnpackingIterator unpacker = std::ranges::begin(range);

	auto end = std::ranges::end(range);

	REQUIRE((unpacker != end));
	REQUIRE(*unpacker == 1);

	for (size_t i = 2; i <= 6; i++)
	{
		++unpacker;
		REQUIRE((unpacker != end));
		REQUIRE(*unpacker == i);
	}

	++unpacker;
	REQUIRE((unpacker == end));
}