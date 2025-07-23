
#ifndef FISK_PRECOMPILER_LINE_READER_H
#define FISK_PRECOMPILER_LINE_READER_H

#include <string>
#include <fstream>

namespace fisk::precompiler
{

	class LineReader
	{
	public:
		LineReader(std::istream& aStream);

		LineReader& operator++();
		std::string operator*();

		bool operator==(const std::nullptr_t aOther);
		bool operator!=(const std::nullptr_t aOther);

	private:

		void NextLine();

		enum class State
		{
			UnPrimed,
			Primed,
			EndOfFile
		};

		State myState = State::UnPrimed;

		std::istream& myStream;

		std::string myLineBuffer;
		size_t myLineNumber = 0;
	};
}

#endif // !FISK_PRECOMPILER_FILE_READER_H
