#pragma once
#include <Geode/Geode.hpp>
#include "Utils.hpp"
#include <chrono>

using namespace geode::prelude;

// Forward declare cocos2d types for public API without requiring full includes
namespace cocos2d { class CCNode; class CCScene; }

/// Extracted toast state — safe to store across frames with no pointer lifetime issues.
struct ToastData {
    Category* category = nullptr;
    int oldValue = 0;
    int newValue = 0;
    int goalValue = 0;
    float remainingTime = 0.f;
};

class NotificationSystem {
public:
    static NotificationSystem* get();

    // Internal type (defined in .cpp as private nested class)
    class ToastNode;

    /// Show a toast immediately on the current scene
    void showToast(Category* category, int oldValue, int newValue, int goalValue);

    /// Show a toast resumed at final state (no animation, no slide-in)
    /// Used when the old toast was destroyed by scene cleanup and we recreate on new scene
    void showToastResumed(Category* category, int oldValue, int newValue, int goalValue, float remainingTime);

    /// Store a toast to be shown later (survives scene transitions)
    void showDeferredToast(Category* category, int oldValue, int newValue, int goalValue);

    /// Show all pending deferred toasts on the current running scene
    void flushPendingToasts();

    /// Show all pending deferred toasts on a specific scene (for scene transitions)
    void flushToScene(CCScene* scene);

    /// Check if there's an active toast being displayed
    bool hasActiveToast() const { return m_currentToast != nullptr; }

    /// Detach the active toast for scene transition.
    /// Extracts all data from the toast immediately (no raw pointers stored),
    /// kills old actions to prevent stale callbacks, and clears tracking.
    /// Returns true if there was an active toast with valid data.
    bool detachForReparent(ToastData& outData);

    /// Dismiss the current toast immediately
    void dismissCurrent();

    /// Force-clear all pending toasts
    void clearAll();

    /// Clear stale toast reference after scene transition (node may be destroyed)
    void clearStaleState();

    /// Check if there are pending deferred toasts waiting to be shown
    bool isPendingEmpty() const { return m_deferred.empty(); }

    void onToastDone();

    /// Process the next pending toast. Safe to call from any context.
    /// Must NOT be called from inside CCActionManager::update() — use deferred
    /// scheduling instead (see QueueProcessHelper in .cpp).
    void processQueue();

private:
    NotificationSystem() {}

    ToastNode* m_currentToast = nullptr;
    std::vector<std::tuple<Category*, int, int, int>> m_pending; // active queue
    std::vector<std::tuple<Category*, int, int, int>> m_deferred; // for scene transitions
    bool m_animating = false;
    CCScene* m_flushScene = nullptr; // override target scene for next flush
};
