
#pragma once

#include <string>

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
	using BeginIteratorOf = decltype(std::ranges::begin(declval<Range>()));

	template<class Range>
	using EndIteratorOf = decltype(std::ranges::end(declval<Range>()));

	template<class Iterator>
	using ValueType = decltype(*declval<Iterator>());

	template<class BaseIterator>
	class UnpackingIterator
	{
	public:

		UnpackingIterator(BaseIterator aIterator)
			: myAt(aIterator)
		{
		}

		auto operator*()
		{
			if (!myInnerAt)
				myInnerAt = std::ranges::begin(*myAt);

			return **myInnerAt;
		}

		UnpackingIterator& operator++()
		{
			if (!myInnerAt)
				myInnerAt = std::ranges::begin(*myAt);

			if (*myInnerAt == std::ranges::end(*myAt))
			{
				myAt++;
				myInnerAt = {};
				return;
			}

			++*myInnerAt;
		}

		template<class T>
		bool operator==(T&& aOther)
		{
			return myAt == aOther;
		}
		
		template<class T>
		bool operator==(const T& aOther) const
		{
			return myAt == aOther;
		}
		
		template<class T>
		bool operator!=(T&& aOther)
		{
			return myAt != aOther;
		}
		
		template<class T>
		bool operator!=(const T& aOther) const
		{
			return myAt != aOther;
		}

	private:
		BaseIterator myAt;
		BeginIteratorOf<ValueType<BaseIterator>> myInnerAt;
	};


	struct SourceChar
	{
		std::string myFilePath;
		size_t myLine;
		size_t myColumn;

		char myCharacter;

		bool operator==(const char aOther) const;
	};

	struct SourceLine
	{
		std::string myFile;
		std::string myText;
		size_t myLine;

		struct SourceLineIterator
		{
			SourceLine& myLine;
			size_t myIndex = 0;

			SourceChar operator*();
			bool operator==(nullptr_t);
			bool operator==(SourceLineIterator aOther);
		};

		SourceLineIterator begin();
		nullptr_t end();

		SimpleRange<SourceLineIterator, SourceLineIterator> TrimEnd(size_t aAmount);
	};
}