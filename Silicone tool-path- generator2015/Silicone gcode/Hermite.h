#pragma  once
#include "Entity.h"


#define  COUNT 1001

class AFX_CLASS_EXPORT CHermite:public CEntity
{
public:
	CHermite(void);
	CHermite(CPoint3D bforePt,CPoint3D pts, CPoint3D pte, CPoint3D afterPt);
	~CHermite(void);
protected:
public:
    void Interpolate();//根据三个点计算方程
	double Getlength();
	void Draw(COpenGLDC *pDC, COLORREF clr);
public:
	CPoint3D m_ptStart;
	CPoint3D m_ptEnd;
	CPoint3D m_BeforePt;
	CPoint3D m_AfterPt;


public:
	CVector3D vec1;
	CVector3D vec2;
	CPoint3D m_vD[COUNT];
	

};

