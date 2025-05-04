#include "stdafx.h"
#include "Mesh.h"
#include "GraphicsPipeline.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
CPolygon::CPolygon(int nVertices, COLORREF color)
{
	m_nVertices = nVertices;
	m_pVertices = new CVertex[nVertices];

	polygonColor = color;
}

CPolygon::~CPolygon()
{
	if (m_pVertices) delete[] m_pVertices;
}

void CPolygon::SetVertex(int nIndex, CVertex& vertex)
{
	if ((0 <= nIndex) && (nIndex < m_nVertices) && m_pVertices)
	{
		m_pVertices[nIndex] = vertex;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMesh::CMesh(int nPolygons)
{
	m_nPolygons = nPolygons;
	m_ppPolygons = new CPolygon*[nPolygons];
}

CMesh::~CMesh()
{
	if (m_ppPolygons)
	{
		for (int i = 0; i < m_nPolygons; i++) if (m_ppPolygons[i]) delete m_ppPolygons[i];
		delete[] m_ppPolygons;
	}
}

int CMesh::getTargetStage()
{
	return targetStage;
}

void CMesh::SetPolygon(int nIndex, CPolygon *pPolygon)
{
	if ((0 <= nIndex) && (nIndex < m_nPolygons)) {
		m_ppPolygons[nIndex] = pPolygon;
	}
}

void Draw2DLine(HDC hDCFrameBuffer, XMFLOAT3& f3PreviousProject, XMFLOAT3& f3CurrentProject)
{
	XMFLOAT3 f3Previous = CGraphicsPipeline::ScreenTransform(f3PreviousProject);
	XMFLOAT3 f3Current = CGraphicsPipeline::ScreenTransform(f3CurrentProject);
	::MoveToEx(hDCFrameBuffer, (long)f3Previous.x, (long)f3Previous.y, NULL);
	::LineTo(hDCFrameBuffer, (long)f3Current.x, (long)f3Current.y);
}

void CMesh::Render(HDC hDCFrameBuffer)
{
	/*XMFLOAT3 f3InitialProject, f3PreviousProject;
	bool bPreviousInside = false, bInitialInside = false, bCurrentInside = false, bIntersectInside = false;

	for (int j = 0; j < m_nPolygons; j++)
	{
		int nVertices = m_ppPolygons[j]->m_nVertices;
		CVertex* pVertices = m_ppPolygons[j]->m_pVertices;

		f3PreviousProject = f3InitialProject = CGraphicsPipeline::Project(pVertices[0].m_xmf3Position);
		bPreviousInside = bInitialInside = (-1.0f <= f3InitialProject.x) && (f3InitialProject.x <= 1.0f) && (-1.0f <= f3InitialProject.y) && (f3InitialProject.y <= 1.0f);
		for (int i = 1; i < nVertices; i++)
		{
			XMFLOAT3 f3CurrentProject = CGraphicsPipeline::Project(pVertices[i].m_xmf3Position);
			bCurrentInside = (-1.0f <= f3CurrentProject.x) && (f3CurrentProject.x <= 1.0f) && (-1.0f <= f3CurrentProject.y) && (f3CurrentProject.y <= 1.0f);
			if (((0.0f <= f3CurrentProject.z) && (f3CurrentProject.z <= 1.0f)) && ((bCurrentInside || bPreviousInside))) ::Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3CurrentProject);
			f3PreviousProject = f3CurrentProject;
			bPreviousInside = bCurrentInside;
		}
		if (((0.0f <= f3InitialProject.z) && (f3InitialProject.z <= 1.0f)) && ((bInitialInside || bPreviousInside))) ::Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3InitialProject);
	}*/
	for (int j = 0; j < m_nPolygons; j++) {
		int nVertices = m_ppPolygons[j]->m_nVertices;
		CVertex* pVertices = m_ppPolygons[j]->m_pVertices;

		POINT points[16]; // 꼭짓점 최대 16개로 확장
		for (int i = 0; i < nVertices; ++i) {
			XMFLOAT3 proj = CGraphicsPipeline::ScreenTransform(
				CGraphicsPipeline::Project(pVertices[i].m_xmf3Position)
			);
			points[i] = { (LONG)proj.x, (LONG)proj.y };
		}

		// 펜: 선 색깔 지정
		HPEN hPen = CreatePen(PS_SOLID, 1, m_ppPolygons[j]->polygonColor);
		HPEN hOldPen = (HPEN)SelectObject(hDCFrameBuffer, hPen);

		// 브러시: NULL로 설정하여 내부를 채우지 않음
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hDCFrameBuffer, GetStockObject(NULL_BRUSH));

		// 다각형 외곽선만 그리기
		Polygon(hDCFrameBuffer, points, nVertices);

		// 복원 및 정리
		SelectObject(hDCFrameBuffer, hOldBrush);
		SelectObject(hDCFrameBuffer, hOldPen);
		DeleteObject(hPen);
	}

}

BOOL CMesh::RayIntersectionByTriangle(XMVECTOR& xmRayOrigin, XMVECTOR& xmRayDirection, XMVECTOR v0, XMVECTOR v1, XMVECTOR v2, float* pfNearHitDistance)
{
	float fHitDistance;
	BOOL bIntersected = TriangleTests::Intersects(xmRayOrigin, xmRayDirection, v0, v1, v2, fHitDistance);
	if (bIntersected && (fHitDistance < *pfNearHitDistance)) *pfNearHitDistance = fHitDistance;

	return(bIntersected);
}

int CMesh::CheckRayIntersection(XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection, float* pfNearHitDistance)
{
	int nIntersections = 0;
	bool bIntersected = m_xmOOBB.Intersects(xmvPickRayOrigin, xmvPickRayDirection, *pfNearHitDistance);
	if (bIntersected)
	{
		for (int i = 0; i < m_nPolygons; i++)
		{
			switch (m_ppPolygons[i]->m_nVertices)
			{
			case 3:
			{
				XMVECTOR v0 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[0].m_xmf3Position));
				XMVECTOR v1 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[1].m_xmf3Position));
				XMVECTOR v2 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[2].m_xmf3Position));
				BOOL bIntersected = RayIntersectionByTriangle(xmvPickRayOrigin, xmvPickRayDirection, v0, v1, v2, pfNearHitDistance);
				if (bIntersected) nIntersections++;
				break;
			}
			case 4:
			{
				XMVECTOR v0 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[0].m_xmf3Position));
				XMVECTOR v1 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[1].m_xmf3Position));
				XMVECTOR v2 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[2].m_xmf3Position));
				BOOL bIntersected = RayIntersectionByTriangle(xmvPickRayOrigin, xmvPickRayDirection, v0, v1, v2, pfNearHitDistance);
				if (bIntersected) nIntersections++;
				v0 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[0].m_xmf3Position));
				v1 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[2].m_xmf3Position));
				v2 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[3].m_xmf3Position));
				bIntersected = RayIntersectionByTriangle(xmvPickRayOrigin, xmvPickRayDirection, v0, v1, v2, pfNearHitDistance);
				if (bIntersected) nIntersections++;
				break;
			}
			}
		}
	}
	return(nIntersections);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
CCubeMesh::CCubeMesh(float fWidth, float fHeight, float fDepth) : CMesh(6)
{
	float fHalfWidth = fWidth * 0.5f;
	float fHalfHeight = fHeight * 0.5f;
	float fHalfDepth = fDepth * 0.5f;

	CPolygon *pFrontFace = new CPolygon(4, RGB(0, 0, 0));
	pFrontFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
	SetPolygon(0, pFrontFace);

	CPolygon *pTopFace = new CPolygon(4, RGB(0, 0, 0));
	pTopFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
	pTopFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
	pTopFace->SetVertex(2, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
	pTopFace->SetVertex(3, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
	SetPolygon(1, pTopFace);

	CPolygon *pBackFace = new CPolygon(4, RGB(0, 0, 0));
	pBackFace->SetVertex(0, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(1, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(2, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(3, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
	SetPolygon(2, pBackFace);

	CPolygon *pBottomFace = new CPolygon(4, RGB(0, 0, 0));
	pBottomFace->SetVertex(0, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
	pBottomFace->SetVertex(1, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
	pBottomFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBottomFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
	SetPolygon(3, pBottomFace);

	CPolygon *pLeftFace = new CPolygon(4, RGB(0, 0, 0));
	pLeftFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
	pLeftFace->SetVertex(1, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
	pLeftFace->SetVertex(2, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
	pLeftFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
	SetPolygon(4, pLeftFace);

	CPolygon *pRightFace = new CPolygon(4, RGB(0, 0, 0));
	pRightFace->SetVertex(0, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
	pRightFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
	pRightFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
	pRightFace->SetVertex(3, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
	SetPolygon(5, pRightFace);

	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fHalfWidth, fHalfHeight, fHalfDepth), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
CWallMesh::CWallMesh(float fWidth, float fHeight, float fDepth, int nSubRects) : CMesh((4 * nSubRects * nSubRects) + 2)
{
	float fHalfWidth = fWidth * 0.5f;
	float fHalfHeight = fHeight * 0.5f;
	float fHalfDepth = fDepth * 0.5f;
	float fCellWidth = fWidth * (1.0f / nSubRects);
	float fCellHeight = fHeight * (1.0f / nSubRects);
	float fCellDepth = fDepth * (1.0f / nSubRects);

	int k = 0;
	CPolygon* pLeftFace;
	for (int i = 0; i < nSubRects; i++)
	{
		for (int j = 0; j < nSubRects; j++)
		{
			pLeftFace = new CPolygon(4, RGB(0, 0, 0));
			pLeftFace->SetVertex(0, CVertex(-fHalfWidth, -fHalfHeight + (i * fCellHeight), -fHalfDepth + (j * fCellDepth)));
			pLeftFace->SetVertex(1, CVertex(-fHalfWidth, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + (j * fCellDepth)));
			pLeftFace->SetVertex(2, CVertex(-fHalfWidth, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth)));
			pLeftFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight + (i * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth)));
			SetPolygon(k++, pLeftFace);
		}
	}

	CPolygon* pRightFace;
	for (int i = 0; i < nSubRects; i++)
	{
		for (int j = 0; j < nSubRects; j++)
		{
			pRightFace = new CPolygon(4, RGB(0, 0, 0));
			pRightFace->SetVertex(0, CVertex(+fHalfWidth, -fHalfHeight + (i * fCellHeight), -fHalfDepth + (j * fCellDepth)));
			pRightFace->SetVertex(1, CVertex(+fHalfWidth, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + (j * fCellDepth)));
			pRightFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth)));
			pRightFace->SetVertex(3, CVertex(+fHalfWidth, -fHalfHeight + (i * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth)));
			SetPolygon(k++, pRightFace);
		}
	}

	CPolygon* pTopFace;
	for (int i = 0; i < nSubRects; i++)
	{
		for (int j = 0; j < nSubRects; j++)
		{
			pTopFace = new CPolygon(4, RGB(0, 0, 0));
			pTopFace->SetVertex(0, CVertex(-fHalfWidth + (i * fCellWidth), +fHalfHeight, -fHalfDepth + (j * fCellDepth)));
			pTopFace->SetVertex(1, CVertex(-fHalfWidth + ((i + 1) * fCellWidth), +fHalfHeight, -fHalfDepth + (j * fCellDepth)));
			pTopFace->SetVertex(2, CVertex(-fHalfWidth + ((i + 1) * fCellWidth), +fHalfHeight, -fHalfDepth + ((j + 1) * fCellDepth)));
			pTopFace->SetVertex(3, CVertex(-fHalfWidth + (i * fCellWidth), +fHalfHeight, -fHalfDepth + ((j + 1) * fCellDepth)));
			SetPolygon(k++, pTopFace);
		}
	}

	CPolygon* pBottomFace;
	for (int i = 0; i < nSubRects; i++)
	{
		for (int j = 0; j < nSubRects; j++)
		{
			pBottomFace = new CPolygon(4, RGB(0, 0, 0));
			pBottomFace->SetVertex(0, CVertex(-fHalfWidth + (i * fCellWidth), -fHalfHeight, -fHalfDepth + (j * fCellDepth)));
			pBottomFace->SetVertex(1, CVertex(-fHalfWidth + ((i + 1) * fCellWidth), -fHalfHeight, -fHalfDepth + (j * fCellDepth)));
			pBottomFace->SetVertex(2, CVertex(-fHalfWidth + ((i + 1) * fCellWidth), -fHalfHeight, -fHalfDepth + ((j + 1) * fCellDepth)));
			pBottomFace->SetVertex(3, CVertex(-fHalfWidth + (i * fCellWidth), -fHalfHeight, -fHalfDepth + ((j + 1) * fCellDepth)));
			SetPolygon(k++, pBottomFace);
		}
	}

	CPolygon* pFrontFace = new CPolygon(4, RGB(0, 0, 0));
	pFrontFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
	SetPolygon(k++, pFrontFace);

	CPolygon* pBackFace = new CPolygon(4, RGB(0, 0, 0));
	pBackFace->SetVertex(0, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(1, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(2, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(3, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
	SetPolygon(k++, pBackFace);

	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fHalfWidth, fHalfHeight, fHalfDepth), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAirplaneMesh::CAirplaneMesh(float fWidth, float fHeight, float fDepth) : CMesh(24)
{
	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;

	float x1 = fx * 0.2f, y1 = fy * 0.2f, x2 = fx * 0.1f, y3 = fy * 0.3f, y2 = ((y1 - (fy - y3)) / x1)*x2 + (fy - y3);
	int i = 0;

	//Upper Plane
	CPolygon *pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), -fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(2, CVertex(0.0f, 0.0f, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), -fz));
	pFace->SetVertex(1, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(+x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(+fx, -y3, -fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(-x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, -fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, -fz));
	SetPolygon(i++, pFace);

	//Lower Plane
	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(1, CVertex(0.0f, 0.0f, +fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, +fz));pFace = new CPolygon(3, RGB(0, 0, 0));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(0.0f, 0.0f, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(+x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(-x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(-fx, -y3, +fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, +fz));
	SetPolygon(i++, pFace);

	//Right Plane
	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), -fz));
	pFace->SetVertex(1, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(2, CVertex(+x2, +y2, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(+x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(2, CVertex(+x2, +y2, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(+x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(+x2, +y2, +fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(+fx, -y3, -fz));
	pFace->SetVertex(1, CVertex(+x2, +y2, +fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, +fz));
	SetPolygon(i++, pFace);

	//Back/Right Plane
	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(+fx, -y3, -fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(+fx, -y3, +fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(0.0f, 0.0f, +fz));
	SetPolygon(i++, pFace);

	//Left Plane
	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(1, CVertex(0.0f, +(fy + y3), -fz));
	pFace->SetVertex(2, CVertex(-x2, +y2, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(1, CVertex(-x2, +y2, -fz));
	pFace->SetVertex(2, CVertex(-x2, +y2, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(-x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(-x2, +y2, -fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(-x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(-fx, -y3, -fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, +fz));
	SetPolygon(i++, pFace);

	//Back/Left Plane
	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(0.0f, 0.0f, +fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(-x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(-x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(-fx, -y3, +fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, -fz));
	SetPolygon(i++, pFace);

	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fx, fy, fz), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

CAxisMesh::CAxisMesh(float fWidth, float fHeight, float fDepth) : CMesh(3)
{
	float fHalfWidth = fWidth * 0.5f;
	float fHalfHeight = fHeight * 0.5f;
	float fHalfDepth = fDepth * 0.5f;

	CPolygon* pxAxis = new CPolygon(2, RGB(0, 0, 0));
	pxAxis->SetVertex(0, CVertex(-fHalfWidth, 0.0f, 0.0f));
	pxAxis->SetVertex(1, CVertex(+fHalfWidth, 0.0f, 0.0f));
	SetPolygon(0, pxAxis);

	CPolygon* pyAxis = new CPolygon(2, RGB(0, 0, 0));
	pyAxis->SetVertex(0, CVertex(0.0f, -fHalfWidth, 0.0f));
	pyAxis->SetVertex(1, CVertex(0.0f, +fHalfWidth, 0.0f));
	SetPolygon(1, pyAxis);

	CPolygon* pzAxis = new CPolygon(2, RGB(0, 0, 0));
	pzAxis->SetVertex(0, CVertex(0.0f, 0.0f, -fHalfWidth));
	pzAxis->SetVertex(1, CVertex(0.0f, 0.0f, +fHalfWidth));
	SetPolygon(2, pzAxis);
}

void CAxisMesh::Render(HDC hDCFrameBuffer)
{
	XMFLOAT3 f3PreviousProject = CGraphicsPipeline::Project(m_ppPolygons[0]->m_pVertices[0].m_xmf3Position);
	XMFLOAT3 f3CurrentProject = CGraphicsPipeline::Project(m_ppPolygons[0]->m_pVertices[1].m_xmf3Position);
	HPEN hPen = ::CreatePen(PS_SOLID, 0, RGB(255, 0, 0));
	HPEN hOldPen = (HPEN)::SelectObject(hDCFrameBuffer, hPen);
	::Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3CurrentProject);
	::SelectObject(hDCFrameBuffer, hOldPen);
	::DeleteObject(hPen);

	f3PreviousProject = CGraphicsPipeline::Project(m_ppPolygons[1]->m_pVertices[0].m_xmf3Position);
	f3CurrentProject = CGraphicsPipeline::Project(m_ppPolygons[1]->m_pVertices[1].m_xmf3Position);
	hPen = ::CreatePen(PS_SOLID, 0, RGB(0, 0, 255));
	hOldPen = (HPEN)::SelectObject(hDCFrameBuffer, hPen);
	::Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3CurrentProject);
	::SelectObject(hDCFrameBuffer, hOldPen);
	::DeleteObject(hPen);

	f3PreviousProject = CGraphicsPipeline::Project(m_ppPolygons[2]->m_pVertices[0].m_xmf3Position);
	f3CurrentProject = CGraphicsPipeline::Project(m_ppPolygons[2]->m_pVertices[1].m_xmf3Position);
	hPen = ::CreatePen(PS_SOLID, 0, RGB(0, 255, 0));
	hOldPen = (HPEN)::SelectObject(hDCFrameBuffer, hPen);
	::Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3CurrentProject);
	::SelectObject(hDCFrameBuffer, hOldPen);
	::DeleteObject(hPen);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
CLandMesh::CLandMesh(float fWidth, float fHeight, float fDepth, std::array<COLORREF, 3> color) : CMesh(6)
{
	float fHalfWidth = fWidth * 0.5f;
	float fHalfHeight = fHeight * 0.5f;
	float fHalfDepth = fDepth * 0.5f;

	CPolygon* pFrontFace = new CPolygon(4, RGB(0, 0, 0));
	pFrontFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
	SetPolygon(0, pFrontFace);

	CPolygon* pTopFace = new CPolygon(4, RGB(0, 0, 0));
	pTopFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
	pTopFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
	pTopFace->SetVertex(2, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
	pTopFace->SetVertex(3, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
	SetPolygon(1, pTopFace);

	CPolygon* pBackFace = new CPolygon(4, RGB(0, 0, 0));
	pBackFace->SetVertex(0, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(1, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(2, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(3, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
	SetPolygon(2, pBackFace);

	CPolygon* pBottomFace = new CPolygon(4, RGB(0, 0, 0));
	pBottomFace->SetVertex(0, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
	pBottomFace->SetVertex(1, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
	pBottomFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBottomFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
	SetPolygon(3, pBottomFace);

	CPolygon* pLeftFace = new CPolygon(4, RGB(0, 0, 0));
	pLeftFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
	pLeftFace->SetVertex(1, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
	pLeftFace->SetVertex(2, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
	pLeftFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
	SetPolygon(4, pLeftFace);

	CPolygon* pRightFace = new CPolygon(4, RGB(0, 0, 0));
	pRightFace->SetVertex(0, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
	pRightFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
	pRightFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
	pRightFace->SetVertex(3, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
	SetPolygon(5, pRightFace);

	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fHalfWidth, fHalfHeight, fHalfDepth), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CCartMesh.cpp

CCartMesh::CCartMesh() : CMesh(71)
{
	int i = 0;

	COLORREF bodyColor		= RGB(0, 100, 0);
	COLORREF innerColor		= RGB(166, 73, 53);
	COLORREF seatColor		= RGB(211, 178, 129);
	COLORREF barColor		= RGB(50, 50, 50);
	COLORREF frameColor		= RGB(100, 100, 100);

	// Cart 바닥				| 사용 Mesh : 6
	{
		float x1 = -4.0f;	float x2 = 4.0f;
		float y1 = 10.0f;	float y2 = -10.0f;  // 앞뒤 (Y축)
		float z1 = 3.0f;	float z2 = 2.0f;  // 아래위 (Z축)
		COLORREF color = RGB(0, 0, 0);

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, color);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 아랫면 (z1)
		CPolygon* bottom = new CPolygon(4, color);
		bottom->SetVertex(0, CVertex({ x2, y1, z1 }));
		bottom->SetVertex(1, CVertex({ x1, y1, z1 }));
		bottom->SetVertex(2, CVertex({ x1, y2, z1 }));
		bottom->SetVertex(3, CVertex({ x2, y2, z1 }));
		SetPolygon(i++, bottom);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, color);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, color);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, color);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, color);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// Cart 뒤쪽 벽				| 사용 Mesh : 5
	{
		float x1 = -3.8f;	float x2 = 3.8f;
		float y1 = -8.8f;	float y2 = -9.8f;  // 앞뒤 (Y축)
		float z1 = 2.0f;	float z2 = -2.0f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, frameColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, innerColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, bodyColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, bodyColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, bodyColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// Cart 오른쪽 벽			| 사용 Mesh : 5
	{
		float x1 = 2.8f;	float x2 = 3.8f;
		float y1 = 9.8f;	float y2 = -9.8f;  // 앞뒤 (Y축)
		float z1 = 2.0f;	float z2 = 0.0f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, frameColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, bodyColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, bodyColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, innerColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, bodyColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// Cart 왼쪽 벽				| 사용 Mesh : 5
	{
		float x1 = -3.8f;	float x2 = -2.8f;
		float y1 = 9.8f;	float y2 = -9.8f;  // 앞뒤 (Y축)
		float z1 = 2.0f;	float z2 = 0.0f;  // 아래위 (Z축)
		
		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, frameColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, bodyColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, bodyColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, bodyColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, innerColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// Cart 앞쪽 벽				| 사용 Mesh : 5
	{
		float x1 = -3.8f;	float x2 = 3.8f;
		float y1 = 9.8f;	float y2 = 8.8f;  // 앞뒤 (Y축)
		float z1 = 2.0f;	float z2 = 0.0f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, frameColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, bodyColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, bodyColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, bodyColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, bodyColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// Cart 본넷				| 사용 Mesh : 5
	{
		float x1 = -3.8f;	float x2 = 3.8f;
		float y1 = 9.8f;	float y2 = 3.8f;  // 앞뒤 (Y축)
		float z1 = 0.0f;	float z2 = -1.0f;  // 아래위 (Z축)

		// 아래면
		CPolygon* bottom = new CPolygon(4, bodyColor);
		bottom->SetVertex(0, CVertex({ x1, y1, z1 }));
		bottom->SetVertex(1, CVertex({ x2, y1, z1 }));
		bottom->SetVertex(2, CVertex({ x2, y2, z1 }));
		bottom->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, bottom);

		// 뒷면
		CPolygon* back = new CPolygon(4, bodyColor);
		back->SetVertex(0, CVertex({ x1, y2, z1 }));
		back->SetVertex(1, CVertex({ x2, y2, z1 }));
		back->SetVertex(2, CVertex({ x2, y2, z2 }));
		back->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, back);

		// 윗면
		CPolygon* top = new CPolygon(4, bodyColor);
		top->SetVertex(0, CVertex({ x1, y1, z1 }));
		top->SetVertex(1, CVertex({ x2, y1, z1 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 오른쪽
		CPolygon* right = new CPolygon(3, bodyColor);
		right->SetVertex(0, CVertex({ x2, y2, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		SetPolygon(i++, right);

		// 왼쪽
		CPolygon* left = new CPolygon(3, bodyColor);
		left->SetVertex(0, CVertex({ x1, y2, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z1 }));
		left->SetVertex(2, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);
	}
	// 앞쪽 좌석
	{
		float x1 = -2.8f;	float x2 = 2.8f;
		float y1 = -2.0f;	float y2 = -3.0f;  // 앞뒤 (Y축)
		float z1 = 2.0f;	float z2 = -1.0f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, seatColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, seatColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, seatColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, seatColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, seatColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// 앞쪽 좌석 머리 1
	{
		float x1 = -1.9f;	float x2 = -0.9f;
		float y1 = -2.2f;	float y2 = -2.8f;  // 앞뒤 (Y축)
		float z1 = -1.0f;	float z2 = -1.75f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, seatColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, seatColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, seatColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, seatColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, seatColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// 앞쪽 좌석 머리 2
	{
		float x1 = 0.9f;	float x2 = 1.9f;
		float y1 = -2.2f;	float y2 = -2.8f;  // 앞뒤 (Y축)
		float z1 = -1.0f;	float z2 = -1.75f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, seatColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, seatColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, seatColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, seatColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, seatColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// 앞쪽 안전 바
	{
		float x1 = -3.8f;	float x2 = 3.8f;
		float y1 = 1.0f;	float y2 = 0.5f;  // 앞뒤 (Y축)
		float z1 = 0.0f;	float z2 = -0.5f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, barColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, barColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, barColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, barColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, barColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// 뒤쪽 좌석
	{
		float x1 = -2.8f;	float x2 = 2.8f;
		float y1 = -7.8f;	float y2 = -8.8f;  // 앞뒤 (Y축)
		float z1 = 2.0f;	float z2 = -1.0f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, seatColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, seatColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, seatColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, seatColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, seatColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// 뒤쪽 좌석 머리 1
	{
		float x1 = -1.9f;	float x2 = -0.9f;
		float y1 = -8.0f;	float y2 = -8.6f;  // 앞뒤 (Y축)
		float z1 = -1.0f;	float z2 = -1.75f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, seatColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, seatColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, seatColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, seatColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, seatColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// 뒤쪽 좌석 머리 2
	{
		float x1 = 0.9f;	float x2 = 1.9f;
		float y1 = -8.0f;	float y2 = -8.6f;  // 앞뒤 (Y축)
		float z1 = -1.0f;	float z2 = -1.75f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, seatColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, seatColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, seatColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, seatColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, seatColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// 뒤쪽 안전 바
	{
		float x1 = -3.8f;	float x2 = 3.8f;
		float y1 = -4.8f;	float y2 = -5.3f;  // 앞뒤 (Y축)
		float z1 = 0.0f;	float z2 = -0.5f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, barColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, barColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, barColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, barColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, barColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}

	// [마무리] 바운딩 박스 설정
	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

CCartMesh::~CCartMesh() {}