#include "common/FeatureSwitch.h"

#include <string>
#include <fstream>
#include <stack>

FeatureSwitch::FeatureSwitch(const std::string& aConfigFile)
{
	std::ifstream configFile(aConfigFile);
	std::string line;
	while (std::getline(configFile, line))
	{
		if(line == "")
			continue;

		if(line[0] == '#')
			continue;

		size_t colonPos = line.find(':');
		if(colonPos == std::string::npos)
			continue;

		std::string name = line.substr(0, colonPos);
		std::vector<std::string> mappings;

		size_t at = colonPos + 1;
		while (true)
		{
			size_t next = line.find(',', at);
			if(next == std::string::npos)
			{
				mappings.push_back(line.substr(at));
				break;
			}
			else
			{
				mappings.push_back(line.substr(at, next - at));
				at = next + 1;
			}
		}
	}

	Enable("default");
}

void FeatureSwitch::Enable(const std::string_view& aFeatureOrCollection)
{
	SetState(aFeatureOrCollection, State::Enabled);
}

void FeatureSwitch::Disable(const std::string_view& aFeatureOrCollection)
{
	SetState(aFeatureOrCollection, State::Disabled);
}

bool FeatureSwitch::IsEnabled(const std::string& aFeature)
{
	decltype(myFeatures)::iterator it = myFeatures.find(aFeature);
	if (it == myFeatures.end())
		return false;

	return it->second == State::Enabled;
}

void FeatureSwitch::SetState(const std::string_view& aFeatureOrCollection, State aState)
{
	std::stack<std::string> stack;
	stack.emplace(aFeatureOrCollection);

	while (!stack.empty())
	{
		std::string feature = stack.top();
		stack.pop();
		decltype(myCollections)::iterator collectionIt = myCollections.find(feature);
		if (collectionIt != myCollections.end())
		{
			for (std::string& subfeature : collectionIt->second)
			{
				stack.push(subfeature);
			}
		}
		else
		{
			myFeatures[feature] = aState;
		}
	}
}
