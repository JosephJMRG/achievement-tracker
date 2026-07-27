#include "ProgressPopup.hpp"
#include "../ProgressCalculator.hpp"

using namespace geode::prelude;

ProgressPopup* ProgressPopup::create(AchievementMenu* achievementMenu, Category* category) {
    auto popup = new ProgressPopup();
    if (popup && popup->init(achievementMenu, category)) {
        popup->autorelease();
        return popup;
    }
    delete popup;
    return nullptr;
}

bool ProgressPopup::init(AchievementMenu* achievementMenu, Category* category) {
    if (!Popup::init(450.f, 280.f))
        return false;

    m_achievementMenu = achievementMenu;
    m_category = category;
    m_numAchievements = m_category->achievements.size();

    // Sort achievements by unlock value
    std::sort(m_category->achievements.begin(), m_category->achievements.end(), [](Achievement* a, Achievement* b) {
        return a->unlockValue < b->unlockValue;
    });

    createTitle();

    if (m_category->name == "Followed Creators")
        m_statValue = gameLevelManager->m_followedCreators->count();
    else
        m_statValue = m_category->statKey.empty() ? 0 : gameStatsManager->getStat(m_category->statKey.c_str());

    addProgressText(m_statValue, m_category->achievements.back()->unlockValue);

    addLogo();

    addCornerSprites();

    // create pages
    m_maxIconsPerPage = 14;
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

    addNavigation();

    return true;
}

CCNode* ProgressPopup::createPage(int pageNum) {
    auto page = CCNode::create();
    page->setPosition({0, 0});

    int numIconsOnPage = std::min(m_maxIconsPerPage, m_numAchievements - m_maxIconsPerPage * pageNum);

    CCNode* progressBar = createProgressBar(pageNum, numIconsOnPage);
    progressBar->setID("progress-bar-container");
    progressBar->setPosition({m_mainLayer->getContentWidth() / 2, 120.f});
    page->addChild(progressBar);

    /* Player Sprites */
    CCMenu* playerMenu = CCMenu::create();
    playerMenu->setID("unlocks-container");
    playerMenu->setPosition(progressBar->getPosition());
    page->addChild(playerMenu);

    float dotSpacing = std::min(50.f, 400.f / (numIconsOnPage + 1));
    bool usePlayerColors = Mod::get()->getSettingValue<bool>("use-player-colors");

    for (int i = 0; i < numIconsOnPage; ++i) {
        Achievement* currAchievement = m_category->achievements[i + pageNum * m_maxIconsPerPage];
        bool earned = achievementManager->isAchievementEarned(currAchievement->id.c_str());

        CCMenuItemSpriteExtra* unlockButton = createAchievementIconButton(
            currAchievement, earned, usePlayerColors, this, menu_selector(ProgressPopup::onIcon), std::to_string(i));

        float baseX = -dotSpacing * numIconsOnPage / 2.f + dotSpacing * (i + 1);
        float baseY = (i % 2 == 0) ? 40.f : -40.f;
        unlockButton->setPosition({baseX, baseY});
        playerMenu->addChild(unlockButton);

        // Create the text that shows how much is needed to unlock
        CCLabelBMFont* unlockValue = CCLabelBMFont::create(formatWithCommas(currAchievement->unlockValue).c_str(), "bigFont.fnt");
        unlockValue->setID("unlock-text-" + std::to_string(i));
        unlockValue->setScale(.25f);
        float textYOffset = (i % 2 == 0) ? 18.f : -16.f;
        unlockValue->setPosition({baseX, baseY + textYOffset});
        playerMenu->addChild(unlockValue);
    }

    return page;
}

CCNode* ProgressPopup::createProgressBar(int pageNum, int numIconsOnPage) {
    /* Progress Bar */
    CCNode* progressBar = CCNode::create();

    int numDotsOnPage = numIconsOnPage + 1;
    float dotSpacing = std::min(50.f, 400.f / numDotsOnPage);
    progressBar->addChild(buildProgressBarBg({numDotsOnPage, numIconsOnPage, dotSpacing}));

    // Fill (player color or white)
    bool usePlayerColors = Mod::get()->getSettingValue<bool>("use-player-colors");
    ccColor3B fillColor = usePlayerColors ? gameManager->colorForIdx(gameManager->getPlayerColor()) : ccc3(255, 255, 255);

    int numAchievementsUnlocked = computeCategoryProgress(
        m_category->displayType, m_category->achievements, m_category->statKey,
        m_category->achievements.empty() ? 0 : m_category->achievements.back()->unlockValue);

    int pageStart = pageNum * m_maxIconsPerPage;
    int numDotsToFill = std::max(0, std::min(numDotsOnPage, numAchievementsUnlocked - pageStart + 1));

    CCNode* progressBarFill = buildProgressFill(
        numDotsToFill, numDotsOnPage,
        m_category->achievements, pageStart,
        float(m_statValue), {dotSpacing, float(numIconsOnPage), fillColor},
        true);
    progressBar->addChild(progressBarFill);

    return progressBar;
}
