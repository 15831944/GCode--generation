#include "stdafx.h"
#include "LVertex.h"

using namespace std;

LVertex::LVertex()
{

}

LVertex::LVertex(const LVertex& vertex)
{
	x=vertex.x;
	y=vertex.y;
	z=vertex.z;
}
LVertex::LVertex(const CPoint3D& pt)
{
	x=pt.x;
	y=pt.y;
	z=pt.z;
}

bool LVertex::operator < (const LVertex& vertex)  // 左下角 到 右上角
{
	if ((x<vertex.x) 
		|| (x==vertex.x && y<vertex.y) 
		|| (x==vertex.x && y==vertex.y && z<vertex.z))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool LVertex::operator > (const LVertex& vertex)
{
	if (*this == vertex)
	{
		return false;
	}

	return !((*this)<vertex);
}

bool LVertex::IsLower(const LVertex& vertex)
{
	if (z < vertex.z)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool LVertex::IsLower(LVertex* pVertex)
{
	return IsLower(*pVertex);
}


//////////////////////////////////////////////////////////////////////////

LEdge::LEdge()
{
	v1=v2=NULL;
	t=NULL;
	e_prev=e_next=e_adja=NULL;
}

LEdge::LEdge(LVertex* _v1, LVertex* _v2)
{
	v1=_v1;
	v2=_v2;

	e_adja=NULL;
}

//////////////////////////////////////////////////////////////////////////

LTriangle::LTriangle()
{
	v1=v2=v3=NULL;
	e1=e2=e3=NULL;
	n=NULL;
}

LTriangle::LTriangle(LVertex* v1, LVertex* v2, LVertex* v3)
{
	this->v1 = v1;
	this->v2 = v2;
	this->v3 = v3;

	e1=e2=e3=NULL;
	n=NULL;
}

CVector3D* LTriangle::GntNormal()
{
	CVector3D vec1 = *v2 - *v1;
	CVector3D vec2 = *v3 - *v2;
	CVector3D vec = vec1*vec2;
	vec.Normalize();
	n=new CVector3D(vec);
	return n;  // 返回n放在外面数组里以便统一管理
}

LTriangle* LTriangle::GetNbTri1() const
{
	return e1->e_adja ? e1->e_adja->t : NULL;
}

LTriangle* LTriangle::GetNbTri2() const
{
	return e2->e_adja ? e2->e_adja->t : NULL;
}

LTriangle* LTriangle::GetNbTri3() const
{
	return e3->e_adja ? e3->e_adja->t : NULL;
}

void LTriangle::Draw(COpenGLDC* pDC)
{
	COLORREF oldClr;
	if (GetIsSelected())
		oldClr=pDC->SetMaterialColor(CLR_SELECTED);
	else
		oldClr=pDC->SetMaterialColor(m_clr);

	pDC->DrawTriChip(*n,*v1,*v2,*v3);

	pDC->SetMaterialColor(oldClr);
}

//////////////////////////////////////////////////////////////////////////

bool LEdgeHull::IsOpposite(const LEdgeHull& edgeHull)
{
	if (edge->v1 == edgeHull.edge->v2 && edge->v2 == edgeHull.edge->v1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool LEdgeHull::operator < (const LEdgeHull& edgeHull)
{
	LVertex v1 = *edge->v1;  // v1 < v2
	LVertex v2 = *edge->v2;
	if (v2 < v1)
	{
		swap(v1,v2);
	}

	LVertex v1_ = *edgeHull.edge->v1;
	LVertex v2_ = *edgeHull.edge->v2;
	if (v2_ < v1_)
	{
		swap(v1_,v2_);
	}

	double& a1=v1.x;
	double& b1=v1.y;
	double& c1=v1.z;
	double& d1=v2.x;
	double& e1=v2.y;
	double& f1=v2.z;

	double& a2=v1_.x;
	double& b2=v1_.y;
	double& c2=v1_.z;
	double& d2=v2_.x;
	double& e2=v2_.y;
	double& f2=v2_.z;

	if ( (a1<a2)  
		|| (a1==a2 && b1<b2)
		|| (a1==a2 && b1==b2 && c1<c2)
		|| (a1==a2 && b1==b2 && c1==c2 && d1<d2)
		|| (a1==a2 && b1==b2 && c1==c2 && d1==d2 && e1<e2)
		|| (a1==a2 && b1==b2 && c1==c2 && d1==d2 && e1==e2 && f1<f2))
	{
		return true;
	}
	else
	{
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
TopologySTL::TopologySTL()
{

}

TopologySTL::~TopologySTL()
{
	int sz=m_vertices.size();
	for (int i=sz-1;i>=0;i--)
	{
		if (NULL != m_vertices[i])
		{
			delete m_vertices[i];
			m_vertices[i]=NULL;
		}
	}
	m_vertices.clear();

	sz=m_ltris.size();
	for (int i=sz-1;i>=0;i--)
	{
		if (NULL != m_ltris[i])
		{
			delete m_ltris[i];
			m_ltris[i]=NULL;
		}
	}
	m_ltris.clear();

	sz=m_edges.size();
	for (int i=sz-1;i>=0;i--)
	{
		if (NULL != m_edges[i])
		{
			delete m_edges[i];
			m_edges[i]=NULL;
		}
	}
	m_edges.clear();

	sz=m_normals.size();
	for (int i=sz-1;i>=0;i--)
	{
		if (NULL != m_normals[i])
		{
			delete m_normals[i];
			m_normals[i]=NULL;
		}
	}
}