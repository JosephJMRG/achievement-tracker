#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <string>
#include "Utils.hpp"

using namespace geode::prelude;

struct ProgressResult {
    int completed = 0;
    int total = 0;
    int percentage = 0;
};

UnlockType unlockTypeFromString(const std::string& str);

// Compute category progress based on display type
ProgressResult computeCategoryProgress(
    const std::string& displayType,
    const std::vector<Achievement*>& achievements,
    const std::string& statKey,
    int goalHint = 0);