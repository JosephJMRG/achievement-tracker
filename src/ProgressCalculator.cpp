#include "ProgressCalculator.hpp"
#include "Utils.hpp"

// ── Unified progress calculation ──
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