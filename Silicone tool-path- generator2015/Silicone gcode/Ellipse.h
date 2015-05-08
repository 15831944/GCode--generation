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
			double angleStart=0,       // ����ڳ���ĽǶ�
			double angleEnd=TWOPI);    // ����ڳ���ĽǶ�

	// An ellipse contructed by three points, two end points of the major axis,
	// and some point locate on the ellipse besides two end points
	CEllipse(const CPoint3D& ptS_major, const CPoint3D& ptE_major, const CPoint3D& pt_some);

	const CEllipse& operator=(const CEllipse& ellipse);

	void Create(const CPoint3D& ptCenter, 
				double a, 
				double b, 
				double angleRotate=0, 
				double angleStart=0,       // ����ڳ���ĽǶ�
				double angleEnd=TWOPI);    // ����ڳ���ĽǶ�

	virtual void Draw(COpenGLDC* pDC,COLORREF clr);

	void Rotate(double rotateAngle); // ��ת�Ƕ���ʱ��Ϊ��

protected:
	CPoint3D m_ptCenter;  // ��Բ���ĵ�
	double m_a;           // �볤��
	double m_b;           // �����
	double m_angleRotate;       // ��Բ�����X������н�,��ʱ��Ϊ��
	double m_angleStart;  // ��Բ�����Ƕȣ�����ڳ���
	double m_angleEnd;    // ��Բ���յ�Ƕȣ�����ڳ���

	CPoint3D m_pt[362];
	int      m_count;     // the count of drawing points of ellipse arc
};
