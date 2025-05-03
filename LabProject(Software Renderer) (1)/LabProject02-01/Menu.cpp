#include "stdafx.h"
#include "3DGP_variableMenu.h"
#include "Scene.h"
#include "Menu.h"
#include "GraphicsPipeline.h"

Menu::Menu(CPlayer* pPlayer) : CScene(pPlayer)
{
	m_pPlayer = pPlayer;
}

Menu::~Menu()
{
}

void Menu::BuildObjects()
{
	CExplosiveObject::PrepareExplosion();

	std::array<CMesh*, 5> pMenuMesh; 
	std::array<COLORREF, 3> color{ 0, 0, 0 };

	pMenuMesh[0] = new CTextMesh(1.0f, 1.0f, 1.0f, 1, color, tutorialText, LocationX, LocationY);
	pMenuMesh[1] = new CTextMesh(1.0f, 1.0f, 1.0f, 2, color, level1Text, LocationX, LocationY);
	pMenuMesh[2] = new CTextMesh(1.0f, 1.0f, 1.0f, 3, color, level2Text, LocationX, LocationY);
	pMenuMesh[3] = new CTextMesh(1.0f, 1.0f, 1.0f, 2, color, startText, LocationX, LocationY);
	pMenuMesh[4] = new CTextMesh(1.0f, 1.0f, 1.0f, 1, color, endText, LocationX, LocationY);

	m_nObjects = 5;

	m_ppObjects = new CGameObject * [m_nObjects];

	for (int i = 0; i < m_nObjects; ++i) {
		m_ppObjects[i] = new CExplosiveObject();
		m_ppObjects[i]->SetMesh(pMenuMesh[i]);

		m_ppObjects[i]->SetColor(RGB(0, 0, 0));

		m_ppObjects[i]->SetPosition(0.0f, 10.0f + (i * -10.f), 50.f);

		m_ppObjects[i]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_ppObjects[i]->SetRotationSpeed(0.0f);
		m_ppObjects[i]->SetMovingDirection(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_ppObjects[i]->SetMovingSpeed(0.0f);

		pMenuMesh[i]->Release();
	}


#ifdef _WITH_DRAW_AXIS
	m_pWorldAxis = new CGameObject();
	CAxisMesh* pAxisMesh = new CAxisMesh(0.5f, 0.5f, 0.5f);
	m_pWorldAxis->SetMesh(pAxisMesh);
#endif
}

void Menu::ReleaseObjects()
{
	if (CExplosiveObject::m_pExplosionMesh) CExplosiveObject::m_pExplosionMesh->Release();

	for (int i = 0; i < m_nObjects; i++) if (m_ppObjects[i]) delete m_ppObjects[i];
	if (m_ppObjects) delete[] m_ppObjects;

#ifdef _WITH_DRAW_AXIS
	if (m_pWorldAxis) delete m_pWorldAxis;
#endif
}

void Menu::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}

void Menu::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			CExplosiveObject* pExplosiveObject = (CExplosiveObject*)m_ppObjects[int(wParam - '1')];
			pExplosiveObject->m_bBlowingUp = true;
			break;
		}
		case 'A':
			for (int i = 0; i < m_nObjects; i++)
			{
				CExplosiveObject* pExplosiveObject = (CExplosiveObject*)m_ppObjects[i];
				pExplosiveObject->m_bBlowingUp = true;
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void Menu::Animate(float fElapsedTime)
{
	for (int i = 0; i < m_nObjects; i++) m_ppObjects[i]->Animate(fElapsedTime);

	//CheckObjectByObjectCollisions();

	//CheckObjectByBulletCollisions();
}

void Menu::Render(HDC hDCFrameBuffer, CCamera* pCamera)
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
