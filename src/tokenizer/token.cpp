
#include "tokenizer/token.h"

#include "common/CompilerContext.h"

namespace tokenizer
{
	Token Token::SafetyToken = Token(Token::Type::Invalid, "unmapped Token", 0, 0);

	std::filesystem::path
	Token::GetCurrentFile()
	{
		return CompilerContext::GetCurrentFile();
	}

	bool
	Token::IsPrepoccessorSpecific() const
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
		return myRawText.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") > 0 &&
			myRawText.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 1) == std::string::npos;
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

			case Token::Type::Integer_literal:		return "Integer";
			case Token::Type::String_literal:		return "String";
			case Token::Type::Char_literal:			return "Char";
			case Token::Type::Floating_literal:		return "Floating";

			case Token::Type::kw_alignas:			return "alignas";
			case Token::Type::kw_alignof:			return "alignof";
			case Token::Type::kw_asm:				return "asm";
			case Token::Type::kw_auto:				return "auto";
			case Token::Type::kw_bool:				return "bool";
			case Token::Type::kw_break:				return "break";
			case Token::Type::kw_case:				return "case";
			case Token::Type::kw_catch:				return "catch";
			case Token::Type::kw_char:				return "char";
			case Token::Type::kw_char8_t:			return "char8_t";
			case Token::Type::kw_char16_t:			return "char16_t";
			case Token::Type::kw_char32_t:			return "char32_t";
			case Token::Type::kw_class:				return "class";
			case Token::Type::kw_concept:			return "concept";
			case Token::Type::kw_const:				return "const";
			case Token::Type::kw_consteval:			return "consteval";
			case Token::Type::kw_constexpr:			return "constexpr";
			case Token::Type::kw_constinit:			return "constinit";
			case Token::Type::kw_const_cast:		return "const_cast";
			case Token::Type::kw_continue:			return "continue";
			case Token::Type::kw_co_await:			return "co_await";
			case Token::Type::kw_co_return:			return "co_return";
			case Token::Type::kw_co_yield:			return "co_yield";
			case Token::Type::kw_decltype:			return "decltype";
			case Token::Type::kw_default:			return "default";
			case Token::Type::kw_delete:			return "delete";
			case Token::Type::kw_do:				return "do";
			case Token::Type::kw_double:			return "double";
			case Token::Type::kw_dynamic_cast:		return "dynamic_cast";
			case Token::Type::kw_else:				return "else";
			case Token::Type::kw_enum:				return "enum";
			case Token::Type::kw_explicit:			return "explicit";
			case Token::Type::kw_export:			return "export";
			case Token::Type::kw_extern:			return "extern";
			case Token::Type::kw_false:				return "false";
			case Token::Type::kw_float:				return "float";
			case Token::Type::kw_for:				return "for";
			case Token::Type::kw_friend:			return "friend";
			case Token::Type::kw_goto:				return "goto";
			case Token::Type::kw_if:				return "if";
			case Token::Type::kw_inline:			return "inline";
			case Token::Type::kw_int:				return "int";
			case Token::Type::kw_long:				return "long";
			case Token::Type::kw_mutable:			return "mutable";
			case Token::Type::kw_namespace:			return "namespace";
			case Token::Type::kw_new:				return "new";
			case Token::Type::kw_noexcept:			return "noexcept";
			case Token::Type::kw_nullptr:			return "nullptr";
			case Token::Type::kw_operator:			return "operator";
			case Token::Type::kw_private:			return "private";
			case Token::Type::kw_protected:			return "protected";
			case Token::Type::kw_public:			return "public";
			case Token::Type::kw_register:			return "register";
			case Token::Type::kw_reinterpret_cast:	return "reinterpret_cast";
			case Token::Type::kw_requires:			return "requires";
			case Token::Type::kw_return:			return "return";
			case Token::Type::kw_short:				return "short";
			case Token::Type::kw_signed:			return "signed";
			case Token::Type::kw_sizeof:			return "sizeof";
			case Token::Type::kw_static:			return "static";
			case Token::Type::kw_static_assert:		return "static_assert";
			case Token::Type::kw_static_cast:		return "static_cast";
			case Token::Type::kw_struct:			return "struct";
			case Token::Type::kw_switch:			return "switch";
			case Token::Type::kw_template:			return "template";
			case Token::Type::kw_this:				return "this";
			case Token::Type::kw_thread_local:		return "thread_local";
			case Token::Type::kw_throw:				return "throw";
			case Token::Type::kw_true:				return "true";
			case Token::Type::kw_try:				return "try";
			case Token::Type::kw_typedef:			return "typedef";
			case Token::Type::kw_typeid:			return "typeid";
			case Token::Type::kw_typename:			return "typename";
			case Token::Type::kw_union:				return "union";
			case Token::Type::kw_unsigned:			return "unsigned";
			case Token::Type::kw_using:				return "using";
			case Token::Type::kw_virtual:			return "virtual";
			case Token::Type::kw_void:				return "void";
			case Token::Type::kw_volatile:			return "volatile";
			case Token::Type::kw_wchar_t:			return "wchar_t";
			case Token::Type::kw_while:				return "while";

			case Token::Type::L_Brace:				return "{";
			case Token::Type::R_Brace:				return "}";
			case Token::Type::L_Bracket:			return "[";
			case Token::Type::R_Bracket:			return "]";
			case Token::Type::Hash:					return "#";
			case Token::Type::HashHash:				return "##";
			case Token::Type::L_Paren:				return "(";
			case Token::Type::R_Paren:				return ")";
			case Token::Type::Semicolon:			return ";";
			case Token::Type::Colon:				return ":";
			case Token::Type::Ellipsis:				return "...";
			case Token::Type::Question:				return "?";
			case Token::Type::ColonColon:			return "::";
			case Token::Type::Dot:					return ".";
			case Token::Type::DotStar:				return ".*";
			case Token::Type::Arrow:				return "->";
			case Token::Type::ArrowStar:			return "->*";
			case Token::Type::Comma:				return ",";

			case Token::Type::Complement:			return "~";
			case Token::Type::Not:					return "!";
			case Token::Type::Plus:					return "+";
			case Token::Type::Minus:				return "-";
			case Token::Type::Star:					return "*";
			case Token::Type::Div:					return "/";
			case Token::Type::Mod:					return "%";
			case Token::Type::Xor:					return "^";
			case Token::Type::BitAnd:				return "&";
			case Token::Type::BitOr:				return "|";
			case Token::Type::Equal:				return "=";
			case Token::Type::PlusEqual:			return "+=";
			case Token::Type::MinusEqual:			return "-=";
			case Token::Type::StarEqual:			return "*=";
			case Token::Type::DivEqual:				return "/=";
			case Token::Type::ModEqual:				return "%=";
			case Token::Type::XorEqual:				return "^=";
			case Token::Type::BitAndEqual:			return "&=";
			case Token::Type::BitOrEqual:			return "|=";
			case Token::Type::EqualEqual:			return "==";
			case Token::Type::NotEquals:			return "!=";
			case Token::Type::Less:					return "<";
			case Token::Type::Greater:				return ">";
			case Token::Type::LessEqual:			return "<=";
			case Token::Type::GreaterEqual:			return ">=";
			case Token::Type::LessEqualGreater:		return "<=>";
			case Token::Type::And:					return "&&";
			case Token::Type::Or:					return "||";
			case Token::Type::LessLess:				return "<<";
			case Token::Type::GreaterGreater:		return ">>";
			case Token::Type::LessLessEqual:		return "<<=";
			case Token::Type::GreaterGreaterEqual:	return ">>=";
			case Token::Type::PlusPlus:				return "++";
			case Token::Type::MinusMinus:			return "--";

			case Token::Type::WhiteSpace:			return "WhiteSpace";
			case Token::Type::NewLine:				return "NewLine";

			case Token::Type::Identifier:			return "Identifier";
		}

		return "Missing token spec for " + std::to_string(static_cast<int>(aType));
	}
} // tokenizer