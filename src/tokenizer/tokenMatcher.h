#ifndef TOKENIZER_TOKENMATCHER_H
#define TOKENIZER_TOKENMATCHER_H

#include "tokenizer/token.h"

#include <string>
#include <vector>
#include <memory>

class TokenMatcher
{
public:
	static void MatchTokens(std::vector<Token>& aWrite,const std::string& aLine);
private:
	class Pattern
	{
		size_t Consume(const char* aStart);
	};

	static void LoadPatterns();

	static std::vector<std::shared_ptr<Pattern>> outPatterns;
};

#endif // !TOKENIZER_TOKENMATCHER_H