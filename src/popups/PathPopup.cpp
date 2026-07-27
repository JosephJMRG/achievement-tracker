#include "PathPopup.hpp"

using namespace geode::prelude;

PathPopup* PathPopup::create(AchievementMenu* achievementMenu, Category* category) {
    auto popup = new PathPopup();
    if (popup && popup->init(achievementMenu, category)) {
        popup->autorelease();
        return popup;
    }
    delete popup;
    return nullptr;
}

bool PathPopup::init(AchievementMenu* achievementMenu, Category* category) {
    if (!Popup::init(450.f, 280.f))
        return false;

    m_achievementMenu = achievementMenu;
    m_category = category;
    m_numAchievements = m_category->achievements.size();

    addCornerSprites();

    // create pages
    auto SFC = CCSpriteFrameCache::get();
    SFC->addSpriteFramesWithFile("GJ_PathSheet.plist");

    m_maxIconsPerPage = 11;
    m_numPages = (m_numAchievements + m_maxIconsPerPage - 1) / m_maxIconsPerPage;
    for (int i = 0; i < m_numPages; i++) {
        CCNode* page = createPage(i);
        page->setID("page-" + std::to_string(i));
        page->setTag(i);
        page->retain();

        m_mainLayer->addChild(page);
        page->setVisible(i == 0);
    }

    addTrackingRow();

    addNavigation(0);

    return true;
}

void PathPopup::addNavigation(int activePage) {
    bool refresh = false;
    if (m_navButtons) {
        m_navButtons->removeAllChildren();
        m_navButtons->removeFromParent();
        refresh = true;
    }

    if (!refresh) {
        m_navMenu = CCMenu::create();
        m_navMenu->setID("nav-menu");
        m_navMenu->setContentSize(m_mainLayer->getContentSize());
        m_navMenu->setPosition({0.f, 0.f});
        m_mainLayer->addChild(m_navMenu);
    }

    m_navButtons = CCMenu::create();
    m_navButtons->setPosition({m_navMenu->getContentWidth() / 2, 20.f});
    m_navButtons->setContentSize({400.f, 20.f});
    m_navButtons->setID("nav-buttons");
    m_navButtons->setLayout(RowLayout::create()
                                ->setGap(0.f)
                                ->setAutoScale(false)
                                ->setAxisAlignment(AxisAlignment::Center));
    m_navMenu->addChild(m_navButtons);

    // navigation buttons
    for (int i = 0; i < m_numPages; i++) {
        CCSprite* onSprite = CCSprite::createWithSpriteFrameName(fmt::format("pathIcon_{:02}_001.png", i + 1).c_str());
        onSprite->setScale(0.75f);
        CCSprite* offSprite = CCSprite::createWithSpriteFrameName(fmt::format("pathIcon_{:02}_001.png", i + 1).c_str());
        offSprite->setScale(0.5f);

        bool showCheckmark = gameStatsManager->getStat(std::to_string(i + 30).c_str()) >= 1000;
        if (showCheckmark) {
            CCSprite* checkmark = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            checkmark->setID("checkmark");
            checkmark->setZOrder(1);

            if (i == activePage) {
                checkmark->setPosition({onSprite->getContentWidth() / 2, onSprite->getContentHeight() / 2});
                onSprite->addChild(checkmark);
            } else {
                checkmark->setPosition({offSprite->getContentWidth() / 2, offSprite->getContentHeight() / 2});
                offSprite->addChild(checkmark);
            }
        } else {
            CCLabelBMFont* rankLabel = CCLabelBMFont::create(std::to_string(gameStatsManager->getStat(std::to_string(i + 30).c_str()) / 100).c_str(), "bigFont.fnt");
            rankLabel->setID("rank-label");
            rankLabel->setScale(0.5f);
            if (i == activePage) {
                rankLabel->setPosition({onSprite->getContentWidth() / 2 + 0.5f, onSprite->getContentHeight() / 2 + 1});
                onSprite->addChild(rankLabel);
            } else {
                rankLabel->setPosition({offSprite->getContentWidth() / 2 + 0.5f, offSprite->getContentHeight() / 2 + 1});
                offSprite->addChild(rankLabel);
            }
        }

        CCMenuItemSpriteExtra* button = CCMenuItemSpriteExtra::create(
            i == activePage ? onSprite : offSprite,
            this,
            menu_selector(PathPopup::onNavButton));
        button->setID("page-button-" + std::to_string(i));
        button->setTag(i);
        button->setScale(0.8f);
        button->m_baseScale = 0.8f;
        button->setContentWidth(40.f);
        if (auto sprite = typeinfo_cast<CCSprite*>(button->getChildren()->objectAtIndex(0)))
            sprite->setPositionX(20.f);

        m_navButtons->addChild(button);
    }
    m_navButtons->updateLayout();

    if (refresh) return;

    // navigation arrows
    CCSprite* leftArrowSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    CCMenuItemSpriteExtra* leftArrow = CCMenuItemSpriteExtra::create(
        leftArrowSprite,
        this,
        menu_selector(PathPopup::onArrow));
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
        menu_selector(PathPopup::onArrow));
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

void PathPopup::onNavButton(CCObject* sender) {
    CCMenuItemSpriteExtra* button = static_cast<CCMenuItemSpriteExtra*>(sender);
    int pageNum = button->getTag();

    // update page visibility
    for (int i = 0; i < m_numPages; i++) {
        CCNode* page = m_mainLayer->getChildByID("page-" + std::to_string(i));
        if (page) {
            page->setVisible(i == pageNum);
        }
    }

    addNavigation(pageNum);

    // update arrow visibility
    m_navMenu->getChildByID("left-arrow")->setVisible(pageNum > 0);
    m_navMenu->getChildByID("right-arrow")->setVisible(pageNum < m_numPages - 1);
}

void PathPopup::onArrow(CCObject* sender) {
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

    addNavigation(newPage);

    // update arrow visibility
    m_navMenu->getChildByID("left-arrow")->setVisible(newPage > 0);
    m_navMenu->getChildByID("right-arrow")->setVisible(newPage < m_numPages - 1);
}

cocos2d::CCNode* PathPopup::createPage(int pageNum) {
    auto page = CCNode::create();
    page->setPosition({0, 0});

    /* Path Logo*/
    CCSprite* pathLogo = CCSprite::createWithSpriteFrameName(fmt::format("pathLabel_{:02}_001.png", pageNum + 1).c_str());
    if (!pathLogo) {
        log::error("Failed to load path logo for page {}", pageNum + 1);
    } else {
        pathLogo->setID("path-logo");
        pathLogo->setPosition({225, 260});
        page->addChild(pathLogo);
    }

    /* Progress Fraction */
    CCNode* container = CCNode::create();
    container->setID("fraction-complete");
    container->setAnchorPoint({0.5f, 0.5f});
    container->setContentSize({100, 20});

    int progress = std::min(1000, gameStatsManager->getStat(std::to_string(pageNum + 30).c_str()));

    CCLabelBMFont* progressLabel = nullptr;
    if (progress < 1000) {
        progressLabel = CCLabelBMFont::create(formatWithCommas(progress).c_str(), "bigFont.fnt");
        progressLabel->setScale(0.39f);
        progressLabel->setPosition({container->getContentWidth() / 2 - 2, container->getContentHeight() / 2 - 0.25f});
    } else {
        progressLabel = CCLabelBMFont::create(formatWithCommas(progress).c_str(), "goldFont.fnt");
        progressLabel->setScale(0.5f);
        progressLabel->setPosition({container->getContentWidth() / 2 - 2, container->getContentHeight() / 2});
    }

    CCLabelBMFont* goalLabel = CCLabelBMFont::create("/1,000", "goldFont.fnt");
    goalLabel->setScale(0.5f);
    goalLabel->setAnchorPoint({0, 0.5f});
    goalLabel->setPosition({container->getContentWidth() / 2 - 3, container->getContentHeight() / 2});
    progressLabel->setAnchorPoint({1, 0.5f});

    CCSprite* star = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
    star->setID("star");
    star->setScale(0.5f);
    star->setPosition({container->getContentWidth() / 2 + 44, container->getContentHeight() / 2 - 1});

    CCSprite* moon = CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png");
    moon->setID("moon");
    moon->setPosition({container->getContentWidth() / 2 + 56, container->getContentHeight() / 2 - 1});
    moon->setScale(0.5f);

    container->addChild(progressLabel);
    container->addChild(goalLabel);
    container->addChild(star);
    container->addChild(moon);
    container->setPosition({225, 240});
    page->addChild(container);

    /* Rank Logo */
    CCSprite* rankLogo = CCSprite::createWithSpriteFrameName(fmt::format("pathIcon_{:02}_001.png", pageNum + 1).c_str());
    rankLogo->setID("rank-logo");
    rankLogo->setScale(0.8f);
    rankLogo->setPosition({225, 205});

    CCLabelBMFont* rankLabel = CCLabelBMFont::create(std::to_string(gameStatsManager->getStat(std::to_string(pageNum + 30).c_str()) / 100).c_str(), "bigFont.fnt");
    rankLabel->setID("rank-label");
    rankLabel->setScale(0.5f);
    rankLabel->setPosition({rankLogo->getContentWidth() / 2 + 0.5f, rankLogo->getContentHeight() / 2 + 1});
    rankLogo->addChild(rankLabel);

    page->addChild(rankLogo);

    /* Progress Bar */
    CCNode* progressBar = CCNode::create();
    progressBar->setID("progress-bar-container");
    progressBar->setPosition({m_mainLayer->getContentWidth() / 2, 120.f});
    page->addChild(progressBar);

    int numIconsOnPage = 11;
    int numDotsOnPage = numIconsOnPage;
    float dotSpacing = std::min(50.f, 400.f / numDotsOnPage);
    progressBar->addChild(buildProgressBarBg({
        numDotsOnPage, numIconsOnPage, dotSpacing,
        dotSpacing / 2.f,   // PathPopup offsets dots by half spacing
        false,              // PathPopup draws vertical bars for ALL dots
        true                // PathPopup inverts vertical bar alternation
    }));

    // Fill (player color or white)
    bool usePlayerColors = Mod::get()->getSettingValue<bool>("use-player-colors");

    CCNode* progressBarFill = CCNode::create();
    progressBarFill->setID("progress-bar-fill");
    progressBarFill->setPosition({0, 0});
    progressBar->addChild(progressBarFill);

    int numAchievementsUnlocked = 0;
    for (int i = m_maxIconsPerPage * pageNum; i < m_maxIconsPerPage * (pageNum + 1); ++i) {
        if (achievementManager->isAchievementEarned(m_category->achievements[i]->id.c_str())) {
            numAchievementsUnlocked++;
        }
    }

    for (int i = 0; i < numAchievementsUnlocked; ++i) {
        CCSprite* dotFillSpr = CCSprite::create("smallDot.png");  // the colored dots that mark the reached unlock points
        dotFillSpr->setID("dot-fill-sprite-" + std::to_string(i));
        dotFillSpr->setPosition({-dotSpacing * numIconsOnPage / 2.f + dotSpacing * i + dotSpacing / 2, 0});
        dotFillSpr->setColor(usePlayerColors ? gameManager->colorForIdx(gameManager->getPlayerColor()) : ccc3(255, 255, 255));
        dotFillSpr->setScale(0.7f);
        progressBarFill->addChild(dotFillSpr);
    }

    float ratio = std::min(1.f, float(gameStatsManager->getStat(std::to_string(pageNum + 30).c_str())) / 1000);

    CCSprite* fillSpr = CCSprite::createWithSpriteFrameName("whiteSquare20_001.png");  // the colored bar that fills up to the next unlock point and marks the progress
    fillSpr->setID("progress-bar-fill-sprite");
    fillSpr->setAnchorPoint({0, 0.5f});
    fillSpr->setPosition({-dotSpacing * numIconsOnPage / 2.f + dotSpacing / 2, 0});
    fillSpr->setColor(usePlayerColors ? gameManager->colorForIdx(gameManager->getPlayerColor()) : ccc3(255, 255, 255));
    fillSpr->setScaleX(dotSpacing * ratio);
    fillSpr->setScaleY(0.2f);
    progressBarFill->addChild(fillSpr);

    /* Player Sprites */
    CCMenu* playerMenu = CCMenu::create();
    playerMenu->setID("unlocks-container");
    playerMenu->setPosition(progressBar->getPosition());
    page->addChild(playerMenu);

    for (int i = 0; i < numIconsOnPage; ++i) {
        Achievement* currAchievement = m_category->achievements[i + pageNum * m_maxIconsPerPage];
        bool earned = achievementManager->isAchievementEarned(currAchievement->id.c_str());

        CCMenuItemSpriteExtra* unlockButton = createAchievementIconButton(
            currAchievement, earned, usePlayerColors, this, menu_selector(PathPopup::onIcon), std::to_string(i));

        float baseX = -dotSpacing * numIconsOnPage / 2.f + dotSpacing * i + dotSpacing / 2;
        float baseY = (i % 2 == 0) ? 40.f : -40.f;
        unlockButton->setPosition({baseX, baseY});
        playerMenu->addChild(unlockButton);

        // Create the text that shows how much is needed to unlock
        CCLabelBMFont* unlockValue = CCLabelBMFont::create(formatWithCommas(i * 100).c_str(), "bigFont.fnt");
        unlockValue->setID("unlock-text-" + std::to_string(i));
        unlockValue->setScale(.25f);
        float textYOffset = (i % 2 == 0) ? 18.f : -16.f;
        unlockValue->setPosition({baseX, baseY + textYOffset});
        playerMenu->addChild(unlockValue);
    }

    return page;
}