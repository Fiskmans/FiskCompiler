#ifndef TOKENIZER_TOKENSTREAM_H
#define TOKENIZER_TOKENSTREAM_H

#include <vector>

#include "tokenizer/token.h"

namespace tokenizer
{
	class TokenStream
	{
	public:
		std::vector<Token>& Get() &&;

		TokenStream&	operator<<(const Token& aToken);
		TokenStream&	operator<<(const std::vector<Token>& aTokens);

	private:
		std::vector<Token> myTokens;
	};
}

#endif // TOKENIZER_TOKENSTREAM_H