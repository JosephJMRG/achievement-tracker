#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GameStatsManager.hpp>
#include <Geode/binding/AchievementNotifier.hpp>
#include <Geode/modify/AppDelegate.hpp>
#include "AchievementMenu.hpp"
#include "popups/AchievementCategoryPopup.hpp"
#include "NotificationSystem.hpp"
#include "TrackingManager.hpp"
#include "Utils.hpp"

using namespace geode::prelude;

// Set to 1 to enable debug toast buttons (TOAST / x3) on the main menu
#define DEBUG_TOASTS 0

// Cache the target keycode at mod load
static int s_targetKey = 0xBC; // VK_OEM_COMMA

$on_mod(Loaded) {
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

        // Flush any deferred toasts from gameplay (onQuit / levelComplete)
        NotificationSystem::get()->flushPendingToasts();

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
        int oldVal = goal * (60 + rand() % 16) / 100;
        int newVal = goal * (80 + rand() % 16) / 100;
        if (newVal <= oldVal) newVal = oldVal + goal / 20;
        if (newVal > goal) newVal = goal;
        NotificationSystem::get()->showToast(cat, oldVal, newVal, goal);
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
            int oldVal = goal * pctMin / 100;
            int newVal = goal * (pctMin + 20) / 100;
            NotificationSystem::get()->showToast(cat, oldVal, newVal, goal);
        }
    }
#endif
};

// Global keyboard hook
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double timestamp) {
        // FIX: When a toast is active, consume ESC/back before it reaches GD's handler.
        // GD routes ESC through the scene tree and it can reach the toast's CCMenu,
        // triggering onClose() → onHidden() → processQueue() → runAction() INSIDE
        // CCActionManager::update() → corrupts the action manager's hash table → staged crash.
        if (isKeyDown && !isKeyRepeat && static_cast<int>(key) == 27 /* VK_ESCAPE */) {
            if (NotificationSystem::get()->hasActiveToast()) {
                log::info("[Toast] ESC consumed — toast is active, blocking to prevent close+crash");
                return true;
            }
        }

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
//
// Persistence: toasts survive scene transitions by re-parenting via
// AchievementNotifier::willSwitchToScene, the same mechanism GD uses
// for its own AchievementBar popups.

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
                // Only show toast for tracked categories
                if (!TrackingManager::get()->isCategoryTracked(cat.name)) {
                    log::info("[Toast] {} changed but NOT tracked, skipping", cat.name);
                    break;
                }

                int goal = getNextMilestone(cat, change.newValue);
                log::info("[Toast] {} changed: {} -> {} (milestone: {})",
                    cat.name, change.oldValue, change.newValue, goal);
                NotificationSystem::get()->showDeferredToast(
                    const_cast<Category*>(&cat), change.oldValue, change.newValue, goal);
                break;
            }
        }
        if (!found) {
            log::info("[Toast] No category found for statKey='{}' (value {} -> {})",
                key, change.oldValue, change.newValue);
        }
    }
    s_pendingChanges.clear();
}

// Hook: GD tells us exactly when a stat changes and by how much
// Captures stat changes BOTH during gameplay (accumulated, shown on level exit)
// AND outside gameplay (immediately flushed, e.g. quest rewards, rating, liking).
class $modify(GameStatsManagerHook, GameStatsManager) {
    void incrementStat(char const* key, int amount) {
        GameStatsManager::incrementStat(key, amount);

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
            NotificationSystem::get()->flushPendingToasts();
        }
    }
};

static bool s_toastsAlreadyFlushed = false;

class $modify(PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        log::info("[Toast] PlayLayer::init — starting tracking session");

        // Clear any stale toast state from previous scene — the old toast node
        // may have been destroyed when the scene transitioned.
        NotificationSystem::get()->clearStaleState();

        s_inGameplay = true;
        s_pendingChanges.clear();
        s_toastsAlreadyFlushed = false;

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
            log::info("[Toast] PlayLayer::onQuit — accumulating toasts");
            showAccumulatedToasts();
        }

        PlayLayer::onQuit();  // triggers scene/layer change

        // Toasts will be flushed/re-parented by AchievementNotifierHook::willSwitchToScene
        // when the level page scene loads — same mechanism GD uses for its own bars.
        if (!NotificationSystem::get()->isPendingEmpty()) {
            log::info("[Toast] PlayLayer::onQuit — deferred (waiting for willSwitchToScene)");
        }
    }

    void levelComplete() {
        PlayLayer::levelComplete();  // GD awards Stars, Coins, etc. here FIRST

        // Now s_pendingChanges has all stats (including completion-only ones)
        log::info("[Toast] PlayLayer::levelComplete — accumulating toasts after GD awards");
        showAccumulatedToasts();
        s_toastsAlreadyFlushed = true;

        // Flush after accumulation — toast will show on the current scene.
        log::info("[Toast] PlayLayer::levelComplete — flushing after popup");
        NotificationSystem::get()->flushPendingToasts();
    }

    void resetLevel() {
        PlayLayer::resetLevel();
    }
};

// ─── Re-parent toasts across scene transitions (like GD's AchievementBar) ─
// When a scene transition happens, we extract the active toast's data
// (category, values, remaining time) into a ToastData struct — no raw pointers.
// On the next frame, we create a fresh toast on the new scene from that data.

class ToastReparentHelper : public CCObject {
public:
    ToastData m_data;
    bool m_hasData = false;
    CCScene* m_scene = nullptr;

    void scheduleReparent(const ToastData& data, CCScene* scene) {
        // Cancel any pending reparent
        if (m_scene) {
            m_scene->release();
            m_scene = nullptr;
        }
        auto* sched = CCDirector::sharedDirector()->getScheduler();
        sched->unscheduleSelector(schedule_selector(ToastReparentHelper::executeReparent), this);

        m_data = data;
        m_hasData = true;
        m_scene = scene;
        m_scene->retain();

        sched->scheduleSelector(schedule_selector(ToastReparentHelper::executeReparent),
                                this, 0.f, false);
    }

    void executeReparent(float) {
        // One-shot: unschedule immediately
        CCDirector::sharedDirector()->getScheduler()->unscheduleSelector(
            schedule_selector(ToastReparentHelper::executeReparent), this);

        if (!m_hasData || !m_scene || !m_data.category) {
            cleanupHelper();
            return;
        }

        // Create a fresh toast on the new scene from extracted data.
        // No raw pointers — the old node is long gone by now.
        log::info("[Toast] executeReparent: creating fresh toast on new scene for {}", m_data.category->name);
        NotificationSystem::get()->showToastResumed(
            m_data.category, m_data.oldValue, m_data.newValue,
            m_data.goalValue, m_data.remainingTime);

        cleanupHelper();
    }

    void cleanupHelper() {
        m_hasData = false;
        if (m_scene) { m_scene->release(); m_scene = nullptr; }
        CCDirector::sharedDirector()->getScheduler()->unscheduleSelector(
            schedule_selector(ToastReparentHelper::executeReparent), this);
    }
};

static ToastReparentHelper* s_reparentHelper = nullptr;

class $modify(AppDelegateHook, AppDelegate) {
    void willSwitchToScene(CCScene* scene) {
        AppDelegate::willSwitchToScene(scene);

        auto* ns = NotificationSystem::get();

        // 1) Extract active toast data immediately (no pointer storage)
        ToastData data;
        if (ns->detachForReparent(data)) {
            log::info("[Toast] willSwitchToScene: deferring toast reparent to next frame");
            if (!s_reparentHelper) {
                s_reparentHelper = new ToastReparentHelper();
            }
            s_reparentHelper->scheduleReparent(data, scene);
        }

        // 2) Flush any deferred toasts directly onto the new scene
        ns->flushToScene(scene);
    }
};
