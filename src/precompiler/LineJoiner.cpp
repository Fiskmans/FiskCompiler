
#include "LineJoiner.h"

#include "common/CompilerContext.h"

namespace fisk::precompiler
{
	LineJoiner::LineJoiner(std::shared_ptr<LineReader> aFile)
		: myFile(aFile)
	{
        Next();
	}

	LineJoiner::Range& LineJoiner::operator*() const
	{
        return myItem;
	}

	LineJoiner& LineJoiner::operator++()
	{
		Next();

		return *this;
	}

	LineJoiner LineJoiner::operator++(int)
	{
        LineJoiner copy(*this);
        ++(*this);
        return copy;
	}

	bool LineJoiner::operator==(std::nullptr_t aOther)
	{
		if (!myLines.empty())
			return false;

		return *myFile == aOther;
	}

	bool LineJoiner::operator!=(std::nullptr_t aOther)
	{
		if (!myLines.empty())
			return true;

		return *myFile != aOther;
	}

	void LineJoiner::Next()
	{
		myLines.clear();

		if (*myFile == nullptr)
			return;

		bool another;

		do
		{
			if (*myFile == nullptr)
			{
                SourceLine::Iterator lastChar = myLines[myLines.size() - 1].begin();
                SourceLine::Iterator e = myLines[myLines.size() - 1].end();

                SourceLine::Iterator n = lastChar;
				while (n != e)
                {
                    lastChar = n;
                    n++;
                }

				// TODO figure out why this leads to a linker error
                // CompilerContext::EmitWarning("unexpected-eof", *lastChar);
				return;
			}

			another = false;
			SourceLine line = **myFile;

			std::string::size_type last = line.myText.find_last_not_of("\n\t \r\b\v");

			if (last != std::string::npos)
            {
                // TODO 5.2 Phases of translation [lex.phases]
				// 1.2
				// This should not splice when inside a raw string literal

                if (line.myText[last] == '\\')
                {
                    line.myText.resize(last);
                    another = true;
                }
            }

			myLines.push_back(line);

			++*myFile;

		} while (another);

		myItem = {std::begin(myLines), std::end(myLines)};
	}
}