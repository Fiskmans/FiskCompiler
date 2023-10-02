
#include "markup/Patterns.h"
#include "markup/Concepts.h"

namespace markup 
{
	class TokenStream
	{
	public:
		using iterator = std::vector<tokenizer::Token>::const_iterator;
		TokenStream(const std::vector<tokenizer::Token>& aTokens)
		{
			myBegin = aTokens.begin();
			myEnd = aTokens.end();
		}

		bool Empty()
		{
			return myBegin == myEnd;
		}

		const tokenizer::Token& Token()
		{
			if (Empty())
			{
				CompilerContext::EmitError("Unexpected eof");
				return tokenizer::Token::SafetyToken;
			}
			return *myBegin;
		}

		tokenizer::Token::Type
		TokenType()
		{
			if (Empty())
			{
				CompilerContext::EmitError("Unexpected eof");
				return tokenizer::Token::Type::Invalid;
			}
			return myBegin->myType;
		}

		const tokenizer::Token* Consume()
		{
			if (Empty())
			{
				CompilerContext::EmitError("Unexpected eof");
				return &tokenizer::Token::SafetyToken;
			}
			iterator out = myBegin;
			myBegin++;
			return &*out;
		}

	private:
		iterator myBegin;
		iterator myEnd;
	};

	size_t indent = 0;

	
	template<AssignableBy<EmptyDeclaration> T>
	bool ParseEmptyDeclaration(TokenStream& aStream, T& aOut);
	template<AssignableBy<AttributeDeclaration> T>
	bool ParseAttributeDeclaration(TokenStream& aStream, T& aOut);
	bool ParseIdExpression(TokenStream& aStream, IdExpression& aOut);
	bool ParseExpression(TokenStream& aStream, Expression& aOut);
	bool ParseDeclaration(TokenStream& aStream, Declaration& aOut);
	bool ParseRBraceTerminatedDeclarations(TokenStream& aStream, std::vector<Declaration>& aOutDeclarations);
	template<AssignableBy<NamespaceDefinition> T>
	bool ParseNamespaceDefinition(TokenStream& aStream, T& aOut);


	template<AssignableBy<BlockDeclaration> T>
	bool ParseBlockDeclaration(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}


	template<AssignableBy<FunctionDeclaration> T>
	bool ParseFunctionDeclaration(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}

	template<AssignableBy<TypeParameter> T>
	bool ParseTypeParameter(TokenStream& aStream, T& aOut)
	{
		TypeParameter type;
		TokenStream stream(aStream);

		if (stream.TokenType() == tokenizer::Token::Type::kw_template)
		{
			TemplateType templateType;
			if (!ParseTemplateType(stream, templateType))
				return false;

			type.myBaseTemplateType = std::make_shared<TemplateType>(templateType);
		}

		switch (stream.TokenType())
		{
			case tokenizer::Token::Type::kw_typename:
				if (type.myBaseTemplateType)
				{
					CompilerContext::EmitError("Expected class", stream.Token());
					return false;
				}
			case tokenizer::Token::Type::kw_class:
				type.myClassOrTypename = stream.Consume();
				break;
			default:
				if (type.myBaseTemplateType)
				{
					CompilerContext::EmitError("Expected class", stream.Token());
				}
				else
				{
					CompilerContext::EmitError("Expected class or typename", stream.Token());
				}
				return false;
		}

		if (stream.TokenType() == tokenizer::Token::Type::Ellipsis)
			type.myEllipsis = stream.Consume();





		// TODO
		return false;
	}

	template<AssignableBy<ParameterDeclaration> T>
	bool ParseTypeParameter(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}

	bool ParseTemplateParameter(TokenStream& aStream, TemplateParameter& aOut)
	{
		// TODO 
		return false;
	}

	bool ParseTemplateType(TokenStream& aStream, TemplateType& aOut)
	{
		if (aStream.TokenType() != tokenizer::Token::Type::kw_template)
			return false;

		TokenStream stream(aStream);
		aOut.myTemplate = stream.Consume();
		
		if (stream.TokenType() != tokenizer::Token::Type::Less)
			return false;

		do
		{
			TemplateParameter parameter;
			if (!ParseTemplateParameter(stream,parameter))
				return false;

			aOut.myParamaters.push_back(parameter);

			if (stream.TokenType() == tokenizer::Token::Type::Comma)
			{
				stream.Consume();
				continue;
			}

		} while(false);

		if (stream.TokenType() != tokenizer::Token::Type::Greater)
			return false;

		aOut.myClosing = stream.Consume();

		aStream = stream;

		return true;
	}

	template<AssignableBy<TemplateDeclaration> T>
	bool ParseTemplateDeclaration(TokenStream& aStream, T& aOut)
	{
		if (aStream.TokenType() != tokenizer::Token::Type::kw_template)
			return false;

		TokenStream stream(aStream);
		TemplateDeclaration templateDecl;

		if (!ParseTemplateType(stream, templateDecl.myType))
			return false;

		Declaration decl;

		if (!ParseDeclaration(stream, decl))
			return false;

		templateDecl.myDeclaration = std::make_shared<Declaration>(decl);

		aOut = templateDecl;
		aStream = stream;

		return true;
	}

	template<AssignableBy<ExplicitInstantiation> T>
	bool ParseExplicitInstantiation(TokenStream& aStream, T& aOut)
	{
		ExplicitInstantiation explicitInstantiation;
		TokenStream stream(aStream);
		
		if (stream.TokenType() == tokenizer::Token::Type::kw_extern)
			explicitInstantiation.myExtern = stream.Consume();

		if (stream.TokenType() != tokenizer::Token::Type::kw_template)
			return false;

		explicitInstantiation.myTemplate = stream.Consume();

		Declaration decl;

		if (!ParseDeclaration(stream, decl))
			return false;

		explicitInstantiation.myDeclaration = std::make_shared<Declaration>(decl);
		
		aOut = explicitInstantiation;
		aStream = stream;

		return true;
	}

	template<AssignableBy<ExplicitSpecialization> T>
	bool ParseExplicitSpecialization(TokenStream& aStream, T& aOut)
	{
		if (aStream.TokenType() != tokenizer::Token::Type::kw_template)
			return false;

		ExplicitSpecialization explicitSpecialization;
		TokenStream& stream(aStream);

		explicitSpecialization.myTemplate = stream.Consume();

		if (stream.TokenType() != tokenizer::Token::Type::Less)
		{
			CompilerContext::EmitError("Expected <", stream.Token());
			return false;
		}

		explicitSpecialization.myOpening = stream.Consume();

		if (stream.TokenType() != tokenizer::Token::Type::Greater)
			return false;

		explicitSpecialization.myClosing = stream.Consume();

		Declaration decl;
		if (!ParseDeclaration(stream, decl))
		{
			CompilerContext::EmitError("Unkown declaration");
			return false;
		}

		explicitSpecialization.myDeclaration = std::make_shared<Declaration>(decl);

		aOut = explicitSpecialization;
		aStream = stream;

		return true;
	}

	template<AssignableBy<LinkageSpecification> T>
	bool ParseLinkageSpecification(TokenStream& aStream, T& aOut)
	{
		if (aStream.TokenType() != tokenizer::Token::Type::kw_extern)
			return false;

		TokenStream stream(aStream);

		LinkageSpecification linkage;

		linkage.myExtern = stream.Consume();

		if (stream.TokenType() != tokenizer::Token::Type::String_literal)
		{
			CompilerContext::EmitError("Expected string literal", stream.Token());
			return false;
		}

		linkage.myString = stream.Consume();

		if (stream.TokenType() == tokenizer::Token::Type::L_Brace)
		{
			LinkageSpecification_block block;
			block.myOpeningBrace = stream.Consume();

			if (!ParseRBraceTerminatedDeclarations(stream, block.myDeclarations))
				return false;

			if (stream.TokenType() != tokenizer::Token::Type::R_Brace)
			{
				CompilerContext::EmitError("Expected }", stream.Token());
				CompilerContext::EmitError("To close LinkageSpecification", *linkage.myExtern);
				return false;
			}

			block.myClosingBrace = stream.Consume();

			linkage.myDeclaration = block;
		}
		else
		{
			Declaration decl;
			if (!ParseDeclaration(stream, decl))
				return false;

			linkage.myDeclaration = std::make_shared<Declaration>(decl);
		}

		aOut = linkage;
		aStream = stream;

		return true;
	}

	bool ParseDeclaration(TokenStream& aStream, Declaration& aOut)
	{
		if (ParseBlockDeclaration(aStream, aOut))
			return true;

		if (ParseFunctionDeclaration(aStream, aOut))
			return true;

		if (ParseTemplateDeclaration(aStream, aOut))
			return true;

		if (ParseExplicitInstantiation(aStream, aOut))
			return true;

		if (ParseExplicitSpecialization(aStream, aOut))
			return true;

		if (ParseLinkageSpecification(aStream, aOut))
			return true;

		if (ParseNamespaceDefinition(aStream, aOut))
			return true;

		if (ParseEmptyDeclaration(aStream, aOut))
			return true;

		if (ParseAttributeDeclaration(aStream, aOut))
			return true;

		return false;
	}

	bool ParseRBraceTerminatedDeclarations(TokenStream& aStream, std::vector<Declaration>& aOutDeclarations)
	{
		TokenStream& stream(aStream);

		while (stream.TokenType() != tokenizer::Token::Type::R_Brace)
		{
			Declaration decl;

			if (!ParseDeclaration(stream, decl))
			{
				CompilerContext::EmitError("Unkown declaration", stream.Token());
				return false;
			}

			aOutDeclarations.push_back(decl);
		}

		aStream = stream;
		return true;
	}

	template<AssignableBy<NamespaceDefinition> T>
	bool ParseNamespaceDefinition(TokenStream& aStream, T& aOut)
	{

		NamespaceDefinition namespaceDefinition;
		TokenStream stream(aStream);

		if (stream.TokenType() == tokenizer::Token::Type::kw_inline)
			namespaceDefinition.myInline = stream.Consume();

		if (stream.TokenType() != tokenizer::Token::Type::kw_namespace)
			return false;

		namespaceDefinition.myNamespace = stream.Consume();

		if (stream.TokenType() == tokenizer::Token::Type::Identifier)
			namespaceDefinition.myIdentifier = stream.Consume();

		if (stream.TokenType() != tokenizer::Token::Type::L_Brace)
		{
			CompilerContext::EmitError("Expected {", stream.Token());
			return false;
		}

		namespaceDefinition.myOpeningBrace = stream.Consume();

		if (!ParseRBraceTerminatedDeclarations(stream, namespaceDefinition.myDeclarations))
			return false;

		namespaceDefinition.myClosingBrace = stream.Consume();
		aOut = namespaceDefinition;
		aStream = stream;

		return true;
	}


	template<AssignableBy<EmptyDeclaration> T>
	bool ParseEmptyDeclaration(TokenStream& aStream, T& aOut)
	{
		if (aStream.TokenType() != tokenizer::Token::Type::Semicolon)
			return false;

		EmptyDeclaration temp;
		temp.mySemicolon = aStream.Consume();

		aOut = temp;
		return true;
	}

	template<AssignableBy<AttributeArgumentClause> T>
	bool ParseAttributeArgumentClause(TokenStream& aStream, T& aOut)
	{
		TokenStream stream(aStream);

		AttributeArgumentClause clause;

		if (stream.TokenType() != tokenizer::Token::Type::L_Paren)
			return false;

		clause.myOpeningParenthesis = stream.Consume();

		if (stream.TokenType() == tokenizer::Token::Type::R_Paren)
			return false;

		std::stack<tokenizer::Token::Type> closers;
		closers.push(tokenizer::Token::Type::R_Paren);

		while (!closers.empty())
		{
			switch (stream.TokenType())
			{
				case tokenizer::Token::Type::L_Paren:
					closers.push(tokenizer::Token::Type::R_Paren);
					break;
				case tokenizer::Token::Type::L_Bracket:
					closers.push(tokenizer::Token::Type::R_Bracket);
					break;
				case tokenizer::Token::Type::L_Brace:
					closers.push(tokenizer::Token::Type::R_Brace);
					break;
				case tokenizer::Token::Type::R_Paren:
				case tokenizer::Token::Type::R_Bracket:
				case tokenizer::Token::Type::R_Brace:
					if (closers.top() != stream.TokenType())
						return false;
					closers.pop();
					break;
			}

			if (!closers.empty())
				clause.myBalancedTokenSequence.push_back(stream.Consume());
		}

		clause.myClosingParenthesis = stream.Consume();

		aOut = clause;
		aStream = stream;

		return true;

	}

	bool ParseAttribute(TokenStream& aStream, Attribute& aOut)
	{
		TokenStream stream(aStream);

		if (stream.TokenType() != tokenizer::Token::Type::Identifier)
			return false;

		const tokenizer::Token* firstIdentifier = stream.Consume();

		if (stream.TokenType() == tokenizer::Token::Type::Colon_colon)
		{
			aOut.myAttributeNamespace = firstIdentifier;
			aOut.myColonColon = stream.Consume();
			if (stream.TokenType() != tokenizer::Token::Type::Identifier)
				return false;

			aOut.myIdentifier = stream.Consume();
		}
		else
		{
			aOut.myIdentifier = firstIdentifier;
		}

		ParseAttributeArgumentClause(stream, aOut.myArgumentClause);

		aStream = stream;

		return true;
	}


	bool ParseAttributeList(TokenStream& aStream, AttributeList& aOut)
	{
		TokenStream stream(aStream);

		while (true)
		{
			if (stream.TokenType() == tokenizer::Token::Type::Ellipsis)
			{
				if (aOut.myAttributes.empty())
					return false;

				aOut.myEllipsis = &stream.Token();
				stream.Consume();
				break;
			}

			if (stream.TokenType() != tokenizer::Token::Type::Identifier)
				break;

			Attribute attr;

			if (!ParseAttribute(stream, attr))
				return false;

			aOut.myAttributes.push_back(attr);

			if (stream.TokenType() != tokenizer::Token::Type::Comma)
				break;

			stream.Consume();
		}

		aStream = stream;

		return true;
	}

	bool ParseDeclTypeSpecifier(TokenStream& aStream, DecltypeSpecifier& aOut)
	{
		// TODO
		return false;
	}

	template<AssignableBy<NestedNameSpecifier> T>
	bool ParseNestedNameSpecifier(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}

	bool ParseSimpleTamplateId(TokenStream& aStream, SimpleTemplateId& aOut)
	{
		// TODO
		return false;
	}

	bool ParseTypename(TokenStream& aStream, Typename& aOut)
	{
		TokenStream stream(aStream);
		SimpleTemplateId templateId;

		if (stream.TokenType() != tokenizer::Token::Type::Identifier)
			return false;

		if (ParseSimpleTamplateId(stream, templateId))
		{
			aOut = templateId;
			aStream = stream;
			return true;
		}

		aOut = stream.Consume();
		aStream = stream;

		return true;
	}

	template<AssignableBy<SimpleTypeSpecifier> T>
	bool ParseSimpleTypeSpecifier(TokenStream& aStream, T& aOut)
	{
		TokenStream stream(aStream);
		SimpleTypeSpecifier simpleTypeSpecifier;
		switch (stream.TokenType())
		{
			case tokenizer::Token::Type::kw_char:
			case tokenizer::Token::Type::kw_char16_t:
			case tokenizer::Token::Type::kw_char32_t:
			case tokenizer::Token::Type::kw_wchar_t:
			case tokenizer::Token::Type::kw_bool:
			case tokenizer::Token::Type::kw_short:
			case tokenizer::Token::Type::kw_int:
			case tokenizer::Token::Type::kw_long:
			case tokenizer::Token::Type::kw_signed:
			case tokenizer::Token::Type::kw_unsigned:
			case tokenizer::Token::Type::kw_float:
			case tokenizer::Token::Type::kw_double:
			case tokenizer::Token::Type::kw_void:
			case tokenizer::Token::Type::kw_auto:
			{
				SimpleTypeSpecifier_Builtin builtin;
				builtin.myType = stream.Consume();

				simpleTypeSpecifier = builtin;
				aOut = simpleTypeSpecifier;
				aStream = stream;
				return true;
			} 
			case tokenizer::Token::Type::kw_decltype:
			{
				DecltypeSpecifier declType;
				if (!ParseDeclTypeSpecifier(stream, declType))
					return false;
				
				simpleTypeSpecifier = declType;
				aOut = simpleTypeSpecifier;
				aStream = stream;
				return true;
			}
		}

		const tokenizer::Token* colonColon = nullptr;

		if (stream.TokenType() == tokenizer::Token::Type::Colon_colon)
			colonColon = stream.Consume();

		std::optional<NestedNameSpecifier> nestedNameSpecifer;

		ParseNestedNameSpecifier(stream, nestedNameSpecifer);

		if (stream.TokenType() == tokenizer::Token::Type::kw_template)
		{
			if (!nestedNameSpecifer)
				return false;

			SimpleTypeSpecifier_TemplateTypename templatedTypename;

			templatedTypename.myColonColon = colonColon;
			templatedTypename.myNameSpecifier = *nestedNameSpecifer;

			if (!ParseSimpleTamplateId(stream, templatedTypename.mySimpleTemplateId))
				return false;

			simpleTypeSpecifier = templatedTypename;
			aOut = simpleTypeSpecifier;
			aStream = stream;

			return true;
		}

		SimpleTypeSpecifier_Typename typeName;
		typeName.myColonColon = colonColon;
		typeName.myNameSpecifier = nestedNameSpecifer;

		if (!ParseTypename(stream, typeName.myTypename))
			return false;

		simpleTypeSpecifier = typeName;
		aOut = simpleTypeSpecifier;
		aStream = stream;

		return true;
	}

	bool ParseTypeSpecifier(TokenStream& aStream, TypeSpecifier& aOut)
	{
		TokenStream stream(aStream);
		if (ParseSimpleTypeSpecifier(stream, aOut))
		{
			aStream = stream;
			return true;
		}

		// TODO
		return false;
	}

	template<AssignableBy<AbstractDeclarator> T>
	bool ParseAbstractDeclarator(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}

	template<AssignableBy<TypeId> T>
	bool ParseTypeId(TokenStream& aStream, T& aOut)
	{
		TokenStream stream(aStream);
		TypeId typeId;

		typeId.myTypeSpecifierSequence = std::make_shared<TypeSpecifierSequence>();

		while (true)
		{
			TypeSpecifier typeSpecifier;
			if (!ParseTypeSpecifier(stream, typeSpecifier))
				break;

			typeId.myTypeSpecifierSequence->myTypeSpecifiers.push_back(typeSpecifier);
		}

		if (typeId.myTypeSpecifierSequence->myTypeSpecifiers.size() == 0)
			return false;

		AbstractDeclarator declarator;
		if (ParseAbstractDeclarator(stream, declarator))
			typeId.myAbstractDeclarator = std::make_shared<AbstractDeclarator>(declarator);

		aStream = stream;
		aOut = typeId;

		return true;
	}
	
	bool ParsePostfixExpression_cast(TokenStream& aStream, PostfixExpression& aOut)
	{
		// TODO
		return false;
	}

	bool ParsePostfixExpression_construction(TokenStream& aStream, PostfixExpression& aOut)
	{
		// TODO
		return false;
	}

	bool ParsePostfixExpression_extendable(TokenStream& aStream, PostfixExpression& aOut)
	{
		// TODO
		return false;
	}
	
	bool ParseLambdaExpression(TokenStream& aStream, LambdaExpression& aOut)
	{
		// TODO
		return false;
	}

	template<AssignableBy<PrimaryExpression_Parenthesis> T>
	bool ParsePrimaryExpression_parenthesis(TokenStream& aStream, T& aOut)
	{
		TokenStream stream(aStream);
		PrimaryExpression_Parenthesis parenthesis;

		if (stream.TokenType() != tokenizer::Token::Type::L_Paren)
			return false;

		parenthesis.myOpeningParenthesis = stream.Consume();

		Expression content;
		if (!ParseExpression(stream, content))
			return false;

		parenthesis.myExpression = std::make_shared<Expression>(content);

		if (stream.TokenType() != tokenizer::Token::Type::R_Paren)
			return false;

		parenthesis.myClosingParenthesis = stream.Consume();
		aStream = stream;
		aOut = parenthesis;

		return true;
	}

	bool ParsePrimaryExpression(TokenStream& aStream, PrimaryExpression& aOut)
	{
		switch (aStream.TokenType())
		{
			case tokenizer::Token::Type::kw_this:
			case tokenizer::Token::Type::Integer_literal:
			case tokenizer::Token::Type::Char_literal:
			case tokenizer::Token::Type::String_literal:
			case tokenizer::Token::Type::kw_true:
			case tokenizer::Token::Type::kw_false:
			case tokenizer::Token::Type::kw_nullptr:
			case tokenizer::Token::Type::Floating_literal:
				aOut = aStream.Consume();
				return true;
			case tokenizer::Token::Type::L_Paren:
				if (!ParsePrimaryExpression_parenthesis(aStream, aOut))
					return false;
				return true;
		}

		TokenStream stream(aStream);

		IdExpression idExpression;
		if (ParseIdExpression(stream, idExpression))
		{
			aOut = idExpression;
			aStream = stream;
			return true;
		}

		LambdaExpression lambdaExpression;
		if (ParseLambdaExpression(stream, lambdaExpression))
		{
			aOut = lambdaExpression;
			aStream = stream;
			return true;
		}

		return false;
	}

	bool ParsePostfixExpression_nonrecurse(markup::TokenStream& aStream, markup::PostfixExpression& aOut)
	{
		switch (aStream.TokenType())
		{
			case tokenizer::Token::Type::kw_const_cast:
			case tokenizer::Token::Type::kw_dynamic_cast:
			case tokenizer::Token::Type::kw_reinterpret_cast:
			case tokenizer::Token::Type::kw_static_cast:
				if (!ParsePostfixExpression_cast(aStream, aOut))
					return false;

				return true;

			case tokenizer::Token::Type::kw_typeid: {
				TokenStream& stream(aStream);

				PostfixExpression_Typeid id;
				id.myTypeid = stream.Consume();

				if (stream.TokenType() != tokenizer::Token::Type::L_Paren)
					return false;

				id.myOpeningParenthesis = stream.Consume();

				if (!ParseTypeId(stream, id.myContent))
				{
					Expression subExpression;
					if (!ParseExpression(stream, subExpression))
						return false;

					id.myContent = std::make_shared<Expression>(subExpression);
				}

				if (stream.TokenType() == tokenizer::Token::Type::R_Paren)
					return false;

				id.myClosingParenthesis = stream.Consume();

				aOut.myContent = id;

				return true;
			}
		}

		if (ParsePostfixExpression_construction(aStream, aOut))
			return true;

		PrimaryExpression primary;

		if (!ParsePrimaryExpression(aStream, primary))
			return false;

		aOut.myContent	= primary;
		return true;
	}

	bool ParseBracedInitList(TokenStream& aStream, BracedInitList& aOut)
	{
		// TODO
		return false;
	}

	bool ParseExpressionList(TokenStream& aStream, ExpressionList& aOut)
	{
		// TODO
		return false;
	}

	bool ParseIdExpression(TokenStream& aStream, IdExpression& aOut)
	{
		// TODO
		return false;
	}

	bool ParsePseudoDestructorName(TokenStream& aStream, PseudoDestructorName& aOut)
	{
		// TODO
		return false;
	}

	template <AssignableBy<PostfixExpression> T>
	bool ParsePostfixExpression(TokenStream& aStream, T& aOut)
	{
		PostfixExpression expr;
		if (!ParsePostfixExpression_nonrecurse(aStream, expr))
			return false;

		TokenStream stream(aStream);

		while(true)
		{
			switch (stream.TokenType())
			{
				case tokenizer::Token::Type::L_Bracket:
				{
					PostfixExpression_Subscript subscript;
					subscript.myLeftHandSide = std::make_shared<PostfixExpression>(expr);
					subscript.myOpeningBracket = stream.Consume();

					Expression accessExpression;
					BracedInitList accessList;

					if (ParseExpression(stream, accessExpression))
					{
						subscript.myAccess = std::make_shared<Expression>(accessExpression);
					}
					else if (ParseBracedInitList(stream, accessList))
					{
						subscript.myAccess = accessList;
					}
					else
					{
						subscript.myAccess = std::optional<BracedInitList>{};
					}

					if (stream.TokenType() != tokenizer::Token::Type::R_Bracket)
						return false;

					subscript.myClosingBracket = stream.Consume();
					expr.myContent = subscript;
					break;
				}

				case tokenizer::Token::Type::L_Paren:
				{
					PostfixExpression_Call call;

					call.myLeftHandSide = std::make_shared<PostfixExpression>();
					call.myOpeningParenthesis = stream.Consume();

					ExpressionList expressionList;

					if (ParseExpressionList(stream, expressionList))
					{
						call.myExpression = expressionList;
					}

					if (stream.TokenType() != tokenizer::Token::Type::R_Paren)
						return false;

					call.myClosingParenthesis = stream.Consume();

					expr.myContent = call;
					break;
				}

				case tokenizer::Token::Type::Dot:
				case tokenizer::Token::Type::Arrow:
				{
					const tokenizer::Token* op = stream.Consume();

					PostfixExpression_Access access;
					
					if (stream.TokenType() == tokenizer::Token::Type::kw_template)
					{
						access.myLeftHandSide = std::make_shared<PostfixExpression>(expr);
						access.myAccessOperator = op;
						access.myTemplate = stream.Consume();

						if (!ParseIdExpression(stream, access.myIdExpression))
							return false;

						expr.myContent = access;
						break;
					}

					if (ParseIdExpression(stream, access.myIdExpression))
					{
						access.myLeftHandSide = std::make_shared<PostfixExpression>(expr);
						access.myAccessOperator = op;

						expr.myContent = access;
						break;
					}


					PostfixExpression_Destruct destruct;
					if (ParsePseudoDestructorName(stream, destruct.myPseudoDestructorName))
					{
						destruct.myLeftHandSide = std::make_shared<PostfixExpression>(expr);
						destruct.myAccessOperator = op;

						expr.myContent = destruct;
						break;
					}

					return false;
				}

				case tokenizer::Token::Type::PlusPlus:
				case tokenizer::Token::Type::MinusMinus:
				{
					PostfixExpression_IncDec IncDec;
					IncDec.myLeftHandSide = std::make_shared<PostfixExpression>(expr);
					IncDec.myOperator = stream.Consume();

					expr.myContent = IncDec;
					break;
				}
				default:
					aOut = expr;
					return true;
			}
		}
	}

	template<AssignableBy<NoexceptExpression> T>
	bool ParseNoexceptExpression(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}

	template<AssignableBy<NewExpression> T>
	bool ParseNewExpression(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}

	template<AssignableBy<DeleteExpression> T>
	bool ParseDeleteExpression(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}

	bool ParseCastExpression(TokenStream& aStream, CastExpression& aOut);

	template<AssignableBy<UnaryExpression> T>
	bool ParseUnaryExpression(TokenStream& aStream, T& aOut)
	{
		UnaryExpression unaryExpression;

		switch (aStream.TokenType())
		{
			case tokenizer::Token::Type::PlusPlus:
			case tokenizer::Token::Type::MinusMinus:
			case tokenizer::Token::Type::Star:
			case tokenizer::Token::Type::BitAnd:
			case tokenizer::Token::Type::Plus:
			case tokenizer::Token::Type::Minus:
			case tokenizer::Token::Type::Not:
			case tokenizer::Token::Type::Complement:
			{
				TokenStream stream(aStream);
				UnaryExpression_PrefixExpression prefix;
				prefix.myOperator = stream.Consume();
				prefix.myRightHandSide = std::make_shared<CastExpression>();

				if (!ParseCastExpression(stream, *prefix.myRightHandSide))
					return false;

				unaryExpression = prefix;
				aOut = unaryExpression;
				aStream = stream;
				return true;
			}
			case tokenizer::Token::Type::kw_sizeof:
			{
				// TODO
				return false;
			}
			case tokenizer::Token::Type::kw_alignof:
			{
				// TODO
				return false;
			}
		}

		if (ParseNoexceptExpression(aStream, unaryExpression))
		{
			aOut = unaryExpression;
			return true;
		}

		if (ParseNewExpression(aStream, unaryExpression))
		{
			aOut = unaryExpression;
			return true;
		}

		if (ParseDeleteExpression(aStream, unaryExpression))
		{
			aOut = unaryExpression;
			return true;
		}

		if (ParsePostfixExpression(aStream, unaryExpression))
		{
			aOut = unaryExpression;
			return true;
		}

		return false;
	}

	bool ParseCastExpression(TokenStream& aStream, CastExpression& aOut)
	{
		do
		{
			TokenStream stream(aStream);
			if (stream.TokenType() != tokenizer::Token::Type::L_Paren)
				break;

			CastExpression_recurse recurse;
			recurse.myOpeningParenthesis = stream.Consume();

			if (!ParseTypeId(stream, recurse.myTypeId))
				break;

			if (stream.TokenType() != tokenizer::Token::Type::R_Paren)
				break;

			CastExpression child;
			if (!ParseCastExpression(stream, child))
				break;

			recurse.myRightHandSide = std::make_shared<CastExpression>(child);
			aStream = stream;
			aOut.myContent = recurse;
			return true;

		} while (false);

		if (!ParseUnaryExpression(aStream, aOut.myContent))
			return false;

		return true;
	}
	
	bool ParsePMExpression(TokenStream& aStream, PMExpression& aOut)
	{
		TokenStream stream(aStream);
		PMExpression expr;

		while (true)
		{
			if (!ParseCastExpression(stream, expr.myRightHandSide))
				return false;

			const tokenizer::Token::Type candidates[] = 
			{
				tokenizer::Token::Type::DotStar,
				tokenizer::Token::Type::ArrowStar
			};

			if (std::find(std::begin(candidates),std::end(candidates), stream.TokenType()) == std::end(candidates))
				break;

			PMExpression next;
			next.myDereferenceOperator = stream.Consume();
			next.myLeftHandSide = std::make_shared<PMExpression>(expr);

			expr = next;
		}

		aOut = expr;
		aStream = stream;
		return true;
	}

	bool ParseMultiplicativeExpression(TokenStream& aStream, MultiplicativeExpression& aOut)
	{
		TokenStream stream(aStream);
		MultiplicativeExpression expr;

		while (true)
		{
			if (!ParsePMExpression(stream, expr.myRightHandSide))
				return false;

			const tokenizer::Token::Type candidates[] = 
			{
				tokenizer::Token::Type::Star,
				tokenizer::Token::Type::Div,
				tokenizer::Token::Type::Mod
			};

			if (std::find(std::begin(candidates),std::end(candidates), stream.TokenType()) == std::end(candidates))
				break;

			MultiplicativeExpression next;
			next.myMultiplicationOperator = stream.Consume();
			next.myLeftHandSide = std::make_shared<MultiplicativeExpression>(expr);

			expr = next;
		}

		aOut = expr;
		aStream = stream;
		return true;
	}

	bool ParseAddativeExpression(TokenStream& aStream, AddativeExpression& aOut)
	{
		TokenStream stream(aStream);
		AddativeExpression expr;

		while (true)
		{
			if (!ParseMultiplicativeExpression(stream, expr.myRightHandSide))
				return false;

			const tokenizer::Token::Type candidates[] = 
			{
				tokenizer::Token::Type::Plus,
				tokenizer::Token::Type::Minus
			};

			if (std::find(std::begin(candidates),std::end(candidates), stream.TokenType()) == std::end(candidates))
				break;

			AddativeExpression next;
			next.myAdditionOperator = stream.Consume();
			next.myLeftHandSide = std::make_shared<AddativeExpression>(expr);

			expr = next;
		}

		aOut = expr;
		aStream = stream;
		return true;
	}

	bool ParseShiftExpression(TokenStream& aStream, ShiftExpression& aOut)
	{
		TokenStream stream(aStream);
		ShiftExpression expr;

		while (true)
		{
			if (!ParseAddativeExpression(stream, expr.myRightHandSide))
				return false;

			const tokenizer::Token::Type candidates[] = 
			{
				tokenizer::Token::Type::LessLess,
				tokenizer::Token::Type::GreaterGreater
			};

			if (std::find(std::begin(candidates),std::end(candidates), stream.TokenType()) == std::end(candidates))
				break;

			ShiftExpression next;
			next.myShiftOperator = stream.Consume();
			next.myLeftHandSide = std::make_shared<ShiftExpression>(expr);

			expr = next;
		}

		aOut = expr;
		aStream = stream;
		return true;
	}

	bool ParseRelationalExpression(TokenStream& aStream, RelationalExpression& aOut)
	{
		TokenStream stream(aStream);
		RelationalExpression expr;

		while (true)
		{
			if (!ParseShiftExpression(stream, expr.myRightHandSide))
				return false;

			const tokenizer::Token::Type candidates[] = 
			{
				tokenizer::Token::Type::Less,
				tokenizer::Token::Type::Greater,
				tokenizer::Token::Type::LessEqual,
				tokenizer::Token::Type::GreaterEqual
			};

			if (std::find(std::begin(candidates),std::end(candidates), stream.TokenType()) == std::end(candidates))
				break;

			RelationalExpression next;
			next.myRelationOperator = stream.Consume();
			next.myLeftHandSide = std::make_shared<RelationalExpression>(expr);

			expr = next;
		}

		aOut = expr;
		aStream = stream;
		return true;
	}

	bool ParseEqualityExpression(TokenStream& aStream, EqualityExpression& aOut)
	{
		TokenStream stream(aStream);
		EqualityExpression expr;

		while (true)
		{
			if (!ParseRelationalExpression(stream, expr.myRightHandSide))
				return false;

			const tokenizer::Token::Type candidates[] = 
			{
				tokenizer::Token::Type::EqualEqual,
				tokenizer::Token::Type::NotEquals
			};

			if (std::find(std::begin(candidates),std::end(candidates), stream.TokenType()) == std::end(candidates))
				break;

			EqualityExpression next;
			next.myEqualityOperator = stream.Consume();
			next.myLeftHandSide = std::make_shared<EqualityExpression>(expr);

			expr = next;
		}

		aOut = expr;
		aStream = stream;
		return true;
	}

	bool ParseAndExpression(TokenStream& aStream, AndExpression& aOut)
	{
		TokenStream stream(aStream);
		AndExpression expr;

		while (true)
		{
			if (!ParseEqualityExpression(stream, expr.myRightHandSide))
				return false;

			if (stream.TokenType() != tokenizer::Token::Type::BitAnd)
				break;

			AndExpression next;
			next.myAnd = stream.Consume();
			next.myLeftHandSide = std::make_shared<AndExpression>(expr);

			expr = next;
		}

		aOut = expr;
		aStream = stream;
		return true;
	}

	bool ParseExclusiveOrExpression(TokenStream& aStream, ExclusiveOrExpression& aOut)
	{
		TokenStream stream(aStream);
		ExclusiveOrExpression expr;

		while (true)
		{
			if (!ParseAndExpression(stream, expr.myRightHandSide))
				return false;

			if (stream.TokenType() != tokenizer::Token::Type::Xor)
				break;

			ExclusiveOrExpression next;
			next.myXor = stream.Consume();
			next.myLeftHandSide = std::make_shared<ExclusiveOrExpression>(expr);

			expr = next;
		}

		aOut = expr;
		aStream = stream;
		return true;
	}

	bool ParseInclusiveOrExpression(TokenStream& aStream, InclusiveOrExpression& aOut)
	{
		TokenStream stream(aStream);
		InclusiveOrExpression expr;

		while (true)
		{
			if (!ParseExclusiveOrExpression(stream, expr.myRightHandSide))
				return false;

			if (stream.TokenType() != tokenizer::Token::Type::BitOr)
				break;

			InclusiveOrExpression next;
			next.myOr = stream.Consume();
			next.myLeftHandSide = std::make_shared<InclusiveOrExpression>(expr);

			expr = next;
		}

		aOut = expr;
		aStream = stream;
		return true;
	}

	bool ParseLogicalAndExpression(TokenStream& aStream, LogicalAndExpression& aOut)
	{
		TokenStream stream(aStream);
		LogicalAndExpression expr;

		while (true)
		{
			if (!ParseInclusiveOrExpression(stream, expr.myRightHandSide))
				return false;

			if (stream.TokenType() != tokenizer::Token::Type::And)
				break;

			LogicalAndExpression next;
			next.myAndAnd = stream.Consume();
			next.myLeftHandSide = std::make_shared<LogicalAndExpression>(expr);

			expr = next;
		}

		aOut = expr;
		aStream = stream;
		return true;
	}

	bool ParseLogicalOrExpression(TokenStream& aStream, LogicalOrExpression& aOut)
	{
		TokenStream stream(aStream);
		LogicalOrExpression	expr;

		while (true)
		{
			if (!ParseLogicalAndExpression(stream, expr.myRightHandSide))
				return false;

			if (stream.TokenType() != tokenizer::Token::Type::Or)
				break;

			LogicalOrExpression next;
			next.myOrOr = stream.Consume();
			next.myLeftHandSide = std::make_shared<LogicalOrExpression>(expr);

			expr = next;
		}

		aOut	= expr;
		aStream = stream;
		return true;
	}
	
	template<AssignableBy<AssignmentExpression> T>
	bool ParseAssignmentExpression(TokenStream& aStream, T& aOut);

	bool ParseExpression(TokenStream& aStream, Expression& aOut)
	{
		TokenStream stream(aStream);
		Expression expr;
		
		while(true)
		{
			if (!ParseAssignmentExpression(stream, expr.myRightHandSide))
				return false;

			if (stream.TokenType() != tokenizer::Token::Type::Comma)
				break;

			Expression next;
			next.myComma = stream.Consume();
			next.myLeftHandSide = std::make_shared<Expression>(expr);
			
			expr = next;
		}

		aOut = expr;
		aStream = stream;
		return true;
	}

	template<AssignableBy<ConditionalExpression> T>
	bool ParseConditionalExpression(TokenStream& aStream, T& aOut)
	{
		TokenStream stream(aStream);

		ConditionalExpression expr;

		if (!ParseLogicalOrExpression(stream, expr.myCondition))
			return false;

		if (stream.TokenType() != tokenizer::Token::Type::Question)
		{
			aOut = expr;
			aStream = stream;
			return true;
		}

		expr.myOnTruthy = std::make_shared<Expression>();
		expr.myQuestionMark = stream.Consume();
		expr.myOnFalsy = std::make_shared<AssignmentExpression>();

		if (!ParseExpression(stream, *expr.myOnTruthy))
			return false;

		if (stream.TokenType() != tokenizer::Token::Type::Colon)
			return false;

		expr.myColon = stream.Consume();
		
		if (!ParseAssignmentExpression(stream, *expr.myOnFalsy))
			return false;

		aOut = expr;
		aStream = stream;

		return true;
	}
	
	template<AssignableBy<AssignmentExpression_Assignment> T>
	bool ParseAssignmentExpression_Assignment(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}
	
	template<AssignableBy<ThrowExpression> T>
	bool ParseThrowExpression(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}

	template<AssignableBy<AssignmentExpression> T>
	bool ParseAssignmentExpression(TokenStream& aStream, T& aOut)
	{
		AssignmentExpression assign;

		if (ParseConditionalExpression(aStream, assign))
		{
			aOut = assign;
			return true;
		}

		if (ParseAssignmentExpression_Assignment(aStream, assign))
		{
			aOut = assign;
			return true;
		}

		if (ParseThrowExpression(aStream, assign))
		{
			aOut = assign;
			return true;
		}

		return false;
	}

	bool ParseAttributeSpecifier(TokenStream& aStream, std::variant<AttributeSpecifier, AlignmentSpecifier>& aOut)
	{
		TokenStream stream(aStream);

		if (stream.TokenType() == tokenizer::Token::Type::L_Bracket)
		{
			AttributeSpecifier attr;

			attr.myOuterOpening = stream.Consume();

			if (stream.TokenType() != tokenizer::Token::Type::L_Bracket)
				return false;

			attr.myInnerOpening = stream.Consume();

			if (!ParseAttributeList(stream, attr.myAttributeList))
				return false;

			if (stream.TokenType() != tokenizer::Token::Type::R_Bracket)
				return false;

			attr.myInnerClosing = stream.Consume();

			if (stream.TokenType() != tokenizer::Token::Type::R_Bracket)
				return false;

			attr.myOuterClosing = stream.Consume();

			aOut = attr;
			aStream = stream;

			return true;
		}

		if (stream.TokenType() == tokenizer::Token::Type::kw_alignas)
		{
			AlignmentSpecifier align;

			align.myAlignas = stream.Consume();

			if (stream.TokenType() != tokenizer::Token::Type::L_Paren)
				return false;

			align.myOpeningParen = stream.Consume();

			if (!ParseTypeId(stream, align.myContent))
				if (!ParseAssignmentExpression(stream, align.myContent))
					return false;

			if (stream.TokenType() == tokenizer::Token::Type::Ellipsis)
			{
				align.myEllipsis = &stream.Token();
				stream.Consume();
			}

			if (stream.TokenType() != tokenizer::Token::Type::R_Paren)
				return false;

			align.myOpeningParen = stream.Consume();
			
			aOut = align;
			aStream = stream;

			return true;
		}

		return false;
	}

	template<AssignableBy<AttributeDeclaration> T>
	bool ParseAttributeDeclaration(TokenStream& aStream, T& aOut)
	{
		TokenStream stream(aStream);

		AttributeDeclaration decl;

		do
		{
			std::variant<AttributeSpecifier, AlignmentSpecifier> attributeSpecifier;

			if (!ParseAttributeSpecifier(stream, attributeSpecifier))
				return false;

			decl.mySpecifiers.push_back(attributeSpecifier);

		} while (stream.TokenType() != tokenizer::Token::Type::Semicolon);

		decl.mySemicolon = &stream.Token();
		stream.Consume();

		aOut = decl;
		aStream = stream;

		return true;
	}

	bool ParseTranslation(TokenStream& aStream, TranslationUnit& aOutUnit)
	{
		while (!aStream.Empty())
		{
			Declaration decl;

			if (ParseBlockDeclaration(aStream, decl))
			{
				aOutUnit.myDeclarations.push_back(decl);
				continue;
			}
			if (ParseFunctionDeclaration(aStream, decl))
			{
				aOutUnit.myDeclarations.push_back(decl);
				continue;
			}
			if (ParseTemplateDeclaration(aStream, decl))
			{
				aOutUnit.myDeclarations.push_back(decl);
				continue;
			}
			if (ParseExplicitInstantiation(aStream, decl))
			{
				aOutUnit.myDeclarations.push_back(decl);
				continue;
			}
			if (ParseLinkageSpecification(aStream, decl))
			{
				aOutUnit.myDeclarations.push_back(decl);
				continue;
			}
			if (ParseNamespaceDefinition(aStream, decl))
			{
				aOutUnit.myDeclarations.push_back(decl);
				continue;
			}
			if (ParseEmptyDeclaration(aStream, decl))
			{
				aOutUnit.myDeclarations.push_back(decl);
				continue;
			}
			if (ParseAttributeDeclaration(aStream, decl))
			{
				aOutUnit.myDeclarations.push_back(decl);
				continue;
			}

			CompilerContext::EmitError("Unkown declaration", aStream.Token());
			break;
		}
		return true;
	}

	std::string NewLine()
	{
		return "\n" + std::string(indent, ' ');
	}

	TranslationUnit Markup(const std::vector<tokenizer::Token>& aTokens)
	{
		TokenStream stream(aTokens);
		TranslationUnit unit;

		if (!ParseTranslation(stream, unit))
		{
			CompilerContext::EmitError("<internal error> Failed to parse translation unit");
			return unit;
		}

		if (!stream.Empty())
		{
			CompilerContext::EmitError("<internal error> Unparsed tokens left");
			return unit;
		}

		return unit;
	}
	
	void operator<<(std::ostream& aStream, std::initializer_list<const tokenizer::Token*> aTokens)
	{
		for (const tokenizer::Token* tok : aTokens)
		{
			if (tok)
				aStream << tok->myRawText << " ";
		}
	}

	void operator<<(std::ostream& aStream, const TranslationUnit& aTranslationUnit)
	{
		indent = 0;
		aStream << "TranslationUnit";
		aStream << NewLine() << "\\Declarations: " << aTranslationUnit.myDeclarations.size();
		aStream << NewLine() << "{";
		indent++;
		
		for (const Declaration& decl : aTranslationUnit.myDeclarations)
		{
			aStream << decl;
		}
		
		indent--;
		aStream << NewLine() << "}";
	}

	void operator<<(std::ostream& aStream, const Declaration& aDeclaration)
	{
		switch (aDeclaration.index())
		{
			case 0:
				aStream << std::get<BlockDeclaration>(aDeclaration);
				break;
			case 1:
				aStream << std::get<FunctionDeclaration>(aDeclaration);
				break;
			case 2:
				aStream << std::get<TemplateDeclaration>(aDeclaration);
				break;
			case 3:
				aStream << std::get<ExplicitInstantiation>(aDeclaration);
				break;
			case 4:
				aStream << std::get<ExplicitSpecialization>(aDeclaration);
				break;
			case 5:
				aStream << std::get<LinkageSpecification>(aDeclaration);
				break;
			case 6:
				aStream << std::get<NamespaceDefinition>(aDeclaration);
				break;
			case 7:
				aStream << std::get<EmptyDeclaration>(aDeclaration);
				break;
			case 8:
				aStream << std::get<AttributeDeclaration>(aDeclaration);
				break;
		}
	}


	void operator<<(std::ostream& aStream, const BlockDeclaration& aDeclaration)
	{
		aStream  << NewLine() << "BlockDeclaration";
	}

	void operator<<(std::ostream& aStream, const FunctionDeclaration& aDeclaration)
	{
		aStream  << NewLine() << "FunctionDeclaration";
	}

	void operator<<(std::ostream& aStream, const TemplateDeclaration& aDeclaration)
	{
		aStream  << NewLine() << "TemplateDeclaration";
	}

	void operator<<(std::ostream& aStream, const ExplicitInstantiation& aDeclaration)
	{
		aStream  << NewLine() << "ExplicitInstantiation";
	}

	void operator<<(std::ostream& aStream, const ExplicitSpecialization& aDeclaration)
	{
		aStream << NewLine() << "ExplicitSpecialization";
		aStream << NewLine() << "{";
		indent++;

		aStream << *aDeclaration.myDeclaration;

		indent--;
		aStream << NewLine() << "}";
	}

	void operator<<(std::ostream& aStream, const LinkageSpecification& aDeclaration)
	{
		aStream << NewLine() << "LinkageSpecification";
		aStream << NewLine() << "\\Type: " << aDeclaration.myString->myRawText;
		
		switch (aDeclaration.myDeclaration.index())
		{
			case 0:
			{
				const LinkageSpecification_block& block = std::get<LinkageSpecification_block>(aDeclaration.myDeclaration);
				aStream << NewLine() << "\\Declarations: " << block.myDeclarations.size();

				aStream << NewLine() << "{";
				indent++;

				for (const Declaration& decl : block.myDeclarations)
				{
					aStream << decl;
				}

				indent--;
				aStream << NewLine() << "}";
				break;
			}
			case 1:
			{
				const Declaration& declaration = *std::get<NotNullPtr<Declaration>>(aDeclaration.myDeclaration);
				aStream << NewLine() << "\\Declarations: 1";

				aStream << NewLine() << "{";
				indent++;
				
				aStream << declaration;

				indent--;
				aStream << NewLine() << "}";

				break;
			}

			default:
				break;
		}

	}

	void operator<<(std::ostream& aStream, const NamespaceDefinition& aDeclaration)
	{
		aStream << NewLine() << "NamespaceDefinition";
		aStream << NewLine() << "\\Declarations: " << aDeclaration.myDeclarations.size();
		if (aDeclaration.myIdentifier)
		{
			aStream << NewLine() << "\\Name: " << aDeclaration.myIdentifier->myRawText;
		}
		else
		{
			aStream << NewLine() << "\\Anonymous";
		}

		if (!aDeclaration.myDeclarations.empty())
		{
			aStream << NewLine() << "{";
			indent++;

			for (const Declaration& decl : aDeclaration.myDeclarations)
			{
				aStream << decl;
			}

			indent--;
			aStream << NewLine() << "}";
		}
	}

	void operator<<(std::ostream& aStream, const EmptyDeclaration& aDeclaration)
	{
		aStream  << NewLine() << "EmptyDeclaration";
	}
	
	void operator<<(std::ostream& aStream, const Attribute& aAttribute)
	{
		aStream << NewLine() << Tokens("", aAttribute.myAttributeNamespace, aAttribute.myColonColon, aAttribute.myIdentifier);
		if (aAttribute.myArgumentClause)
		{
			aStream << "("; 
			bool first = true;
			for (const tokenizer::Token* tok : aAttribute.myArgumentClause->myBalancedTokenSequence)
			{
				if(!first) aStream << " ";
				first = false;
				aStream << tok->myRawText;
			}
			aStream << ")";
		}
	}

	void operator<<(std::ostream& aStream, const AttributeDeclaration& aDeclaration)
	{
		aStream << NewLine() << "AttributeDeclaration";
		aStream << NewLine() << "{";
		indent++;

		aStream << aDeclaration.mySpecifiers;

		indent--;
		aStream << NewLine() << "}";
	}
	
	void operator<<(std::ostream& aStream, const TypeId& aTypeId)
	{
		aStream << NewLine() << "Type";
		aStream << NewLine() << "{";
		indent++;

		aStream << *aTypeId.myTypeSpecifierSequence;
		
		if (aTypeId.myAbstractDeclarator)
			aStream << *aTypeId.myAbstractDeclarator;

		indent--;
		aStream << NewLine() << "}";
	}
	
	void operator<<(std::ostream& aStream, const AssignmentExpression& aExpression)
	{
		switch (aExpression.index())
		{
			case 0:
				aStream << std::get<ConditionalExpression>(aExpression);
				break;
			case 1:
				aStream << std::get<AssignmentExpression_Assignment>(aExpression);
				break;
			case 2:
				aStream << std::get<ThrowExpression>(aExpression);
				break;
		}
	}

	void operator<<(std::ostream& aStream, const TypeSpecifierSequence& aTypeSpecifierSequence)
	{
		for (TypeSpecifier typeSpecifier : aTypeSpecifierSequence.myTypeSpecifiers)
		{
			switch (typeSpecifier.index())
			{
				case 0:
					aStream << std::get<TrailingTypeSpecifier>(typeSpecifier);
					break;
				case 1:
					aStream << std::get<ClassSpecifier>(typeSpecifier);
					break;
				case 2:
					aStream << std::get<EnumSpecifier>(typeSpecifier);
					break;
			}
		}

		if (aTypeSpecifierSequence.myAttributeSpecifierSequence)
			aStream << *aTypeSpecifierSequence.myAttributeSpecifierSequence;
	}

	void operator<<(std::ostream& aStream, const AbstractDeclarator& aActractDeclarator)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const AttributeSpecifierSequence& aAttributeSpecifireSequence)
	{
		for (std::variant<AttributeSpecifier, AlignmentSpecifier> specifier : aAttributeSpecifireSequence)
		{
			switch (specifier.index())
			{
				case 0: {
					AttributeSpecifier& attr = std::get<AttributeSpecifier>(specifier);
					aStream << NewLine() << "AttributeSpecifier";

					if (!attr.myAttributeList.myAttributes.empty())
					{
						aStream << NewLine() << "{";
						indent++;

						aStream << NewLine() << "Attributes";
						aStream << NewLine() << "{";
						indent++;

						for (Attribute attribute : attr.myAttributeList.myAttributes)
						{
							aStream << attribute;
						}

						indent--;
						aStream << NewLine() << "}";

						aStream << NewLine() << "Ellipsis: ";
						if (attr.myAttributeList.myEllipsis)
							aStream << "True";
						else
							aStream << "False";

						indent--;
						aStream << NewLine() << "}";
					}
					break;
				}
				case 1:
					AlignmentSpecifier& align = std::get<AlignmentSpecifier>(specifier);
					aStream << NewLine() << "Alignas";
					aStream << NewLine() << "{";
					indent++;

					switch (align.myContent.index())
					{
						case 0:
							aStream << std::get<TypeId>(align.myContent);
							break;
						case 1:
							aStream << std::get<AssignmentExpression>(align.myContent);
							break;
					}

					indent--;
					aStream << NewLine() << "}";
					break;
			}
		}
	}
	
	void operator<<(std::ostream& aStream, const TrailingTypeSpecifier& aTrailingTypeSpecifier)
	{
		switch (aTrailingTypeSpecifier.index())
		{
			case 0:
				aStream << std::get<SimpleTypeSpecifier>(aTrailingTypeSpecifier);
				break;
			case 1:
				aStream << std::get<ElaborateTypeSpecifier>(aTrailingTypeSpecifier);
				break;
			case 2:
				aStream << std::get<TypenameSpecifier>(aTrailingTypeSpecifier);
				break;
			case 3:
				aStream << std::get<CVQualifier>(aTrailingTypeSpecifier);
				break;
		}
	}

	void operator<<(std::ostream& aStream, const ClassSpecifier& aClassSpecifier)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const EnumSpecifier& aEnumSpecifier)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const SimpleTypeSpecifier& aSimpleTypeSpecifier)
	{
		switch (aSimpleTypeSpecifier.index())
		{
			case 0:
				aStream << std::get<SimpleTypeSpecifier_Typename>(aSimpleTypeSpecifier);
				break;
			case 1:
				aStream << std::get<SimpleTypeSpecifier_TemplateTypename>(aSimpleTypeSpecifier);
				break;
			case 2:
				aStream << std::get<SimpleTypeSpecifier_Builtin>(aSimpleTypeSpecifier);
				break;
			case 3:
				aStream << std::get<DecltypeSpecifier>(aSimpleTypeSpecifier);
				break;
		}
	}

	void operator<<(std::ostream& aStream, const ElaborateTypeSpecifier& aElaborateTypeSpecifier)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const TypenameSpecifier& aTypenameSpecifier)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const CVQualifier& aCVQualifier)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const SimpleTypeSpecifier_Typename& aTypeName)
	{
		aStream << NewLine() << "Typename";
		aStream << NewLine() << "{";
		indent++;

		aStream << NewLine() << "Explicitly global: " << (aTypeName.myColonColon ? "True": "False");
		
		if (aTypeName.myNameSpecifier)
			aStream << *aTypeName.myNameSpecifier;
		
		aStream << aTypeName.myTypename;


		indent--;
		aStream << NewLine() << "}";
	}

	void operator<<(std::ostream& aStream, const SimpleTypeSpecifier_TemplateTypename& aTemplatedTypename)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const SimpleTypeSpecifier_Builtin& aBuiltin)
	{
		aStream << NewLine() << aBuiltin.myType->myRawText;
	}

	void operator<<(std::ostream& aStream, const DecltypeSpecifier& aDecltypeSpecifier)
	{
		aStream << NewLine() << "TODO";
	}
	
	void operator<<(std::ostream& aStream, const NestedNameSpecifier& aNestedNameSpecifier)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const Typename& aTypename)
	{
		switch (aTypename.index())
		{
			case 0:
				aStream << NewLine() << std::get<Identifier>(aTypename)->myRawText;
				break;
			case 1:
				aStream << std::get<SimpleTemplateId>(aTypename);
				break;
		}
	}
	
	void operator<<(std::ostream& aStream, const ConditionalExpression& aExpression)
	{
		if (!aExpression.myQuestionMark)
		{
			aStream << aExpression.myCondition;
			return;
		}

		aStream << NewLine() << "Ternary";
		aStream << NewLine() << "{";
		indent++;

			aStream << NewLine() << "Condition";
			aStream << NewLine() << "{";
			indent++;

				aStream << aExpression.myCondition;

			indent--;
			aStream << NewLine() << "}";
			aStream << NewLine() << "Truthy";
			aStream << NewLine() << "{";
			indent++;

				aStream << *aExpression.myOnTruthy;

			indent--;
			aStream << NewLine() << "}";
			aStream << NewLine() << "Falsy";
			aStream << NewLine() << "{";
			indent++;

				aStream << *aExpression.myOnFalsy;

			indent--;
			aStream << NewLine() << "}";

		indent--;
		aStream << NewLine() << "}";
	}

	void operator<<(std::ostream& aStream, const AssignmentExpression_Assignment& aExpression)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const ThrowExpression& aExpression)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const Expression& aExpression)
	{
		if (!aExpression.myLeftHandSide)
		{
			aStream << aExpression.myRightHandSide;
			return;
		}

		std::stack<std::shared_ptr<Expression>> parts;
		parts.push(std::make_shared<Expression>(aExpression));

		while (parts.top()->myLeftHandSide)
		{
			parts.push(parts.top()->myLeftHandSide);
		}

		aStream << NewLine() << "Comma";
		aStream << NewLine() << "{";
		indent++;

		while (!parts.empty())
		{
			aStream << parts.top()->myRightHandSide;
			parts.pop();
		}

		indent--;
		aStream << NewLine() << "}";
	}
	
	void operator<<(std::ostream& aStream, const LogicalOrExpression& aExpression)
	{
		if (!aExpression.myLeftHandSide)
		{
			aStream << aExpression.myRightHandSide;
			return;
		}

		std::stack<std::shared_ptr<LogicalOrExpression>> parts;
		parts.push(std::make_shared<LogicalOrExpression>(aExpression));

		while (parts.top()->myLeftHandSide)
		{
			parts.push(parts.top()->myLeftHandSide);
		}

		aStream << NewLine() << "LogicalOr";
		aStream << NewLine() << "{";
		indent++;

		while (!parts.empty())
		{
			aStream << parts.top()->myRightHandSide;
			parts.pop();
		}

		indent--;
		aStream << NewLine() << "}";
	}

	void operator<<(std::ostream& aStream, const LogicalAndExpression& aExpression)
	{
		if (!aExpression.myLeftHandSide)
		{
			aStream << aExpression.myRightHandSide;
			return;
		}

		std::stack<std::shared_ptr<LogicalAndExpression>> parts;
		parts.push(std::make_shared<LogicalAndExpression>(aExpression));

		while (parts.top()->myLeftHandSide)
		{
			parts.push(parts.top()->myLeftHandSide);
		}

		aStream << NewLine() << "LogicalAnd";
		aStream << NewLine() << "{";
		indent++;

		while (!parts.empty())
		{
			aStream << parts.top()->myRightHandSide;
			parts.pop();
		}

		indent--;
		aStream << NewLine() << "}";
	}

	void operator<<(std::ostream& aStream, const InclusiveOrExpression& aExpression)
	{
		if (!aExpression.myLeftHandSide)
		{
			aStream << aExpression.myRightHandSide;
			return;
		}

		std::stack<std::shared_ptr<InclusiveOrExpression>> parts;
		parts.push(std::make_shared<InclusiveOrExpression>(aExpression));

		while (parts.top()->myLeftHandSide)
		{
			parts.push(parts.top()->myLeftHandSide);
		}

		aStream << NewLine() << "bitwise or";
		aStream << NewLine() << "{";
		indent++;

		while (!parts.empty())
		{
			aStream << parts.top()->myRightHandSide;
			parts.pop();
		}

		indent--;
		aStream << NewLine() << "}";
	}

	void operator<<(std::ostream& aStream, const ExclusiveOrExpression& aExpression)
	{
		if (!aExpression.myLeftHandSide)
		{
			aStream << aExpression.myRightHandSide;
			return;
		}

		std::stack<std::shared_ptr<ExclusiveOrExpression>> parts;
		parts.push(std::make_shared<ExclusiveOrExpression>(aExpression));

		while (parts.top()->myLeftHandSide)
		{
			parts.push(parts.top()->myLeftHandSide);
		}

		aStream << NewLine() << "bitwise xor";
		aStream << NewLine() << "{";
		indent++;

		while (!parts.empty())
		{
			aStream << parts.top()->myRightHandSide;
			parts.pop();
		}

		indent--;
		aStream << NewLine() << "}";
	}

	void operator<<(std::ostream& aStream, const AndExpression& aExpression)
	{
		if (!aExpression.myLeftHandSide)
		{
			aStream << aExpression.myRightHandSide;
			return;
		}

		std::stack<std::shared_ptr<AndExpression>> parts;
		parts.push(std::make_shared<AndExpression>(aExpression));

		while (parts.top()->myLeftHandSide)
		{
			parts.push(parts.top()->myLeftHandSide);
		}

		aStream << NewLine() << "bitwise And";
		aStream << NewLine() << "{";
		indent++;

		while (!parts.empty())
		{
			aStream << parts.top()->myRightHandSide;
			parts.pop();
		}

		indent--;
		aStream << NewLine() << "}";
	}

	void operator<<(std::ostream& aStream, const EqualityExpression& aExpression)
	{
		if (!aExpression.myLeftHandSide)
		{
			aStream << aExpression.myRightHandSide;
			return;
		}

		std::stack<std::shared_ptr<EqualityExpression>> parts;
		parts.push(std::make_shared<EqualityExpression>(aExpression));

		while (parts.top()->myLeftHandSide)
		{
			parts.push(parts.top()->myLeftHandSide);
		}

		aStream << NewLine() << "Equality";
		aStream << NewLine() << "{";
		indent++;

		while (!parts.empty())
		{
			switch (parts.top()->myEqualityOperator->myType)
			{
				case tokenizer::Token::Type::EqualEqual:
					aStream << NewLine() << "Equals";
					break;

				case tokenizer::Token::Type::NotEquals:
					aStream << NewLine() << "NotEquals";
					break;
			}

			aStream << NewLine() << "{";
			indent++;

			aStream << parts.top()->myRightHandSide;
			parts.pop();

			indent--;
			aStream << NewLine() << "}";
		}

		indent--;
		aStream << NewLine() << "}";
	}

	void operator<<(std::ostream& aStream, const RelationalExpression& aExpression)
	{
		if (!aExpression.myLeftHandSide)
		{
			aStream << aExpression.myRightHandSide;
			return;
		}

		std::stack<std::shared_ptr<RelationalExpression>> parts;
		parts.push(std::make_shared<RelationalExpression>(aExpression));

		while (parts.top()->myLeftHandSide)
		{
			parts.push(parts.top()->myLeftHandSide);
		}

		aStream << NewLine() << "Relation";
		aStream << NewLine() << "{";
		indent++;

		while (!parts.empty())
		{

			switch (parts.top()->myRelationOperator->myType)
			{
				case tokenizer::Token::Type::Less:
					aStream << NewLine() << "Less";
					break;

				case tokenizer::Token::Type::Greater:
					aStream << NewLine() << "Greater";
					break;

				case tokenizer::Token::Type::LessEqual:
					aStream << NewLine() << "LessOrEqual";
					break;

				case tokenizer::Token::Type::GreaterEqual:
					aStream << NewLine() << "GreaterOrEqual";
					break;
			}

			aStream << NewLine() << "{";
			indent++;

			aStream << parts.top()->myRightHandSide;
			parts.pop();

			indent--;
			aStream << NewLine() << "}";
		}

		indent--;
		aStream << NewLine() << "}";
	}

	void operator<<(std::ostream& aStream, const ShiftExpression& aExpression)
	{
		if (!aExpression.myLeftHandSide)
		{
			aStream << aExpression.myRightHandSide;
			return;
		}

		std::stack<std::shared_ptr<ShiftExpression>> parts;
		parts.push(std::make_shared<ShiftExpression>(aExpression));

		while (parts.top()->myLeftHandSide)
		{
			parts.push(parts.top()->myLeftHandSide);
		}

		aStream << NewLine() << "Shift";
		aStream << NewLine() << "{";
		indent++;

		while (!parts.empty())
		{
			if (parts.top()->myShiftOperator)
			{
				switch (parts.top()->myShiftOperator->myType)
				{
					case tokenizer::Token::Type::LessLess:
						aStream << NewLine() << "Left";
						break;

					case tokenizer::Token::Type::GreaterGreater:
						aStream << NewLine() << "Right";
						break;
				}
				aStream << NewLine() << "{";
				indent++;

				aStream << parts.top()->myRightHandSide;
				parts.pop();

				indent--;
				aStream << NewLine() << "}";
			}
			else
			{
				aStream << parts.top()->myRightHandSide;
				parts.pop();
			}
		}

		indent--;
		aStream << NewLine() << "}";
	}

	void operator<<(std::ostream& aStream, const AddativeExpression& aExpression)
	{
		if (!aExpression.myLeftHandSide)
		{
			aStream << aExpression.myRightHandSide;
			return;
		}

		std::stack<std::shared_ptr<AddativeExpression>> parts;
		parts.push(std::make_shared<AddativeExpression>(aExpression));

		while (parts.top()->myLeftHandSide)
		{
			parts.push(parts.top()->myLeftHandSide);
		}

		aStream << NewLine() << "Add";
		aStream << NewLine() << "{";
		indent++;

		while (!parts.empty())
		{
			if (parts.top()->myAdditionOperator)
			{
				switch (parts.top()->myAdditionOperator->myType)
				{
					case tokenizer::Token::Type::Plus:
						aStream << NewLine() << "Add";
						break;

					case tokenizer::Token::Type::Minus:
						aStream << NewLine() << "Subtract";
						break;
				}
				aStream << NewLine() << "{";
				indent++;

				aStream << parts.top()->myRightHandSide;
				parts.pop();

				indent--;
				aStream << NewLine() << "}";
			}
			else
			{
				aStream << parts.top()->myRightHandSide;
				parts.pop();
			}
		}

		indent--;
		aStream << NewLine() << "}";
	}

	void operator<<(std::ostream& aStream, const MultiplicativeExpression& aExpression)
	{
		if (!aExpression.myLeftHandSide)
		{
			aStream << aExpression.myRightHandSide;
			return;
		}

		std::stack<std::shared_ptr<MultiplicativeExpression>> parts;
		parts.push(std::make_shared<MultiplicativeExpression>(aExpression));

		while (parts.top()->myLeftHandSide)
		{
			parts.push(parts.top()->myLeftHandSide);
		}

		aStream << NewLine() << "Multiplicative";
		aStream << NewLine() << "{";
		indent++;

		while (!parts.empty())
		{
			if (parts.top()->myMultiplicationOperator)
			{
				switch (parts.top()->myMultiplicationOperator->myType)
				{
					case tokenizer::Token::Type::Star:
						aStream << NewLine() << "Times";
						break;

					case tokenizer::Token::Type::Div:
						aStream << NewLine() << "Divide";
						break;

					case tokenizer::Token::Type::Mod:
						aStream << NewLine() << "Modulus";
						break;
				}
				aStream << NewLine() << "{";
				indent++;

				aStream << parts.top()->myRightHandSide;
				parts.pop();

				indent--;
				aStream << NewLine() << "}";
			}
			else
			{
				aStream << parts.top()->myRightHandSide;
				parts.pop();
			}
		}

		indent--;
		aStream << NewLine() << "}";
	}

	void operator<<(std::ostream& aStream, const PMExpression& aExpression)
	{
		if (!aExpression.myLeftHandSide)
		{
			aStream << aExpression.myRightHandSide;
			return;
		}

		std::stack<std::shared_ptr<PMExpression>> parts;
		parts.push(std::make_shared<PMExpression>(aExpression));

		while (parts.top()->myLeftHandSide)
		{
			parts.push(parts.top()->myLeftHandSide);
		}

		aStream << NewLine() << "DereferenceAccess";
		aStream << NewLine() << "{";
		indent++;

		while (!parts.empty())
		{
			if (parts.top()->myDereferenceOperator)
			{
				switch (parts.top()->myDereferenceOperator->myType)
				{
					case tokenizer::Token::Type::DotStar:
						aStream << NewLine() << "DotDereference";
						break;

					case tokenizer::Token::Type::ArrowStar:
						aStream << NewLine() << "ArrowDereference";
						break;
				}
				aStream << NewLine() << "{";
				indent++;

				aStream << parts.top()->myRightHandSide;
				parts.pop();

				indent--;
				aStream << NewLine() << "}";
			}
			else
			{
				aStream << parts.top()->myRightHandSide;
				parts.pop();
			}
		}

		indent--;
		aStream << NewLine() << "}";
	}

	void operator<<(std::ostream& aStream, const CastExpression& aExpression)
	{
		if (aExpression.myContent.index() == 0)
		{
			aStream << std::get<UnaryExpression>(aExpression.myContent);
			return;
		}

		const CastExpression_recurse& recurse = std::get<CastExpression_recurse>(aExpression.myContent);

		aStream << NewLine() << "C-style Cast";
		aStream << NewLine() << "{";
		indent++;

		aStream << recurse.myTypeId;
		aStream << *recurse.myRightHandSide;

		indent--;
		aStream << NewLine() << "}";
	}
	
	void operator<<(std::ostream& aStream, const UnaryExpression& aExpression)
	{
		switch (aExpression.index())
		{
			case 0:
				aStream << std::get<PostfixExpression>(aExpression);
				break;
			case 1:
				aStream << std::get<UnaryExpression_PrefixExpression>(aExpression);
				break;
			case 2:
				aStream << std::get<UnaryExpression_sizeof>(aExpression);
				break;
			case 3:
				aStream << std::get<UnaryExpression_alignof>(aExpression);
				break;
			case 4:
				aStream << std::get<NoexceptExpression>(aExpression);
				break;
			case 5:
				aStream << std::get<NewExpression>(aExpression);
				break;
			case 6:
				aStream << std::get<DeleteExpression>(aExpression);
				break;
			default:
				aStream << NewLine() << "ERROR";
				break;
		}
	}

	void operator<<(std::ostream& aStream, const UnaryExpression_PrefixExpression& aExpression)
	{

		// ++ -- * & + - ! ~
		switch (aExpression.myOperator->myType)
		{
			case tokenizer::Token::Type::PlusPlus:
				aStream << NewLine() << "Preincrement";
				break;
			case tokenizer::Token::Type::MinusEqual:
				aStream << NewLine() << "Predecrement";
				break;
			case tokenizer::Token::Type::Star:
				aStream << NewLine() << "Dereference";
				break;
			case tokenizer::Token::Type::BitAnd:
				aStream << NewLine() << "AddressOf";
				break;
			case tokenizer::Token::Type::Plus:
				aStream << NewLine() << "Unary plus";
				break;
			case tokenizer::Token::Type::Minus:
				aStream << NewLine() << "Unary minus";
				break;
			case tokenizer::Token::Type::Not:
				aStream << NewLine() << "Not";
				break;
			case tokenizer::Token::Type::Complement:
				aStream << NewLine() << "Complement";
				break;
		}

		aStream << NewLine() << "{";
		indent++;

		aStream << *aExpression.myRightHandSide;

		indent--;
		aStream << NewLine() << "}";
	}
	
	void operator<<(std::ostream& aStream, const UnaryExpression_sizeof& aExpression)
	{
		aStream << NewLine() << "TODO";
	}
	
	void operator<<(std::ostream& aStream, const UnaryExpression_alignof& aExpression)
	{
		aStream << NewLine() << "TODO";
	}
	
	void operator<<(std::ostream& aStream, const NoexceptExpression& aExpression)
	{
		aStream << NewLine() << "TODO";
	}
	
	void operator<<(std::ostream& aStream, const NewExpression& aExpression)
	{
		aStream << NewLine() << "TODO";
	}
	
	void operator<<(std::ostream& aStream, const DeleteExpression& aExpression)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const PostfixExpression& aExpression)
	{
		switch (aExpression.myContent.index())
		{
			case 0:
				aStream << std::get<PrimaryExpression>(aExpression.myContent);
				break;
			case 1:
				aStream << std::get<PostfixExpression_Subscript>(aExpression.myContent);
				break;
			case 2:
				aStream << std::get<PostfixExpression_Call>(aExpression.myContent);
				break;
			case 3:
				aStream << std::get<PostfixExpression_Construct>(aExpression.myContent);
				break;
			case 4:
				aStream << std::get<PostfixExpression_Access>(aExpression.myContent);
				break;
			case 5:
				aStream << std::get<PostfixExpression_Destruct>(aExpression.myContent);
				break;
			case 6:
				aStream << std::get<PostfixExpression_IncDec>(aExpression.myContent);
				break;
			case 7:
				aStream << std::get<PostfixExpression_Cast>(aExpression.myContent);
				break;
			case 8:
				aStream << std::get<PostfixExpression_Typeid>(aExpression.myContent);
				break;
			default:
				aStream << NewLine() << "ERROR";
				break;
		}
	}
	void operator<<(std::ostream& aStream, const PostfixExpression_Subscript& aExpression)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const PostfixExpression_Call& aExpression)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const PostfixExpression_Construct& aExpression)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const PostfixExpression_Access& aExpression)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const PostfixExpression_Destruct& aExpression)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const PostfixExpression_IncDec& aExpression)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const PostfixExpression_Cast& aExpression)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const PostfixExpression_Typeid& aExpression)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const PrimaryExpression& aExpression)
	{
		switch (aExpression.index())
		{
			case 0:
				aStream << NewLine() << std::get<const tokenizer::Token*>(aExpression)->myRawText;
				break;
			case 1:
				aStream << std::get<PrimaryExpression_Parenthesis>(aExpression);
				break;
			case 2:
				aStream << std::get<IdExpression>(aExpression);
				break;
			case 3:
				aStream << std::get<LambdaExpression>(aExpression);
				break;
			default:
				break;
		}
	}

	void operator<<(std::ostream& aStream, const PrimaryExpression_Parenthesis& aExpression)
	{
		aStream << NewLine() << "Parenthesis";
		aStream << NewLine() << "{";
		indent++;

		aStream << *aExpression.myExpression;

		indent--;
		aStream << NewLine() << "}";
	}

	void operator<<(std::ostream& aStream, const IdExpression& aExpression)
	{
		aStream << NewLine() << "TODO";
	}

	void operator<<(std::ostream& aStream, const LambdaExpression& aExpression)
	{
		aStream << NewLine() << "TODO";
	}	
}
