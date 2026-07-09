
#include "common/CompilerContext.h"
#include "pattern_matcher/PatternMatcher.h"
#include "precompiler/LineJoiner.h"
#include "precompiler/LineReader.h"
#include "precompiler/Types.h"

#include <iterator>
#include <stack>
#include <string>

namespace fisk::precompiler
{
    class Precompiler
    {
        using FileReader = ReIterator<LineJoiner>;

      public:
        using iterator_category = std::iterator_traits<LineReader>::iterator_category;
        using difference_type = std::iterator_traits<LineReader>::difference_type;

        using inner_iterator = UnpackingIterator<FileReader>;

        using value_type = SimpleRangeWrapper<inner_iterator, inner_iterator>;
        using pointer = value_type*;
        using reference = value_type&;

        Precompiler() = default;
        Precompiler(std::string aFile);

        value_type& operator*() const;
        Precompiler& operator++();
        Precompiler operator++(int);
        bool operator==(std::nullptr_t aEnd);
        bool operator==(const Precompiler& aOther) const = default;

      private:
        void PushFile(std::string aFile);
        void Next();
        void Process(pattern_matcher::Success<inner_iterator> aSegment);

        enum class State 
        {
            Empty,
            HasItem,
            Done
        };

        static pattern_matcher::PatternMatcher<std::string> ourPreprocessor;

        State myState;
        mutable std::optional<value_type> myCurrentItem;

        struct FileSegmentState
        {
            pattern_matcher::Success<inner_iterator> myMatch;
            size_t myIndex;
        };

        std::stack<std::stack<FileSegmentState>> myFiles;
    };

	using PrecompilationOutput = SentinelRange<Precompiler>;

	PrecompilationOutput Precompile(std::string aFile);

    namespace tmp
    {
        using FileReader = ReIterator<LineJoiner>;

        static_assert(std::input_iterator<LineReader>);
        static_assert(std::input_iterator<LineJoiner>);
        static_assert(std::input_iterator<FileReader>);

        using inner_iterator = UnpackingIterator<FileReader>;

        using value_type = SimpleRangeWrapper<inner_iterator, inner_iterator>;
    }
}
