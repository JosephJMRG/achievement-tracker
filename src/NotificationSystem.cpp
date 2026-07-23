#include "NotificationSystem.hpp"
#include <chrono>

using namespace geode::prelude;

// ─── Deferred queue processor ──────────────────────────────────────────────
// processQueue() must NOT be called inside CCActionManager::update(). When
// onHidden() fires from a CCCallFunc, it's inside the action manager's update
// loop. Calling processQueue() → showToast() → runAction() adds a new action
// to the manager's hash table while it's being iterated → corrupted state →
// staged crash on next scene transition. Instead, we schedule a one-shot
// callback that runs on the next scheduler tick when the action manager is idle.

class QueueProcessHelper : public CCObject {
public:
    void process(float) {
        auto* sched = CCDirector::sharedDirector()->getScheduler();
        if (sched) {
            sched->unscheduleSelector(
                schedule_selector(QueueProcessHelper::process), this);
        }
        NotificationSystem::get()->processQueue();
    }
};

static QueueProcessHelper* s_queueHelper = nullptr;

// ─── ToastNode: like a tracking card, but for stat progress ───────────────

class NotificationSystem::ToastNode : public CCNode {
public:
    static ToastNode* create(Category* category, int oldValue, int newValue, int goalValue) {
        auto ret = new ToastNode();
        if (ret && ret->init(category, oldValue, newValue, goalValue)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    void startAnim(float slideInDuration, float displayDuration, float slideOutDuration) {
        m_displayDuration = displayDuration;
        float cardFullW = m_cardW + 6.f;
        float startX = CCDirector::sharedDirector()->getWinSize().width + cardFullW;
        setPositionX(startX);

        // After slide-in finishes, start the counter animation
        auto slideIn  = CCMoveTo::create(slideInDuration, {m_targetX, getPositionY()});
        auto easeIn   = CCEaseOut::create(slideIn, 2.f);
        auto startCounter = CCCallFunc::create(this, callfunc_selector(ToastNode::startCounterAnim));
        auto wait     = CCDelayTime::create(displayDuration);
        float endX    = CCDirector::sharedDirector()->getWinSize().width + cardFullW;
        auto slideOut = CCMoveTo::create(slideOutDuration, {endX, getPositionY()});
        auto easeOut  = CCEaseIn::create(slideOut, 2.f);
        auto done     = CCCallFunc::create(this, callfunc_selector(ToastNode::onHidden));

        runAction(CCSequence::create(easeIn, startCounter, wait, easeOut, done, nullptr));
    }

    void startCounterAnim() {
        if (m_oldValue >= m_newValue) {
            updateStatLabel(m_newValue);
            updatePctLabel(m_newValue);
            return;
        }
        // Schedule update every frame for ~0.8s counting animation
        m_counterElapsed = 0.f;
        m_counterDuration = 0.8f;
        schedule(schedule_selector(ToastNode::tickCounter), 0.f);
    }

    void tickCounter(float dt) {
        m_counterElapsed += dt;
        float t = std::min(1.f, m_counterElapsed / m_counterDuration);
        // Ease-out quad
        float eased = t * (2.f - t);
        int val = m_oldValue + (int)((m_newValue - m_oldValue) * eased);
        if (val < m_oldValue) val = m_oldValue;
        if (val > m_newValue) val = m_newValue;
        updateStatLabel(val);
        updatePctLabel(val);

        if (t >= 1.f) {
            unschedule(schedule_selector(ToastNode::tickCounter));
        }
    }

    void updateStatLabel(int val) {
        if (m_statLabel) {
            std::string text = formatWithCommas(val) + " / " + formatWithCommas(m_goalValue);
            m_statLabel->setString(text.c_str());
        }
    }

    void updatePctLabel(int val) {
        if (m_pctLabel && m_goalValue > 0) {
            int pct = std::min(100, (int)((float)val / (float)m_goalValue * 100.f));
            std::string text = std::to_string(pct) + "%";
            m_pctLabel->setString(text.c_str());
        }
    }

    void onHidden() {
        // CRITICAL: Do NOT call removeFromParent() here.
        // onHidden() is triggered by CCCallFunc inside CCActionManager::update.
        // removeFromParent() → cleanup() → removeAllActionsFromTarget() modifies
        // the action manager's hash table while it's being iterated → crash.
        // Just hide the node and break tracking. Scene cleanup handles the node.
        setVisible(false);
        NotificationSystem::get()->onToastDone();
    }

    void onClose(CCObject*) {
        stopAllActions();
        float cardFullW = m_cardW + 6.f;
        float endX = CCDirector::sharedDirector()->getWinSize().width + cardFullW;
        auto slideOut = CCMoveTo::create(0.25f, {endX, getPositionY()});
        auto easeOut = CCEaseIn::create(slideOut, 2.f);
        auto done = CCCallFunc::create(this, callfunc_selector(ToastNode::onHidden));
        runAction(CCSequence::create(easeOut, done, nullptr));
    }

    /// Show the toast at its final visual state (no slide-in, no counter animation).
    /// Used when the old toast was destroyed by scene cleanup and we recreate on new scene.
    void showFinalStateAndWait(float remainingTime) {
        setPositionX(m_targetX);
        updateStatLabel(m_newValue);
        updatePctLabel(m_newValue);

        if (remainingTime <= 0.1f) {
            onHidden();
            return;
        }

        float cardFullW = m_cardW + 6.f;
        float endX = CCDirector::sharedDirector()->getWinSize().width + cardFullW;

        auto wait     = CCDelayTime::create(remainingTime);
        auto slideOut = CCMoveTo::create(0.35f, {endX, getPositionY()});
        auto easeOut  = CCEaseIn::create(slideOut, 2.f);
        auto done     = CCCallFunc::create(this, callfunc_selector(ToastNode::onHidden));

        runAction(CCSequence::create(wait, easeOut, done, nullptr));
    }

    float getTargetX() const { return m_targetX; }

    // Getters for recreation after scene cleanup
    Category* getCategory() const { return m_category; }
    int getOldValue() const { return m_oldValue; }
    int getNewValue() const { return m_newValue; }
    int getGoalValue() const { return m_goalValue; }
    float getDisplayDuration() const { return m_displayDuration; }
    std::chrono::steady_clock::time_point getCreationTime() const { return m_creationTime; }

private:
    bool init(Category* category, int oldValue, int newValue, int goalValue) {
        if (!CCNode::init()) return false;

        m_creationTime = std::chrono::steady_clock::now();
        m_category = category;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        float cardW = 220.f;
        float cardH = 56.f;
        m_cardW = cardW;

        m_targetX = winSize.width - cardW - 10.f;
        setAnchorPoint({0.f, 0.f});
        setPosition({m_targetX, 30.f});
        setContentSize({cardW, cardH});

        // We use a "content layer" as the card's interior, positioned the same
        // way the tracking card positions its layers. We'll treat x=0, baseY=0
        // as our anchor, matching the tracking card's createPanel convention
        // where x and baseY are the card's top-left position.
        //
        // In the tracking card: createPanel(info, x, baseY) where x,baseY is
        // the card's top-left corner. Here our node IS the card, so x=0, baseY=0.

        float x = 0.f;
        float baseY = 0.f;

        // -- Layer 1: White background (cardW+6 x cardH+6) --
        auto layerWhite = CCScale9Sprite::create("square02b_001.png");
        layerWhite->setContentSize({cardW + 6.f, cardH + 6.f});
        layerWhite->setAnchorPoint({0.f, 0.f});
        layerWhite->setPosition({x - 2.f, baseY - 3.f});
        layerWhite->setColor({255, 255, 255});
        layerWhite->setID("toast-layer-white");
        addChild(layerWhite);

        // -- Layer 2: Dark blue shadow (cardW+4 x cardH+4) --
        auto layerDark = CCScale9Sprite::create("square02b_001.png");
        layerDark->setContentSize({cardW + 4.f, cardH + 4.f});
        layerDark->setAnchorPoint({0.f, 0.f});
        layerDark->setPosition({x - 1.f, baseY - 2.f});
        layerDark->setColor({20, 30, 100});
        layerDark->setID("toast-layer-dark");
        addChild(layerDark);

        // -- Layer 3: Light blue interior (cardW+2 x cardH+2) --
        auto layerLight = CCScale9Sprite::create("square02b_001.png");
        layerLight->setContentSize({cardW + 2.f, cardH + 2.f});
        layerLight->setAnchorPoint({0.f, 0.f});
        layerLight->setPosition({x, baseY - 1.f});
        layerLight->setColor({45, 59, 142});
        layerLight->setID("toast-layer-light");
        addChild(layerLight);

        // === LEFT SIDE ===
        float lx = x + 12.f;
        float offSetYTitleSubtitle = 5.f;

        // Title (goldFont)
        auto titleLabel = CCLabelBMFont::create(category->name.c_str(), "goldFont.fnt");
        titleLabel->setAnchorPoint({0.f, 0.5f});
        titleLabel->setScale(0.65f);
        titleLabel->limitLabelWidth(cardW * 0.55f, 0.65f, 0.1f);
        titleLabel->setPosition({lx, baseY - offSetYTitleSubtitle + cardH - 12.f});
        titleLabel->setID("toast-title");
        addChild(titleLabel);

        // Stat text starts at oldValue, will animate to newValue
        m_oldValue = oldValue;
        m_newValue = newValue;
        m_goalValue = goalValue;

        m_statLabel = CCLabelBMFont::create("", "bigFont.fnt");
        m_statLabel->setAnchorPoint({0.f, 0.5f});
        m_statLabel->setScale(0.47f);
        m_statLabel->limitLabelWidth(cardW * 0.55f, 0.47f, 0.1f);
        m_statLabel->setColor({255, 204, 0});
        m_statLabel->setPosition({lx, baseY - offSetYTitleSubtitle + cardH - 27.f});
        m_statLabel->setID("toast-stat");
        addChild(m_statLabel);

        // Set initial text to oldValue
        updateStatLabel(oldValue);

        // --- Progress bar ---
        float barX = lx;
        float barY = baseY + 10.f;
        float barW = cardW * 0.70f - 20.f;
        float barH = 13.f;
        float ins = 2.f;
        float offSetYBar = 5.f;

        // Blue background (#1a237e)
        auto barBg = CCLayerColor::create({26, 35, 126, 255}, barW, barH);
        barBg->setAnchorPoint({0.f, 0.5f});
        barBg->setPosition({barX, barY - offSetYBar});
        barBg->setID("toast-bar-bg");
        addChild(barBg, 10);
        barBg->setOpacity(255);

        // Green fill (#39ff14): animated from old% to new%
        float oldPct = goalValue > 0 ? std::min(1.f, (float)oldValue / (float)goalValue) : 0.f;
        float newPct = goalValue > 0 ? std::min(1.f, (float)newValue / (float)goalValue) : 0.f;
        float oldFillW = (barW - ins * 2.f) * oldPct;
        float newFillW = (barW - ins * 2.f) * newPct;

        if (newFillW > 0.5f) {
            float fillH = barH - ins * 2.f;
            auto barFill = CCLayerColor::create({57, 255, 20, 255}, std::max(0.5f, oldFillW), fillH);
            barFill->setAnchorPoint({0.f, 0.5f});
            barFill->setPosition({barX + ins, barY + 2.f - offSetYBar});
            barFill->setID("toast-bar-fill");
            addChild(barFill, 10);
            barFill->setOpacity(255);

            // Animate to new width
            if (newFillW > oldFillW + 0.5f) {
                float ratio = newFillW / std::max(0.5f, oldFillW);
                barFill->runAction(CCScaleTo::create(0.8f, ratio, 1.f));
            }
        }

        // === RIGHT SIDE ===
        float toggleX = x + cardW - 8.f;

        // Percentage (gold) — starts at old%, animates with counter
        float textRx = toggleX - 3.f;
        float pctY = baseY + cardH - 30.f;
        int oldPctInt = goalValue > 0 ? std::min(100, (int)((float)oldValue / (float)goalValue * 100.f)) : 0;
        m_pctLabel = CCLabelBMFont::create((std::to_string(oldPctInt) + "%").c_str(), "goldFont.fnt");
        m_pctLabel->setAnchorPoint({1.f, 0.5f});
        m_pctLabel->setScale(0.65f);
        m_pctLabel->setColor({255, 204, 0});
        m_pctLabel->setPosition({textRx, pctY});
        m_pctLabel->setID("toast-pct");
        addChild(m_pctLabel);

        // "Completed" label (gold)
        auto compLabel = CCLabelBMFont::create("Completed", "bigFont.fnt");
        compLabel->setAnchorPoint({1.f, 0.5f});
        compLabel->setScale(0.25f);
        compLabel->setColor({255, 204, 0});
        compLabel->setPosition({textRx, pctY - 12.f});
        compLabel->setID("toast-completed-label");
        addChild(compLabel);

        // Close button (X) — replaces the tracking toggle checkbox
        auto closeSpr = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        closeSpr->setScale(0.50f);
        auto closeBtn = CCMenuItemSpriteExtra::create(
            closeSpr, this, menu_selector(ToastNode::onClose));
        closeBtn->setID("toast-close-btn");
        auto closeMenu = CCMenu::create();
        closeMenu->setPosition({toggleX, baseY + cardH - 9.f});
        closeMenu->addChild(closeBtn);
        addChild(closeMenu);

        return true;
    }

    float m_cardW = 0.f;
    float m_targetX = 0.f;

    // Timing
    std::chrono::steady_clock::time_point m_creationTime;
    float m_displayDuration = 4.0f;

    // Category data (stored for recreation after scene cleanup)
    Category* m_category = nullptr;

    // Counter animation state
    CCLabelBMFont* m_statLabel = nullptr;
    CCLabelBMFont* m_pctLabel = nullptr;
    int m_oldValue = 0;
    int m_newValue = 0;
    int m_goalValue = 0;
    float m_counterElapsed = 0.f;
    float m_counterDuration = 0.8f;
};

// ─── NotificationSystem ───────────────────────────────────────────────────

static NotificationSystem* s_instance = nullptr;

NotificationSystem* NotificationSystem::get() {
    if (!s_instance)
        s_instance = new NotificationSystem();
    return s_instance;
}

void NotificationSystem::showToast(Category* category, int oldValue, int newValue, int goalValue) {
    // If toast was destroyed externally (e.g. scene transition), reset state
    if (m_currentToast && !m_currentToast->getParent()) {
        log::info("[Toast] showToast: stale m_currentToast, resetting");
        m_currentToast = nullptr;
        m_animating = false;
    }

    if (m_animating) {
        log::info("[Toast] showToast: m_animating=true, queuing {} pending={}", category->name, m_pending.size());
        m_pending.emplace_back(category, oldValue, newValue, goalValue);
        return;
    }

    // Use override scene if set (for scene transition flush), otherwise running scene
    auto scene = m_flushScene ? m_flushScene : CCDirector::sharedDirector()->getRunningScene();
    log::info("[Toast] showToast: scene={}, creating toast for {}", (void*)scene, category->name);
    if (!scene) return;

    auto toast = ToastNode::create(category, oldValue, newValue, goalValue);
    if (!toast) {
        log::info("[Toast] showToast: ToastNode::create FAILED for {}", category->name);
        return;
    }

    m_currentToast = toast;
    m_animating = true;

    scene->addChild(toast, 200);
    toast->startAnim(0.35f, 4.0f, 0.35f);
    log::info("[Toast] showToast: toast ADDED to scene for {}", category->name);
}

void NotificationSystem::dismissCurrent() {
    if (m_currentToast && m_currentToast->getParent()) {
        m_currentToast->onClose(nullptr);
    } else {
        m_currentToast = nullptr;
        m_animating = false;
    }
}

void NotificationSystem::clearAll() {
    if (m_currentToast) {
        m_currentToast->stopAllActions();
        if (m_currentToast->getParent()) {
            m_currentToast->removeFromParent();
        }
        m_currentToast = nullptr;
    }
    m_pending.clear();
    m_deferred.clear();
    m_animating = false;
}

void NotificationSystem::clearStaleState() {
    // Don't touch the pointer — node may already be destroyed by scene transition.
    // Just reset our tracking state so we don't dereference a dangling pointer.
    m_currentToast = nullptr;
    m_animating = false;
    m_pending.clear();
    m_deferred.clear();
}

void NotificationSystem::showDeferredToast(Category* category, int oldValue, int newValue, int goalValue) {
    m_deferred.emplace_back(category, oldValue, newValue, goalValue);
    log::info("[Toast] showDeferredToast: {} {}->{}, deferred={}", category->name, oldValue, newValue, m_deferred.size());
}

void NotificationSystem::showToastResumed(Category* category, int oldValue, int newValue, int goalValue, float remainingTime) {
    // If toast was destroyed externally, reset state
    if (m_currentToast && !m_currentToast->getParent()) {
        m_currentToast = nullptr;
        m_animating = false;
    }

    if (m_animating) {
        m_pending.emplace_back(category, oldValue, newValue, goalValue);
        return;
    }

    auto scene = m_flushScene ? m_flushScene : CCDirector::sharedDirector()->getRunningScene();
    if (!scene) return;

    auto toast = ToastNode::create(category, oldValue, newValue, goalValue);
    if (!toast) return;

    m_currentToast = toast;
    m_animating = true;

    scene->addChild(toast, 200);
    toast->showFinalStateAndWait(remainingTime);
    log::info("[Toast] showToastResumed: {} shown at final state, remaining={:.1f}s", category->name, remainingTime);
}

void NotificationSystem::flushPendingToasts() {
    log::info("[Toast] flushPendingToasts: deferred={}, pending={}, animating={}", m_deferred.size(), m_pending.size(), m_animating);
    if (m_deferred.empty()) return;

    for (auto& toast : m_deferred) {
        m_pending.push_back(std::move(toast));
    }
    m_deferred.clear();

    // If nothing is currently showing, start processing
    if (!m_animating) {
        processQueue();
    }
}

void NotificationSystem::flushToScene(CCScene* scene) {
    log::info("[Toast] flushToScene: deferred={}, scene={}", m_deferred.size(), (void*)scene);
    if (m_deferred.empty() || !scene) return;

    m_flushScene = scene;
    flushPendingToasts();
    m_flushScene = nullptr;
}

bool NotificationSystem::detachForReparent(ToastData& outData) {
    auto* toast = m_currentToast;
    if (!toast) return false;

    log::info("[Toast] detachForReparent: extracting data and killing old actions");

    // Extract data FIRST — this is the only info we need.
    // No retain, no raw pointer storage, no use-after-free risk.
    outData.category = toast->getCategory();
    outData.oldValue = toast->getOldValue();
    outData.newValue = toast->getNewValue();
    outData.goalValue = toast->getGoalValue();

    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - toast->getCreationTime()).count();
    outData.remainingTime = (float)((double)toast->getDisplayDuration() - elapsed + 0.35);
    if (outData.remainingTime < 0.f) outData.remainingTime = 0.f;

    // Kill old actions to prevent onHidden() → onToastDone() from firing
    // after we've already set up a new toast on the new scene.
    toast->stopAllActions();

    // Clear tracking — the old node will be cleaned up by scene destruction
    m_currentToast = nullptr;
    m_animating = false;

    return true;
}

void NotificationSystem::onToastDone() {
    m_currentToast = nullptr;
    m_animating = false;

    // CRITICAL: Do NOT call processQueue() directly here.
    // onToastDone() is called from onHidden(), which fires from CCCallFunc
    // inside CCActionManager::update(). Calling processQueue() → showToast()
    // → runAction() would add a new action to the manager's hash table while
    // it's being iterated → corruption → staged crash on next scene transition.
    // Instead, schedule a one-shot callback for the next scheduler tick.
    if (!s_queueHelper) {
        s_queueHelper = new QueueProcessHelper();
    }
    auto* sched = CCDirector::sharedDirector()->getScheduler();
    if (sched) {
        sched->unscheduleSelector(
            schedule_selector(QueueProcessHelper::process), s_queueHelper);
        sched->scheduleSelector(
            schedule_selector(QueueProcessHelper::process), s_queueHelper, 0.f, false);
    }
}

void NotificationSystem::processQueue() {
    log::info("[Toast] processQueue: pending={}", m_pending.size());
    if (m_pending.empty()) return;

    auto [cat, oldVal, newVal, goalVal] = m_pending.front();
    m_pending.erase(m_pending.begin());

    showToast(cat, oldVal, newVal, goalVal);
}
