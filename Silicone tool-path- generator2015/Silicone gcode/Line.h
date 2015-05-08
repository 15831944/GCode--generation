#pragma once

#include "Entity.h"


class AFX_CLASS_EXPORT CLine : public CEntity
{
public:
	CLine(void);
	~CLine(void);
	CLine(const CLine& line);
	CLine(const CPoint3D& pt1, const CPoint3D& pt2);
	CLine(const CPoint3D& pt1, const CPoint3D& pt2, double length);
	const CLine& operator=(const CLine& line);

	virtual void Draw(COpenGLDC* pDC,COLORREF clr);
	virtual void Move(const CVector3D& vec);
	virtual void Rotate(double angle);
	virtual void Rotate(const CMatrix3D& m);
	void SetPt1(const CPoint3D& pt1);
	void SetPt2(const CPoint3D& pt2);
	BOOL operator*(const CLine& line) const;       // Is two line segment cross?
	void operator*=(const CMatrix3D& matrix);
	BOOL IsCrossWith(const CLine& line, CPoint3D& ptCross) const;
	double operator^(const CLine& line) const;     // Return the angle of the two lines (the lines are considered as vectors pointing m_pt1 to m_pt2),The returned angle is 0-180 degree
	double operator|(const CLine& line) const;     // Line dotProduct, the same as vector
	double GetLength2() const;                     // Get the square of length
	double GetLength() const;                      // Get length 
	double GetDistanceTo(const CPoint3D& pt) const;    // Get the distance between a line segment and a point
	double GetDistanceTo(const CLine& line) const;     // Get the distance between two line segments
	void   Reverse();                        // Reverse the direction of the line, like: pt1=pt2, pt2=pt1
	CVector3D GetVector() const;
	CVector3D GetNormalVector() const;

public:
	CPoint3D m_pt1;
	CPoint3D m_pt2;
	double   m_length;
};
typedef  CTypedPtrArray<CObArray,CLine*>  LineAry; 