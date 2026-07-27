#include "ProgressCalculator.hpp"
#include "Utils.hpp"

// Map a string like "icon", "ship", "ball", etc. to UnlockType
UnlockType unlockTypeFromString(const std::string& str) {
    if (str == "icon")       return UnlockType::Cube;
    if (str == "ship")       return UnlockType::Ship;
    if (str == "ball")       return UnlockType::Ball;
    if (str == "bird")       return UnlockType::Bird;
    if (str == "dart")       return UnlockType::Dart;
    if (str == "robot")      return UnlockType::Robot;
    if (str == "spider")     return UnlockType::Spider;
    if (str == "special" || str == "trail") return UnlockType::Streak;
    if (str == "death")      return UnlockType::Death;
    if (str == "swing")      return UnlockType::Swing;
    if (str == "jetpack")    return UnlockType::Jetpack;
    if (str == "shipfire")   return UnlockType::ShipFire;
    if (str == "color" || str == "colour") return UnlockType::Col1;
    if (str == "color2" || str == "colour2") return UnlockType::Col2;
    if (str == "item")       return UnlockType::GJItem;
    // "set"/"setmaterial" and "screen"/"screenfire" are not in the UnlockType enum in this Geode version
    if (str == "set" || str == "setmaterial") return UnlockType::GJItem;
    if (str == "screen" || str == "screenfire") return UnlockType::GJItem;
    // Fallback
    return UnlockType::GJItem;
}

// ── Capa 2: Calculo unificado de progreso ──
int computeCategoryProgress(
    const std::string& displayType,
    const std::vector<Achievement*>& achievements,
    const std::string& statKey,
    int goalHint)
{
    if (displayType == "progress" && !statKey.empty()) {
        // Progreso basado en un stat global (ej: estrellas, diamantes, etc.)
        return (statKey == "followed_creators")
            ? gameLevelManager->m_followedCreators->count()
            : gameStatsManager->getStat(statKey.c_str());
    } else {
        return countEarnedAchievements(achievements);
    }
}

// Get the official target limit for an achievement from GD's data
int getAchievementLimit(const std::string& achievementId) {
    return AchievementManager::sharedState()->limitForAchievement(achievementId.c_str());
}

// Get the official completion percentage (0-100) for an achievement from GD's data
int getAchievementPercent(const std::string& achievementId) {
    return AchievementManager::sharedState()->percentForAchievement(achievementId.c_str());
}