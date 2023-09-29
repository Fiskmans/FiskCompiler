
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
	template<AssignableBy<TemplateDeclaration> T>
	bool ParseTemplateDeclaration(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}
	template<AssignableBy<ExplicitInstantiation> T>
	bool ParseExplicitInstantiation(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}
	template<AssignableBy<LinkageSpecification> T>
	bool ParseLinkageSpecification(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}
	template<AssignableBy<NamespaceDefinition> T>
	bool ParseNamespaceDefinition(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}
	template<AssignableBy<EmptyDeclaration> T>
	bool ParseEmptyDeclaration(TokenStream& aStream, T& aOut)
	{
		if (aStream.TokenType() != tokenizer::Token::Type::Semicolon)
			return false;

		EmptyDeclaration temp;
		temp.mySemicolon = &aStream.Consume();

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
	}

	bool ParsePostfixExpression_construction(TokenStream& aStream, PostfixExpression& aOut)
	{
	}

	bool ParsePostfixExpression_extendable(TokenStream& aStream, PostfixExpression& aOut)
	{
	}
	
	bool ParseExpression(TokenStream& aStream, Expression& aOut);

	bool ParsePrimaryExpression(TokenStream& aStream, PrimaryExpression& aOut)
	{
		// TODO
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

	template <AssignableBy<PostfixExpression> T>
	bool ParsePostfixExpression(TokenStream& aStream, T& aOut)
	{
		PostfixExpression expr;
		if (!ParsePostfixExpression_nonrecurse(aStream, expr))
			return false;

		while(true)
		{
			switch (aStream.TokenType())
			{
				case tokenizer::Token::Type::L_Bracket:
				{
					TokenStream& stream(aStream);

					PostfixExpression_Subscript subscript;
					subscript.myLeftHandSide = std::make_shared<PostfixExpression>(expr);
					subscript.myOpeningBracket = stream.Consume();

					Expression accessExpression;
					BracedInitList accessList;

					if (ParseExpression(stream, accessExpression))
					{
						subscript.myAccess = accessExpression
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
					expr = subscript;
					aStream = stream;
					break;
				}

				case tokenizer::Token::Type::L_Paren:
				{
					TokenStream& stream(aStream);
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

					expr = call;
					aStream = stream;
					break;
				}

				case tokenizer::Token::Type::Dot:
				case tokenizer::Token::Type::Arrow:
				{
					TokenStream& stream(aStream);

					const tokenizer::Token* op = stream.Consume();

					PostfixExpression_Access access;
					
					if (stream.TokenType() == tokenizer::Token::Type::kw_template)
					{
						access.myLeftHandSide = std::make_shared<PostfixExpression>(expr);
						access.myAccessOperator = op;
						access.myTemplate = stream.Consume();

						if (!ParseIdExpression(stream, access.myIdExpression))
							return false;
					}

				}

				case tokenizer::Token::Type::PlusPlus:
				case tokenizer::Token::Type::MinusMinus:

					PostfixExpression_IncDec IncDec;
					IncDec.myLeftHandSide

					break;
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

			if (std::find(std::begin(candidates),std::end(candidates), stream.TokenType()) != std::end(candidates))
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

			if (std::find(std::begin(candidates),std::end(candidates), stream.TokenType()) != std::end(candidates))
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

			if (std::find(std::begin(candidates),std::end(candidates), stream.TokenType()) != std::end(candidates))
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

			if (std::find(std::begin(candidates),std::end(candidates), stream.TokenType()) != std::end(candidates))
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

			if (std::find(std::begin(candidates),std::end(candidates), stream.TokenType()) != std::end(candidates))
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

			if (std::find(std::begin(candidates),std::end(candidates), stream.TokenType()) != std::end(candidates))
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
			decltype(TranslationUnit::myDeclarations)::value_type temp;

			if (ParseBlockDeclaration(aStream, temp))
			{
				aOutUnit.myDeclarations.push_back(temp);
				continue;
			}
			if (ParseFunctionDeclaration(aStream, temp))
			{
				aOutUnit.myDeclarations.push_back(temp);
				continue;
			}
			if (ParseTemplateDeclaration(aStream, temp))
			{
				aOutUnit.myDeclarations.push_back(temp);
				continue;
			}
			if (ParseExplicitInstantiation(aStream, temp))
			{
				aOutUnit.myDeclarations.push_back(temp);
				continue;
			}
			if (ParseLinkageSpecification(aStream, temp))
			{
				aOutUnit.myDeclarations.push_back(temp);
				continue;
			}
			if (ParseNamespaceDefinition(aStream, temp))
			{
				aOutUnit.myDeclarations.push_back(temp);
				continue;
			}
			if (ParseEmptyDeclaration(aStream, temp))
			{
				aOutUnit.myDeclarations.push_back(temp);
				continue;
			}
			if (ParseAttributeDeclaration(aStream, temp))
			{
				aOutUnit.myDeclarations.push_back(temp);
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
		aStream << "TranslationUnit" << NewLine();
		aStream << "Declarations: " << aTranslationUnit.myDeclarations.size() << NewLine();
		aStream << "{";
		
		indent++;
		
		for (auto& decl : aTranslationUnit.myDeclarations)
		{
			switch (decl.index())
			{
			case 0:
				aStream << std::get<BlockDeclaration>(decl);
				break;
			case 1:
				aStream << std::get<FunctionDeclaration>(decl);
				break;
			case 2:
				aStream << std::get<TemplateDeclaration>(decl);
				break;
			case 3:
				aStream << std::get<ExplicitInstantiation>(decl);
				break;
			case 4:
				aStream << std::get<ExplicitSpecialization>(decl);
				break;
			case 5:
				aStream << std::get<LinkageSpecification>(decl);
				break;
			case 6:
				aStream << std::get<NamespaceDefinition>(decl);
				break;
			case 7:
				aStream << std::get<EmptyDeclaration>(decl);
				break;
			case 8:
				aStream << std::get<AttributeDeclaration>(decl);
				break;
			}
		}
		
		indent--;

		aStream << NewLine() << "}";
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
		aStream  << NewLine() << "ExplicitSpecialization";
	}

	void operator<<(std::ostream& aStream, const LinkageSpecification& aDeclaration)
	{
		aStream  << NewLine() << "LinkageSpecification";
	}

	void operator<<(std::ostream& aStream, const NamespaceDefinition& aDeclaration)
	{
		aStream  << NewLine() << "NamespaceDefinition";
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
		aStream << NewLine() << "TODO";
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

}
