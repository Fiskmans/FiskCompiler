#ifndef COMMON_HELP_PRINTER_H
#define COMMON_HELP_PRINTER_H

#include <unordered_map>
#include <string_view>
#include <string>

class HelpPrinter
{
public:
	HelpPrinter();
	
	void Emit();

private:

	void Setup();

	class ColumnAlignment
	{
	public:
		void
		Update(size_t aWidth)
		{
			if (aWidth > myAlignment)
				myAlignment = aWidth;
		}

		size_t
		Get() const
		{
			return myAlignment;
		}

	private:
		size_t myAlignment = 0;
	};

	class Section
	{
	public:
		Section(const std::string_view& aLine);

		std::string myHeader;
		std::string myTag;

		std::string myDigest;
		std::string myExtended;
	};

	std::string myHeader;

	ColumnAlignment							myHeaderAlignment;
	ColumnAlignment							myTagAlignment;
	ColumnAlignment							myDigestAlignment;

	std::unordered_map<std::string, Section> mySections;
};

#endif // !COMMON_HELP_PRINTER_H
