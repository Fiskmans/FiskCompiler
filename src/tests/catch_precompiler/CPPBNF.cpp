

#include <catch2/catch_all.hpp>

#include "precompiler/Types.h"

TEST_CASE("precompiler::bnf::cpp", "[iterator]")
{
    std::ifstream cppBNFFile("data/cpp.bnf.md");
    std::stringstream cppBNF;
    cppBNF << cppBNFFile.rdbuf();

    pattern_matcher::PatternMatcher matcher = pattern_matcher::PatternBuilder::FromBNF(cppBNF.str());
}