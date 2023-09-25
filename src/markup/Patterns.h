#ifndef COMPILER_PATTERNS_H
#define COMPILER_PATTERNS_H

#include "Pattern.h"

#include <vector>
#include <iostream>
#include <variant>

alignas(throw);

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
		const tokenizer::Token* mySemicolon;
	};

	struct Attribute
	{

	};

	struct AttributeList
	{
		std::vector<Attribute> myAttributes;

		const tokenizer::Token* myEllipsis;
	};

	struct AttributeSpecifier
	{
		bool isAlingas = false;

		const tokenizer::Token* myOuterOpening;
		const tokenizer::Token* myInnerOpening;
		AttributeList myAttributeList;
		const tokenizer::Token* myInnerClosing;
		const tokenizer::Token* myOuterClosing;

		asdas
	};

	struct AttributeDeclaration
	{
		std::vector<AttributeSpecifier> mySpecifiers;
		const tokenizer::Token* mySemicolon;
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
