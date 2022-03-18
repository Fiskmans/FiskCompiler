#ifndef COMMON_COMPILER_CONTEXT_H
#define COMMON_COMPILER_CONTEXT_H

#include <string>
#include <stack>
#include <vector>
#include <string_view>
#include <unordered_map>

class CompilerContext
{
public:
	static void EmitWarning(const std::string& aMessage, size_t aColumn, size_t aLine = myCurrentLine, size_t aSize = 1);
	static void EmitError(const std::string& aMessage, size_t aColumn, size_t aLine = myCurrentLine, size_t aSize = 1);

	static void SetPrintContext(const std::vector<std::string>& aPrintContext);
	static void SetCurrentLine(size_t aLine);

	static void PushFile(const std::string_view& aFile);
	static void PopFile();

	static bool HasErrors() { return myHasErrors; };

	static std::vector<const char*> ParseCommandLine(int argc, char** argv);

	static bool HasFlag(const std::string_view& aFlag);
	static std::string GetFlag(const std::string_view& aFlag);

private:
	static bool											myHasErrors;
	static size_t										myCurrentLine;
	static std::stack<std::string>						myFileStack;
	static std::vector<std::string>						myPrintContext;
	static std::unordered_map<std::string, std::string> myFlags;
};

#endif