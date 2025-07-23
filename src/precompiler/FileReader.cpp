
#include "FileReader.h"

namespace fisk::precompiler
{
	FileReader::FileReader(std::string aPath)
		: myPath(aPath)
		, myFile(aPath)
		, myLineReader(myFile)
		, myLineNumber(1)
	{
	}

	SourceLine FileReader::operator*()
	{

		return {
			myPath,
			*myLineReader,
			myLineNumber
		};
	}

	FileReader& FileReader::operator++()
	{
		++myLineReader;
		++myLineNumber;
		return *this;
	}

	bool FileReader::operator==(nullptr_t aOther)
	{
		return myLineReader == aOther;
	}
	bool FileReader::operator!=(nullptr_t aOther)
	{
		return myLineReader != aOther;
	}
}
