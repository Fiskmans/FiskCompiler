#pragma once

#include "FileReader.h"
#include "IncludeFinder.h"
#include "Types.h"

#include "pattern_matcher/PatternMatcher.h"


#include <string>
#include <memory>
#include <stack>

namespace fisk::precompiler
{

	class LineJoiner
	{
	public:
		LineJoiner(std::unique_ptr<FileReader> aFile);

		using Lines = std::vector<SourceLine>;
		using Range = SimpleRangeWrapper<UnpackingIterator<Lines::iterator>, Lines::iterator>;

		Range operator*();
		LineJoiner& operator++();

		bool operator==(std::nullptr_t aOther);
		bool operator!=(std::nullptr_t aOther);

	private:

		void Next();

		std::unique_ptr<FileReader> myFile;
		Lines myLines;
	};
}