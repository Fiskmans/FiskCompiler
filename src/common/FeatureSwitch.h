#ifndef COMMON_FEATURE_SWITCH_H
#define COMMON_FEATURE_SWITCH_H

#include <string_view>
#include <unordered_map>
#include <string>

class FeatureSwitch
{
public:
	FeatureSwitch(const std::string& aConfigFile);

	void Enable(const std::string_view& aFeatureOrCollection);
	void Disable(const std::string_view& aFeatureOrCollection);

	bool IsEnabled(const std::string& aFeature);

private:
	enum class State
	{
		Enabled,
		Disabled
	};

	void SetState(const std::string_view& aFeatureOrCollection, State aState);

	std::unordered_map<std::string, std::vector<std::string>> myCollections;
	std::unordered_map<std::string, State> myFeatures;
};

#endif 