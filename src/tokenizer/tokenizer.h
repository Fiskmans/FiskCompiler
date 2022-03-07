#ifndef TOKENIZER_TOKENIZER_H
#define TOKENIZER_TOKENIZER_H

#include <vector>
#include <string>
#include <string_view>

#include "tokenizer/token.h"

std::vector<Token> Tokenize(const std::string_view& aFilePath);

#endif