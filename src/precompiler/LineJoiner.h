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
		using Range = SimpleRange<UnpackingIterator<Lines::iterator>, Lines::iterator>;

		Range operator*();
		LineJoiner& operator++();

		bool operator==(nullptr_t aOther);
		bool operator!=(nullptr_t aOther);

	private:

		void Next();

		std::unique_ptr<FileReader> myFile;
		Lines myLines;
	};
}