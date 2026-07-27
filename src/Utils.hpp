#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

using namespace geode::prelude;

// ──── Bridge types for compatibility with AchievementsReimagined popups ────
// We wrap AchievementTracker data into these structures so the existing popup code works.

struct Achievement {
    std::string id;
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
extern std::map<std::string, std::pair<std::string, std::string>> betterDescriptions;

// ──── Global shared categories ────
extern std::vector<Category> s_achievementCategories;
void buildSharedCategories(); // populates s_achievementCategories once

// ──── Helper functions ────
cocos2d::CCNode* createFractionLabel(int num, int denom);
std::string formatWithCommas(int number);
int extractValue(const std::string& desc);
void addCornerSprites(cocos2d::CCLayer* layer);

// Creates a GJItemIcon for an achievement. If earned, uses player colors (when
// usePlayerColors is true) and applies glow. If not earned, creates a gray
// browser item. Caller is responsible for adding a lock sprite if !earned.
GJItemIcon* createAchievementIcon(const Achievement* ach, bool earned, bool usePlayerColors);

// Creates an achievement icon button with lock overlay and callback data.
// Returns a CCMenuItemSpriteExtra ready to be added to a menu.
// Caller provides the parent popup (for menu_selector), the selector, and a tag for naming.
CCMenuItemSpriteExtra* createAchievementIconButton(
    Achievement* ach,
    bool earned,
    bool usePlayerColors,
    CCObject* target,
    SEL_MenuHandler selector,
    const std::string& tag = ""
);

// Creates a jumping player icon with glow, rotated 50°. Caller positions and adds to parent.
GJItemIcon* createJumpsIcon();

// Find the next unearned milestone value in a progress category.
// Returns the smallest achievement unlockValue > currentValue,
// or currentValue if all milestones are reached.
int getNextMilestone(const Category& cat, int currentValue);

// ──── Category tracking (persisted via GameManager) ────
bool isCategoryTracked(const std::string& categoryName);
void setCategoryTracked(const std::string& categoryName, bool tracked);
void migrateTrackingData(); // one-time migration from old JSON format

// Count how many achievements in the list are earned
int countEarnedAchievements(const std::vector<Achievement*>& achievements);

// ──── Progress bar background helper ────
struct ProgressBarBgParams {
    int numDots;           // total number of dots
    int numIconsOnPage;    // number of icons (for positioning offset)
    float dotSpacing;      // spacing between dots
    float dotOffset = 0.f; // extra horizontal offset (PathPopup uses dotSpacing/2)
    bool skipFirstVerticalBar = true;  // skip vertical bar at i==0
    bool invertVerticalBarAnchors = false; // PathPopup inverts anchor logic
};

// Creates the dark background of a progress bar: bar sprite, dots, and vertical connectors.
cocos2d::CCNode* buildProgressBarBg(const ProgressBarBgParams& params);

// ──── Progress bar fill helper ────
struct ProgressFillParams {
    float dotSpacing;
    float numIconsOnPage;
    cocos2d::ccColor3B fillColor;
};

// Creates filled dots and bar segments for a progress bar (ShardPopup/ProgressPopup style).
// numDotsToFill: number of colored dots to create (caller calculates from unlock count).
// pageStartIndex: global index of first achievement on this page.
// statValue: current stat value for ratio calculation.
// globalProgress: if true, "is first" ratio check uses global index; if false, uses page-relative.
cocos2d::CCNode* buildProgressFill(
    int numDotsToFill,
    int numDotsOnPage,
    const std::vector<Achievement*>& achievements,
    int pageStartIndex,
    float statValue,
    const ProgressFillParams& params,
    bool globalProgress = false
);
