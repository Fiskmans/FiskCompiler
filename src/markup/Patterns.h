#ifndef COMPILER_PATTERNS_H
#define COMPILER_PATTERNS_H

#include "Pattern.h"

#include <vector>
#include <iostream>
#include <variant>

namespace markup 
{
	inline std::string Demangle(std::string aClassName)
	{
		std::string s = aClassName;
		if (s.starts_with("struct "))
			s = s.substr(7);

		if (s.starts_with("class "))
			s = s.substr(6);

		if (s.starts_with("markup::"))
			s = s.substr(8);

		return s;
	}
	
	template <class Head>
	inline std::string PrettyNameClasses()
	{
		return Demangle(typeid(Head).name());
	}

	template<class Head, class Second, class... Tail>
	inline std::string PrettyNameClasses()
	{
		return Demangle(typeid(Head).name()) + ", " + PrettyNameClasses<Second, Tail...>();
	}

	struct Token
	{
		Token(tokenizer::Token::Type aType)
		{
			if (MarkupGlobals::StateStack.top().GetToken().myType != aType)
			{
				MarkupGlobals::StateStack.top().Error("Invalid token, expected: " + tokenizer::Token::TypeToString(aType));
				return;
			}

			myToken = &MarkupGlobals::StateStack.top().GetToken();
			MarkupGlobals::StateStack.top().Consume();
		}

		const tokenizer::Token* myToken;
	};

	struct BlockDeclaration
	{
		BlockDeclaration()
		{
			MarkupGlobals::StateStack.top().Error("Not yet implemented");
		}
	};

	struct FunctionDeclaration
	{
		FunctionDeclaration()
		{
			MarkupGlobals::StateStack.top().Error("Not yet implemented");
		}
	};

	struct TemplateDeclaration
	{
		TemplateDeclaration()
		{
			MarkupGlobals::StateStack.top().Error("Not yet implemented");
		}
	};

	struct ExplicitInstantiation
	{
		ExplicitInstantiation()
		{
			MarkupGlobals::StateStack.top().Error("Not yet implemented");
		}
	};

	struct ExplicitSpecialization
	{
		ExplicitSpecialization()
		{
			MarkupGlobals::StateStack.top().Error("Not yet implemented");
		}
	};

	struct LinkageSpecification
	{
		LinkageSpecification()
		{
			MarkupGlobals::StateStack.top().Error("Not yet implemented");
		}
	};

	struct NamespaceDefinition
	{
		NamespaceDefinition()
		{
			MarkupGlobals::StateStack.top().Error("Not yet implemented");
		}
	};

	struct EmptyDeclaration
	{
		Token mySemicolon = tokenizer::Token::Type::Semicolon;
	};

	struct AttributeDeclaration
	{
		AttributeDeclaration()
		{
			MarkupGlobals::StateStack.top().Error("Not yet implemented");
		}
	};

	struct Declaration
	{
		std::optional<std::variant<
			BlockDeclaration,
			FunctionDeclaration,
			TemplateDeclaration,
			ExplicitInstantiation,
			ExplicitSpecialization,
			LinkageSpecification,
			NamespaceDefinition,
			EmptyDeclaration,
			AttributeDeclaration>>	myDeclaration = AnyOf<	BlockDeclaration,
															FunctionDeclaration,
															TemplateDeclaration,
															ExplicitInstantiation,
															ExplicitSpecialization,
															LinkageSpecification,
															NamespaceDefinition,
															EmptyDeclaration,
															AttributeDeclaration>();
	};

	struct TranslationUnit
	{
		std::vector<Declaration> myDeclarations = UnseperatedSequence<Declaration>();
	};

	TranslationUnit Markup(const std::vector<tokenizer::Token>& aTokens);
	

	void operator<<(std::ostream& aStream, const TranslationUnit& aTranslationUnit);
	void operator<<(std::ostream& aStream, const Declaration& aDeclaration);
	void operator<<(std::ostream& aStream, const BlockDeclaration& aDeclaration);
	void operator<<(std::ostream& aStream, const FunctionDeclaration& aDeclaration);
	void operator<<(std::ostream& aStream, const TemplateDeclaration& aDeclaration);
	void operator<<(std::ostream& aStream, const ExplicitInstantiation& aDeclaration);
	void operator<<(std::ostream& aStream, const ExplicitSpecialization& aDeclaration);
	void operator<<(std::ostream& aStream, const LinkageSpecification& aDeclaration);
	void operator<<(std::ostream& aStream, const NamespaceDefinition& aDeclaration);
	void operator<<(std::ostream& aStream, const EmptyDeclaration& aDeclaration);
	void operator<<(std::ostream& aStream, const AttributeDeclaration& aDeclaration);
}

#endif // !COMPILER_SCOPE_H
