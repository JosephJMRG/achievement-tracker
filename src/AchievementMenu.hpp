#ifndef ACHIEVEMENTMENU_HPP
#define ACHIEVEMENTMENU_HPP

#include <Geode/Geode.hpp>

#include "Utils.hpp"
#include "TrackingManager.hpp"

class AchievementMenu : public geode::Popup {
   protected:
    bool init() override;

   public:
    static AchievementMenu* create();

    AchievementManager* m_achievementManager;

    std::vector<cocos2d::CCNode*> m_categoriesMenu;
    cocos2d::CCMenu* m_navMenu;
    cocos2d::CCMenu* m_navButtons;
    int m_categoryPage = 0;
    int m_maxCategoriesPerPage = 12;

    void createCategoryMenu();
    void createSummaryPage();
    void createTrackingPage();
    void refreshTrackingPage();
    void doDelayedRefreshTracking(float dt);
    void addCategoryButtons(cocos2d::CCMenu* menuPage, std::string pageTitle, int& totalAchievementsInPage, int& completedAchievementsInPage);
    void onCategoryButton(CCObject* sender);
    void onTrackingToggle(CCObject* sender);

    void addNavigation();
    void onCategoryArrow(CCObject* sender);
    void onNavButton(CCObject* sender);
    void applyPage();

    void addCornerSprites();
    Category* getCategoryForAchievement(const std::string& id, const std::string& achievedDescription);

    // Inline helper: called by AchievementCategoryPopup on close
    void showArrows() {
        if (m_navMenu) {
            m_navMenu->getChildByID("left-arrow")->setVisible(m_categoryPage > 0);
            m_navMenu->getChildByID("right-arrow")->setVisible(m_categoryPage < m_categoriesMenu.size() - 1);
        }
    }

    std::vector<Category> m_achievementCategories;

    int m_trackingPageTag = -1;
};

#endif
