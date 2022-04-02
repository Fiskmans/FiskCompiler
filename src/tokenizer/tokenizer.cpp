#include "tokenizer.h"

#include <stack>

#include "tools/fileHelpers.h"

#include "tokenizer/tokenMatcher.h"

#include "common/CompilerContext.h"

// 5.2 Phases of translation Step 1
std::vector<std::string> UniversalEscape(const std::vector<std::string>& aLines)
{
	// 5.3 Character sets
	const size_t basicCharSetSize = 96;
	const char basicCharSet[basicCharSetSize] = 
	{
		'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','y', 'z',
		'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Y', 'Z',
		'0','1','2','3','4','5','6','7','8','9',
		'_','{','}','[',']','#','(',')','<','>','%',':',';','.','?','*','+','-','/','^','&','|','~','!','=',',','\\','"','\'', ' ', '\t', '\r'
	};
	/*
	Hello
	*/
	const char* beginSearch = basicCharSet;
	const char* endSearch = beginSearch + basicCharSetSize;

	std::vector<std::string> out;
	size_t lineCount = 0;
	for (const std::string& line : aLines)
	{
		lineCount++;
		bool hasWeird = false;
		for (const char c : line)
		{
			if (std::find(beginSearch, endSearch, c) != endSearch)
			{
				hasWeird = true;
			}
		}
		if (hasWeird)
		{
			std::string replacement;
			size_t columnCount = 0;
			for (const char c : line)
			{
				columnCount++;
				if (std::find(beginSearch, endSearch, c) == endSearch)
				{	
					CompilerContext::EmitWarning("unkown character [" + std::to_string(static_cast<int>(c)) + "] replacing with [?]", lineCount, columnCount);
					replacement.push_back('?');
				}
				else
				{
					replacement.push_back(c);
				}
			}
			out.push_back(replacement);
		}
		else
		{
			out.push_back(line);
		}
	}

	return out;
}

std::vector<std::string> Reduce(const std::vector<std::string>& aLines)
{
	std::vector<std::string> out;
	for (size_t i = 0; i < aLines.size(); i++)
	{
		const std::string& line = aLines[i];
		if (line.length() > 0 && line.back() == '\\')
		{
			if (i + 1 == aLines.size())
			{
				CompilerContext::EmitError("[\\] concatination at end of line", i, line.length() - 1);
			}

			std::string concatenated = line.substr(0, line.length() - 1) + aLines[i + 1];
			out.push_back(concatenated);
			i++;
		}
		else
		{
			out.push_back(line);
		}
	}
	return out;
}

std::vector<Token> Tokenize(const std::vector<std::string>& aLines)
{
	std::vector<Token> out;

	TokenMatcher::Context context;
	for (size_t i = 0; i < aLines.size(); i++)
	{
		CompilerContext::SetCurrentLine(i);
		TokenMatcher::MatchTokens(out, aLines[i], context);
	}

	return out;
}

std::vector<Token> PreCompile(const std::vector<Token>& aTokens)
{
	std::vector<Token> out;
	using iterator = std::vector<Token>::const_iterator;


	iterator readHead = aTokens.begin();
	auto isEnd = [&aTokens](const std::vector<Token>::const_iterator& aIt) 
		{ 
			return aIt == aTokens.end(); 
		};

	auto getNextNotWhitespace = [&isEnd](std::vector<Token>::const_iterator aIt) -> std::optional<iterator>
		{
			aIt++;
			while(!isEnd(aIt))
			{
				if (aIt->myType != Token::Type::WhiteSpace)
					return aIt;
			}
			return {};
		};


	while (!isEnd(readHead))
	{
		if (readHead->myType == Token::Type::Include_directive)
		{
			if(std::optional<iterator> expectedPath = getNextNotWhitespace(readHead))
			{
				iterator path = *expectedPath;
				if (path->myType == Token::Type::Header_name)
				{
					out.push_back(Token(Token::Type::Comment, "Would have included " + path->myRawText, 0, 0));

					if (std::optional<iterator> expectedNewLine = getNextNotWhitespace(path))
					{
						iterator newLine = *expectedNewLine;
						if (newLine->myType != Token::Type::NewLine)
						{
							CompilerContext::EmitError("Malformed include directive, expected newline after header_name", *path);
							break;
						}
						readHead = newLine;
					}

					readHead++;
					continue;
				}
				else
				{
					CompilerContext::EmitError("Malformed include directive, expected header name in the form \"header name\" or <header name>", *path);
				}
			}
			else
			{
				CompilerContext::EmitError("Malformed include directive, unexpected end of line", *readHead);
			}
		}

		out.push_back(*readHead);

		readHead++;
	}

	return out;
}

std::vector<Token> Tokenize(const std::string_view& aFilePath)
{
	CompilerContext::PushFile(std::string(aFilePath));

	std::vector<std::string> physicalSource = ReadWholeFile(std::string(aFilePath));
	CompilerContext::SetPrintContext(physicalSource);
	
	std::vector<std::string> escapedPhysicalSource = UniversalEscape(physicalSource);
	CompilerContext::SetPrintContext(escapedPhysicalSource);

	std::vector<std::string> logicalSource = Reduce(escapedPhysicalSource);
	CompilerContext::SetPrintContext(logicalSource);

	std::vector<Token> tokens = Tokenize(logicalSource);

	std::vector<Token> precompiled = PreCompile(tokens);

	CompilerContext::PopFile();

	return precompiled;
}