
#pragma once

#include <cassert>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace fisk::precompiler
{
    template <std::input_or_output_iterator BeginIterator, class EndIteratorOrSentinel> struct SimpleRangeWrapper
    {
        SimpleRangeWrapper() = default;
        SimpleRangeWrapper(const SimpleRangeWrapper& aOther) = default;
        SimpleRangeWrapper(BeginIterator aBegin, EndIteratorOrSentinel aEnd) : myBegin(aBegin), myEnd(aEnd)
        {
        }

        bool operator==(const SimpleRangeWrapper&) const = default;

        BeginIterator myBegin;
        EndIteratorOrSentinel myEnd;

        BeginIterator begin() const
        {
            return myBegin;
        }

        EndIteratorOrSentinel end() const
        {
            return myEnd;
        }
    };

    static_assert(std::ranges::range<SimpleRangeWrapper<int*, int*>>);

    template <class Iterator> using SentinelRange = SimpleRangeWrapper<Iterator, std::nullptr_t>;

    static_assert(std::ranges::range<SentinelRange<int*>>);

    template <class Iterator> using ValueType = decltype(*std::declval<Iterator>());

    template <class BaseIterator> class UnpackingIterator
    {
    public:
        using iterator_category = std::iterator_traits<BaseIterator>::iterator_category;
        using difference_type = std::iterator_traits<BaseIterator>::difference_type;

        using inner_range_t = std::iter_value_t<BaseIterator>;

        using value_type = std::ranges::range_value_t<inner_range_t>;
        using pointer = value_type*;
        using reference = value_type&;

        UnpackingIterator(BaseIterator aIterator) : myAt(aIterator)
        {
        }

        UnpackingIterator() = default;
        UnpackingIterator(const UnpackingIterator& aOther) = default;
        UnpackingIterator& operator=(const UnpackingIterator& aOther) = default;

        reference operator*() const
        {
            assert(!!myInnerAt && "Dereferenced without checking for end");
            return **myInnerAt;
        }

        UnpackingIterator operator++(int)
        {
            UnpackingIterator cpy(*this);
            ++(*this);
            return cpy;
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

        bool operator==(const UnpackingIterator& aOther) const
        {
            if (myInnerAt)
            {
                if (!aOther.myInnerAt)
                    return myInnerAt == std::begin(*aOther.myAt);

                return myInnerAt == aOther.myInnerAt;
            }

            if (aOther.myInnerAt)
                return std::begin(*myAt) == aOther.myInnerAt;

            return myAt == aOther.myAt;
        }

        template <class T> bool operator==(T&& aOther)
        {
            if (myAt == aOther)
                return true;

            if (!myInnerAt)
                myInnerAt = std::begin(*myAt);

            if (*myInnerAt == std::end(*myAt))
            {
                myAt++;

                myInnerAt = {};
                return *this == std::forward<T>(aOther);
            }

            return false;
        }

        template <class T> bool operator!=(T&& aOther)
        {
            return !(*this == std::forward<T>(aOther));
        }

    private:
        BaseIterator myAt;
        std::optional<std::ranges::iterator_t<inner_range_t>> myInnerAt;
    };

    template <class InnerIterator> class ReIterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;

        using value_type = std::iterator_traits<InnerIterator>::value_type;
        using pointer = value_type*;
        using reference = value_type&;

        ReIterator()
            : myIndex(0), ourData(nullptr)
        {
        }

        ReIterator(InnerIterator aInner) : myIndex(0), ourData(std::make_shared<SharedData>(aInner))
        {
        }

        reference operator*() const
        {

            assert(myIndex < ourData->myValues.size() &&
                   "Invalid usage: dereference after potentially walking past end");

            return ourData->myValues[myIndex];
        }

        ReIterator& operator++()
        {
            myIndex++;
            if (myIndex > ourData->myHighestIndex)
            {
                ++(ourData->myIterator);
                ++(ourData->myHighestIndex);
            }
            return *this;
        }

        ReIterator operator++(int)
        {
            ReIterator out(*this);
            ++(*this);
            return out;
        }

        bool operator==(const ReIterator& aOther) const
        {
            assert(ourData.get() == aOther.ourData.get() && "Invalid usage: comparing unrelated ReIterators");

            return myIndex == aOther.myIndex;
        }
        bool operator!=(const ReIterator& aOther) const
        {
            assert(ourData.get() == aOther.ourData.get() && "Invalid usage: comparing unrelated ReIterators");

            return myIndex != aOther.myIndex;
        }

        bool operator==(InnerIterator aOther)
        {
            if (myIndex < ourData->myValues.size())
                return false;

            if (ourData->myIterator == aOther)
                return true;

            if (myIndex == ourData->myValues.size())
                ourData->myValues.push_back(*(ourData->myIterator));

            return false;
        }

        bool operator==(std::nullptr_t aOther)
        {
            if (myIndex < ourData->myValues.size())
                return false;

            if (ourData->myIterator == aOther)
                return true;

            if (myIndex == ourData->myValues.size())
                ourData->myValues.push_back(*(ourData->myIterator));

            return false;
        }

    private:
        size_t myIndex;

        struct SharedData
        {
            SharedData(InnerIterator aIterator) : myHighestIndex(0), myIterator(aIterator)
            {
            }

            size_t myHighestIndex;
            InnerIterator myIterator;
            std::vector<value_type> myValues;
        };

        std::shared_ptr<SharedData> ourData;
    };

    struct SourceChar
    {
        std::string myFilePath;
        size_t myLine;
        size_t myColumn;

        char myCharacter;

        operator char() const
        {
            return myCharacter;
        };
        bool operator==(const SourceChar& aOther) const = default;
    };

    struct SourceLine
    {
        std::string myPath;
        std::string myText;
        size_t myLine;

        class Iterator
        {
        public:
            using InnerIterator = std::string::const_iterator;

            using iterator_category = InnerIterator::iterator_category;
            using difference_type = InnerIterator::difference_type;

            using value_type = SourceChar;
            using pointer = value_type*;
            using reference = value_type&;

            Iterator() = default;
            Iterator(InnerIterator aStart, SourceChar aDefaults);

            SourceChar& operator*() const;
            Iterator& operator++();
            Iterator operator++(int);

            template <class T> bool operator==(T&& aOther)
            {
                return myInner == std::forward<T>(aOther);
            }
            template <class T> bool operator==(T&& aOther) const
            {
                return myInner == std::forward<T>(aOther);
            }

        private:
            InnerIterator myStart;
            InnerIterator myInner;
            mutable SourceChar myCurrent;
            SourceChar myDefaults;
        };

        bool operator==(const SourceLine& aOther) const;

        Iterator begin() const;
        Iterator end() const;
    };

    static_assert(std::ranges::range<SourceLine>);
} // namespace fisk::precompiler

inline std::ostream& operator<<(std::ostream& os, const std::nullptr_t& value)
{
    os << std::string("nullptr");
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const fisk::precompiler::SourceLine& value)
{
    os << value.myPath << std::string("#L") << std::to_string(value.myLine) << std::string(" \"") << value.myText
       << std::string("\"");
    return os;
}