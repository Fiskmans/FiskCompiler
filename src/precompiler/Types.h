
#pragma once

#include <string>
#include <utility>
#include <optional>

namespace fisk::precompiler
{
	template<class BeginIterator, class EndIterator>
	struct SimpleRangeWrapper
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
	using SentinelRange = SimpleRangeWrapper<Iterator, std::nullptr_t>;

	template<class Iterator>
	using ValueType = decltype(*std::declval<Iterator>());

	template<class BaseIterator>
	class UnpackingIterator
	{
	public:
		using inner_range_t = std::iter_value_t<BaseIterator>;
		using inner_value_t = std::ranges::range_value_t<inner_range_t>;

		UnpackingIterator(BaseIterator aIterator)
			: myAt(aIterator)
		{
		}

		UnpackingIterator(const UnpackingIterator& aOther) = default;
		UnpackingIterator& operator=(const UnpackingIterator& aOther) = default;

		inner_value_t operator*()
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
		std::optional<std::ranges::iterator_t<inner_range_t>> myInnerAt;
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

		class Iterator
		{
		public:
			using InnerIterator = std::ranges::iterator_t<std::string>;

			using iterator_category = InnerIterator::iterator_category;
			using difference_type	= InnerIterator::difference_type;

			using value_type 		= SourceChar; 
			using pointer			= value_type*;
			using reference			= value_type&;

			Iterator() = default;
			Iterator(InnerIterator aStart, SourceChar aDefaults);

			SourceChar 	operator*() const;
			Iterator& 	operator++();
			Iterator 	operator++(int);

			template<class T>
			bool operator==(T&& aOther) { return myInner == std::forward<T>(aOther); }
			template<class T>
			bool operator==(T&& aOther) const { return myInner == std::forward<T>(aOther); }

		private:
			InnerIterator myStart;
			InnerIterator myInner;
			SourceChar myDefaults;
		};

		Iterator begin();
		Iterator end();
	};

	static_assert(std::ranges::range<SourceLine>);
}