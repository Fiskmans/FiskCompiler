#include "HelpPrinter.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

#include "common/CompilerContext.h"

HelpPrinter::HelpPrinter()
{
	Setup();
}

void HelpPrinter::Emit()
{
	std::string tag;
	{
		std::optional<std::string> commandLine = CompilerContext::GetFlag("help");
		if (!commandLine)
			commandLine = CompilerContext::GetFlag("h");

		if (commandLine)
			tag = *commandLine;
	}

	if(!tag.empty())
	{
		decltype(mySections)::iterator foundSection = mySections.find(tag);
		if(foundSection == mySections.end())
		{
			std::cout << "No such tag: " << tag << std::endl;
			return;
		}

		Section section = foundSection->second;

		std::cout << std::setw(1)
			<< "[" << section.myTag << "]\t"
			<< section.myHeader << ":\n\t"
			<< section.myDigest << "\n\t"
			<< section.myExtended;
		return;
	}

	std::cout << "\n\t" << myHeader << "\n\n";

	for (decltype(mySections)::iterator::value_type& taggedSection : mySections)
	{
		Section& section = taggedSection.second;
		std::cout << "\t" 
			<< std::setw(myHeaderAlignment.Get() + 1) << std::left << section.myHeader << std::right << std::setw(1) << ": " 
			<< std::setw(myTagAlignment.Get() + 2) << "[" + section.myTag + "]" << std::setw(1)
			<< " " << std::setw(myDigestAlignment.Get() + 2) << std::left << section.myDigest << std::right << std::setw(1) << "\n";
	}

	std::cout << std::endl;
}

void HelpPrinter::Setup()
{
	std::ifstream helpFile("data/help.txt");
	std::string line;
	while (std::getline(helpFile, line))
	{
#if _WIN32
		if(line == ":windows")
			break;
#else
#error "unkown platform"
#endif
	}
	
	if(std::getline(helpFile, line))
		myHeader = line;

	while (helpFile)
	{
		while (std::getline(helpFile, line))
		{
			if(line.empty())
				continue;
		
			if (line[0] == '#') 
				continue;

			if(line[0] == ':')
			{
				if (line == ":common")
					continue;
				break;
			}

			Section section(line);
			if(!section.myTag.empty())
			{
				myHeaderAlignment.Update(section.myHeader.length());
				myTagAlignment.Update(section.myTag.length());
				myDigestAlignment.Update(section.myHeader.length());

				mySections.emplace(section.myTag, section);
			}
		}
		while (std::getline(helpFile, line))
		{
			if (line == ":common")
				break;
		}
	}
}

HelpPrinter::Section::Section(const std::string_view& aLine)
{
	size_t	tagStart		= aLine.find(';');
	size_t	tagEnd			= aLine.find(';', tagStart + 1);
	size_t	extendedStart	= aLine.find(';', tagEnd + 1);

	if(tagStart != std::string_view::npos)
		myHeader = aLine.substr(0, tagStart);
	
	if(tagEnd != std::string_view::npos)
		myTag = aLine.substr(tagStart + 1, tagEnd - tagStart - 1);

	if (extendedStart != std::string_view::npos)
	{
		myDigest = aLine.substr(tagEnd + 1, extendedStart - tagEnd - 1);
		myExtended = aLine.substr(extendedStart + 1);
	}
	else
	{
		myDigest = aLine.substr(tagEnd + 1);
	}
}
