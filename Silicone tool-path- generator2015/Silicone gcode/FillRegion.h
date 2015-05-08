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
	static void AdjustPolygonDirections(Polylines& boundaries);   // 调整一组多边形的方向，使其最外层逆时针，然后顺时针...
	static bool IsOutMost(CPolyline* pCurBoundary, Polylines& boundaries);
	static bool IsContain(CPolyline* pElement, CPolyline* pContainer);
};
