#ifndef TRACKINGMANAGER_HPP
#define TRACKINGMANAGER_HPP

#include <Geode/Geode.hpp>
#include "Utils.hpp"

using namespace geode::prelude;

class TrackingManager {
public:
    static TrackingManager* get();

    // --- Category tracking ---
    bool isCategoryTracked(const std::string& categoryName) const;
    void setCategoryTracked(const std::string& categoryName, bool tracked);
    void setCategoryTrackedBatch(const std::string& categoryName, bool tracked);

    // --- Persistence ---
    void save();
    void load();

private:
    TrackingManager() {}

    std::unordered_map<std::string, bool> m_categoryTracking;
};

#endif
