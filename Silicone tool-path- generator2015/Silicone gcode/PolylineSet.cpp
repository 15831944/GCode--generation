#include "StdAfx.h"
#include "PolylineSet.h"
#include "Polyline.h"

CPolylineSet::CPolylineSet(void)
{
	m_nType=TYPE_POLYLINESET;
}

CPolylineSet::~CPolylineSet(void)
{
}

void CPolylineSet::Draw(COpenGLDC *pDC,COLORREF clr)
{
	COLORREF oldClr;
	if (GetIsSelected())
		oldClr=pDC->SetMaterialColor(CLR_SELECTED);
	else
		oldClr=pDC->SetMaterialColor(clr);

	int sz=m_polylineList.GetSize();
	for (int i=0;i<sz;i++)
	{
		int szPt=m_polylineList[i]->m_pts.GetSize();
		for (int j=0;j<szPt-1;j++)
		{
			pDC->DrawLine(*m_polylineList[i]->m_pts[j], *m_polylineList[i]->m_pts[j+1]);
		}
	}

	pDC->SetMaterialColor(oldClr);
}