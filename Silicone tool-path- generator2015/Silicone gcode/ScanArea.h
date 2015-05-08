#pragma once

#include "Polyline.h"

class CScanArea
{
public:
	CScanArea(void);
	~CScanArea(void);

	CPolyline* m_outerBoundary;    // ccw
	vector<CPolyline*> m_islands;  // cw
};
