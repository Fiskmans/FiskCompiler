
#include "precompiler/Precompiler.h"

#include "pattern_matcher/PatternBuilder.h"
#include "pattern_matcher/PatternMatcher.h"

namespace fisk::precompiler
{
    PrecompilationOutput Precompile(std::string aFile)
    {
        std::ifstream cppBNFFile("data/cpp.bnf");
        std::stringstream cppBNF;
        cppBNF << cppBNFFile.rdbuf();

        pattern_matcher::PatternMatcher matcher = pattern_matcher::PatternBuilder::FromBNF(cppBNF.str());

        return 
            SentinelRange(
                ReIterator(
                    LineJoiner(std::make_shared<LineReader>(std::make_shared<std::ifstream>(aFile), aFile))));
    }
} // namespace fisk::precompiler