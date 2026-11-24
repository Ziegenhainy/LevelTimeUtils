#include "Geode/cocos/CCDirector.h"
#include "Geode/ui/Popup.hpp"
#include <Geode/Enums.hpp>
#include <Geode/Geode.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <fmt/format.h>

using namespace geode::prelude;

#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>

class $modify(HookedPlayLayer, PlayLayer) {
	struct Fields {
		int m_tempTimestamp = 0;
		bool m_override_timestamp;
	};

	void onQuit() {
		log::info("Hi im exiting now wheee");
		PlayLayer::onQuit(); 
		if (m_fields->m_override_timestamp) m_level->m_timestamp = m_fields->m_tempTimestamp;
	}

	void levelComplete() {
		PlayLayer::levelComplete();
		m_fields->m_tempTimestamp = m_gameState.m_currentProgress >> 1;
		m_fields->m_override_timestamp = false;
	}

	void onMyButton(CCObject*) {
		// FLAlertLayer::create("Geode", fmt::format("Nya is {}, boobie is {}", this->m_playLayer->m_level->m_timestamp, static_cast<HookedPlayLayer*>(this->m_playLayer)->m_fields->m_tempTimestamp), "OK")->show();
		geode::createQuickPopup(
			"Override Level Length",
			"Are you sure you wanna override the level time?", 
			"Cancel", 
			"Yes",
			[this](auto, bool btn2) {
				if (btn2) {
					log::info("{} {}", m_level->m_dontSave,m_level->m_savedTime);
					if (m_fields->m_tempTimestamp == 0) m_fields->m_tempTimestamp = m_gameState.m_currentProgress >> 1;
					m_level->m_timestamp = m_fields->m_tempTimestamp;
					m_fields->m_override_timestamp = true;
				}
		});
	}
};


class $modify(HookedPauseLayer, PauseLayer) {
	void customSetup() {
		PauseLayer::customSetup();
		if (PlayLayer::get() == nullptr || PlayLayer::get()->m_level->m_levelType != GJLevelType::Editor) return;

		auto myButton = CCMenuItemSpriteExtra::create(
			CCSprite::createWithSpriteFrameName("threadBtn_timeOn_001.png"),
			this,
			menu_selector(HookedPauseLayer::onMyButton)
		);
		auto menu = m_mainLayer->getChildByID("left-button-menu");
		menu->addChild(myButton);
		myButton->setID("level-time-override"_spr);
		menu->updateLayout();
	}

	void onMyButton(CCObject* a) {
		auto playLayer = static_cast<HookedPlayLayer*>(PlayLayer::get());
		playLayer->onMyButton(a);
	}
};

class $modify(HookedEndLevelLayer, EndLevelLayer) {
	// struct Fields {
	// 	int m_finalTime = 
	// }
	void showLayer(bool instant) {
		EndLevelLayer::showLayer(instant);
		if (m_playLayer->m_level->m_levelType != GJLevelType::Editor) return;
		log::debug("Hello from my playLayer::init hook! This layer has {} children.", this->getChildrenCount());

		auto myButton = CCMenuItemSpriteExtra::create(
			CCSprite::createWithSpriteFrameName("threadBtn_timeOn_001.png"),
			this,
			menu_selector(HookedEndLevelLayer::onMyButton)
		);
		auto menu = this->getChildByID("hide-layer-menu");
		menu->addChild(myButton);

		myButton->setID("level-time-override"_spr);
		
		menu->updateLayout();
	}	

	void onMyButton(CCObject* a) {
		static_cast<HookedPlayLayer*>(m_playLayer)->onMyButton(a);
	}
	
};
