#pragma once
#include "Entity.h"

#define  CIRCLE_PRECISION   120    // A circle is made up of 120 line segments, for drawing purpose

class AFX_CLASS_EXPORT CCircle : public CEntity
{
public:
	CCircle(void);
	~CCircle(void);
	CCircle(const CCircle& circle);
	CCircle(const CPoint3D& ptCenter, const double radius);                  // Center and radius circle
	CCircle(const CPoint3D& pt1, const CPoint3D& pt2, const CPoint3D& pt3);  // Three points circle
	const CCircle& operator=(const CCircle& circle);

	virtual void Draw(COpenGLDC* pDC,COLORREF clr);
	virtual void Move(const CVector3D& vec);

	CPoint3D pt[CIRCLE_PRECISION];  // For drawing purpose	

public:
	CPoint3D m_ptCenter;			// The center point of the circle
	double   m_radius;				// The circle radius
	//CPoint3D pt[CIRCLE_PRECISION];  // For drawing purpose	
};
