#ifndef COMPILER_PATTERN_H
#define COMPILER_PATTERN_H

#include "common/CompilerContext.h"

#include <variant>
#include <vector>

namespace markup
{
	struct MarkupState
	{
		using iterator = std::vector<tokenizer::Token>::const_iterator;
		
		MarkupState(const std::vector<tokenizer::Token>& aTokens)
		{
			myBegin = aTokens.begin();
			myEnd = aTokens.end();
		}

		bool Empty()
		{
			return myBegin == myEnd;
		}

		const tokenizer::Token& GetToken()
		{
			if (Empty())
			{
				Error("Unexpected end of file");
				return tokenizer::Token::SafetyToken;
			}

			return *myBegin;
		}

		void Consume()
		{
			if (Empty())
			{
				Error("Unexpected end of file");
				return;
			}
			myBegin++;
		}

		void Error(std::string aError)
		{
			myError = aError;
		}

		bool HasError()
		{
			return myError != "";
		}

		void CommitError()
		{
			if (!HasError())
			{
				CompilerContext::EmitError("Internal error, error was committed when none had happened", tokenizer::Token::SafetyToken);
				return;
			}
			
			if (Empty())
			{
				CompilerContext::EmitError(myError, tokenizer::Token::SafetyToken);
				return;
			}

			CompilerContext::EmitError(myError, GetToken());
		}

	private:
		iterator myBegin;
		iterator myEnd;

		std::string myError;
	};

	class MarkupGlobals
	{
	public:
		static std::stack<MarkupState> StateStack;
	};


	template<class Type>
	std::vector<Type> UnseperatedSequence()
	{
		std::vector<Type> out;
		MarkupState currentState = MarkupGlobals::StateStack.top();
		while (true)
		{
			MarkupGlobals::StateStack.push(currentState);
			Type t;

			if (MarkupGlobals::StateStack.top().HasError())
			{
				MarkupGlobals::StateStack.top().CommitError();
				break;
			}

			currentState = MarkupGlobals::StateStack.top();
			MarkupGlobals::StateStack.pop();

			out.push_back(t);

			if (currentState.Empty())
				break;
		}
		MarkupGlobals::StateStack.top() = currentState;
		return out;
	}

	template<class... Types>
	std::optional<std::variant<Types...>> AnyOf()
	{
		std::optional<std::variant<Types...>> out;
		MarkupState currentState = MarkupGlobals::StateStack.top();

		MarkupState nextState = currentState;

		int successes = 0;

		(
			(													// for each type
				MarkupGlobals::StateStack.push(currentState),			// push next state
				out = Types(),									// construct
				(MarkupGlobals::StateStack.top().HasError() ? 0 :			// if not failed, i.e success
					((	successes++,							// count up successes
						nextState = MarkupGlobals::StateStack.top()),0)),	// store state
				MarkupGlobals::StateStack.pop()							// pop state
			)
			, ... 
		);

		if (successes == 0)
		{
			MarkupGlobals::StateStack.top().Error( 
				"Invalid token sequence, doesn't match any valid options, candidates: " + PrettyNameClasses<Types...>());
			return out;
		}

		if (successes > 1)
		{
			MarkupGlobals::StateStack.top().Error("Is ambiguous, candidates: " + PrettyNameClasses<Types...>());
			return out;
		}

		MarkupGlobals::StateStack.top() = nextState;

		return out;
	}

}

#endif // !COMPILER_SCOPE_H
