#ifndef COMPILER_PATTERNS_H
#define COMPILER_PATTERNS_H

#include "markup/Pattern.h"

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

	struct EmptyDeclaration
	{
		const tokenizer::Token* mySemicolon = nullptr;
	};

	struct BalancedToken_Braced;

	using BalancedToken = std::variant<BalancedToken_Braced, const tokenizer::Token*>;

	struct BalancedToken_Braced
	{
		const tokenizer::Token* myOpener = nullptr;
		NotNullPtr<BalancedToken> myContent = nullptr;
		const tokenizer::Token* myCloser = nullptr;
	};

	struct AttributeArgumentClause
	{
		const tokenizer::Token* myOpeningParenthesis = nullptr;

		std::vector<BalancedToken> myBalancedTokenSequence;

		const tokenizer::Token* myClosingParenthesis = nullptr;
	};

	struct Attribute
	{
		const tokenizer::Token* myAttributeNamespace = nullptr;
		const tokenizer::Token* myColonColon = nullptr;

		Identifier myIdentifier = nullptr;

		const tokenizer::Token* myEllipsis = nullptr;
		std::optional<AttributeArgumentClause> myArgumentClause;
	};

	struct AttributeList
	{
		std::vector<std::variant<Attribute,nullptr_t>> myAttributes;
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

	struct InitializerList
	{
	};

	using ExpressionList = InitializerList;

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
		const tokenizer::Token*, // this or literal
		PrimaryExpression_Parenthesis,
		IdExpression,
		LambdaExpression>;


	struct PostfixExpression;

	struct PostfixExpression_Subscript
	{
		NotNullPtr<PostfixExpression> myLeftHandSide;
		const tokenizer::Token* myOpeningBracket = nullptr;
		std::variant<NotNullPtr<Expression>, std::optional<BracedInitList>> myAccess;
		const tokenizer::Token* myClosingBracket = nullptr;
	};

	struct PostfixExpression_Call
	{
		NotNullPtr<PostfixExpression> myLeftHandSide;
		const tokenizer::Token* myOpeningParenthesis = nullptr;
		std::optional<ExpressionList> myExpression;
		const tokenizer::Token* myClosingParenthesis = nullptr;
	};

	struct PostfixExpression_Construct
	{
		std::variant<SimpleTypeSpecifier, TypenameSpecifier> myLeftHandSide;

		const tokenizer::Token* myOpener = nullptr; // ( {
		InitializerList myArguments;
		const tokenizer::Token* myCloser = nullptr; // ) }
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
			PostfixExpression_Construct, 
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

	using UnaryExpression = std::variant<PostfixExpression, UnaryExpression_PrefixExpression, UnaryExpression_sizeof, UnaryExpression_alignof, NoexceptExpression, NewExpression, DeleteExpression>;

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
	
	struct NamespaceDefinition;
	struct LinkageSpecification;
	struct ExplicitSpecialization;
	struct ExplicitInstantiation;
	struct TemplateDeclaration;

	using Declaration = std::variant<
			BlockDeclaration,
			FunctionDeclaration,
			TemplateDeclaration,
			ExplicitInstantiation,
			ExplicitSpecialization,
			LinkageSpecification,
			NamespaceDefinition,
			EmptyDeclaration,
			AttributeDeclaration>;

	struct TemplateType;

	struct TypeParameter
	{
		MaybeNullPtr<TemplateType> myBaseTemplateType;
		const tokenizer::Token* myClassOrTypename = nullptr;
		const tokenizer::Token* myEllipsis = nullptr; // exclusive with default
		Identifier myIdentifer = nullptr;

		const tokenizer::Token* myDefaultEquals = nullptr;
		std::optional<IdExpression> myDefaultExpression;
	};

	struct ParameterDeclaration
	{
	};

	using TemplateParameter = std::variant<TypeParameter, ParameterDeclaration>;

	struct TemplateType
	{
		const tokenizer::Token* myTemplate = nullptr;
		const tokenizer::Token* myOpening = nullptr;
		std::vector<TemplateParameter> myParamaters;
		const tokenizer::Token* myClosing = nullptr;
	};

	struct TemplateDeclaration
	{
		TemplateType myType;
		NotNullPtr<Declaration> myDeclaration;
	};

	struct ExplicitInstantiation
	{
		const tokenizer::Token* myExtern = nullptr;
		const tokenizer::Token* myTemplate = nullptr;
		NotNullPtr<Declaration> myDeclaration;
	};

	struct ExplicitSpecialization
	{
		const tokenizer::Token* myTemplate = nullptr;
		const tokenizer::Token* myOpening = nullptr;
		const tokenizer::Token* myClosing = nullptr;
		NotNullPtr<Declaration> myDeclaration;
	};

	struct LinkageSpecification_block
	{
		const tokenizer::Token* myOpeningBrace = nullptr;
		std::vector<Declaration> myDeclarations;
		const tokenizer::Token* myClosingBrace = nullptr;
	};

	struct LinkageSpecification
	{
		const tokenizer::Token* myExtern = nullptr;
		const tokenizer::Token* myString = nullptr;
		std::variant<LinkageSpecification_block, NotNullPtr<Declaration>> myDeclaration;
	};

	struct NamespaceDefinition
	{
		const tokenizer::Token* myInline = nullptr;
		const tokenizer::Token* myNamespace = nullptr;
		Identifier myIdentifier = nullptr;
		const tokenizer::Token* myOpeningBrace = nullptr;
		std::vector<Declaration> myDeclarations;
		const tokenizer::Token* myClosingBrace = nullptr;
	};

	struct TranslationUnit
	{
		std::vector<Declaration> myDeclarations;
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
	void operator<<(std::ostream& aStream, const Declaration& aDeclaration);
	void operator<<(std::ostream& aStream, const BlockDeclaration& aDeclaration);
	void operator<<(std::ostream& aStream, const FunctionDeclaration& aDeclaration);
	void operator<<(std::ostream& aStream, const TemplateDeclaration& aDeclaration);
	void operator<<(std::ostream& aStream, const ExplicitInstantiation& aDeclaration);
	void operator<<(std::ostream& aStream, const ExplicitSpecialization& aDeclaration);
	void operator<<(std::ostream& aStream, const LinkageSpecification& aDeclaration);
	void operator<<(std::ostream& aStream, const NamespaceDefinition& aDeclaration);
	void operator<<(std::ostream& aStream, const EmptyDeclaration& aDeclaration);
	void operator<<(std::ostream& aStream, const BalancedToken& aBalancedToken);
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

	void operator<<(std::ostream& aStream, const ConditionalExpression& aExpression);
	void operator<<(std::ostream& aStream, const AssignmentExpression_Assignment& aExpression);
	void operator<<(std::ostream& aStream, const ThrowExpression& aExpression);

	void operator<<(std::ostream& aStream, const Expression& aExpression);
	void operator<<(std::ostream& aStream, const LogicalOrExpression& aExpression);
	void operator<<(std::ostream& aStream, const LogicalAndExpression& aExpression);
	void operator<<(std::ostream& aStream, const InclusiveOrExpression& aExpression);
	void operator<<(std::ostream& aStream, const ExclusiveOrExpression& aExpression);
	void operator<<(std::ostream& aStream, const AndExpression& aExpression);
	void operator<<(std::ostream& aStream, const EqualityExpression& aExpression);
	void operator<<(std::ostream& aStream, const RelationalExpression& aExpression);
	void operator<<(std::ostream& aStream, const ShiftExpression& aExpression);
	void operator<<(std::ostream& aStream, const AddativeExpression& aExpression);
	void operator<<(std::ostream& aStream, const MultiplicativeExpression& aExpression);
	void operator<<(std::ostream& aStream, const PMExpression& aExpression);
	void operator<<(std::ostream& aStream, const CastExpression& aExpression);
	void operator<<(std::ostream& aStream, const UnaryExpression& aExpression);
	void operator<<(std::ostream& aStream, const UnaryExpression_PrefixExpression& aExpression);
	void operator<<(std::ostream& aStream, const UnaryExpression_sizeof& aExpression);
	void operator<<(std::ostream& aStream, const UnaryExpression_alignof& aExpression);
	void operator<<(std::ostream& aStream, const NoexceptExpression& aExpression);
	void operator<<(std::ostream& aStream, const NewExpression& aExpression);
	void operator<<(std::ostream& aStream, const DeleteExpression& aExpression);

	void operator<<(std::ostream& aStream, const PostfixExpression& aExpression);
	void operator<<(std::ostream& aStream, const PostfixExpression_Subscript& aExpression);
	void operator<<(std::ostream& aStream, const PostfixExpression_Call& aExpression);
	void operator<<(std::ostream& aStream, const PostfixExpression_Construct& aExpression);
	void operator<<(std::ostream& aStream, const PostfixExpression_Access& aExpression);
	void operator<<(std::ostream& aStream, const PostfixExpression_Destruct& aExpression);
	void operator<<(std::ostream& aStream, const PostfixExpression_IncDec& aExpression);
	void operator<<(std::ostream& aStream, const PostfixExpression_Cast& aExpression);
	void operator<<(std::ostream& aStream, const PostfixExpression_Typeid& aExpression);

	void operator<<(std::ostream& aStream, const PrimaryExpression& aExpression);

	void operator<<(std::ostream& aStream, const PrimaryExpression_Parenthesis& aExpression);
	void operator<<(std::ostream& aStream, const IdExpression& aExpression);
	void operator<<(std::ostream& aStream, const LambdaExpression& aExpression);

}

#endif // !COMPILER_SCOPE_H
