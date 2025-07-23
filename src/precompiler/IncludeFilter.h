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

	class IncludeFilter
	{
	public:
		IncludeFilter(std::unique_ptr<FileReader> aFile, IIncludeFinder& aIncludeFinder);

		std::vector<SourceLine>& operator*();
		IncludeFilter& operator++();

		bool operator==(nullptr_t aOther);
		bool operator!=(nullptr_t aOther);

	private:
		void SetupPatternMatcher();
		void Next();

		enum class State
		{
			UnPrimed,
			Primed,
			EndOfFile
		};


		PatternMatcher<> myMatcher;
		IIncludeFinder& myIncludeFinder;

		State myState;
		std::stack<std::unique_ptr<FileReader>> myFiles;
		std::vector<SourceLine> myLines;
	};
}