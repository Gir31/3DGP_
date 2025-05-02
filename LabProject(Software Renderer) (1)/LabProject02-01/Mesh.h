#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CVertex
{
public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(float x, float y, float z) { m_xmf3Position = XMFLOAT3(x, y, z); }
	~CVertex() { }

	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
};

class CPolygon
{
public:
	CPolygon() { }
	CPolygon(int nVertices);
	~CPolygon();

	int							m_nVertices = 0;
	CVertex						*m_pVertices = NULL;

	void SetVertex(int nIndex, CVertex& vertex);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CMesh
{
public:
	CMesh() { }
	CMesh(int nPolygons);
	virtual ~CMesh();

private:
	int							m_nReferences = 1;

public:
	void AddRef() { m_nReferences++; }
	void Release() { m_nReferences--; if (m_nReferences <= 0) delete this; }

protected:
	int							m_nPolygons = 0;
	CPolygon					**m_ppPolygons = NULL;

public:
	BoundingOrientedBox			m_xmOOBB = BoundingOrientedBox();

public:
	void SetPolygon(int nIndex, CPolygon *pPolygon);

	virtual void Render(HDC hDCFrameBuffer);

	BOOL RayIntersectionByTriangle(XMVECTOR& xmRayOrigin, XMVECTOR& xmRayDirection, XMVECTOR v0, XMVECTOR v1, XMVECTOR v2, float* pfNearHitDistance);
	int CheckRayIntersection(XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection, float* pfNearHitDistance);
};

class CCubeMesh : public CMesh
{
public:
	CCubeMesh(float fWidth = 4.0f, float fHeight = 4.0f, float fDepth = 4.0f);
	virtual ~CCubeMesh() { }
};

class CWallMesh : public CMesh
{
public:
	CWallMesh(float fWidth = 4.0f, float fHeight = 4.0f, float fDepth = 4.0f, int nSubRects = 20);
	virtual ~CWallMesh() { }
};

class CAirplaneMesh : public CMesh
{
public:
	CAirplaneMesh(float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 4.0f);
	virtual ~CAirplaneMesh() { }
};

class CAxisMesh : public CMesh
{
public:
	CAxisMesh(float fWidth = 4.0f, float fHeight = 4.0f, float fDepth = 4.0f);
	virtual ~CAxisMesh() { }

	virtual void Render(HDC hDCFrameBuffer);
};

class CTextMesh : public CMesh
{
public:
	template <size_t N1, size_t N2, size_t N3>

	CTextMesh(float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f,
		std::array<bool, N1> text = true, std::array<float, N2> cx = 0, std::array<float, N3> cy = 0) 
		: CMesh(count(text.begin(), text.end(), true)*6)
	{
		float fHalfWidth = fWidth * 0.5f;
		float fHalfHeight = fHeight * 0.5f;
		float fHalfDepth = fDepth * 0.5f;

		int cnt = 0;

		for (int i = 0; i < text.size(); ++i) {
			int x = i % cx.size();
			int y = i / cx.size();
			if (text[i]) {
				CPolygon* pFrontFace = new CPolygon(4);
				pFrontFace->SetVertex(0, CVertex(cx[x] - fHalfWidth, cy[y] + fHalfHeight, -fHalfDepth));
				pFrontFace->SetVertex(1, CVertex(cx[x] + fHalfWidth, cy[y] + fHalfHeight, -fHalfDepth));
				pFrontFace->SetVertex(2, CVertex(cx[x] + fHalfWidth, cy[y] - fHalfHeight, -fHalfDepth));
				pFrontFace->SetVertex(3, CVertex(cx[x] - fHalfWidth, cy[y] - fHalfHeight, -fHalfDepth));
				SetPolygon((cnt * 6), pFrontFace);

				CPolygon* pTopFace = new CPolygon(4);
				pTopFace->SetVertex(0, CVertex(cx[x] - fHalfWidth, cy[y] + fHalfHeight, +fHalfDepth));
				pTopFace->SetVertex(1, CVertex(cx[x] + fHalfWidth, cy[y] + fHalfHeight, +fHalfDepth));
				pTopFace->SetVertex(2, CVertex(cx[x] + fHalfWidth, cy[y] + fHalfHeight, -fHalfDepth));
				pTopFace->SetVertex(3, CVertex(cx[x] - fHalfWidth, cy[y] + fHalfHeight, -fHalfDepth));
				SetPolygon((cnt * 6) + 1, pTopFace);

				CPolygon* pBackFace = new CPolygon(4);
				pBackFace->SetVertex(0, CVertex(cx[x] - fHalfWidth, cy[y] - fHalfHeight, +fHalfDepth));
				pBackFace->SetVertex(1, CVertex(cx[x] + fHalfWidth, cy[y] - fHalfHeight, +fHalfDepth));
				pBackFace->SetVertex(2, CVertex(cx[x] + fHalfWidth, cy[y] + fHalfHeight, +fHalfDepth));
				pBackFace->SetVertex(3, CVertex(cx[x] - fHalfWidth, cy[y] + fHalfHeight, +fHalfDepth));
				SetPolygon((cnt * 6) + 2, pBackFace);

				CPolygon* pBottomFace = new CPolygon(4);
				pBottomFace->SetVertex(0, CVertex(cx[x] - fHalfWidth, cy[y] - fHalfHeight, -fHalfDepth));
				pBottomFace->SetVertex(1, CVertex(cx[x] + fHalfWidth, cy[y] - fHalfHeight, -fHalfDepth));
				pBottomFace->SetVertex(2, CVertex(cx[x] + fHalfWidth, cy[y] - fHalfHeight, +fHalfDepth));
				pBottomFace->SetVertex(3, CVertex(cx[x] - fHalfWidth, cy[y] - fHalfHeight, +fHalfDepth));
				SetPolygon((cnt * 6) + 3, pBottomFace);

				CPolygon* pLeftFace = new CPolygon(4);
				pLeftFace->SetVertex(0, CVertex(cx[x] - fHalfWidth, cy[y] + fHalfHeight, +fHalfDepth));
				pLeftFace->SetVertex(1, CVertex(cx[x] - fHalfWidth, cy[y] + fHalfHeight, -fHalfDepth));
				pLeftFace->SetVertex(2, CVertex(cx[x] - fHalfWidth, cy[y] - fHalfHeight, -fHalfDepth));
				pLeftFace->SetVertex(3, CVertex(cx[x] - fHalfWidth, cy[y] - fHalfHeight, +fHalfDepth));
				SetPolygon((cnt * 6) + 4, pLeftFace);

				CPolygon* pRightFace = new CPolygon(4);
				pRightFace->SetVertex(0, CVertex(cx[x] + fHalfWidth, cy[y] + fHalfHeight, -fHalfDepth));
				pRightFace->SetVertex(1, CVertex(cx[x] + fHalfWidth, cy[y] + fHalfHeight, +fHalfDepth));
				pRightFace->SetVertex(2, CVertex(cx[x] + fHalfWidth, cy[y] - fHalfHeight, +fHalfDepth));
				pRightFace->SetVertex(3, CVertex(cx[x] + fHalfWidth, cy[y] - fHalfHeight, -fHalfDepth));
				SetPolygon((cnt * 6) + 5, pRightFace);

				++cnt;
			}
		}

		m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(cx[cx.size() - 1], cy[0], fHalfDepth), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

	}
	virtual ~CTextMesh() { }
};