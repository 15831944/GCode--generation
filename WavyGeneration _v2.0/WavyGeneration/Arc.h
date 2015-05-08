#pragma once
#include "entity.h"

class AFX_CLASS_EXPORT CArc : public CEntity
{
public:
	CArc(void);
	~CArc(void);
	CArc(const CArc& arc);
	CArc(const CPoint3D& ptCenter, double radius, double angleStart, double angleEnd);     // angle in rad
	CArc(const CPoint3D& pt1, const CPoint3D& pt2, const CPoint3D& ptSome);          // 3 points from the arc
	const CArc& operator = (const CArc& arc);

	void Create(const CPoint3D& ptCenter, double radius, double angleStart_rad, double angleEnd_rad);
	virtual void Draw(COpenGLDC* pDC, COLORREF clr);

	int GetPtCount() const;

	CPoint3D  m_pt[362];     // For drawing purpose

protected:
	CPoint3D m_ptCenter;
	double   m_radius;
	double   m_angle1;  // Start angle, degree
	double   m_angle2;  // End angle,   degree


	int       m_count;       // The count of the drawing point, index from 0 to (m_count-1)
};
