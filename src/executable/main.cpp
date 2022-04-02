
#include <iostream>
#include <fstream>

#include "common/CompilerContext.h"

#include "tokenizer/tokenizer.h"
#include "main.h"


std::optional<std::ofstream> GetDumpFile(std::filesystem::path aPath, std::string extension)
{
	if (std::optional<std::string> outDir = CompilerContext::GetFlag("dump_dir"))
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
		}

		return std::ofstream(p);
	}

	return {};
}

void DumpTokens(std::vector<Token>& tokens, std::filesystem::path aPath)
{
	std::string line;
	std::string annotation;
	std::ostream* out = &std::cout;
	std::ofstream file;
	size_t columnLimit = 120;

	if (std::optional<std::ofstream> dumpFile = GetDumpFile(aPath, ".tok"))
	{
		if (*dumpFile)
		{
			file = std::move(*dumpFile);
			out = &file;
			columnLimit = -1;
		}
		else
		{
			CompilerContext::EmitError("Failed to create file to write token output to for: " + aPath.string(), 0, 0);
			return;
		}
	}


	for (Token& tok : tokens)
	{
		while (annotation.length() > line.length()) { line += ' '; }
		while (line.length() > annotation.length()) { annotation += ' '; }
		line += Escape(tok.myRawText);
		annotation += "[" + Token::TypeToString(tok.myType) + "]";

		if (tok.myType == Token::Type::NewLine || line.length() > columnLimit)
		{
			*out << line << "\n" << annotation << "\n\n";
			line = "";
			annotation = "";
		}
	}
	*out << line << "\n" << annotation << "\n\n";
}


int main(int argc, char** argv)
{
	std::vector<std::filesystem::path> files = CompilerContext::ParseCommandLine(argc, argv);

	if (files.empty())
	{
		std::cout << "usage " << argv[0] << " file.cpp " << std::endl;
		return EXIT_FAILURE;
	}

	for (std::filesystem::path file : files)
	{
		std::vector<Token> tokens = Tokenize(file);

		if (CompilerContext::GetFlag("dump") == "tokens") DumpTokens(tokens, file);
	}

	return CompilerContext::HasErrors() ? EXIT_FAILURE : EXIT_SUCCESS;
}