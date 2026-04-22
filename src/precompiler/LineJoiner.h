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