
#include <optional>
#include <filesystem>
#include <variant>
#include <functional>
#include <ranges>

#include <iostream>

#include "common/CompilerContext.h"
#include "common/IteratorRange.h"
#include "precompiler/precompiler.h"
#include "tokenizer/tokenizer.h"

Precompiler::Context Precompiler::myContext;

void Precompiler::ResetContext()
{
	myContext = Context();
}

void Precompiler::ConsumeLine(FileContext& aFileContext, TokenStream& aOutTokens, const std::vector<Token>& aTokens)
{

	using iterator = std::vector<Token>::const_iterator;

	auto getNextNotWhitespace = [&aTokens](iterator aIt) -> std::optional<iterator> {
		while (aIt != std::end(aTokens))
		{
			if (aIt->myType != Token::Type::WhiteSpace)
				return aIt;
			aIt++;
		}
		return {};
	};

	if (std::optional<iterator> startIt = getNextNotWhitespace(begin(aTokens)))
	{
		iterator& start = *startIt;
		IfState& currentState = aFileContext.myIfStack.top();

		switch (start->myType)
		{
		case Token::Type::Include_directive:
			if (currentState == IfState::Active)
				IncludeFile(aOutTokens, aTokens, start);
			return;
		case Token::Type::Hash:
			if (std::optional<iterator> identifierIt = getNextNotWhitespace(start + 1))
			{
				iterator& identifier = *identifierIt;
				switch (identifier->myType)
				{
				case Token::Type::kw_if:
					aFileContext.myIfStack.push(EvaluateExpression(IteratorRange(identifier + 1, std::end(aTokens))));
					return;
				case Token::Type::kw_else:
					if (currentState == IfState::Active)
						currentState = IfState::HasBeenActive;
					else if (currentState == IfState::Inactive)
						currentState = IfState::Active;
					return;
				case Token::Type::Identifier:
					if (identifier->myRawText == "elif")
					{
						if (currentState == IfState::Active)
							currentState = IfState::HasBeenActive;
						else if (currentState == IfState::Inactive)
							currentState = EvaluateExpression(IteratorRange(identifier + 1, std::end(aTokens)));
					}
					else if (identifier->myRawText == "endif")
					{
						aFileContext.myIfStack.pop();
						if (aFileContext.myIfStack.empty())
							CompilerContext::EmitError("Unmatched endif", *identifier);
					}
					else if (identifier->myRawText == "ifdef")
					{
						if (std::optional<iterator> ifdefIt = getNextNotWhitespace(identifier + 1))
						{
							iterator& ifdef = *ifdefIt;
							aFileContext.myIfStack.push(myContext.myMacros.count(ifdef->myRawText) != 0 ? IfState::Active : IfState::Inactive);
						}
						else
						{
							CompilerContext::EmitError("expected an identifier after #ifdef", *identifier);
						}
					}
					else if (identifier->myRawText == "ifndef")
					{
						if (std::optional<iterator> ifdefIt = getNextNotWhitespace(identifier + 1))
						{
							iterator& ifdef = *ifdefIt;
							aFileContext.myIfStack.push(myContext.myMacros.count(ifdef->myRawText) != 0 ? IfState::Inactive : IfState::Active);
						}
						else
						{
							CompilerContext::EmitError("expected an identifier after #ifdef", *identifier);
						}
					}
					else if (identifier->myRawText == "define")
					{
						Define(IteratorRange(identifier + 1, std::end(aTokens)));
						return;
					}
					return;


				default:
					CompilerContext::EmitError("Malformed preprocessor directive, expected identifier after #", *identifier);
					return;
				}
			}
			else
			{
				CompilerContext::EmitError("Malformed preprocessor directive, expected identifier after #", *start);
			}
			return;
		default:
			if (currentState == IfState::Active)
				aOutTokens << TranslateTokenRange(aTokens);
			return;
		}
	}
}

void Precompiler::IncludeFile(TokenStream& aOutTokens, const std::vector<Token>& aTokens, std::vector<Token>::const_iterator aIncludeIt)
{
	using iterator = std::vector<Token>::const_iterator;

	auto getNextNotWhitespace = [&aTokens](iterator aIt) -> std::optional<iterator> {
		while (aIt != std::end(aTokens))
		{
			if (aIt->myType != Token::Type::WhiteSpace)
				return aIt;
			aIt++;
		}
		return {};
	};

	if (std::optional<iterator> expectedPath = getNextNotWhitespace(aIncludeIt + 1))
	{
		iterator path = *expectedPath;
		if (path->myType == Token::Type::Header_name)
		{
			std::string_view rawPath(path->myRawText.begin() + 1, path->myRawText.end() - 1); // trim quotes and angle brackets
			bool expandedSearch = path->myRawText[0] == '<';
			if (std::optional<std::filesystem::path> expectedFilePath = CompilerContext::FindFile(rawPath, expandedSearch))
			{
				aOutTokens << Tokenize(*expectedFilePath);
			}
			else
			{
				CompilerContext::EmitError("Malformed include directive, unable to find file: " + std::string(rawPath), *path);
			}

			if (std::optional<iterator> expectedNewLine = getNextNotWhitespace(path + 1))
			{
				iterator newLine = *expectedNewLine;
				if (newLine->myType != Token::Type::NewLine)
				{
					CompilerContext::EmitError("Malformed include directive, expected newline after header_name", *path);
					return;
				}
				return;
			}
		}
		else
		{
			CompilerContext::EmitError("Malformed include directive, expected header name in the form \"header name\" or <header name>", *path);
		}
	}
	else
	{
		CompilerContext::EmitError("Malformed include directive, unexpected end of line", *aIncludeIt);
	}
}

Precompiler::FileContext::FileContext()
{
	myIfStack.push(IfState::Active);
}

Precompiler::FileContext::~FileContext()
{
	if (myIfStack.size() != 1)
		CompilerContext::EmitError("Unmatched #if directive at eof", 0, 0, 0);
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

	void AddValue(PreprocessorNumber aValue, std::vector<Token>& aPendingOperators, std::vector<PreprocessorNumber>& aValues)
	{
		{
			if (aPendingOperators.size() == aValues.size())
			{
				aValues.push_back(aValue);
				return;
			}

			Token op = aPendingOperators.back();
			aPendingOperators.pop_back();

			for (size_t i = 0; i < unaryOperators.size(); i++)
			{
				if (op.myType == unaryOperators[i])
				{
					if (CompilerContext::GetFlag("verbose") == "precompiler_math")
						std::cout << "performed unary transform " << Token::TypeToString(op.myType) << " on " << aValue << "\n";

					AddValue(unaryOperatorFunctors[i](aValue), aPendingOperators, aValues);
					return;
				}
			}

			CompilerContext::EmitError("Only unary operators can can be used with a single operand", op);
		}
	}
}


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
				PreprocessorNumber first = values[i];
				PreprocessorNumber second = values[i + 1];
				PreprocessorNumber result = precompiler_internal_math::operatorFunctors[opIndex](first, second);

				if (CompilerContext::GetFlag("verbose") == "precompiler_math")
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
			CompilerContext::EmitError("Unmatched parenthesis", CompilerContext::npos);
	}

	return it;
}


template<std::ranges::contiguous_range TokenCollection>
inline void Precompiler::Define(TokenCollection aTokens)
{
	Macro macro(aTokens | std::ranges::views::filter([](Token aToken) { return aToken.myType != Token::Type::WhiteSpace && aToken.myType != Token::Type::NewLine; }));
	if(macro.myIdentifier.empty())
		return;

	myContext.myMacros.emplace(macro.myIdentifier, macro);
}

template<std::ranges::input_range TokenCollection>
inline Precompiler::Macro::Macro(TokenCollection aRange)
{
	auto it = std::ranges::begin(aRange);
	auto end = std::ranges::end(aRange);
	
	if (it == end)
	{
		CompilerContext::EmitError("Expected an identifier", CompilerContext::npos);
		return;
	}

	if (!it->IsTextToken())
	{
		CompilerContext::EmitError("Expected an identifier", *it);
		return;
	}

	myIdentifier = it->myRawText;

	it++;
	if (it == end)
		return;

	std::vector<std::string_view> arguments;

	if (it->myType == Token::Type::L_Paren)
	{
		it++;
		while (it != end)
		{
			if (!it->IsTextToken())
			{
				CompilerContext::EmitError("Expected an identifier", *it);
				return;
			}
			arguments.emplace_back(it->myRawText);
			it++;
			if (it == end)
			{
				CompilerContext::EmitError("Expected a ',' or ')'", CompilerContext::npos);
				return;
			}

			if (it->myType == Token::Type::R_Paren)
			{
				it++;
				break;
			}

			if (it->myType != Token::Type::Comma)
			{
				CompilerContext::EmitError("Expected a ',' or ')'", *it);
				return;
			}
		}
	}

}

template<std::forward_iterator IteratorType>
inline std::vector<Token> Precompiler::Macro::Evaluate(IteratorType& aInOutBegin, const IteratorType& aEnd)
{
	return std::vector<Token>();
}
