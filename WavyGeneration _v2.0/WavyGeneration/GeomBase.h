// CadBase.h: interface for the CadBase class.
// 
//////////////////////////////////////////////////////////////////////

// By Lin 2010...

// History:

// 2011-6-3
// �����˵���ӵĺ��������������������ֱ���ӡ�Ҳ�����˵��ʵ����˲�����������ÿ�������ʵ����
// ----------------------------------------------------------------------------------------------
// 2011-5-25:
// �޸��� GetAngle() ������ԭ������ acos(double d) �������� d ��ȡֵ��Χ�� [-1,1], ������ʵ������
// ����������˵�ʱ��ż������ִ��� 1 ������������͵����˺�������Ĵ��󣬼���ֵ��Ч��
// ----------------------------------------------------------------------------------------------
// 2011-4-24:
// �� CPoint3D ��������һ����Ա���� w����ΪԤ��������������Ϊ�õ㵽ĳ��ľ��룬Ҳ����Ϊ�������ж�
// �õ��Ƿ���Ч��
// ----------------------------------------------------------------------------------------------
// 2013-6-9:
// ������CPoint2D, CVector2Dƽ����������

#pragma once

#include <math.h>


#ifndef PI
#define PI			 3.1415926535897932384626433832795
#define TWOPI		 2.0*PI
#define HALFPI		 PI/2.0
#define ONEHALFPI    1.5*PI  
#endif

// ZERO marco is defined to compare double number
#define ZERO      0.0      
#define ONE       1.0

#define  POINT_RESOLUTION     1e-8 * 1e-8


typedef CArray<int,int>        intAry;
typedef CArray<double,double>  doubleAry;

class CPoint2D;
class CVector2D;
class CPoint3D;     ////////////////////////////////////////////////
class CVector3D;    // �����������Ҫ�������޷�����
class CMatrix3D;    ////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
// ��
//////////////////////////////////////////////////////////////////////////
class AFX_CLASS_EXPORT CPoint2D
{
public:     ////// constructor
	CPoint2D();											// ���캯�����������                                           
	CPoint2D(double ix,double iy);		// ���캯��
	CPoint2D(const double* p);							// �����鹹�캯��
	CPoint2D(const CPoint2D& pt);						 // �õ㹹��
	virtual ~CPoint2D();								 // ��������           

public:     ////// operator
	const CPoint2D& operator=(const CPoint2D& pt);
	void operator=(double val);
	CPoint2D operator+(const CVector2D& v) const;        // ���������ӣ����ص�, const ��������ʾ�ú������ܸı����ĳ�Ա����
	CPoint2D operator+(const CPoint2D& pt) const;
	void operator+=(const CVector2D& v);                 // ��������ӵ�
	CPoint2D operator-(const CVector2D& v) const;        // ���������������ص�
	void operator-=(const CVector2D& v);                 // �����������
	CVector2D operator-(const CPoint2D& pt) const;       // ���ȥ��õ�����
	BOOL operator==(const CPoint2D& pt) const;           // Complete the same
	BOOL operator^=(const CPoint2D& pt) const;           // More less the same ~==
	BOOL operator!=(const CPoint2D& pt) const;           // �Ƚϲ�����
	CPoint2D operator*(const double& dFactor) const;

	CPoint3D ToPoint3D(double zVal=0) const;

public:
	double x;
	double y;
};


class  AFX_CLASS_EXPORT CPoint3D:public CObject
{
public:     ////// constructor
	CPoint3D();                                  // ���캯�����������                                           
	CPoint3D(double ix, double iy, double iz);     // ���캯��
	CPoint3D(const double* p);                   // �����鹹�캯��
	CPoint3D(const CPoint3D& pt);                        // �õ㹹��
	virtual ~CPoint3D();                         // ��������           
	
public:     ////// operator
	const CPoint3D& operator=(const CPoint3D& pt);
	void operator=(double val);
	CPoint3D operator+(const CVector3D& v) const;        // ���������ӣ����ص�, const ��������ʾ�ú������ܸı����ĳ�Ա����
	CPoint3D operator+(const CPoint3D& pt) const;
	void operator+=(const CVector3D& v);                 // ��������ӵ�
	CPoint3D operator-(const CVector3D& v) const;        // ���������������ص�
	void operator-=(const CVector3D& v);                 // �����������
	CVector3D operator-(const CPoint3D& pt) const;       // ���ȥ��õ�����
	BOOL operator==(const CPoint3D& pt) const;           // Complete the same
	BOOL operator^=(const CPoint3D& pt) const;           // More less the same ~==
	BOOL operator!=(const CPoint3D& pt) const;           // �Ƚϲ�����
	CPoint3D operator*(const CMatrix3D& matrix) const;   // ��;�����˵õ��㣬��ʵ���Ǳ任����
	CPoint3D operator*(const double& dFactor) const;
	void operator*=(const CMatrix3D& matrix);            // ����Ծ���
	void ResetIndexes();  // Reset indexes to -1

	CPoint2D ToPoint2D() const;

public:
	double x;
	double y;
	double z;
	//int    m_index1;      // I feel it necessary to add a flag member to the CPoint3D class
	//int    m_index2;      // And I aso feel it necessary to add another member to memrize the sequence of the point if it's in an array
};
typedef CTypedPtrArray<CObArray,CPoint3D*> Point3DAry;   


//////////////////////////////////////////////////////////////////////////
// ����
//////////////////////////////////////////////////////////////////////////

class AFX_CLASS_EXPORT CVector2D
{
public:
	CVector2D();                                         // ���캯��������������
	CVector2D(double ix, double iy);        // ���캯��
	CVector2D(const double* p);                          // ���캯���������鹹��
	CVector2D(const CVector2D& v);                       // ���캯��������������
	virtual ~CVector2D();                                // ��������

public:
	const CVector2D& operator=(const CVector2D& v);
	BOOL operator==(const CVector2D& v) const;
	CVector2D operator+(const CVector2D& v) const;       // ������������ӵõ�����
	void operator+=(const CVector2D& v);                 // �����������ӵ�
	CVector2D operator-(const CVector2D& v) const;       // ��������������õ�����
	void operator-=(const CVector2D& v);                 // ��������������
	CVector2D operator-();                               // ���ţ�ȡ������
	CVector2D operator*(const double& d) const;          // ���������˵õ�����
	void operator*=(const double& d);                    // ���������˵�
	CVector2D operator/(const double& d) const;          // �����������õ�����
	void operator/=(const double& d);                    // ������������
	double operator|(const CVector2D& v) const;          // �������������


	static CVector2D FromPoint2D(const CPoint2D& pt);
	static double GetAngle(CVector2D v1,CVector2D v2);   // ȡ�������ļн�

	double GetLength() const;                            // ����ƽ��
	double GetMod2() const;                              // ����ģ��ƽ��
	double GetMod() const;								 // ����ģ

	CVector2D GetNormal() const;                         // �õ������ĵ�λ���������ص�λ����
	CVector2D Normalize();                               // ������λ��
	BOOL IsZeroVector() const;                           // �ж�����ģ�ǲ�����

	CVector3D ToVector3D(double dzVal) const;

public:
	double dx;
	double dy;
};


class AFX_CLASS_EXPORT CVector3D:public CObject
{
public:
	CVector3D();                                         // ���캯��������������
	CVector3D(const double& ix,const double& iy,const double& iz);   // ���캯��
	CVector3D(const double* p);                          // ���캯���������鹹��
	CVector3D(const CVector3D& v);                       // ���캯��������������
	virtual ~CVector3D();                                // ��������
	
public:
    const CVector3D& operator=(const CVector3D& v);
	BOOL operator==(const CVector3D& v) const;
	CVector3D operator+(const CVector3D& v) const;       // ������������ӵõ�����
	void operator+=(const CVector3D& v);                 // �����������ӵ�
	CVector3D operator-(const CVector3D& v) const;       // ��������������õ�����
	void operator-=(const CVector3D& v);                 // ��������������

	CVector3D operator-();                               // ���ţ�ȡ������
	
	CVector3D operator*(const double& d) const;          // ���������˵õ�����
	void operator*=(const double& d);                    // ���������˵�
	CVector3D operator/(const double& d) const;          // �����������õ�����
	void operator/=(const double& d);                    // ������������
	
	CVector3D operator*(const CVector3D& v) const;       // �������������
	double operator|(const CVector3D& v) const;          // �������������

	CVector3D operator*(const CMatrix3D& matrix) const;  // �����;�����˵õ�����
	void operator*=(const CMatrix3D& matrix);            // �����;���˵�

	static CVector3D FromPoint3D(const CPoint3D& pt);
    static double GetAngle(CVector3D v1,CVector3D v2);   // ȡ�������ļн�
	double GetLength() const;                            // ����ģ
	double GetMod2() const;                              // ����ƽ��
	double GetMod() const;
	double GetLengthXY() const;                          // 
	double GetLengthYZ() const;                          //
	double GetLengthZX() const;                          // const��ʾ�������ò���ı��Ա������ֵ��
	CVector3D Reverse();

	CVector3D GetNormal() const;                         // �õ������ĵ�λ���������ص�λ����
	CVector3D Normalize();                               // ������λ��
	BOOL IsZeroVector() const;                           // �ж�����ģ�ǲ�����
	CVector2D ToVector2D() const;

public:
	double dx;
	double dy;
	double dz;
};

#define ZERO_VECTOR3D  CVector3D(0,0,0)

//////////////////////////////////////////////////////////////////////////
// ����
//////////////////////////////////////////////////////////////////////////
// Rotate around which axis:
#define  AXIS_X     1
#define  AXIS_Y     2
#define  AXIS_Z     3

class AFX_CLASS_EXPORT CMatrix3D:public CObject
{
public:
	CMatrix3D();                                         // ���캯�������쵥λ����
	CMatrix3D(const CMatrix3D& matrix);                   // ���캯�����þ�����
	CMatrix3D(const double* p);                                // ���캯���������鹹��
	virtual ~CMatrix3D();                                // ��������
	
public:
	const CMatrix3D& operator=(const CMatrix3D& matrix);
	CMatrix3D operator*(const CMatrix3D& matrix) const;  // ������ˣ����ؾ���
	void operator*=(const CMatrix3D& matrix);            // ����˵�

	void      IdenticalMatrix();                              // ��λ������
	double    GetDet(BOOL bHomogeneous=FALSE) const;                             // ��ȡ������ʽֵ
	CMatrix3D GetReverseMatrix(BOOL bHomogeneous=FALSE);         // �Ƿ�����Σ�4x4������Ĭ����3x3����������


public:
	static double GetDet(const double& a00,const double& a01,const double& a02,
		                 const double& a10,const double& a11,const double& a12,
						 const double& a20,const double& a21,const double& a22);                      //
	static CMatrix3D CreateMirrorMatrix(CVector3D planeNorm);                      // ��ԭ���ƽ��Գƾ���
	static CMatrix3D CreateRotateMatrix(const double& angle,CVector3D axis);              // �������ǹ�ԭ���,��ʱ��
	static CMatrix3D CreateRotateMatrix(const double& angle, int nAxis);
	static CMatrix3D CreateScaleMatrix(const double& xyzAxis);                            // ����һ������
	static CMatrix3D CreateScaleMatrix(const double& xAxis,const double& yAxis,const double& zAxis);    // x,y,z������ֱ�����
	static CMatrix3D CreateTransferMatrix(const CVector3D& vec);                          // ƽ�ƾ���

public:
	double A[4][4];  // ��һ�����У��ڶ�������
};

class AFX_CLASS_EXPORT CXAlignRect : public CObject  // x y ƽ���ϵ���������
{
public:
	CXAlignRect(){}
	~CXAlignRect(){}
	CXAlignRect(double left_in, double right_in, double bottom_in, double top_in)
	{
		left=left_in;
		right=right_in;
		bottom=bottom_in;
		top=top_in;
	}
	CXAlignRect(const CXAlignRect& rt)
	{
		left=rt.left;
		right=rt.right;
		bottom=rt.bottom;
		top=rt.top;
	}
	CXAlignRect(const CPoint3D& ptCenter, double offset_dis)
	{
		left=ptCenter.x-offset_dis;
		right=ptCenter.x+offset_dis;
		bottom=ptCenter.y-offset_dis;
		top=ptCenter.y+offset_dis;
	}
	const CXAlignRect& operator=(const CXAlignRect& rt)
	{
		left=rt.left;
		right=rt.right;
		bottom=rt.bottom;
		top=rt.top;
		return *this;
	}

	BOOL operator*(const CXAlignRect& rt) // if two rect intersect?
	{
		if (right < rt.left)
		{
			return FALSE;
		}
		else if (left > rt.right)
		{
			return FALSE;
		}
		else if (top < rt.bottom)
		{
			return FALSE;
		}
		else if (bottom > rt.top)
		{
			return FALSE;
		}

		return TRUE;
	}

	double GetMinBorder()
	{
		double wide=right-left;
		double height=top-bottom;
		double minBorder=wide;
		if (height<minBorder)
			minBorder=height;

		return minBorder;
	}

public:
	double left;
	double right;
	double bottom;
	double top;
};

class AFX_CLASS_EXPORT CRect3D
{
public:
	CRect3D();
	~CRect3D();
	CRect3D(const CRect3D& rt);
	CRect3D(const CPoint3D& A, const CPoint3D& B, const CPoint3D& C, const CPoint3D& D);

	void MakeABLength();
	CRect3D Shrink(double percent);  // ����percent
	bool IsPtInRect(CPoint3D pt);
	bool IsPtOnBorder(CPoint3D pt);
	bool IsPtOutRect(CPoint3D pt);

	CPoint3D A, B, C, D;   // ��0,1�ǳ���1,2�ǿ�
};

class AFX_CLASS_EXPORT COBBox// oriented bounding box
{
public:
	double x_min, x_max, y_min, y_max, z_min, z_max;

	COBBox();
	void Init(double xMin, double xMax, double yMin, double yMax, double zMin, double zMax);
	CPoint3D GetCenterPt() const;
};

// Query the performance time of a function                                      //
// This function is put before the to-be-mesured function together in a block {} //
// For example :
/*
	CString str;
	{
		QueryPerformanceTimer timer(str);
		Function();
	}
	AfxMessageBox(str);
*/
class AFX_CLASS_EXPORT QueryPerformanceTimer
{
public:
	QueryPerformanceTimer(CString& strMilliSeconds)
		: m_strMilliSeconds(strMilliSeconds)
	{
		::QueryPerformanceCounter(&m_Start);
	}

	~QueryPerformanceTimer(void)
	{
		LARGE_INTEGER stop;
		LARGE_INTEGER freq;
		::QueryPerformanceCounter(&stop);
		::QueryPerformanceFrequency(&freq);
		double t=1000*(double)(stop.QuadPart-m_Start.QuadPart)/(double)freq.QuadPart;
		m_strMilliSeconds.Format(_T("%.3fms"),t);
	}
protected:
	LARGE_INTEGER m_Start;
	CString& m_strMilliSeconds;
};

// �����ı��滻�꣬�������Ժ�������ʱ��
/* �÷���
>. QUERY_TIME_START
>. function();
>. QUERY_TIME_END
*/
#define  QUERY_TIME_START   CString str_XX_;{ QueryPerformanceTimer timer(str_XX_);
#define  QUERY_TIME_END     } AfxMessageBox(str_XX_);

//////////////////////////////////////////////////////////////////////////
// �������
//////////////////////////////////////////////////////////////////////////
double AFX_API_EXPORT GetAngle(CVector3D v1,CVector3D v2, bool bNormalized = false);  // 0-PI degree
double AFX_API_EXPORT Angle(const CVector3D& v1, const CVector3D& v2); // v1, v2 has been normlized
double AFX_API_EXPORT GetAngle(const CVector3D& v);         // Get the angle between v and positive x axis on xy plane, 0-360 degree
double AFX_API_EXPORT GetDistance(const CPoint3D& pt1,const CPoint3D& pt2);


// Calculate the distance square from a point to a segment
double AFX_API_EXPORT GetDist2PtToSeg(const CPoint3D& pt, 
									  const CPoint3D& ptS_seg, 
									  const CPoint3D& ptE_seg);

double AFX_API_EXPORT GetDistPtToSeg(const CPoint3D& pt, 
									 const CPoint3D& ptS_seg, 
									 const CPoint3D& ptE_seg);

// Calculatethe distance from a point to a line
double AFX_API_EXPORT GetDistPtToLine(const CPoint3D& pt, 
									  const CPoint3D& pt1_line, 
									  const CPoint3D& pt2_line);

bool   AFX_API_EXPORT IsParallel(const CVector3D& v1,const CVector3D& v2);
bool   AFX_API_EXPORT IsOrhogonal(const CVector3D& v1,const CVector3D& v2);
double AFX_API_EXPORT Round(const double& d, const int& iHowManyAfterDot=5); // ����С�������λ

/* 
   Get the cross point of two lines.
   return value: 
   0 : two lines are parallel;
   1 : two lines are coincidant;
   2 : two lines have corss point.
   paramenters:
   pt_out   : the cross point if it exist;
   ptS_seg1, ptE_seg1 : the two points of the first line;
   ptS_seg2, ptE_seg2 : the two points of the second line;
*/
int    AFX_API_EXPORT GetCrossPoint(CPoint3D& pt_out, 
									const CPoint3D& ptS_line1,
									const CPoint3D& ptE_line1,
									const CPoint3D& ptS_line2,
									const CPoint3D& ptE_line2); 

bool   AFX_API_EXPORT Is2SegsCross(const CPoint3D& ptS_seg1,
								   const CPoint3D& ptE_seg1,
								   const CPoint3D& ptS_seg2,
								   const CPoint3D& ptE_seg2); // ����ֱ���غ�Ҳ�Ǹ����

bool   AFX_API_EXPORT Is2SegsCross(CPoint3D& ptCross,
								   const CPoint3D& ptS_seg1,
								   const CPoint3D& ptE_seg1, 
								   const CPoint3D& ptS_seg2,
								   const CPoint3D& ptE_seg2); // ����ֱ���غ�Ҳ�Ǹ����

// Calculate the orthogal vector of the given vector on XY plane, 
// the obtained vector is on the left of the given vector
CVector3D AFX_API_EXPORT GetOrthoVectorOnXYPlane(const CPoint3D& ptS, const CPoint3D& ptE); 
CVector3D AFX_API_EXPORT GetOrthoVectorOnXYPlane(const CVector3D& vec); 
 
// Get "one of" the orthogal vector in space of the give vector
CVector3D AFX_API_EXPORT GetOrthoVector(const CVector3D& vec);

double    AFX_API_EXPORT rad2deg(double angle_rad);
double    AFX_API_EXPORT deg2rad(double angle_deg);

// Judge if three points in a line //
bool      AFX_API_EXPORT Is3PtInLine(const CPoint3D& pt1, const CPoint3D& pt2, const CPoint3D& pt3);

/* �жϵ��Ƿ����߶��ϣ�����ĵ������߶�����ֱ�߷����ϵģ����Բ��þ������ж϶���������˺�0�Ƚϴ�С */
bool      AFX_API_EXPORT IsPtOnSeg(const CPoint3D& pt, const CPoint3D& ptS_seg, const CPoint3D& ptE_seg);
