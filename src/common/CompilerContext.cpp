#include "common/CompilerContext.h"

#include <iostream>

std::vector<std::string> CompilerContext::myPrintContext;
std::stack<std::string> CompilerContext::myFileStack;
bool CompilerContext::myHasErrors = false;
size_t CompilerContext::myCurrentLine = 0;
std::unordered_map<std::string, std::string> CompilerContext::myFlags;


void CompilerContext::EmitWarning(const std::string& aMessage, size_t aColumn, size_t aLine, size_t aSize)
{
	std::cout << "WARNING [" << myFileStack.top() << ":" << aLine << ":" << aColumn << "] " << aMessage << "\n";

	if (myPrintContext.size() > aLine)
	{
		std::cout << myPrintContext[aLine] << "\n";
		for (size_t i = 0; i < aColumn; i++)
		{
			std::cout << ' ';
		}
		std::cout << '^';
		for (size_t i = 0; i < aSize - 1; i++)
		{
			std::cout << '~';
		}
		std::cout << "\n\n";
	}
}

void CompilerContext::EmitError(const std::string& aMessage, size_t aColumn, size_t aLine, size_t aSize)
{
	myHasErrors = true;

	std::cerr << "ERROR [" << myFileStack.top() << ":" << aLine << ":" << aColumn << "] " << aMessage << "\n";
	
	if (myPrintContext.size() > aLine)
	{
		std::cout << myPrintContext[aLine] << "\n";
		for (size_t i = 0; i < aColumn; i++)
		{
			std::cout << ' ';
		}
		std::cout << '^';
		for (size_t i = 0; i < aSize - 1; i++)
		{
			std::cout << '~';
		}
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