
#ifndef TOOLS_FILEHELEPERS_H
#define TOOLS_FILEHELEPERS_H

#include <vector>
#include <string>
#include <filesystem>

std::vector<std::string> ReadWholeFile(const std::filesystem::path& aFilePath);

#endif