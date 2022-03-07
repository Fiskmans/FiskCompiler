#ifndef TOKENIZER_TOKEN_H
#define TOKENIZER_TOKEN_H

#include <string_view>

struct Token
{
	enum class Type
	{
		SemiColon,

		L_Paren,
		R_Paren,

		L_Brace,
		R_Brace,

		Less,
		LessLess,
		Plus,
		PlusPlus,

		NewLine,

		WhiteSpace,

		Text
	};

	Type				myType;
	std::string_view	myRawText;
};

#endif