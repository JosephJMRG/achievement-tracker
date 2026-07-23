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
ProgressResult computeCategoryProgress(
    const std::string& displayType,
    const std::vector<Achievement*>& achievements,
    const std::string& statKey,
    int goalHint)
{
    ProgressResult result;
    result.total = static_cast<int>(achievements.size());

    if (displayType == "progress" && !statKey.empty()) {
        // Progreso basado en un stat global (ej: estrellas, diamantes, etc.)
        int current = (statKey == "followed_creators")
            ? gameLevelManager->m_followedCreators->count()
            : gameStatsManager->getStat(statKey.c_str());
        int goal = goalHint;
        if (goal == 0 && !achievements.empty()) {
            goal = achievements.back()->unlockValue;
        }
        result.completed = current;
        result.total = goal;
        result.percentage = (goal > 0) ? std::min(100, static_cast<int>(100.f * current / goal)) : 0;
    } else {
        // Conteo simple de achievements earned (distinct, shard, path, progress sin statKey, fallback)
        for (auto* ach : achievements) {
            if (achievementManager->isAchievementEarned(ach->id.c_str())) {
                result.completed++;
            }
        }
        result.percentage = (result.total > 0) ? static_cast<int>(100.f * result.completed / result.total) : 0;
    }

    return result;
}