#include "tokenMatcher.h"

#include "common/CompilerContext.h"

namespace tokenizer
{

	std::vector<std::shared_ptr<TokenMatcher::RootPattern>> TokenMatcher::ourRootPatterns;

	namespace patterns
	{
	
		class CharPattern : public TokenMatcher::Pattern
		{
		public:
			CharPattern(char aChar)
				: myChar(aChar)
			{
			}

			virtual std::optional<size_t> Match(const std::string_view& aView) override
			{
				if (aView.length() < 1)
					return {};

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

			virtual std::optional<size_t> Match(const std::string_view& aView) override
			{
				if (aView.starts_with(myWord))
					return myWord.length();
	
				return {};
			}
		private:
			std::string myWord;
		};

		class CaseInsensativeWordPattern : public TokenMatcher::Pattern
		{
		public:
			CaseInsensativeWordPattern(const std::string_view& aWord)
				: myWord(aWord)
			{
			}

			virtual std::optional<size_t> Match(const std::string_view& aView) override
			{
				if (aView.length() < myWord.length())
					return {};

				for (size_t i = 0; i < myWord.length(); i++)
					if(!CaseInsensativeMatch(aView.at(i), myWord.at(i)))
						return {};
	
				return myWord.length();
			}

			bool CaseInsensativeMatch(char aLHS, char aRHS)
			{
				return std::tolower(aLHS) == std::tolower(aRHS);
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

			virtual std::optional<size_t> Match(const std::string_view& aView) override
			{
				if (aView.empty())
					return {};

				if (myChars.find(aView[0]) != std::string::npos)
					return 1;
	
				return {};
			}
		private:
			std::string myChars;
		};

		class NotOfPattern : public TokenMatcher::Pattern
		{
	
		public:
			NotOfPattern(const std::string_view& aChars)
				: myChars(aChars)
			{
			}

			virtual std::optional<size_t> Match(const std::string_view& aView) override
			{
				if (myChars.find(aView[0]) == std::string::npos)
					return 1;
	
				return {};
			}
		private:
			std::string myChars;
		};

		class EitherPattern : public TokenMatcher::Pattern
		{
		public:
			EitherPattern(std::shared_ptr<TokenMatcher::Pattern> aOne, std::shared_ptr<TokenMatcher::Pattern> aTwo)
			{
				if(EitherPattern* one = dynamic_cast<EitherPattern*>(aOne.get()))
				{
					for (std::shared_ptr<TokenMatcher::Pattern>& option : one->myOptions)
						myOptions.push_back(option);
				}
				else
				{
					myOptions.push_back(aOne);
				}
				if(EitherPattern* two = dynamic_cast<EitherPattern*>(aTwo.get()))
				{
					for (std::shared_ptr<TokenMatcher::Pattern>& option : two->myOptions)
						myOptions.push_back(option);
				}
				else
				{
					myOptions.push_back(aTwo);
				}
			}

			EitherPattern(const std::initializer_list<std::shared_ptr<TokenMatcher::Pattern>>& aList)
				: myOptions(aList)
			{
			}
	
			virtual std::optional<size_t> Match(const std::string_view& aView) override
			{
				size_t longest = 0;

				for (std::shared_ptr<TokenMatcher::Pattern>& patternName : myOptions)
				{
					std::optional<size_t> res = patternName->Match(aView);
					if (res)
						longest = std::max(longest, *res);
				}

				return longest > 0 ? longest : std::optional<size_t>();
			}

		private:
			std::vector<std::shared_ptr<TokenMatcher::Pattern>> myOptions;
		};

		class ComboPattern : public TokenMatcher::Pattern
		{
		public:
			ComboPattern(const std::initializer_list<std::shared_ptr<TokenMatcher::Pattern>>& aList)
				: myList(aList)
			{
			}

			ComboPattern(const std::vector<std::shared_ptr<TokenMatcher::Pattern>>& aList)
				: myList(aList)
			{
			}
	
			virtual std::optional<size_t> Match(const std::string_view& aView) override
			{
				size_t total = 0;

				for (std::shared_ptr<TokenMatcher::Pattern>& pattern : myList)
				{
					const std::string_view next(aView.begin() + total, aView.end());
					std::optional<size_t> res = pattern->Match(next);

					if (!res)
						return {};

					total += *res;
				}

				return total;
			}

		private:
			std::vector<std::shared_ptr<TokenMatcher::Pattern>> myList;
		};

		class RepeatPattern : public TokenMatcher::Pattern
		{
		public:
			RepeatPattern(std::shared_ptr<TokenMatcher::Pattern> aBase)
				: myBase(aBase)
			{
			}
	
			virtual std::optional<size_t> Match(const std::string_view& aView) override
			{
				size_t total = 0;
				std::string_view view(aView);
				while (!view.empty())
				{
					std::optional<size_t> res = myBase->Match(view);
					if (!res)
						break;
					view = std::string_view(view.begin() + *res, view.end());
					total += *res;
				}
				return total > 0 ? total : std::optional<size_t>();
			}
		private:
			std::shared_ptr<TokenMatcher::Pattern> myBase;
		};
	
		class RepeatPatternRange : public TokenMatcher::Pattern
		{
		public:
			RepeatPatternRange(std::shared_ptr<TokenMatcher::Pattern> aBase, size_t aMinimun, size_t aMaximum)
				: myBase(aBase)
				, myMinimum(aMinimun)
				, myMaximum(aMaximum)
			{
			}
	
			virtual std::optional<size_t> Match(const std::string_view& aView) override
			{
				size_t total = 0;
				size_t totalMatches = 0;
				std::string_view view(aView);
				for (size_t i = 0; i < myMaximum; i++)
				{
					if(view.empty())
						break;
			
					std::optional<size_t> res = myBase->Match(view);
					if (!res)
						break;
				
					totalMatches++;

					view = std::string_view(view.begin() + *res, view.end());
					total += *res;
				}
				return totalMatches > myMinimum ? total : std::optional<size_t>();
			}
		private:
			std::shared_ptr<TokenMatcher::Pattern> myBase;
			size_t myMinimum;
			size_t myMaximum;
		};

		class OptionalPattern : public TokenMatcher::Pattern
		{
		public:
			OptionalPattern(std::shared_ptr<TokenMatcher::Pattern> aBase)
				: myBase(aBase)
			{
			}
	
			virtual std::optional<size_t> Match(const std::string_view& aView) override
			{
				std::optional<size_t> res = myBase->Match(aView);
				return res ? res : 0;
			}
		private:
			std::shared_ptr<TokenMatcher::Pattern> myBase;
		};
	}

	namespace pattern_literals
	{
		using namespace patterns;
		std::shared_ptr<TokenMatcher::Pattern> operator""_c(const char aValue)
		{
			return std::shared_ptr<TokenMatcher::Pattern>(new CharPattern(aValue));
		}
	
		std::shared_ptr<TokenMatcher::Pattern> operator""_exact(const char* aValue, size_t aSize)
		{
			return std::shared_ptr<TokenMatcher::Pattern>(new WordPattern(std::string_view(aValue, aSize)));
		}
	
		std::shared_ptr<TokenMatcher::Pattern> operator""_nocase(const char* aValue, size_t aSize)
		{
			return std::shared_ptr<TokenMatcher::Pattern>(new CaseInsensativeWordPattern(std::string_view(aValue, aSize)));
		}

		std::shared_ptr<TokenMatcher::Pattern> operator""_any(const char* aValue, size_t aSize)
		{
			return std::shared_ptr<TokenMatcher::Pattern>(new AnyOfPattern(std::string_view(aValue, aSize)));
		}

		std::shared_ptr<TokenMatcher::Pattern> operator""_notof(const char* aValue, size_t aSize)
		{
			return std::shared_ptr<TokenMatcher::Pattern>(new NotOfPattern(std::string_view(aValue, aSize)));
		}
	}

	namespace pattern_combinations
	{
		using namespace patterns;
		std::shared_ptr<TokenMatcher::Pattern> operator or(std::shared_ptr<TokenMatcher::Pattern> aLHS, std::shared_ptr<TokenMatcher::Pattern> aRHS)
		{
			return std::shared_ptr<TokenMatcher::Pattern>(new EitherPattern(aLHS, aRHS));
		}

		std::shared_ptr<TokenMatcher::Pattern> operator and(std::shared_ptr<TokenMatcher::Pattern> aLHS, std::shared_ptr<TokenMatcher::Pattern> aRHS)
		{
			return std::shared_ptr<TokenMatcher::Pattern>(new ComboPattern({ aLHS, aRHS }));
		}
	}

	namespace pattern_helpers
	{
		using namespace patterns;
		std::shared_ptr<TokenMatcher::Pattern> Repeat(std::shared_ptr<TokenMatcher::Pattern> aPattern)
		{
			return std::shared_ptr<TokenMatcher::Pattern>(new RepeatPattern(aPattern));
		}

		std::shared_ptr<TokenMatcher::Pattern> RepeatCapped(std::shared_ptr<TokenMatcher::Pattern> aPattern, size_t aMaximum)
		{
			return std::shared_ptr<TokenMatcher::Pattern>(new RepeatPatternRange(aPattern, 0, aMaximum));
		}

		std::shared_ptr<TokenMatcher::Pattern> Optionally(std::shared_ptr<TokenMatcher::Pattern> aPattern)
		{
			return std::shared_ptr<TokenMatcher::Pattern>(new OptionalPattern(aPattern));
		}
	}

	void TokenMatcher::MatchTokens(std::vector<Token>& aWrite, const std::string& aLine, Context& aContext)
	{
		using namespace pattern_literals;
		using namespace pattern_helpers;
		using namespace pattern_combinations;

		LoadPatterns();

		//Tokens with special rules
		std::shared_ptr<Pattern> rawString			= "R\""_exact and Optionally(RepeatCapped(" ()\\\t\f\v"_notof, 16)) and '('_c; // Matches:		R"<delimiter>(
		std::shared_ptr<Pattern> multiLineComment	= "/*"_exact;
		std::shared_ptr<Pattern> comment			= "//"_exact;
		std::shared_ptr<Pattern> includeDiretive	= '#'_c and Optionally(Repeat(" \t"_any)) and "include"_exact;
		std::shared_ptr<Pattern> headerName			=	('<'_c and Repeat("\n>"_notof) and '>'_c)
													or	('"'_c and Repeat("\n\""_notof) and '"'_c);

		std::string_view lineLeft = aLine; 
		size_t column = 0;

		bool hasIncludeDirective = false;
		bool trimWhitespace = !!CompilerContext::GetFlag("p:no_whitespace");

		while (!lineLeft.empty())
		{
			size_t toConsume = 0;
			Token::Type resultingType = Token::Type::Invalid;


			if (aContext.myCurrentTokenIsPotentiallyMultiLine)
			{
				size_t pos = lineLeft.find(aContext.endSequence);
				if (pos == std::string_view::npos)
				{
					aContext.tokenBuffer += lineLeft;
					aContext.tokenBuffer += "\n";
					return;
				}

				aContext.tokenBuffer += SplitView(lineLeft, pos + aContext.endSequence.length());

				aWrite.push_back(Token(aContext.tokenType, aContext.tokenBuffer, CompilerContext::GetCurrentLine(), aContext.column));

				aContext.myCurrentTokenIsPotentiallyMultiLine = false;
				continue;
			}

			{
				std::optional<size_t> includeDirectiveResult = includeDiretive->Match(lineLeft);
				if (includeDirectiveResult)
				{
					aWrite.push_back(Token(Token::Type::Include_directive, SplitView(lineLeft, *includeDirectiveResult), CompilerContext::GetCurrentLine(), column));
					column += *includeDirectiveResult;
					hasIncludeDirective = true;
					continue;
				}
			}

			{
				std::optional<size_t> rawStringResult = rawString->Match(lineLeft);
				if (rawStringResult)
				{
					aContext.myCurrentTokenIsPotentiallyMultiLine = true;
					aContext.tokenType = Token::Type::String_literal;
					aContext.tokenBuffer += SplitView(lineLeft, *rawStringResult);
					aContext.endSequence = ")" + aContext.tokenBuffer.substr(2, *rawStringResult - 3) + "\"";
					aContext.column = column;
					column += *rawStringResult;
					continue;
				}
			}

			{
				std::optional<size_t> multiLineCommentResult = multiLineComment->Match(lineLeft);
				if (multiLineCommentResult)
				{
					aContext.myCurrentTokenIsPotentiallyMultiLine = true;
					aContext.tokenType = Token::Type::Comment;
					aContext.tokenBuffer += SplitView(lineLeft, *multiLineCommentResult);
					aContext.endSequence = "*/";
					aContext.column = column;
					column += *multiLineCommentResult;
					continue;
				}
			}

			if (comment->Match(lineLeft))
			{
				aWrite.push_back(Token(Token::Type::Comment, lineLeft, CompilerContext::GetCurrentLine(), column));
				break;
			}

			if (hasIncludeDirective)
			{
				std::optional<size_t> amount = headerName->Match(lineLeft);
				if(amount)
				{
					aWrite.push_back(Token(Token::Type::Header_name, SplitView(lineLeft, *amount), CompilerContext::GetCurrentLine(), column));
					column += *amount;
					continue;
				}
			}

			for (auto& pattern : ourRootPatterns)
			{
				auto [amount, type] = pattern->Match(lineLeft);
				if(amount > toConsume)
				{
					toConsume = amount;
					resultingType = type;
				}
			}


			if (toConsume == 0)
			{
				CompilerContext::EmitError("Invalid token", CompilerContext::GetCurrentFile(), column, CompilerContext::GetCurrentLine(), lineLeft.size());
				break;
			}

			std::string_view rawToken = SplitView(lineLeft, toConsume);
			column += toConsume;

			if (trimWhitespace && resultingType == Token::Type::WhiteSpace)
				continue;

			aWrite.push_back(Token(resultingType, rawToken, CompilerContext::GetCurrentLine(), column - toConsume));
		}
		if (aContext.myCurrentTokenIsPotentiallyMultiLine)
		{
			aContext.tokenBuffer += "\n";
		}
		else
		{
			aWrite.push_back(Token(Token::Type::NewLine, "\n", CompilerContext::GetCurrentLine(), column));
		}
	}

	std::string_view TokenMatcher::SplitView(std::string_view& aInOutLeft, size_t aAmount)
	{
		std::string_view out(aInOutLeft.begin(), aInOutLeft.begin() + aAmount);
		aInOutLeft = std::string_view(aInOutLeft.begin() + aAmount, aInOutLeft.end());
		return out;
	}



	void TokenMatcher::LoadPatterns()
	{
		if(!ourRootPatterns.empty())
			return;

		using namespace pattern_literals;
		using namespace pattern_combinations;
		using namespace pattern_helpers;

		std::shared_ptr<TokenMatcher::Pattern> nondigit						= "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"_any;
		std::shared_ptr<TokenMatcher::Pattern> digit						= "0123456789"_any;
		std::shared_ptr<TokenMatcher::Pattern> nonzero_digit				= "123456789"_any;
		std::shared_ptr<TokenMatcher::Pattern> octal_digit					= "01234567"_any;
		std::shared_ptr<TokenMatcher::Pattern> hexadecimal_digit			= "0123456789ABCDEFabcdef"_any;
		std::shared_ptr<TokenMatcher::Pattern> escape_sequence				=	('\\'_c			and	"'\"?\\abfnrtv"_any	)													//simple-escape-sequence
																			or	('\\'_c			and	octal_digit and Optionally(octal_digit) and Optionally(octal_digit))	//octal-escape-sequence
																			or	("\\x"_exact	and hexadecimal_digit and Optionally(Repeat(hexadecimal_digit)));			//hexadecimal-escape-sequence;
		
		std::shared_ptr<TokenMatcher::Pattern> digitSequence				= digit and Optionally(Repeat(
																				Optionally('\''_c)
																				and digit));
		
		std::shared_ptr<TokenMatcher::Pattern> hexadecimalDigitSequence		= hexadecimal_digit and Optionally(Repeat(
																				Optionally('\''_c)
																				and hexadecimal_digit));

	#pragma region keywords
		BuildPattern(Token::Type::kw_alignas)			= "alignas"_exact;
		BuildPattern(Token::Type::kw_alignof)			= "alignof"_exact;
		BuildPattern(Token::Type::kw_asm)				= "asm"_exact;
		BuildPattern(Token::Type::kw_auto)				= "auto"_exact;
		BuildPattern(Token::Type::kw_bool)				= "bool"_exact;
		BuildPattern(Token::Type::kw_break)				= "break"_exact;
		BuildPattern(Token::Type::kw_case)				= "case"_exact;
		BuildPattern(Token::Type::kw_catch)				= "catch"_exact;
		BuildPattern(Token::Type::kw_char)				= "char"_exact;
		BuildPattern(Token::Type::kw_char8_t)			= "char8_t"_exact;
		BuildPattern(Token::Type::kw_char16_t)			= "char16_t"_exact;
		BuildPattern(Token::Type::kw_char32_t)			= "char32_t"_exact;
		BuildPattern(Token::Type::kw_class)				= "class"_exact;
		BuildPattern(Token::Type::kw_concept)			= "concept"_exact;
		BuildPattern(Token::Type::kw_const)				= "const"_exact;
		BuildPattern(Token::Type::kw_consteval)			= "consteval"_exact;
		BuildPattern(Token::Type::kw_constexpr)			= "constexpr"_exact;
		BuildPattern(Token::Type::kw_constinit)			= "constinit"_exact;
		BuildPattern(Token::Type::kw_const_cast)		= "const_cast"_exact;
		BuildPattern(Token::Type::kw_continue)			= "continue"_exact;
		BuildPattern(Token::Type::kw_co_await)			= "co_await"_exact;
		BuildPattern(Token::Type::kw_co_return)			= "co_return"_exact;
		BuildPattern(Token::Type::kw_co_yield)			= "co_yield"_exact;
		BuildPattern(Token::Type::kw_decltype)			= "decltype"_exact;
		BuildPattern(Token::Type::kw_default)			= "default"_exact;
		BuildPattern(Token::Type::kw_delete)			= "delete"_exact;
		BuildPattern(Token::Type::kw_do)				= "do"_exact;
		BuildPattern(Token::Type::kw_double)			= "double"_exact;
		BuildPattern(Token::Type::kw_dynamic_cast)		= "dynamic_cast"_exact;
		BuildPattern(Token::Type::kw_else)				= "else"_exact;
		BuildPattern(Token::Type::kw_enum)				= "enum"_exact;
		BuildPattern(Token::Type::kw_explicit)			= "explicit"_exact;
		BuildPattern(Token::Type::kw_export)			= "export"_exact;
		BuildPattern(Token::Type::kw_extern)			= "extern"_exact;
		BuildPattern(Token::Type::kw_false)				= "false"_exact;
		BuildPattern(Token::Type::kw_float)				= "float"_exact;
		BuildPattern(Token::Type::kw_for)				= "for"_exact;
		BuildPattern(Token::Type::kw_friend)			= "friend"_exact;
		BuildPattern(Token::Type::kw_goto)				= "goto"_exact;
		BuildPattern(Token::Type::kw_if)				= "if"_exact;
		BuildPattern(Token::Type::kw_inline)			= "inline"_exact;
		BuildPattern(Token::Type::kw_int)				= "int"_exact;
		BuildPattern(Token::Type::kw_long)				= "long"_exact;
		BuildPattern(Token::Type::kw_mutable)			= "mutable"_exact;
		BuildPattern(Token::Type::kw_namespace)			= "namespace"_exact;
		BuildPattern(Token::Type::kw_new)				= "new"_exact;
		BuildPattern(Token::Type::kw_noexcept)			= "noexcept"_exact;
		BuildPattern(Token::Type::kw_nullptr)			= "nullptr"_exact;
		BuildPattern(Token::Type::kw_operator)			= "operator"_exact;
		BuildPattern(Token::Type::kw_private)			= "private"_exact;
		BuildPattern(Token::Type::kw_protected)			= "protected"_exact;
		BuildPattern(Token::Type::kw_public)			= "public"_exact;
		BuildPattern(Token::Type::kw_register)			= "register"_exact;
		BuildPattern(Token::Type::kw_reinterpret_cast)	= "reinterpret_cast"_exact;
		BuildPattern(Token::Type::kw_requires)			= "requires"_exact;
		BuildPattern(Token::Type::kw_return)			= "return"_exact;
		BuildPattern(Token::Type::kw_short)				= "short"_exact;
		BuildPattern(Token::Type::kw_signed)			= "signed"_exact;
		BuildPattern(Token::Type::kw_sizeof)			= "sizeof"_exact;
		BuildPattern(Token::Type::kw_static)			= "static"_exact;
		BuildPattern(Token::Type::kw_static_assert)		= "static_assert"_exact;
		BuildPattern(Token::Type::kw_static_cast)		= "static_cast"_exact;
		BuildPattern(Token::Type::kw_struct)			= "struct"_exact;
		BuildPattern(Token::Type::kw_switch)			= "switch"_exact;
		BuildPattern(Token::Type::kw_template)			= "template"_exact;
		BuildPattern(Token::Type::kw_this)				= "this"_exact;
		BuildPattern(Token::Type::kw_thread_local)		= "thread_local"_exact;
		BuildPattern(Token::Type::kw_throw)				= "throw"_exact;
		BuildPattern(Token::Type::kw_true)				= "true"_exact;
		BuildPattern(Token::Type::kw_try)				= "try"_exact;
		BuildPattern(Token::Type::kw_typedef)			= "typedef"_exact;
		BuildPattern(Token::Type::kw_typeid)			= "typeid"_exact;
		BuildPattern(Token::Type::kw_typename)			= "typename"_exact;
		BuildPattern(Token::Type::kw_union)				= "union"_exact;
		BuildPattern(Token::Type::kw_unsigned)			= "unsigned"_exact;
		BuildPattern(Token::Type::kw_using)				= "using"_exact;
		BuildPattern(Token::Type::kw_virtual)			= "virtual"_exact;
		BuildPattern(Token::Type::kw_void)				= "void"_exact;
		BuildPattern(Token::Type::kw_volatile)			= "volatile"_exact;
		BuildPattern(Token::Type::kw_wchar_t)			= "wchar_t"_exact;
		BuildPattern(Token::Type::kw_while)				= "while"_exact;
	#pragma endregion
	


	#pragma region operators

		//Punctuation
		BuildPattern(Token::Type::L_Brace)				= '{'_c;
		BuildPattern(Token::Type::R_Brace)				= '}'_c;
		BuildPattern(Token::Type::L_Bracket)			= '['_c;
		BuildPattern(Token::Type::R_Bracket)			= ']'_c;
		BuildPattern(Token::Type::Hash)					= '#'_c;
		BuildPattern(Token::Type::HashHash)				= "##"_exact;
		BuildPattern(Token::Type::L_Paren)				= '('_c;
		BuildPattern(Token::Type::R_Paren)				= ')'_c;
		BuildPattern(Token::Type::Semicolon)			= ';'_c;
		BuildPattern(Token::Type::Colon)				= ':'_c;
		BuildPattern(Token::Type::Ellipsis)				= "..."_exact;
		//						  kw_new				= "new"_exact;		//also a keyword
		//						  kw_delete				= "delete"_exact;	//also a keyword
		BuildPattern(Token::Type::Question)				= '?'_c;
		BuildPattern(Token::Type::Colon_colon)			= "::"_exact;
		BuildPattern(Token::Type::Dot)					= '.'_c;
		BuildPattern(Token::Type::DotStar)				= ".*"_exact;
		BuildPattern(Token::Type::Arrow)				= "->"_exact;
		BuildPattern(Token::Type::ArrowStar)			= "->*"_exact;
		BuildPattern(Token::Type::Comma)				= ','_c;

 


		//operators
		BuildPattern(Token::Type::Complement)			= '~'_c;
		BuildPattern(Token::Type::Not)					= '!'_c;
		BuildPattern(Token::Type::Plus)					= '+'_c;
		BuildPattern(Token::Type::Minus)				= '-'_c;
		BuildPattern(Token::Type::Star)					= '*'_c;
		BuildPattern(Token::Type::Div)					= '/'_c;
		BuildPattern(Token::Type::Mod)					= '%'_c;
		BuildPattern(Token::Type::Xor)					= '^'_c;
		BuildPattern(Token::Type::BitAnd)				= '&'_c;
		BuildPattern(Token::Type::BitOr)				= '|'_c;
		BuildPattern(Token::Type::Equal)				= '='_c;
		BuildPattern(Token::Type::PlusEqual)			= "+="_exact;
		BuildPattern(Token::Type::MinusEqual)			= "-="_exact;
		BuildPattern(Token::Type::StarEqual)			= "*="_exact;
		BuildPattern(Token::Type::DivEqual)				= "/="_exact;
		BuildPattern(Token::Type::ModEqual)				= "%="_exact;
		BuildPattern(Token::Type::ModEqual)				= "%="_exact;
		BuildPattern(Token::Type::XorEqual)				= "^="_exact;
		BuildPattern(Token::Type::BitAndEqual)			= "&="_exact;
		BuildPattern(Token::Type::BitOrEqual)			= "|="_exact;
		BuildPattern(Token::Type::EqualEqual)			= "=="_exact;
		BuildPattern(Token::Type::NotEquals)			= "!="_exact;
		BuildPattern(Token::Type::Less)					= '<'_c;
		BuildPattern(Token::Type::Greater)				= '>'_c;
		BuildPattern(Token::Type::LessEqual)			= "<="_exact;
		BuildPattern(Token::Type::GreaterEqual)			= ">="_exact;
		BuildPattern(Token::Type::LessEqualGreater)		= "<=>"_exact;
		BuildPattern(Token::Type::And)					= "&&"_exact;
		BuildPattern(Token::Type::Or)					= "||"_exact;
		BuildPattern(Token::Type::LessLess)				= "<<"_exact;
		BuildPattern(Token::Type::GreaterGreater)		= ">>"_exact;
		BuildPattern(Token::Type::LessLessEqual)		= "<<="_exact;
		BuildPattern(Token::Type::GreaterGreaterEqual)	= ">>="_exact;
		BuildPattern(Token::Type::PlusPlus)				= "++"_exact;
		BuildPattern(Token::Type::MinusMinus)			= "--"_exact;


		//Alternate forms
		BuildPattern(Token::Type::And)					= "and"_exact;
		BuildPattern(Token::Type::BitAndEqual)			= "and_eq"_exact;
		BuildPattern(Token::Type::BitAnd)				= "bitand"_exact;
		BuildPattern(Token::Type::BitOr)				= "bitor"_exact;
		BuildPattern(Token::Type::Complement)			= "compl"_exact;
		BuildPattern(Token::Type::Not)					= "not"_exact;
		BuildPattern(Token::Type::NotEquals)			= "not_eq"_exact;
		BuildPattern(Token::Type::Or)					= "or"_exact;
		BuildPattern(Token::Type::BitOrEqual)			= "or_eq"_exact;
		BuildPattern(Token::Type::Xor)					= "xor"_exact;
		BuildPattern(Token::Type::XorEqual)				= "xor_eq"_exact;

	#pragma endregion


		BuildPattern(Token::Type::Identifier)			= nondigit
														and Optionally(Repeat(digit or nondigit));

		BuildPattern(Token::Type::WhiteSpace)			= Repeat(" \t\r\b"_any);

		BuildPattern(Token::Type::Integer_literal)		=	(	("0x"_nocase	and Optionally(Repeat(Optionally('\''_c) and hexadecimal_digit)))
															or	('0'_c			and Optionally(Repeat(Optionally('\''_c) and octal_digit)))
															or	("0b"_nocase	and Optionally(Repeat(Optionally('\''_c) and "01"_any)))
															or	(nonzero_digit	and Optionally(Repeat(Optionally('\''_c) and "0123456789"_any))))
														and Optionally(
																("u"_nocase		and Optionally("l"_nocase or "ll"_nocase))
															or	("l"_nocase		and Optionally("u"_nocase))
															or	("ll"_nocase	and Optionally("u"_nocase)));

		BuildPattern(Token::Type::String_literal)		=	Optionally(
																"u8"_exact 
															or	'u'_c 
															or	'U'_c 
															or	'L'_c)
														and	'"'_c
														and Optionally(Repeat(
																"\\\"\n"_notof
															or	escape_sequence))
														and '"'_c;

		BuildPattern(Token::Type::Char_literal)			=	Optionally(
																"u8"_exact
															or	'u'_c
															or	'U'_c
															or	'L'_c)
														and '\''_c
														and Repeat(
																"\\\'\n"_notof
															or	escape_sequence)
														and '\''_c;
		BuildPattern(Token::Type::Floating_literal)		=	(Optionally(digitSequence)
																and '.'_c
																and digitSequence
																and Optionally("eE"_any and Optionally("+-"_any) and digitSequence)
																and "flFL"_any)
														or	(Optionally(hexadecimalDigitSequence)
																and '.'_c
																and hexadecimalDigitSequence
																and Optionally("eE"_any and Optionally("+-"_any) and hexadecimalDigitSequence)
																and "flFL"_any);
	}

	TokenMatcher::PatternBuilder::PatternBuilder(Token::Type aType, std::vector<std::shared_ptr<RootPattern>>& aRootPatternCollection)
		: myType(aType)
		, myRootPatternCollecton(aRootPatternCollection)
	{

	}

	TokenMatcher::PatternBuilder::~PatternBuilder()
	{
		if (myPatterns.size() == 1)
		{
			myRootPatternCollecton.push_back(std::make_shared<RootPattern>(myPatterns[0], myType));
		}
		else
		{
			myRootPatternCollecton.push_back(std::make_shared<RootPattern>(std::shared_ptr<Pattern>(new patterns::ComboPattern(myPatterns)), myType));
		}
	}

	TokenMatcher::PatternBuilder& TokenMatcher::PatternBuilder::operator=(std::shared_ptr<Pattern> aPattern)
	{
		myPatterns.push_back(aPattern);
		return *this;
	}

	TokenMatcher::PatternBuilder& TokenMatcher::PatternBuilder::operator and(std::shared_ptr<Pattern> aPattern)
	{
		myPatterns.push_back(aPattern);
		return *this;
	}

} // tokenizer