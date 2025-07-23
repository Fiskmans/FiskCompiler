
#include "FileReader.h"
#include "IncludeFilter.h"

namespace fisk::precompiler
{
	IncludeFilter::IncludeFilter(std::unique_ptr<FileReader> aFile, IIncludeFinder& aIncludeFinder)
		: myIncludeFinder(aIncludeFinder)
	{
		myFiles.push(std::move(aFile));
	}

	std::vector<SourceLine>& IncludeFilter::operator*()
	{
		if (myLines.empty())
			Next();

		return myLines;
	}

	void IncludeFilter::Next()
	{
		myLines.clear();

		if (myFiles.top() == nullptr)
		{
			myFiles.pop();
			Next();
			return;
		}

		bool another;

		do
		{
			if (myFiles.top() == nullptr)
			{
				// TODO: Report unexpected-end-of-file
				return;
			}

			another = false;
			SourceLine line = **myFiles.top();

			if (line.myText.ends_with("\\"))
			{
				line.myText.pop_back();
				another = true;
			}

			myLines.push_back(line);

			++*myFiles.top();

		} while (another);
	}
}