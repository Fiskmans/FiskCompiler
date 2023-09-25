#include "tokenizer/tokenStream.h"

namespace tokenizer
{
	std::vector<Token>& TokenStream::Get()&&
	{
		return myTokens;
	}

	TokenStream& TokenStream::operator<<(const Token& aToken)
	{
		myTokens.push_back(aToken);
		return *this;
	}

	TokenStream& TokenStream::operator<<(const std::vector<Token>& aTokens)
	{
		//myTokens.reserve(myTokens.size() + aTokens.size());
		myTokens.insert(std::end(myTokens), std::begin(aTokens), std::end(aTokens));
		return *this;
	}
} // tokenizer
