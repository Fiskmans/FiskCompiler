#include "tokenMatcher.h"

#include "common/CompilerContext.h"

std::vector<std::shared_ptr<TokenMatcher::RootPattern>> TokenMatcher::ourRootPatterns;
std::unordered_map<std::string, std::shared_ptr<TokenMatcher::Pattern>> TokenMatcher::ourPatterns;

void TokenMatcher::MatchTokens(std::vector<Token>& aWrite, const std::string& aLine)
{
	LoadPatterns();

	std::string_view lineLeft = aLine; 
	size_t aColumn = 0;

	bool trimWhitespace = CompilerContext::HasFlag("p:no_whitespace");

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

		std::string_view rawToken = SplitView(lineLeft, toConsume);
		aColumn += toConsume;

		if (trimWhitespace && resultingType == Token::Type::WhiteSpace)
			continue;

		aWrite.push_back(Token(resultingType, rawToken));
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

	virtual std::optional<size_t> Match(const std::string_view& aView, const TokenMatcher::PatternCollection& aPatterns) override
	{
		if (aView[0] == myChar)
			return 1;
	
		return {};
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

	virtual std::optional<size_t> Match(const std::string_view& aView, const TokenMatcher::PatternCollection& aPatterns) override
	{
		if (aView.starts_with(myWord))
			return myWord.length();
	
		return {};
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

	virtual std::optional<size_t> Match(const std::string_view& aView, const TokenMatcher::PatternCollection& aPatterns) override
	{
		if (myChars.find(aView[0]) != std::string::npos)
			return 1;
	
		return {};
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
	
	virtual std::optional<size_t> Match(const std::string_view& aView, const TokenMatcher::PatternCollection& aPatterns) override
	{
		size_t longest = 0;

		for (std::string& patternName : myOptions)
		{
			std::optional<size_t> res = aPatterns.at(patternName)->Match(aView, aPatterns);
			if (res)
				longest = std::max(longest, *res);
		}

		return longest > 0 ? longest : std::optional<size_t>();
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
	
	virtual std::optional<size_t> Match(const std::string_view& aView, const TokenMatcher::PatternCollection& aPatterns) override
	{
		size_t total = 0;

		for (std::string& patternName : myList)
		{
			const std::string_view next(aView.begin() + total, aView.end());
			std::optional<size_t> res = aPatterns.at(patternName)->Match(next, aPatterns);

			if (!res)
				return {};

			total += *res;
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
	
	virtual std::optional<size_t> Match(const std::string_view& aView, const TokenMatcher::PatternCollection& aPatterns) override
	{
		size_t total = 0;
		std::string_view view(aView);
		while (!view.empty())
		{
			std::optional<size_t> res = aPatterns.at(myBase)->Match(view, aPatterns);
			if (!res)
				break;
			view = std::string_view(view.begin() + *res, view.end());
			total += *res;
		}
		return total > 0 ? total : std::optional<size_t>();
	}
private:
	std::string myBase;
};

class OptionalPattern : public TokenMatcher::Pattern
{
public:
	OptionalPattern(std::shared_ptr<TokenMatcher::Pattern> aBase)
		: myBase(aBase)
	{
	}
	
	virtual std::optional<size_t> Match(const std::string_view& aView, const TokenMatcher::PatternCollection& aPatterns) override
	{
		std::optional<size_t> res = myBase->Match(aView, aPatterns);
		return res ? res : 0;
	}
private:
	std::shared_ptr<TokenMatcher::Pattern> myBase;
};

void TokenMatcher::LoadPatterns()
{
	if(!ourPatterns.empty())
		return;

	ourPatterns.insert(std::make_pair("semicolon",				new CharPattern(';')));
	ourPatterns.insert(std::make_pair("comma",					new CharPattern(',')));
	
	ourPatterns.insert(std::make_pair("star",					new CharPattern('*')));

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
		ourPatterns.insert(std::make_pair(keyword,							new WordPattern(keyword)));

	ourPatterns.insert(std::make_pair("identifier",							new ComboPattern({ "nondigit", "identifier:rep" })));
	ourPatterns.insert(std::make_pair("identifier:rep",						new RepeatPattern( "identifier:tail" )));
	ourPatterns.insert(std::make_pair("identifier:tail",					new EitherPattern({ "nondigit", "digit" })));

	ourPatterns.insert(std::make_pair("whitespace",							new RepeatPattern( "whitechar" )));
	ourPatterns.insert(std::make_pair("whitechar",							new AnyOfPattern(" \r\b")));

	ourPatterns.insert(std::make_pair("nondigit",							new AnyOfPattern("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_")));
	ourPatterns.insert(std::make_pair("digit",								new AnyOfPattern("0123456789")));
	ourPatterns.insert(std::make_pair("nonzero-digit",						new AnyOfPattern("123456789")));
	
	ourPatterns.insert(std::make_pair("integer-literal",					new EitherPattern( { "binary-literal", "octal-literal", "decimal-literal", "hexadecimal-literal" } )));

	ourPatterns.insert(std::make_pair("binary-literal",						new ComboPattern( { "binary-literal-prefix", "binary-digit-sequence" } )));
	ourPatterns.insert(std::make_pair("binary-literal-prefix",				new EitherPattern( { "binary-literal-prefix:lower", "binary-literal-prefix:upper" })));
	ourPatterns.insert(std::make_pair("binary-literal-prefix:lower",		new WordPattern("0b")));
	ourPatterns.insert(std::make_pair("binary-literal-prefix:upper",		new WordPattern("0B")));
	ourPatterns.insert(std::make_pair("binary-digit-sequence",				new RepeatPattern("binary-digit:segment")));
	ourPatterns.insert(std::make_pair("binary-digit:segment",				new ComboPattern( { "integer-literal:optional-tick", "binary-digit" } )));
	ourPatterns.insert(std::make_pair("binary-digit",						new AnyOfPattern( "01" )));
	
	ourPatterns.insert(std::make_pair("octal-literal",						new ComboPattern( { "octal-literal-prefix", "octal-digit-sequence" } )));
	ourPatterns.insert(std::make_pair("octal-literal-prefix",				new WordPattern("0")));
	ourPatterns.insert(std::make_pair("octal-digit-sequence",				new OptionalPattern(std::shared_ptr<Pattern>(new RepeatPattern("octal-digit:segment")))));
	ourPatterns.insert(std::make_pair("octal-digit:segment",				new ComboPattern( { "integer-literal:optional-tick", "octal-digit" } )));
	ourPatterns.insert(std::make_pair("octal-digit",						new AnyOfPattern( "01234567" )));
	
	ourPatterns.insert(std::make_pair("decimal-literal",					new ComboPattern( { "nonzero-digit", "decimal-digit-sequence" } )));
	ourPatterns.insert(std::make_pair("decimal-digit-sequence",				new OptionalPattern(std::shared_ptr<Pattern>(new RepeatPattern("decimal-digit:segment")))));
	ourPatterns.insert(std::make_pair("decimal-digit:segment",				new ComboPattern( { "integer-literal:optional-tick", "digit" } )));
	
	ourPatterns.insert(std::make_pair("hexadecimal-literal",				new ComboPattern( { "hexadecimal-literal-prefix", "hexadecimal-digit-sequence" } )));
	ourPatterns.insert(std::make_pair("hexadecimal-literal-prefix",			new EitherPattern( { "hexadecimal-literal-prefix:lower", "hexadecimal-literal-prefix:upper" })));
	ourPatterns.insert(std::make_pair("hexadecimal-literal-prefix:lower",	new WordPattern("0x")));
	ourPatterns.insert(std::make_pair("hexadecimal-literal-prefix:upper",	new WordPattern("0X")));
	ourPatterns.insert(std::make_pair("hexadecimal-digit-sequence",			new RepeatPattern("hexadecimal-digit:segment")));
	ourPatterns.insert(std::make_pair("hexadecimal-digit:segment",			new ComboPattern( { "integer-literal:optional-tick", "hexadecimal-digit" } )));
	ourPatterns.insert(std::make_pair("hexadecimal-digit",					new AnyOfPattern( "0123456789abcdefABCDEF" )));

	ourPatterns.insert(std::make_pair("integer-literal:optional-tick",		new OptionalPattern(std::shared_ptr<Pattern>(new CharPattern('\'')))));
	
	ourRootPatterns.emplace_back(new RootPattern("integer-literal",		Token::Type::Integer));

	ourRootPatterns.emplace_back(new RootPattern("semicolon",			Token::Type::Semicolon));
	ourRootPatterns.emplace_back(new RootPattern("comma",				Token::Type::Comma));

	ourRootPatterns.emplace_back(new RootPattern("star",				Token::Type::Star));

	ourRootPatterns.emplace_back(new RootPattern("l-paren",				Token::Type::L_Paren));
	ourRootPatterns.emplace_back(new RootPattern("r-paren",				Token::Type::R_Paren));
	ourRootPatterns.emplace_back(new RootPattern("l-brace",				Token::Type::L_Brace));
	ourRootPatterns.emplace_back(new RootPattern("r-brace",				Token::Type::R_Brace));

	ourRootPatterns.emplace_back(new RootPattern("int",					Token::Type::kw_int));
	ourRootPatterns.emplace_back(new RootPattern("char",				Token::Type::kw_char));
	ourRootPatterns.emplace_back(new RootPattern("return",				Token::Type::kw_return));

	ourRootPatterns.emplace_back(new RootPattern("identifier",			Token::Type::Identifier));
	ourRootPatterns.emplace_back(new RootPattern("whitespace",			Token::Type::WhiteSpace));
}
