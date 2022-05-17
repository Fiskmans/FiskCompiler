
#include "tokenizer/tokenStream.h"

#include <vector>
#include <ranges>
#include <unordered_map>
#include <functional>
#include <iostream>

#include "common/RangeCollapse.h"

using PreprocessorNumber = long long;

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

	struct Macro
	{
		template<std::ranges::input_range TokenCollection>
		Macro(TokenCollection aRange);

		template<std::forward_iterator IteratorType>
		std::vector<Token> Evaluate(IteratorType& aInOutBegin, const IteratorType& aEnd);

		struct Component
		{
			enum class Type
			{
				Token,
				Argument,
				VariadicExpansion
			};

			std::optional<Token> myToken;
			size_t myArgumentIndex;
			Type myType;
		};

		std::string myIdentifier;
		bool myHasVariadic = false;
		size_t myArguments = 0;
		std::vector<Component> myTokens;
	};

	struct Context
	{
		std::unordered_map<std::string, Macro> myMacros;
	};
	
	template<std::ranges::contiguous_range TokenCollection>
	static std::vector<Token> TranslateTokenRange(TokenCollection aTokens);
	
	static void IncludeFile(TokenStream& aOutTokens, const std::vector<Token>& aTokens, std::vector<Token>::const_iterator aIncludeIt);

	template<std::ranges::contiguous_range TokenCollection>
	static IfState EvaluateExpression(TokenCollection aTokens);
	
	template<std::ranges::contiguous_range TokenCollection>
	static PreprocessorNumber EvalutateSequence(TokenCollection aTokens);
	
	template<std::forward_iterator IteratorType>
	static IteratorType FindMatchingEndParen(IteratorType aBegin, IteratorType aEnd);

	template<std::ranges::contiguous_range TokenCollection>
	static void Define(TokenCollection aTokens);


	static Context myContext;
};
