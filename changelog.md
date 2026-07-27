# Changelog

## v3.0.3

### Changed
- Added "Why this mod?" section to README and about page

## v3.0.2

### Changed
- Removed unused `getAchievementLimit()` and `getAchievementPercent()` functions
- Removed unused `unlockedDescription` field from icon callback data
- Consolidated `playerUnlockTypes` vector into a single shared definition
- Moved `unlockTypeFromString()` to Utils (was exposed in ProgressCalculator header unnecessarily)
- Extracted navigation arrows and page-switch logic into shared base class helpers, eliminating duplication across PathPopup, ShardPopup, and AchievementCategoryPopup

## v3.0.1

### Fixed
- **Category popup UI restored**: Main Levels, Tower Levels, Geometry Dash Meltdown/World/SubZero, Friends, Creator, Vaults, Secret, Misc, and Steam Exclusive categories now display their original paginated tile grid layout with icon previews, descriptions, and completion checkmarks
- Removed unused `AchievementList` module that replaced the tile grid during the refactor

## v3.0.0

### Changed
- **Major refactor complete**: all internal subsystems now use native GD APIs and shared utility functions
- **Over 1,100 lines removed** across the entire codebase
- Verified complete coverage of all 570 GD achievements across every category definition
- **Dead code removed**: unused stat limit/percent calculations cleaned from ProgressCalculator

## v2.7.0

### Changed
- **Toast system rebuilt from scratch**: new `TrackingToast` with animated progress bars, percentage counters, stat labels, and slide-in/out transitions
- **Toast persistence fixed**: uses GD's own overlay layer to survive scene transitions reliably, replacing previous fragile reparenting logic
- **Stat toasts are now silent**: achievement sounds play only through GD's native achievement system; stat progress notifications display visually without audio

### Removed
- Deferred toast scheduling system (no longer needed with overlay-based persistence)
- Previous reparent-based scene transition handling

## v2.6.0

### Changed
- **Achievement list rebuilt** using `BoomListView` + native `AchievementCell` for consistent visual integration with the base game
- **Path, Shard, and Progress popups simplified**: shared helper utilities eliminate duplicated layout and rendering code across all popups
- **Achievement icon rendering consolidated** into a single reusable builder that handles label badges, earned/unearned states, and corner decorations
- **Progress bar rendering unified** across all popups with consistent sizing, color tinting, and inset fill behavior

## v2.5.0

### Changed
- **Achievement detection** now leverages GD's built-in achievement validation pipeline instead of custom stat-matching logic
- **Notification pipeline** replaced with GD's native `AchievementNotifier` for achievement completion toasts
- **Custom notification system removed**: all notification logic consolidated into existing GD mechanisms

### Removed
- Legacy `NotificationSystem` module (fully replaced by native APIs)
- Duplicate stat validation code

## v2.4.0

### Added
- **Stat progress toasts**: shows notification with animated progress bar when leaving or completing a level, for any tracked stat category
- **Real-time stat tracking**: hooks `GameStatsManager::incrementStat` to capture stat changes during gameplay (jumps, attempts, stars, coins, diamonds, etc.)
- **Toast persistence across scene transitions**: reuses GD's own `AchievementNotifier` mechanism (`willSwitchToScene`) to keep toasts alive when navigating between screens
- **GJGameLevel fallback**: uses per-level data (m_jumps, m_attempts) as backup for stats that GD doesn't commit to GameStatsManager during gameplay
- **Debug toast buttons** (TOAST / x3) on main menu for testing notification display
- **Deferred toast system**: toasts accumulate during gameplay and flush when the player returns to the menu

### Fixed
- **Crash on ESC spam**: `executeReparent` now checks toast node validity (`getActionManager()`) before operating; dead nodes are recreated at final state via `resumeDeadToast` instead of crashing
- **Achievement duplication in menu**: each achievement was shown twice because `AchievementMenu::init()` re-parsed all achievements on top of the already-populated copy from `buildSharedCategories()`
- **Memory leak in toast reparent**: scene reference was not released when a new reparent superseded a pending one
- **Duplicate toast on levelComplete**: `onQuit` and `levelComplete` could both fire accumulation; now guarded by `s_toastsAlreadyFlushed` flag
- **Stale pointer crash after scene transitions**: `clearStaleState()` resets tracking without dereferencing dangling pointers
- **Infinite scheduler loop**: toast flush helper was rescheduling itself endlessly; fixed with proper one-shot scheduling
- **Toast not appearing on first attempt**: `backfillFromLevelStats()` now reads GJGameLevel deltas to catch stats that `incrementStat` misses on the first attempt

## v2.1.0 (2026-07-20)

### Added
- New pill-shaped progress bars on tracking cards using `CCLayerColor` with inset green fill
- Z-ordering for progress bar (z=10) and fraction text (z=20) to layer correctly

### Changed
- **about.md**: Rewritten to describe Achievement Tracker (fork) instead of original mod
- **Tracking page layout**: Cards now feature a 2-column quest-card layout (Incomplete / Completed)
- **Checkbox**: Moved to top-right corner (8px from edge, `cardH - 9`)
- **Percentage & "Completed"**: Moved 15px right (under checkbox), raised 10px (`pctY` +10)
- **"Completed" label**: Reduced gap to percentage text (18px → 10px → 12px)
- **Card title font size**: Scale increased 0.40 → 0.65
- **Card subtitle font size**: Scale increased 0.22 → 0.47
- **Card title/subtitle Y**: Both moved down 5px
- **Progress bar**: Reworked from CCSprite → CCDrawNode → CCLayerColor for reliable rendering; height 11→13px; width reduced 20px
- **version**: `mod.json` bumped from v2.0.0 to v2.1.0
