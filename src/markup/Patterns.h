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
		const tokenizer::Token* myToken;
	};

	struct BlockDeclaration
	{
	};

	struct FunctionDeclaration
	{
	};

	struct TemplateDeclaration
	{
	};

	struct ExplicitInstantiation
	{
	};

	struct ExplicitSpecialization
	{
	};

	struct LinkageSpecification
	{
	};

	struct NamespaceDefinition
	{
	};

	struct EmptyDeclaration
	{
		const tokenizer::Token* mySemicolon = nullptr;
	};

	struct AttributeArgumentClause
	{
		const tokenizer::Token* myOpeningParenthesis = nullptr;

		std::vector<const tokenizer::Token*> myBalancedTokenSequence;

		const tokenizer::Token* myClosingParenthesis = nullptr;
	};

	struct Attribute
	{
		const tokenizer::Token* myAttributeNamespace = nullptr;
		
		const tokenizer::Token* myColonColon = nullptr;

		const tokenizer::Token* myIdentifier = nullptr;

		std::optional<AttributeArgumentClause> myArgumentClause;
	};

	struct AttributeList
	{
		std::vector<Attribute> myAttributes;

		const tokenizer::Token* myEllipsis = nullptr;
	};

	struct TypeId
	{
	};

	struct AssignmentExpression
	{
	
	};

	struct AlignmentSpecifier
	{
		const tokenizer::Token* myAlignas = nullptr;
		const tokenizer::Token* myOpeningParen = nullptr;

		std::variant<TypeId, AssignmentExpression> myContent; 

		const tokenizer::Token* myEllipsis = nullptr;
		const tokenizer::Token* myClosingParen = nullptr;
	};

	
	struct AttributeSpecifier
	{
		const tokenizer::Token* myOuterOpening = nullptr;
		const tokenizer::Token* myInnerOpening = nullptr;
		AttributeList myAttributeList;
		const tokenizer::Token* myInnerClosing = nullptr;
		const tokenizer::Token* myOuterClosing = nullptr;
	};

	struct AttributeDeclaration
	{
		std::vector<std::variant<AttributeSpecifier, AlignmentSpecifier>> mySpecifiers;
		const tokenizer::Token* mySemicolon = nullptr;
	};

	struct TranslationUnit
	{
		std::vector<std::variant<
			BlockDeclaration,
			FunctionDeclaration,
			TemplateDeclaration,
			ExplicitInstantiation,
			ExplicitSpecialization,
			LinkageSpecification,
			NamespaceDefinition,
			EmptyDeclaration,
			AttributeDeclaration>> myDeclarations;
	};

	TranslationUnit Markup(const std::vector<tokenizer::Token>& aTokens);
	

	void operator<<(std::ostream& aStream, const TranslationUnit& aTranslationUnit);
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
