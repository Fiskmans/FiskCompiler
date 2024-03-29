#ifndef TOKENIZER_TOKEN_H
#define TOKENIZER_TOKEN_H

#include <string>
#include <string_view>
#include <ranges>
#include <filesystem>

namespace tokenizer
{
	struct Token
	{
	public:
		enum class Type
		{
			Invalid,
			Comment,

			Include_directive,
			Header_name,

			Integer_literal,
			String_literal,
			Char_literal,
			Floating_literal,

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

			L_Brace,
			R_Brace,
			L_Bracket,
			R_Bracket,
			Hash,
			HashHash,
			L_Paren,
			R_Paren,
			Semicolon,
			Colon,
			Ellipsis,
			Question,
			ColonColon,
			Dot,
			DotStar,
			Arrow,
			ArrowStar,
			Comma,

			Complement,
			Not,
			Plus,
			Minus,
			Star,
			Div,
			Mod,
			Xor,
			BitAnd,
			BitOr,
			Equal,
			PlusEqual,
			MinusEqual,
			StarEqual,
			DivEqual,
			ModEqual,
			XorEqual,
			BitAndEqual,
			BitOrEqual,
			EqualEqual,
			NotEquals,
			Less,
			Greater,
			LessEqual,
			GreaterEqual,
			LessEqualGreater,
			And,
			Or,
			LessLess,
			GreaterGreater,
			LessLessEqual,
			GreaterGreaterEqual,
			PlusPlus,
			MinusMinus,

			WhiteSpace,
			NewLine,
			Identifier
		};

		static std::filesystem::path GetCurrentFile();

		Token(Type aType, std::string_view aRawText, size_t aLine, size_t aColumn)
			: myType(aType)
			, myFile(GetCurrentFile())
			, myRawText(aRawText)
			, myLine(aLine)
			, myColumn(aColumn)
		{
		}

		//auto NotWhitespace = std::ranges::views::filter([](Token aToken) { return aToken.myType != Type::WhiteSpace && aToken.myType != Type::NewLine; });

		bool IsPrepoccessorSpecific() const;
		bool IsTextToken() const;
		size_t EvaluateIntegral() const;

		static std::string	TypeToString(Type);

		Type					myType;
		std::filesystem::path	myFile;
		std::string				myRawText;
		size_t					myColumn;
		size_t					myLine;


		static Token			SafetyToken;
	};


	namespace token_helpers
	{
		inline auto IsNotWhitespace = std::ranges::views::filter([](const Token& aToken) { return aToken.myType != Token::Type::WhiteSpace && aToken.myType != Token::Type::NewLine; });
		inline auto IsLogical = std::ranges::views::filter([](const Token& aToken ) { return !aToken.IsPrepoccessorSpecific(); });
		inline auto AsString = std::ranges::views::transform([](const Token::Type& aType ) { return Token::TypeToString(aType); });
	}

	using Identifier = Token;
} // tokenizer

#endif