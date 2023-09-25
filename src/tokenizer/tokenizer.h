#ifndef TOKENIZER_TOKENIZER_H
#define TOKENIZER_TOKENIZER_H

#include <vector>
#include <string>
#include <filesystem>

#include "tokenizer/token.h"

namespace tokenizer
{
	std::vector<Token> Tokenize(const std::filesystem::path& aFilePath);
}

#endif