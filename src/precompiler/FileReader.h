
#pragma once

#include "Types.h"

#include "LineReader.h"

#include <string>
#include <fstream>
namespace fisk::precompiler
{

	class FileReader
	{
	public:
		FileReader(std::string aPath);

		SourceLine operator*();
		FileReader& operator++();

		bool operator== (nullptr_t aOther);
		bool operator!= (nullptr_t aOther);

	private:


		std::string myPath;
		std::ifstream myFile;
		LineReader myLineReader;
		size_t myLineNumber;
	};
}