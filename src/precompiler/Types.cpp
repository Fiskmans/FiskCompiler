#include "Types.h"

#include <cassert>

namespace fisk::precompiler
{
    SourceLine::Iterator::Iterator(std::ranges::iterator_t<std::string> aStart, SourceChar aDefaults)
    {
        myStart = aStart;
        myInner = aStart;
        myDefaults = aDefaults;
    }

    SourceChar SourceLine::Iterator::operator*() const
    {
        SourceChar decorated{myDefaults};
        decorated.myCharacter = *myInner;
        decorated.myColumn = std::distance(myStart, myInner);
        return decorated;
    }

    SourceLine::Iterator& SourceLine::Iterator::operator++()
    {
        myInner++;
        return *this;
    }

    SourceLine::Iterator SourceLine::Iterator::operator++(int)
    {
        Iterator copy{*this};
        ++(*this);
        return copy;
    }

    bool SourceChar::operator==(const char aOther) const
    {
        return myCharacter == aOther;
    }

    bool SourceLine::operator==(const SourceLine &aOther) const
    {
        return myPath == aOther.myPath
            && myText == aOther.myText
            && myLine == aOther.myLine;
    }

    SourceLine::Iterator SourceLine::begin()
    {
        return Iterator(std::ranges::begin(myText), { myPath, myLine, 0, '\0'});
    }

    SourceLine::Iterator SourceLine::end()
    {
        return Iterator(std::ranges::end(myText), { myPath, myLine, 0, '\0'});
    }

}

