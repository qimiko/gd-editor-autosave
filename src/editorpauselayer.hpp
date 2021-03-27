#pragma once
#ifndef EDITORPAUSELAYER_HPP
#define EDITORPAUSELAYER_HPP

#include <cocos2d.h>

// 0x1C0
class EditorPauseLayer : public cocos2d::CCLayerColor {
	void *protocol_;
	bool levelSaved_;

	void *audioOnBtn_;
	void *audioOffBtn_;
	LevelEditorLayer *levelEditorLayer_;
};

#endif