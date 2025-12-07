
#include "tokenizer/tokenStream.h"

#include <vector>
#include <ranges>
#include <unordered_map>
#include <functional>
#include <iostream>

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
	static void ConsumeLine(FileContext& aFileContext, tokenizer::TokenStream& aOutTokens, const std::vector<tokenizer::Token>& aTokens);

private:

	struct Macro
	{
		template<std::ranges::input_range TokenCollection>
		Macro(TokenCollection aRange);

		template<std::forward_iterator IteratorType>
		std::optional<std::vector<tokenizer::Token>> Evaluate(IteratorType& aInOutBegin, const IteratorType& aEnd);

		struct Component
		{
			enum class Type
			{
				Token,
				Argument,
				VariadicExpansion
			};

			std::optional<tokenizer::Token> myToken;
			size_t myArgumentIndex;
			Type myType;
		};

		std::string myIdentifier;
		bool myHasVariadic = false;
		size_t myArguments = 0;
		std::vector<Component> myComponents;
	};

	struct Context
	{
		std::unordered_map<std::string, Macro> myMacros;
	};
	
	template<std::ranges::contiguous_range TokenCollection>
	static std::vector<tokenizer::Token> TranslateTokenRange(TokenCollection aTokens);
	
	static void IncludeFile(tokenizer::TokenStream& aOutTokens, const std::vector<tokenizer::Token>& aTokens, std::vector<tokenizer::Token>::const_iterator aIncludeIt);

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
