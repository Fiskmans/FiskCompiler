
#include "precompiler/Precompiler.h"

#include "pattern_matcher/PatternBuilder.h"
#include "pattern_matcher/PatternMatcher.h"

namespace fisk::precompiler
{

    pattern_matcher::PatternMatcher<std::string> Precompiler::ourPreprocessor;

    PrecompilationOutput Precompile(std::string aFile)
    {
        return PrecompilationOutput{Precompiler(aFile), nullptr};
    }

    Precompiler::Precompiler(std::string aFile) : myState(State::Empty)
    {
        if (ourPreprocessor["preprocessing-file"])
        {
            std::ifstream cppBNFFile("data/cpp.bnf.md");
            std::stringstream cppBNF;
            cppBNF << cppBNFFile.rdbuf();

            ourPreprocessor = pattern_matcher::PatternBuilder::FromBNF(cppBNF.str());
        }

        PushFile(aFile);
    }

    Precompiler::value_type& Precompiler::operator*() const
    {
        assert(myState == State::HasItem && "Improper iterator usage, dereference before checking past end");
        return *myCurrentItem;
    }

    Precompiler& Precompiler::operator++()
    {
        if (myState == State::Empty)
            Next();

        assert(myState != State::Done && "Walked iterator past end");

        myState = State::Empty;
        return *this;
    }

    Precompiler Precompiler::operator++(int)
    {
        Precompiler copy(*this);
        ++(*this);
        return copy;
    }

    bool Precompiler::operator==(std::nullptr_t aEnd)
    {
        if (myState == State::Empty)
            Next();

        return myState == State::Done;
    }

    void Precompiler::PushFile(std::string aFile)
    {
        inner_iterator fileReader =
            ReIterator(LineJoiner(std::make_shared<LineReader>(std::make_shared<std::ifstream>(aFile), aFile)));

        std::optional<pattern_matcher::Success<inner_iterator>> parsed =
            ourPreprocessor.Match(ourPreprocessor["preprocessing-file"], fileReader, nullptr);

        assert(parsed && "this should always be valid because preprocessing-file will match against 0 chars");

        if (parsed->end() != nullptr)
        {
            CompilerContext::EmitError("code-unparsable", *parsed->end());
            return;
        }

        std::stack<FileSegmentState> file;
        file.push({.myMatch = *parsed, .myIndex = 0});
        myFiles.push(file);
    }

    void Precompiler::Next()
    {
        while (myState == State::Empty)
        {
            if (myFiles.size() == 0)
            {
                myState = State::Done;
                return;
            }

            std::stack<FileSegmentState>& file = myFiles.top();
            if (file.size() == 0)
            {
                myFiles.pop();
                continue;
            }

            FileSegmentState& segment = file.top();
            if (segment.myIndex >= segment.myMatch.mySubMatches.size())
            {
                file.pop();
                continue;
            }

            Process(segment.myMatch.mySubMatches[segment.myIndex++]);
        }
    }

    void Precompiler::Process(pattern_matcher::Success<inner_iterator> aSegment)
    {
        const pattern_matcher::Fragment* fragment = aSegment.myFragment;

        if (fragment == ourPreprocessor["text-line"])
        {
            myCurrentItem = {aSegment.begin(), aSegment.end()};
            myState = State::HasItem;
            return;
        }

        static std::vector<const pattern_matcher::Fragment*> fragmentsToExpand{
            ourPreprocessor["preprocessing-file"], ourPreprocessor["group"], ourPreprocessor["group-part"]
        };

        if (std::find(fragmentsToExpand.begin(), fragmentsToExpand.end(), fragment) != fragmentsToExpand.end())
        {
            myFiles.top().push({aSegment, 0});
            return;
        }
    }

    namespace tmp
    {

        using Iter2 = fisk::precompiler::UnpackingIterator<std::vector<int>*>;
        static_assert(std::input_iterator<Iter2>);

        using T = std::vector<SourceLine>::iterator;
        using Iter = fisk::precompiler::UnpackingIterator<T>;
        using Range = fisk::precompiler::SimpleRangeWrapper<Iter, Iter>;
        static_assert(std::input_iterator<Iter>);
        static_assert(std::copyable<Iter>);
        static_assert(std::default_initializable<Iter>);
        static_assert(std::semiregular<Iter>);

        static_assert(std::equality_comparable<T>);

        static_assert(std::equality_comparable<Iter>);
        static_assert(std::regular<Iter>);
        static_assert(std::incrementable<Iter>);
        static_assert(std::forward_iterator<Iter>);
        static_assert(std::indirectly_readable<Iter>);
        static_assert(std::ranges::range<Range>);

        static_assert(std::equality_comparable<Precompiler>);
        static_assert(std::regular<Precompiler>);
        static_assert(std::input_or_output_iterator<Precompiler>);
        static_assert(std::indirectly_readable<Precompiler>);
        static_assert(std::input_iterator<Precompiler>);
    } // namespace tmp

} // namespace fisk::precompiler