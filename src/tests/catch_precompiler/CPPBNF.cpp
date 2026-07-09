

#include <catch2/catch_all.hpp>

#include "precompiler/Types.h"

#include "pattern_matcher/PatternMatcher.h"
#include "pattern_matcher/PatternBuilder.h"

#include <fstream>
#include <sstream>

TEST_CASE("precompiler::bnf::cpp", "[iterator]")
{
    std::ifstream cppBNFFile("data/cpp.bnf.md");
    std::stringstream cppBNF;
    cppBNF << cppBNFFile.rdbuf();

    pattern_matcher::PatternMatcher matcher = pattern_matcher::PatternBuilder::FromBNF(cppBNF.str());
}