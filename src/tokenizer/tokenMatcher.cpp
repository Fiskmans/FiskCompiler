#include "tokenMatcher.h"

#include "common/CompilerContext.h"

std::vector<std::shared_ptr<TokenMatcher::RootPattern>> TokenMatcher::ourRootPatterns;
std::unordered_map<std::string, std::shared_ptr<TokenMatcher::Pattern>> TokenMatcher::ourPatterns;

void TokenMatcher::MatchTokens(std::vector<Token>& aWrite, const std::string& aLine)
{
	LoadPatterns();

	std::string_view lineLeft = aLine; 
	size_t aColumn = 0;

	while (!lineLeft.empty())
	{
		size_t toConsume = 0;
		Token::Type resultingType = Token::Type::Invalid;

		for (auto& pattern : ourRootPatterns)
		{
			auto [amount, type] = pattern->Match(lineLeft, ourPatterns);
			if(amount > toConsume)
			{
				toConsume = amount;
				resultingType = type;
			}
		}

		if (toConsume == 0)
		{
			CompilerContext::EmitError("Invalid token", aColumn);
			break;
		}

		aWrite.push_back(Token(resultingType, SplitView(lineLeft, toConsume)));
		aColumn += toConsume;
	}

	aWrite.push_back(Token(Token::Type::NewLine, "\n"));
}

std::string_view TokenMatcher::SplitView(std::string_view& aInOutLeft, size_t aAmount)
{
	std::string_view out(aInOutLeft.begin(), aInOutLeft.begin() + aAmount);
	aInOutLeft = std::string_view(aInOutLeft.begin() + aAmount, aInOutLeft.end());
	return out;
}

class CharPattern : public TokenMatcher::Pattern
{
public:
	CharPattern(char aChar)
		: myChar(aChar)
	{
	}

	virtual size_t Match(const std::string_view& aView, const TokenMatcher::PatternCollection& aPatterns) override
	{
		if (aView[0] == myChar)
			return 1;
	
		return 0;
	}
private:
	char myChar;
};

class WordPattern : public TokenMatcher::Pattern
{
	
public:
	WordPattern(const std::string_view& aWord)
		: myWord(aWord)
	{
	}

	virtual size_t Match(const std::string_view& aView, const TokenMatcher::PatternCollection& aPatterns) override
	{
		if (aView.starts_with(myWord))
			return myWord.length();
	
		return 0;
	}
private:
	std::string myWord;
};

class AnyOfPattern : public TokenMatcher::Pattern
{
	
public:
	AnyOfPattern(const std::string_view& aChars)
		: myChars(aChars)
	{
	}

	virtual size_t Match(const std::string_view& aView, const TokenMatcher::PatternCollection& aPatterns) override
	{
		if (myChars.find(aView[0]) != std::string::npos)
			return 1;
	
		return 0;
	}
private:
	std::string myChars;
};

class EitherPattern : public TokenMatcher::Pattern
{
public:
	EitherPattern(const std::initializer_list<std::string>& aList)
		: myOptions(aList)
	{
	}
	
	virtual size_t Match(const std::string_view& aView, const TokenMatcher::PatternCollection& aPatterns) override
	{
		size_t longest = 0;

		for (std::string& patternName : myOptions)
			longest = std::max(longest, aPatterns.at(patternName)->Match(aView, aPatterns));

		return longest;
	}

private:
	std::vector<std::string> myOptions;
};

class ComboPattern : public TokenMatcher::Pattern
{
public:
	ComboPattern(const std::initializer_list<std::string>& aList)
		: myList(aList)
	{
	}
	
	virtual size_t Match(const std::string_view& aView, const TokenMatcher::PatternCollection& aPatterns) override
	{
		size_t total = 0;

		for (std::string& patternName : myList)
		{
			const std::string_view next(aView.begin() + total, aView.end());
			size_t amount = aPatterns.at(patternName)->Match(next, aPatterns);

			if (amount == 0)
				return 0;

			total += amount;
		}

		return total;
	}

private:
	std::vector<std::string> myList;
};

class RepeatPattern : public TokenMatcher::Pattern
{
public:
	RepeatPattern(const std::string_view& aBase)
		: myBase(aBase)
	{
	}
	
	virtual size_t Match(const std::string_view& aView, const TokenMatcher::PatternCollection& aPatterns) override
	{
		size_t total = 0;
		std::string_view view(aView);
		while (!view.empty())
		{
			size_t amount = aPatterns.at(myBase)->Match(view, aPatterns);
			if (amount == 0)
				break;
			view = std::string_view(view.begin() + amount, view.end());
			total += amount;
		}
		return total;
	}
private:
	std::string myBase;
};

void TokenMatcher::LoadPatterns()
{
	if(!ourPatterns.empty())
		return;

	ourPatterns.insert(std::make_pair("l-paren",				new CharPattern('(')));
	ourPatterns.insert(std::make_pair("r-paren",				new CharPattern(')')));
	ourPatterns.insert(std::make_pair("l-brace",				new CharPattern('{')));
	ourPatterns.insert(std::make_pair("r-brace",				new CharPattern('}')));

	for (const char* keyword : {
		"alignas", "alignof", "asm", "auto", "bool", "break", "case", "catch", 
		"char", "char8_t", "char16_t", "char32_t", "class", "concept", "const", 
		"consteval","constexpr", "constinit", "const_cast", "continue", "co_await", 
		"co_return", "co_yield", "decltype", "default", "delete", "do", 
		"double", "dynamic_cast", "else", "enum", "explicit",
		"export", "extern", "false", "float", "for", "friend", "goto", "if",
		"inline", "int", "long", "mutable", "namespace", "new", "noexcept",
		"nullptr", "operator", "private", "protected", "public", "register",
		"reinterpret_cast", "requires", "return", "short", "signed", "sizeof",
		"static", "static_assert", "static_cast", "struct", "switch", "template",
		"this", "thread_local", "throw", "true", "try", "typedef", "typeid",
		"typename", "union", "unsigned", "using", "virtual", 
		"void", "volatile", "wchar_t", "while"})
		ourPatterns.insert(std::make_pair(keyword,				new WordPattern(keyword)));

	ourPatterns.insert(std::make_pair("identifier",				new ComboPattern({ "nondigit", "identifier:rep" })));
	ourPatterns.insert(std::make_pair("identifier:rep",			new RepeatPattern( "identifier:tail" )));
	ourPatterns.insert(std::make_pair("identifier:tail",		new EitherPattern({ "nondigit", "digit" })));

	ourPatterns.insert(std::make_pair("whitespace",				new RepeatPattern( "whitechar" )));

	ourPatterns.insert(std::make_pair("nondigit",				new AnyOfPattern("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_")));
	ourPatterns.insert(std::make_pair("digit",					new AnyOfPattern("0123456789")));
	ourPatterns.insert(std::make_pair("whitechar",				new AnyOfPattern(" \t\n\r\b")));



	ourRootPatterns.emplace_back(new RootPattern("l-paren",		Token::Type::L_Paren));
	ourRootPatterns.emplace_back(new RootPattern("r-paren",		Token::Type::R_Paren));
	ourRootPatterns.emplace_back(new RootPattern("l-brace",		Token::Type::L_Brace));
	ourRootPatterns.emplace_back(new RootPattern("r-brace",		Token::Type::R_Brace));

	ourRootPatterns.emplace_back(new RootPattern("int",			Token::Type::kw_int));

	ourRootPatterns.emplace_back(new RootPattern("identifier",	Token::Type::Identifier));
	ourRootPatterns.emplace_back(new RootPattern("whitespace",	Token::Type::WhiteSpace));
}
