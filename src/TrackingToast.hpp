#pragma once
#include <Geode/Geode.hpp>
#include "Utils.hpp"

using namespace geode::prelude;

/// Minimal toast system: full visual tracking card with slide-in/out animation,
/// counter animation, progress bar, and serial queue.
/// No scene-transition handling, no deferred queue — just show and queue.
class TrackingToast {
public:
    static TrackingToast* get();

    /// Show a toast. If one is already animating, it queues.
    void showToast(Category* category, int oldValue, int newValue, int goalValue);

    /// Dismiss current toast immediately
    void dismissCurrent();

    /// Clear everything
    void clearAll();

    // Internal (called by ToastNode / QueueHelper)
    void onToastDone();
    void processNext();

private:
    class ToastNode;

    TrackingToast() {}
    ToastNode* m_currentToast = nullptr;
    bool m_animating = false;
    std::vector<std::tuple<Category*, int, int, int>> m_pending;
};
