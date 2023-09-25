
#include "markup/Patterns.h"

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

		void Consume()
		{
			if (Empty())
			{
				CompilerContext::EmitError("Unexpected eof");
				return;
			}
			myBegin++;
			return;
		}

	private:
		iterator myBegin;
		iterator myEnd;
	};

	size_t indent = 0;

	template<class T>
	bool ParseBlockDeclaration(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}
	template<class T>
	bool ParseFunctionDeclaration(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}
	template<class T>
	bool ParseTemplateDeclaration(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}
	template<class T>
	bool ParseExplicitInstantiation(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}
	template<class T>
	bool ParseLinkageSpecification(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}
	template<class T>
	bool ParseNamespaceDefinition(TokenStream& aStream, T& aOut)
	{
		// TODO
		return false;
	}
	template<class T>
	bool ParseEmptyDeclaration(TokenStream& aStream, T& aOut)
	{
		if (aStream.Token().myType != tokenizer::Token::Type::Semicolon)
			return false;

		EmptyDeclaration temp;
		temp.mySemicolon = &aStream.Token();
		aStream.Consume();

		aOut = temp;
		return true;
	}
	template<class T>
	bool ParseAttributeDeclaration(TokenStream& aStream, T& aOut)
	{
		TokenStream stream(aStream);

		AttributeDeclaration decl;

		do
		{
			if (stream.Token().myType != tokenizer::Token::Type::L_Bracket)
				return false;





		} while (stream.Token().myType != tokenizer::Token::Type::Semicolon);

		decl.mySemicolon = &stream.Token();
		stream.Consume();

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
