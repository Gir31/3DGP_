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



void StageManager::buildStage()
{ 
	stageArr[currStage]->BuildObjects();  
}

void StageManager::releaseStage()
{
	stageArr[currStage]->ReleaseObjects();
}

void StageManager::changeStage(int nextStage)
{
	if (ready) {
		stageArr[currStage]->ReleaseObjects();
		currStage = nextStage;

		stageArr[currStage]->BuildObjects();
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
	return stageArr[currStage]; 
}