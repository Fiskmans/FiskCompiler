
#include "tokenizer/tokenStream.h"

#include "precompiler/Types.h"

#include "precompiler/LineReader.h"
#include "precompiler/LineJoiner.h"

#include "pattern_matcher/PatternMatcher.h"
#include "pattern_matcher/PatternBuilder.h"

namespace fisk::precompiler
{
	

	inline auto Precompile(std::string aFile)
	{
        std::ifstream cppBNFFile("data/cpp.bnf");
		std::stringstream cppBNF;
        cppBNF << cppBNFFile.rdbuf();

		pattern_matcher::PatternMatcher matcher = pattern_matcher::PatternBuilder::FromBNF(cppBNF.str());

		return SentinelRange(
			ReIterator(
				LineJoiner(
				std::make_shared<LineReader>(
					std::make_shared<std::ifstream>(aFile), 
					aFile))
				));
	}
}
