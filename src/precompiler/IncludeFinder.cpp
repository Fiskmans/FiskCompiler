#include "IncludeFinder.h"

#include <fstream>

std::unique_ptr<std::istream> FileInclude::FindInclude(std::string aFrom, std::string aWants)
{
    return std::make_unique<std::ifstream>(aWants);
}
