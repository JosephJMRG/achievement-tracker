#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <regex>
#include <map>
#include <tuple>

using namespace geode::prelude;

// ──── Bridge types for compatibility with AchievementsReimagined popups ────
// We wrap AchievementTracker data into these structures so the existing popup code works.

struct Achievement {
    std::string id;
    std::string title;
    std::string achievedDescription;
    std::string unachievedDescription;
    UnlockType unlockType;
    int unlockID;
    int unlockValue;
};

struct Category {
    std::string name;
    std::string formattedName;  // with new lines
    std::string page;           // which page: "Levels", "Stats", or "Other"
    std::string displayType;    // "distinct" or "progress" or "shard" or "path"
    std::string logo;
    std::vector<std::string> identifiers;
    std::string statKey;
    std::vector<Achievement*> achievements;
};

struct IconCallbackData : public cocos2d::CCObject {
    UnlockType unlockType;
    int unlockID;
    std::string unlockedDescription;

    IconCallbackData(UnlockType type, int id, std::string desc) : unlockType(type), unlockID(id), unlockedDescription(desc) {}
};

// ──── Global pointers ────
extern AchievementManager* achievementManager;
extern GameManager* gameManager;
extern GameStatsManager* gameStatsManager;
extern GameLevelManager* gameLevelManager;

// ──── Better descriptions for secrets/vaults ────
extern std::map<std::string, std::tuple<std::string, std::string>> betterDescriptions;

// ──── Global shared categories ────
extern std::vector<Category> s_achievementCategories;
void buildSharedCategories(); // populates s_achievementCategories once

// ──── Helper functions ────
cocos2d::CCNode* createFractionLabel(int num, int denom);
std::string formatWithCommas(int number);
int extractValue(const std::string& desc);
void addCornerSprites(cocos2d::CCLayer* layer);

// Find the next unearned milestone value in a progress category.
// Returns the smallest achievement unlockValue > currentValue,
// or currentValue if all milestones are reached.
int getNextMilestone(const Category& cat, int currentValue);
