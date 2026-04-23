
#include "tokenizer/tokenStream.h"

#include "precompiler/Types.h"
#include "precompiler/LineJoiner.h"

namespace fisk::precompiler
{
	using PrecompilationOutput = SentinelRange<ReIterator<LineJoiner>>;

	PrecompilationOutput Precompile(std::string aFile);
}
