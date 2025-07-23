#include "Types.h"

#include <cassert>

namespace fisk::precompiler
{
    SourceChar SourceLine::SourceLineIterator::operator*()
    {
        return {
            myLine->myFile,
            myLine->myLine,
            myIndex,
            myLine->myText[myIndex]
        };
    }

    SourceLine::SourceLineIterator& SourceLine::SourceLineIterator::operator++()
    {
        myIndex++;
        return *this;
    }

    bool SourceLine::SourceLineIterator::operator==(nullptr_t)
    {
        return myIndex == myLine->myText.length();
    }

    bool SourceLine::SourceLineIterator::operator==(SourceLineIterator aOther)
    {
        assert(&myLine == &aOther.myLine && "Unrelated iterators are being compared");

        return myIndex == aOther.myIndex;
    }

    bool SourceChar::operator==(const char aOther) const
    {
        return myCharacter == aOther;
    }

    SourceLine::SourceLineIterator SourceLine::begin()
    {
        return { this };
    }

    nullptr_t SourceLine::end()
    {
        return nullptr;
    }

}

