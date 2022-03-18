
#include "tokenizer/token.h"


std::string Token::TypeToString(Type aType)
{
	switch (aType)	
	{
		case Token::Type::Invalid:			return "Invalid";		break;
		case Token::Type::Semicolon:		return "Semicolon";		break;
		case Token::Type::Integer:			return "Integer";		break;
		case Token::Type::L_Paren:			return "L_Paren";		break;
		case Token::Type::R_Paren:			return "R_Paren";		break;
		case Token::Type::L_Brace:			return "L_Brace";		break;
		case Token::Type::R_Brace:			return "R_Brace";		break;
		case Token::Type::Less:				return "Less";			break;
		case Token::Type::LessLess:			return "LessLess";		break;
		case Token::Type::Plus:				return "Plus";			break;
		case Token::Type::PlusPlus:			return "PlusPlus";		break;
		case Token::Type::NewLine:			return "NewLine";		break;
		case Token::Type::kw_int:			return "kw_int";		break;
		case Token::Type::kw_return:		return "kw_return";		break;
		case Token::Type::WhiteSpace:		return "Whitespace";	break;
		case Token::Type::Identifier:		return "Identifier";	break;
	}

	return "Missing token spec for " + std::to_string(static_cast<int>(aType));
}