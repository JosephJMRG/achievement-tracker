#include "TrackingManager.hpp"

using namespace geode::prelude;

TrackingManager* TrackingManager::get() {
    static TrackingManager instance;
    static bool loaded = false;
    if (!loaded) {
        instance.load();
        loaded = true;
    }
    return &instance;
}

// --- Category tracking ---

bool TrackingManager::isCategoryTracked(const std::string& categoryName) const {
    auto it = m_categoryTracking.find(categoryName);
    return it != m_categoryTracking.end() && it->second;
}

void TrackingManager::setCategoryTracked(const std::string& categoryName, bool tracked) {
    m_categoryTracking[categoryName] = tracked;
    save();
}

void TrackingManager::setCategoryTrackedBatch(const std::string& categoryName, bool tracked) {
    m_categoryTracking[categoryName] = tracked;
    save();
}

// --- Persistence ---

void TrackingManager::save() {
    auto json = matjson::Value::object();

    // Categories
    auto categories = matjson::Value::object();
    for (const auto& [name, tracked] : m_categoryTracking) {
        categories[name] = tracked;
    }
    json["categories"] = categories;

    Mod::get()->setSavedValue<matjson::Value>("tracking-data", json);
    log::info("TrackingManager::save() -> {} categories saved", categories.size());
}

void TrackingManager::load() {
    m_categoryTracking.clear();

    auto json = Mod::get()->getSavedValue<matjson::Value>(
        "tracking-data", matjson::Value::object()
    );

    if (!json.isObject()) {
        log::warn("TrackingManager: saved data is not an object, resetting");
        return;
    }

    // Load categories
    if (json.contains("categories")) {
        auto& catVal = json["categories"];
        if (catVal.isObject()) {
            for (auto& [name, val] : catVal) {
                if (val.isBool()) {
                    m_categoryTracking[name] = val.asBool().unwrap();
                }
            }
        }
    }

    log::info("TrackingManager::load() -> {} categories loaded", m_categoryTracking.size());
}
