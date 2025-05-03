#pragma once
#include "stdafx.h"
#include "GameObject.h"
#include "Camera.h"
#include "Player.h"
#include "Scene.h"

class Stage2 : public CScene
{
public:
	Stage2(CPlayer* pPlayer);
	virtual ~Stage2();

public:
	void BuildObjects() override;
	void ReleaseObjects() override;

	void Animate(float fElapsedTime) override;
	void Render(HDC hDCFrameBuffer, CCamera* pCamera) override;

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
};

