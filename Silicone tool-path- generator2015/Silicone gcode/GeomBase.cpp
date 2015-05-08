// CadBase.cpp: implementation of the CadBase class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GeomBase.h"


//////////////////////////////////////////////////////////////////////
// 点 2D
//////////////////////////////////////////////////////////////////////
CPoint2D::CPoint2D()
{
	x=y=0;
}

CPoint2D::CPoint2D(double ix, double iy)
{
	x=ix;
	y=iy;
}

CPoint2D::CPoint2D(const double* p)
{
	x=p[0];
	y=p[1];
}

CPoint2D::CPoint2D(const CPoint2D& pt)
{
	x=pt.x;
	y=pt.y;
}

CPoint2D::~CPoint2D()
{

}

const CPoint2D& CPoint2D::operator=(const CPoint2D& pt)
{
	x=pt.x;
	y=pt.y;

	return *this;
}

void CPoint2D::operator = (double val)
{
	x=y=val;
}

CPoint2D CPoint2D::operator+(const CVector2D& v) const
{
	return CPoint2D(x+v.dx,y+v.dy);
}

CPoint2D CPoint2D::operator+(const CPoint2D& pt) const
{
	return CPoint2D(x+pt.x,y+pt.y); 
}

CPoint2D CPoint2D::operator *(const double& dFactor) const
{
	return CPoint2D(x*dFactor,y*dFactor);
}

void CPoint2D::operator+=(const CVector2D& v)
{
	x+=v.dx;
	y+=v.dy;
}

CPoint2D CPoint2D::operator-(const CVector2D& v) const 
{
	return CPoint2D(x-v.dx,y-v.dy);
}      

void CPoint2D::operator-=(const CVector2D& v)                 // point and vector operation
{
	x-=v.dx;
	y-=v.dy;
}

CVector2D CPoint2D::operator-(const CPoint2D& pt) const       // point minus point to get vector
{
	return CVector2D(x-pt.x,y-pt.y);
}

BOOL CPoint2D::operator==(const CPoint2D& pt) const  // Complete the same 
{
	if (x!=pt.x) return FALSE;
	if (y!=pt.y) return FALSE;
	return TRUE;
}

BOOL CPoint2D::operator^=(const CPoint2D& pt) const  // More or less the same
{
	CVector2D vec(x-pt.x,y-pt.y);
	double len=vec.GetMod2();
	if(len<=POINT_RESOLUTION) 
		return TRUE;
	else 
		return FALSE;
}

BOOL CPoint2D::operator!=(const CPoint2D& pt) const
{
	return !(*this==pt);
} 

CPoint3D CPoint2D::ToPoint3D(double zVal/* =0 */) const
{
	return CPoint3D(x,y,zVal);
}

 

//////////////////////////////////////////////////////////////////////
// 点 3D
//////////////////////////////////////////////////////////////////////

CPoint3D::CPoint3D()
{
	x=y=z=0;
// 	m_index1=-1;
// 	m_index2=-1;
}

CPoint3D::CPoint3D(double ix, double iy, double iz)
{
	x=ix;
	y=iy;
	z=iz;
}

CPoint3D::CPoint3D(const double* p)
{
	x=p[0];
	y=p[1];
	z=p[2];
}

CPoint3D::CPoint3D(const CPoint3D& pt)
{
	x=pt.x;
	y=pt.y;
	z=pt.z;
// 	m_index1=pt.m_index1;
// 	m_index2=pt.m_index2;
}

CPoint3D::~CPoint3D()
{
	
}

const CPoint3D& CPoint3D::operator=(const CPoint3D& pt)
{
	x=pt.x;
	y=pt.y;
	z=pt.z;
// 	m_index1=pt.m_index1;
// 	m_index2=pt.m_index2;

	return *this;
}

void CPoint3D::operator = (double val)
{
	x=y=z=val;
}

CPoint3D CPoint3D::operator+(const CVector3D& v) const
{
	return CPoint3D(x+v.dx,y+v.dy,z+v.dz);
}

CPoint3D CPoint3D::operator+(const CPoint3D& pt) const
{
	return CPoint3D(x+pt.x,y+pt.y,z+pt.z); 
}

CPoint3D CPoint3D::operator *(const double& dFactor) const
{
	return CPoint3D(x*dFactor,y*dFactor,z*dFactor);
}

void CPoint3D::operator+=(const CVector3D& v)
{
	x+=v.dx;
	y+=v.dy;
	z+=v.dz;
}

CPoint3D CPoint3D::operator-(const CVector3D& v) const 
{
	return CPoint3D(x-v.dx,y-v.dy,z-v.dz);
}      

void CPoint3D::operator-=(const CVector3D& v)                 // point and vector operation
{
	x-=v.dx;
	y-=v.dy;
	z-=v.dz;
}

CVector3D CPoint3D::operator-(const CPoint3D& pt) const       // point minus point to get vector
{
	return CVector3D(x-pt.x,y-pt.y,z-pt.z);
}

BOOL CPoint3D::operator==(const CPoint3D& pt) const  // Complete the same 
{
	if (x!=pt.x) return FALSE;
	if (y!=pt.y) return FALSE;
	if (z!=pt.z) return FALSE;
	return TRUE;
}

BOOL CPoint3D::operator^=(const CPoint3D& pt) const  // More or less the same
{
	CVector3D vec(x-pt.x,y-pt.y,z-pt.z);
	double len=vec.GetMod2();
	if(len<=POINT_RESOLUTION) 
		return TRUE;
	else 
		return FALSE;
}

BOOL CPoint3D::operator!=(const CPoint3D& pt) const
{
	return !(*this==pt);
} 

CPoint3D CPoint3D::operator*(const CMatrix3D& matrix) const
{
	double rx, ry, rz, sc;
	rx=x*matrix.A[0][0]+y*matrix.A[1][0]+z*matrix.A[2][0]+matrix.A[3][0];
	ry=x*matrix.A[0][1]+y*matrix.A[1][1]+z*matrix.A[2][1]+matrix.A[3][1];
	rz=x*matrix.A[0][2]+y*matrix.A[1][2]+z*matrix.A[2][2]+matrix.A[3][2];
    sc=x*matrix.A[0][3]+y*matrix.A[1][3]+z*matrix.A[2][3]+matrix.A[3][3];
    rx/=sc;
	ry/=sc;
	rz/=sc;
	return CPoint3D(rx,ry,rz);
}   

void CPoint3D::operator*=(const CMatrix3D& matrix)            // point by matrix to get point   
{
	(*this)=(*this)*matrix;
}

void CPoint3D::ResetIndexes()
{
// 	m_index1=-1;
// 	m_index2=-1;
}

CPoint2D CPoint3D::ToPoint2D() const
{
	return CPoint2D(x,y);  // z is neglected
}


//////////////////////////////////////////////////////////////////////////
// CVector2D
//////////////////////////////////////////////////////////////////////////

CVector2D::CVector2D()
{
	dx=dy=0;	
}

CVector2D::~CVector2D()
{

}

CVector2D::CVector2D(double ix, double iy)
{
	dx=ix;
	dy=iy;
}

CVector2D::CVector2D(const double* p)
{
	dx=p[0];
	dy=p[1];
}

CVector2D::CVector2D(const CVector2D& v)
{
	dx=v.dx;
	dy=v.dy;
}

const CVector2D& CVector2D::operator=(const CVector2D& v)
{
	dx=v.dx;
	dy=v.dy;

	return *this;
}

BOOL CVector2D::operator==(const CVector2D& v) const
{
	if (dx==v.dx && dy==v.dy)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

CVector2D CVector2D::operator+(const CVector2D& v) const
{
	return CVector2D(dx+v.dx,dy+v.dy);
}

void CVector2D::operator+=(const CVector2D& v)
{
	dx+=v.dx;
	dy+=v.dy;
}

CVector2D CVector2D::operator-(const CVector2D& v) const
{
	return CVector2D(dx-v.dx,dy-v.dy);
}

void CVector2D::operator-=(const CVector2D& v)
{
	dx-=v.dx;
	dy-=v.dy;
}

CVector2D CVector2D::operator -()
{
	return CVector2D(-dx,-dy);
}

CVector2D CVector2D::operator*(const double& d) const
{
	return CVector2D(dx*d,dy*d);
}

void CVector2D::operator*=(const double& d)
{
	dx=dx*d;
	dy=dy*d;
}

CVector2D CVector2D::operator/(const double& d) const
{
	return CVector2D(dx/d,dy/d);
}

void CVector2D::operator/=(const double& d)
{
	dx=dx/d;
	dy=dy/d;
}

double CVector2D::operator|(const CVector2D& v) const      //点乘
{
	return dx*v.dx+dy*v.dy;
}


CVector2D CVector2D::FromPoint2D(const CPoint2D& pt)
{
	CVector2D vec;
	vec.dx=pt.x;
	vec.dy=pt.y;

	return vec;
}

double CVector2D::GetLength() const
{
	return sqrt(dx*dx + dy*dy);
}

double CVector2D::GetMod2() const
{
	return dx*dx + dy*dy;
}

double CVector2D::GetMod() const
{
	return sqrt(dx*dx + dy*dy);
}

CVector2D CVector2D::GetNormal() const
{
	double length;
	length=this->GetLength();
	return CVector2D(dx/length,dy/length);
}

CVector2D CVector2D::Normalize()
{
	double length;
	length=this->GetLength();
	if(0!=length)
	{	
		dx/=length;
		dy/=length;
	}
	else
	{
		dx=0;
		dy=0;
	}

	return *this;
}

double CVector2D::GetAngle(CVector2D v1, CVector2D v2)
{
	v1.Normalize();
	v2.Normalize();
	double dotProduct=v1|v2;
	if (dotProduct>1)
		dotProduct=1;
	if (dotProduct<-1)
		dotProduct=-1;

	return acos(v1|v2);
}

BOOL CVector2D::IsZeroVector() const
{
	if(0==this->GetLength())
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

CVector3D CVector2D::ToVector3D(double dzVal) const
{
	return CVector3D(dx,dy,dzVal);
}

//////////////////////////////////////////////////////////////////////
// 向量 3D
//////////////////////////////////////////////////////////////////////

CVector3D::CVector3D()
{
	dx=dy=dz=0;	
}

CVector3D::~CVector3D()
{
	
}

CVector3D::CVector3D(const double& ix,const double& iy,const double& iz)
{
	dx=ix;
	dy=iy;
	dz=iz;
}

CVector3D::CVector3D(const double* p)
{
	dx=p[0];
	dy=p[1];
	dz=p[2];
}

CVector3D::CVector3D(const CVector3D& v)
{
	dx=v.dx;
	dy=v.dy;
	dz=v.dz;
}

const CVector3D& CVector3D::operator=(const CVector3D& v)
{
	dx=v.dx;
	dy=v.dy;
	dz=v.dz;
	return *this;
}

BOOL CVector3D::operator==(const CVector3D& v) const
{
	if (dx==v.dx && dy==v.dy && dz==v.dz)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CVector3D::operator !=(const CVector3D& v) const
{
	return !(*this == v);
}

CVector3D CVector3D::operator+(const CVector3D& v) const
{
	return CVector3D(dx+v.dx,dy+v.dy,dz+v.dz);
}

void CVector3D::operator+=(const CVector3D& v)
{
	dx+=v.dx;
	dy+=v.dy;
	dz+=v.dz;
}

CVector3D CVector3D::operator-(const CVector3D& v) const
{
    return CVector3D(dx-v.dx,dy-v.dy,dz-v.dz);
}

void CVector3D::operator-=(const CVector3D& v)
{
	dx-=v.dx;
	dy-=v.dy;
	dz-=v.dz;
}

CVector3D CVector3D::operator -()
{
	return CVector3D(-dx,-dy,-dz);
}

CVector3D CVector3D::operator*(const double& d) const
{
	return CVector3D(dx*d,dy*d,dz*d);
}

void CVector3D::operator*=(const double& d)
{
    dx=dx*d;
	dy=dy*d;
	dz=dz*d;
}

CVector3D CVector3D::operator/(const double& d) const
{
    return CVector3D(dx/d,dy/d,dz/d);
}

void CVector3D::operator/=(const double& d)
{
	dx=dx/d;
	dy=dy/d;
	dz=dz/d;
}

CVector3D CVector3D::operator*(const CVector3D& v) const    //叉乘
{
	return CVector3D(dy*v.dz-dz*v.dy,dz*v.dx-dx*v.dz,dx*v.dy-dy*v.dx);
}

double CVector3D::operator|(const CVector3D& v) const      //点乘
{
	return dx*v.dx+dy*v.dy+dz*v.dz;
}

CVector3D CVector3D::operator*(const CMatrix3D& matrix) const
{
	// 2012-11-7
	// 向量在齐次矩阵里（dx, dy, dz, 0）
	// 对向量进行平移变换没有什么意义
	// 对向量进行缩放也没什么意义
	
	double rx, ry, rz;
	rx=dx*matrix.A[0][0]+dy*matrix.A[1][0]+dz*matrix.A[2][0];//+matrix.A[3][0];
	ry=dx*matrix.A[0][1]+dy*matrix.A[1][1]+dz*matrix.A[2][1];//+matrix.A[3][1];
	rz=dx*matrix.A[0][2]+dy*matrix.A[1][2]+dz*matrix.A[2][2];//+matrix.A[3][2];
//     sc=dx*matrix.A[0][3]+dy*matrix.A[1][3]+dz*matrix.A[2][3]+matrix.A[3][3];
//     rx/=sc;
// 	ry/=sc;
// 	rz/=sc;
	return CVector3D(rx,ry,rz);
}

void CVector3D::operator*=(const CMatrix3D& matrix)
{
	(*this)=(*this)*matrix;
}

CVector3D CVector3D::FromPoint3D(const CPoint3D& pt)
{
	CVector3D vec;
	vec.dx=pt.x;
	vec.dy=pt.y;
	vec.dz=pt.z;
	return vec;
}

double CVector3D::GetLength() const
{
	return sqrt(dx*dx + dy*dy + dz*dz);
}

double CVector3D::GetMod2() const
{
	return dx*dx + dy*dy + dz*dz;
}

double CVector3D::GetMod() const
{
	return sqrt(dx*dx + dy*dy + dz*dz);
}

double CVector3D::GetLengthXY() const
{
	return sqrt(dx*dx+dy*dy);
}

double CVector3D::GetLengthYZ() const
{
	return sqrt(dy*dy+dz*dz);
}

double CVector3D::GetLengthZX() const 
{
	return sqrt(dz*dz+dx*dx);
} 

CVector3D CVector3D::Reverse()
{
	dx = -dx;
	dy = -dy;
	dz = -dz;
	return *this;
}

CVector3D CVector3D::GetNormal() const
{
	double length;
	length=this->GetLength();
	return CVector3D(dx/length,dy/length,dz/length);
}

CVector3D CVector3D::Normalize()
{
	double length;
	length=this->GetLength();
	if(0!=length)
	{	
		dx/=length;
	    dy/=length;
	    dz/=length;
	}
	else
	{
	    dx=0;
		dy=0;
		dz=0;
	}

	return *this;
}

double CVector3D::GetAngle(CVector3D v1, CVector3D v2)
{
	v1.Normalize();
	v2.Normalize();
	double dotProduct=v1|v2;
	if (dotProduct>1)
		dotProduct=1;
	if (dotProduct<-1)
		dotProduct=-1;

	return acos(v1|v2);
}

BOOL CVector3D::IsZeroVector() const
{
	if(0==this->GetLength())
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

CVector2D CVector3D::ToVector2D() const
{
	return CVector2D(dx,dy);
};

//////////////////////////////////////////////////////////////////////
// 矩阵
//////////////////////////////////////////////////////////////////////

CMatrix3D::CMatrix3D()
{
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
		{
			this->A[i][j]=0;
		}
	}
	for(int i=0;i<4;i++)
		this->A[i][i]=1;	
}

CMatrix3D::~CMatrix3D()
{
	
}

CMatrix3D::CMatrix3D(const CMatrix3D& matrix)
{
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
		{
			A[i][j]=matrix.A[i][j];
		}
	}
}

CMatrix3D::CMatrix3D(const double* p)
{
	for(int i=0;i<4;i++)
	{
		for(int j=0;i<4;j++)
		{
			A[i][j]=p[i*4+j];
		}
	}
}

const CMatrix3D& CMatrix3D::operator=(const CMatrix3D& matrix)
{
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
		{
			A[i][j]=matrix.A[i][j];
		}
	}
	return *this;
}

CMatrix3D CMatrix3D::operator*(const CMatrix3D& matrix) const
{
	CMatrix3D matrixTemp;
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
		{
			matrixTemp.A[i][j]=A[i][0]*matrix.A[0][j]
				+A[i][1]*matrix.A[1][j]
				+A[i][2]*matrix.A[2][j]
				+A[i][3]*matrix.A[3][j];
		}
	}
	return matrixTemp;
}

void CMatrix3D::operator*=(const CMatrix3D& matrix)
{
	(*this)=(*this)*matrix;
}


void CMatrix3D::IdenticalMatrix()
{
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
		{
			this->A[i][j]=0;
		}
	}
	for(int i=0;i<4;i++)
		this->A[i][i]=1;
}

double CMatrix3D::GetDet(BOOL bHomogeneous/* =FALSE */) const
{
	if (FALSE==bHomogeneous)
	{
		return A[0][0]*A[1][1]*A[2][2]+
			A[0][1]*A[1][2]*A[2][0]+
			A[0][2]*A[1][0]*A[2][1]+
			A[0][2]*A[1][1]*A[2][0]-
			A[0][1]*A[1][0]*A[2][2]-
			A[0][0]*A[1][2]*A[2][1];

	}
	else
	{
		return  A[0][3]* A[1][2]* A[2][1]* A[3][0]-
				A[0][2]* A[1][3]* A[2][1]* A[3][0]-
				A[0][3]* A[1][1]* A[2][2]* A[3][0]+
				A[0][1]* A[1][3]* A[2][2]* A[3][0]+
				A[0][2]* A[1][1]* A[2][3]* A[3][0]-
				A[0][1]* A[1][2]* A[2][3]* A[3][0]-
				A[0][3]* A[1][2]* A[2][0]* A[3][1]+
				A[0][2]* A[1][3]* A[2][0]* A[3][1]+
				A[0][3]* A[1][0]* A[2][2]* A[3][1]-
				A[0][0]* A[1][3]* A[2][2]* A[3][1]-
				A[0][2]* A[1][0]* A[2][3]* A[3][1]+
				A[0][0]* A[1][2]* A[2][3]* A[3][1]+
				A[0][3]* A[1][1]* A[2][0]* A[3][2]-
				A[0][1]* A[1][3]* A[2][0]* A[3][2]-
				A[0][3]* A[1][0]* A[2][1]* A[3][2]+
				A[0][0]* A[1][3]* A[2][1]* A[3][2]+
				A[0][1]* A[1][0]* A[2][3]* A[3][2]-
				A[0][0]* A[1][1]* A[2][3]* A[3][2]-
				A[0][2]* A[1][1]* A[2][0]* A[3][3]+
				A[0][1]* A[1][2]* A[2][0]* A[3][3]+
				A[0][2]* A[1][0]* A[2][1]* A[3][3]-
				A[0][0]* A[1][2]* A[2][1]* A[3][3]-
				A[0][1]* A[1][0]* A[2][2]* A[3][3]+
				A[0][0]* A[1][1]* A[2][2]* A[3][3];
	}
}

double CMatrix3D::GetDet(const double& a00,const double& a01,const double& a02,
						   const double& a10,const double& a11,const double& a12,
						   const  double& a20,const double& a21,const double& a22)
{
	return  a00*a11*a22+
		    a01*a12*a20+
		    a02*a10*a21+
		    a02*a11*a20-
		    a01*a10*a22-
		    a00*a12*a21;
}

// 默认情况下是计算非其次（3x3）的逆矩阵
CMatrix3D CMatrix3D::GetReverseMatrix(BOOL bHomogeneous/* =FALSE */)
{
	CMatrix3D m;
	double det=GetDet(bHomogeneous);

	if (FALSE==bHomogeneous)
	{
		m.A[0][0]=(A[1][1]*A[2][2]-A[1][2]*A[2][1])/det;
		m.A[0][1]=(A[0][2]*A[2][1]-A[0][1]*A[2][2])/det;
		m.A[0][2]=(A[0][1]*A[1][2]-A[0][2]*A[1][1])/det;
		m.A[1][0]=(A[1][2]*A[2][0]-A[1][0]*A[2][2])/det;
		m.A[1][1]=(A[0][0]*A[2][2]-A[0][2]*A[2][0])/det;
		m.A[1][2]=(A[0][2]*A[1][0]-A[0][0]*A[1][2])/det;
		m.A[2][0]=(A[1][0]*A[2][1]-A[1][1]*A[2][0])/det;
		m.A[2][1]=(A[0][1]*A[2][0]-A[0][0]*A[2][1])/det;
		m.A[2][2]=(A[0][0]*A[1][1]-A[0][1]*A[1][0])/det;
	}
	else
	{
		m.A[0][0] = (-A[1][3]*A[2][2]*A[3][1] + A[1][2]*A[2][3]*A[3][1] + A[1][3]*A[2][1]*A[3][2] - A[1][1]*A[2][3]*A[3][2] - A[1][2]*A[2][1]*A[3][3] + A[1][1]*A[2][2]*A[3][3])/det;
		m.A[0][1] = ( A[0][3]*A[2][2]*A[3][1] - A[0][2]*A[2][3]*A[3][1] - A[0][3]*A[2][1]*A[3][2] + A[0][1]*A[2][3]*A[3][2] + A[0][2]*A[2][1]*A[3][3] - A[0][1]*A[2][2]*A[3][3])/det;
		m.A[0][2] = (-A[0][3]*A[1][2]*A[3][1] + A[0][2]*A[1][3]*A[3][1] + A[0][3]*A[1][1]*A[3][2] - A[0][1]*A[1][3]*A[3][2] - A[0][2]*A[1][1]*A[3][3] + A[0][1]*A[1][2]*A[3][3])/det;
		m.A[0][3] = ( A[0][3]*A[1][2]*A[2][1] - A[0][2]*A[1][3]*A[2][1] - A[0][3]*A[1][1]*A[2][2] + A[0][1]*A[1][3]*A[2][2] + A[0][2]*A[1][1]*A[2][3] - A[0][1]*A[1][2]*A[2][3])/det;
		m.A[1][0] =  (A[1][3]*A[2][2]*A[3][0] - A[1][2]*A[2][3]*A[3][0] - A[1][3]*A[2][0]*A[3][2] + A[1][0]*A[2][3]*A[3][2] + A[1][2]*A[2][0]*A[3][3] - A[1][0]*A[2][2]*A[3][3])/det;
		m.A[1][1] = (-A[0][3]*A[2][2]*A[3][0] + A[0][2]*A[2][3]*A[3][0] + A[0][3]*A[2][0]*A[3][2] - A[0][0]*A[2][3]*A[3][2] - A[0][2]*A[2][0]*A[3][3] + A[0][0]*A[2][2]*A[3][3])/det;
		m.A[1][2] = ( A[0][3]*A[1][2]*A[3][0] - A[0][2]*A[1][3]*A[3][0] - A[0][3]*A[1][0]*A[3][2] + A[0][0]*A[1][3]*A[3][2] + A[0][2]*A[1][0]*A[3][3] - A[0][0]*A[1][2]*A[3][3])/det;
		m.A[1][3] = (-A[0][3]*A[1][2]*A[2][0] + A[0][2]*A[1][3]*A[2][0] + A[0][3]*A[1][0]*A[2][2] - A[0][0]*A[1][3]*A[2][2] - A[0][2]*A[1][0]*A[2][3] + A[0][0]*A[1][2]*A[2][3])/det;
		m.A[2][0] = (-A[1][3]*A[2][1]*A[3][0] + A[1][1]*A[2][3]*A[3][0] + A[1][3]*A[2][0]*A[3][1] - A[1][0]*A[2][3]*A[3][1] - A[1][1]*A[2][0]*A[3][3] + A[1][0]*A[2][1]*A[3][3])/det;
		m.A[2][1] = ( A[0][3]*A[2][1]*A[3][0] - A[0][1]*A[2][3]*A[3][0] - A[0][3]*A[2][0]*A[3][1] + A[0][0]*A[2][3]*A[3][1] + A[0][1]*A[2][0]*A[3][3] - A[0][0]*A[2][1]*A[3][3])/det;
		m.A[2][2] = (-A[0][3]*A[1][1]*A[3][0] + A[0][1]*A[1][3]*A[3][0] + A[0][3]*A[1][0]*A[3][1] - A[0][0]*A[1][3]*A[3][1] - A[0][1]*A[1][0]*A[3][3] + A[0][0]*A[1][1]*A[3][3])/det;
		m.A[2][3] = ( A[0][3]*A[1][1]*A[2][0] - A[0][1]*A[1][3]*A[2][0] - A[0][3]*A[1][0]*A[2][1] + A[0][0]*A[1][3]*A[2][1] + A[0][1]*A[1][0]*A[2][3] - A[0][0]*A[1][1]*A[2][3])/det;
		m.A[3][0] =  (A[1][2]*A[2][1]*A[3][0] - A[1][1]*A[2][2]*A[3][0] - A[1][2]*A[2][0]*A[3][1] + A[1][0]*A[2][2]*A[3][1] + A[1][1]*A[2][0]*A[3][2] - A[1][0]*A[2][1]*A[3][2])/det;
		m.A[3][1] = (-A[0][2]*A[2][1]*A[3][0] + A[0][1]*A[2][2]*A[3][0] + A[0][2]*A[2][0]*A[3][1] - A[0][0]*A[2][2]*A[3][1] - A[0][1]*A[2][0]*A[3][2] + A[0][0]*A[2][1]*A[3][2])/det;
		m.A[3][2] = ( A[0][2]*A[1][1]*A[3][0] - A[0][1]*A[1][2]*A[3][0] - A[0][2]*A[1][0]*A[3][1] + A[0][0]*A[1][2]*A[3][1] + A[0][1]*A[1][0]*A[3][2] - A[0][0]*A[1][1]*A[3][2])/det;
		m.A[3][3] = (-A[0][2]*A[1][1]*A[2][0] + A[0][1]*A[1][2]*A[2][0] + A[0][2]*A[1][0]*A[2][1] - A[0][0]*A[1][2]*A[2][1] - A[0][1]*A[1][0]*A[2][2] + A[0][0]*A[1][1]*A[2][2])/det;
	}

	return m;
}

CMatrix3D CMatrix3D::CreateMirrorMatrix(CVector3D planeNorm)     
{
	planeNorm.Normalize();
	CVector3D z(0,0,1);
	CVector3D b;
	b=planeNorm*z;
	double alphe;
	alphe=acos(planeNorm|z);

	CMatrix3D m1=CMatrix3D::CreateRotateMatrix(alphe,b);
	CMatrix3D mXOY;
	mXOY.A[2][2]=-1;
	CMatrix3D m2=CMatrix3D::CreateRotateMatrix(-alphe,b);

	CMatrix3D m;
	m=m1*mXOY*m2;
	return m;
}

CMatrix3D CMatrix3D::CreateRotateMatrix(const double& angle,CVector3D axis)
{
	CMatrix3D m;
	if(0==angle)
		return m;

	if(0==axis.GetLength())
		return m;

	axis.Normalize();
	double cosa=cos(angle);
	double sina=sin(angle);

	if (CVector3D(1,0,0)==axis)
	{
		m.A[1][1]=cosa;
		m.A[1][2]=sina;
		m.A[2][1]=-sina;
		m.A[2][2]=cosa;  //x
		return m;
	}

	if (CVector3D(0,1,0)==axis)
	{
		m.A[0][0]=cosa;
		m.A[0][2]=-sina;
		m.A[2][0]=sina;
		m.A[2][2]=cosa;  //y
		return m;
	}

	if (CVector3D(0,0,1)==axis)
	{
		m.A[0][0]=cosa;
		m.A[0][1]=sina;
		m.A[1][0]=-sina;
		m.A[1][1]=cosa;  //z
		return m;
	}

	double minuscosa=1-cosa;
    m.A[0][0]=cosa+minuscosa*axis.dx*axis.dx;
	m.A[1][0]=minuscosa*axis.dx*axis.dy-axis.dz*sina;
    m.A[2][0]=minuscosa*axis.dx*axis.dz+axis.dy*sina;
	m.A[0][1]=minuscosa*axis.dx*axis.dy+axis.dz*sina;
	m.A[1][1]=minuscosa*axis.dy*axis.dy+cosa;
	m.A[2][1]=minuscosa*axis.dy*axis.dz-axis.dx*sina;
	m.A[0][2]=minuscosa*axis.dx*axis.dz-axis.dy*sina;
	m.A[1][2]=minuscosa*axis.dy*axis.dz+axis.dx*sina;
	m.A[2][2]=minuscosa*axis.dz*axis.dz+cosa;

	return m;
}

CMatrix3D CMatrix3D::CreateRotateMatrix(const double &angle, int nAxis)
{
	CMatrix3D m;
	double cosa=cos(angle);
	double sina=sin(angle);

	if (AXIS_X==nAxis)
	{
		m.A[1][1]=cosa;
		m.A[1][2]=sina;
		m.A[2][1]=-sina;
		m.A[2][2]=cosa;  //x
		return m;
	}
	else if (AXIS_Y==nAxis)
	{
		m.A[0][0]=cosa;
		m.A[0][2]=-sina;
		m.A[2][0]=sina;
		m.A[2][2]=cosa;  //y
		return m;
	}
	else if (AXIS_Z==nAxis)
	{
		m.A[0][0]=cosa;
		m.A[0][1]=sina;
		m.A[1][0]=-sina;
		m.A[1][1]=cosa;  //z
		return m;
	}
	else
	{
		return m;
	}
}

CMatrix3D CMatrix3D::CreateScaleMatrix(const double& xyzAxis)
{
	CMatrix3D m;
	m.A[0][0]=m.A[1][1]=m.A[2][2]=xyzAxis;
	return m;
}

CMatrix3D CMatrix3D::CreateScaleMatrix(const double& xAxis,const double& yAxis,const double& zAxis)
{
	CMatrix3D m;
	m.A[0][0]=xAxis;
	m.A[1][1]=yAxis;
	m.A[2][2]=zAxis;
	return m;
}

CMatrix3D CMatrix3D::CreateTransferMatrix(const CVector3D& v)
{
	CMatrix3D m;
	m.A[3][0]=v.dx;
	m.A[3][1]=v.dy;
	m.A[3][2]=v.dz;
	return m;
}

//////////////////////////////////////////////////////////////////////////
CRect3D::CRect3D()
{

}

CRect3D::~CRect3D()
{

}

CRect3D::CRect3D(const CRect3D& rt)
{
	A=rt.A; B=rt.B; C=rt.C; D=rt.D;
}
CRect3D::CRect3D(const CPoint3D& A, const CPoint3D& B, const CPoint3D& C, const CPoint3D& D)
{
	this->A=A; this->B=B; this->C=C; this->D=D;
}

void CRect3D::MakeABLength()
{
	double lenAB = GetDistance(A,B);
	double lenCD = GetDistance(B,C);
	if (lenAB<lenCD)
	{
		CPoint3D pt[4];
		pt[0]=B; pt[1]=C; pt[2]=D; pt[3]=A;
		A=pt[0]; B=pt[1]; C=pt[2]; D=pt[3];
	}
}

CRect3D CRect3D::Shrink(double percent)
{
	CPoint3D O = (A+C)*0.5;
	CVector3D OA = (A-O)*(1-percent);
	CVector3D OB = (B-O)*(1-percent);
	CVector3D OC = (C-O)*(1-percent);
	CVector3D OD = (D-O)*(1-percent);

	return CRect3D(O+OA, O+OB, O+OC, O+OD);
}

bool CRect3D::IsPtInRect(CPoint3D pt)
{
	CVector3D PA = A-pt;
	CVector3D PB = B-pt;
	CVector3D PC = C-pt;
	CVector3D PD = D-pt;

	CVector3D V1 = PA*PB;
	CVector3D V2 = PB*PC;
	CVector3D V3 = PC*PD;
	CVector3D V4 = PD*PA;

	double d1 = V1|V2;
	double d2 = V1|V3;
	double d3 = V1|V4;
	if (d1*d2>0 && d2*d3>0 && d1*d3>0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool CRect3D::IsPtOnBorder(CPoint3D pt)
{
	CVector3D PA = A-pt;
	CVector3D PB = B-pt;
	CVector3D PC = C-pt;
	CVector3D PD = D-pt;

	if (PA*PB == ZERO_VECTOR3D)
	{
		return true;
	}
	else if (PB*PC == ZERO_VECTOR3D)
	{
		return true;
	}
	else if (PC*PD == ZERO_VECTOR3D)
	{
		return true;
	}
	else if (PC*PD == ZERO_VECTOR3D)
	{
		return true;
	}
	else 
	{
		return false;
	}
}

bool CRect3D::IsPtOutRect(CPoint3D pt)
{
	if (IsPtInRect(pt))
	{
		return false;
	}
	else if (IsPtOnBorder(pt))
	{
		return false;
	}
	else
	{
		return true;
	}
}

COBBox::COBBox()
{
	x_min=x_max=y_min=y_max=z_min=z_max=0;
}

CPoint3D COBBox::GetCenterPt() const
{
	return CPoint3D((x_min+x_max)/2,(y_min+y_max)/2,(z_min+z_max)/2);
}

void COBBox::Init(double xMin, double xMax, double yMin, double yMax, double zMin, double zMax)
{
	x_min=xMin;
	x_max=xMax;
	y_min=yMin;
	y_max=yMax;
	z_min=zMin;
	z_max=zMax;
}

bool COBBox::IsBoxIn(const COBBox& box)
{
	if (x_min >= box.x_min && x_max <= box.x_max 
	 && y_min >= box.y_min && y_max <= box.y_max
	 && z_min >= box.z_min && z_max <= box.z_max)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void COBBox::Shrink(double ratio)
{
	double lenX = x_max - x_min;
	double lenY = y_max - y_min;
	double lenZ = z_max - z_min;
	double delta_x = lenX*ratio/2.0;
	double delta_y = lenY*ratio/2.0;
	double delta_z = lenZ*ratio/2.0;

	x_max += delta_x;
	x_min -= delta_x;
	y_max += delta_y;
	y_min -= delta_y;
	z_max += delta_z;
	z_min -= delta_z;
}

//////////////////////////////////////////////////////////////////////
// 输出函数
//////////////////////////////////////////////////////////////////////
double GetAngle(CVector3D v1,CVector3D v2, bool bNormalized /*= false*/)
{
	if (false==bNormalized)
	{
		v1.Normalize();
		v2.Normalize();
	}
	return Angle(v1,v2);
}

double Angle(const CVector3D& v1, const CVector3D& v2)
{
	double dotProduct;
	dotProduct=v1|v2;
	if (dotProduct>1) dotProduct=1;
	if (dotProduct<-1) dotProduct=-1;
	return acos(dotProduct);
}

double GetAngle(const CVector3D& v)
{
	CVector3D v_x(1,0,0);
	double angle=GetAngle(v,v_x);
	if (v.dy<0)
		angle=TWOPI-angle;

	return angle;
}

double GetDistance2(const CPoint3D& pt1,const CPoint3D& pt2)
{
	CVector3D vec=pt1-pt2;
	return vec.GetMod2();
}

double GetDistance(const CPoint3D& pt1,const CPoint3D& pt2)
{
	CVector3D vec=pt1-pt2;
	return vec.GetLength();
}

bool IsParallel(const CVector3D& v1,const CVector3D& v2)
{
	if(0.0==(v1*v2).dz) 
		return TRUE;
	else
		return FALSE;
}

bool IsOrhogonal(const CVector3D& v1,const CVector3D& v2)
{
	if(0.0==(v1|v2))
		return TRUE;
	else
		return FALSE;
}

double Round(const double& d, const int& iHowManyAfterDot)  // 保留5位小数
{
	if (5==iHowManyAfterDot)
	{
		int i=(int)((d+0.000005)*100000);
		return (double)i/(double)100000;
	}
	else
	{
		CString str;

		str=_T("1");
		for (int i=0;i<iHowManyAfterDot;i++)
		{
			str+=_T("0");
		}
		int t=_wtoi(str);

		str=_T("0.5");
		for (int i=0;i<iHowManyAfterDot;i++)
		{
			str.Insert(2,'0');
		}
		double dot=_wtof(str);

		int i=(int)((d+dot)*t);
		return (double)i/(double)t;
	}
}

 bool Is2SegsCross(const CPoint3D& ptS_seg1,const CPoint3D& ptE_seg1,const CPoint3D& ptS_seg2,const CPoint3D& ptE_seg2) // 两条直线重合也是干涉的
{   // 干涉返回TURE
	CPoint3D ptCross;
	int state=GetCrossPoint(ptCross,ptS_seg1,ptE_seg1,ptS_seg2,ptE_seg2);
	if (0==state) return FALSE;// 0平行，1重合，2相交
	if (1==state) return TRUE; 
		
	CVector3D vec1,vec2,vec3,vec4;
	vec1=ptS_seg1-ptCross;
	vec2=ptE_seg1-ptCross;
	vec3=ptS_seg2-ptCross;
	vec4=ptE_seg2-ptCross;
	if ((vec1|vec2)<=0 && (vec3|vec4)<=0)
		return TRUE;
	else
		return FALSE;
}

bool Is2SegsCross(CPoint3D& ptCross,const CPoint3D& ptS_seg1,const CPoint3D& ptE_seg1,const CPoint3D& ptS_seg2,const CPoint3D& ptE_seg2) // 两条直线重合也是干涉的
{   // 干涉返回TURE
	int state=GetCrossPoint(ptCross,ptS_seg1,ptE_seg1,ptS_seg2,ptE_seg2);
	if (0==state) return FALSE;// 0平行，1重合，2相交
	if (1==state) return TRUE; 

	CVector3D vec1,vec2,vec3,vec4;
	vec1=ptS_seg1-ptCross;
	vec2=ptE_seg1-ptCross;
	vec3=ptS_seg2-ptCross;
	vec4=ptE_seg2-ptCross;
	if ((vec1|vec2)<=0 && (vec3|vec4)<=0) 
		return TRUE;
	else 
		return FALSE;
}

// 0平行，1重合，2相交  
// 2011/5/21 
// 修改了两条直线求交点的函数，原来使用的是两直线方程，问题在于当直线竖直时会出现问题。现在用向量的方法
int GetCrossPoint(CPoint3D& pt_out,
				  const CPoint3D& ptS_line1,
				  const CPoint3D& ptE_line1,
				  const CPoint3D& ptS_line2,
				  const CPoint3D& ptE_line2) // 对于两条相交直线求交点
{
	CVector3D v1=ptE_line1-ptS_line1;
	CVector3D v2=ptE_line2-ptS_line2;
	if (0==(v1*v2).dz)
	{
		CVector3D v3=ptE_line1-ptE_line2;
		if (0==(v3*v1).dz) 
			return TWO_LINES_COINCIDE;
		else 
			return TWO_LINES_PARALLEL;
	}
	double lamda=((ptS_line2.x-ptS_line1.x)*v2.dy-(ptS_line2.y-ptS_line1.y)*v2.dx)/(v1.dx*v2.dy-v1.dy*v2.dx);
	pt_out=ptS_line1+v1*lamda;
	return TWO_LINES_CROSS;
}

CVector3D GetOrthoVectorOnXYPlane(const CPoint3D& ptS, const CPoint3D& ptE) 
{
	CVector3D vec=ptE-ptS;
	CVector3D vec_r;
	if (0==vec.dx) 
		vec_r=CVector3D(1,0,0);
	else 
		vec_r=CVector3D(-(vec.dy/vec.dx),1,0);

	if ((vec_r*vec).dz>0)  // 指向左边的
		vec_r=vec_r*(-1);  // 减小计算量 2011/4/26

	return vec_r;
}

CVector3D GetOrthoVectorOnXYPlane(const CVector3D& vec)
{
	CVector3D vec_r;
	if (0==vec.dx)
		vec_r=CVector3D(1,0,0);
	else
		vec_r=CVector3D(-(vec.dy/vec.dx),1,0);

	if ((vec_r*vec).dz>0)
		vec_r=vec_r*(-1);

	return vec_r;
}

CVector3D GetOrthoVector(const CVector3D& vec)
{
	if (0!=vec.dx)
	{
		return CVector3D(-vec.dy/vec.dx,1,0);
	}
	else if (0!=vec.dy)
	{
		return CVector3D(1,-vec.dx/vec.dy,0);
	}
	else
	{
		return CVector3D(1,1,0);
	}
}

double GetDist2PtToSeg(const CPoint3D& pt, 
					   const CPoint3D& ptS_seg, 
					   const CPoint3D& ptE_seg)
{
	if (ptS_seg==ptE_seg) return (pt-ptS_seg).GetMod2();

	double lamda;
	CVector3D vec_P1P=pt-ptS_seg;
	CVector3D vec_P1P2=ptE_seg-ptS_seg;
	lamda=(vec_P1P|vec_P1P2)/vec_P1P2.GetMod2();

	if (lamda<0)
	{
		double x=vec_P1P.GetMod2();
		return vec_P1P.GetMod2();
	}
	if (lamda>=0 && lamda<=1)
	{
		CPoint3D pt_=ptS_seg+vec_P1P2*lamda;
		CVector3D vec=pt_-pt;
		return vec.GetMod2();
	}
	if (lamda>1)
	{
		CVector3D vec=ptE_seg-pt;
		return vec.GetMod2();
	}

	return 0.0;
}

double GetDistPtToSeg(const CPoint3D& pt, 
					  const CPoint3D& ptS_seg, 
					  const CPoint3D& ptE_seg)
{
	return sqrt(GetDist2PtToSeg(pt,ptS_seg,ptE_seg));
}

double GetDist2PtToLine(const CPoint3D& pt, 
					   const CPoint3D& pt1_line, 
					   const CPoint3D& pt2_line)
{
	CVector3D vec_P1P=pt-pt1_line;
	CVector3D vec_P1P2=pt2_line-pt1_line;
	double lamda=(vec_P1P|vec_P1P2)/vec_P1P2.GetMod2();
	CPoint3D pt_shadow=pt1_line+vec_P1P2*lamda;
	return GetDistance2(pt,pt_shadow);
}

double GetDistPtToLine(const CPoint3D& pt, 
						const CPoint3D& pt1_line, 
						const CPoint3D& pt2_line)
{
	return sqrt(GetDist2PtToLine(pt,pt1_line,pt2_line));
}

double rad2deg(double angle_rad)
{
	return 180.0*angle_rad/PI;
}

double deg2rad(double angle_deg)
{
	return angle_deg*PI/180.0;
}


bool Is3PtInLine(const CPoint3D& pt1, 
				 const CPoint3D& pt2, 
				 const CPoint3D& pt3)
{
	CVector3D v1,v2;
	v1=pt1-pt2;
	v2=pt3-pt2;

	double r=(v1*v2).dz;
	if (r<0) r=-r;

	if (r<0.0000000001) return TRUE;
	else return FALSE;
}

bool IsPtOnSeg(const CPoint3D& pt, const CPoint3D& ptS_seg, const CPoint3D& ptE_seg)
{
	CVector3D vec1=pt-ptS_seg;
	CVector3D vec2=pt-ptE_seg;

	if ((vec1|vec2)<=0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}	
}
