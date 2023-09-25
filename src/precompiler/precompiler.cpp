
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

void Precompiler::ConsumeLine(FileContext& aFileContext, tokenizer::TokenStream& aOutTokens, const std::vector<tokenizer::Token>& aTokens)
{

	using iterator = std::vector<tokenizer::Token>::const_iterator;

	auto getNextNotWhitespace = [&aTokens](iterator aIt) -> std::optional<iterator> {
		while (aIt != std::end(aTokens))
		{
			if (aIt->myType != tokenizer::Token::Type::WhiteSpace)
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
		case tokenizer::Token::Type::Include_directive:
			if (currentState == IfState::Active)
				IncludeFile(aOutTokens, aTokens, start);
			return;
		case tokenizer::Token::Type::Hash:
			if (std::optional<iterator> identifierIt = getNextNotWhitespace(start + 1))
			{
				iterator& identifier = *identifierIt;
				switch (identifier->myType)
				{
				case tokenizer::Token::Type::kw_if:
					aFileContext.myIfStack.push(EvaluateExpression(IteratorRange(identifier + 1, std::end(aTokens))));
					return;
				case tokenizer::Token::Type::kw_else:
					if (currentState == IfState::Active)
						currentState = IfState::HasBeenActive;
					else if (currentState == IfState::Inactive)
						currentState = IfState::Active;
					return;
				case tokenizer::Token::Type::Identifier:
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
						if (currentState == IfState::Active)
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

void Precompiler::IncludeFile(tokenizer::TokenStream& aOutTokens, const std::vector<tokenizer::Token>& aTokens, std::vector<tokenizer::Token>::const_iterator aIncludeIt)
{
	using iterator = std::vector<tokenizer::Token>::const_iterator;

	auto getNextNotWhitespace = [&aTokens](iterator aIt) -> std::optional<iterator> {
		while (aIt != std::end(aTokens))
		{
			if (aIt->myType != tokenizer::Token::Type::WhiteSpace)
				return aIt;
			aIt++;
		}
		return {};
	};

	if (std::optional<iterator> expectedPath = getNextNotWhitespace(aIncludeIt + 1))
	{
		iterator path = *expectedPath;
		if (path->myType == tokenizer::Token::Type::Header_name)
		{
			std::string_view rawPath(path->myRawText.begin() + 1, path->myRawText.end() - 1); // trim quotes and angle brackets
			bool expandedSearch = path->myRawText[0] == '<';
			if (std::optional<std::filesystem::path> expectedFilePath = CompilerContext::FindFile(rawPath, expandedSearch))
			{
				aOutTokens << tokenizer::Tokenize(*expectedFilePath);
			}
			else
			{
				CompilerContext::EmitError("Malformed include directive, unable to find file: " + std::string(rawPath), *path);
			}

			if (std::optional<iterator> expectedNewLine = getNextNotWhitespace(path + 1))
			{
				iterator newLine = *expectedNewLine;
				if (newLine->myType != tokenizer::Token::Type::NewLine)
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
		CompilerContext::EmitError("Unmatched #if directive at eof");
}

namespace precompiler_internal_math
{

	const std::vector<tokenizer::Token::Type> unaryOperators =
	{
		tokenizer::Token::Type::Not,
		tokenizer::Token::Type::Complement,
		tokenizer::Token::Type::Plus,
		tokenizer::Token::Type::Minus,
	};

	const std::vector<std::function<PreprocessorNumber(PreprocessorNumber)>> unaryOperatorFunctors =
	{
		[](PreprocessorNumber aFirst) { return !aFirst; },
		[](PreprocessorNumber aFirst) { return ~aFirst; },
		[](PreprocessorNumber aFirst) { return +aFirst; },
		[](PreprocessorNumber aFirst) { return -aFirst; }
	};

	const std::vector<tokenizer::Token::Type> operators =
	{
		tokenizer::Token::Type::Div,
		tokenizer::Token::Type::Star,
		tokenizer::Token::Type::Mod,

		tokenizer::Token::Type::Plus,
		tokenizer::Token::Type::Minus,

		tokenizer::Token::Type::BitAnd,

		tokenizer::Token::Type::Xor,

		tokenizer::Token::Type::BitOr,

		tokenizer::Token::Type::And,

		tokenizer::Token::Type::Or
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

	void AddValue(PreprocessorNumber aValue, std::vector<tokenizer::Token>& aPendingOperators, std::vector<PreprocessorNumber>& aValues)
	{
		{
			if (aPendingOperators.size() == aValues.size())
			{
				aValues.push_back(aValue);
				return;
			}

			tokenizer::Token op = aPendingOperators.back();
			aPendingOperators.pop_back();

			for (size_t i = 0; i < unaryOperators.size(); i++)
			{
				if (op.myType == unaryOperators[i])
				{
					if (CompilerContext::GetFlag("verbose") == "precompiler_math")
						std::cout << "performed unary transform " << tokenizer::Token::TypeToString(op.myType) << " on " << aValue << "\n";

					AddValue(unaryOperatorFunctors[i](aValue), aPendingOperators, aValues);
					return;
				}
			}

			CompilerContext::EmitError("Only unary operators can can be used with a single operand", op);
		}
	}
}


template<std::ranges::contiguous_range TokenCollection>
inline std::vector<tokenizer::Token> Precompiler::TranslateTokenRange(TokenCollection aTokens)
{
	tokenizer::TokenStream stream;

	auto filtered = aTokens | tokenizer::token_helpers::IsNotWhitespace;
	auto it = std::ranges::begin(aTokens);
	auto end = std::ranges::end(aTokens);
	while (it != end)
	{
		const tokenizer::Token& tok = *it;

		decltype(Context::myMacros)::iterator potentialMacro = myContext.myMacros.find(tok.myRawText);
		if (potentialMacro != std::end(myContext.myMacros))
		{
			it++;
			if (std::optional<std::vector<tokenizer::Token>> result = potentialMacro->second.Evaluate(it, end))
			{
				stream << *result;
				continue;
			}
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
	std::vector<tokenizer::Token> buffer = TranslateTokenRange(aTokens);

	return EvalutateSequence(buffer) == 0 ? IfState::Inactive : IfState::Active;
}

template<std::ranges::contiguous_range TokenCollection>
inline PreprocessorNumber Precompiler::EvalutateSequence(TokenCollection aTokens)
{
	if (std::ranges::size(aTokens) == 0)
	{
		CompilerContext::EmitError("Expected an expression");
		return 0;
	}

	auto begin = std::ranges::begin(aTokens);
	auto end = std::ranges::end(aTokens);
	auto it = begin;

	std::vector<tokenizer::Token> pendingOperators;
	std::vector<PreprocessorNumber> values;

	while (it != end)
	{
		const tokenizer::Token& tok = *it;
		switch (tok.myType)
		{
		case tokenizer::Token::Type::Integer:
			precompiler_internal_math::AddValue(tok.EvaluateIntegral(), pendingOperators, values);
			break;
		case tokenizer::Token::Type::L_Paren: {
			it++;
			auto matching = FindMatchingEndParen(it, end) - 1;
			PreprocessorNumber number = EvalutateSequence(IteratorRange(it, matching));
			precompiler_internal_math::AddValue(number, pendingOperators, values);
			it = matching;
		}
								 break;
		case tokenizer::Token::Type::WhiteSpace:
		case tokenizer::Token::Type::NewLine:
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
		const tokenizer::Token::Type& op = precompiler_internal_math::operators[opIndex];

		for (size_t i = 0; i < pendingOperators.size(); i++)
		{
			if (pendingOperators[i].myType == op)
			{
				if (values.size() < i + 1)
				{
					CompilerContext::EmitError("Incomplete expression");
					return 0;
				}
				PreprocessorNumber first = values[i];
				PreprocessorNumber second = values[i + 1];
				PreprocessorNumber result = precompiler_internal_math::operatorFunctors[opIndex](first, second);

				if (CompilerContext::GetFlag("verbose") == "precompiler_math")
					std::cout << "did " << tokenizer::Token::TypeToString(op) << " [" << opIndex << "] on " << first << " and " << second << " resulting in " << result << "\n";

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
		case tokenizer::Token::Type::L_Paren:
			depth++;
			break;

		case tokenizer::Token::Type::R_Paren:
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
			CompilerContext::EmitError("Unmatched parenthesis", CompilerContext::GetCurrentFile(),  CompilerContext::npos);
	}

	return it;
}


template<std::ranges::contiguous_range TokenCollection>
inline void Precompiler::Define(TokenCollection aTokens)
{
	Macro macro(aTokens | tokenizer::token_helpers::IsNotWhitespace);
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
		CompilerContext::EmitError("Expected an identifier",  CompilerContext::GetCurrentFile(),  CompilerContext::npos);
		return;
	}

	if (!it->IsTextToken())
	{
		CompilerContext::EmitError("Expected an identifier", *it);
		return;
	}

	myIdentifier = it->myRawText;

	if(CompilerContext::GetFlag("verbose") == "macros")
		std::cout << "new macro added [" << myIdentifier << "] \n";

	it++;
	if (it == end)
		return;

	std::vector<std::string_view> arguments;

	if (it->myType == tokenizer::Token::Type::L_Paren)
	{
		it++;
		while (it != end)
		{
			if(it->myType == tokenizer::Token::Type::Ellipsis)
			{
				myHasVariadic = true;
				it++;
				if (it == end)
				{
					CompilerContext::EmitError("Expected a ')'", CompilerContext::GetCurrentFile(), CompilerContext::npos);
					return;
				}

				if(it->myType != tokenizer::Token::Type::R_Paren)
				{
					CompilerContext::EmitError("Expected a ')'", *it);
					return;
				}
				it++;
				break;
			}

			if (!it->IsTextToken())
			{
				CompilerContext::EmitError("Expected an identifier", *it);
				return;
			}
			arguments.emplace_back(it->myRawText);
			it++;
			if (it == end)
			{
				CompilerContext::EmitError("Expected a ',' or ')'", CompilerContext::GetCurrentFile(),  CompilerContext::npos);
				return;
			}

			if (it->myType == tokenizer::Token::Type::R_Paren)
			{
				it++;
				break;
			}

			if (it->myType != tokenizer::Token::Type::Comma)
			{
				CompilerContext::EmitError("Expected a ',' or ')'", *it);
				return;
			}
			it++;
		}
	}

	myArguments = arguments.size();

	if (CompilerContext::GetFlag("verbose") == "macros")
	{
		if(!arguments.empty())
		{
			std::cout << "arguments: ";
			size_t index = 0;
			for(const std::string_view& view : arguments)
				std::cout << index++ << ":" << view << (index != arguments.size() ? ", " : (myHasVariadic ? " ... \n" : "\n"));
		}
	}

	if (CompilerContext::GetFlag("verbose") == "macros")
		std::cout << "Result:";

	while(it != end)
	{
		if(myHasVariadic)
		{
			if(it->myRawText == "__VA_ARGS__")
			{
				if (CompilerContext::GetFlag("verbose") == "macros")
					std::cout << " [Variadic arguments]";

				Component comp;
				comp.myType = Component::Type::VariadicExpansion;
				myComponents.push_back(comp);
				it++;

				continue;
			}
		}

		if(it->IsTextToken())
		{
			bool found = false;
			for(size_t i = 0; i < arguments.size(); i++)
			{
				if (arguments[i] == it->myRawText)
				{
					if (CompilerContext::GetFlag("verbose") == "macros")
						std::cout << " {" << i << "}";

					Component comp;
					comp.myType = Component::Type::Argument;
					comp.myArgumentIndex = i;
					myComponents.push_back(comp);
					it++;

					found = true;
					break;
				}
			}
			if (found)
				continue;
		}

		if (CompilerContext::GetFlag("verbose") == "macros")
			std::cout << " " << it->myRawText;

		Component comp;
		comp.myType = Component::Type::Token;
		comp.myToken = *it;
		myComponents.push_back(comp);
		it++;
	}

	if (CompilerContext::GetFlag("verbose") == "macros")
		std::cout << "\n";
}

template<std::forward_iterator IteratorType>
inline std::optional<std::vector<tokenizer::Token>> Precompiler::Macro::Evaluate(IteratorType& aInOutBegin, const IteratorType& aEnd)
{
	std::vector<tokenizer::Token> out;
	
	if(myArguments != 0 || myHasVariadic)
	{
		if(aInOutBegin == aEnd)
			return {};

		if(aInOutBegin->myType != tokenizer::Token::Type::L_Paren)
			return {};

		aInOutBegin++;
	}

	std::vector<std::vector<tokenizer::Token>> arguments;
	std::vector<tokenizer::Token> variadic;

	arguments.resize(myArguments);

	bool parenClosed = false;
	for(size_t i = 0; i < myArguments; i++)
	{
		std::vector<tokenizer::Token> arg;
		size_t depth = 0;
		while (aInOutBegin != aEnd)
		{
			if(aInOutBegin->myType == tokenizer::Token::Type::R_Paren)
			{
				if(depth > 0)
				{
					depth--;
					aInOutBegin++;
					continue;
				}

				if(i == myArguments - 1)
				{
					parenClosed = true;
					aInOutBegin++;
					break;
				}

				CompilerContext::EmitError("Expected more arguments for macro", *aInOutBegin);
				return {};
			}

			if(aInOutBegin->myType == tokenizer::Token::Type::L_Paren)
			{
				depth++;
				aInOutBegin++;
				continue;
			}

			if (depth == 0)
			{
				if(aInOutBegin->myType == tokenizer::Token::Type::Comma)
				{
					aInOutBegin++;
					break;
				}
			}

			arguments[i].push_back(*aInOutBegin);
			aInOutBegin++;
		}
	}

	if (myHasVariadic && !parenClosed)
	{
		size_t depth = 0;
		while (aInOutBegin != aEnd)
		{
			if(aInOutBegin->myType == tokenizer::Token::Type::L_Paren)
				depth++;

			if(aInOutBegin->myType == tokenizer::Token::Type::R_Paren)
			{
				if(depth == 0)
				{
					aInOutBegin++;
					break;
				}

				depth--;
			}

			variadic.push_back(*aInOutBegin);
			aInOutBegin++;
		}
	}

	for (Component& comp : myComponents)
	{
		switch (comp.myType)
		{
		case Component::Type::Argument:
			{
				std::vector<tokenizer::Token>& arg = arguments[comp.myArgumentIndex];
				out.insert(std::end(out), std::begin(arg), std::end(arg));
			}
			break;
		case Component::Type::Token:
			out.push_back(*comp.myToken);
			break;
		case Component::Type::VariadicExpansion:
			out.insert(std::end(out), std::begin(variadic), std::end(variadic));
			break;
		}
	}

	return out;
}
