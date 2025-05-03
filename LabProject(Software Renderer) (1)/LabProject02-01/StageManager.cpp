#include "stdafx.h"
#include "Scene.h"
#include "StageManager.h"

StageManager::StageManager(CScene* stage1, CScene* stage2, CScene* stage3, CScene* stage4)
{
	stageArr[0] = stage1;
	stageArr[1] = stage2;
	stageArr[2] = stage3;
	stageArr[3] = stage4;
} 


void StageManager::setReady(bool rflag)
{
	ready = rflag;
}

void StageManager::setNextLevel(int level)
{
	nextLevel = level;
}

bool StageManager::getReady()
{
	return ready;
}

int StageManager::getCurrLevel()
{
	return currLevel;
}

void StageManager::buildStage()
{ 
	stageArr[currLevel]->BuildObjects();  
}

void StageManager::releaseStage()
{
	stageArr[currLevel]->ReleaseObjects();
}

void StageManager::changeStage(int level)
{
	if (change) {
		stageArr[currLevel]->ReleaseObjects();
		currLevel = level;

		stageArr[currLevel]->BuildObjects();
	}
}

void StageManager::waitTime(float fElapsedTime)
{
	elapsedTime += fElapsedTime;

	if (elapsedTime > 2.f) {
		elapsedTime = 0.f;
		ready = false;
		change = true;

		changeStage(nextLevel);
	}
}

void StageManager::show()
{
	for (const CScene* s : stageArr)
	{
		OutputDebugStringA(typeid(*s).name());
		OutputDebugStringA("\n");
	}
}

CScene* StageManager::getCurrStage()
{
	return stageArr[currLevel]; 
}