#include "Utils.hpp"
#include "ProgressCalculator.hpp"

using namespace geode::prelude;

std::map<std::string, std::pair<std::string, std::string>> betterDescriptions = {
    // Secret
    {"geometry.ach.secret04", {"Find the hidden coin on the Coming Soon screen", "Found the hidden coin on the Coming Soon screen"}},
    {"geometry.ach.secret11", {"Find the secret by destroying this cube on the main menu", "Found the secret by destroying this cube on the main menu"}},
    {"geometry.ach.secret12", {"Find the secret by destroying this cube on the main menu", "Found the secret by destroying this cube on the main menu"}},

    // Vaults
    {"geometry.ach.secret05", {"Find the secret by entering 'lenny' into the Vault", "Found the secret by entering 'lenny' into the Vault"}},
    {"geometry.ach.secret06", {"Steal the Vault Keeper's coin by entering 'sparky' into the Vault", "Stole the Vault Keeper's coin by entering 'sparky' into the Vault"}},
    {"geometry.ach.secret07", {"Steal the Vault Keeper's icon by entering 'spooky' into the Vault", "Stole the Vault Keeper's icon by entering 'spooky' into the Vault"}},
    {"geometry.ach.secret08", {"Find the secret by entering 'blockbite' into the Vault", "Found the secret by entering 'blockbite' into the Vault"}},
    {"geometry.ach.secret09", {"Find the secret by entering 'robotop' into the Vault", "Found the secret by entering 'robotop' into the Vault"}},
    {"geometry.ach.secret10", {"Find the secret by entering 'ahead' into the Vault", "Found the secret by entering 'ahead' into the Vault"}},
    {"geometry.ach.secret13", {"Find the secret by entering 'mule' into the Vault", "Found the secret by entering 'mule' into the Vault"}},
    {"geometry.ach.secret14", {"Find the secret by entering 'neverending' into the Vault", "Found the secret by entering 'neverending' into the Vault"}},
    {"geometry.ach.secret15", {"Find the secret by entering 'gandalfpotter' into the Vault", "Found the secret by entering 'gandalfpotter' into the Vault"}},
    {"geometry.ach.secret16", {"Find the secret by consecutively entering '8', '16', '30', '32', '46' and '84' into the Vault", "Found the secret by consecutively entering '8', '16', '30', '32', '46' and '84' into the Vault"}},
    {"geometry.ach.secret17", {"Find the secret by entering your username into the Vault", "Found the secret by entering your username into the Vault"}},
    {"geometry.ach.secret19", {"Find the secret by entering 'finalboss' into the Vault", "Found the secret by entering 'finalboss' into the Vault"}},
    {"geometry.ach.v2.secret01", {"Find the secret by entering 'brainpower' into the Vault of Secrets", "Found the secret by entering 'brainpower' into the Vault of Secrets"}},
    {"geometry.ach.v2.secret02", {"Find the secret by solving the 'cod3breaker' puzzle in the Vault of Secrets", "Found the secret by solving the 'cod3breaker' puzzle in the Vault of Secrets"}},
    {"geometry.ach.v2.secret03", {"Steal the Keymaster's coin by solving the 'glubfub' puzzle in the Vault of Secrets", "Stole the Keymaster's coin by solving the 'glubfub' puzzle in the Vault of Secrets"}},
    {"geometry.ach.v2.secret04", {"Find the secret by entering 'octocube' into the Vault of Secrets", "Found the secret by entering 'octocube' into the Vault of Secrets"}},
    {"geometry.ach.v2.secret05", {"Find the secret by entering your star count into the Vault of Secrets", "Found the secret by entering your star count into the Vault of Secrets"}},
    {"geometry.ach.v2.secret06", {"Find the secret by entering 'seven' into the Vault of Secrets", "Found the secret by entering 'seven' into the Vault of Secrets"}},
    {"geometry.ach.v2.secret07", {"Find the secret by entering 'gimmiethecolor' into the Vault of Secrets", "Found the secret by entering 'gimmiethecolor' into the Vault of Secrets"}},
    {"geometry.ach.v2.secret08", {"Find the secret by entering 'thechickenisonfire' into the Vault of Secrets", "Found the secret by entering 'thechickenisonfire' into the Vault of Secrets"}},
    {"geometry.ach.v2.secret09", {"Find the secret by entering 'd4shg30me7ry' into the Vault of Secrets", "Found the secret by entering 'd4shg30me7ry' into the Vault of Secrets"}},
    {"geometry.ach.v2.secret10", {"Find the secret by entering 'thechickenisready' into the Vault of Secrets", "Found the secret by entering 'thechickenisready' into the Vault of Secrets"}},
    {"geometry.ach.v3.secret01", {"Find the secret by entering 'darkness' into the Chamber of Time", "Found the secret by entering 'darkness' into the Chamber of Time"}},
    {"geometry.ach.v3.secret02", {"Find the secret by entering 'silence' into the Chamber of Time", "Found the secret by entering 'silence' into the Chamber of Time"}},
    {"geometry.ach.v3.secret03", {"Find the secret by entering 'river' into the Chamber of Time", "Found the secret by entering 'river' into the Chamber of Time"}},
    {"geometry.ach.v3.secret04", {"Find the secret by entering 'hunger' into the Chamber of Time", "Found the secret by entering 'hunger' into the Chamber of Time"}},
    {"geometry.ach.v3.secret05", {"Find the secret by entering 'volcano' into the Chamber of Time", "Found the secret by entering 'volcano' into the Chamber of Time"}},
    {"geometry.ach.v3.secret06", {"Find the secret by entering 'backontrack' into the Chamber of Time", "Found the secret by entering 'backontrack' into the Chamber of Time"}},
    {"geometry.ach.v3.secret07", {"Find the secret by entering 'givemehelper' into the Chamber of Time", "Found the secret by entering 'givemehelper' into the Chamber of Time"}},

    // Players Destroyed
    {"geometry.ach.secret01", {"Destroy 1 player on the main menu", "Destroyed 1 player on the main menu"}},
    {"geometry.ach.secret02", {"Destroy 50 players on the main menu", "Destroyed 50 players on the main menu"}},
    {"geometry.ach.secret02b", {"Destroy 100 players on the main menu", "Destroyed 100 players on the main menu"}},
    {"geometry.ach.secret03", {"Destroy 200 players on the main menu", "Destroyed 200 players on the main menu"}},
    {"geometry.ach.secret03b", {"Destroy 500 players on the main menu", "Destroyed 500 players on the main menu"}},
    {"geometry.ach.secret18", {"Destroy 750 players on the main menu", "Destroyed 750 players on the main menu"}},
};

AchievementManager* achievementManager = AchievementManager::sharedState();
GameManager* gameManager = GameManager::sharedState();
GameStatsManager* gameStatsManager = GameStatsManager::sharedState();
GameLevelManager* gameLevelManager = GameLevelManager::sharedState();

cocos2d::CCNode* createFractionLabel(int num, int denom) {
    CCLabelBMFont* denominatorLabel = CCLabelBMFont::create(("/" + formatWithCommas(denom)).c_str(), "goldFont.fnt");
    denominatorLabel->setScale(0.375f);
    denominatorLabel->setAnchorPoint({0, 0.5f});

    CCLabelBMFont* numeratorLabel = nullptr;
    if (num < denom) {
        numeratorLabel = CCLabelBMFont::create(formatWithCommas(num).c_str(), "bigFont.fnt");
        numeratorLabel->setScale(0.2925f);
    } else {
        numeratorLabel = CCLabelBMFont::create(formatWithCommas(num).c_str(), "goldFont.fnt");
        numeratorLabel->setScale(0.375f);
    }
    numeratorLabel->setAnchorPoint({1, 0.5f});

    CCNode* container = CCNode::create();
    container->setContentSize({numeratorLabel->getContentWidth() * numeratorLabel->getScaleX() + denominatorLabel->getContentWidth() * denominatorLabel->getScaleX(), numeratorLabel->getContentHeight() * numeratorLabel->getScaleY()});
    container->setAnchorPoint({0.5f, 0.5f});
    container->addChild(numeratorLabel);
    container->addChild(denominatorLabel);

    // Adjust position of labels based on the newly set container size
    if (num < denom) {
        numeratorLabel->setPosition({container->getContentWidth() / 2 - 2, container->getContentHeight() / 2 - 0.25f});
    } else {
        numeratorLabel->setPosition({container->getContentWidth() / 2 - 2, container->getContentHeight() / 2});
    }
    denominatorLabel->setPosition({container->getContentWidth() / 2 - 3, container->getContentHeight() / 2});

    return container;
}

std::string formatWithCommas(int number) {
    std::string s = std::to_string(number);
    int n = s.length();
    for (int i = n - 3; i > 0; i -= 3) {
        s.insert(i, ",");
    }
    return s;
}

void addCornerSprites(cocos2d::CCLayer* layer) {
    auto cornerContainer = cocos2d::CCNode::create();
    cornerContainer->setID("corners");
    cornerContainer->setContentSize(layer->getContentSize());
    cornerContainer->setPosition({0.f, 0.f});
    layer->addChild(cornerContainer);

    auto bl = cocos2d::CCSprite::createWithSpriteFrameName("dailyLevelCorner_001.png");
    bl->setID("bottom-left");
    bl->setAnchorPoint({0.f, 0.f});
    bl->setPosition({0.f, 0.f});
    cornerContainer->addChild(bl);

    auto br = cocos2d::CCSprite::createWithSpriteFrameName("dailyLevelCorner_001.png");
    br->setID("bottom-right");
    br->setAnchorPoint({1.f, 0.f});
    br->setPosition({cornerContainer->getContentWidth(), 0.f});
    br->setFlipX(true);
    cornerContainer->addChild(br);

    auto tl = cocos2d::CCSprite::createWithSpriteFrameName("dailyLevelCorner_001.png");
    tl->setID("top-left");
    tl->setAnchorPoint({0.f, 1.f});
    tl->setPosition({0.f, cornerContainer->getContentHeight()});
    tl->setFlipY(true);
    cornerContainer->addChild(tl);

    auto tr = cocos2d::CCSprite::createWithSpriteFrameName("dailyLevelCorner_001.png");
    tr->setID("top-right");
    tr->setAnchorPoint({1.f, 1.f});
    tr->setPosition({cornerContainer->getContentWidth(), cornerContainer->getContentHeight()});
    tr->setFlipX(true);
    tr->setFlipY(true);
    cornerContainer->addChild(tr);
}

// ──── Global shared categories ────
std::vector<Category> s_achievementCategories;
static bool s_categoriesBuilt = false;

void buildSharedCategories() {
    if (s_categoriesBuilt) return;
    s_categoriesBuilt = true;

    s_achievementCategories = {
        // Levels
        {"Main Levels", "Main\nLevels", "Levels", "distinct", "main_levels.png"_spr, {"level##a", "level##b", "demoncoin##", "special##"}},
        {"Tower Levels", "Tower\nLevels", "Levels", "distinct", "tower_levels.png"_spr, {"tower##", "tower##Coin"}},
        {"User Levels", "User\nLevels", "Levels", "progress", "user_levels.png"_spr, {"custom##"}, "4"},
        {"Geometry Dash Meltdown", "Meltdown", "Levels", "distinct", "meltdown.png"_spr, {"mdlevel##b", "mdcoin##", "mdrate"}},
        {"Geometry Dash World", "World", "Levels", "distinct", "world.png"_spr, {"world"}},
        {"Geometry Dash Subzero", "Subzero", "Levels", "distinct", "subzero.png"_spr, {"subzero"}},
        {"Demons", "Demons", "Levels", "progress", "demon.png"_spr, {"demon##"}, "5"},
        {"Insanes", "Insanes", "Levels", "progress", "insane.png"_spr, {"insane##"}, "42"},
        {"Daily Levels", "Daily\nLevels", "Levels", "progress", "daily.png"_spr, {"daily##"}, "15"},
        {"Map Packs", "Map\nPacks", "Levels", "progress", "map_packs.png"_spr, {"mappacks##"}, "7"},
        {"Gauntlets", "Gauntlets", "Levels", "progress", "gauntlets.png"_spr, {"gauntlets##"}, "40"},
        {"Lists", "Lists", "Levels", "progress", "lists.png"_spr, {"lists##"}, "41"},

        // Stats
        {"Stars", "Stars", "Stats", "progress", "GJ_bigStar_noShadow_001.png", {"stars##"}, "6"},
        {"Moons", "Moons", "Stats", "progress", "GJ_bigMoon_noShadow_001.png", {"moons##"}, "28"},
        {"Diamonds", "Diamonds", "Stats", "progress", "GJ_bigDiamond_noShadow_001.png", {"diamonds##"}, "13"},
        {"Secret Coins", "Secret\nCoins", "Stats", "progress", "secretCoin_01_001.png", {"coins##"}, "8"},
        {"User Coins", "User\nCoins", "Stats", "progress", "secretCoin_2_01_001.png", {"usercoins##"}, "12"},
        {"Jumps", "Jumps", "Stats", "progress", "jumps.png"_spr, {"jump##"}, "1"},
        {"Attempts", "Attempts", "Stats", "progress", "attempts.png"_spr, {"attempt##"}, "2"},
        {"Shards", "Shards", "Stats", "shard", "shards.png"_spr, {}},
        {"Paths", "Paths", "Stats", "path", "paths.png"_spr, {}},

        // Other
        {"Liked/Disliked Levels", "Liked /\nDisliked\nLevels", "Other", "progress", "like_dislike.png"_spr, {"like", "like##", "like##b"}, "10"},
        {"Rated Levels", "Rated\nLevels", "Other", "progress", "rate.png"_spr, {"rateDiff", "rateDiff##", "rateDiff##b"}, "11"},
        {"Followed Creators", "Followed\nCreators", "Other", "progress", "followed_creators.png"_spr, {"followCreator", "followCreator##"}},
        {"Friends", "Friends", "Other", "distinct", "friends.png"_spr, {"friends##"}},
        {"Creator", "Creator", "Other", "distinct", "GJ_hammerIcon_001.png", {"creator##", "submit"}},
        {"Vaults", "Vaults", "Other", "distinct", "vaults.png"_spr, {"v#"}},
        {"Players Destroyed", "Players\nDestroyed", "Other", "progress", "players_destroyed.png"_spr, {}, "9"},
        {"Secret", "Secret", "Other", "distinct", "secret.png"_spr, {"secret##", "secret##b"}},
        {"Misc", "Misc", "Other", "distinct", "misc.png"_spr, {"rate", "moreGames", "facebook", "youtube", "twitter"}},
        {"Steam Exclusive", "Steam\nExclusive", "Other", "distinct", "steam.png"_spr, {"steam##"}}};

    // ─── Parse achievements from GD and assign to categories ───
    auto* am = AchievementManager::sharedState();
    CCArray* array = am->m_allAchievements;
    for (int i = 0; i < array->count(); i++) {
        auto item = array->objectAtIndex(i);
        auto dict = typeinfo_cast<CCDictionary*>(item);
        if (!dict) continue;

        Achievement* ach = new Achievement();
        ach->id = std::string(dict->valueForKey("identifier")->getCString());

        std::string achievedDesc = std::string(dict->valueForKey("achievedDescription")->getCString());
        if (betterDescriptions.contains(ach->id)) {
            ach->unachievedDescription = betterDescriptions[ach->id].first;
            ach->achievedDescription = betterDescriptions[ach->id].second;
        } else {
            ach->achievedDescription = achievedDesc;
            ach->unachievedDescription = std::string(dict->valueForKey("unachievedDescription")->getCString());
        }

        Category* cat = nullptr;
        for (auto& c : s_achievementCategories) {
            // Extract third dot-segment and replace digits with '#'
            std::string generic;
            size_t d1 = ach->id.find('.');
            size_t d2 = (d1 != std::string::npos) ? ach->id.find('.', d1 + 1) : std::string::npos;
            if (d2 != std::string::npos) {
                size_t d3 = ach->id.find('.', d2 + 1);
                std::string seg = ach->id.substr(d2 + 1,
                    (d3 != std::string::npos ? d3 : ach->id.size()) - d2 - 1);
                for (char ch : seg) generic += (ch >= '0' && ch <= '9') ? '#' : ch;
            }

            if (generic.find("shard") != std::string::npos && c.name == "Shards") { cat = &c; break; }
            if (generic == "path##" && c.name == "Paths") { cat = &c; break; }

            for (const std::string& idPat : c.identifiers) {
                if (generic == idPat) {
                    if (c.name == "Secret" && achievedDesc.find("Destroyed") != std::string::npos) {
                        // Assign to Players Destroyed
                        for (auto& c2 : s_achievementCategories) {
                            if (c2.name == "Players Destroyed") { cat = &c2; break; }
                        }
                        if (cat) break;
                    }
                    if (c.name == "Secret" && achievedDesc.find("Vault") != std::string::npos) {
                        for (auto& c2 : s_achievementCategories) {
                            if (c2.name == "Vaults") { cat = &c2; break; }
                        }
                        if (cat) break;
                    }
                    cat = &c;
                    break;
                }
            }
            if (cat) break;
        }

        if (!cat) {
            delete ach;
            continue;
        }

        ach->unlockValue = (cat->displayType == "progress" || cat->displayType == "shard")
            ? extractValue(ach->achievedDescription) : -1;

        std::string icon = std::string(dict->valueForKey("icon")->getCString());
        size_t pos = icon.find('_');
        if (pos != std::string::npos) {
            ach->unlockID = stoi(icon.substr(pos + 1));
            ach->unlockType = ::unlockTypeFromString(icon.substr(0, pos));
        } else {
            ach->unlockID = -1;
            ach->unlockType = UnlockType::GJItem;
        }

        cat->achievements.push_back(ach);
    }
}

// entry condition: description is from achievement that is part of a progress category
int extractValue(const std::string& desc) {
    std::string num;
    bool inNumber = false;
    for (char c : desc) {
        if (c >= '0' && c <= '9') {
            num += c;
            inNumber = true;
        } else if (inNumber && c == ',') {
            // skip thousand separators
        } else if (inNumber) {
            break;
        }
    }
    if (num.empty()) return 1;
    return numFromString<int>(num).unwrapOr(1);
}

GJItemIcon* createAchievementIcon(const Achievement* ach, bool earned, bool usePlayerColors) {
    static const std::vector<UnlockType> playerUnlockTypes = {
        UnlockType::Cube, UnlockType::Ship, UnlockType::Ball, UnlockType::Bird,
        UnlockType::Dart, UnlockType::Robot, UnlockType::Spider, UnlockType::Swing, UnlockType::Jetpack
    };

    if (earned) {
        bool isIcon = std::find(playerUnlockTypes.begin(), playerUnlockTypes.end(), ach->unlockType) != playerUnlockTypes.end();
        GJItemIcon* icon;
        if (usePlayerColors) {
            icon = GJItemIcon::create(ach->unlockType, ach->unlockID,
                gameManager->colorForIdx(gameManager->getPlayerColor()),
                gameManager->colorForIdx(gameManager->getPlayerColor2()),
                isIcon, false, false, gameManager->colorForIdx(gameManager->getPlayerGlowColor()));
            if (gameManager->m_playerGlow) {
                for (auto child : CCArrayExt(icon->getChildren())) {
                    if (auto spr = typeinfo_cast<SimplePlayer*>(child)) {
                        spr->setGlowOutline(gameManager->colorForIdx(gameManager->getPlayerGlowColor()));
                    }
                }
            }
        } else {
            icon = GJItemIcon::create(ach->unlockType, ach->unlockID,
                {175, 175, 175}, {255, 255, 255}, isIcon, false, false, {255, 255, 255});
        }
        return icon;
    } else {
        return GJItemIcon::createBrowserItem(ach->unlockType, ach->unlockID);
    }
}

CCMenuItemSpriteExtra* createAchievementIconButton(
    Achievement* ach,
    bool earned,
    bool usePlayerColors,
    CCObject* target,
    SEL_MenuHandler selector,
    const std::string& tag)
{
    GJItemIcon* unlockItem = createAchievementIcon(ach, earned, usePlayerColors);

    if (!earned) {
        auto* lock = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
        lock->setID("lock-" + tag);
        lock->setZOrder(1);
        lock->setPosition({unlockItem->getContentWidth() / 2.f, unlockItem->getContentHeight() / 2.f});
        unlockItem->addChild(lock);
    }

    unlockItem->setID("item-" + tag);

    auto* button = CCMenuItemSpriteExtra::create(unlockItem, target, selector);
    button->setID(tag.empty() ? "unlock-sprite" : "unlock-sprite-" + tag);
    button->m_baseScale = 0.7f;
    button->setScale(0.7f);

    auto* data = new IconCallbackData(ach->unlockType, ach->unlockID, ach->achievedDescription);
    data->autorelease();
    button->setUserObject(data);

    return button;
}

GJItemIcon* createJumpsIcon() {
    GJItemIcon* icon = GJItemIcon::create(UnlockType::Cube, gameManager->getPlayerFrame(),
        gameManager->colorForIdx(gameManager->getPlayerColor()),
        gameManager->colorForIdx(gameManager->getPlayerColor2()),
        true, false, false, gameManager->colorForIdx(gameManager->getPlayerGlowColor()));

    if (gameManager->m_playerGlow) {
        for (auto child : CCArrayExt(icon->getChildren())) {
            if (auto spr = typeinfo_cast<SimplePlayer*>(child)) {
                spr->setGlowOutline(gameManager->colorForIdx(gameManager->getPlayerGlowColor()));
            }
        }
    }

    icon->setRotation(50.f);
    return icon;
}

// Find the next unearned milestone value in a progress category.
int getNextMilestone(const Category& cat, int currentValue) {
    int next = -1;
    for (const auto* ach : cat.achievements) {
        int val = ach->unlockValue;
        if (val > currentValue) {
            if (next == -1 || val < next)
                next = val;
        }
    }
    if (next != -1) return next;
    return currentValue; // all milestones reached
}

// ──── Category tracking (persisted via GameManager) ────
// Uses GameManager::getGameVariable/setGameVariable so tracking state
// is automatically saved in GD's own save file.

static const std::string TRACKING_PREFIX = "at_track_";

bool isCategoryTracked(const std::string& categoryName) {
    return GameManager::get()->getGameVariable((TRACKING_PREFIX + categoryName).c_str());
}

void setCategoryTracked(const std::string& categoryName, bool tracked) {
    GameManager::get()->setGameVariable((TRACKING_PREFIX + categoryName).c_str(), tracked);
    log::info("Tracking: '{}' set to {}", categoryName, tracked);
}

void migrateTrackingData() {
    // One-time migration from old Mod::get()->setSavedValue JSON format
    // to GameManager::setGameVariable (persists in GD save).
    auto json = Mod::get()->getSavedValue<matjson::Value>(
        "tracking-data", matjson::Value::object()
    );

    if (!json.isObject() || !json.contains("categories")) return;

    auto& catVal = json["categories"];
    if (!catVal.isObject()) return;

    int migrated = 0;
    for (auto& [name, val] : catVal) {
        if (val.isBool()) {
            setCategoryTracked(name, val.asBool().unwrap());
            migrated++;
        }
    }

    if (migrated > 0) {
        // Clear old data
        Mod::get()->setSavedValue<matjson::Value>("tracking-data", matjson::Value::object());
        log::info("Migrated {} tracking categories from old JSON format to GameManager", migrated);
    }
}

// Count how many achievements in the list are earned
int countEarnedAchievements(const std::vector<Achievement*>& achievements) {
    if (achievements.empty()) return 0;

    // Fast path: use GD's native batch check for the common case
    auto* arr = CCArray::create();
    for (const auto* ach : achievements) {
        arr->addObject(CCString::create(ach->id));
    }
    if (achievementManager->areAchievementsEarned(arr))
        return (int)achievements.size();

    // Slow path: count individually
    int count = 0;
    for (const auto* ach : achievements) {
        if (achievementManager->isAchievementEarned(ach->id.c_str()))
            count++;
    }
    return count;
}

// ──── Progress bar background helper ────
CCNode* buildProgressBarBg(const ProgressBarBgParams& params) {
    CCNode* progressBarBg = CCNode::create();
    progressBarBg->setID("progress-bar-bg");
    progressBarBg->setPosition({0, 0});

    // Background bar sprite
    CCSprite* progressBarBgSpr = CCSprite::createWithSpriteFrameName("whiteSquare20_001.png");
    progressBarBgSpr->setID("progress-bar-bg-sprite");
    progressBarBgSpr->setScaleX(params.dotSpacing / 10 * params.numDots - params.dotSpacing / 10);
    progressBarBgSpr->setScaleY(0.5f);
    progressBarBgSpr->setPosition({0, 0});
    progressBarBgSpr->setColor({37, 20, 12});
    progressBarBg->addChild(progressBarBgSpr);

    float posBase = -params.dotSpacing * params.numIconsOnPage / 2.f;

    for (int i = 0; i < params.numDots; ++i) {
        float xPos = posBase + params.dotSpacing * i + params.dotOffset;

        // Dot
        CCSprite* dotBgSpr = CCSprite::create("smallDot.png");
        dotBgSpr->setID("dot-bg-sprite-" + std::to_string(i));
        dotBgSpr->setPosition({xPos, 0});
        dotBgSpr->setColor({37, 20, 12});
        progressBarBg->addChild(dotBgSpr);

        // Vertical connector bar
        if (params.skipFirstVerticalBar && i == 0) continue;

        CCSprite* verticalBarBgSpr = CCSprite::createWithSpriteFrameName("whiteSquare20_001.png");
        verticalBarBgSpr->setID("vertical-bar-sprite-" + std::to_string(i));

        bool barAbove;
        if (params.invertVerticalBarAnchors)
            barAbove = (i % 2 != 0);
        else
            barAbove = (i % 2 == 0);

        if (barAbove) {
            verticalBarBgSpr->setAnchorPoint({0.5f, 1});
            verticalBarBgSpr->setPosition({xPos, -10.f});
        } else {
            verticalBarBgSpr->setAnchorPoint({0.5f, 0});
            verticalBarBgSpr->setPosition({xPos, 10.f});
        }

        verticalBarBgSpr->setScaleX(0.1f);
        verticalBarBgSpr->setScaleY(1.5f);
        verticalBarBgSpr->setColor({37, 20, 12});
        verticalBarBgSpr->setOpacity(50);
        progressBarBg->addChild(verticalBarBgSpr);
    }

    return progressBarBg;
}

// ──── Progress bar fill helper ────
CCNode* buildProgressFill(
    int numDotsToFill,
    int numDotsOnPage,
    const std::vector<Achievement*>& achievements,
    int pageStartIndex,
    float statValue,
    const ProgressFillParams& params,
    bool globalProgress)
{
    CCNode* fill = CCNode::create();
    fill->setID("progress-bar-fill");
    fill->setPosition({0, 0});

    float xBase = -params.dotSpacing * params.numIconsOnPage / 2.f;

    // Create colored dots for unlocked achievements
    for (int i = 0; i < numDotsToFill; ++i) {
        auto* dot = CCSprite::create("smallDot.png");
        dot->setID("dot-fill-sprite-" + std::to_string(i));
        dot->setPosition({xBase + params.dotSpacing * i, 0});
        dot->setColor(params.fillColor);
        dot->setScale(0.7f);
        fill->addChild(dot);
    }

    // Create per-segment fill bars
    for (int i = 0; i < numDotsOnPage - 1; ++i) {
        Achievement* curr = achievements[i + pageStartIndex];

        float ratio;
        bool isFirst = globalProgress ? (pageStartIndex + i == 0) : (i == 0);
        if (isFirst)
            ratio = std::min(1.f, float(statValue) / curr->unlockValue);
        else
            ratio = std::min(1.f, float(statValue - achievements[i - 1 + pageStartIndex]->unlockValue)
                / (curr->unlockValue - achievements[i - 1 + pageStartIndex]->unlockValue));

        if (ratio < 0.f) break;

        auto* fillSpr = CCSprite::createWithSpriteFrameName("whiteSquare20_001.png");
        fillSpr->setID("progress-bar-fill-sprite-" + std::to_string(i));
        fillSpr->setAnchorPoint({0, 0.5f});
        fillSpr->setPosition({xBase + params.dotSpacing * i, 0});
        fillSpr->setColor(params.fillColor);
        fillSpr->setScaleX(params.dotSpacing / 10 * ratio);
        fillSpr->setScaleY(0.2f);
        fill->addChild(fillSpr);
    }

    return fill;
}
