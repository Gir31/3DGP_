#pragma once
#include "stdafx.h"
#include "GameObject.h"
#include "Camera.h"
#include "Player.h"
#include "Scene.h"

class Stage1 : public CScene
{
public:
	Stage1(CPlayer* pPlayer);
	virtual ~Stage1();

public:
	void BuildObjects() override;
	void ReleaseObjects() override;

	void Animate(float fElapsedTime) override;
	void Render(HDC hDCFrameBuffer, CCamera* pCamera) override;

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;

	XMFLOAT3 CatmullRom(const XMFLOAT3& p0, const XMFLOAT3& p1, const XMFLOAT3& p2, const XMFLOAT3& p3, float t);
};

