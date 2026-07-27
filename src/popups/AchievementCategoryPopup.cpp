#include "AchievementCategoryPopup.hpp"

#include "../AchievementMenu.hpp"
#include "../ProgressCalculator.hpp"
#include "../Utils.hpp"

using namespace geode::prelude;

void AchievementCategoryPopup::createTitle() {
    CCLabelBMFont* unlockTitle = CCLabelBMFont::create(m_category->name.c_str(), "goldFont.fnt");
    unlockTitle->setID("page-title");
    unlockTitle->setPosition({m_mainLayer->getContentWidth() / 2, 260});
    unlockTitle->setScale(1.0f);
    m_mainLayer->addChild(unlockTitle);
}

void AchievementCategoryPopup::addProgressText(int statValue, int goalValue) {
    CCLabelBMFont* numCompleteLabel = nullptr;

    if (statValue < goalValue) {
        numCompleteLabel = CCLabelBMFont::create(formatWithCommas(statValue).c_str(), "bigFont.fnt");
        numCompleteLabel->setScale(0.78f);
        numCompleteLabel->setPosition({-2, -0.25f});
    } else {
        numCompleteLabel = CCLabelBMFont::create(formatWithCommas(statValue).c_str(), "goldFont.fnt");
        numCompleteLabel->setPosition({-2, 0});
    }

    std::string goalText = formatWithCommas(goalValue);
    auto goalLabel = CCLabelBMFont::create(("/" + goalText).c_str(), "goldFont.fnt");
    goalLabel->setAnchorPoint({0, 0.5f});
    goalLabel->setPosition({-3, 0});
    numCompleteLabel->setAnchorPoint({1, 0.5f});

    auto container = CCNode::create();
    container->setID("fraction-complete");
    container->addChild(numCompleteLabel);
    container->addChild(goalLabel);

    container->setPosition({225, 237});
    container->setScale(0.5f);
    m_mainLayer->addChild(container);
}

void AchievementCategoryPopup::addLogo() {
    const std::vector<std::string> fromSpritesheet = {"Stars", "Moons", "Diamonds", "Secret Coins", "User Coins", "Creator"};

    CCSprite* logo;
    if (std::find(fromSpritesheet.begin(), fromSpritesheet.end(), m_category->name) != fromSpritesheet.end())
        logo = CCSprite::createWithSpriteFrameName(m_category->logo.c_str());  // try from spritesheet
    else
        logo = CCSprite::create(m_category->logo.c_str());  // otherwise try mod resources from logos/

    if (!logo) {
        log::error("Failed to load logo for category: {}", m_category->name);
    } else {
        logo->setID("logo");
        logo->setAnchorPoint({0.5f, 0.5f});
        logo->setScale(std::min(20.f / logo->getContentWidth(), 20.f / logo->getContentHeight()));
        logo->setPosition({225.f, 215.f});
        logo->setZOrder(1);
        m_mainLayer->addChild(logo);

        if (m_category->name == "Jumps") {
            auto* jumpingIcon = createJumpsIcon();
            jumpingIcon->setPosition({40, 60});
            logo->addChild(jumpingIcon);
        }
    }
}

void AchievementCategoryPopup::addCornerSprites() {
    ::addCornerSprites(m_mainLayer);
}

void AchievementCategoryPopup::addNavigation() {
    if (m_numPages <= 1) return;

    m_navMenu = CCMenu::create();
    m_navMenu->setID("nav-menu");
    m_navMenu->setContentSize(m_mainLayer->getContentSize());
    m_navMenu->setPosition({0.f, 0.f});

    m_navButtons = CCMenu::create();
    m_navButtons->setPosition({m_navMenu->getContentWidth() / 2, 20.f});
    m_navButtons->setContentSize({400.f, 20.f});
    m_navButtons->setID("nav-buttons");
    m_navButtons->setLayout(RowLayout::create()
                                ->setGap(10.0f)
                                ->setAutoScale(false)
                                ->setAxisAlignment(AxisAlignment::Center));
    m_navMenu->addChild(m_navButtons);
    m_mainLayer->addChild(m_navMenu);

    // navigation buttons
    for (int i = 0; i < m_numPages; i++) {
        CCMenuItemSpriteExtra* button = CCMenuItemSpriteExtra::create(
            i == 0 ? CCSprite::createWithSpriteFrameName("gj_navDotBtn_on_001.png") : CCSprite::createWithSpriteFrameName("gj_navDotBtn_off_001.png"),
            this,
            menu_selector(AchievementCategoryPopup::onNavButton));
        button->setID("page-button-" + std::to_string(i));
        button->setTag(i);
        button->setScale(0.8f);
        button->m_baseScale = 0.8f;
        m_navButtons->addChild(button);
    }
    m_navButtons->updateLayout();

    // navigation arrows
    CCSprite* leftArrowSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    CCMenuItemSpriteExtra* leftArrow = CCMenuItemSpriteExtra::create(
        leftArrowSprite,
        this,
        menu_selector(AchievementCategoryPopup::onArrow));
    if (m_mainLayer->getPositionX() - m_mainLayer->getContentWidth() / 2 > 40)
        leftArrow->setPosition({-30.f, m_navMenu->getContentHeight() / 2});
    else {
        leftArrow->setAnchorPoint({0, 0.5f});
        leftArrow->setPosition({-(getContentWidth() - m_navMenu->getContentWidth()) / 2, m_navMenu->getContentHeight() / 2});
    }
    leftArrow->setTag(0);
    leftArrow->setID("left-arrow");
    m_navMenu->addChild(leftArrow);
    leftArrow->setVisible(false);

    CCSprite* rightArrowSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    rightArrowSprite->setFlipX(true);
    CCMenuItemSpriteExtra* rightArrow = CCMenuItemSpriteExtra::create(
        rightArrowSprite,
        this,
        menu_selector(AchievementCategoryPopup::onArrow));
    if (m_mainLayer->getPositionX() - m_mainLayer->getContentWidth() / 2 > 40)
        rightArrow->setPosition({m_navMenu->getContentWidth() + 30.f, m_navMenu->getContentHeight() / 2});
    else {
        rightArrow->setAnchorPoint({1, 0.5f});
        rightArrow->setPosition({m_navMenu->getContentWidth() + (getContentWidth() - m_navMenu->getContentWidth()) / 2, m_navMenu->getContentHeight() / 2});
    }
    rightArrow->setTag(1);
    rightArrow->setID("right-arrow");
    m_navMenu->addChild(rightArrow);
}

void AchievementCategoryPopup::onIcon(CCObject* sender) {
    CCMenuItemSpriteExtra* button = static_cast<CCMenuItemSpriteExtra*>(sender);
    IconCallbackData* data = dynamic_cast<IconCallbackData*>(button->getUserObject());

    ItemInfoPopup* popup = ItemInfoPopup::create(data->unlockID, data->unlockType);
    if (!popup) return;

    popup->show();

    std::vector<UnlockType> playerUnlockTypes = {UnlockType::Cube, UnlockType::Ship, UnlockType::Ball, UnlockType::Bird, UnlockType::Dart, UnlockType::Robot, UnlockType::Spider, UnlockType::Swing};

    if (std::find(playerUnlockTypes.begin(), playerUnlockTypes.end(), data->unlockType) == playerUnlockTypes.end()) return;  // is not a player icon

    if (!Mod::get()->getSettingValue<bool>("use-player-colors")) return;  // use player colors setting is off

    auto mainLayer = dynamic_cast<CCLayer*>(popup->getChildByID("main-layer"));
    if (!mainLayer) return;

    auto icon = dynamic_cast<GJItemIcon*>(mainLayer->getChildByID("item-icon"));
    if (!icon) return;

    auto children = icon->getChildren();
    for (int i = 0; i < children->count(); i++) {
        auto simplePlayer = dynamic_cast<SimplePlayer*>(children->objectAtIndex(i));

        simplePlayer->setColors(
            gameManager->colorForIdx(gameManager->getPlayerColor()),
            gameManager->colorForIdx(gameManager->getPlayerColor2()));

        if (gameManager->m_playerGlow) {
            simplePlayer->setGlowOutline(
                gameManager->colorForIdx(gameManager->getPlayerGlowColor()));
        }
    }
}

void AchievementCategoryPopup::onNavButton(CCObject* sender) {
    CCMenuItemSpriteExtra* button = static_cast<CCMenuItemSpriteExtra*>(sender);
    int pageNum = button->getTag();

    // update page visibility
    for (int i = 0; i < m_numPages; i++) {
        CCNode* page = m_mainLayer->getChildByID("page-" + std::to_string(i));
        if (page) {
            page->setVisible(i == pageNum);
        }
    }

    // update navigation buttons
    for (int i = 0; i < m_numPages; i++) {
        CCMenuItemSpriteExtra* navButton = static_cast<CCMenuItemSpriteExtra*>(m_navButtons->getChildByID("page-button-" + std::to_string(i)));
        if (navButton) {
            navButton->setNormalImage(i == pageNum ? CCSprite::createWithSpriteFrameName("gj_navDotBtn_on_001.png") : CCSprite::createWithSpriteFrameName("gj_navDotBtn_off_001.png"));
        }
    }

    // update arrow visibility
    m_navMenu->getChildByID("left-arrow")->setVisible(pageNum > 0);
    m_navMenu->getChildByID("right-arrow")->setVisible(pageNum < m_numPages - 1);
}

void AchievementCategoryPopup::onArrow(CCObject* sender) {
    CCMenuItemSpriteExtra* button = static_cast<CCMenuItemSpriteExtra*>(sender);
    int direction = button->getTag();

    int currentPage = -1;
    for (int i = 0; i < m_numPages; i++) {
        if (m_mainLayer->getChildByID("page-" + std::to_string(i))->isVisible()) {
            currentPage = i;
            break;
        }
    }

    int newPage = currentPage + (direction == 0 ? -1 : 1);
    if (newPage < 0 || newPage >= m_numPages) return;

    // update page visibility
    for (int i = 0; i < m_numPages; i++) {
        CCNode* page = m_mainLayer->getChildByID("page-" + std::to_string(i));
        if (page) {
            page->setVisible(i == newPage);
        }
    }

    // update navigation buttons
    for (int i = 0; i < m_numPages; i++) {
        CCMenuItemSpriteExtra* navButton = static_cast<CCMenuItemSpriteExtra*>(m_navButtons->getChildByID("page-button-" + std::to_string(i)));
        if (navButton) {
            navButton->setNormalImage(i == newPage ? CCSprite::createWithSpriteFrameName("gj_navDotBtn_on_001.png") : CCSprite::createWithSpriteFrameName("gj_navDotBtn_off_001.png"));
        }
    }

    // update arrow visibility
    m_navMenu->getChildByID("left-arrow")->setVisible(newPage > 0);
    m_navMenu->getChildByID("right-arrow")->setVisible(newPage < m_numPages - 1);
}

void AchievementCategoryPopup::onClose(CCObject* sender) {
    Popup::onClose(sender);

    // Only call showArrows if the achievement menu is still a valid node in the scene.
    // If another AchievementMenu was opened on top (by pressing comma again),
    // m_achievementMenu will be a dangling pointer and must not be touched.
    if (m_achievementMenu && m_achievementMenu->getParent()) {
        m_achievementMenu->showArrows();
    }
}

void AchievementCategoryPopup::addTrackingRow() {
    // ── Tracking indicator: "Tracking" label + checkbox, top-right corner ──
    auto trackingNode = CCMenu::create();
    trackingNode->setID("tracking-node");
    trackingNode->setPosition({380.f, 255.f}); // top-right, inside popup bounds
    m_mainLayer->addChild(trackingNode);

    // "Tracking" label
    auto label = CCLabelBMFont::create("Tracking", "bigFont.fnt");
    label->setID("tracking-label");
    label->setAnchorPoint({1.f, 0.5f});
    label->setScale(0.30f);
    label->setPosition({-13.f, 0.f});
    trackingNode->addChild(label);

    // Checkbox sprite (clickable)
    auto checkOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    auto checkOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");

    auto toggleBtn = CCMenuItemToggler::create(
        checkOff, checkOn,
        this,
        menu_selector(AchievementCategoryPopup::onToggleCategoryTracked));
    toggleBtn->setID("tracking-toggle-btn");
    toggleBtn->setPosition({0.f, 0.f});
    toggleBtn->setScale(0.65f);
    trackingNode->addChild(toggleBtn);

    // Initialize state from GameManager (single source of truth)
    bool tracked = isCategoryTracked(m_category->name);
    int completedCount = computeCategoryProgress(
        m_category->displayType, m_category->achievements, m_category->statKey,
        m_category->achievements.empty() ? 0 : m_category->achievements.back()->unlockValue);
    if (completedCount == m_numAchievements) {
        // Completed: hide toggle + label, show "¡Completed!" in gold instead
        toggleBtn->setVisible(false);
        label->setVisible(false);

        auto completedLabel = CCLabelBMFont::create("¡Completed!", "goldFont.fnt");
        completedLabel->setID("tracking-completed-label");
        completedLabel->setAnchorPoint({1.f, 0.5f});
        completedLabel->setScale(0.35f);
        completedLabel->setColor({255, 204, 0});
        completedLabel->setPosition({0.f, 0.f});
        trackingNode->addChild(completedLabel);
    } else {
        toggleBtn->toggle(tracked);
    }
}

void AchievementCategoryPopup::onToggleCategoryTracked(CCObject* sender) {
    if (!m_category) return;

    // Toggle via GameManager (single source of truth)
    bool tracked = !isCategoryTracked(m_category->name);
    setCategoryTracked(m_category->name, tracked);

    // If completed, force toggle off and show completed icon instead
    int completedCount = computeCategoryProgress(
        m_category->displayType, m_category->achievements, m_category->statKey,
        m_category->achievements.empty() ? 0 : m_category->achievements.back()->unlockValue);

    if (completedCount == m_numAchievements) {
        // Completed: hide toggle + label, show "¡Completed!" in gold
        auto trackingNode = m_mainLayer->getChildByID("tracking-node");
        if (trackingNode) {
            auto toggleBtn = typeinfo_cast<CCMenuItemToggler*>(trackingNode->getChildByID("tracking-toggle-btn"));
            if (toggleBtn) toggleBtn->setVisible(false);
            auto trackingLabel = trackingNode->getChildByID("tracking-label");
            if (trackingLabel) trackingLabel->setVisible(false);
            // Only add if not already present
            if (!trackingNode->getChildByID("tracking-completed-label")) {
                auto completedLabel = CCLabelBMFont::create("¡Completed!", "goldFont.fnt");
                completedLabel->setID("tracking-completed-label");
                completedLabel->setAnchorPoint({1.f, 0.5f});
                completedLabel->setScale(0.35f);
                completedLabel->setColor({255, 204, 0});
                completedLabel->setPosition({0.f, 0.f});
                trackingNode->addChild(completedLabel);
            }
        }
    }

    // Notify AchievementMenu to refresh the tracked page
    if (m_achievementMenu) {
        m_achievementMenu->refreshTrackingPage();
    }
}
