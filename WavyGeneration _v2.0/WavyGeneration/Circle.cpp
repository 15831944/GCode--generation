#include "StdAfx.h"
#include "Circle.h"

CCircle::CCircle(void)
{
}

CCircle::~CCircle(void)
{
}

CCircle::CCircle(const CCircle &circle)
{
	m_ptCenter=circle.m_ptCenter;
	m_radius=circle.m_radius;
	for (int i=0;i<CIRCLE_PRECISION;i++)
	{
		pt[i]=circle.pt[i];
	}
}

CCircle::CCircle(const CPoint3D& ptCenter, const double radius)
{
	m_ptCenter=ptCenter;
	m_radius=radius;

	double step=2*PI/CIRCLE_PRECISION;  // 3 degree
	double angle;
	for (int i=0;i<CIRCLE_PRECISION;i++)
	{
		angle=step*(double)i;
		pt[i].x = ptCenter.x + m_radius*cos(angle);
		pt[i].y = ptCenter.y + m_radius*sin(angle);
		pt[i].z = ptCenter.z;
	}	
}

CCircle::CCircle(const CPoint3D &pt1, const CPoint3D &pt2, const CPoint3D &pt3)
{
	CVector3D v1,v2,v_r;
	CPoint3D ptM1,ptM2,ptCenter;
	ptM1=(pt1+pt2)*0.5;
	ptM2=(pt1+pt3)*0.5;
	v1=GetOrthoVectorOnXYPlane(pt1,pt2);
	v2=GetOrthoVectorOnXYPlane(pt1,pt3);
	GetCrossPoint(ptCenter,ptM1,ptM1+v1,ptM2,ptM2+v2);
	v_r=pt1-ptCenter;
	double radius=v_r.GetLength();
	*this=CCircle(ptCenter,radius);
}

const CCircle& CCircle::operator =(const CCircle &circle)
{
	m_ptCenter=circle.m_ptCenter;
	m_radius=circle.m_radius;
	for (int i=0;i<CIRCLE_PRECISION;i++)
	{
		pt[i]=circle.pt[i];
	}
	return *this;
}

void CCircle::Draw(COpenGLDC *pDC,COLORREF clr)
{
	COLORREF oldClr;
	if (GetIsSelected())
		oldClr=pDC->SetMaterialColor(CLR_SELECTED);
	else
		oldClr=pDC->SetMaterialColor(clr);

	pDC->DrawPolyline(pt,CIRCLE_PRECISION);

	pDC->SetMaterialColor(oldClr);
}

void CCircle::Move(const CVector3D& vec)
{

}