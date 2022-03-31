#ifndef TOKENIZER_TOKENMATCHER_H
#define TOKENIZER_TOKENMATCHER_H

#include "tokenizer/token.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <optional>
#include <concepts>

class TokenMatcher
{
public:
	struct Context
	{
		Context() = default;
	private:
		friend TokenMatcher;
		
		bool myCurrentTokenIsPotentiallyMultiLine = false;

		Token::Type tokenType = Token::Type::Invalid;
		std::string tokenBuffer;
		std::string endSequence;
		size_t column;
	};


	static void MatchTokens(std::vector<Token>& aWrite,const std::string& aLine, Context& aContext);

	class Pattern;
	typedef std::unordered_map<std::string, std::shared_ptr<Pattern>> PatternCollection;

	class Pattern
	{
	public:
		
		virtual std::optional<size_t> Match(const std::string_view& aView) = 0;
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

		RootPattern(std::shared_ptr<Pattern> aBasePattern, Token::Type aType)
			: myBasePattern(aBasePattern)
			, myType(aType)
		{
		}

		MatchResult Match(const std::string_view& aView)
		{
			std::optional<size_t> res = myBasePattern->Match(aView);
			return { res ? *res : 0, res ? myType : Token::Type::Invalid };
		}

		std::shared_ptr<Pattern> myBasePattern;
		Token::Type myType;
	};


	class PatternBuilder
	{
	public:
		PatternBuilder(Token::Type aType, std::vector<std::shared_ptr<RootPattern>>& aRootPatternCollection);

		~PatternBuilder();

		PatternBuilder& operator=(std::shared_ptr<Pattern> aPattern);
		PatternBuilder& operator and(std::shared_ptr<Pattern> aPattern);
	
	private:
		std::vector<std::shared_ptr<Pattern>> myPatterns;

		Token::Type myType;
		std::vector<std::shared_ptr<RootPattern>>& myRootPatternCollecton;
	};


	static void LoadPatterns();
	static PatternBuilder BuildPattern(Token::Type aType)
	{
		return PatternBuilder(aType, ourRootPatterns);
	};

	static std::vector<std::shared_ptr<RootPattern>> ourRootPatterns;
};

#endif // !TOKENIZER_TOKENMATCHER_H