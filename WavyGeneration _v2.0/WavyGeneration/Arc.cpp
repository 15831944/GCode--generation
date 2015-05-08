#include "StdAfx.h"
#include "Arc.h"

CArc::CArc(void)
{

}

CArc::~CArc(void)
{
}

CArc::CArc(const CArc& arc)
{
	m_ptCenter=arc.m_ptCenter;
	m_radius=arc.m_radius;
	m_angle1=arc.m_angle1;
	m_angle2=arc.m_angle2;
	m_count=arc.m_count;

	for (int i=0;i<m_count;i++)
		m_pt[i]=arc.m_pt[i];
}

CArc::CArc(const CPoint3D &ptCenter, double radius, double angleStart, double angleEnd)
{
	Create(ptCenter,radius,angleStart,angleEnd);
}

CArc::CArc(const CPoint3D &pt1, const CPoint3D &pt2, const CPoint3D &ptSome)
{
	CVector3D v1,v2,v_r;
	CPoint3D ptM1,ptM2,ptCenter;
	ptM1=(pt1+pt2)*0.5;
	ptM2=(pt1+ptSome)*0.5;
	v1=GetOrthoVectorOnXYPlane(pt1,pt2);
	v2=GetOrthoVectorOnXYPlane(pt1,ptSome);

	if (2==GetCrossPoint(ptCenter,ptM1,ptM1+v1,ptM2,ptM2+v2))
	{
		v_r=pt1-ptCenter;
		double radius=v_r.GetLength();

		double angle1,angle2;
		CVector3D v(1,0,0);
		v1=v_r;
		v1.Normalize();
		double dotProduct=v1|v;
		if (dotProduct>1) dotProduct=1;
		if (dotProduct<-1) dotProduct=-1;
		angle1=acos(dotProduct);
		if ((v1*v).dz>0)
			angle1=2*PI-angle1;

		v2=pt2-ptCenter;;
		v2.Normalize();
		dotProduct=v2|v;
		if (dotProduct>1) dotProduct=1;
		if (dotProduct<-1) dotProduct=-1;
		angle2=acos(dotProduct);
		if ((v2*v).dz>0)
			angle2=2*PI-angle2;

		v1=pt1-ptSome;
		v2=pt2-ptSome;

		if ((v1*v2).dz<0)
		{
			Create(ptCenter,radius,angle1,angle2);
		}
		else
		{
			Create(ptCenter,radius,angle2,angle1);
		}
	}
	else
	{
		m_count=2;
		m_pt[0]=pt1;
		m_pt[1]=pt2;
	}

	// The situation when three points on the same line
}

const CArc& CArc::operator = (const CArc& arc)
{
	m_ptCenter=arc.m_ptCenter;
	m_radius=arc.m_radius;
	m_angle1=arc.m_angle1;
	m_angle2=arc.m_angle2;
	m_count=arc.m_count;
	for (int i=0;i<m_count;i++)
		m_pt[i]=arc.m_pt[i];

	return *this;
}

void CArc::Create(const CPoint3D& ptCenter, double radius, double angleStart_rad, double angleEnd_rad)
{
	m_ptCenter=ptCenter;
	m_radius=radius;
	m_angle1=angleStart_rad;
	m_angle2=angleEnd_rad;

	if (angleStart_rad>angleEnd_rad)
		angleEnd_rad+=TWOPI;

	double step=deg2rad(2);
	double angle=angleStart_rad;
	int i=0;
	while (angle<angleEnd_rad)
	{
		m_pt[i].x=ptCenter.x+radius*cos(angle);
		m_pt[i].y=ptCenter.y+radius*sin(angle);
		angle+=step;
		i++;
	}
	m_pt[i].x=ptCenter.x+radius*cos(angleEnd_rad);
	m_pt[i].y=ptCenter.y+radius*sin(angleEnd_rad);
	m_count=i+1;
}

void CArc::Draw(COpenGLDC* pDC,COLORREF clr)
{
	COLORREF oldClr;
	if (GetIsSelected())
		oldClr=pDC->SetMaterialColor(CLR_SELECTED);
	else
		oldClr=pDC->SetMaterialColor(clr);

	for (int i=0;i<m_count-1;i++)
		pDC->DrawLine(m_pt[i],m_pt[i+1],m_nStyle,m_nWith);

	pDC->SetMaterialColor(oldClr);
}

int CArc::GetPtCount() const
{
	return m_count;
}