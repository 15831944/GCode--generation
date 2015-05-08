#pragma once

#include "GeomBase.h"
#include "Entity.h"
#include <vector>
using std::vector;

// 定义指针结构的点、边、三角形
struct LVertex;
struct LEdge;
struct LTriangle;

struct LEdgeHull;

//////////////////////////////////////////////////////////////////////////
struct LVertex : public CPoint3D
{
	LVertex();
	LVertex(const LVertex& vertex);
	LVertex(const CPoint3D& pt);
	bool operator < (const LVertex& vertex);  // 左下角 到 右上角
	bool operator > (const LVertex& vertex);
	bool IsLower(const LVertex& vertex);
	bool IsLower(LVertex* pVertex);
};

//////////////////////////////////////////////////////////////////////////
struct LEdge   // Half edge structure
{
	LEdge();
	LEdge(LVertex* _v1, LVertex* _v2);

	LVertex   *v1, *v2;   // Two vertices, v1: start   v2: end
	LTriangle *t;         // The triangle the edge belong to
	LEdge     *e_prev,    // previous edge
			  *e_next,    // next edge
			  *e_adja;    // adjacent edge together with which forms edge
};

//////////////////////////////////////////////////////////////////////////
struct LTriangle : public CEntity
{
	LTriangle();
	LTriangle(LVertex* v1, LVertex* v2, LVertex* v3);
	LTriangle* GetNbTri1() const;
	LTriangle* GetNbTri2() const;
	LTriangle* GetNbTri3() const;
	CVector3D* GntNormal(); 
	virtual void Draw(COpenGLDC* pDC);

	LVertex *v1, *v2, *v3;  // 逆时针排列
	LEdge   *e1, *e2, *e3;  // 半边
	CVector3D* n;           // 法向量
};


struct LEdgeHull
{
	LEdge* edge;

	bool IsOpposite(const LEdgeHull& edgeHull);
	bool operator < (const LEdgeHull& edgeHull);
};


struct TopologySTL    // 具有拓扑关系的点线面集合
{
	TopologySTL();
	~TopologySTL();
	vector<LVertex*>   m_vertices;
	vector<LEdge*>     m_edges;
	vector<LTriangle*> m_ltris;
	vector<CVector3D*> m_normals;
};
