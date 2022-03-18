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
		'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','y',
		'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Y',
		'0','1','2','3','4','5','6','7','8','9',
		'_','{','}','[',']','#','(',')','<','>','%',':',';','.','?','*','+','-','/','^','&','|','~','!','=',',','\\','"','\'', ' '
	};

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

	return aLines;
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

	for (size_t i = 0; i < aLines.size(); i++)
	{
		CompilerContext::SetCurrentLine(i);
		TokenMatcher::MatchTokens(out, aLines[i]);
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

	std::vector<std::string> logicalSource = Reduce(physicalSource);
	CompilerContext::SetPrintContext(logicalSource);

	std::vector<Token> tokens = Tokenize(logicalSource);

	//#include expand

	CompilerContext::PopFile();

	return tokens;
}