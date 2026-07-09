#include "Types.h"

#include <cassert>

namespace fisk::precompiler
{
    SourceLine::Iterator::Iterator(std::string::const_iterator aStart, SourceChar aDefaults)
    {
        myStart = aStart;
        myInner = aStart;
        myDefaults = aDefaults;
    }

    SourceChar& SourceLine::Iterator::operator*() const
    {
        myCurrent = myDefaults;
        myCurrent.myCharacter = *myInner;
        myCurrent.myColumn = std::distance(myStart, myInner);
        return myCurrent;
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

    bool SourceLine::operator==(const SourceLine &aOther) const
    {
        return myPath == aOther.myPath
            && myText == aOther.myText
            && myLine == aOther.myLine;
    }

    SourceLine::Iterator SourceLine::begin() const
    {
        return Iterator(std::ranges::begin(myText), { myPath, myLine, 0, '\0'});
    }

    SourceLine::Iterator SourceLine::end() const
    {
        return Iterator(std::ranges::end(myText), { myPath, myLine, 0, '\0'});
    }

}

