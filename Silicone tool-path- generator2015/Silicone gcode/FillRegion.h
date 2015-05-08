#pragma once
#include "Polyline.h"

class AFX_CLASS_EXPORT CFillRegion
{
public:
	CFillRegion(void);
	~CFillRegion(void);

	void AddOuterBoundary(CPolyline* pOuter);
	void AddHoles(Polylines& holes);
	void AddHole(CPolyline* pHole);

	CPolyline* m_outer;
	Polylines  m_holes;
};

typedef std::vector<CFillRegion> FillRegions;

class AFX_CLASS_EXPORT CFindFillRegions
{
public:
	static void FindFillRegions(Polylines& boundaries, FillRegions& regions);
	static void AdjustPolygonDirections(Polylines& boundaries);   // ����һ�����εķ���ʹ���������ʱ�룬Ȼ��˳ʱ��...
	static bool IsOutMost(CPolyline* pCurBoundary, Polylines& boundaries);
	static bool IsContain(CPolyline* pElement, CPolyline* pContainer);
};
