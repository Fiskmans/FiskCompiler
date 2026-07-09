
#include "LineReader.h"

namespace fisk::precompiler
{
	LineReader::LineReader(std::shared_ptr<std::istream> aStream, std::string aSourcePath)
		: myStream(aStream)
	{
        myItem.myPath = aSourcePath;
        myItem.myLine = 0;
	}

    LineReader& LineReader::operator++()
	{
		if (myState == State::UnPrimed)
			NextLine();

		NextLine();

		return *this;
	}

    LineReader LineReader::operator++(int)
	{
        LineReader copy(*this);
        ++(*this);
		return copy;
	}

	SourceLine& LineReader::operator*() const
	{
        assert(myState == State::Primed && "Incorrect iterator usage, dereference past end or without checking against end");
		return myItem;
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
        myItem.myText.clear();
        myItem.myLine++;

		while (myStream)
        {
            int next = myStream->get();
            if (next == EOF)
            {
                if (myItem.myText.empty())
                {
                    myState = State::EndOfFile;
                    return;
                }
                break;
			}
            
			myItem.myText += (char)next;

            if (next == '\n')
                break;
        }

		myState = State::Primed;
	}
}