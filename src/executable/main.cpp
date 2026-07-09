
#include <iostream>
#include <fstream>

#include "common/CompilerContext.h"
#include "common/HelpPrinter.h"

#include "precompiler/Types.h"
#include "precompiler/Precompiler.h"


std::optional<std::ofstream> GetArtifactsFile(std::filesystem::path aPath, std::string extension)
{
	if (std::optional<std::string> outDir = CompilerContext::GetFlag("artifact_dir"))
	{
		std::filesystem::path p = std::filesystem::current_path();
		p /= *outDir;

		if (!std::filesystem::exists(p))
			std::filesystem::create_directories(p);

		p /= aPath.stem().string() + extension;
		size_t counter = 1;
		while (std::filesystem::exists(p))
		{
			p = p.parent_path();
			p /= aPath.stem().string() + std::to_string(counter) + extension;
			counter++;
		}

		return std::ofstream(p);
	}

	return {};
}

void printHelp()
{
	HelpPrinter printer;
	printer.Emit();
}

template<class Range>
void EmitLine(const Range& aLine)
{
	for(auto c : aLine)
		fprintf(stderr, "%c", static_cast<char>(c));
 
	fprintf(stderr, "\n");
}

template<class Range>
void EmitLines(const Range& aRangeOfLines)
{
	for(auto line : aRangeOfLines)
	{
		EmitLine(line);
	}
}

int main(int argc, char** argv)
{
	std::vector<std::filesystem::path> files = CompilerContext::ParseCommandLine(argc, argv);

	files.push_back("main.cpp");

	if (files.empty() || CompilerContext::GetFlag("help") || CompilerContext::GetFlag("h"))
	{
		printHelp();
		return EXIT_FAILURE;
	}

	for (std::filesystem::path path : files)
	{
		auto file = fisk::precompiler::Precompile(path.string());

		if (CompilerContext::GetFlag("o:type") == "precompiled")
			EmitLines(file);
			
	}

	return CompilerContext::HasErrors() ? EXIT_FAILURE : EXIT_SUCCESS;
}