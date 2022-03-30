#ifndef TOKENIZER_TOKEN_H
#define TOKENIZER_TOKEN_H

#include <string>
#include <string_view>

struct Token
{
public:

	enum class Type
	{
		Invalid,
		Comment,

		Hash,

		Semicolon,
		Dot,
		Comma,
		Colon,
		Colon_colon,

		Star,
		And,
		Equals,
		NotEquals,
		Less,
		LessLess,
		Greater,
		GreaterGreater,


		Plus,
		PlusPlus,
		
		Minus,
		MinusMinus,

		Integer,
		String_literal,
		Char_literal,

		L_Paren,
		R_Paren,

		L_Brace,
		R_Brace,
		
		L_Bracket,
		R_Bracket,


		NewLine,

		kw_alignas,
		kw_alignof,
		kw_asm,
		kw_auto,
		kw_bool,
		kw_break,
		kw_case,
		kw_catch,
		kw_char,
		kw_char8_t,
		kw_char16_t,
		kw_char32_t,
		kw_class,
		kw_concept,
		kw_const,
		kw_consteval,
		kw_constexpr,
		kw_constinit,
		kw_const_cast,
		kw_continue,
		kw_co_await,
		kw_co_return,
		kw_co_yield,
		kw_decltype,
		kw_default,
		kw_delete,
		kw_do,
		kw_double,
		kw_dynamic_cast,
		kw_else,
		kw_enum,
		kw_explicit,
		kw_export,
		kw_extern,
		kw_false,
		kw_float,
		kw_for,
		kw_friend,
		kw_goto,
		kw_if,
		kw_inline,
		kw_int,
		kw_long,
		kw_mutable,
		kw_namespace,
		kw_new,
		kw_noexcept,
		kw_nullptr,
		kw_operator,
		kw_private,
		kw_protected,
		kw_public,
		kw_register,
		kw_reinterpret_cast,
		kw_requires,
		kw_return,
		kw_short,
		kw_signed,
		kw_sizeof,
		kw_static,
		kw_static_assert,
		kw_static_cast,
		kw_struct,
		kw_switch,
		kw_template,
		kw_this,
		kw_thread_local,
		kw_throw,
		kw_true,
		kw_try,
		kw_typedef,
		kw_typeid,
		kw_typename,
		kw_union,
		kw_unsigned,
		kw_using,
		kw_virtual,
		kw_void,
		kw_volatile,
		kw_wchar_t,
		kw_while,

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