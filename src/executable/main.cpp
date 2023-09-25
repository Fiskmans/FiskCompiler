
#include <iostream>
#include <fstream>

#include "common/CompilerContext.h"
#include "common/HelpPrinter.h"

#include "tokenizer/tokenizer.h"
#include "markup/Patterns.h"
#include "main.h"


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

void DumpTokens(std::vector<tokenizer::Token>& tokens, std::filesystem::path aPath)
{
	std::string line;
	std::string annotation;
	std::ostream* out = &std::cout;
	std::ofstream file;
	size_t columnLimit = 120;

	if (std::optional<std::ofstream> dumpFile = GetArtifactsFile(aPath, ".tok"))
	{
		if (*dumpFile)
		{
			file = std::move(*dumpFile);
			out = &file;
			columnLimit = -1;
		}
		else
		{
			CompilerContext::EmitError("Failed to create file to write token output to", aPath);
			return;
		}
	}


	for (tokenizer::Token& tok : tokens)
	{
		while (annotation.length() > line.length()) { line += ' '; }
		while (line.length() > annotation.length()) { annotation += ' '; }
		line += Escape(tok.myRawText);
		annotation += "[" + tokenizer::Token::TypeToString(tok.myType) + "]";

		if (tok.myType == tokenizer::Token::Type::NewLine || line.length() > columnLimit)
		{
			*out << line << "\n" << annotation << "\n\n";
			line = "";
			annotation = "";
		}
	}
	*out << line << "\n" << annotation << "\n\n";
}

void DumpMarkup(const markup::TranslationUnit& aMarkup, std::filesystem::path aPath)
{
	std::ostream* out = &std::cout;
	std::ofstream file;
	size_t columnLimit = 120;

	if (std::optional<std::ofstream> dumpFile = GetArtifactsFile(aPath, ".markup"))
	{
		if (*dumpFile)
		{
			file = std::move(*dumpFile);
			out = &file;
		}
		else
		{
			CompilerContext::EmitError("Failed to create file to write markup output to", aPath);
			return;
		}
	}

	*out << aMarkup;
}

void printHelp()
{
	HelpPrinter printer;
	printer.Emit();
}


int main(int argc, char** argv)
{
	std::vector<std::filesystem::path> files = CompilerContext::ParseCommandLine(argc, argv);

	if (files.empty() || CompilerContext::GetFlag("help") || CompilerContext::GetFlag("h"))
	{
		printHelp();
		return EXIT_FAILURE;
	}

	for (std::filesystem::path file : files)
	{
		CompilerContext::PushFile(file);

		std::vector<tokenizer::Token> tokens = tokenizer::Tokenize(file);

		if (CompilerContext::GetFlag("dump") == "tokens") DumpTokens(tokens, file);

		markup::TranslationUnit translationUnit = markup::Markup(tokens);

		if (CompilerContext::GetFlag("dump") == "markup") DumpMarkup(translationUnit, file);

		CompilerContext::PopFile();
	}

	return CompilerContext::HasErrors() ? EXIT_FAILURE : EXIT_SUCCESS;
}