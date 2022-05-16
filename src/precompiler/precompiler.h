
#include "tokenizer/tokenStream.h"

#include <vector>
#include <ranges>
#include <unordered_map>
#include <functional>
#include <iostream>

#include "common/RangeCollapse.h"

using PreprocessorNumber = long long;

class Precompiler
{
	enum class IfState
	{
		Active,
		Inactive,
		HasBeenActive
	};

public:
	class FileContext
	{
	public:
		FileContext();
		~FileContext();

	private:
		friend Precompiler;
		std::stack<IfState> myIfStack;
	};

	static void ResetContext();
	static void ConsumeLine(FileContext& aFileContext, TokenStream& aOutTokens, const std::vector<Token>& aTokens);

private:

	struct Macro
	{
		template<std::ranges::contiguous_range TokenCollection>
		Macro(TokenCollection aRange);

		template<std::forward_iterator IteratorType>
		std::vector<Token> Evaluate(IteratorType& aInOutBegin, const IteratorType& aEnd);

		struct Component
		{
			~Component()
			{
				switch (myType)
				{
				case Precompiler::Macro::Component::Type::Token:
					myToken.~Token();
					break;
				case Precompiler::Macro::Component::Type::Argument:
				case Precompiler::Macro::Component::Type::VariadicExpansion:
					break;
				}
			}

			enum class Type
			{
				Token,
				Argument,
				VariadicExpansion
			};

			union
			{
				Token myToken;
				size_t myArgumentIndex;
			};
			Type myType;
		};

		bool myHasVariadic = false;
		size_t myArguments;
		std::vector<Token> myTokens;
	};

	struct Context
	{
		std::unordered_map<std::string, Macro> myMacros;
	};
	
	template<std::ranges::contiguous_range TokenCollection>
	static std::vector<Token> TranslateTokenRange(TokenCollection aTokens);
	
	static void IncludeFile(TokenStream& aOutTokens, const std::vector<Token>& aTokens, std::vector<Token>::const_iterator aIncludeIt);

	template<std::ranges::contiguous_range TokenCollection>
	static IfState EvaluateExpression(TokenCollection aTokens);
	
	template<std::ranges::contiguous_range TokenCollection>
	static PreprocessorNumber EvalutateSequence(TokenCollection aTokens);
	
	template<std::forward_iterator IteratorType>
	static IteratorType FindMatchingEndParen(IteratorType aBegin, IteratorType aEnd);

	template<std::ranges::contiguous_range TokenCollection>
	static void Define(TokenCollection aTokens);


	static Context myContext;
};

template<std::ranges::contiguous_range TokenCollection>
inline std::vector<Token> Precompiler::TranslateTokenRange(TokenCollection aTokens)
{
	TokenStream stream;


	auto it = std::ranges::begin(aTokens);
	auto end = std::ranges::end(aTokens);
	while (it != end)
	{
		const Token& tok = *it;

		decltype(Context::myMacros)::iterator potentialMacro = myContext.myMacros.find(tok.myRawText);
		if (potentialMacro != std::end(myContext.myMacros))
		{
			it++;
			stream << potentialMacro->second.Evaluate(it, end);
			continue;
		}

		if (!tok.IsPrepoccessorSpecific())
			stream << tok;
		it++;
	}

	return std::move(stream).Get();
}

template<std::ranges::contiguous_range TokenCollection>
inline Precompiler::IfState Precompiler::EvaluateExpression(TokenCollection aTokens)
{
	std::vector<Token> buffer = TranslateTokenRange(aTokens);

	return EvalutateSequence(buffer) == 0 ? IfState::Inactive : IfState::Active;
}


namespace precompiler_internal_math
{
	const std::vector<Token::Type> unaryOperators = 
	{
		Token::Type::Not,
		Token::Type::Complement,
		Token::Type::Plus,
		Token::Type::Minus,
	};

	const std::vector<std::function<PreprocessorNumber(PreprocessorNumber)>> unaryOperatorFunctors = 
	{
		[](PreprocessorNumber aFirst) { return !aFirst; },
		[](PreprocessorNumber aFirst) { return ~aFirst; },
		[](PreprocessorNumber aFirst) { return +aFirst; },
		[](PreprocessorNumber aFirst) { return -aFirst; }
	};

	const std::vector<Token::Type> operators = 
	{
		Token::Type::Div,
		Token::Type::Star,
		Token::Type::Mod,

		Token::Type::Plus,
		Token::Type::Minus,

		Token::Type::BitAnd,

		Token::Type::Xor,

		Token::Type::BitOr,

		Token::Type::And,

		Token::Type::Or
	};

	const std::vector<std::function<PreprocessorNumber(PreprocessorNumber, PreprocessorNumber)>> operatorFunctors = 
	{
		[](PreprocessorNumber aFirst, PreprocessorNumber aSecond) { return aFirst / aSecond; },
		[](PreprocessorNumber aFirst, PreprocessorNumber aSecond) { return aFirst * aSecond; },
		[](PreprocessorNumber aFirst, PreprocessorNumber aSecond) { return aFirst % aSecond; },

		[](PreprocessorNumber aFirst, PreprocessorNumber aSecond) { return aFirst + aSecond; },
		[](PreprocessorNumber aFirst, PreprocessorNumber aSecond) { return aFirst - aSecond; },

		[](PreprocessorNumber aFirst, PreprocessorNumber aSecond) { return aFirst & aSecond; },

		[](PreprocessorNumber aFirst, PreprocessorNumber aSecond) { return aFirst ^ aSecond; },

		[](PreprocessorNumber aFirst, PreprocessorNumber aSecond) { return aFirst | aSecond; },

		[](PreprocessorNumber aFirst, PreprocessorNumber aSecond) { return aFirst && aSecond; },

		[](PreprocessorNumber aFirst, PreprocessorNumber aSecond) { return aFirst || aSecond; },
	};

	void AddValue(PreprocessorNumber aValue, std::vector<Token>& aPendingOperators, std::vector<PreprocessorNumber>& aValues);
}

template<std::ranges::contiguous_range TokenCollection>
inline PreprocessorNumber Precompiler::EvalutateSequence(TokenCollection aTokens)
{
	if (std::ranges::size(aTokens) == 0)
	{
		CompilerContext::EmitError("Expected an expression", 0);
		return 0;
	}

	auto begin = std::ranges::begin(aTokens);
	auto end = std::ranges::end(aTokens);
	auto it = begin;

	std::vector<Token> pendingOperators;
	std::vector<PreprocessorNumber> values;

	while (it != end)
	{
		const Token& tok = *it;
		switch (tok.myType)
		{
			case Token::Type::Integer:
				precompiler_internal_math::AddValue(tok.EvaluateIntegral(), pendingOperators, values);
				break;
			case Token::Type::L_Paren: {
				it++;
				auto matching = FindMatchingEndParen(it, end) - 1;
				PreprocessorNumber number = EvalutateSequence(IteratorRange(it, matching));
				precompiler_internal_math::AddValue(number, pendingOperators, values);
				it = matching;
			}
				break;
			case Token::Type::WhiteSpace:
			case Token::Type::NewLine:
				break;
			default:
				if (values.empty()) //TODO: this should be when equal not when empty
				{
					if (std::find(std::begin(precompiler_internal_math::unaryOperators), std::end(precompiler_internal_math::unaryOperators), tok.myType) != std::end(precompiler_internal_math::unaryOperators))
						pendingOperators.push_back(tok);
					else
						CompilerContext::EmitError("Unexpected token", tok);
				}
				else
				{
					if (std::find(std::begin(precompiler_internal_math::operators), std::end(precompiler_internal_math::operators), tok.myType) != std::end(precompiler_internal_math::operators))
						pendingOperators.push_back(tok);
					else
						CompilerContext::EmitError("Unexpected token", tok);
				}

				break;
		}
		it++;
	}

	for (size_t opIndex = 0; opIndex < precompiler_internal_math::operators.size(); opIndex++)
	{
		const Token::Type& op = precompiler_internal_math::operators[opIndex];

		for (size_t i = 0; i < pendingOperators.size(); i++)
		{
			if (pendingOperators[i].myType == op)
			{
				if (values.size() < i + 1)
				{
					CompilerContext::EmitError("Incomplete expression", 0);
					return 0;
				}
				PreprocessorNumber first  = values[i];
				PreprocessorNumber second = values[i + 1];
				PreprocessorNumber result = precompiler_internal_math::operatorFunctors[opIndex](first, second);

				if(CompilerContext::GetFlag("verbose") == "precompiler_math")
					std::cout << "did " << Token::TypeToString(op) << " [" << opIndex << "] on " << first << " and " << second << " resulting in " << result << "\n";

				values.erase(std::begin(values) + i, std::begin(values) + 2 + i);
				values.insert(std::begin(values) + i, result);

				pendingOperators.erase(std::begin(pendingOperators) + i);
				i--;
			}
		}
	}

	if (values.empty())
	{
		CompilerContext::EmitError("Expected an expression", *it);
		return 0;
	}

	if (values.size() > 1 && CompilerContext::IsWarningEnabled("if_contamitaion"))
	{
		CompilerContext::EmitWarning("Expected single expression", *begin);
	}

	std::cout << "\n";
	return values[0];
}

template<std::forward_iterator IteratorType>
inline IteratorType Precompiler::FindMatchingEndParen(IteratorType aBegin, IteratorType aEnd)
{
	size_t depth = 1;
	IteratorType it = aBegin;
	while (it != aEnd && depth > 0)
	{
		switch (it->myType)
		{
			case Token::Type::L_Paren:
				depth++;
				break;

			case Token::Type::R_Paren:
				depth--;
				break;
		}
		it++;
	}
	if (depth > 0)
	{
		if (aBegin != aEnd)
			CompilerContext::EmitError("Unmatched parenthesis", *(aEnd - 1));
		else
			CompilerContext::EmitError("Unmatched parenthesis", 0);
	}

	return it;
}

template<std::ranges::contiguous_range TokenCollection>
inline void Precompiler::Define(TokenCollection aTokens)
{
	Macro macro(std::ranges::views::filter([](Token aToken) { return aToken.myType != Token::Type::WhiteSpace && aToken.myType != Token::Type::NewLine; }));

}

template<std::ranges::contiguous_range TokenCollection>
inline Precompiler::Macro::Macro(TokenCollection aRange)
{
	if (std::ranges::size(aRange) == 0)
	{
		CompilerContext::EmitError("Expected an identifier", 0);
		myType = Type::
		return;
	}
}

template<std::forward_iterator IteratorType>
inline std::vector<Token> Precompiler::Macro::Evaluate(IteratorType& aInOutBegin, const IteratorType& aEnd)
{
	return std::vector<Token>();
}
