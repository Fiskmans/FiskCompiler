#ifndef ITERATOR_RANGE
#define ITERATOR_RANGE

#include <concepts>
#include <iterator>
#include <ranges>

template<std::input_iterator IteratorType>
class IteratorRange
{
public:
	IteratorRange(const IteratorType& aBegin, const IteratorType& aEnd)
		: myBegin(aBegin), myEnd(aEnd)
	{

	}

	IteratorType begin() { return myBegin; }
	IteratorType end() { return myEnd; }

private:
	IteratorType myBegin;
	IteratorType myEnd;
};


static_assert(std::ranges::forward_range<IteratorRange<int*>>);
#endif //ITERATOR_RANGE