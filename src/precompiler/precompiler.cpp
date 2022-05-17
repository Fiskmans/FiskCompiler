
#include <optional>
#include <filesystem>
#include <variant>

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

	std::stack<Token> pendingOperators;
	std::stack<int> values;

	for (Token& tok : fullExpression)
	{
		
	}

	return IfState::Active;
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
