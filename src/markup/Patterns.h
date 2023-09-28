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

	struct NestedNameSpecifier
	{
	};
	
	struct SimpleTemplateId
	{
	};

	struct EnumName
	{
	};

	struct TypedefName
	{
	};

	struct DecltypeSpecifier
	{
	};

	struct ElaborateTypeSpecifier
	{
	};

	struct TypenameSpecifier
	{
	};

	// type-name
	// class-name, enum-name, and typedef-name are all baked into Identifier and SimpleTemplateId here
	using Typename = std::variant<Identifier, SimpleTemplateId>;

	struct SimpleTypeSpecifier_Typename
	{
		const tokenizer::Token*				myColonColon = nullptr;
		std::optional<NestedNameSpecifier>	myNameSpecifier;
		Typename							myTypename;
	};

	struct SimpleTypeSpecifier_TemplateTypename
	{
		const tokenizer::Token* myColonColon = nullptr;
		NestedNameSpecifier		myNameSpecifier;
		SimpleTemplateId		mySimpleTemplateId;
	};

	struct SimpleTypeSpecifier_Builtin
	{
		const tokenizer::Token* myType = nullptr;
	};

	// simple-type-specifier
	using SimpleTypeSpecifier	= std::variant<SimpleTypeSpecifier_Typename, SimpleTypeSpecifier_TemplateTypename, SimpleTypeSpecifier_Builtin, DecltypeSpecifier>;

	struct Expression;

	struct IdExpression
	{
	
	};

	struct ExpressionList
	{
	
	};

	struct BracedInitList
	{
	
	};

	struct PseudoDestructorName
	{
	};

	struct CastExpression;

	using Literal = const tokenizer::Token*;

	struct PrimaryExpression_Parenthesis
	{
		const tokenizer::Token* myOpeningParenthesis = nullptr;
		NotNullPtr<Expression> myExpression;
		const tokenizer::Token* myClosingParenthesis = nullptr;
	};

	struct LambdaExpression
	{
	
	};

	using PrimaryExpression = std::variant<
		Literal,
		const tokenizer::Token*, // this
		PrimaryExpression_Parenthesis,
		IdExpression,
		LambdaExpression>;

	struct PostfixExpression;

	struct PostfixExpression_Subscript
	{
		NotNullPtr<PostfixExpression> myLeftHandSide;
		const tokenizer::Token* myOpeningBracket = nullptr;
		std::variant<NotNullPtr<Expression>, std::optional<BracedInitList>> myExpression;
		const tokenizer::Token* myClosingBracket = nullptr;
	};

	struct PostfixExpression_Call
	{
		std::variant<SimpleTypeSpecifier, TypenameSpecifier, NotNullPtr<PostfixExpression>> myLeftHandSide;
		const tokenizer::Token* myOpeningBracket = nullptr;
		std::optional<ExpressionList> myExpression;
		const tokenizer::Token* myClosingBracket = nullptr;
	};

	struct PostfixExpression_BracedInitialization
	{
		std::variant<SimpleTypeSpecifier, TypenameSpecifier> myLeftHandSide;
		BracedInitList myBracedInitializerList;
	};

	struct PostfixExpression_Access
	{
		NotNullPtr<PostfixExpression> myLeftHandSide;
		const tokenizer::Token* myAccessOperator = nullptr; // . ->
		const tokenizer::Token* myTemplate = nullptr;
		IdExpression myIdExpression;
	};

	struct PostfixExpression_Destruct
	{
		NotNullPtr<PostfixExpression> myLeftHandSide;
		const tokenizer::Token* myAccessOperator = nullptr; // . ->
		PseudoDestructorName myPseudoDestructorName;
	};

	struct PostfixExpression_IncDec
	{
		NotNullPtr<PostfixExpression> myLeftHandSide;
		const tokenizer::Token* myOperator; // ++ --
	};

	struct PostfixExpression_Cast
	{
		const tokenizer::Token* myCastType = nullptr; // dynamic_cast static_cast reinterpret_cast const_cast
		const tokenizer::Token* myOpeningLessThan = nullptr;
		TypeId myTypeId;
		const tokenizer::Token* myClosingGreaterThan = nullptr;
		const tokenizer::Token* myOpeningParenthesis = nullptr;
		NotNullPtr<Expression> myExpression;
		const tokenizer::Token* myClosingParenthesis = nullptr;
	};

	struct PostfixExpression_Typeid
	{
		const tokenizer::Token* myTypeid = nullptr;
		const tokenizer::Token* myOpeningParenthesis = nullptr;
		std::variant<TypeId, NotNullPtr<Expression>> myContent;
		const tokenizer::Token* myClosingParenthesis = nullptr;
	};

	struct PostfixExpression
	{
		std::variant<
			PrimaryExpression, 
			PostfixExpression_Subscript, 
			PostfixExpression_Call, 
			PostfixExpression_Call, 
			PostfixExpression_BracedInitialization, 
			PostfixExpression_Access, 
			PostfixExpression_Destruct, 
			PostfixExpression_IncDec, 
			PostfixExpression_Cast, 
			PostfixExpression_Typeid> myContent;
	};

	struct UnaryExpression_PrefixExpression
	{
		const tokenizer::Token* myOperator = nullptr; // ++ -- * & + - ! ~
		NotNullPtr<CastExpression> myRightHandSide;
	};

	struct UnaryExpression_sizeof
	{
	};

	struct UnaryExpression_alignof
	{
	};

	struct NoexceptExpression
	{
	};

	struct NewExpression
	{
	};

	struct DeleteExpression
	{
	};

	using UnaryExpression = std::variant<PostfixExpression, UnaryExpression_PrefixExpression, UnaryExpression_sizeof, UnaryExpression_alignof, NoexceptExtression, NewExpression, DeleteExpression>;

	struct CastExpression_recurse
	{
		const tokenizer::Token* myOpeningParenthesis = nullptr;
		TypeId myTypeId;
		const tokenizer::Token* myClosingParenthesis = nullptr;
		NotNullPtr<CastExpression> myRightHandSide;
	};

	struct CastExpression
	{
		std::variant<UnaryExpression, CastExpression_recurse> myContent;
	};

	struct PMExpression
	{
		MaybeNullPtr<PMExpression> myLeftHandSide;
		const tokenizer::Token* myDereferenceOperator = nullptr; // .* ->*

		CastExpression myRightHandSide;
	};

	struct MultiplicativeExpression
	{
		MaybeNullPtr<MultiplicativeExpression> myLeftHandSide;
		const tokenizer::Token* myMultiplicationOperator = nullptr; // * / %

		PMExpression myRightHandSide;
	};

	struct AddativeExpression
	{
		MaybeNullPtr<AddativeExpression> myLeftHandSide;
		const tokenizer::Token* myAdditionOperator = nullptr; // - +

		MultiplicativeExpression myRightHandSide;
	};

	struct ShiftExpression
	{
		MaybeNullPtr<ShiftExpression> myLeftHandSide;
		const tokenizer::Token* myShiftOperator = nullptr; // << >>

		AddativeExpression myRightHandSide;
	};

	struct RelationalExpression
	{
		MaybeNullPtr<RelationalExpression> myLeftHandSide;
		const tokenizer::Token* myRelationOperator = nullptr; // < > <= >=

		ShiftExpression myRightHandSide;
	};

	struct EqualityExpression
	{
		MaybeNullPtr<EqualityExpression> myLeftHandSide;
		const tokenizer::Token* myEqualityOperator = nullptr; // == !=

		RelationalExpression myRightHandSide;
	};

	struct AndExpression
	{
		MaybeNullPtr<AndExpression> myLeftHandSide;
		const tokenizer::Token* myAnd = nullptr; // &

		EqualityExpression myRightHandSide;
	};

	struct ExclusiveOrExpression
	{
		MaybeNullPtr<ExclusiveOrExpression> myLeftHandSide;
		const tokenizer::Token* myXor = nullptr; // ^

		AndExpression myRightHandSide;
	};

	// inclusive-or-expression
	struct InclusiveOrExpression
	{
		MaybeNullPtr<InclusiveOrExpression> myLeftHandSide;
		const tokenizer::Token* myOr = nullptr; // |

		ExclusiveOrExpression myRightHandSide;
	};

	// logical-and-expression
	struct LogicalAndExpression
	{
		MaybeNullPtr<LogicalAndExpression> myLeftHandSide;
		const tokenizer::Token* myAndAnd = nullptr; // &&

		InclusiveOrExpression myRightHandSide;
	};

	// logical-or-expression
	struct LogicalOrExpression
	{
		MaybeNullPtr<LogicalOrExpression> myLeftHandSide;
		const tokenizer::Token* myOrOr = nullptr; // ||

		LogicalAndExpression myRightHandSide;
	};

	using AssingmentOperator = const tokenizer::Token*;

	struct InitializerClause
	{
	};

	struct AssignmentExpression_Assignment
	{
		LogicalOrExpression myLogicalOrExpression;
		AssingmentOperator myOperator;
		InitializerClause myInitializerClause;
	};

	struct ThrowExpression
	{
	};

	struct ConditionalExpression;

	// assignment-expression
	using AssignmentExpression = std::variant<ConditionalExpression, AssignmentExpression_Assignment, ThrowExpression>;

	struct ConditionalExpression
	{
		LogicalOrExpression myCondition;
		const tokenizer::Token* myQuestionMark	= nullptr;
		MaybeNullPtr<Expression> myOnTruthy;
		const tokenizer::Token* myColon = nullptr;
		MaybeNullPtr<AssignmentExpression> myOnFalsy;
	};


	struct Expression
	{
		MaybeNullPtr<Expression> myLeftHandSide;
		const tokenizer::Token* myComma = nullptr; // ,

		AssignmentExpression myRightHandSide;
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
