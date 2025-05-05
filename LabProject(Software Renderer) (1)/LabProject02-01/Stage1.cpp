#include "stdafx.h"
#include "Scene.h"
#include "GraphicsPipeline.h"
#include "Stage1.h"

Stage1::Stage1(CPlayer* pPlayer) : CScene(pPlayer)
{
	m_pPlayer = pPlayer;
}

Stage1::~Stage1()
{
}

std::array<XMFLOAT3, 41> points = {
	XMFLOAT3{0.0f, 0.0f, 0.0f},
	XMFLOAT3{10.0f, 50.0f, 30.0f},
	XMFLOAT3{20.0f, -50.0f, 60.0f},
	XMFLOAT3{10.0f, 70.0f, 90.0f},
	XMFLOAT3{-10.0f, -30.0f, 120.0f},
	XMFLOAT3{-30.0f, 50.0f, 150.0f},
	XMFLOAT3{-10.0f, -10.0f, 180.0f},
	XMFLOAT3{0.0f, 30.0f, 210.0f},
	XMFLOAT3{20.0f, 50.0f, 240.0f},
	XMFLOAT3{40.0f, 50.0f, 270.0f},
	XMFLOAT3{50.0f, -50.0f, 300.0f},
	XMFLOAT3{55.0f, -20.0f, 330.0f},
	XMFLOAT3{40.0f, -30.0f, 360.0f},
	XMFLOAT3{30.0f, 10.0f, 390.0f},
	XMFLOAT3{20.0f, -50.0f, 420.0f},
	XMFLOAT3{10.0f, -20.0f, 450.0f},
	XMFLOAT3{0.0f, -40.0f, 480.0f},
	XMFLOAT3{10.0f, -70.0f, 510.0f},
	XMFLOAT3{0.0f, -100.0f, 540.0f},
	XMFLOAT3{-20.0f, -50.0f, 570.0f},
	XMFLOAT3{-20.0f, -30.0f, 600.0f},
	XMFLOAT3{-10.0f, 0.0f, 630.0f},
	XMFLOAT3{10.0f, 50.0f, 660.0f},
	XMFLOAT3{20.0f, 70.0f, 690.0f},
	XMFLOAT3{30.0f, 80.0f, 720.0f},
	XMFLOAT3{20.0f, 50.0f, 750.0f},
	XMFLOAT3{30.0f, 30.0f, 780.0f},
	XMFLOAT3{20.0f, 50.0f, 810.0f},
	XMFLOAT3{-10.0f, 20.0f, 840.0f},
	XMFLOAT3{0.0f, 0.0f, 870.0f},
	XMFLOAT3{10.0f, -50.0f, 900.0f},
	XMFLOAT3{30.0f, 50.0f, 930.0f},
	XMFLOAT3{20.0f, -10.0f, 960.0f},
	XMFLOAT3{40.0f, 20.0f, 990.0f},
	XMFLOAT3{50.0f, 50.0f, 1020.0f},
	XMFLOAT3{60.0f, 70.0f, 1050.0f},
	XMFLOAT3{70.0f, 80.0f, 1080.0f},
	XMFLOAT3{50.0f, 50.0f, 1110.0f},
	XMFLOAT3{40.0f, 0.0f, 1140.0f},
	XMFLOAT3{30.0f, 0.0f, 1170.0f},
	XMFLOAT3{20.0f, 0.0f, 1200.0f},
};




void Stage1::BuildObjects()
{
	const float railSamplingStep = 0.05f;     // 곡선 보간 간격
	const float railWidth = 0.2f;             // 레일 가로폭
	const float railHeight = 0.2f;            // 레일 높이
	const float deltaT = 0.01f;               // 미분 근사용 Δt

	std::vector<CGameObject*> railObjects;

	for (size_t i = 0; i + 3 < points.size(); ++i)
	{
		const XMFLOAT3& p0 = points[i];
		const XMFLOAT3& p1 = points[i + 1];
		const XMFLOAT3& p2 = points[i + 2];
		const XMFLOAT3& p3 = points[i + 3];

		for (float t = 0.0f; t <= 1.0f; t += railSamplingStep)
		{
			XMFLOAT3 pos = CatmullRom(p0, p1, p2, p3, t);
			float tAhead = (t + deltaT > 1.0f) ? 1.0f : (t + deltaT);
			XMFLOAT3 posAhead = CatmullRom(p0, p1, p2, p3, tAhead);

			XMFLOAT3 dir = Vector3::Normalize(Vector3::Subtract(pos, posAhead)); // pos - posAhead
			float len = Vector3::Length(dir);
			if (len > 0.001f)
			{
				dir = Vector3::Normalize(dir);

				// 회전: dir이 Z축이 되도록
				XMFLOAT4X4 rot = Matrix4x4::LookToLH(XMFLOAT3(0, 0, 0), dir, XMFLOAT3(0, 1, 0));

				// 큐브는 길이 방향이 Z축이므로 Z축 기준 len만큼 설정
				CCubeMesh* pRailMesh = new CCubeMesh(railWidth, railHeight, railSamplingStep * 100.0f); // 길이 보정

				CGameObject* pRail = new CGameObject();
				pRail->SetMesh(pRailMesh);
				pRail->SetPosition(pos); // 중심은 현재 위치로 설정
				pRail->SetRotationTransform(&rot);
				pRail->SetColor(RGB(100, 100, 255));

				railObjects.push_back(pRail);
			}
		}
	}

	m_nObjects = static_cast<int>(railObjects.size());
	m_ppObjects = new CGameObject * [m_nObjects];
	for (int i = 0; i < m_nObjects; ++i)
		m_ppObjects[i] = railObjects[i];

#ifdef _WITH_DRAW_AXIS
	m_pWorldAxis = new CGameObject();
	CAxisMesh* pAxisMesh = new CAxisMesh(0.5f, 0.5f, 0.5f);
	m_pWorldAxis->SetMesh(pAxisMesh);
#endif
}

void Stage1::ReleaseObjects()
{
	if (CExplosiveObject::m_pExplosionMesh) CExplosiveObject::m_pExplosionMesh->Release();

	for (int i = 0; i < m_nObjects; i++) if (m_ppObjects[i]) delete m_ppObjects[i];
	if (m_ppObjects) delete[] m_ppObjects;

#ifdef _WITH_DRAW_AXIS
	if (m_pWorldAxis) delete m_pWorldAxis;
#endif
}

void Stage1::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}

void Stage1::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
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

void Stage1::Animate(float fElapsedTime)
{
	for (int i = 0; i < m_nObjects; i++) m_ppObjects[i]->Animate(fElapsedTime);
}

void Stage1::Render(HDC hDCFrameBuffer, CCamera* pCamera)
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

// Catmull-Rom 보간 함수
XMFLOAT3 Stage1::CatmullRom(const XMFLOAT3& p0, const XMFLOAT3& p1, const XMFLOAT3& p2, const XMFLOAT3& p3, float t)
{
	XMVECTOR v0 = XMLoadFloat3(&p0);
	XMVECTOR v1 = XMLoadFloat3(&p1);
	XMVECTOR v2 = XMLoadFloat3(&p2);
	XMVECTOR v3 = XMLoadFloat3(&p3);

	float t2 = t * t;
	float t3 = t2 * t;

	XMVECTOR result =
		0.5f * (
			(2.0f * v1) +
			(-v0 + v2) * t +
			(2.0f * v0 - 5.0f * v1 + 4.0f * v2 - v3) * t2 +
			(-v0 + 3.0f * v1 - 3.0f * v2 + v3) * t3
			);

	XMFLOAT3 final;
	XMStoreFloat3(&final, result);
	return final;
}
