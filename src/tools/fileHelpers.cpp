#include "fileHelpers.h"

#include <fstream>

std::vector<std::string> ReadWholeFile(const std::string& aFilePath)
{
	std::vector<std::string> out;

	std::ifstream file(aFilePath);
	std::string line;
	
	while (std::getline(file, line))
		out.push_back(line);

	return std::vector<std::string>();
}