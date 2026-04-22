
#include "LineReader.h"

namespace fisk::precompiler
{

	LineReader::LineReader(std::shared_ptr<std::istream> aStream, std::string aSourcePath)
		: myStream(aStream), mySourcePath(aSourcePath)
	{
	}

    LineReader::LineReader(std::string aSourcePath)
		: myStream(std::make_shared<std::ifstream>(aSourcePath)), mySourcePath(aSourcePath)
    {
    }

    LineReader& LineReader::operator++()
	{
		if (myState == State::UnPrimed)
			NextLine();

		NextLine();

		return *this;
	}

	SourceLine LineReader::operator*()
	{
		if (myState == State::UnPrimed)
			NextLine();

		return {
			.myPath = mySourcePath,
			.myText = myLineBuffer,
			.myLine = myLineNumber
		};
	}

	bool LineReader::operator==(const std::nullptr_t aOther)
	{
		if (myState == State::UnPrimed)
			NextLine();

		return myState == State::EndOfFile;
	}

	bool LineReader::operator!=(const std::nullptr_t aOther)
	{
		return !(*this == aOther);
	}

	void LineReader::NextLine()
	{
		myLineBuffer.clear();

		if (!std::getline(*myStream, myLineBuffer))
		{
			if (myLineBuffer.empty())
				myState = State::EndOfFile;

			return;
		}

		myLineNumber++;
		myState = State::Primed;
	}
}