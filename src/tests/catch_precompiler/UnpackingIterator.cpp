

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

namespace static_test
{
	using Iter = std::vector<int>*;
	static_assert(std::input_iterator<Iter>);

	using ValueType = std::iterator_traits<Iter>::value_type;
	static_assert(std::is_same_v<std::vector<int>, ValueType>);

	using InnerType = std::ranges::range_value_t<ValueType>;
	static_assert(std::is_same_v<int, InnerType>);

	using Iter2 = fisk::precompiler::UnpackingIterator<Iter>;
    static_assert(std::is_same_v<ValueType, Iter2::inner_range_t>);
    static_assert(std::is_same_v<InnerType, Iter2::value_type>);
    static_assert(std::is_same_v<InnerType, std::iterator_traits<Iter2>::value_type>);


	static_assert(std::input_iterator<Iter2>);
}