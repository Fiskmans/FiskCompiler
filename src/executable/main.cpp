
#include <iostream>

#include "common/CompilerContext.h"

#include "tokenizer/tokenizer.h"

int main(int argc, char** argv)
{
	std::vector<const char*> files = CompilerContext::ParseCommandLine(argc, argv);

	if (files.empty())
	{
		std::cout << "usage " << argv[0] << " file.cpp " << std::endl;
		return EXIT_FAILURE;
	}

	const char* file = argv[1];

	std::vector<Token> tokens = Tokenize(file);

	if(CompilerContext::GetFlag("dump") == "tokens")
	{
		for (Token& tok : tokens)
		{
			std::cout << tok.myRawText << "\n" << Token::TypeToString(tok.myType) << "\n\n";
		}
	}

	return CompilerContext::HasErrors() ? EXIT_FAILURE : EXIT_SUCCESS;
}