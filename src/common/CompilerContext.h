#ifndef COMMON_COMPILER_CONTEXT_H
#define COMMON_COMPILER_CONTEXT_H

#include <string>
#include <stack>
#include <vector>
#include <string_view>
#include <unordered_map>
#include <optional>
#include <filesystem>

#include "common/FeatureSwitch.h"
#include "precompiler/Types.h"
#include "precompiler/LineReader.h"

namespace {
	thread_local size_t dummy;
}

std::string Escape(std::string aString, size_t& aOutEscapeCount = dummy);
std::string Dequote(std::string aString);

class CompilerContext
{
public:

	static void EmitWarning(const std::string &aTag, fisk::precompiler::SourceChar aAt, fisk::precompiler::SourceChar aUntil = {});
	static void EmitError(const std::string &aTag, fisk::precompiler::SourceChar aAt, fisk::precompiler::SourceChar aUntil = {});

	static void RegisterFile(fisk::precompiler::ReIterator<fisk::precompiler::LineReader> aFileStart, const std::string& aFilePath);

	static bool HasErrors() { return myHasErrors; };

	static std::vector<std::filesystem::path> ParseCommandLine(int argc, char** argv);

	static std::optional<const std::string> GetFlag(const std::string_view& aFlag);

	static bool IsWarningEnabled(const std::string& aWarning);

	const static size_t npos = ~(0ull);

	class IgnoreHandle
	{
	public:
		IgnoreHandle(size_t& aIgnoreDepthPtr);
		~IgnoreHandle();
	private:
		size_t& myIgnoreDepth;
	};

	static IgnoreHandle IgnoreErrors();

private:
    enum class ConsoleColor
    {
		Reset,
        Red,
        Yellow
    };

    static void PrintContext(std::ostream &aStream, ConsoleColor aColor, fisk::precompiler::SourceChar aAt, fisk::precompiler::SourceChar aUntil);
    static void ChangeColor(std::ostream &aStream, ConsoleColor aColor);
    static bool ShouldWarn(const std::string& aTag);

	static std::unordered_map<std::string, fisk::precompiler::ReIterator<fisk::precompiler::LineReader>> myFiles;
	static FeatureSwitch								myWarningSwitches;
	static size_t										myIgnoreDepth;
	static bool											myHasErrors;
	static std::vector<std::filesystem::path>			myBaseDirectories;
	static std::vector<std::filesystem::path>			myAdditionalDirectories;
	static std::unordered_map<std::string, std::string> myFlags;
};

#endif