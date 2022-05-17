
#include "tokenizer/token.h"


bool Token::IsPrepoccessorSpecific() const
{
	
	switch (myType)
	{
	case Token::Type::Invalid:
	case Token::Type::Comment:
	case Token::Type::Include_directive:
	case Token::Type::Header_name:
	case Token::Type::Hash:
	case Token::Type::HashHash:
	case Token::Type::WhiteSpace:
	case Token::Type::NewLine:
		return true;
	default:
		return false;
	}
}

bool Token::IsTextToken() const
{
	return myRawText.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") == std::string::npos;
}

size_t Token::EvaluateIntegral() const
{
	size_t base = 10;
	size_t at = 0;

	if(myRawText.starts_with("0"))
	{
		base = 8;
		at = 1;
	}
	else if (myRawText.starts_with("0x") || myRawText.starts_with("0X"))
	{
		base = 16;
		at = 2;
	}
	else if (myRawText.starts_with("0b") || myRawText.starts_with("0B"))
	{
		base = 2;
		at = 2;
	}

	size_t total = 0;
	for(size_t i = at; i < myRawText.size(); i++)
	{
		char c = myRawText[i];
		if(c == '\'')
			continue;

		size_t current;
		if(c >= '0' && c <= '9')
		{
			current = c - '0';
		}
		else if(c >= 'A' && c <= 'F')
		{
			current = c - 'A' + 9;
		}
		else if (c >= 'a' && c <= 'a')
		{
			current = c - 'a' + 9;
		}
		else
			throw std::exception("bad int parse");

		if (current >= base)
			throw std::exception("bad int parse");
		
		total *= base;
		total += current;
	}

	return total;
}

std::string Token::TypeToString(Type aType)
{
	switch (aType)	
	{
		case Token::Type::Invalid:				return "Invalid";
		case Token::Type::Comment:				return "Comment";

		case Token::Type::Include_directive:	return "Include_directive";
		case Token::Type::Header_name:			return "Header_name";

		case Token::Type::Integer:				return "Integer";
		case Token::Type::String_literal:		return "String_literal";
		case Token::Type::Char_literal:			return "Char_literal";

		case Token::Type::kw_alignas:			return "kw_alignas";
		case Token::Type::kw_alignof:			return "kw_alignof";
		case Token::Type::kw_asm:				return "kw_asm";
		case Token::Type::kw_auto:				return "kw_auto";
		case Token::Type::kw_bool:				return "kw_bool";
		case Token::Type::kw_break:				return "kw_break";
		case Token::Type::kw_case:				return "kw_case";
		case Token::Type::kw_catch:				return "kw_catch";
		case Token::Type::kw_char:				return "kw_char";
		case Token::Type::kw_char8_t:			return "kw_char8_t";
		case Token::Type::kw_char16_t:			return "kw_char16_t";
		case Token::Type::kw_char32_t:			return "kw_char32_t";
		case Token::Type::kw_class:				return "kw_class";
		case Token::Type::kw_concept:			return "kw_concept";
		case Token::Type::kw_const:				return "kw_const";
		case Token::Type::kw_consteval:			return "kw_consteval";
		case Token::Type::kw_constexpr:			return "kw_constexpr";
		case Token::Type::kw_constinit:			return "kw_constinit";
		case Token::Type::kw_const_cast:		return "kw_const_cast";
		case Token::Type::kw_continue:			return "kw_continue";
		case Token::Type::kw_co_await:			return "kw_co_await";
		case Token::Type::kw_co_return:			return "kw_co_return";
		case Token::Type::kw_co_yield:			return "kw_co_yield";
		case Token::Type::kw_decltype:			return "kw_decltype";
		case Token::Type::kw_default:			return "kw_default";
		case Token::Type::kw_delete:			return "kw_delete";
		case Token::Type::kw_do:				return "kw_do";
		case Token::Type::kw_double:			return "kw_double";
		case Token::Type::kw_dynamic_cast:		return "kw_dynamic_cast";
		case Token::Type::kw_else:				return "kw_else";
		case Token::Type::kw_enum:				return "kw_enum";
		case Token::Type::kw_explicit:			return "kw_explicit";
		case Token::Type::kw_export:			return "kw_export";
		case Token::Type::kw_extern:			return "kw_extern";
		case Token::Type::kw_false:				return "kw_false";
		case Token::Type::kw_float:				return "kw_float";
		case Token::Type::kw_for:				return "kw_for";
		case Token::Type::kw_friend:			return "kw_friend";
		case Token::Type::kw_goto:				return "kw_goto";
		case Token::Type::kw_if:				return "kw_if";
		case Token::Type::kw_inline:			return "kw_inline";
		case Token::Type::kw_int:				return "kw_int";
		case Token::Type::kw_long:				return "kw_long";
		case Token::Type::kw_mutable:			return "kw_mutable";
		case Token::Type::kw_namespace:			return "kw_namespace";
		case Token::Type::kw_new:				return "kw_new";
		case Token::Type::kw_noexcept:			return "kw_noexcept";
		case Token::Type::kw_nullptr:			return "kw_nullptr";
		case Token::Type::kw_operator:			return "kw_operator";
		case Token::Type::kw_private:			return "kw_private";
		case Token::Type::kw_protected:			return "kw_protected";
		case Token::Type::kw_public:			return "kw_public";
		case Token::Type::kw_register:			return "kw_register";
		case Token::Type::kw_reinterpret_cast:	return "kw_reinterpret_cast";
		case Token::Type::kw_requires:			return "kw_requires";
		case Token::Type::kw_return:			return "kw_return";
		case Token::Type::kw_short:				return "kw_short";
		case Token::Type::kw_signed:			return "kw_signed";
		case Token::Type::kw_sizeof:			return "kw_sizeof";
		case Token::Type::kw_static:			return "kw_static";
		case Token::Type::kw_static_assert:		return "kw_static_assert";
		case Token::Type::kw_static_cast:		return "kw_static_cast";
		case Token::Type::kw_struct:			return "kw_struct";
		case Token::Type::kw_switch:			return "kw_switch";
		case Token::Type::kw_template:			return "kw_template";
		case Token::Type::kw_this:				return "kw_this";
		case Token::Type::kw_thread_local:		return "kw_thread_local";
		case Token::Type::kw_throw:				return "kw_throw";
		case Token::Type::kw_true:				return "kw_true";
		case Token::Type::kw_try:				return "kw_try";
		case Token::Type::kw_typedef:			return "kw_typedef";
		case Token::Type::kw_typeid:			return "kw_typeid";
		case Token::Type::kw_typename:			return "kw_typename";
		case Token::Type::kw_union:				return "kw_union";
		case Token::Type::kw_unsigned:			return "kw_unsigned";
		case Token::Type::kw_using:				return "kw_using";
		case Token::Type::kw_virtual:			return "kw_virtual";
		case Token::Type::kw_void:				return "kw_void";
		case Token::Type::kw_volatile:			return "kw_volatile";
		case Token::Type::kw_wchar_t:			return "kw_wchar_t";
		case Token::Type::kw_while:				return "kw_while";

		case Token::Type::L_Brace:				return "L_Brace";
		case Token::Type::R_Brace:				return "R_Brace";
		case Token::Type::L_Bracket:			return "L_Bracket";
		case Token::Type::R_Bracket:			return "R_Bracket";
		case Token::Type::Hash:					return "Hash";
		case Token::Type::HashHash:				return "HashHash";
		case Token::Type::L_Paren:				return "L_Paren";
		case Token::Type::R_Paren:				return "R_Paren";
		case Token::Type::Semicolon:			return "Semicolon";
		case Token::Type::Colon:				return "Colon";
		case Token::Type::Ellipsis:				return "Ellipsis";
		case Token::Type::Question:				return "Question";
		case Token::Type::Colon_colon:			return "Colon_colon";
		case Token::Type::Dot:					return "Dot";
		case Token::Type::DotStar:				return "DotStar";
		case Token::Type::Arrow:				return "Arrow";
		case Token::Type::ArrowStar:			return "ArrowStar";
		case Token::Type::Comma:				return "Comma";

		case Token::Type::Complement:			return "Complement";
		case Token::Type::Not:					return "Not";
		case Token::Type::Plus:					return "Plus";
		case Token::Type::Minus:				return "Minus";
		case Token::Type::Star:					return "Star";
		case Token::Type::Div:					return "Div";
		case Token::Type::Mod:					return "Mod";
		case Token::Type::Xor:					return "Xor";
		case Token::Type::BitAnd:				return "BitAnd";
		case Token::Type::BitOr:				return "BitOr";
		case Token::Type::Equal:				return "Equal";
		case Token::Type::PlusEqual:			return "PlusEqual";
		case Token::Type::MinusEqual:			return "MinusEqual";
		case Token::Type::StarEqual:			return "StarEqual";
		case Token::Type::DivEqual:				return "DivEqual";
		case Token::Type::ModEqual:				return "ModEqual";
		case Token::Type::XorEqual:				return "XorEqual";
		case Token::Type::BitAndEqual:			return "BitAndEqual";
		case Token::Type::BitOrEqual:			return "BitOrEqual";
		case Token::Type::EqualEqual:			return "EqualEqual";
		case Token::Type::NotEquals:			return "NotEquals";
		case Token::Type::Less:					return "Less";
		case Token::Type::Greater:				return "Greater";
		case Token::Type::LessEqual:			return "LessEqual";
		case Token::Type::GreaterEqual:			return "GreaterEqual";
		case Token::Type::LessEqualGreater:		return "LessEqualGreater";
		case Token::Type::And:					return "And";
		case Token::Type::Or:					return "Or";
		case Token::Type::LessLess:				return "LessLess";
		case Token::Type::GreaterGreater:		return "GreaterGreater";
		case Token::Type::LessLessEqual:		return "LessLessEqual";
		case Token::Type::GreaterGreaterEqual:	return "GreaterGreaterEqual";
		case Token::Type::PlusPlus:				return "PlusPlus";
		case Token::Type::MinusMinus:			return "MinusMinus";

		case Token::Type::WhiteSpace:			return "WhiteSpace";
		case Token::Type::NewLine:				return "NewLine";

		case Token::Type::Identifier:			return "Identifier";
	}

	return "Missing token spec for " + std::to_string(static_cast<int>(aType));
}