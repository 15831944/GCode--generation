// 定义3次B样条类，主要用来处理B样条函数的插值。要插值点的个数要〉=4
// 输入插值点，输出B样条曲线的控制顶点 m_ctrlPtList 和节点向量 u
// 同时也可以用P（t）函数计算样条线上的任意点。
// by Lin 2010-10-18

// 2012-7-20
// 进行了较大的修改

#pragma once
#include "afx.h"
#include "PointSet.h"

class AFX_CLASS_EXPORT CBSpline : public CPointSet  
{
public:
	CBSpline(void);
	~CBSpline(void);
	CBSpline(const CBSpline& spline);
	const CBSpline& operator=(const CBSpline& spline);

	virtual void Draw(COpenGLDC* pDC,COLORREF clr);           // 绘图

public:
	void SetDegree(int degree);           // 设置样条线的次数，默认是3次的
	int  AddCtrlPts(CPoint3D* pPt);
	int  AddFitPts(CPoint3D* pPt);
	int  AddKnot(double uu);
	int  AddInterpolationPts(CPoint3D* pPt);        // Add fit points one by one 
	void SetLastFitPt(const CPoint3D& pt);

	//BOOL Approximate(int numOfCtrlPts, int degree=3);
	BOOL Interpolate();
    void Scatter();                // 离散点
	void UnitizeKnot();            // 标准化节点矢量 0 0 0 0 ... 1 1 1 1,从dxf读出来的文件有时不为1的

	CPoint3D  P(double t)  const;  // 正向计算,给定输入参数t(0~1)，输出点的坐标
	CVector3D dP(double t) const;  // P在t处的导数


	double GetLength();//total length
	double Getlength(double u);//Get the legth from start to u;

protected:  // protected functions
	double N3(int i,double uu) const;   // 3次B样条基函数
	double N2(int i, double uu) const;  // 2次B样条基
	double dN3(int i, double uu) const; // 对3次B样条基求导
	double Delta(int i);

public:
	doubleAry  m_u;       // 节点矢量
	Point3DAry m_ctrls; // 控制顶点
	Point3DAry m_fits;  // 拟合点
   
protected:
	int   m_degree;       // 次数，方程的次数是介数减1
	BOOL  m_bCanDraw;     // 控制作用，绘图的时候不能进行拟合或运算
	BOOL  m_bCanFit;      // 

protected:
	//void SetKnotUpperIndex(int upperIndex);


};

