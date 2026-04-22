#pragma once

#include "LineReader.h"
#include "IncludeFinder.h"
#include "Types.h"

#include <vector>
#include <string>
#include <memory>
#include <stack>

namespace fisk::precompiler
{

	class LineJoiner
	{
	public:
		LineJoiner(std::shared_ptr<LineReader> aFile);

		using Lines = std::vector<SourceLine>;
		using Range = SimpleRangeWrapper<UnpackingIterator<Lines::iterator>, Lines::iterator>;

		using iterator_category = std::iterator_traits<LineReader>::iterator_category;
        using difference_type = std::iterator_traits<LineReader>::difference_type;

        using value_type = Range;
        using pointer = value_type *;
        using reference = value_type &;

		Range operator*();
		LineJoiner& operator++();

		bool operator==(std::nullptr_t aOther);
		bool operator!=(std::nullptr_t aOther);

	private:

		void Next();

		std::shared_ptr<LineReader> myFile;
		Lines myLines;
	};
}