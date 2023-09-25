#ifndef COMMON_COMPILER_CONTEXT_H
#define COMMON_COMPILER_CONTEXT_H

#include <string>
#include <stack>
#include <vector>
#include <string_view>
#include <unordered_map>
#include <optional>
#include <filesystem>

#include "tokenizer/token.h"
#include "common/FeatureSwitch.h"

namespace {
	thread_local size_t dummy;
}

std::string Escape(std::string aString, size_t& aOutEscapeCount = dummy);
std::string Dequote(std::string aString);

class CompilerContext
{
public:
	static void EmitWarning(const std::string& aMessage, const tokenizer::Token& aToken);
	static void EmitWarning(const std::string& aMessage, std::filesystem::path aFile = myFileStack.top(), size_t aColumn = npos, size_t aLine = myCurrentLine, size_t aSize = 1);

	static void EmitError(const std::string& aMessage, const tokenizer::Token& aToken);
	static void EmitError(const std::string& aMessage, std::filesystem::path aFile = myFileStack.top(), size_t aColumn = npos, size_t aLine = myCurrentLine, size_t aSize = 1);

	static std::optional<std::filesystem::path> FindFile(const std::filesystem::path& aPath, bool aExpandedLookup = false);

	static void SetPrintContext(const std::vector<std::string>& aPrintContext);
	static void SetCurrentLine(size_t aLine);
	static size_t GetCurrentLine();

	static void PushFile(const std::filesystem::path& aFile);
	static void PopFile();

	static std::filesystem::path GetCurrentFile();

	static bool HasErrors() { return myHasErrors; };

	static std::vector<std::filesystem::path> ParseCommandLine(int argc, char** argv);

	static std::optional<const std::string> GetFlag(const std::string_view& aFlag);

	static bool IsWarningEnabled(const std::string& aWarning);

	const static size_t npos = ~(0ull);

private:

	static FeatureSwitch								myWarningSwitches;
	static bool											myHasErrors;
	static size_t										myCurrentLine;
	static std::stack<std::filesystem::path>			myFileStack;
	static std::vector<std::string>						myPrintContext;
	static std::stack<std::vector<std::string>>			myPrintContextStack;
	static std::vector<std::filesystem::path>			myBaseDirectories;
	static std::vector<std::filesystem::path>			myAdditionalDirectories;
	static std::unordered_map<std::string, std::string> myFlags;
};

#endif