#include "fileHelpers.h"

#include <fstream>

#include "common/CompilerContext.h"

std::vector<std::string> ReadWholeFile(const std::filesystem::path& aFilePath)
{
	std::vector<std::string> out;
	std::ifstream file(aFilePath);
	std::string line;
	if(!file)
		CompilerContext::EmitError("Failed to open file", aFilePath);
	
	while (std::getline(file, line))
		out.push_back(line);

	return out;
}