#include "ShardPopup.hpp"

using namespace geode::prelude;

ShardPopup* ShardPopup::create(AchievementMenu* achievementMenu, Category* category) {
    auto popup = new ShardPopup();
    if (popup && popup->init(achievementMenu, category)) {
        popup->autorelease();
        return popup;
    }
    delete popup;
    return nullptr;
}

bool ShardPopup::init(AchievementMenu* achievementMenu, Category* category) {
    if (!Popup::init(450.f, 280.f))
        return false;

    m_achievementMenu = achievementMenu;
    m_category = category;
    m_numAchievements = m_category->achievements.size();

    addCornerSprites();

    // create pages
    m_maxIconsPerPage = 5;
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

void ShardPopup::addNavigation(int activePage) {
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
        CCSprite* onSprite = CCSprite::createWithSpriteFrameName(shardSprites[i]);
        onSprite->setScale(i == 5 || i == 11 ? 1.5f : 0.75f);
        CCSprite* offSprite = CCSprite::createWithSpriteFrameName(shardSprites[i]);
        offSprite->setScale(i == 5 || i == 11 ? 1.f : 0.5f);

        int gameStatID = gameStatIDs[i];
        int progress = 100;
        if (i == 5 || i == 11) {  // bonus pages, these values aren't stored so need to calculate them
            for (int j = i - 5; j < i; ++j) {
                progress = std::min(progress, gameStatsManager->getStat(std::to_string(gameStatIDs[j]).c_str()));
            }
        } else
            progress = gameStatsManager->getStat(std::to_string(gameStatID).c_str());

        if (progress >= 100) {
            CCSprite* checkmark = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
            checkmark->setID("checkmark");
            checkmark->setZOrder(1);
            checkmark->setScale(i == 5 || i == 11 ? 0.5f : 1.f);

            if (i == activePage) {
                checkmark->setPosition({onSprite->getContentWidth() / 2, onSprite->getContentHeight() / 2});
                onSprite->addChild(checkmark);
            } else {
                checkmark->setPosition({offSprite->getContentWidth() / 2, offSprite->getContentHeight() / 2});
                offSprite->addChild(checkmark);
            }
        }

        CCMenuItemSpriteExtra* button = CCMenuItemSpriteExtra::create(
            i == activePage ? onSprite : offSprite,
            this,
            menu_selector(ShardPopup::onNavButton));
        button->setID("page-button-" + std::to_string(i));
        button->setTag(i);
        button->setScale(0.8f);
        button->m_baseScale = 0.8f;
        button->setContentWidth(30.f);
        if (auto sprite = typeinfo_cast<CCSprite*>(button->getChildren()->objectAtIndex(0)))
            sprite->setPositionX(15.f);

        m_navButtons->addChild(button);
    }
    m_navButtons->updateLayout();

    if (refresh) return;

    // navigation arrows
    addNavArrows();
}

void ShardPopup::onNavButton(CCObject* sender) {
    CCMenuItemSpriteExtra* button = static_cast<CCMenuItemSpriteExtra*>(sender);
    int pageNum = button->getTag();
    switchToPage(pageNum);
    addNavigation(pageNum);
}

void ShardPopup::onArrow(CCObject* sender) {
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

    switchToPage(newPage);
    addNavigation(newPage);
}

cocos2d::CCNode* ShardPopup::createPage(int pageNum) {
    auto page = CCNode::create();
    page->setPosition({0, 0});

    /* Path Logo*/
    CCSprite* shardLogo = CCSprite::createWithSpriteFrameName(shardLogos[pageNum]);
    if (!shardLogo) {
        log::error("Failed to load shard logo for page {}", pageNum);
    } else {
        shardLogo->setID("shard-logo");
        shardLogo->setScale(1.2f);
        shardLogo->setPosition({225, 260});
        page->addChild(shardLogo);
    }

    /* Progress Fraction */
    CCNode* container = CCNode::create();
    container->setID("fraction-complete");
    container->setAnchorPoint({0.5f, 0.5f});
    container->setContentSize({100, 20});

    int gameStatID = gameStatIDs[pageNum];
    int progress = 100;
    if (pageNum == 5 || pageNum == 11) {  // bonus pages, these values aren't stored so need to calculate them
        for (int i = pageNum - 5; i < pageNum; ++i) {
            progress = std::min(progress, gameStatsManager->getStat(std::to_string(gameStatIDs[i]).c_str()));
        }
    } else
        progress = gameStatsManager->getStat(std::to_string(gameStatID).c_str());

    CCLabelBMFont* progressLabel = nullptr;
    if (progress < 100) {
        progressLabel = CCLabelBMFont::create(formatWithCommas(progress).c_str(), "bigFont.fnt");
        progressLabel->setScale(0.39f);
        progressLabel->setPosition({container->getContentWidth() / 2 - 2, container->getContentHeight() / 2 - 0.25f});
    } else {
        progressLabel = CCLabelBMFont::create(formatWithCommas(progress).c_str(), "goldFont.fnt");
        progressLabel->setScale(0.5f);
        progressLabel->setPosition({container->getContentWidth() / 2 - 2, container->getContentHeight() / 2});
    }

    CCLabelBMFont* goalLabel = CCLabelBMFont::create("/100", "goldFont.fnt");
    goalLabel->setScale(0.5f);
    goalLabel->setAnchorPoint({0, 0.5f});
    goalLabel->setPosition({container->getContentWidth() / 2 - 3, container->getContentHeight() / 2});
    progressLabel->setAnchorPoint({1, 0.5f});

    CCSprite* shard = CCSprite::createWithSpriteFrameName(shardSprites[pageNum]);
    shard->setID("shard");
    shard->setScale(pageNum == 5 || pageNum == 11 ? 0.75f : 0.5f);
    shard->setPosition({container->getContentWidth() / 2 + 34, container->getContentHeight() / 2 - 1});

    container->addChild(progressLabel);
    container->addChild(goalLabel);
    container->addChild(shard);
    container->setPosition({225, 240});
    page->addChild(container);

    /* Progress Bar */
    CCNode* progressBar = CCNode::create();
    progressBar->setID("progress-bar-container");
    progressBar->setPosition({m_mainLayer->getContentWidth() / 2, 120.f});
    page->addChild(progressBar);

    int numIconsOnPage = 5;
    int numDotsOnPage = numIconsOnPage + 1;
    float dotSpacing = std::min(50.f, 400.f / numDotsOnPage);
    progressBar->addChild(buildProgressBarBg({numDotsOnPage, numIconsOnPage, dotSpacing}));

    // Fill (player color or white)
    bool usePlayerColors = Mod::get()->getSettingValue<bool>("use-player-colors");
    ccColor3B fillColor = usePlayerColors ? gameManager->colorForIdx(gameManager->getPlayerColor()) : ccc3(255, 255, 255);

    int numAchievementsUnlocked = 0;
    for (int i = m_maxIconsPerPage * pageNum; i < m_maxIconsPerPage * (pageNum + 1); ++i) {
        if (achievementManager->isAchievementEarned(m_category->achievements[i]->id.c_str())) {
            numAchievementsUnlocked++;
        }
    }

    CCNode* progressBarFill = buildProgressFill(
        numAchievementsUnlocked + 1, numDotsOnPage,
        m_category->achievements, pageNum * m_maxIconsPerPage,
        float(progress), {dotSpacing, float(numIconsOnPage), fillColor});
    progressBar->addChild(progressBarFill);

    /* Player Sprites */
    CCMenu* playerMenu = CCMenu::create();
    playerMenu->setID("unlocks-container");
    playerMenu->setPosition(progressBar->getPosition());
    page->addChild(playerMenu);

    const int unlockValues[] = {5, 15, 35, 65, 100};

    for (int i = 0; i < numIconsOnPage; ++i) {
        Achievement* currAchievement = m_category->achievements[i + pageNum * m_maxIconsPerPage];
        bool earned = achievementManager->isAchievementEarned(currAchievement->id.c_str());

        CCMenuItemSpriteExtra* unlockButton = createAchievementIconButton(
            currAchievement, earned, usePlayerColors, this, menu_selector(ShardPopup::onIcon), std::to_string(i));

        float baseX = -dotSpacing * numIconsOnPage / 2.f + dotSpacing * (i + 1);
        float baseY = (i % 2 == 0) ? 40.f : -40.f;
        unlockButton->setPosition({baseX, baseY});
        playerMenu->addChild(unlockButton);

        // Create the text that shows how much is needed to unlock
        CCLabelBMFont* unlockValue = CCLabelBMFont::create(formatWithCommas(unlockValues[i]).c_str(), "bigFont.fnt");
        unlockValue->setID("unlock-text-" + std::to_string(i));
        unlockValue->setScale(.25f);
        float textYOffset = (i % 2 == 0) ? 18.f : -16.f;
        unlockValue->setPosition({baseX, baseY + textYOffset});
        playerMenu->addChild(unlockValue);
    }

    return page;
}