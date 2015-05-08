// ����3��B�����࣬��Ҫ��������B���������Ĳ�ֵ��Ҫ��ֵ��ĸ���Ҫ��=4
// �����ֵ�㣬���B�������ߵĿ��ƶ��� m_ctrlPtList �ͽڵ����� u
// ͬʱҲ������P��t�����������������ϵ�����㡣
// by Lin 2010-10-18

// 2012-7-20
// �����˽ϴ���޸�

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

	virtual void Draw(COpenGLDC* pDC,COLORREF clr);           // ��ͼ

public:
	void SetDegree(int degree);           // ���������ߵĴ�����Ĭ����3�ε�
	int  AddCtrlPts(CPoint3D* pPt);
	int  AddFitPts(CPoint3D* pPt);
	int  AddKnot(double uu);
	int  AddInterpolationPts(CPoint3D* pPt);        // Add fit points one by one 
	void SetLastFitPt(const CPoint3D& pt);

	//BOOL Approximate(int numOfCtrlPts, int degree=3);
	BOOL Interpolate();
    void Scatter();                // ��ɢ��
	void UnitizeKnot();            // ��׼���ڵ�ʸ�� 0 0 0 0 ... 1 1 1 1,��dxf���������ļ���ʱ��Ϊ1��

	CPoint3D  P(double t)  const;  // �������,�����������t(0~1)������������
	CVector3D dP(double t) const;  // P��t���ĵ���


	double GetLength();//total length
	double Getlength(double u);//Get the legth from start to u;

protected:  // protected functions
	double N3(int i,double uu) const;   // 3��B����������
	double N2(int i, double uu) const;  // 2��B������
	double dN3(int i, double uu) const; // ��3��B��������
	double Delta(int i);

public:
	doubleAry  m_u;       // �ڵ�ʸ��
	Point3DAry m_ctrls; // ���ƶ���
	Point3DAry m_fits;  // ��ϵ�
   
protected:
	int   m_degree;       // ���������̵Ĵ����ǽ�����1
	BOOL  m_bCanDraw;     // �������ã���ͼ��ʱ���ܽ�����ϻ�����
	BOOL  m_bCanFit;      // 

protected:
	//void SetKnotUpperIndex(int upperIndex);


};

