#ifndef COMPILER_PATTERNS_H
#define COMPILER_PATTERNS_H

#include "Pattern.h"

#include <vector>
#include <iostream>
#include <variant>
#include <string>

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
	// https://alx71hub.github.io/hcb/

	template<class T>
	using NotNullPtr = std::shared_ptr<T>;

	template<class T>
	using MaybeNullPtr = std::shared_ptr<T>;

	struct TypeSpecifierSequence;
	struct AbstractDeclarator;

	// type-id
	struct TypeId
	{
		NotNullPtr<TypeSpecifierSequence> myTypeSpecifierSequence = nullptr;
		MaybeNullPtr<AbstractDeclarator> myAbstractDeclarator = nullptr;
	};

	// identifier
	using Identifier = const tokenizer::Token*;

	// block-declaration
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

		Identifier myIdentifier = nullptr;

		std::optional<AttributeArgumentClause> myArgumentClause;
	};

	struct AttributeList
	{
		std::vector<Attribute> myAttributes;

		const tokenizer::Token* myEllipsis = nullptr;
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

	using AttributeSpecifierSequence = std::vector<std::variant<AttributeSpecifier, AlignmentSpecifier>>;

	struct NestedNameSpecifier
	{
	
	};


	struct EnumName
	{
	};

	struct TypedefName
	{
	};

	struct SimpleTemplateId
	{
	};

	// type-name
	// class-name, enum-name, and typedef-name are all baked into Identifier and SimpleTemplateId here
	using Typename = std::variant<Identifier, SimpleTemplateId>;

	struct DecltypeSpecifier
	{
	
	};

	struct SimpleTypeSpecifier_Typename
	{
		const tokenizer::Token* myColonColon = nullptr;
		std::optional<NestedNameSpecifier> myNameSpecifier;
		Typename myTypename;
	};

	struct SimpleTypeSpecifier_TemplateTypename
	{
		const tokenizer::Token* myColonColon = nullptr;
		NestedNameSpecifier myNameSpecifier;
		SimpleTemplateId mySimpleTemplateId;
	};

	struct SimpleTypeSpecifier_Builtin
	{
		const tokenizer::Token* myType = nullptr;
	};

	// simple-type-specifier
	using SimpleTypeSpecifier = std::variant<SimpleTypeSpecifier_Typename, SimpleTypeSpecifier_TemplateTypename, SimpleTypeSpecifier_Builtin, DecltypeSpecifier>;

	struct ElaborateTypeSpecifier
	{
	};

	struct TypenameSpecifier
	{
	};

	struct CVQualifier
	{
	};

	// trailing-type-specifier
	using TrailingTypeSpecifier = std::variant<SimpleTypeSpecifier, ElaborateTypeSpecifier, TypenameSpecifier, CVQualifier>;

	struct ClassSpecifier
	{
	};

	struct EnumSpecifier
	{
	};

	struct AbstractDeclarator
	{
	};

	// type-specifier
	using TypeSpecifier = std::variant<TrailingTypeSpecifier, ClassSpecifier, EnumSpecifier>;

	// type-specifier-seq
	struct TypeSpecifierSequence
	{
		std::vector<TypeSpecifier> myTypeSpecifiers;
		std::optional<AttributeSpecifierSequence> myAttributeSpecifierSequence;
	};

	struct AttributeDeclaration
	{
		AttributeSpecifierSequence mySpecifiers;
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
	
	template<std::same_as<const tokenizer::Token*>... Types> 
	std::string Tokens(std::string aSeparator, Types... aOthers)
	{
		using namespace std::string_literals;
		bool first = true;
		return  ((aOthers ? ((first ? (first = false, ""s) : aSeparator) + aOthers->myRawText) : ""s) + ...);
	}

	void operator<<(std::ostream& aStream, const TranslationUnit& aTranslationUnit);
	void operator<<(std::ostream& aStream, const BlockDeclaration& aDeclaration);
	void operator<<(std::ostream& aStream, const FunctionDeclaration& aDeclaration);
	void operator<<(std::ostream& aStream, const TemplateDeclaration& aDeclaration);
	void operator<<(std::ostream& aStream, const ExplicitInstantiation& aDeclaration);
	void operator<<(std::ostream& aStream, const ExplicitSpecialization& aDeclaration);
	void operator<<(std::ostream& aStream, const LinkageSpecification& aDeclaration);
	void operator<<(std::ostream& aStream, const NamespaceDefinition& aDeclaration);
	void operator<<(std::ostream& aStream, const EmptyDeclaration& aDeclaration);
	void operator<<(std::ostream& aStream, const Attribute& aAttribute);
	void operator<<(std::ostream& aStream, const AttributeDeclaration& aDeclaration);
	void operator<<(std::ostream& aStream, const TypeId& aTypeId);
	void operator<<(std::ostream& aStream, const AssignmentExpression& aExpression);
	void operator<<(std::ostream& aStream, const AttributeSpecifierSequence& aAttributeSpecifireSequence);
	void operator<<(std::ostream& aStream, const TypeSpecifierSequence& aTypeSpecifierSequence);
	void operator<<(std::ostream& aStream, const AbstractDeclarator& aActractDeclarator);
	void operator<<(std::ostream& aStream, const TrailingTypeSpecifier& aTrailingTypeSpecifier);
	void operator<<(std::ostream& aStream, const ClassSpecifier& aClassSpecifier);
	void operator<<(std::ostream& aStream, const EnumSpecifier& aEnumSpecifier);

	void operator<<(std::ostream& aStream, const SimpleTypeSpecifier& aSimpleTypeSpecifier);
	void operator<<(std::ostream& aStream, const ElaborateTypeSpecifier& aElaborateTypeSpecifier);
	void operator<<(std::ostream& aStream, const TypenameSpecifier& aTypenameSpecifier);
	void operator<<(std::ostream& aStream, const CVQualifier& aCVQualifier);

	void operator<<(std::ostream& aStream, const SimpleTypeSpecifier_Typename& aTypeName);
	void operator<<(std::ostream& aStream, const SimpleTypeSpecifier_TemplateTypename& aTemplatedTypename);
	void operator<<(std::ostream& aStream, const SimpleTypeSpecifier_Builtin& aBuiltin);
	void operator<<(std::ostream& aStream, const DecltypeSpecifier& aDecltypeSpecifier);

	void operator<<(std::ostream& aStream, const NestedNameSpecifier& aNestedNameSpecifier);
	void operator<<(std::ostream& aStream, const Typename& aTypename);

}

#endif // !COMPILER_SCOPE_H
