
#include <catch2/catch_all.hpp>

#include "precompiler/FileReader.h"

void CompareFile(std::string aFile, std::vector<std::string> aLines)
{
	fisk::precompiler::FileReader reader(aFile);

	int line = 1;
	for (std::string& text : aLines)
	{
		REQUIRE((reader != nullptr));

		REQUIRE((*reader).myFile == aFile);
		REQUIRE((*reader).myLine == line);
		REQUIRE((*reader).myText == text);

		++line;
		++reader;
	}
	REQUIRE((reader == nullptr));
}

TEST_CASE("file_reader::basic", "")
{
	CompareFile("test/precompiler/file_reader/1_basic.txt", { "int main(){}" });
	CompareFile("test/precompiler/file_reader/2_multiline.txt", { "int main()", "{", "}" });
}