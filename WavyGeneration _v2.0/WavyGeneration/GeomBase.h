// CadBase.h: interface for the CadBase class.
// 
//////////////////////////////////////////////////////////////////////

// By Lin 2010...

// History:

// 2011-6-3
// 增加了点相加的函数，就是两个点的坐标分别相加。也增加了点和实数相乘操作符，就是每个坐标和实数乘
// ----------------------------------------------------------------------------------------------
// 2011-5-25:
// 修改了 GetAngle() 函数。原因在于 acos(double d) 函数参数 d 的取值范围是 [-1,1], 而在现实计算中
// 比如向量点乘的时候偶尔会出现大于 1 的情况，这样就导致了函数计算的错误，计算值无效。
// ----------------------------------------------------------------------------------------------
// 2011-4-24:
// 给 CPoint3D 类增加了一个成员变量 w，作为预留的量，可以作为该点到某点的距离，也可作为布尔型判断
// 该点是否有效。
// ----------------------------------------------------------------------------------------------
// 2013-6-9:
// 增加了CPoint2D, CVector2D平面点和向量类

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
class CVector3D;    // 这个声明很重要，否则无法编译
class CMatrix3D;    ////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
// 点
//////////////////////////////////////////////////////////////////////////
class AFX_CLASS_EXPORT CPoint2D
{
public:     ////// constructor
	CPoint2D();											// 构造函数：构造零点                                           
	CPoint2D(double ix,double iy);		// 构造函数
	CPoint2D(const double* p);							// 用数组构造函数
	CPoint2D(const CPoint2D& pt);						 // 用点构造
	virtual ~CPoint2D();								 // 析构函数           

public:     ////// operator
	const CPoint2D& operator=(const CPoint2D& pt);
	void operator=(double val);
	CPoint2D operator+(const CVector2D& v) const;        // 点和向量相加，返回点, const 放在最后表示该函数不能改变该类的成员变量
	CPoint2D operator+(const CPoint2D& pt) const;
	void operator+=(const CVector2D& v);                 // 点和向量加等
	CPoint2D operator-(const CVector2D& v) const;        // 点和向量相减，返回点
	void operator-=(const CVector2D& v);                 // 点和向量操作
	CVector2D operator-(const CPoint2D& pt) const;       // 点减去点得到向量
	BOOL operator==(const CPoint2D& pt) const;           // Complete the same
	BOOL operator^=(const CPoint2D& pt) const;           // More less the same ~==
	BOOL operator!=(const CPoint2D& pt) const;           // 比较操作符
	CPoint2D operator*(const double& dFactor) const;

	CPoint3D ToPoint3D(double zVal=0) const;

public:
	double x;
	double y;
};


class  AFX_CLASS_EXPORT CPoint3D:public CObject
{
public:     ////// constructor
	CPoint3D();                                  // 构造函数：构造零点                                           
	CPoint3D(double ix, double iy, double iz);     // 构造函数
	CPoint3D(const double* p);                   // 用数组构造函数
	CPoint3D(const CPoint3D& pt);                        // 用点构造
	virtual ~CPoint3D();                         // 析构函数           
	
public:     ////// operator
	const CPoint3D& operator=(const CPoint3D& pt);
	void operator=(double val);
	CPoint3D operator+(const CVector3D& v) const;        // 点和向量相加，返回点, const 放在最后表示该函数不能改变该类的成员变量
	CPoint3D operator+(const CPoint3D& pt) const;
	void operator+=(const CVector3D& v);                 // 点和向量加等
	CPoint3D operator-(const CVector3D& v) const;        // 点和向量相减，返回点
	void operator-=(const CVector3D& v);                 // 点和向量操作
	CVector3D operator-(const CPoint3D& pt) const;       // 点减去点得到向量
	BOOL operator==(const CPoint3D& pt) const;           // Complete the same
	BOOL operator^=(const CPoint3D& pt) const;           // More less the same ~==
	BOOL operator!=(const CPoint3D& pt) const;           // 比较操作符
	CPoint3D operator*(const CMatrix3D& matrix) const;   // 点和矩阵相乘得到点，其实就是变换操作
	CPoint3D operator*(const double& dFactor) const;
	void operator*=(const CMatrix3D& matrix);            // 点乘以矩阵
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
// 向量
//////////////////////////////////////////////////////////////////////////

class AFX_CLASS_EXPORT CVector2D
{
public:
	CVector2D();                                         // 构造函数，构造零向量
	CVector2D(double ix, double iy);        // 构造函数
	CVector2D(const double* p);                          // 构造函数：用数组构造
	CVector2D(const CVector2D& v);                       // 构造函数：用向量构造
	virtual ~CVector2D();                                // 析构函数

public:
	const CVector2D& operator=(const CVector2D& v);
	BOOL operator==(const CVector2D& v) const;
	CVector2D operator+(const CVector2D& v) const;       // 向量和向量相加得到向量
	void operator+=(const CVector2D& v);                 // 向量和向量加等
	CVector2D operator-(const CVector2D& v) const;       // 向量和向量相减得到向量
	void operator-=(const CVector2D& v);                 // 向量和向量减等
	CVector2D operator-();                               // 负号，取负操作
	CVector2D operator*(const double& d) const;          // 向量和数乘得到向量
	void operator*=(const double& d);                    // 向量和数乘等
	CVector2D operator/(const double& d) const;          // 向量和数除得到向量
	void operator/=(const double& d);                    // 向量和数除等
	double operator|(const CVector2D& v) const;          // 向量和向量点乘


	static CVector2D FromPoint2D(const CPoint2D& pt);
	static double GetAngle(CVector2D v1,CVector2D v2);   // 取得向量的夹角

	double GetLength() const;                            // 距离平方
	double GetMod2() const;                              // 向量模的平方
	double GetMod() const;								 // 向量模

	CVector2D GetNormal() const;                         // 得到向量的单位向量，返回单位向量
	CVector2D Normalize();                               // 向量单位化
	BOOL IsZeroVector() const;                           // 判断向量模是不是零

	CVector3D ToVector3D(double dzVal) const;

public:
	double dx;
	double dy;
};


class AFX_CLASS_EXPORT CVector3D:public CObject
{
public:
	CVector3D();                                         // 构造函数，构造零向量
	CVector3D(const double& ix,const double& iy,const double& iz);   // 构造函数
	CVector3D(const double* p);                          // 构造函数：用数组构造
	CVector3D(const CVector3D& v);                       // 构造函数：用向量构造
	virtual ~CVector3D();                                // 析构函数
	
public:
    const CVector3D& operator=(const CVector3D& v);
	BOOL operator==(const CVector3D& v) const;
	CVector3D operator+(const CVector3D& v) const;       // 向量和向量相加得到向量
	void operator+=(const CVector3D& v);                 // 向量和向量加等
	CVector3D operator-(const CVector3D& v) const;       // 向量和向量相减得到向量
	void operator-=(const CVector3D& v);                 // 向量和向量减等

	CVector3D operator-();                               // 负号，取负操作
	
	CVector3D operator*(const double& d) const;          // 向量和数乘得到向量
	void operator*=(const double& d);                    // 向量和数乘等
	CVector3D operator/(const double& d) const;          // 向量和数除得到向量
	void operator/=(const double& d);                    // 向量和数除等
	
	CVector3D operator*(const CVector3D& v) const;       // 向量和向量叉乘
	double operator|(const CVector3D& v) const;          // 向量和向量点乘

	CVector3D operator*(const CMatrix3D& matrix) const;  // 向量和矩阵相乘得到向量
	void operator*=(const CMatrix3D& matrix);            // 向量和矩阵乘等

	static CVector3D FromPoint3D(const CPoint3D& pt);
    static double GetAngle(CVector3D v1,CVector3D v2);   // 取得向量的夹角
	double GetLength() const;                            // 向量模
	double GetMod2() const;                              // 距离平方
	double GetMod() const;
	double GetLengthXY() const;                          // 
	double GetLengthYZ() const;                          //
	double GetLengthZX() const;                          // const表示函数调用不会改变成员变量的值。
	CVector3D Reverse();

	CVector3D GetNormal() const;                         // 得到向量的单位向量，返回单位向量
	CVector3D Normalize();                               // 向量单位化
	BOOL IsZeroVector() const;                           // 判断向量模是不是零
	CVector2D ToVector2D() const;

public:
	double dx;
	double dy;
	double dz;
};

#define ZERO_VECTOR3D  CVector3D(0,0,0)

//////////////////////////////////////////////////////////////////////////
// 矩阵
//////////////////////////////////////////////////////////////////////////
// Rotate around which axis:
#define  AXIS_X     1
#define  AXIS_Y     2
#define  AXIS_Z     3

class AFX_CLASS_EXPORT CMatrix3D:public CObject
{
public:
	CMatrix3D();                                         // 构造函数，构造单位矩阵
	CMatrix3D(const CMatrix3D& matrix);                   // 构造函数，用矩阵构造
	CMatrix3D(const double* p);                                // 构造函数，用数组构造
	virtual ~CMatrix3D();                                // 析构函数
	
public:
	const CMatrix3D& operator=(const CMatrix3D& matrix);
	CMatrix3D operator*(const CMatrix3D& matrix) const;  // 矩阵相乘，返回矩阵
	void operator*=(const CMatrix3D& matrix);            // 矩阵乘等

	void      IdenticalMatrix();                              // 单位化矩阵
	double    GetDet(BOOL bHomogeneous=FALSE) const;                             // 获取子行列式值
	CMatrix3D GetReverseMatrix(BOOL bHomogeneous=FALSE);         // 是否是齐次（4x4）矩阵，默认求3x3矩阵的逆矩阵


public:
	static double GetDet(const double& a00,const double& a01,const double& a02,
		                 const double& a10,const double& a11,const double& a12,
						 const double& a20,const double& a21,const double& a22);                      //
	static CMatrix3D CreateMirrorMatrix(CVector3D planeNorm);                      // 过原点的平面对称矩阵
	static CMatrix3D CreateRotateMatrix(const double& angle,CVector3D axis);              // 向量都是过原点的,逆时针
	static CMatrix3D CreateRotateMatrix(const double& angle, int nAxis);
	static CMatrix3D CreateScaleMatrix(const double& xyzAxis);                            // 三轴一起缩放
	static CMatrix3D CreateScaleMatrix(const double& xAxis,const double& yAxis,const double& zAxis);    // x,y,z三个轴分别缩放
	static CMatrix3D CreateTransferMatrix(const CVector3D& vec);                          // 平移矩阵

public:
	double A[4][4];  // 第一个是行，第二个是列
};

class AFX_CLASS_EXPORT CXAlignRect : public CObject  // x y 平面上的轴对齐矩形
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
	CRect3D Shrink(double percent);  // 收缩percent
	bool IsPtInRect(CPoint3D pt);
	bool IsPtOnBorder(CPoint3D pt);
	bool IsPtOutRect(CPoint3D pt);

	CPoint3D A, B, C, D;   // 让0,1是长，1,2是宽
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

// 定义文本替换宏，用来测试函数运行时间
/* 用法：
>. QUERY_TIME_START
>. function();
>. QUERY_TIME_END
*/
#define  QUERY_TIME_START   CString str_XX_;{ QueryPerformanceTimer timer(str_XX_);
#define  QUERY_TIME_END     } AfxMessageBox(str_XX_);

//////////////////////////////////////////////////////////////////////////
// 输出函数
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
double AFX_API_EXPORT Round(const double& d, const int& iHowManyAfterDot=5); // 保留小数点后三位

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
								   const CPoint3D& ptE_seg2); // 两条直线重合也是干涉的

bool   AFX_API_EXPORT Is2SegsCross(CPoint3D& ptCross,
								   const CPoint3D& ptS_seg1,
								   const CPoint3D& ptE_seg1, 
								   const CPoint3D& ptS_seg2,
								   const CPoint3D& ptE_seg2); // 两条直线重合也是干涉的

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

/* 判断点是否在线段上，这里的点是在线段所在直线方向上的，所以不用距离来判断而用向量点乘和0比较大小 */
bool      AFX_API_EXPORT IsPtOnSeg(const CPoint3D& pt, const CPoint3D& ptS_seg, const CPoint3D& ptE_seg);
