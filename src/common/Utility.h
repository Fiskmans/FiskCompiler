
#ifndef COMMON_UTILITY_H
#define COMMON_UTILITY_H

#include <ranges>

namespace common {


	template<std::ranges::viewable_range RangeType>
		requires std::convertible_to<std::ranges::range_reference_t<RangeType>, std::string>
	std::string Join(RangeType&& aValues, std::string aDelimiter = ", ")
	{
		auto at = std::ranges::begin(aValues);
		auto end = std::ranges::end(aValues);

		if (at == end)
			return "";

		std::string acc = *at;
		at++;

		while (at != end)
			acc = std::move(acc) + aDelimiter + *at;

		return acc;
	}
}

#endif