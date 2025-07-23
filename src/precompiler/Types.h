
#pragma once

#include <string>
#include <utility>
#include <optional>

namespace fisk::precompiler
{
	template<class BeginIterator, class EndIterator>
	struct SimpleRange
	{
		BeginIterator myBegin = {};
		EndIterator myEnd = {};

		BeginIterator begin()
		{
			return myBegin;
		}

		EndIterator end()
		{
			return myEnd;
		}
	};

	template<class Iterator>
	using SentinelRange = SimpleRange<Iterator, nullptr_t>;

	template<class Range>
	using BeginIteratorOf = decltype(std::begin(std::declval<Range>()));

	template<class Range>
	using EndIteratorOf = decltype(std::end(std::declval<Range>()));

	template<class Iterator>
	using ValueType = decltype(*std::declval<Iterator>());

	template<class BaseIterator>
	class UnpackingIterator
	{
	public:

		UnpackingIterator(BaseIterator aIterator)
			: myAt(aIterator)
		{
		}

		UnpackingIterator(const UnpackingIterator& aOther) = default;
		UnpackingIterator& operator=(const UnpackingIterator& aOther) = default;

		auto operator*()
		{
			while (!myInnerAt)
			{
				myInnerAt = std::begin(*myAt);
				if (*myInnerAt == std::end(*myAt))
				{
					++myAt;
					myInnerAt = {};
				}
			}

			return **myInnerAt;
		}

		UnpackingIterator& operator++()
		{
			if (!myInnerAt)
				myInnerAt = std::begin(*myAt);

			++*myInnerAt;
			if (*myInnerAt == std::end(*myAt))
			{
				myAt++;
				myInnerAt = {};
				return *this;
			}
			return *this;
		}

		template<class T>
		bool operator==(T&& aOther)
		{
			if (myAt == aOther)
				return true;

			if (!myInnerAt)
				myInnerAt = std::begin(*myAt);

			if (*myInnerAt == std::end(*myAt))
			{
				myAt++;

				myInnerAt = {};
				return *this == aOther;
			}

			return false;
		}
		
		template<class T>
		bool operator!=(T&& aOther)
		{
			return !(*this == aOther);
		}

	private:
		BaseIterator myAt;
		std::optional<BeginIteratorOf<ValueType<BaseIterator>>> myInnerAt;
	};


	struct SourceChar
	{
		std::string myFilePath;
		size_t myLine;
		size_t myColumn;

		char myCharacter;

		bool operator==(const char aOther) const;
		bool operator==(const SourceChar& aOther) const = default;
	};

	struct SourceLine
	{
		std::string myFile;
		std::string myText;
		size_t myLine;

		struct SourceLineIterator
		{
			SourceLine* myLine;
			size_t myIndex = 0;

			SourceChar operator*();
			SourceLineIterator& operator++();
			bool operator==(nullptr_t);
			bool operator==(SourceLineIterator aOther);
		};

		SourceLineIterator begin();
		nullptr_t end();
	};
}