
#include "tokenizer/tokenStream.h"

#include "precompiler/Types.h"
#include "precompiler/FileReader.h"

namespace fisk::precompiler
{
	inline auto Precompile(std::string aFile)
	{
		return SimpleRangeWrapper{ FileReader(aFile), nullptr };
	}
}
