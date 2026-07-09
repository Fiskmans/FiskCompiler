
#ifndef FISK_PRECOMPILER_LINE_READER_H
#define FISK_PRECOMPILER_LINE_READER_H

#include "precompiler/Types.h"

#include <string>
#include <fstream>
#include <iterator>
#include <memory>

namespace fisk::precompiler
{

	class LineReader
	{
	public:
		using iterator_category = std::input_iterator_tag;
		using difference_type	= std::ptrdiff_t;

		using value_type 		= SourceLine; 
		using pointer			= value_type*;
		using reference			= value_type&;
		
		LineReader() = default;
		LineReader(const LineReader&) = default;
		LineReader(LineReader&&) = default;
		LineReader(std::shared_ptr<std::istream> aStream, std::string aSourcePath);
		~LineReader() = default;
		
		LineReader& operator=(const LineReader&) = default;
		LineReader& operator=(LineReader&&) = default;

		LineReader& operator++();
		LineReader operator++(int);
		SourceLine& operator*() const;

		bool operator==(const LineReader&) const = default;
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
		std::shared_ptr<std::istream> myStream;

        mutable SourceLine myItem;
	};
}

#endif // !FISK_PRECOMPILER_FILE_READER_H
