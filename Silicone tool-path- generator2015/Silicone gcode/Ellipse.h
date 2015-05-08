#pragma once

#include "Entity.h"

class AFX_CLASS_EXPORT CEllipse : public CEntity
{
public:
	CEllipse(void);
	~CEllipse(void);
	CEllipse(const CPoint3D& ptCenter,  // Basic constructor
			double a, 
			double b, 
			double angleRotate=0, 
			double angleStart=0,       // 相对于长轴的角度
			double angleEnd=TWOPI);    // 相对于长轴的角度

	// An ellipse contructed by three points, two end points of the major axis,
	// and some point locate on the ellipse besides two end points
	CEllipse(const CPoint3D& ptS_major, const CPoint3D& ptE_major, const CPoint3D& pt_some);

	const CEllipse& operator=(const CEllipse& ellipse);

	void Create(const CPoint3D& ptCenter, 
				double a, 
				double b, 
				double angleRotate=0, 
				double angleStart=0,       // 相对于长轴的角度
				double angleEnd=TWOPI);    // 相对于长轴的角度

	virtual void Draw(COpenGLDC* pDC,COLORREF clr);

	void Rotate(double rotateAngle); // 旋转角度逆时针为正

protected:
	CPoint3D m_ptCenter;  // 椭圆中心点
	double m_a;           // 半长轴
	double m_b;           // 半短轴
	double m_angleRotate;       // 椭圆长轴和X轴正向夹角,逆时针为正
	double m_angleStart;  // 椭圆弧起点角度，相对于长轴
	double m_angleEnd;    // 椭圆弧终点角度，相对于长轴

	CPoint3D m_pt[362];
	int      m_count;     // the count of drawing points of ellipse arc
};
