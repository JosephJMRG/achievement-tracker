#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <string>
#include "Utils.hpp"

using namespace geode::prelude;

// Returns the number of completed achievements (or current stat value for progress categories).
int computeCategoryProgress(
    const std::string& displayType,
    const std::vector<Achievement*>& achievements,
    const std::string& statKey,
    int goalHint = 0);