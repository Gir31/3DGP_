#pragma once
#include "Scene.h"

class StageManager
{
private:
	std::array<CScene*, 4> stageArr{};

	int currLevel = 0;
	int nextLevel = 0;

	bool ready = true;
	bool change = false;

	float elapsedTime = 0.f;

public:
	StageManager(CScene* stage1, CScene* stage2, CScene* stage3, CScene* stage4);
	~StageManager();

	void setReady(bool rflag);
	void setNextLevel(int level);
	bool getReady();
	int getCurrLevel();
	 
	void buildStage();
	void releaseStage();
	void changeStage(int nextStage);
	void waitTime(float fElapsedTime);

	void show();

	CScene* getCurrStage();
};

