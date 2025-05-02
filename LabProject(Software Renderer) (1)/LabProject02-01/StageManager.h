#pragma once
#include "Scene.h"

class StageManager
{
private:
	std::array<CScene*, 4> stageArr{};

	int currStage = 0;

	bool ready = true;
public:
	StageManager(CScene* stage1, CScene* stage2, CScene* stage3, CScene* stage4);
	~StageManager();
	 
	void buildStage();
	void releaseStage();
	void changeStage(int nextStage);

	void show();

	CScene* getCurrStage();
};

