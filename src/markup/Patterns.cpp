
#include "markup/Patterns.h"

namespace markup 
{
	size_t indent = 0;

	std::string NewLine()
	{
		return "\n" + std::string(indent, ' ');
	}

	TranslationUnit Markup(const std::vector<tokenizer::Token>& aTokens)
	{
		MarkupGlobals::StateStack.push(aTokens);

		TranslationUnit unit;

		return unit;
	}

	void operator<<(std::ostream& aStream, const TranslationUnit& aTranslationUnit)
	{
		indent = 0;
		aStream << "TranslationUnit" << NewLine();
		aStream << "Declarations: " << aTranslationUnit.myDeclarations.size() << NewLine();
		aStream << "{";
		
		indent++;
		
		for (Declaration decl : aTranslationUnit.myDeclarations)
		{
			aStream << decl;
		}
		
		indent--;

		aStream << NewLine() << "}";
	}

	void operator<<(std::ostream& aStream, const Declaration& aDeclaration)
	{
		if (aDeclaration.myDeclaration)
		{
			switch (aDeclaration.myDeclaration->index())
			{
				case 0:
					aStream << std::get<BlockDeclaration>(*aDeclaration.myDeclaration);
					return;
				case 1:
					aStream << std::get<FunctionDeclaration>(*aDeclaration.myDeclaration);
					return;
				case 2:
					aStream << std::get<TemplateDeclaration>(*aDeclaration.myDeclaration);
					return;
				case 3:
					aStream << std::get<ExplicitInstantiation>(*aDeclaration.myDeclaration);
					return;
				case 4:
					aStream << std::get<ExplicitSpecialization>(*aDeclaration.myDeclaration);
					return;
				case 5:
					aStream << std::get<LinkageSpecification>(*aDeclaration.myDeclaration);
					return;
				case 6:
					aStream << std::get<NamespaceDefinition>(*aDeclaration.myDeclaration);
					return;
				case 7:
					aStream << std::get<EmptyDeclaration>(*aDeclaration.myDeclaration);
					return;
				case 8:
					aStream << std::get<AttributeDeclaration>(*aDeclaration.myDeclaration);
					return;
			}
		}
		else
		{
			aStream << "Unkown declaration" << NewLine();
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
