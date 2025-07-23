#pragma once

#include <memory>
#include <iostream>

class IIncludeFinder
{
public:
	virtual ~IIncludeFinder() = default;

	virtual std::unique_ptr<std::istream> FindInclude(std::string aFrom, std::string aWants) = 0;
};

class FileInclude : public IIncludeFinder
{
public:
	virtual ~FileInclude() = default;

	std::unique_ptr<std::istream> FindInclude(std::string aFrom, std::string aWants) override;

};