#include "common/CompilerContext.h"

#include <iostream>

#if WIN32
#define NOMINMAX
#include <Windows.h>
#endif


std::vector<std::string> CompilerContext::myPrintContext;
std::stack<std::string> CompilerContext::myFileStack;
bool CompilerContext::myHasErrors = false;
size_t CompilerContext::myCurrentLine = 0;
std::unordered_map<std::string, std::string> CompilerContext::myFlags;

std::string Escape(std::string aString, size_t& aOutEscapeCount)
{
	std::string out;
	size_t at = 0;
	while (true)
	{
		size_t pos = aString.find_first_of("\t\n\r\a\v\b", at);
		if(pos == std::string::npos)
		{
			out += aString.substr(at);
			return out;
		}

		aOutEscapeCount++;
		out += aString.substr(at,pos-at);
		switch (aString[pos])
		{
		case '\t': out += "\\t"; break;
		case '\n': out += "\\n"; break;
		case '\r': out += "\\r"; break;
		case '\a': out += "\\a"; break;
		case '\v': out += "\\v"; break;
		case '\b': out += "\\b"; break;
		}
		at = pos + 1;
	}
}


void CompilerContext::EmitWarning(const std::string& aMessage, size_t aColumn, size_t aLine, size_t aSize)
{

#if _WIN32
	std::cout << std::flush;
	 HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	 CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo;
	 if(!GetConsoleScreenBufferInfo(hConsole, &screenBufferInfo))
		 return;
	
	 const WORD backgroundMask = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY | BACKGROUND_RED;

	 if(!SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_RED | (screenBufferInfo.wAttributes & backgroundMask)))
		 return;
#endif

	std::cout << "WARNING";

#if WIN32
	std::cout << std::flush;
	if(!SetConsoleTextAttribute(hConsole, screenBufferInfo.wAttributes))
		 return;
#endif

	std::cout << " " << aMessage <<  " [in file " << myFileStack.top() << ":" << aLine << ":" << aColumn << "] " << "\n";

	if (myPrintContext.size() > aLine)
	{
		size_t offset = 0;
		std::cout << Escape(myPrintContext[aLine], offset) << "\n";
		for (size_t i = 0; i < aColumn + offset; i++)
		{
			std::cout << ' ';
		}

#if WIN32
	std::cout << std::flush;
	 if(!SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | (screenBufferInfo.wAttributes & backgroundMask)))
		 return;
#endif

		std::cout << '^';
		for (size_t i = 0; i < aSize - 1; i++)
		{
			std::cout << '~';
		}

#if WIN32
	std::cout << std::flush;
	if(!SetConsoleTextAttribute(hConsole, screenBufferInfo.wAttributes))
		 return;
#endif

		std::cout << "\n\n";
	}
}

void CompilerContext::EmitError(const std::string& aMessage, size_t aColumn, size_t aLine, size_t aSize)
{
	myHasErrors = true;
	
#if _WIN32
	std::cout << std::flush;
	 HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	 CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo;
	 if(!GetConsoleScreenBufferInfo(hConsole, &screenBufferInfo))
		 return;
	
	 const WORD backgroundMask = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY | BACKGROUND_RED;

	 if(!SetConsoleTextAttribute(hConsole, FOREGROUND_RED | (screenBufferInfo.wAttributes & backgroundMask)))
		 return;
#endif

	std::cerr << "ERROR";
	
#if WIN32
	std::cout << std::flush;
	if(!SetConsoleTextAttribute(hConsole, screenBufferInfo.wAttributes))
		 return;
#endif
	std::cout << " " << aMessage << " [in file " << myFileStack.top() << ":" << aLine << ":" << aColumn << "] "  << "\n";
	if (myPrintContext.size() > aLine)
	{
		size_t offset = 0;
		std::cout << Escape(myPrintContext[aLine], offset) << "\n";
		for (size_t i = 0; i < aColumn + offset; i++)
		{
			std::cout << ' ';
		}

#if WIN32
		std::cout << std::flush;
		 if(!SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | (screenBufferInfo.wAttributes & backgroundMask)))
			 return;
#endif

		std::cout << '^';
		for (size_t i = 0; i < aSize - 1; i++)
		{
			std::cout << '~';
		}

#if WIN32
		std::cout << std::flush;
		if(!SetConsoleTextAttribute(hConsole, screenBufferInfo.wAttributes))
			 return;
#endif

		std::cout << "\n\n";
	}
}

void CompilerContext::SetPrintContext(const std::vector<std::string>& aPrintContext)
{
	myPrintContext = aPrintContext;
}

void CompilerContext::SetCurrentLine(size_t aLine)
{
	myCurrentLine = aLine;
}

size_t CompilerContext::GetCurrentLine()
{
	return myCurrentLine;
}

void CompilerContext::PushFile(const std::string_view& aFile)
{
	myFileStack.push(std::string(aFile));
}

void CompilerContext::PopFile()
{
	myFileStack.pop();
}

std::vector<const char*> CompilerContext::ParseCommandLine(int argc, char** argv)
{
	std::vector<const char*> files;
	for (size_t i = 1; i < argc; i++)
	{
		if (*argv[i] == '-')
		{
			std::string flagName(argv[i] + 1);
			std::string flagValue = "";
			
			if(argc > i + 1 && *argv[i + 1] != '-')
			{
				flagValue = argv[i + 1];
				i++;
			}
			myFlags.insert(std::pair(flagName, flagValue));
		}
		else
		{
			files.push_back(argv[i]);
		}
	}

	return files;
}

bool CompilerContext::HasFlag(const std::string_view& aFlag)
{
	return myFlags.count(std::string(aFlag)) != 0;
}

std::string CompilerContext::GetFlag(const std::string_view& aFlag)
{
	if(myFlags.count(std::string(aFlag)) != 0)
	{
		return myFlags.at(std::string(aFlag));
	}
	return "";
}