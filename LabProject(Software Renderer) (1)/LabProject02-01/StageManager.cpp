#include "Scene.h"
#include "StageManager.h"

StageManager::StageManager(CScene* stage) 
{
	currStage = stage;
}

void StageManager::set(CScene* stage)
{
	currStage = stage;
}