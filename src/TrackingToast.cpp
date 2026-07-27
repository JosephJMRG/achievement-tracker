#include "TrackingToast.hpp"
#include <Geode/ui/OverlayManager.hpp>

using namespace geode::prelude;

// ─── Queue process helper (must NOT run inside CCActionManager::update) ─────
class ToastQueueHelper : public CCObject {
public:
    void process(float) {
        CCDirector::sharedDirector()->getScheduler()->unscheduleSelector(
            schedule_selector(ToastQueueHelper::process), this);
        TrackingToast::get()->processNext();
    }
};
static ToastQueueHelper* s_queueHelper = nullptr;

// ─── ToastNode: full visual tracking card ──────────────────────────────────

class TrackingToast::ToastNode : public CCNode {
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
        m_counterElapsed = 0.f;
        m_counterDuration = 0.8f;
        schedule(schedule_selector(ToastNode::tickCounter), 0.f);
    }

    void tickCounter(float dt) {
        m_counterElapsed += dt;
        float t = std::min(1.f, m_counterElapsed / m_counterDuration);
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
        // Do NOT removeFromParent here — called from CCCallFunc inside action manager
        setVisible(false);
        TrackingToast::get()->onToastDone();
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

private:
    bool init(Category* category, int oldValue, int newValue, int goalValue) {
        if (!CCNode::init()) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        float cardW = 220.f;
        float cardH = 56.f;
        m_cardW = cardW;
        m_targetX = winSize.width - cardW - 10.f;

        setAnchorPoint({0.f, 0.f});
        setPosition({m_targetX, 30.f});
        setContentSize({cardW, cardH});

        float x = 0.f;
        float baseY = 0.f;

        // Layer 1: White background
        auto layerWhite = CCScale9Sprite::create("square02b_001.png");
        layerWhite->setContentSize({cardW + 6.f, cardH + 6.f});
        layerWhite->setAnchorPoint({0.f, 0.f});
        layerWhite->setPosition({x - 2.f, baseY - 3.f});
        layerWhite->setColor({255, 255, 255});
        addChild(layerWhite);

        // Layer 2: Dark blue shadow
        auto layerDark = CCScale9Sprite::create("square02b_001.png");
        layerDark->setContentSize({cardW + 4.f, cardH + 4.f});
        layerDark->setAnchorPoint({0.f, 0.f});
        layerDark->setPosition({x - 1.f, baseY - 2.f});
        layerDark->setColor({20, 30, 100});
        addChild(layerDark);

        // Layer 3: Light blue interior
        auto layerLight = CCScale9Sprite::create("square02b_001.png");
        layerLight->setContentSize({cardW + 2.f, cardH + 2.f});
        layerLight->setAnchorPoint({0.f, 0.f});
        layerLight->setPosition({x, baseY - 1.f});
        layerLight->setColor({45, 59, 142});
        addChild(layerLight);

        // === LEFT SIDE ===
        float lx = x + 12.f;
        float offSetY = 5.f;

        // Title
        auto titleLabel = CCLabelBMFont::create(category->name.c_str(), "goldFont.fnt");
        titleLabel->setAnchorPoint({0.f, 0.5f});
        titleLabel->setScale(0.65f);
        titleLabel->limitLabelWidth(cardW * 0.55f, 0.65f, 0.1f);
        titleLabel->setPosition({lx, baseY - offSetY + cardH - 12.f});
        addChild(titleLabel);

        m_oldValue = oldValue;
        m_newValue = newValue;
        m_goalValue = goalValue;

        // Stat label (animates from old to new)
        m_statLabel = CCLabelBMFont::create("", "bigFont.fnt");
        m_statLabel->setAnchorPoint({0.f, 0.5f});
        m_statLabel->setScale(0.47f);
        m_statLabel->limitLabelWidth(cardW * 0.55f, 0.47f, 0.1f);
        m_statLabel->setColor({255, 204, 0});
        m_statLabel->setPosition({lx, baseY - offSetY + cardH - 27.f});
        addChild(m_statLabel);
        updateStatLabel(oldValue);

        // Progress bar
        float barX = lx;
        float barY = baseY + 10.f;
        float barW = cardW * 0.70f - 20.f;
        float barH = 13.f;
        float ins = 2.f;
        float offSetYBar = 5.f;

        auto barBg = CCLayerColor::create({26, 35, 126, 255}, barW, barH);
        barBg->setAnchorPoint({0.f, 0.5f});
        barBg->setPosition({barX, barY - offSetYBar});
        addChild(barBg, 10);

        float oldPct = goalValue > 0 ? std::min(1.f, (float)oldValue / (float)goalValue) : 0.f;
        float newPct = goalValue > 0 ? std::min(1.f, (float)newValue / (float)goalValue) : 0.f;
        float oldFillW = (barW - ins * 2.f) * oldPct;
        float newFillW = (barW - ins * 2.f) * newPct;

        if (newFillW > 0.5f) {
            float fillH = barH - ins * 2.f;
            auto barFill = CCLayerColor::create({57, 255, 20, 255}, std::max(0.5f, oldFillW), fillH);
            barFill->setAnchorPoint({0.f, 0.5f});
            barFill->setPosition({barX + ins, barY + 2.f - offSetYBar});
            addChild(barFill, 10);

            if (newFillW > oldFillW + 0.5f) {
                float ratio = newFillW / std::max(0.5f, oldFillW);
                barFill->runAction(CCScaleTo::create(0.8f, ratio, 1.f));
            }
        }

        // === RIGHT SIDE ===
        float toggleX = x + cardW - 8.f;
        float textRx = toggleX - 3.f;
        float pctY = baseY + cardH - 30.f;

        // Percentage
        int oldPctInt = goalValue > 0 ? std::min(100, (int)((float)oldValue / (float)goalValue * 100.f)) : 0;
        m_pctLabel = CCLabelBMFont::create((std::to_string(oldPctInt) + "%").c_str(), "goldFont.fnt");
        m_pctLabel->setAnchorPoint({1.f, 0.5f});
        m_pctLabel->setScale(0.65f);
        m_pctLabel->setColor({255, 204, 0});
        m_pctLabel->setPosition({textRx, pctY});
        addChild(m_pctLabel);

        // "Completed" label
        auto compLabel = CCLabelBMFont::create("Completed", "bigFont.fnt");
        compLabel->setAnchorPoint({1.f, 0.5f});
        compLabel->setScale(0.25f);
        compLabel->setColor({255, 204, 0});
        compLabel->setPosition({textRx, pctY - 12.f});
        addChild(compLabel);

        // Close button
        auto closeSpr = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        closeSpr->setScale(0.50f);
        auto closeBtn = CCMenuItemSpriteExtra::create(
            closeSpr, this, menu_selector(ToastNode::onClose));
        auto closeMenu = CCMenu::create();
        closeMenu->setPosition({toggleX, baseY + cardH - 9.f});
        closeMenu->addChild(closeBtn);
        addChild(closeMenu);

        return true;
    }

    float m_cardW = 0.f;
    float m_targetX = 0.f;
    float m_displayDuration = 4.0f;

    CCLabelBMFont* m_statLabel = nullptr;
    CCLabelBMFont* m_pctLabel = nullptr;
    int m_oldValue = 0;
    int m_newValue = 0;
    int m_goalValue = 0;
    float m_counterElapsed = 0.f;
    float m_counterDuration = 0.8f;
};

// ─── TrackingToast ─────────────────────────────────────────────────────────

static TrackingToast* s_instance = nullptr;

TrackingToast* TrackingToast::get() {
    if (!s_instance) s_instance = new TrackingToast();
    return s_instance;
}

void TrackingToast::showToast(Category* category, int oldValue, int newValue, int goalValue) {
    if (m_currentToast && !m_currentToast->getParent()) {
        m_currentToast = nullptr;
        m_animating = false;
    }

    if (m_animating) {
        m_pending.emplace_back(category, oldValue, newValue, goalValue);
        return;
    }

    auto toast = ToastNode::create(category, oldValue, newValue, goalValue);
    if (!toast) return;

    m_currentToast = toast;
    m_animating = true;
    // Add to overlay (persists across scene transitions, just like geode::Notification)
    OverlayManager::get()->addChild(toast, 200);
    toast->startAnim(0.35f, 4.0f, 0.35f);
}

void TrackingToast::dismissCurrent() {
    if (m_currentToast && m_currentToast->getParent()) {
        m_currentToast->onClose(nullptr);
    } else {
        m_currentToast = nullptr;
        m_animating = false;
    }
}

void TrackingToast::clearAll() {
    if (m_currentToast) {
        m_currentToast->stopAllActions();
        if (m_currentToast->getParent())
            m_currentToast->removeFromParent();
        m_currentToast = nullptr;
    }
    m_pending.clear();
    m_animating = false;
}

void TrackingToast::onToastDone() {
    m_currentToast = nullptr;
    m_animating = false;

    // Schedule queue processing for next tick (avoid action manager reentrance)
    if (!s_queueHelper) s_queueHelper = new ToastQueueHelper();
    auto* sched = CCDirector::sharedDirector()->getScheduler();
    if (sched) {
        sched->unscheduleSelector(schedule_selector(ToastQueueHelper::process), s_queueHelper);
        sched->scheduleSelector(schedule_selector(ToastQueueHelper::process), s_queueHelper, 0.f, false);
    }
}

void TrackingToast::processNext() {
    if (m_pending.empty()) return;

    auto [cat, oldVal, newVal, goalVal] = m_pending.front();
    m_pending.erase(m_pending.begin());
    showToast(cat, oldVal, newVal, goalVal);
}
