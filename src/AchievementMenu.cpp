#include "AchievementMenu.hpp"
#include "popups/DistinctPopup.hpp"
#include "popups/PathPopup.hpp"
#include "popups/ProgressPopup.hpp"
#include "popups/ShardPopup.hpp"
#include "Utils.hpp"
#include <tuple>

using namespace geode::prelude;

bool AchievementMenu::init() {
    if (!Popup::init(450.f, 280.f))
        return false;

    buildSharedCategories();
    m_achievementCategories = s_achievementCategories;

    setTitle("Achievements", "goldFont.fnt", 1.0f, 15.f);

    addCornerSprites();

    // Achievement parsing is handled entirely by buildSharedCategories() in Utils.cpp.
    // It already populates s_achievementCategories with all achievements from GD,
    // which we copied above. No second pass needed.

    auto SFC = CCSpriteFrameCache::get();
    SFC->addSpriteFramesWithFile("TowerSheet.plist");

    if (Mod::get()->getSettingValue<bool>("summary-page-first")) {
        createSummaryPage();
        createCategoryMenu();
        createTrackingPage();
    } else {
        createCategoryMenu();
        createSummaryPage();
        createTrackingPage();
    }

    addNavigation();

    return true;
}

AchievementMenu* AchievementMenu::create() {
    auto popup = new AchievementMenu();
    if (popup && popup->init()) {
        popup->autorelease();
        return popup;
    }
    delete popup;
    return nullptr;
}

void AchievementMenu::createCategoryMenu() {
    std::vector<std::string> pageTitles = {"Levels", "Stats", "Other"};
    for (int i = 0; i < pageTitles.size(); i++) {
        auto menuPage = CCNode::create();
        menuPage->setID("page-" + std::to_string(m_categoriesMenu.size()));
        menuPage->setTag(m_categoriesMenu.size());
        menuPage->setContentSize({m_mainLayer->getContentWidth(), m_mainLayer->getContentHeight() - 70.f});
        menuPage->setPosition({0, 0});
        menuPage->setVisible(m_categoriesMenu.size() == m_categoryPage);
        m_mainLayer->addChild(menuPage);

        auto subTitle = CCLabelBMFont::create(pageTitles[i].c_str(), "bigFont.fnt");
        subTitle->setID("page-subtitle");
        subTitle->setScale(0.5f);
        subTitle->setPosition({menuPage->getContentWidth() / 2, 247});
        menuPage->addChild(subTitle);

        auto buttonMenu = CCMenu::create();
        buttonMenu->setID("categories-menu");
        buttonMenu->setContentSize({m_mainLayer->getContentWidth() - 70, m_mainLayer->getContentHeight() - 70.f});
        buttonMenu->setPosition({m_mainLayer->getContentWidth() / 2, m_mainLayer->getContentHeight() / 2 - 8.f});
        buttonMenu->setLayout(RowLayout::create()
                                  ->setGap(12.f)
                                  ->setAxisAlignment(AxisAlignment::Center)
                                  ->setCrossAxisAlignment(AxisAlignment::Even)
                                  ->setGrowCrossAxis(true));
        menuPage->addChild(buttonMenu);

        int totalAchievementsInPage = 0;
        int completedAchievementsInPage = 0;
        addCategoryButtons(buttonMenu, pageTitles[i], totalAchievementsInPage, completedAchievementsInPage);
        buttonMenu->updateLayout();

        CCNode* progressText = createFractionLabel(completedAchievementsInPage, totalAchievementsInPage);
        progressText->setID("page-progress-fraction");
        progressText->setPosition({menuPage->getContentWidth() / 2, 235});
        menuPage->addChild(progressText);

        m_categoriesMenu.push_back(menuPage);
    }
}

void AchievementMenu::createSummaryPage() {
    auto summaryPage = CCNode::create();
    summaryPage->setID("page-summary");
    summaryPage->setTag(m_categoriesMenu.size());
    summaryPage->setContentSize({m_mainLayer->getContentWidth(), m_mainLayer->getContentHeight() - 70.f});
    summaryPage->setPosition({0, 0});
    summaryPage->setVisible(m_categoriesMenu.size() == m_categoryPage);
    m_mainLayer->addChild(summaryPage);

    auto subTitle = CCLabelBMFont::create("Summary", "bigFont.fnt");
    subTitle->setID("page-subtitle");
    subTitle->setScale(0.5f);
    subTitle->setPosition({summaryPage->getContentWidth() / 2, 247});
    summaryPage->addChild(subTitle);

    std::vector<std::tuple<std::string, std::string, std::string>> summaryTiles = {
        {"geometry-dash", "Geometry Dash", "geometry_dash.png"_spr},
        {"geometry-dash-meltdown", "Geometry Dash Meltdown", "meltdown.png"_spr},
        {"geometry-dash-world", "Geometry Dash World", "world.png"_spr},
        {"geometry-dash-subzero", "Geometry Dash Subzero", "subzero.png"_spr}};

    CCMenu* tiles = CCMenu::create();
    tiles->setPosition({m_mainLayer->getContentWidth() / 2, m_mainLayer->getContentHeight() / 2});
    tiles->setContentSize({80 * float(summaryTiles.size()) / 2 + 50, m_mainLayer->getContentHeight() - 100});
    tiles->setID("tiles");
    tiles->setLayout(RowLayout::create()
                         ->setAutoScale(false)
                         ->setGrowCrossAxis(true)
                         ->setAxisAlignment(AxisAlignment::Center)
                         ->setCrossAxisAlignment(AxisAlignment::Start)
                         ->setGap(12.f));
    summaryPage->addChild(tiles);

    for (int i = 0; i < summaryTiles.size(); i++) {
        CCMenu* tile = CCMenu::create();
        tile->setContentSize({80, 80});
        tile->setID("tile-" + std::get<0>(summaryTiles[i]));
        tiles->addChild(tile);

        /* Background */
        CCScale9Sprite* bg = CCScale9Sprite::create("square02b_001.png");
        bg->setID("bg");
        bg->setContentSize(tile->getContentSize());
        bg->setPosition({tile->getContentWidth() / 2, tile->getContentHeight() / 2});
        bg->setColor({116, 56, 29});
        tile->addChild(bg);

        /* Description */
        SimpleTextArea* desc = SimpleTextArea::create(std::get<1>(summaryTiles[i]), "bigFont.fnt", 0.2f, 70);
        desc->setAlignment(kCCTextAlignmentCenter);
        desc->setWrappingMode(WrappingMode::SPACE_WRAP);
        desc->setAnchorPoint({0.5f, 0.5f});
        desc->setID("desc");
        desc->setPosition({tile->getContentWidth() / 2, tile->getContentHeight() - 10});
        tile->addChild(desc);

        /* Logo sprite */
        CCSprite* logoSprite = CCSprite::create(std::get<2>(summaryTiles[i]).c_str());
        logoSprite->setID("logo-" + std::get<0>(summaryTiles[i]));
        logoSprite->setScale(0.5f);
        logoSprite->setPosition({tile->getContentWidth() / 2, tile->getContentHeight() / 2});

        tile->addChild(logoSprite);

        /* Progress fraction */
        int total = 0;
        int completed = 0;
        for (auto& cat : m_achievementCategories) {
            bool include = false;
            if (std::get<1>(summaryTiles[i]) == "Geometry Dash") {
                include = cat.name != "Geometry Dash Meltdown"
                       && cat.name != "Geometry Dash World"
                       && cat.name != "Geometry Dash Subzero";
            } else {
                include = cat.name == std::get<1>(summaryTiles[i]);
            }
            if (!include) continue;
            total += (int)cat.achievements.size();
            completed += countEarnedAchievements(cat.achievements);
        }
        CCNode* progressText = createFractionLabel(completed, total);
        progressText->setID("progress-" + std::get<0>(summaryTiles[i]));
        progressText->setPosition({tile->getContentWidth() / 2, 10});
        tile->addChild(progressText);
    }

    tiles->updateLayout();

    m_categoriesMenu.push_back(summaryPage);
}

void AchievementMenu::createTrackingPage() {
    //
    // Two-column quest-card layout inspired by UI example.html.
    // Left column = Incomplete, right column = Completed.
    // Each card mirrors the HTML exactly:
    //   - #2d3b8e bg + brown border (#5c3a21) as rounded rect
    //   - Left: title (white w/ shadow look via goldFont), subtitle (gold), progress bar
    //     (50% of card width, proportional height, #1a237e bg, #39ff14 fill)
    //   - Right: big %, "Completed" label, gold color
    //   - Track toggle checkbox on far right edge
    //
    auto trackingPage = CCNode::create();
    trackingPage->setID("page-tracking");
    trackingPage->setTag((int)m_categoriesMenu.size());
    trackingPage->setContentSize({m_mainLayer->getContentWidth(), m_mainLayer->getContentHeight() - 70.f});
    trackingPage->setPosition({0, 35});
    trackingPage->setVisible((int)m_categoriesMenu.size() == m_categoryPage);
    m_mainLayer->addChild(trackingPage);

    auto subTitle = CCLabelBMFont::create("Tracking", "bigFont.fnt");
    subTitle->setID("page-subtitle");
    subTitle->setScale(0.5f);
    subTitle->setPosition({trackingPage->getContentWidth() / 2, trackingPage->getContentHeight()});
    trackingPage->addChild(subTitle);

    struct CatInfo {
        int catIndex;
        int totalCount;
        int earnedCount;
    };
    std::vector<CatInfo> incomplete;
    std::vector<CatInfo> completed;

    for (int i = 0; i < (int)m_achievementCategories.size(); i++) {
        auto& cat = m_achievementCategories[i];
        int total = (int)cat.achievements.size();
        if (total == 0) continue;
        int earned = countEarnedAchievements(cat.achievements);
        CatInfo info{i, total, earned};
        if (earned == total)
            completed.push_back(info);
        else
            incomplete.push_back(info);
    }

    // Sort incomplete: tracked first, then by completion % descending
    std::stable_sort(incomplete.begin(), incomplete.end(),
        [this](const CatInfo& a, const CatInfo& b) {
            bool aT = isCategoryTracked(m_achievementCategories[a.catIndex].name);
            bool bT = isCategoryTracked(m_achievementCategories[b.catIndex].name);
            if (aT != bT) return aT && !bT;
            float pctA = a.totalCount > 0 ? (float)a.earnedCount / (float)a.totalCount : 0.f;
            float pctB = b.totalCount > 0 ? (float)b.earnedCount / (float)b.totalCount : 0.f;
            return pctA > pctB;
        }
    );
    // Sort completed by name
    std::stable_sort(completed.begin(), completed.end(),
        [this](const CatInfo& a, const CatInfo& b) {
            return m_achievementCategories[a.catIndex].name < m_achievementCategories[b.catIndex].name;
        }
    );

    // --- Headers (static, outside scroll) ---
    float colPad = 8.f;
    float colW = (trackingPage->getContentWidth() - 22.f - colPad) / 2.f;
    float hdrY = trackingPage->getContentHeight() - 8.f;
    float hdrScale = 0.38f;

    auto incHdr = CCLabelBMFont::create("Incomplete", "bigFont.fnt");
    incHdr->setScale(hdrScale);
    incHdr->setPosition({8.f + colW / 2, hdrY});
    incHdr->setID("header-incomplete");
    incHdr->setColor({255, 204, 0});
    trackingPage->addChild(incHdr);

    auto cmpHdr = CCLabelBMFont::create("Completed", "bigFont.fnt");
    cmpHdr->setScale(hdrScale);
    cmpHdr->setPosition({8.f + colW + colPad + colW / 2, hdrY});
    cmpHdr->setID("header-completed");
    cmpHdr->setColor({255, 204, 0});
    trackingPage->addChild(cmpHdr);

    // --- ScrollLayer ---
    float scrollW = trackingPage->getContentWidth() - 22.f;
    float scrollH = trackingPage->getContentHeight() - 28.f;
    auto scrollLayer = geode::ScrollLayer::create({scrollW, scrollH}, true, true);
    scrollLayer->setID("tracking-scroll");
    scrollLayer->setPosition({10.f, 0.f});
    trackingPage->addChild(scrollLayer);

    auto cl = scrollLayer->m_contentLayer;

    // --- Layout constants ---
    float cardW = colW - 8.f;
    float cardH = 70.f;
    float cardGap = 15.f;
	

    int maxItems = std::max((int)incomplete.size(), (int)completed.size());
    float totalContentH = 4.f + maxItems * (cardH + cardGap) - cardGap;
    if (totalContentH < scrollH) totalContentH = scrollH;
    cl->setContentSize({scrollW, totalContentH});

    // --- Panel builder ---
    auto createPanel = [&, this](const CatInfo& info, float x, float baseY) {
        auto& cat = m_achievementCategories[info.catIndex];
        float pct = info.totalCount > 0 ? std::min(1.f, (float)info.earnedCount / (float)info.totalCount) : 0.f;
        int pctInt = (int)(pct * 100.f);
        bool isComplete = info.earnedCount == info.totalCount;

        // -- Layer 1: White background (cardW+6 x cardH+6) --
        auto layerWhite = CCScale9Sprite::create("square02b_001.png");
        layerWhite->setContentSize({cardW + 6.f, cardH + 6.f});
        layerWhite->setAnchorPoint({0.f, 0.f});
        layerWhite->setPosition({x - 2.f, baseY - 3.f});
        layerWhite->setColor({255, 255, 255});
        layerWhite->setID("card-layer-white");
        cl->addChild(layerWhite);

        // -- Layer 2: Dark blue shadow (cardW+4 x cardH+4) --
        auto layerDark = CCScale9Sprite::create("square02b_001.png");
        layerDark->setContentSize({cardW + 4.f, cardH + 4.f});
        layerDark->setAnchorPoint({0.f, 0.f});
        layerDark->setPosition({x - 1.f, baseY - 2.f});
        layerDark->setColor({20, 30, 100});
        layerDark->setID("card-layer-dark");
        cl->addChild(layerDark);

        // -- Layer 3: Light blue interior (cardW+2 x cardH+2) --
        auto layerLight = CCScale9Sprite::create("square02b_001.png");
        layerLight->setContentSize({cardW + 2.f, cardH + 2.f});
        layerLight->setAnchorPoint({0.f, 0.f});
        layerLight->setPosition({x, baseY - 1.f});
        layerLight->setColor({45, 59, 142});
        layerLight->setID("card-layer-light");
        cl->addChild(layerLight);

        // === LEFT SIDE ===
        float lx = x + 12.f;
		float offSetYTitleSubtitle = 5.f;

        // Title (white with black shadow look = goldFont)
        auto titleLabel = CCLabelBMFont::create(cat.name.c_str(), "goldFont.fnt");
        titleLabel->setAnchorPoint({0.f, 0.5f});
        titleLabel->setScale(0.65f);
        // Prevent title from overflowing horizontally (limits width to ~55% of card)
        titleLabel->limitLabelWidth(cardW * 0.55f, 0.65f, 0.1f);
        titleLabel->setPosition({lx, baseY - offSetYTitleSubtitle + cardH - 12.f});
        titleLabel->setID("card-title");
        cl->addChild(titleLabel);

        // Subtitle (gold #ffcc00)
        std::string subText;
        if (isComplete)
            subText = "ALL ACHIEVEMENTS COMPLETE!";
        else {
            int rem = info.totalCount - info.earnedCount;
            subText = std::to_string(rem) + " MORE" + (rem != 1 ? "S" : "") + " REMAINING";
        }
        auto subLabel = CCLabelBMFont::create(subText.c_str(), "bigFont.fnt");
        subLabel->setAnchorPoint({0.f, 0.5f});
        subLabel->setScale(0.47f);
        // Prevent subtitle from overflowing
        subLabel->limitLabelWidth(cardW * 0.55f, 0.47f, 0.1f);
        subLabel->setColor({255, 204, 0});
        subLabel->setPosition({lx, baseY - offSetYTitleSubtitle + cardH - 27.f});
        subLabel->setID("card-subtitle");
        cl->addChild(subLabel);

        // --- Progress bar (layer-colors, always visible thru scroll clip) ---
        float barX = lx;
        float barY = baseY + 10.f;
        float barW = cardW * 0.70f - 20.f;  // 20px narrower (shrunk from right)
        float barH = 13.f;
        float ins = 2.f;        // inset for green fill
		float offSetYBar = 5.f;
		

        // Blue background (#1a237e) as simple rect
        auto barBg = CCLayerColor::create({26, 35, 126, 255}, barW, barH);
        barBg->setAnchorPoint({0.f, 0.5f});
        barBg->setPosition({barX, barY - offSetYBar});
        barBg->setID("progress-bar-bg");
        cl->addChild(barBg, 10);
        barBg->setOpacity(255);

        // Green fill (#39ff14): 2px inset all around, centered inside barBg
        float fillW = (barW - ins * 2.f) * pct;
        if (fillW > 0.5f) {
            float fillH = barH - ins * 2.f;
            auto barFill = CCLayerColor::create({57, 255, 20, 255}, fillW, fillH);
            barFill->setAnchorPoint({0.f, 0.5f});
            // inset from left + same center Y so it's vertically centered
            barFill->setPosition({barX + ins, barY + 2.f - offSetYBar});
            barFill->setID("progress-bar-fill");
            cl->addChild(barFill, 10);
            barFill->setOpacity(255);
        }

        // Fraction text on top of bar (white)
        std::string fracText = std::to_string(info.earnedCount) + " / " + std::to_string(info.totalCount);
        auto fracLabel = CCLabelBMFont::create(fracText.c_str(), "goldFont.fnt");
        fracLabel->setScale(0.35f);
        fracLabel->setColor({255, 255, 255});
        float fracY = barY + 2.f;  // 2px above bar center
        fracLabel->setPosition({barX + barW / 2, fracY});
        fracLabel->setID("frac-label");
        cl->addChild(fracLabel, 20);

        // === RIGHT SIDE ===
        // Position of the toggle checkbox (on the far right, now at top-right)
        float toggleX = x + cardW - 8.f;

        // Big percentage (gold) — moved 15px right, under checkbox
        float textRx = toggleX - 3.f; // (toggleX - 18) + 15 = toggleX - 3
        float pctY = baseY + cardH - 30.f;
        std::string pctText = std::to_string(pctInt) + "%";
        auto pctLabel = CCLabelBMFont::create(pctText.c_str(), "goldFont.fnt");
        pctLabel->setAnchorPoint({1.f, 0.5f}); // Anchor en la derecha! (evita el desbordamiento)
        pctLabel->setScale(0.65f);
        pctLabel->setColor({255, 204, 0});
        pctLabel->setPosition({textRx, pctY});
        pctLabel->setID("card-pct");
        cl->addChild(pctLabel);

        // "Completed" label (gold) — moved 15px right, under %
        auto compLabel = CCLabelBMFont::create("Completed", "bigFont.fnt");
        compLabel->setAnchorPoint({1.f, 0.5f}); // Anchor en la derecha!
        compLabel->setScale(0.25f);
        compLabel->setColor({255, 204, 0});
        compLabel->setPosition({textRx, pctY - 12.f});
        compLabel->setID("card-completed-label");
        cl->addChild(compLabel);

        // --- Track toggle (checkbox) — top-right (hidden when 100% complete) ---
        if (!isComplete) {
            bool isTracked = isCategoryTracked(cat.name);
            auto checkSprite = CCSprite::createWithSpriteFrameName(
                isTracked ? "GJ_checkOn_001.png" : "GJ_checkOff_001.png"
            );
            checkSprite->setScale(0.50f);
            checkSprite->setID("check-sprite");

            auto checkBtn = CCMenuItemSpriteExtra::create(
                checkSprite, this,
                menu_selector(AchievementMenu::onTrackingToggle)
            );
            checkBtn->setTag(info.catIndex);
            checkBtn->setID("tracking-toggle-" + cat.name);

            auto checkMenu = CCMenu::create();
            checkMenu->setPosition({toggleX, baseY + cardH - 9.f});
            checkMenu->addChild(checkBtn);
            cl->addChild(checkMenu);
        }
    };

    // --- Place panels: left = incomplete, right = completed ---
    float leftX = 2.f;
    float rightX = colW + colPad - 2.f;

    float y = totalContentH - 4.f - cardH;
    for (int i = 0; i < maxItems; i++) {
        if (i < (int)incomplete.size())
            createPanel(incomplete[i], leftX, y);
        if (i < (int)completed.size())
            createPanel(completed[i], rightX, y);
        y -= (cardH + cardGap);
    }

    scrollLayer->moveToTop();

    m_categoriesMenu.push_back(trackingPage);
    m_trackingPageTag = trackingPage->getTag();
}
void AchievementMenu::doDelayedRefreshTracking(float dt) {
    refreshTrackingPage();
}

void AchievementMenu::refreshTrackingPage() {
    // Find old tracking page by tag and remove it
    for (int i = 0; i < (int)m_categoriesMenu.size(); i++) {
        if (m_categoriesMenu[i]->getTag() == m_trackingPageTag) {
            auto oldPage = m_categoriesMenu[i];
            oldPage->removeFromParent();
            m_categoriesMenu.erase(m_categoriesMenu.begin() + i);
            break;
        }
    }
    // Create new one (appends to m_categoriesMenu)
    createTrackingPage();
}

void AchievementMenu::onTrackingToggle(CCObject* sender) {
    auto button = static_cast<CCMenuItemSpriteExtra*>(sender);
    int catIndex = button->getTag();
    auto& cat = m_achievementCategories[catIndex];

    // Read current state and flip it
    bool wasTracked = isCategoryTracked(cat.name);
    bool newState = !wasTracked;

    log::info("AchievementMenu::onTrackingToggle: name='{}' catIndex={} wasTracked={} newState={}", cat.name, catIndex, wasTracked, newState);

    // Swap sprite frame (same pattern as AchievementCategoryPopup::onTrackingToggle)
    auto sprite = static_cast<CCSprite*>(button->getChildren()->objectAtIndex(0));
    sprite->setDisplayFrame(
        CCSpriteFrameCache::get()->spriteFrameByName(
            newState ? "GJ_checkOn_001.png" : "GJ_checkOff_001.png"
        )
    );

    // Update category tracking
    setCategoryTracked(cat.name, newState);

    // Schedule refresh for next frame (safer than refreshing immediately while inside a callback)
    this->scheduleOnce(schedule_selector(AchievementMenu::doDelayedRefreshTracking), 0.f);
}

void AchievementMenu::addCategoryButtons(CCMenu* menuPage, std::string pageTitle, int& totalAchievementsInPage, int& completedAchievementsInPage) {
    for (int i = 0; i < m_achievementCategories.size(); i++) {
        if (m_achievementCategories[i].page != pageTitle) continue;

        int totalAchievementsInCategory = m_achievementCategories[i].achievements.size();
        totalAchievementsInPage += totalAchievementsInCategory;

        int completedAchievementsInCategory = 0;

        ButtonSprite* buttonSprite = ButtonSprite::create(m_achievementCategories[i].formattedName.c_str(), 90.f, true, "bigFont.fnt", "GJ_button_01.png", 40.f, 0.35f);

        // slide over the text to make room for the logo
        buttonSprite->m_label->setPositionX(buttonSprite->m_label->getContentWidth() / 2 * buttonSprite->m_label->getScale() + 37.f);
        buttonSprite->m_label->setPositionY(buttonSprite->m_label->getPositionY() - 1.f);

        // The little logo on the left side of the button
        const std::vector<std::string> fromSpritesheet = {"Stars", "Moons", "Diamonds", "Secret Coins", "User Coins", "Creator"};

        CCSprite* logo;
        if (std::find(fromSpritesheet.begin(), fromSpritesheet.end(), m_achievementCategories[i].name) != fromSpritesheet.end())
            logo = CCSprite::createWithSpriteFrameName(m_achievementCategories[i].logo.c_str());  // try from spritesheet
        else
            logo = CCSprite::create(m_achievementCategories[i].logo.c_str());  // otherwise try mod resources from logos/

        if (!logo) {
            log::error("Failed to load logo for category: {}", m_achievementCategories[i].name);
        } else {
            logo->setID("logo");
            logo->setAnchorPoint({0.5f, 0.5f});
            logo->setScale(std::min(20.f / logo->getContentWidth(), 20.f / logo->getContentHeight()));
            logo->setPosition({20.f, buttonSprite->getContentHeight() / 2});
            logo->setZOrder(1);
            buttonSprite->addChild(logo);

            if (m_achievementCategories[i].name == "Jumps") {
                auto* jumpingIcon = createJumpsIcon();
                jumpingIcon->setPosition({40, 60});
                logo->addChild(jumpingIcon);
            }
        }

        // Calculate some values for the checkmark and progress fraction
        completedAchievementsInCategory = countEarnedAchievements(m_achievementCategories[i].achievements);
        completedAchievementsInPage += completedAchievementsInCategory;
        bool isCategoryCompleted = (completedAchievementsInCategory == totalAchievementsInCategory);
        // Checkmark for completed categories
        CCSprite* checkmark = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
        checkmark->setID("checkmark");
        checkmark->setPosition({buttonSprite->getContentWidth() - 5.f, buttonSprite->getContentHeight() - 5.f});
        checkmark->setZOrder(1);
        buttonSprite->addChild(checkmark);

        checkmark->setVisible(isCategoryCompleted);

        // Progress fraction for progress categories, if enabled
        if (!Mod::get()->getSettingValue<bool>("hide-category-count") && totalAchievementsInCategory > 0) {
            CCNode* completedFraction = createFractionLabel(completedAchievementsInCategory, totalAchievementsInCategory);
            completedFraction->setID("category-progress-fraction");
            completedFraction->setAnchorPoint({0, 0.5f});
            completedFraction->setPosition({4.f, buttonSprite->getContentHeight() - 6.f});
            completedFraction->setScale(0.75f);
            buttonSprite->addChild(completedFraction);
        }

        CCMenuItemSpriteExtra* button = CCMenuItemSpriteExtra::create(
            buttonSprite,
            this,
            menu_selector(AchievementMenu::onCategoryButton));

        button->setID(m_achievementCategories[i].name);
        button->setTag(i);

        menuPage->addChild(button);
    }
}

void AchievementMenu::onCategoryButton(CCObject* sender) {
    CCMenuItemSpriteExtra* categoryButton = static_cast<CCMenuItemSpriteExtra*>(sender);

    int index = categoryButton->getTag();
    Category* category = &m_achievementCategories[index];

    AchievementCategoryPopup* popup;

    if (category->displayType == "distinct" || Mod::get()->getSettingValue<bool>("all-discrete")) {
        popup = DistinctPopup::create(this, category);
    } else if (category->displayType == "progress") {
        popup = ProgressPopup::create(this, category);
    } else if (category->displayType == "shard") {
        popup = ShardPopup::create(this, category);
    } else if (category->displayType == "path") {
        popup = PathPopup::create(this, category);
    }
    popup->m_noElasticity = GameManager::get()->getGameVariable("0168");  // For fast menu setting
    popup->show();

    m_navMenu->getChildByID("left-arrow")->setVisible(false);
    m_navMenu->getChildByID("right-arrow")->setVisible(false);
}

void AchievementMenu::addNavigation() {
    if (m_categoriesMenu.size() <= 1) return;

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
    for (int i = 0; i < m_categoriesMenu.size(); i++) {
        CCMenuItemSpriteExtra* button = CCMenuItemSpriteExtra::create(
            i == 0 ? CCSprite::createWithSpriteFrameName("gj_navDotBtn_on_001.png") : CCSprite::createWithSpriteFrameName("gj_navDotBtn_off_001.png"),
            this,
            menu_selector(AchievementMenu::onNavButton));
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
        menu_selector(AchievementMenu::onCategoryArrow));
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
        menu_selector(AchievementMenu::onCategoryArrow));
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

void AchievementMenu::onCategoryArrow(CCObject* sender) {
    int arrowType = static_cast<CCMenuItemSpriteExtra*>(sender)->getTag();
    if (arrowType == 0) m_categoryPage--;
    else if (arrowType == 1) m_categoryPage++;
    applyPage();
}

void AchievementMenu::onNavButton(CCObject* sender) {
    m_categoryPage = static_cast<CCMenuItemSpriteExtra*>(sender)->getTag();
    applyPage();
}

void AchievementMenu::applyPage() {
    // If navigating to tracking page, rebuild it (ensures checkbox sync)
    if (m_categoryPage == m_trackingPageTag) {
        refreshTrackingPage();
    }

    // page visibility
    for (auto menu : m_categoriesMenu) {
        menu->setVisible(menu->getTag() == m_categoryPage);
    }

    // navigation buttons
    for (int i = 0; i < m_categoriesMenu.size(); i++) {
        CCMenuItemSpriteExtra* navButton = static_cast<CCMenuItemSpriteExtra*>(m_navButtons->getChildByID("page-button-" + std::to_string(i)));
        if (navButton) {
            navButton->setNormalImage(i == m_categoryPage ? CCSprite::createWithSpriteFrameName("gj_navDotBtn_on_001.png") : CCSprite::createWithSpriteFrameName("gj_navDotBtn_off_001.png"));
        }
    }
    // arrow visibility
    m_navMenu->getChildByID("left-arrow")->setVisible(m_categoryPage > 0);
    m_navMenu->getChildByID("right-arrow")->setVisible(m_categoryPage < m_categoriesMenu.size() - 1);
}

void AchievementMenu::addCornerSprites() {
    ::addCornerSprites(m_mainLayer);
}
