#ifndef TOKENIZER_TOKENMATCHER_H
#define TOKENIZER_TOKENMATCHER_H

#include "tokenizer/token.h"

#include <string>
#include <unordered_map>
#include <memory>

class TokenMatcher
{
public:
	static void MatchTokens(std::vector<Token>& aWrite,const std::string& aLine);

	class Pattern;
	typedef std::unordered_map<std::string, std::shared_ptr<Pattern>> PatternCollection;

	class Pattern
	{
	public:
		virtual size_t Match(const std::string_view& aView, const PatternCollection& aPatterns) = 0;
	};


private:

	static std::string_view SplitView(std::string_view& aInOutLeft, size_t aAmount);

	class RootPattern
	{
	public:
		struct MatchResult
		{
			size_t myTokensConsumed;
			Token::Type mType;
		};

		RootPattern(std::string aBasePattern, Token::Type aType)
			: myBasePattern(aBasePattern)
			, myType(aType)
		{
		}

		MatchResult Match(const std::string_view& aView, const PatternCollection& aPatterns)
		{
			size_t size = aPatterns.at(myBasePattern)->Match(aView, aPatterns);
			return { size, myType };
		}

		std::string myBasePattern;
		Token::Type myType;
	};

	static void LoadPatterns();

	static std::vector<std::shared_ptr<RootPattern>> ourRootPatterns;
	static std::unordered_map<std::string, std::shared_ptr<Pattern>> ourPatterns;
};

#endif // !TOKENIZER_TOKENMATCHER_H