
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace fisk::precompiler
{
template <class BeginIterator, class EndIterator> struct SimpleRangeWrapper
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

static_assert(std::ranges::range<SimpleRangeWrapper<int *, int *>>);

template <class Iterator> using SentinelRange = SimpleRangeWrapper<Iterator, std::nullptr_t>;

static_assert(std::ranges::range<SentinelRange<int *>>);

template <class Iterator> using ValueType = decltype(*std::declval<Iterator>());

template <class BaseIterator> class UnpackingIterator
{
  public:
    using inner_range_t = std::iter_value_t<BaseIterator>;
    using inner_value_t = std::ranges::range_value_t<inner_range_t>;

    UnpackingIterator(BaseIterator aIterator) : myAt(aIterator)
    {
    }

    UnpackingIterator(const UnpackingIterator &aOther) = default;
    UnpackingIterator &operator=(const UnpackingIterator &aOther) = default;

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

    UnpackingIterator &operator++()
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

    template <class T> bool operator==(T &&aOther)
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

    template <class T> bool operator!=(T &&aOther)
    {
        return !(*this == aOther);
    }

  private:
    BaseIterator myAt;
    std::optional<std::ranges::iterator_t<inner_range_t>> myInnerAt;
};

template <class InnerIterator> class ReIterator
{
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = ssize_t;

    using value_type = std::iterator_traits<InnerIterator>::value_type;
    using pointer = value_type *;
    using reference = value_type &;

    ReIterator(InnerIterator aInner) : myIndex(0), ourData(std::make_shared<SharedData>(aInner))
    {
    }

    value_type operator*()
    {
        if (myIndex == ourData->myValues.size())
            ourData->myValues.push_back(*(ourData->myIterator));

        assert(myIndex < ourData->myValues.size() && "Invalid usage: dereference after potentially walking past end");

        return ourData->myValues[myIndex];
    }

    ReIterator &operator++()
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

    bool operator==(const ReIterator &aOther) const
    {
        assert(ourData.get() == aOther.ourData.get() && "Invalid usage: comparing unrelated ReIterators");

        return myIndex == aOther.myIndex;
    }
    bool operator!=(const ReIterator &aOther) const
    {
        assert(ourData.get() == aOther.ourData.get() && "Invalid usage: comparing unrelated ReIterators");

        return myIndex != aOther.myIndex;
    }

    bool operator==(std::nullptr_t aOther)
    {
        if (myIndex < ourData->myValues.size())
            return false;

        return ourData->myIterator == aOther;
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

    operator char()
    {
        return myCharacter;
    };
    bool operator==(const char aOther) const;
    bool operator==(const SourceChar &aOther) const = default;
};

struct SourceLine
{
    std::string myPath;
    std::string myText;
    size_t myLine;

    class Iterator
    {
      public:
        using InnerIterator = std::ranges::iterator_t<std::string>;

        using iterator_category = InnerIterator::iterator_category;
        using difference_type = InnerIterator::difference_type;

        using value_type = SourceChar;
        using pointer = value_type *;
        using reference = value_type &;

        Iterator() = default;
        Iterator(InnerIterator aStart, SourceChar aDefaults);

        SourceChar operator*() const;
        Iterator &operator++();
        Iterator operator++(int);

        template <class T> bool operator==(T &&aOther)
        {
            return myInner == std::forward<T>(aOther);
        }
        template <class T> bool operator==(T &&aOther) const
        {
            return myInner == std::forward<T>(aOther);
        }

      private:
        InnerIterator myStart;
        InnerIterator myInner;
        SourceChar myDefaults;
    };

    bool operator==(const SourceLine &aOther) const;

    Iterator begin();
    Iterator end();
};

static_assert(std::ranges::range<SourceLine>);
} // namespace fisk::precompiler

inline std::ostream &operator<<(std::ostream &os, const std::nullptr_t &value)
{
    os << std::string("nullptr");
    return os;
}

inline std::ostream &operator<<(std::ostream &os, const fisk::precompiler::SourceLine &value)
{
    os << value.myPath << "#L" << value.myLine << " \"" << value.myText << "\"";
    return os;
}