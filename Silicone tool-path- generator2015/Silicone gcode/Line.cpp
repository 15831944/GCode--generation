#include "StdAfx.h"
#include "Line.h"

CLine::CLine(void)
{
}

CLine::~CLine(void)
{
}

CLine::CLine(const CLine& line)
{
	m_pt1=line.m_pt1;
	m_pt2=line.m_pt2;
	m_length=line.m_length;
}

CLine::CLine(const CPoint3D& pt1, const CPoint3D& pt2)
{
	m_pt1=pt1;
	m_pt2=pt2;
}

CLine::CLine(const CPoint3D& pt1, const CPoint3D& pt2, double length)
{
	m_pt1=pt1;
	m_pt2=pt2;
	m_length=length;
}

const CLine& CLine::operator =(const CLine& line)
{
	m_pt1=line.m_pt1;
	m_pt2=line.m_pt2;
	m_length=line.m_length;
	return *this;
}

void CLine::Draw(COpenGLDC* pDC,COLORREF clr)
{
	COLORREF oldClr;
	if (GetIsSelected())
		oldClr=pDC->SetMaterialColor(CLR_SELECTED);
	else
		oldClr=pDC->SetMaterialColor(clr);

	pDC->DrawLine(m_pt1,m_pt2,m_nStyle,m_nWith);
	
	pDC->SetMaterialColor(oldClr);
}

void CLine::Move(const CVector3D& vec)
{
	m_pt1+=vec;
	m_pt2+=vec;
}

void CLine::Rotate(double angle)
{
	Rotate(CMatrix3D::CreateRotateMatrix(angle,AXIS_Z));
}

void CLine::Rotate(const CMatrix3D& m)
{
	m_pt1 *= m;
	m_pt2 *= m;
}

void CLine::SetPt1(const CPoint3D& pt1)
{
	m_pt1=pt1;
}

void CLine::SetPt2(const CPoint3D& pt2)
{
	m_pt2=pt2;
}

BOOL CLine::operator * (const CLine &line) const
{
	CVector3D v1=m_pt2-m_pt1;
	CVector3D v2=line.m_pt2-line.m_pt1;
	CPoint3D p1=m_pt1;
	CPoint3D p2=line.m_pt1;
	double lamda1=((p2.x-p1.x)*v2.dy-(p2.y-p1.y)*v2.dx)/(v1.dx*v2.dy-v1.dy*v2.dx);  // 当两条直线平行于坐标系时会有问题
	double lamda2;
	if (0!=v2.dx)
		lamda2=(p1.x-p2.x+lamda1*v1.dx)/v2.dx;
	else
		lamda2=(p1.y-p2.y+lamda1*v1.dy)/v2.dy;

	if (lamda1>=ZERO && lamda1<=ONE && lamda2>=ZERO && lamda2<=ONE)  // 得到贺博指点，将这里改了 2011-8-2
		return TRUE;
	else
		return FALSE;
}

void CLine::operator *= (const CMatrix3D& matrix)
{
	m_pt1*=matrix;
	m_pt2*=matrix;
}

BOOL CLine::IsCrossWith(const CLine& line, CPoint3D& ptCross) const
{
	CVector3D v1=m_pt2-m_pt1;
	CVector3D v2=line.m_pt2-line.m_pt1;
	CPoint3D p1=m_pt1;
	CPoint3D p2=line.m_pt1;
	double lamda1=((p2.x-p1.x)*v2.dy-(p2.y-p1.y)*v2.dx)/(v1.dx*v2.dy-v1.dy*v2.dx);
	double lamda2;
	if (0!=v2.dx)
	{
		lamda2=(p1.x-p2.x+lamda1*v1.dx)/v2.dx;
	}
	else
	{
		lamda2=(p1.y-p2.y+lamda1*v1.dy)/v2.dy;
	}

	if (lamda1>=0 && lamda1<=1 && lamda2>=0 && lamda2<=1)
	{
		ptCross=m_pt1+v1*lamda1;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

double CLine::operator ^(const CLine &line) const
{
	CVector3D v1=m_pt2-m_pt1;
	CVector3D v2=line.m_pt2-line.m_pt1;
	return GetAngle(v1,v2);
}

double CLine::operator |(const CLine &line) const
{
	CVector3D v1=m_pt2-m_pt1;
	CVector3D v2=line.m_pt2-line.m_pt1;
	return v1|v2;
}

double CLine::GetLength() const
{
	return sqrt(GetLength2());
}

double CLine::GetLength2() const
{
	CVector3D vec=m_pt2-m_pt1;
	return vec.GetMod2();
}

double CLine::GetDistanceTo(const CPoint3D& pt) const
{
	CVector3D P1P=pt-m_pt1;
	CVector3D v=m_pt2-m_pt1;
	double lamda=(P1P|v)/v.GetMod2();

	if (lamda<0)
	{
		return P1P.GetMod();
	}
	else if (lamda>1)
	{
		CVector3D P2P=pt-m_pt2;
		return P2P.GetMod();
	}
	else
	{
		CVector3D vec=(-P1P+v*lamda);
		return vec.GetMod();
	}
}

double CLine::GetDistanceTo(const CLine& line) const
{
	if ((*this) * line)  // If two line cross, then return 0
		return 0;

	double d1=GetDistanceTo(line.m_pt1);
	double d2=GetDistanceTo(line.m_pt2);
	double d3=line.GetDistanceTo(m_pt1);
	double d4=line.GetDistanceTo(m_pt2);

	double dMin=d1;
	if (d2<dMin)
		dMin=d2;
	if (d3<dMin)
		dMin=d3;
	if (d4<dMin)
		dMin=d4;

	return dMin;
}

void CLine::Reverse()
{
	CPoint3D tmpPt=m_pt1;
	m_pt1=m_pt2;
	m_pt2=tmpPt;
}

CVector3D CLine::GetVector() const
{
	return (m_pt2-m_pt1);
}

CVector3D CLine::GetNormalVector() const
{
	return GetVector().Normalize();
}