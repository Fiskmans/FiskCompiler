
#include <optional>
#include <filesystem>
#include <variant>
#include <functional>

#include <iostream>

#include "common/CompilerContext.h"
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

	auto filter = [&aOutTokens, &aTokens](iterator aIt)
	{
		while (aIt != std::end(aTokens))
		{
			if(!aIt->IsPrepoccessorSpecific())
				aOutTokens << TranslateToken(*aIt);
			aIt++;
		}
	};

	if(std::optional<iterator> startIt = getNextNotWhitespace(begin(aTokens)))
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
			if(std::optional<iterator> identifierIt = getNextNotWhitespace(start + 1))
			{
				iterator& identifier = *identifierIt;
				switch (identifier->myType)
				{
				case Token::Type::kw_if:
					aFileContext.myIfStack.push(EvaluateExpression(identifier + 1, std::end(aTokens)));
					return;
				case Token::Type::kw_else:
					if(currentState == IfState::Active)
						currentState = IfState::HasBeenActive;
					else if(currentState == IfState::Inactive)
						currentState = IfState::Active;
					return;
				case Token::Type::Identifier:
					if (identifier->myRawText == "elif")
					{
						if (currentState == IfState::Active)
							currentState = IfState::HasBeenActive;
						else if (currentState == IfState::Inactive)
							currentState = EvaluateExpression(identifier + 1, std::end(aTokens));
					}
					else if(identifier->myRawText == "endif")
					{
						aFileContext.myIfStack.pop();
						if (aFileContext.myIfStack.empty())
							CompilerContext::EmitError("Unmatched endif", *identifier);
					}
					else if(identifier->myRawText == "ifdef")
					{
						if(std::optional<iterator> ifdefIt = getNextNotWhitespace(identifier + 1))
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
				filter(start);
			return;
		}
	}
}

void Precompiler::IncludeFile(TokenStream& aOutTokens, const std::vector<Token>& aTokens, std::vector<Token>::const_iterator aIncludeIt)
{
	using iterator = std::vector<Token>::const_iterator;

	auto getNextNotWhitespace	= [&aTokens](iterator aIt) -> std::optional<iterator> {
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

Precompiler::IfState Precompiler::EvaluateExpression(std::vector<Token>::const_iterator aBegin, std::vector<Token>::const_iterator aEnd)
{
	std::vector<Token> fullExpression;
	for (std::vector<Token>::const_iterator it = aBegin; it != aEnd; it++)
	{
		std::vector<Token> buffer = TranslateToken(*it);
		fullExpression.insert(std::end(fullExpression), std::begin(buffer), std::end(buffer));
	}
	
	return EvalutateSequence(fullExpression.begin(), fullExpression.end()) == 0 ? IfState::Inactive : IfState::Active;
}

namespace precompiler_internal
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
				std::cout << "performed unary transform " << Token::TypeToString(op.myType) << " on " << aValue << "\n";
				AddValue(unaryOperatorFunctors[i](aValue), aPendingOperators, aValues);
				return;
			}
		}

		CompilerContext::EmitError("Only unary operators can can be used with a single operand", op);
	}
}

PreprocessorNumber Precompiler::EvalutateSequence(std::vector<Token>::const_iterator aBegin, std::vector<Token>::const_iterator aEnd)
{
	if (aBegin == aEnd)
	{
		CompilerContext::EmitError("Expected an expression", 0);
		return 0;
	}

	std::vector<Token>::const_iterator	it = aBegin;

	std::vector<Token>				pendingOperators;
	std::vector<PreprocessorNumber> values;
	
	while (it != aEnd)
	{
		const Token& tok = *it;
		switch (tok.myType)
		{
			case Token::Type::Integer:
				precompiler_internal::AddValue(tok.EvaluateIntegral(), pendingOperators, values);
				break;
			case Token::Type::L_Paren:
			{
				std::vector<Token>::const_iterator matching = FindMatchingEndParen(it + 1, aEnd);
				precompiler_internal::AddValue(EvalutateSequence(it + 1, matching - 1), pendingOperators, values);
				it = matching;
			}
				continue;
			case Token::Type::WhiteSpace:
			case Token::Type::NewLine:
				break;
			default:
				if(values.empty())
				{
					if (std::find(begin(precompiler_internal::unaryOperators), end(precompiler_internal::unaryOperators), tok.myType) != end(precompiler_internal::unaryOperators))
						pendingOperators.push_back(tok);
					else
						CompilerContext::EmitError("Unexpected token", tok);
				}
				else
				{
					if(std::find(begin(precompiler_internal::operators), end(precompiler_internal::operators), tok.myType) != end(precompiler_internal::operators))
						pendingOperators.push_back(tok);
					else
						CompilerContext::EmitError("Unexpected token", tok);
				}
				
				break;
		}
		it++;
	}

	for (size_t opIndex = 0; opIndex < precompiler_internal::operators.size(); opIndex++)
	{
		const Token::Type& op = precompiler_internal::operators[opIndex];

		for (size_t i = 0; i < pendingOperators.size(); i++)
		{
			if(pendingOperators[i].myType == op)
			{
				if (values.size() < i + 1)
				{
					CompilerContext::EmitError("Incomplete expression", 0);
					return 0;
				}
				PreprocessorNumber first = values[i];
				PreprocessorNumber second = values[i+1];
				PreprocessorNumber result = precompiler_internal::operatorFunctors[opIndex](first, second);

				std::cout << "did " << Token::TypeToString(op) << " [" << opIndex << "] on " << first << " and " << second << " resuting in " << result << "\n";

				values.erase(begin(values) + i, begin(values) + 2 + i);
				values.insert(begin(values) + i, result);

				pendingOperators.erase(begin(pendingOperators) + i);
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
		CompilerContext::EmitWarning("Expected single expression", *aBegin);
	}

	std::cout << "\n";
	return values[0];
}

std::vector<Token>::const_iterator Precompiler::FindMatchingEndParen(std::vector<Token>::const_iterator aBegin, std::vector<Token>::const_iterator aEnd)
{
	size_t depth = 1;
	std::vector<Token>::const_iterator it = aBegin;
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
	if(depth > 0)
	{
		if(aBegin != aEnd)
			CompilerContext::EmitError("Unmatched parenthesis", *(aEnd - 1));
		else
			CompilerContext::EmitError("Unmatched parenthesis", 0);
	}

	return it;
}

std::vector<Token> Precompiler::TranslateToken(const Token& aToken)
{
	TokenStream stream;

	decltype(Context::myMacros)::iterator potentialMacro = myContext.myMacros.find(aToken.myRawText);
	if (potentialMacro != end(myContext.myMacros))
	{
		for (const Token& tok : potentialMacro->second)
			stream << TranslateToken(tok);
	}
	else
	{
		stream << aToken;
	}

	return std::move(stream).Get();
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
