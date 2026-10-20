#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// Global tracking for start positions
static std::vector<StartPosObject*> g_startPositions;
static int g_currentIndex = 0;

class $modify(MyPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool p1, bool p2) {
        if (!PlayLayer::init(level, p1, p2)) return false;

        g_startPositions.clear();
        g_currentIndex = 0;

        // Gather all objects in the level
        if (auto objects = m_objects) {
            for (auto obj : CCArrayExt<GameObject*>(objects)) {
                if (auto startPos = typeinfo_cast<StartPosObject*>(obj)) {
                    g_startPositions.push_back(startPos);
                }
            }
        }

        // Sort start positions by their X coordinate (left to right)
        std::sort(g_startPositions.begin(), g_startPositions.end(), [](StartPosObject* a, StartPosObject* b) {
            return a->getPositionX() < b->getPositionX();
        });

        return true;
    }
};

class $modify(MobileSPPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // Create UI menu on the pause layer
        auto menu = CCMenu::create();
        menu->setPosition(30.f, winSize.height / 2);
        menu->setContentSize({60.f, 100.f});
        menu->setLayout(
            ColumnLayout::create()
                ->setGap(10.f)
                ->setAxisAlignment(AxisAlignment::Center)
        );

        // Previous SP Button
        auto prevSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        auto prevBtn = CCMenuItemSpriteExtra::create(
            prevSprite,
            this,
            menu_selector(MobileSPPauseLayer::onPrevSP)
        );

        // Next SP Button
        auto nextSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        nextSprite->setFlipX(true);
        auto nextBtn = CCMenuItemSpriteExtra::create(
            nextSprite,
            this,
            menu_selector(MobileSPPauseLayer::onNextSP)
        );

        menu->addChild(prevBtn);
        menu->addChild(nextBtn);
        menu->updateLayout();

        this->addChild(menu, 100);
    }

    void onPrevSP(CCObject*) {
        if (g_startPositions.empty()) {
            Notification::create("No Start Positions found!", CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png"))->show();
            return;
        }

        g_currentIndex--;
        if (g_currentIndex < 0) {
            g_currentIndex = g_startPositions.size() - 1; // Loop to last
        }

        switchToStartPos();
    }

    void onNextSP(CCObject*) {
        if (g_startPositions.empty()) {
            Notification::create("No Start Positions found!", CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png"))->show();
            return;
        }

        g_currentIndex++;
        if (g_currentIndex >= g_startPositions.size()) {
            g_currentIndex = 0; // Loop back to first
        }

        switchToStartPos();
    }

    void switchToStartPos() {
        if (auto playLayer = PlayLayer::get()) {
            auto targetSP = g_startPositions[g_currentIndex];
            
            // Set current start position in PlayLayer and restart level from there
            playLayer->m_startPos = targetSP;
            playLayer->resetLevel();
            
            // Close the pause menu so you instantly resume playing
            this->onResumed(nullptr);
            
            log::info("Switched to Start Position index: {}", g_currentIndex);
        }
    }
};
