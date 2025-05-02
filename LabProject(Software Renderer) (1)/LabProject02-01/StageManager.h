#pragma once
#include "Scene.h"

class StageManager
{
private:
	bool ready = false;
	CScene* currStage;

public:
	StageManager(CScene* stage);
	~StageManager(){}

	void set(CScene* stage);
};

