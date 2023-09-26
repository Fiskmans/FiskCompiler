
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

		const const const const tokenizer::Token& Token()
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

		const tokenizer::Token& Consume()
		{
			if (Empty())
			{
				CompilerContext::EmitError("Unexpected eof");
				return tokenizer::Token::SafetyToken;
			}
			iterator out = myBegin;
			myBegin++;
			return *out;
		}

	private:
		iterator myBegin;
		iterator myEnd;
	};

	size_t indent = 0;

	template<AssingableBy<BlockDeclaration> T>
	bool ParseBlockDeclaration(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}
	template<AssingableBy<FunctionDeclaration> T>
	bool ParseFunctionDeclaration(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}
	template<AssingableBy<TemplateDeclaration> T>
	bool ParseTemplateDeclaration(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}
	template<AssingableBy<ExplicitInstantiation> T>
	bool ParseExplicitInstantiation(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}
	template<AssingableBy<LinkageSpecification> T>
	bool ParseLinkageSpecification(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}
	template<AssingableBy<NamespaceDefinition> T>
	bool ParseNamespaceDefinition(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}
	template<AssingableBy<EmptyDeclaration> T>
	bool ParseEmptyDeclaration(TokenStream& aStream, T& aOut)
	{
		if (aStream.TokenType() != tokenizer::Token::Type::Semicolon)
			return false;

		EmptyDeclaration temp;
		temp.mySemicolon = &aStream.Consume();

		aOut = temp;
		return true;
	}

	template<AssingableBy<AttributeArgumentClause> T>
	bool ParseAttributeArgumentClause(TokenStream& aStream, T& aOut)
	{
		TokenStream stream(aStream);

		AttributeArgumentClause clause;

		if (stream.TokenType() != tokenizer::Token::Type::L_Paren)
			return false;

		clause.myOpeningParenthesis = &stream.Consume();

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
				clause.myBalancedTokenSequence.push_back(&stream.Consume());
		}

		clause.myClosingParenthesis = &stream.Consume();

		aOut = clause;
		aStream = stream;

		return true;

	}

	bool ParseAttribute(TokenStream& aStream, Attribute& aOut)
	{
		TokenStream stream(aStream);

		if (stream.TokenType() != tokenizer::Token::Type::Identifier)
			return false;

		const tokenizer::Token* firstIdentifier = &stream.Consume();

		if (stream.TokenType() == tokenizer::Token::Type::Colon_colon)
		{
			aOut.myAttributeNamespace = firstIdentifier;
			aOut.myColonColon = &stream.Consume();
			if (stream.TokenType() != tokenizer::Token::Type::Identifier)
				return false;

			aOut.myIdentifier = &stream.Consume();
		}
		else
		{
			aOut.myIdentifier = firstIdentifier;
		}

		ParseAttributeArgumentClause(stream, aOut.myArgumentClause);

		aStream = stream;

		return false;
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

			Attribute attr;

			if (!ParseAttribute(stream, attr))
				break;

			aOut.myAttributes.push_back(attr);
		}

		aStream = stream;

		return true;
	}

	template<AssingableBy<TypeId> T>
	bool ParseTypeId(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}

	template<AssingableBy<AssignmentExpression> T>
	bool ParseAssignmentExpression(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}

	bool ParseAttributeSpecifier(TokenStream& aStream, std::variant<AttributeSpecifier, AlignmentSpecifier>& aOut)
	{
		TokenStream stream(aStream);

		if (stream.TokenType() == tokenizer::Token::Type::L_Bracket)
		{
			AttributeSpecifier attr;

			attr.myOuterOpening = &stream.Consume();

			if (stream.TokenType() != tokenizer::Token::Type::L_Bracket)
				return false;

			attr.myInnerOpening = &stream.Consume();

			if (!ParseAttributeList(stream, attr.myAttributeList))
				return false;

			if (stream.TokenType() != tokenizer::Token::Type::R_Bracket)
				return false;

			attr.myInnerClosing = &stream.Consume();

			if (stream.TokenType() != tokenizer::Token::Type::R_Bracket)
				return false;

			attr.myOuterClosing = &stream.Consume();

			aOut = attr;
			aStream = stream;

			return true;
		}

		if (stream.TokenType() == tokenizer::Token::Type::kw_alignas)
		{
			AlignmentSpecifier align;

			align.myAlignas = &stream.Consume();

			if (stream.TokenType() != tokenizer::Token::Type::L_Paren)
				return false;

			align.myOpeningParen = &stream.Consume();

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

			align.myOpeningParen = &stream.Consume();
			
			aOut = align;
			aStream = stream;

			return true;
		}

		return false;
	}

	template<AssingableBy<AttributeDeclaration> T>
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

	void operator<<(std::ostream& aStream, const AttributeDeclaration& aDeclaration)
	{
		aStream  << NewLine() << "AttributeDeclaration";
	}

}
