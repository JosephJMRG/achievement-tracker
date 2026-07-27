#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <string>
#include "Utils.hpp"

using namespace geode::prelude;

UnlockType unlockTypeFromString(const std::string& str);

// Returns the number of completed achievements (or current stat value for progress categories).
int computeCategoryProgress(
    const std::string& displayType,
    const std::vector<Achievement*>& achievements,
    const std::string& statKey,
    int goalHint = 0);

// Get the official target limit for an achievement from GD's data
int getAchievementLimit(const std::string& achievementId);

// Get the official completion percentage (0-100) for an achievement from GD's data
int getAchievementPercent(const std::string& achievementId);