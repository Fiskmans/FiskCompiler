
#include "FileReader.h"
#include "LineJoiner.h"

namespace fisk::precompiler
{
	LineJoiner::LineJoiner(std::unique_ptr<FileReader> aFile)
		: myFile(std::move(aFile))
	{
	}

	LineJoiner::Range LineJoiner::operator*()
	{
		if (myLines.empty())
			Next();

		return { std::begin(myLines), std::end(myLines) };
	}

	LineJoiner& LineJoiner::operator++()
	{
		Next();

		return *this;
	}

	bool LineJoiner::operator==(nullptr_t aOther)
	{
		if (!myLines.empty())
			return false;

		return *myFile == aOther;
	}

	bool LineJoiner::operator!=(nullptr_t aOther)
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
				// TODO: Report unexpected-end-of-file
				return;
			}

			another = false;
			SourceLine line = **myFile;

			if (line.myText.ends_with("\\"))
			{
				line.myText.pop_back();
				another = true;
			}

			myLines.push_back(line);

			++*myFile;

		} while (another);
	}
}