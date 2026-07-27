#ifndef ACHIEVEMENTCATEGORYPOPUP_HPP
#define ACHIEVEMENTCATEGORYPOPUP_HPP

#include <Geode/Geode.hpp>

#include "../Utils.hpp"

class AchievementMenu;
class AchievementCategoryPopup : public geode::Popup {
   public:
    void createTitle();
    void addProgressText(int statValue, int goalValue);
    void addLogo();
    void addCornerSprites();
    void addNavigation();

    virtual cocos2d::CCNode* createPage(int pageNum) = 0;

    void onIcon(CCObject* sender);
    virtual void onNavButton(CCObject* sender);
    virtual void onArrow(CCObject* sender);
    void onClose(CCObject* sender) override;

    // Tracking row helper (shared across all popups)
    void addTrackingRow();
    void onToggleCategoryTracked(CCObject* sender);

    int m_maxIconsPerPage;
    int m_numPages;
    Category* m_category;
    int m_numAchievements;

    cocos2d::CCMenu* m_navMenu = nullptr;
    cocos2d::CCMenu* m_navButtons = nullptr;

    AchievementMenu* m_achievementMenu;
};

#endif