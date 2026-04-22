
#include "tokenizer/tokenStream.h"

#include "precompiler/Types.h"

#include "precompiler/LineReader.h"
#include "precompiler/LineJoiner.h"

namespace fisk::precompiler
{
	

	inline auto Precompile(std::string aFile)
	{
		return SentinelRange(
			ReIterator(
				LineJoiner(
				std::make_shared<LineReader>(
					std::make_shared<std::ifstream>(aFile), 
					aFile))
				));
	}
}
