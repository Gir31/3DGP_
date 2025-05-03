#include "stdafx.h"
#include "Scene.h"
#include "GraphicsPipeline.h"
#include "Stage2.h"

Stage2::Stage2(CPlayer* pPlayer) : CScene(pPlayer)
{
	m_pPlayer = pPlayer;
}

Stage2::~Stage2()
{
}

void Stage2::BuildObjects()
{
	CExplosiveObject::PrepareExplosion();

	/*CTextMesh* pTitleMesh = new CTextMesh(1.0f, 1.0f, 1.0f, 0, title, titleLocationX, titleLocationY);
	CTextMesh* pNameMesh = new CTextMesh(1.0f, 1.0f, 1.0f, 1, name, nameLocationX, nameLocationY);

	m_nObjects = 2;
	m_ppObjects = new CGameObject * [m_nObjects];

	m_ppObjects[0] = new CExplosiveObject();
	m_ppObjects[0]->SetMesh(pNameMesh);

	m_ppObjects[0]->SetColor(RGB(0, 0, 0));

	m_ppObjects[0]->SetPosition(0.0f, -10.0f, 50.f);

	m_ppObjects[0]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_ppObjects[0]->SetRotationSpeed(90.0f);
	m_ppObjects[0]->SetMovingDirection(XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_ppObjects[0]->SetMovingSpeed(0.0f);

	pNameMesh->Release();

	m_ppObjects[1] = new CExplosiveObject();
	m_ppObjects[1]->SetMesh(pTitleMesh);

	m_ppObjects[1]->SetColor(RGB(0, 0, 0));

	m_ppObjects[1]->SetPosition(0.0f, 0.0f, 50.0f);

	m_ppObjects[1]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_ppObjects[1]->SetRotationSpeed(90.0f);
	m_ppObjects[1]->SetMovingDirection(XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_ppObjects[1]->SetMovingSpeed(0.0f);

	pTitleMesh->Release();*/

#ifdef _WITH_DRAW_AXIS
	m_pWorldAxis = new CGameObject();
	CAxisMesh* pAxisMesh = new CAxisMesh(0.5f, 0.5f, 0.5f);
	m_pWorldAxis->SetMesh(pAxisMesh);
#endif
}

void Stage2::ReleaseObjects()
{
	if (CExplosiveObject::m_pExplosionMesh) CExplosiveObject::m_pExplosionMesh->Release();

	for (int i = 0; i < m_nObjects; i++) if (m_ppObjects[i]) delete m_ppObjects[i];
	if (m_ppObjects) delete[] m_ppObjects;

#ifdef _WITH_DRAW_AXIS
	if (m_pWorldAxis) delete m_pWorldAxis;
#endif
}

void Stage2::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}

void Stage2::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void Stage2::Animate(float fElapsedTime)
{
	for (int i = 0; i < m_nObjects; i++) m_ppObjects[i]->Animate(fElapsedTime);
}

void Stage2::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGraphicsPipeline::SetViewport(&pCamera->m_Viewport);

	CGraphicsPipeline::SetViewPerspectiveProjectTransform(&pCamera->m_xmf4x4ViewPerspectiveProject);

	for (int i = 0; i < m_nObjects; i++) m_ppObjects[i]->Render(hDCFrameBuffer, pCamera);

	if (m_pPlayer) m_pPlayer->Render(hDCFrameBuffer, pCamera);

	//UI
#ifdef _WITH_DRAW_AXIS
	CGraphicsPipeline::SetViewOrthographicProjectTransform(&pCamera->m_xmf4x4ViewOrthographicProject);
	m_pWorldAxis->SetRotationTransform(&m_pPlayer->m_xmf4x4World);
	m_pWorldAxis->Render(hDCFrameBuffer, pCamera);
#endif
}
