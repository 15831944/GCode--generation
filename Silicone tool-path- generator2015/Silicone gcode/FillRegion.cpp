
#include "stdafx.h"
#include "FillRegion.h"

CFillRegion::CFillRegion(void)
{
}

CFillRegion::~CFillRegion(void)
{
}

void CFillRegion::AddOuterBoundary(CPolyline *pOuter)
{
	m_outer = pOuter;
}

void CFillRegion::AddHoles(Polylines& holes)
{
	int sz = holes.size();
	for (int i=0;i<sz;i++)
	{
		AddHole(holes[i]);
	}
}

void CFillRegion::AddHole(CPolyline *pHole)
{
	if (pHole != NULL)
	{
		m_holes.push_back(pHole);
	}
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


void CFindFillRegions::FindFillRegions(Polylines& boundaries, FillRegions& regions)
{
	AdjustPolygonDirections(boundaries);

	std::vector<CPolyline*> holes;
	int sz = boundaries.size();
	for (int i=0;i<sz;i++)
	{
		if (/*boundaries[i]->IsPolygonCCW()*/ boundaries[i]->m_level % 2 == 0)
		{
			CFillRegion region;
			region.m_outer = boundaries[i];
			regions.push_back(region);
		}
		else
		{
			holes.push_back(boundaries[i]);
		}
	}

	int szIsland = holes.size();
	int szRegion = regions.size();
	for (int i=0;i<szIsland;i++)
	{
		for (int j=0;j<szRegion;j++)
		{
			if ( IsContain(holes[i],regions[j].m_outer) && 
				regions[j].m_outer->m_level == holes[i]->m_level-1 )
			{
				regions[j].AddHole(holes[i]);
			}
		}
	}
}

void CFindFillRegions::AdjustPolygonDirections(Polylines &boundaries)
{
	std::vector<CPolyline*> tmp_boundaries(boundaries); //

	// Identify the level of the boundaries
	int sz = tmp_boundaries.size();
	int level=0;
	while (sz>0)
	{
		std::vector<int> idxes;
		for (int i=0;i<sz;i++)
		{
			if (IsOutMost(tmp_boundaries[i], tmp_boundaries))
			{
				idxes.push_back(i);
				tmp_boundaries[i]->m_level=level;
				if (level%2 == 0)
				{
					tmp_boundaries[i]->MakePolygonCCW();
				}
				else
				{
					tmp_boundaries[i]->MakePolygonCW();          // make the most outside CCW, then CW, ....
				}
			}
		}

		int szIdx=idxes.size();
		for (int i=szIdx-1;i>=0;i--)
		{
			tmp_boundaries.erase(tmp_boundaries.begin()+idxes[i]);
		}
		level++;
		sz = tmp_boundaries.size();
	}
}

bool CFindFillRegions::IsOutMost(CPolyline* pCurBoundary, Polylines& boundaries)
{
	int sz=boundaries.size();
	//CPoint3D ptTest = pCurBoundary->GetFirstPt();
	for (int i=0;i<sz;i++)
	{
		if (pCurBoundary == boundaries[i])
		{
			continue;
		}
		else if (IsContain(pCurBoundary,boundaries[i])  /*boundaries[i]->IsPtInPolygon(ptTest)*/)
		{
			return false;
		}
	}

	return true;
}

bool CFindFillRegions::IsContain(CPolyline* pElement, CPolyline* pContainer)
{
	int i=0, j=0;
	int	szC=pContainer->m_pts.GetSize();
	int szE=pElement->m_pts.GetSize();
	CPoint3D ptTest;
	double d2=0;
	while (i<szE)
	{
		ptTest = *pElement->m_pts[i];
		for (j=0;j<=szC-2;j++)
		{
			d2 = ::GetDist2PtToSeg(ptTest,*pContainer->m_pts[j],*pContainer->m_pts[j+1]);
			if (d2 < 1.0e-12)
			{
				break;
			}
		}

		if (j == szC-1)
		{
			break;
		}
		else
		{
			i++;
		}
	}

	return pContainer->IsPtInPolygon(ptTest);
}
