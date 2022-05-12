
#include "tokenizer/tokenStream.h"

#include <vector>
#include <unordered_map>

class Precompiler
{
	enum class IfState
	{
		Active,
		Inactive,
		HasBeenActive
	};

public:
	class FileContext
	{
	public:
		FileContext();
		~FileContext();

	private:
		friend Precompiler;
		std::stack<IfState> myIfStack;
	};

	static void ResetContext();
	static void ConsumeLine(FileContext& aFileContext, TokenStream& aOutTokens, const std::vector<Token>& aTokens);

private:
	using PreprocessorNumber = long long;

	struct Context
	{
		std::unordered_map<std::string, std::vector<Token>> myMacros;
	};

	static std::vector<Token> TranslateToken(const Token& aToken);
	static void IncludeFile(TokenStream& aOutTokens, const std::vector<Token>& aTokens, std::vector<Token>::const_iterator aIncludeIt);
	static IfState EvaluateExpression(std::vector<Token>::const_iterator aBegin, std::vector<Token>::const_iterator aEnd);
	static PreprocessorNumber EvalutateSequence(std::vector<Token>::const_iterator aBegin, std::vector<Token>::const_iterator aEnd);
	static std::vector<Token>::const_iterator FindMatchingEndParen(std::vector<Token>::const_iterator aBegin, std::vector<Token>::const_iterator aEnd);



	static Context myContext;
};