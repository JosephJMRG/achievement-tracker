#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GameStatsManager.hpp>
#include "TrackingToast.hpp"
#include "AchievementMenu.hpp"
#include "popups/AchievementCategoryPopup.hpp"
#include "Utils.hpp"

using namespace geode::prelude;

// Set to 1 to enable debug toast buttons (TOAST / x3) on the main menu
#define DEBUG_TOASTS 0

// Cache the target keycode at mod load
static int s_targetKey = 0xBC; // VK_OEM_COMMA

$on_mod(Loaded) {
    // One-time migration from old JSON tracking data to GameManager variables
    migrateTrackingData();

    auto keyStr = Mod::get()->getSettingValue<std::string>("toggle-key");

    // Map setting string to Windows VK code
    if (keyStr == "Comma" || keyStr == "comma")         s_targetKey = 0xBC;
    else if (keyStr == "Period" || keyStr == "period")  s_targetKey = 0xBE;
    else if (keyStr == "Slash" || keyStr == "slash")    s_targetKey = 0xBF;
    else if (keyStr == "A" || keyStr == "a")             s_targetKey = 'A';
    else if (keyStr == "B" || keyStr == "b")             s_targetKey = 'B';
    else if (keyStr == "F1")  s_targetKey = VK_F1;
    else if (keyStr == "F2")  s_targetKey = VK_F2;
    else if (keyStr == "F3")  s_targetKey = VK_F3;
    else if (keyStr == "F4")  s_targetKey = VK_F4;
    else if (keyStr == "F5")  s_targetKey = VK_F5;
    else if (keyStr == "F6")  s_targetKey = VK_F6;
    else if (keyStr == "F7")  s_targetKey = VK_F7;
    else if (keyStr == "F8")  s_targetKey = VK_F8;
    else if (keyStr == "F9")  s_targetKey = VK_F9;
    else if (keyStr == "F10") s_targetKey = VK_F10;
    else if (keyStr == "F11") s_targetKey = VK_F11;
    else if (keyStr == "F12") s_targetKey = VK_F12;

    log::info("Achievement Tracker loaded! Key: {} (0x{:X})", keyStr, s_targetKey);
}

// ─── Replace the default achievements button handler in MenuLayer ─────
class $modify(AchievementMenuLayer, MenuLayer) {
    void onAchievements(CCObject* sender) {
        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (!scene) return;

        // If our menu is already open, close it
        auto existing = scene->getChildByID("achievement-menu"_spr);
        if (existing) {
            existing->removeFromParent();
            return;
        }

        // Open our custom achievement menu
        auto layer = AchievementMenu::create();
        if (layer) {
            layer->setID("achievement-menu"_spr);
            scene->addChild(layer, 100);
        }
    }

    bool init() {
        if (!MenuLayer::init()) return false;

#if DEBUG_TOASTS
        // Debug: add test toast buttons in the top-left corner
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto menu = CCMenu::create();
        menu->setID("debug-toast-menu"_spr);
        menu->setPosition({55.f, winSize.height - 22.f});

        auto spr = ButtonSprite::create("TOAST", 60, true, "bigFont.fnt", "GJ_button_04.png", 30, 0.4f);
        spr->setScale(0.6f);
        auto btn = CCMenuItemSpriteExtra::create(
            spr, this, menu_selector(AchievementMenuLayer::onDebugToast));
        btn->setID("debug-toast-btn"_spr);
        menu->addChild(btn);

        auto spr2 = ButtonSprite::create("x3", 40, true, "bigFont.fnt", "GJ_button_05.png", 30, 0.4f);
        spr2->setScale(0.6f);
        auto btn2 = CCMenuItemSpriteExtra::create(
            spr2, this, menu_selector(AchievementMenuLayer::onDebugMultiToast));
        btn2->setID("debug-toast-multi"_spr);
        menu->addChild(btn2);

        menu->setLayout(RowLayout::create()->setGap(5.f));
        menu->updateLayout();
        addChild(menu, 999);
#endif

        return true;
    }

#if DEBUG_TOASTS
    void onDebugToast(CCObject*) {
        buildSharedCategories();
        std::vector<Category*> candidates;
        for (auto& cat : s_achievementCategories) {
            if (cat.displayType == "progress" && !cat.achievements.empty())
                candidates.push_back(&cat);
        }
        if (candidates.empty()) return;
        Category* cat = candidates[rand() % candidates.size()];
        int goal = 0;
        for (const auto* ach : cat->achievements) {
            if (ach->unlockValue > goal) goal = ach->unlockValue;
        }
        if (goal <= 0) return;
        int newVal = goal * (80 + rand() % 16) / 100;
        if (newVal > goal) newVal = goal;
        TrackingToast::get()->showToast(cat, 0, newVal, goal);
    }

    void onDebugMultiToast(CCObject*) {
        buildSharedCategories();
        std::vector<Category*> candidates;
        for (auto& cat : s_achievementCategories) {
            if (cat.displayType == "progress" && !cat.achievements.empty())
                candidates.push_back(&cat);
        }
        if (candidates.empty()) return;
        for (int i = 0; i < 3 && i < (int)candidates.size(); i++) {
            Category* cat = candidates[i];
            int goal = 0;
            for (const auto* ach : cat->achievements) {
                if (ach->unlockValue > goal) goal = ach->unlockValue;
            }
            if (goal <= 0) continue;
            int pctMin = 30 + i * 20;
            int newVal = goal * (pctMin + 20) / 100;
            TrackingToast::get()->showToast(cat, 0, newVal, goal);
        }
    }
#endif
};

// Global keyboard hook
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double timestamp) {
        if (isKeyDown && !isKeyRepeat && static_cast<int>(key) == s_targetKey) {
            auto scene = CCDirector::sharedDirector()->getRunningScene();
            if (!scene) return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, timestamp);

            // Priority 1: if a category popup is open, close it (go back to main menu)
            AchievementCategoryPopup* catPopup = nullptr;
            for (auto child : CCArrayExt<CCNode*>(scene->getChildren())) {
                catPopup = typeinfo_cast<AchievementCategoryPopup*>(child);
                if (catPopup) break;
            }
            if (catPopup) {
                catPopup->onClose(nullptr);
                return true;
            }

            // Priority 2: if the main menu is open, close it
            auto existing = scene->getChildByID("achievement-menu"_spr);
            if (existing) {
                existing->removeFromParent();
                return true;
            }

            // Priority 3: open the main menu
            auto layer = AchievementMenu::create();
            if (layer) {
                layer->setID("achievement-menu"_spr);
                scene->addChild(layer, 100);
            }
            return true;
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, timestamp);
    }
};

// ─── Progress detection via GameStatsManager::incrementStat hook ───────────
// GD calls incrementStat() every time a stat changes. We accumulate changes
// during gameplay and show toasts when the player leaves the level.
// Deferred toasts are flushed in MenuLayer::init when the player returns to menu.

struct StatChange {
    int oldValue;  // value before first increment in this session
    int newValue;  // latest value after all increments
};
static std::unordered_map<std::string, StatChange> s_pendingChanges;
static bool s_inGameplay = false;

// GJGameLevel fallback: snapshot per-level values at init
static int s_initLevelJumps = 0;
static int s_initLevelAttempts = 0;
static int s_globalJumpsBaseline = 0;
static int s_globalAttemptsBaseline = 0;

// Merge GJGameLevel delta into pendingChanges (fallback for incrementStat misses)
static void backfillFromLevelStats() {
    auto* level = PlayLayer::get();
    if (!level || !level->m_level) return;

    auto* gLevel = level->m_level;
    auto* gsm = GameStatsManager::sharedState();

    // Jumps
    int jumpsNow = static_cast<int>(gLevel->m_jumps.value());
    int jumpsDelta = jumpsNow - s_initLevelJumps;
    if (jumpsDelta > 0) {
        std::string key = "1";
        auto it = s_pendingChanges.find(key);
        if (it == s_pendingChanges.end()) {
            int newVal = s_globalJumpsBaseline + jumpsDelta;
            s_pendingChanges[key] = { s_globalJumpsBaseline, newVal };
            log::info("[Toast] Backfill Jumps from GJGameLevel: {} -> {} (delta: {})",
                s_globalJumpsBaseline, newVal, jumpsDelta);
        } else if (it->second.newValue <= it->second.oldValue) {
            // incrementStat recorded no change — override with GJGameLevel delta
            int newVal = s_globalJumpsBaseline + jumpsDelta;
            it->second.newValue = newVal;
            log::info("[Toast] Backfill Jumps (override): {} -> {}", it->second.oldValue, newVal);
        }
    }

    // Attempts
    int attemptsNow = static_cast<int>(gLevel->m_attempts.value());
    int attemptsDelta = attemptsNow - s_initLevelAttempts;
    if (attemptsDelta > 0) {
        std::string key = "2";
        auto it = s_pendingChanges.find(key);
        if (it == s_pendingChanges.end()) {
            int newVal = s_globalAttemptsBaseline + attemptsDelta;
            s_pendingChanges[key] = { s_globalAttemptsBaseline, newVal };
            log::info("[Toast] Backfill Attempts from GJGameLevel: {} -> {} (delta: {})",
                s_globalAttemptsBaseline, newVal, attemptsDelta);
        } else if (it->second.newValue <= it->second.oldValue) {
            int newVal = s_globalAttemptsBaseline + attemptsDelta;
            it->second.newValue = newVal;
            log::info("[Toast] Backfill Attempts (override): {} -> {}", it->second.oldValue, newVal);
        }
    }
}

static void showAccumulatedToasts() {
    // First, backfill from GJGameLevel in case incrementStat missed something
    backfillFromLevelStats();

    buildSharedCategories();
    for (auto& [key, change] : s_pendingChanges) {
        if (change.newValue <= change.oldValue) continue;

        bool found = false;
        for (auto& cat : s_achievementCategories) {
            if (cat.statKey == key && cat.displayType == "progress") {
                found = true;
                if (!isCategoryTracked(cat.name)) {
                    log::info("[Toast] {} changed but NOT tracked, skipping", cat.name);
                    break;
                }

                int goal = getNextMilestone(cat, change.newValue);
                log::info("[Toast] {} changed: {} -> {} (milestone: {})",
                    cat.name, change.oldValue, change.newValue, goal);
                
                // Show tracking toast with full visual (overlay survives scene transitions)
                TrackingToast::get()->showToast(&cat, change.oldValue, change.newValue, goal);
                break;
            }
        }
        if (!found) {
            log::info("[Toast] No category found for statKey='{}' (value {} -> {})",
                key, change.oldValue, change.newValue);
        }
    }
    // Force GD to re-evaluate achievements for all changed stats
    for (auto& [key, change] : s_pendingChanges) {
        if (change.newValue > change.oldValue)
            GameStatsManager::sharedState()->checkAchievement(key.c_str());
    }
    s_pendingChanges.clear();
}

// Hook: GD tells us exactly when a stat changes and by how much
// Captures stat changes BOTH during gameplay (accumulated, shown on level exit)
// AND outside gameplay (immediately flushed, e.g. quest rewards, rating, liking).
class $modify(GameStatsManagerHook, GameStatsManager) {
    void incrementStat(char const* key, int amount) {
        GameStatsManager::incrementStat(key, amount);

        // Force GD to re-evaluate achievements that depend on this stat
        GameStatsManager::sharedState()->checkAchievement(key);

        std::string keyStr(key);
        auto* gsm = GameStatsManager::sharedState();
        int newVal = gsm->getStat(key);

        auto it = s_pendingChanges.find(keyStr);
        if (it == s_pendingChanges.end()) {
            s_pendingChanges[keyStr] = { newVal - amount, newVal };
            log::info("[Toast] incrementStat: key={} {} -> {} (inGameplay={})",
                keyStr, newVal - amount, newVal, s_inGameplay);
        } else {
            it->second.newValue = newVal;
        }

        // OUTSIDE GAMEPLAY: stat changed on menus/quests/etc.
        // Accumulate → immediately create deferred toasts → flush to current scene.
        // No need to wait for a level exit — these are one-shot events.
        if (!s_inGameplay) {
            showAccumulatedToasts();
        }
    }
};

static bool s_toastsAlreadyFlushed = false;

class $modify(PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        // IMPORTANT: Mark gameplay state BEFORE GD's init, because GD calls
        // incrementStat("2", 1) (attempt counter) during PlayLayer::init().
        // If s_inGameplay is still false at that point, the hook flushes
        // the toast immediately instead of accumulating it for level end.
        s_inGameplay = true;
        s_pendingChanges.clear();
        s_toastsAlreadyFlushed = false;

        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        log::info("[Toast] PlayLayer::init — starting tracking session");

        // Snapshot GJGameLevel per-level values for fallback
        s_initLevelJumps = static_cast<int>(level->m_jumps.value());
        s_initLevelAttempts = static_cast<int>(level->m_attempts.value());

        // Snapshot GameStatsManager global baseline
        auto* gsm = GameStatsManager::sharedState();
        s_globalJumpsBaseline = gsm->getStat("1");
        s_globalAttemptsBaseline = gsm->getStat("2");

        log::info("[Toast] Init: levelJumps={}, levelAttempts={}, globalJumps={}, globalAttempts={}",
            s_initLevelJumps, s_initLevelAttempts, s_globalJumpsBaseline, s_globalAttemptsBaseline);
        return true;
    }

    void onQuit() {
        s_inGameplay = false;

        // If levelComplete already accumulated + flushed, skip
        if (!s_toastsAlreadyFlushed) {
            showAccumulatedToasts();
        }

        PlayLayer::onQuit();  // triggers scene/layer change
        // Deferred toasts will be flushed in MenuLayer::init
    }

    void levelComplete() {
        PlayLayer::levelComplete();  // GD awards Stars, Coins, etc. here FIRST

        // Now s_pendingChanges has all stats (including completion-only ones)
        showAccumulatedToasts();
        s_toastsAlreadyFlushed = true;
    }
};
