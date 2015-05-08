#include "StdAfx.h"
#include "Ellipse.h"

CEllipse::CEllipse(void)
{
}

CEllipse::~CEllipse(void)
{

}

CEllipse::CEllipse(const CPoint3D& ptCenter, 
				   double a, 
				   double b, 
				   double angleRotate/* =0 */, 
				   double angleStart/* =0 */, 
				   double angleEnd/* =TWOPI */)
{
	Create(ptCenter,a,b,angleRotate,angleStart,angleEnd);
}

void CEllipse::Create(const CPoint3D& ptCenter, 
					  double a, 
					  double b, 
					  double angleRotate/* =0 */, 
					  double angleStart/* =0 */, 
					  double angleEnd/* =TWOPI */)
{
	m_ptCenter=ptCenter;
	m_a=a;
	m_b=b;
	m_angleRotate=angleRotate;
	m_angleStart=angleStart;
	m_angleEnd=angleEnd;

	CVector3D vecMove=ptCenter-CPoint3D(0,0,0);
	CMatrix3D matrixRotate=CMatrix3D::CreateRotateMatrix(angleRotate,CVector3D(0,0,1));
	if(angleEnd<angleStart) angleEnd+=2*PI;
	double step=deg2rad(1); // 1 degree


    int i=0;
	while(angleStart<angleEnd)
	{
		m_pt[i].x=a*cos(angleStart);
		m_pt[i].y=b*sin(angleStart);
		m_pt[i]*=matrixRotate;
		m_pt[i]+=vecMove;
		i++;
		angleStart+=step;
	}	
	m_pt[i].x=a*cos(angleEnd);
	m_pt[i].y=b*sin(angleEnd);
	m_pt[i]*=matrixRotate;
	m_pt[i]+=vecMove;
	m_count=i+1;
}

CEllipse::CEllipse(const CPoint3D& ptS_major, const CPoint3D& ptE_major, const CPoint3D& pt_some)
{
	CVector3D v_major;
	double a, b,angle;
	CPoint3D ptCenter;
	v_major=ptE_major-ptS_major;
	a=v_major.GetLength()/2;
	ptCenter=(ptS_major+ptE_major)*0.5;
	if (v_major.dy<0)
		v_major=-v_major;

	angle=GetAngle(CVector3D(1,0,0),v_major);

	double x,y;  // x is the distance to minor axis,and y is the distance to major axis

	CVector3D v_minor=GetOrthoVectorOnXYPlane(v_major);
	CPoint3D ptCross;
	GetCrossPoint(ptCross,ptS_major,ptE_major,pt_some,pt_some+v_minor);

	CVector3D v_y=ptCross-pt_some;
	CVector3D v_x=ptCross-ptCenter;
	x=v_x.GetLength();
	y=v_y.GetLength();

	if (x>a)
		b=y;
	else
		b=y/sqrt(1-x*x/(a*a));

	//new(this)CEllipse(ptCenter,a,b,angle);
	Create(ptCenter,a,b,angle);
}

const CEllipse& CEllipse::operator =(const CEllipse &ellipse)
{
	m_ptCenter=ellipse.m_ptCenter;
	m_a=ellipse.m_a;
	m_b=ellipse.m_b;
	m_angleRotate=ellipse.m_angleRotate;
	m_angleStart=ellipse.m_angleStart;
	m_angleEnd=ellipse.m_angleEnd;
	m_count=ellipse.m_count;

	for (int i=0;i<m_count;i++)
		m_pt[i]=ellipse.m_pt[i];

	return *this;
}

void CEllipse::Rotate(double rotate_angle)
{
	if (0==rotate_angle)
		return;

	CPoint3D ptOrg(0,0,0);
	CVector3D vec=ptOrg-m_ptCenter;
	for (int i=0;i<m_count;i++)
		m_pt[i]+=vec;
	CMatrix3D matrix=CMatrix3D::CreateRotateMatrix(rotate_angle,CVector3D(0,0,1));
	for (int i=0;i<m_count;i++)
		m_pt[i]*=matrix;
	vec=-vec;
	for (int i=0;i<m_count;i++)
		m_pt[i]+=vec;
}

void CEllipse::Draw(COpenGLDC *pDC,COLORREF clr)
{
	COLORREF oldClr;
	if (GetIsSelected())
		oldClr=pDC->SetMaterialColor(CLR_SELECTED);
	else
		oldClr=pDC->SetMaterialColor(clr);

	for (int i=0;i<m_count-1;i++)
		pDC->DrawLine(m_pt[i], m_pt[i+1], m_nStyle, m_nWith);

	pDC->SetMaterialColor(oldClr);
}