
#include "tokenizer/token.h"


std::string Token::TypeToString(Type aType)
{
	switch (aType)	
	{
		case Token::Type::Invalid:				return "Invalid";				break;
		case Token::Type::Comment:				return "Comment";				break;

		case Token::Type::Hash:					return "Hash";					break;

		case Token::Type::Semicolon:			return "Semicolon";				break;
		case Token::Type::Dot:					return "Dot";					break;
		case Token::Type::Comma:				return "Comma";					break;
		case Token::Type::Colon:				return "Colon";					break;
		case Token::Type::Colon_colon:			return "Colon_colon";			break;
		

		case Token::Type::Integer:				return "Integer";				break;
		case Token::Type::String_literal:		return "String_literal";		break;
		case Token::Type::Char_literal:			return "Char_literal";			break;

		case Token::Type::L_Paren:				return "L_Paren";				break;
		case Token::Type::R_Paren:				return "R_Paren";				break;
		case Token::Type::L_Brace:				return "L_Brace";				break;
		case Token::Type::R_Brace:				return "R_Brace";				break;
		case Token::Type::L_Bracket:			return "L_Bracket";				break;
		case Token::Type::R_Bracket:			return "R_Bracket";				break;

		case Token::Type::Less:					return "Less";					break;
		case Token::Type::LessLess:				return "LessLess";				break;
		case Token::Type::Greater:				return "Greater";				break;
		case Token::Type::GreaterGreater:		return "GreaterGreater";		break;
		case Token::Type::Plus:					return "Plus";					break;
		case Token::Type::PlusPlus:				return "PlusPlus";				break;
		case Token::Type::Minus:				return "Minus";					break;
		case Token::Type::MinusMinus:			return "MinusMinus";			break;
		case Token::Type::Star:					return "Star";					break;
		case Token::Type::And:					return "And";					break;
		case Token::Type::Equals:				return "Equals";				break;
		case Token::Type::NotEquals:			return "NotEquals";				break;

		case Token::Type::NewLine:				return "NewLine";				break;

		case Token::Type::kw_alignas:			return "kw_alignas";			break;
		case Token::Type::kw_alignof:			return "kw_alignof";			break;
		case Token::Type::kw_asm:				return "kw_asm";				break;
		case Token::Type::kw_auto:				return "kw_auto";				break;
		case Token::Type::kw_bool:				return "kw_bool";				break;
		case Token::Type::kw_break:				return "kw_break";				break;
		case Token::Type::kw_case:				return "kw_case";				break;
		case Token::Type::kw_catch:				return "kw_catch";				break;
		case Token::Type::kw_char:				return "kw_char";				break;
		case Token::Type::kw_char8_t:			return "kw_char8_t";			break;
		case Token::Type::kw_char16_t:			return "kw_char16_t";			break;
		case Token::Type::kw_char32_t:			return "kw_char32_t";			break;
		case Token::Type::kw_class:				return "kw_class";				break;
		case Token::Type::kw_concept:			return "kw_concept";			break;
		case Token::Type::kw_const:				return "kw_const";				break;
		case Token::Type::kw_consteval:			return "kw_consteval";			break;
		case Token::Type::kw_constexpr:			return "kw_constexpr";			break;
		case Token::Type::kw_constinit:			return "kw_constinit";			break;
		case Token::Type::kw_const_cast:		return "kw_const_cast";			break;
		case Token::Type::kw_continue:			return "kw_continue";			break;
		case Token::Type::kw_co_await:			return "kw_co_await";			break;
		case Token::Type::kw_co_return:			return "kw_co_return";			break;
		case Token::Type::kw_co_yield:			return "kw_co_yield";			break;
		case Token::Type::kw_decltype:			return "kw_decltype";			break;
		case Token::Type::kw_default:			return "kw_default";			break;
		case Token::Type::kw_delete:			return "kw_delete";				break;
		case Token::Type::kw_do:				return "kw_do";					break;
		case Token::Type::kw_double:			return "kw_double";				break;
		case Token::Type::kw_dynamic_cast:		return "kw_dynamic_cast";		break;
		case Token::Type::kw_else:				return "kw_else";				break;
		case Token::Type::kw_enum:				return "kw_enum";				break;
		case Token::Type::kw_explicit:			return "kw_explicit";			break;
		case Token::Type::kw_export:			return "kw_export";				break;
		case Token::Type::kw_extern:			return "kw_extern";				break;
		case Token::Type::kw_false:				return "kw_false";				break;
		case Token::Type::kw_float:				return "kw_float";				break;
		case Token::Type::kw_for:				return "kw_for";				break;
		case Token::Type::kw_friend:			return "kw_friend";				break;
		case Token::Type::kw_goto:				return "kw_goto";				break;
		case Token::Type::kw_if:				return "kw_if";					break;
		case Token::Type::kw_inline:			return "kw_inline";				break;
		case Token::Type::kw_int:				return "kw_int";				break;
		case Token::Type::kw_long:				return "kw_long";				break;
		case Token::Type::kw_mutable:			return "kw_mutable";			break;
		case Token::Type::kw_namespace:			return "kw_namespace";			break;
		case Token::Type::kw_new:				return "kw_new";				break;
		case Token::Type::kw_noexcept:			return "kw_noexcept";			break;
		case Token::Type::kw_nullptr:			return "kw_nullptr";			break;
		case Token::Type::kw_operator:			return "kw_operator";			break;
		case Token::Type::kw_private:			return "kw_private";			break;
		case Token::Type::kw_protected:			return "kw_protected";			break;
		case Token::Type::kw_public:			return "kw_public";				break;
		case Token::Type::kw_register:			return "kw_register";			break;
		case Token::Type::kw_reinterpret_cast:	return "kw_reinterpret_cast";	break;
		case Token::Type::kw_requires:			return "kw_requires";			break;
		case Token::Type::kw_return:			return "kw_return";				break;
		case Token::Type::kw_short:				return "kw_short";				break;
		case Token::Type::kw_signed:			return "kw_signed";				break;
		case Token::Type::kw_sizeof:			return "kw_sizeof";				break;
		case Token::Type::kw_static:			return "kw_static";				break;
		case Token::Type::kw_static_assert:		return "kw_static_assert";		break;
		case Token::Type::kw_static_cast:		return "kw_static_cast";		break;
		case Token::Type::kw_struct:			return "kw_struct";				break;
		case Token::Type::kw_switch:			return "kw_switch";				break;
		case Token::Type::kw_template:			return "kw_template";			break;
		case Token::Type::kw_this:				return "kw_this";				break;
		case Token::Type::kw_thread_local:		return "kw_thread_local";		break;
		case Token::Type::kw_throw:				return "kw_throw";				break;
		case Token::Type::kw_true:				return "kw_true";				break;
		case Token::Type::kw_try:				return "kw_try";				break;
		case Token::Type::kw_typedef:			return "kw_typedef";			break;
		case Token::Type::kw_typeid:			return "kw_typeid";				break;
		case Token::Type::kw_typename:			return "kw_typename";			break;
		case Token::Type::kw_union:				return "kw_union";				break;
		case Token::Type::kw_unsigned:			return "kw_unsigned";			break;
		case Token::Type::kw_using:				return "kw_using";				break;
		case Token::Type::kw_virtual:			return "kw_virtual";			break;
		case Token::Type::kw_void:				return "kw_void";				break;
		case Token::Type::kw_volatile:			return "kw_volatile";			break;
		case Token::Type::kw_wchar_t:			return "kw_wchar_t";			break;
		case Token::Type::kw_while:				return "kw_while";				break;

		case Token::Type::WhiteSpace:			return "Whitespace";			break;
		case Token::Type::Identifier:			return "Identifier";			break;
	}

	return "Missing token spec for " + std::to_string(static_cast<int>(aType));
}