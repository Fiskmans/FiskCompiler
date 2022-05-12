
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

#if 1 * -2
#error
#endif

Precompiler::PreprocessorNumber Precompiler::EvalutateSequence(std::vector<Token>::const_iterator aBegin, std::vector<Token>::const_iterator aEnd)
{
	if (aBegin == aEnd)
	{
		CompilerContext::EmitError("Expected an expression", 0);
		return 0;
	}

	const std::vector<Token::Type> unaryOperators = 
	{
		Token::Type::Not,
		Token::Type::Complement,
		Token::Type::Plus,
		Token::Type::Minus,
	};

	const std::vector<std::function<PreprocessorNumber(PreprocessorNumber)>> operatorFunctors = 
	{
		[](PreprocessorNumber	aFirst) { return !aFirst; },
		[](PreprocessorNumber	aFirst) { return ~aFirst; },
		[](PreprocessorNumber	aFirst) { return +aFirst; },
		[](PreprocessorNumber	aFirst) { return -aFirst; }
	};

	const std::vector<Token::Type> operators = 
	{
		Token::Type::And,
		Token::Type::Or,
		Token::Type::Plus,
		Token::Type::Minus,
		Token::Type::Div,
		Token::Type::Star,
		Token::Type::Mod
	};

	const std::vector<std::function<PreprocessorNumber(PreprocessorNumber, PreprocessorNumber)>> operatorFunctors = 
	{
		[](PreprocessorNumber aFirst, PreprocessorNumber aSecond) { return aFirst && aSecond; },
		[](PreprocessorNumber aFirst, PreprocessorNumber aSecond) { return aFirst || aSecond; },
		[](PreprocessorNumber aFirst, PreprocessorNumber aSecond) { return aFirst + aSecond; },
		[](PreprocessorNumber aFirst, PreprocessorNumber aSecond) { return aFirst - aSecond; },
		[](PreprocessorNumber aFirst, PreprocessorNumber aSecond) { return aFirst / aSecond; },
		[](PreprocessorNumber aFirst, PreprocessorNumber aSecond) { return aFirst * aSecond; },
		[](PreprocessorNumber aFirst, PreprocessorNumber aSecond) { return aFirst % aSecond; }
	};



	std::vector<Token>::const_iterator	it = aBegin;

	std::vector<Token>				pendingOperators;
	std::vector<PreprocessorNumber> values;
	
	auto addValue = [&values, &pendingOperators]()
	{
		if(pendingOperators.size() == values.size())
			return;


	};

	while (it != aEnd)
	{
		const Token& tok = *it;
		switch (tok.myType)
		{
			case Token::Type::Integer:
				values.push_back(tok.EvaluateIntegral());
				break;
			case Token::Type::L_Paren:
			{
				std::vector<Token>::const_iterator matching = FindMatchingEndParen(it + 1, aEnd);
				values.push_back(EvalutateSequence(it + 1, matching - 1));
				it = matching;
			}
				continue;
			case Token::Type::WhiteSpace:
			case Token::Type::NewLine:
				break;
			default:
				if(values.empty())
				{
					if (std::find(begin(unaryOperators), end(unaryOperators), tok.myType) != end(unaryOperators))
						pendingOperators.push_back(tok);
					else
						CompilerContext::EmitError("Unexpected token", tok);
				}
				else
				{
					if(std::find(begin(operators), end(operators), tok.myType) != end(operators))
						pendingOperators.push_back(tok);
					else
						CompilerContext::EmitError("Unexpected token", tok);
				}
				
				break;
		}
		it++;
	}

	if (values.size() == pendingOperators.size())
	{
	}

	for (const Token::Type& op : operators)
	{
		for (size_t i = 0; i < pendingOperators.size(); i++)
		{
			if(pendingOperators[i].myType == op)
			{
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

	while (values.size() > 1)
	{
		values.pop();
	}

	return PreprocessorNumber();
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
