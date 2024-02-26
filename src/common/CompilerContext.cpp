#include "common/CompilerContext.h"

#include <iostream>

#if _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

FeatureSwitch CompilerContext::myWarningSwitches("data/warnings.txt");

std::vector<std::filesystem::path> CompilerContext::myBaseDirectories;
std::vector<std::filesystem::path> CompilerContext::myAdditionalDirectories;

std::vector<std::string> CompilerContext::myPrintContext;
std::stack<std::vector<std::string>> CompilerContext::myPrintContextStack;
std::stack<std::filesystem::path> CompilerContext::myFileStack;
size_t CompilerContext::myIgnoreDepth = 0;
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

std::string Dequote(std::string aString)
{
	if (aString.length() < 2)
		return aString;

	if (aString.at(0) == '"' && aString.at(aString.length() - 1) == '"')
		return aString.substr(1, aString.length() - 2);

	return aString;
}


void CompilerContext::EmitWarning(const std::string& aMessage, const tokenizer::Token& aToken)
{
	EmitWarning(aMessage, aToken.myFile, aToken.myColumn, aToken.myLine, aToken.myRawText.length());
}

void CompilerContext::EmitWarning(const std::string& aMessage,std::filesystem::path aFile, size_t aColumn, size_t aLine, size_t aSize)
{
	if (myIgnoreDepth > 0)
		return;

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

#if _WIN32
	std::cout << std::flush;
	if(!SetConsoleTextAttribute(hConsole, screenBufferInfo.wAttributes))
		 return;
#endif

	std::cout << " " << aMessage <<  " [in file " << aFile.string() << ":" << aLine << ":" ;

	if (aColumn == npos)
	{
		std::cout << "eol";
	}
	else
	{
		std::cout << aColumn;
	}

	std::cout << "] " << "\n";

	if (myPrintContext.size() > aLine)
	{
		if(aColumn == npos)
		{
			std::string line = Escape(myPrintContext[aLine]);
			std::cout << line << "\n";
			for(size_t i = 0; i < line.length(); i++)
			{
				std::cout << ' ';
			}
		}
		else
		{
			size_t offset = 0;
			std::cout << Escape(myPrintContext[aLine], offset) << "\n";
			for (size_t i = 0; i < aColumn + offset; i++)
			{
				std::cout << ' ';
			}
		}

#if _WIN32
	std::cout << std::flush;
	 if(!SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | (screenBufferInfo.wAttributes & backgroundMask)))
		 return;
#endif

		std::cout << '^';
		for (size_t i = 0; i < aSize - 1; i++)
		{
			std::cout << '~';
		}

#if _WIN32
	std::cout << std::flush;
	if(!SetConsoleTextAttribute(hConsole, screenBufferInfo.wAttributes))
		 return;
#endif

		std::cout << "\n\n";
	}
}

void CompilerContext::EmitError(const std::string& aMessage, const tokenizer::Token& aToken)
{
	EmitError(aMessage, aToken.myFile, aToken.myColumn, aToken.myLine, aToken.myRawText.length());
}

void CompilerContext::EmitError(const std::string& aMessage, std::filesystem::path aFile, size_t aColumn, size_t aLine, size_t aSize)
{
	if (myIgnoreDepth > 0)
		return;

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
	
#if _WIN32
	std::cout << std::flush;
	if(!SetConsoleTextAttribute(hConsole, screenBufferInfo.wAttributes))
		 return;
#endif
	std::cout << " " << aMessage << " [in file " << aFile.string() << ":" << aLine << ":";
	
	if (aColumn == npos)
	{
		std::cout << "eol";
	}
	else
	{
		std::cout << aColumn;
	}

	std::cout << "] "  << "\n";
	if (myPrintContext.size() > aLine)
	{
		if (aColumn == npos)
		{
			std::string line = Escape(myPrintContext[aLine]);
			std::cout << line << "\n";
			for (size_t i = 0; i < line.length(); i++)
			{
				std::cout << ' ';
			}
		}
		else
		{
			size_t offset = 0;
			std::cout << Escape(myPrintContext[aLine], offset) << "\n";
			for (size_t i = 0; i < aColumn + offset; i++)
			{
				std::cout << ' ';
			}
		}

#if _WIN32
		std::cout << std::flush;
		 if(!SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | (screenBufferInfo.wAttributes & backgroundMask)))
			 return;
#endif

		std::cout << '^';
		for (size_t i = 0; i < aSize - 1; i++)
		{
			std::cout << '~';
		}

#if _WIN32
		std::cout << std::flush;
		if(!SetConsoleTextAttribute(hConsole, screenBufferInfo.wAttributes))
			 return;
#endif

		std::cout << "\n\n";
	}
}

std::optional<std::filesystem::path> CompilerContext::FindFile(const std::filesystem::path& aPath, bool aExpandedLookup)
{
	if (aExpandedLookup)
	{
		for (std::filesystem::path& dir : myBaseDirectories)
		{
			std::filesystem::path fullPath = dir;
			fullPath /= aPath;
			if (std::filesystem::exists(fullPath))
			{
				return fullPath;
			}
		}
	}

	std::filesystem::path fullPath = myFileStack.top().parent_path();
	fullPath /= aPath;
	if (std::filesystem::exists(fullPath))
	{
		return fullPath;
	}

	for (std::filesystem::path& dir : myAdditionalDirectories)
	{
		std::filesystem::path fullPath = dir;
		fullPath /= aPath;
		if (std::filesystem::exists(fullPath))
		{
			return fullPath;
		}
	}


	return {};
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

void CompilerContext::PushFile(const std::filesystem::path& aFile)
{
	myFileStack.push(aFile);
	myPrintContextStack.push(myPrintContext);
}

void CompilerContext::PopFile()
{
	myFileStack.pop();
	SetPrintContext(myPrintContextStack.top());
	myPrintContextStack.pop();
}

std::filesystem::path CompilerContext::GetCurrentFile()
{
	if (myFileStack.empty())
		return "/none";
	return myFileStack.top();
}

bool MatchesPattern(std::string aFilePath, std::string aPattern)
{
	std::vector<std::string> parts;
	
	{
		size_t at = 0;
		do 
		{
			size_t next = aPattern.find('*', at);
			if (next == std::string::npos)
				break;

			if (at == 0 && next > 0)
				parts.push_back("");
			
			parts.push_back(aPattern.substr(at, next - at));
			at = next + 1;

		} while (true);
		parts.push_back(aPattern.substr(at));
	}

	size_t at = 0;
	size_t part = 0;
	do 
	{
		if (parts[part] == "")
		{
			part++;
			continue;
		}

		size_t next = aFilePath.find(parts[part], at);
		if (next == std::string::npos)
			break;

		if (at != 0)
		{
			size_t dirSeperator = aFilePath.find_first_of("/\\", at);
			if (dirSeperator != std::string::npos && dirSeperator < next)
				break;
		}

		at = next + parts[part].length();

		part++;
	} while (part < parts.size());
		
	return at == aFilePath.length();

}



std::vector<std::filesystem::path> CompilerContext::ParseCommandLine(int argc, char** argv)
{
	std::vector<std::string> potentialFiles;
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

			if (flagName.starts_with("w:"))
			{
				if(flagValue == "disable")
					myWarningSwitches.Disable(flagName.substr(2));
				else
					myWarningSwitches.Enable(flagName.substr(2));
			}
			else if (flagName == "p:additional_include" || flagName == "p:i")
			{
				myAdditionalDirectories.push_back(Dequote(flagValue));
			}
			else if (flagName == "dir")
			{
				potentialFiles.push_back(flagValue + "*.cpp");
				myAdditionalDirectories.push_back(flagValue);
			}
			else if (flagName == "file" || flagName == "f")
			{
				potentialFiles.push_back(flagValue);
			}
			else
			{
				myFlags.insert(std::pair(flagName, Dequote(flagValue)));
			}
		}
		else
		{
			potentialFiles.push_back(argv[i]);
		}
	}

	if (!GetFlag("p:no_std"))
	{
		if (std::optional<std::string> dir = GetFlag("p:custom_std"))
			myBaseDirectories.push_back(*dir);
		else
			myBaseDirectories.push_back("std");
	}

	if (!GetFlag("p:no_platform"))
	{
#if _WIN32
		if (std::optional<std::string> dir = GetFlag("p:custom_windows"))
			myBaseDirectories.push_back(*dir);
		else
			myBaseDirectories.push_back("windows");
#endif
	}



	std::vector<std::filesystem::path> files;

	for (const std::string& target : potentialFiles)
	{

		size_t starPos = target.find('*');
		if (starPos == std::string::npos)
		{
			files.push_back(target);
			continue;
		}

		std::string pattern = target;
		std::filesystem::recursive_directory_iterator it(std::filesystem::current_path());
		
		if (starPos > 0)
		{
			size_t dirSeparator = target.find_last_of("/\\", starPos);
			if (dirSeparator != std::string::npos)
			{
				pattern = target.substr(dirSeparator + 1);
				std::filesystem::path basePath = target.substr(0, dirSeparator + 1);
				if (basePath.is_absolute())
				{

					it = std::filesystem::recursive_directory_iterator(basePath);
				}
				else
				{
					std::filesystem::path dir = std::filesystem::current_path();
					dir /= basePath;
					it = std::filesystem::recursive_directory_iterator(dir);
				}
			}
		}


		while (it != std::filesystem::recursive_directory_iterator())
		{
			if (it->is_regular_file())
			{
				std::string file = it->path().string();
				if (MatchesPattern(file, pattern))
				{
					files.push_back(file);
				}
			}
			it++;
		}
	}

	for (std::filesystem::path& file : files)
		file = std::filesystem::relative(file);

	return files;
}

std::optional<const std::string> CompilerContext::GetFlag(const std::string_view& aFlag)
{
	if(myFlags.count(std::string(aFlag)) != 0)
	{
		return myFlags.at(std::string(aFlag));
	}
	return {};
}

bool CompilerContext::IsWarningEnabled(const std::string& aWarning)
{
	return myWarningSwitches.IsEnabled(aWarning);
}

CompilerContext::IgnoreHandle CompilerContext::IgnoreErrors()
{
	return IgnoreHandle(myIgnoreDepth);
}

CompilerContext::IgnoreHandle::IgnoreHandle(size_t& aIgnoreDepthPtr)
	: myIgnoreDepth(aIgnoreDepthPtr)
{
	myIgnoreDepth++;
}

CompilerContext::IgnoreHandle::~IgnoreHandle()
{
	myIgnoreDepth--;
}
