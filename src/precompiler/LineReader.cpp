
#include "LineReader.h"

namespace fisk::precompiler
{

	LineReader::LineReader(std::istream& aStream)
		: myStream(aStream)
	{
	}

	LineReader& LineReader::operator++()
	{
		if (myState == State::UnPrimed)
			NextLine();

		NextLine();

		return *this;
	}

	std::string LineReader::operator*()
	{
		if (myState == State::UnPrimed)
			NextLine();

		return myLineBuffer;
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

		if (!std::getline(myStream, myLineBuffer))
		{
			if (myLineBuffer.empty())
				myState = State::EndOfFile;

			return;
		}

		myState = State::Primed;
	}
}