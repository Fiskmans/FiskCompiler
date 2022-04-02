
#include <iostream>

#include "common/CompilerContext.h"

#include "tokenizer/tokenizer.h"
#include "main.h"


void DumpTokens(std::vector<Token>& tokens)
{
	std::string line;
	std::string annotation;


	for (Token& tok : tokens)
	{
		while (annotation.length() > line.length()) { line += ' '; }
		while (line.length() > annotation.length()) { annotation += ' '; }
		line += Escape(tok.myRawText);
		annotation += "[" + Token::TypeToString(tok.myType) + "]";

		if (tok.myType == Token::Type::NewLine || line.length() > 120)
		{
			std::cout << line << "\n" << annotation << "\n\n";
			line = "";
			annotation = "";
		}
	}
	std::cout << line << "\n" << annotation << "\n\n";
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

		if(CompilerContext::GetFlag("dump") == "tokens") DumpTokens(tokens);
	}

	return CompilerContext::HasErrors() ? EXIT_FAILURE : EXIT_SUCCESS;
}