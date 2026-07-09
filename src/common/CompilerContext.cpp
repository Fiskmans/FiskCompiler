#include "common/CompilerContext.h"

#include <iostream>

#if _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

FeatureSwitch CompilerContext::myWarningSwitches("data/warnings.txt");

std::vector<std::filesystem::path> CompilerContext::myBaseDirectories;
std::vector<std::filesystem::path> CompilerContext::myAdditionalDirectories;

std::unordered_map<std::string, fisk::precompiler::ReIterator<fisk::precompiler::LineReader>> CompilerContext::myFiles;
size_t CompilerContext::myIgnoreDepth = 0;
bool CompilerContext::myHasErrors = false;
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
		case '\t': out += "\u2192"; break; // →
		case '\n': out += "\u00b6"; break; // ¶
		case '\r': out += "\u2b10"; break; // ⬐
		case '\a': out += "?"; break;
		case '\v': out += "?"; break;
		case '\b': out += "?"; break;
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


void CompilerContext::EmitWarning(const std::string& aTag, fisk::precompiler::SourceChar aAt, fisk::precompiler::SourceChar aUntil)
{
    if (!ShouldWarn(aTag))
        return;

	if (myIgnoreDepth > 0)
		return;

	std::ostream& output = std::cerr;

    ChangeColor(output, ConsoleColor::Yellow);
	output << "WARNING" << aTag;
    PrintContext(output, ConsoleColor::Yellow, aAt, aUntil);
}


void CompilerContext::EmitError(const std::string& aTag, fisk::precompiler::SourceChar aAt, fisk::precompiler::SourceChar aUntil)
{
	if (myIgnoreDepth > 0)
		return;

	myHasErrors = true;
	
	std::ostream &output = std::cerr;

    ChangeColor(output, ConsoleColor::Red);
    output << "ERROR " << aTag;
    PrintContext(output, ConsoleColor::Red, aAt, aUntil);
}

void CompilerContext::RegisterFile(fisk::precompiler::ReIterator<fisk::precompiler::LineReader> aFileStart,
                                   const std::string& aFilePath)
{
    myFiles.insert({aFilePath, aFileStart});
}

void CompilerContext::PrintContext(std::ostream& aStream, ConsoleColor aColor, fisk::precompiler::SourceChar aAt, fisk::precompiler::SourceChar aUntil)
{
    ChangeColor(aStream, ConsoleColor::Reset);
    aStream << aAt.myFilePath << ":" << aAt.myLine << ":" << aAt.myColumn << std::endl;

    decltype(myFiles)::iterator fileIt = myFiles.find(aAt.myFilePath);
    if (fileIt == myFiles.end())
    {
        aStream << "Unable to print context, " << aAt.myFilePath << " has not been loaded" << std::endl;
        return;
    }

    fisk::precompiler::ReIterator<fisk::precompiler::LineReader> fileReader = fileIt->second;

    for (size_t i = 1; i < aAt.myLine; i++)
    {
        if (fileReader == nullptr)
        {
            aStream << "Unable to print context, Error is past end of file" << std::endl;
            return;
        }
        fileReader++;
    }

    aStream << *fileReader << std::endl;

	ChangeColor(aStream, aColor);

    for (size_t i = 0; i < aAt.myColumn; i++)
        aStream << ' ';

    aStream << '^';

    if (aAt.myFilePath != aUntil.myFilePath || aAt.myLine != aUntil.myLine)
    {
        for (size_t i = aAt.myColumn + 1; i < (*fileReader).myText.length(); i++)
        {
            aStream << '~';
        }

        aStream << " [Continues]" << std::endl;
        ChangeColor(aStream, ConsoleColor::Reset);
        return;
    }

    for (size_t i = aAt.myColumn + 1; i < aUntil.myColumn; i++)
    {
        aStream << '~';
    }
    aStream << std::endl;
    ChangeColor(aStream, ConsoleColor::Reset);
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

void CompilerContext::ChangeColor(std::ostream &aStream, ConsoleColor aColor)
{
	switch (aColor)
	{
    case ConsoleColor::Reset:
        aStream << "\u001b[0m";
        break;
    case ConsoleColor::Red:
        aStream << "\u001b[31m";
        break;
    case ConsoleColor::Yellow:
        aStream << "\u001b[32m";
        break;
	}
}

bool CompilerContext::ShouldWarn(const std::string& aTag)
{
    std::optional<std::string> flag = GetFlag("w:" + aTag);
    if (!flag)
        return true;

	if (*flag == "no")
        return false;

    return true;
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
