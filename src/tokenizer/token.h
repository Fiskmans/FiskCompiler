#ifndef TOKENIZER_TOKEN_H
#define TOKENIZER_TOKEN_H

#include <string_view>

struct Token
{
public:

	enum class Type
	{
		Invalid,

		Semicolon,

		L_Paren,
		R_Paren,

		L_Brace,
		R_Brace,

		Less,
		LessLess,
		Plus,
		PlusPlus,

		NewLine,

		kw_int,

		WhiteSpace,

		Identifier
	};

	Token(Type aType, std::string_view aRawText)
		: myType(aType)
		, myRawText(aRawText)
	{
	}

	static std::string TypeToString(Type);

	Type		myType;
	std::string	myRawText;
};

#endif