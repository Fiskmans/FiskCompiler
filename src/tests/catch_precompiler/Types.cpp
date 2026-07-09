
#include <catch2/catch_all.hpp>

#include "precompiler/LineReader.h"
#include "precompiler/Types.h"

TEST_CASE("precompiler::types::reiterator", "")
{
    fisk::precompiler::SourceLine lines[] = 
    {
        {"test.txt", "Hello\n", 1 },
        {"test.txt", "there\n", 2 },
        {"test.txt", "general\n", 3 },
        {"test.txt", "kenobi", 4 }
    };

    fisk::precompiler::ReIterator reiterator(fisk::precompiler::LineReader(std::make_shared<std::stringstream>("Hello\nthere\ngeneral\nkenobi"), "test.txt"));

    fisk::precompiler::ReIterator secondPass(reiterator);
    fisk::precompiler::ReIterator thirdPass(reiterator);

    REQUIRE(reiterator != nullptr);
    REQUIRE((*reiterator) == lines[0]);
    REQUIRE((*secondPass) == lines[0]);
    REQUIRE((*thirdPass) == lines[0]);

    ++reiterator;
    REQUIRE(reiterator != nullptr);
    REQUIRE((*reiterator) == lines[1]);
    REQUIRE((*secondPass) == lines[0]);
    REQUIRE((*thirdPass) == lines[0]);
    
    REQUIRE(reiterator != secondPass);
    REQUIRE(secondPass == thirdPass);
    
    ++secondPass;
    REQUIRE(reiterator == secondPass);
    REQUIRE(secondPass != thirdPass);
    REQUIRE((*secondPass) == lines[1]);
    
    ++reiterator;
    REQUIRE(reiterator != nullptr);
    ++secondPass;
    ++thirdPass;
    ++thirdPass;
    REQUIRE((*reiterator) == lines[2]);
    REQUIRE((*secondPass) == lines[2]);
    REQUIRE((*thirdPass) == lines[2]);
    
    ++reiterator;
    REQUIRE(reiterator != nullptr);
    ++secondPass;
    
    REQUIRE((*reiterator) == lines[3]);
    REQUIRE((*secondPass) == lines[3]);
    REQUIRE((*thirdPass) == lines[2]);
    
    ++reiterator;
    REQUIRE(reiterator == nullptr);
    REQUIRE((*secondPass) == lines[3]);
    REQUIRE((*thirdPass) == lines[2]);
    ++secondPass;
    REQUIRE(reiterator == secondPass);
    REQUIRE((*thirdPass) == lines[2]);
}