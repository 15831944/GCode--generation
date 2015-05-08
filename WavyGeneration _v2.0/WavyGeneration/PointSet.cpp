#include "StdAfx.h"
#include "PointSet.h"

#define PT_ON_EDGE_AB   2
#define PT_ON_EDGE_BC   3
#define PT_ON_EDGE_CA   4


CPointSet::CPointSet(void)
{
}

CPointSet::~CPointSet(void)
{
	int sz=m_pts.GetSize();
	for (int i=sz-1;i>=0;i--)
	{
		delete m_pts[i];
		m_pts[i]=NULL;
	}
	m_pts.RemoveAll();
}

void CPointSet::Draw(COpenGLDC* pDC,COLORREF clr)
{
	int sz=m_pts.GetSize();
	COLORREF oldClr=pDC->SetMaterialColor(clr);
	for (int i=0;i<sz;i++)
	{
	    pDC->DrawPoint(*m_pts[i]);
	}
	pDC->SetMaterialColor(oldClr);
}

int CPointSet::GetSize()
{
	return m_pts.GetSize();
}

void CPointSet::AddPt(CPoint3D* pPt)
{
	m_pts.Add(pPt);
}

void CPointSet::AddPt(const CPoint3D& pt)
{
	m_pts.Add(new CPoint3D(pt));
}

void CPointSet::Write2TXT(CStdioFile& file)
{
	int sz=m_pts.GetSize();
	CString str;
	file.WriteString(_T("[begin]\n"));
	str.Format(_T("number of points: %.6d\n"),sz);
	file.WriteString(str);
	for (int i=0;i<sz;i++)
	{
		str.Format(_T("i=%.6d,  X=%.10f, Y=%.10f, Z=%.10f\n"), i, m_pts[i]->x, m_pts[i]->y, m_pts[i]->z);
		file.WriteString(str);
	}
	file.WriteString(_T("[end]"));
}

void CPointSet::Write2TXT(CString sFilePath)
{
	CStdioFile file;
	if (file.Open(sFilePath,CFile::modeCreate|CFile::modeWrite))
	{
		Write2TXT(file);
		file.Close();
	}
}

void CPointSet::ReadTXT(CStdioFile& file)
{
	int sz;
	sz=m_pts.GetSize();
	for (int i=sz-1;i>=0;i--)
		delete m_pts[i];
	m_pts.RemoveAll(); // 先清空数组

	int s,e,l;
	CString str;
	CString strN;
	CPoint3D* pPt=NULL;
	file.ReadString(str);
	file.ReadString(str);
	str=str.Right(6);
	sz=_wtoi(str);
	while (file.ReadString(str))
	{
		if (_T("[end]")==str) break;

		pPt=new CPoint3D;
		s=str.Find('X');
		l=str.GetLength();
		str=str.Right(l-s);
		e=str.Find(',');
		strN=str.Mid(2,e-2);
		pPt->x=_wtof(strN);

		s=str.Find('Y');
		l=str.GetLength();
		str=str.Right(l-s);
		e=str.Find(',');
		strN=str.Mid(2,e-2);
		pPt->y=_wtof(strN);

		s=str.Find('Z');
		l=str.GetLength();
		str=str.Right(l-s);	
		l=str.GetLength();
		strN=str.Right(l-2);
		pPt->z=_wtof(strN);

		m_pts.Add(pPt);
	}
}

// Get Convex Hull for plannar points //
void CPointSet::GetConvexHull(Point3DAry& convexHull) const
{
	int sz0=m_pts.GetSize();
	if (0==sz0) return;
	if (1==sz0)
	{
		CPoint3D* pPt=new CPoint3D(*m_pts[0]);
		convexHull.Add(pPt);
		return;
	}
	if (2==sz0)
	{
		CPoint3D* pPt=new CPoint3D(*m_pts[0]);
		convexHull.Add(pPt);
		pPt=new CPoint3D(*m_pts[1]);
		convexHull.Add(pPt);
		return;
	}

	Point3DAry ptList;    // Temp list
	Point3DAry upperHull; // upper hull
	Point3DAry lowerHull; // lower hull
	CPoint3D* pPt=NULL;
	for (int i=0;i<sz0;i++)
	{
		pPt=new CPoint3D(*m_pts[i]);
		ptList.Add(pPt);
	}
	QSortL2R(ptList,0,sz0-1);
	RemoveSamePts(ptList);  // Remove same points
	sz0=ptList.GetSize();

	int sz;   // Size of upper hull
	CVector3D v1,v2;
	pPt=new CPoint3D(*ptList[0]);
	upperHull.Add(pPt);
	pPt=new CPoint3D(*ptList[1]);
	upperHull.Add(pPt);
	for (int i=2;i<sz0;i++)
	{
		pPt=new CPoint3D(*ptList[i]);
		upperHull.Add(pPt);
		sz=upperHull.GetSize();
		v1=*upperHull[sz-2] - *upperHull[sz-3];
		v2=*upperHull[sz-1] - *upperHull[sz-2];
		while ((v1*v2).dz>=0)
		{
			delete upperHull[sz-2];
			upperHull[sz-2]=NULL;
			upperHull.RemoveAt(sz-2);
			sz=upperHull.GetSize();
			if (sz>=3)
			{
				v1=*upperHull[sz-2] - *upperHull[sz-3];
				v2=*upperHull[sz-1] - *upperHull[sz-2];
			}
			else
			{
				break;
			}
		}
	}

	pPt=new CPoint3D(*ptList[0]);
	lowerHull.Add(pPt);
	pPt=new CPoint3D(*ptList[1]);
	lowerHull.Add(pPt);
	for (int i=2;i<sz0;i++)
	{
		pPt=new CPoint3D(*ptList[i]);
		lowerHull.Add(pPt);
		sz=lowerHull.GetSize();
		v1=*lowerHull[sz-2] - *lowerHull[sz-3];
		v2=*lowerHull[sz-1] - *lowerHull[sz-2];
		while ((v1*v2).dz<=0)
		{
			delete lowerHull[sz-2];
			lowerHull[sz-2]=NULL;
			lowerHull.RemoveAt(sz-2);
			sz=lowerHull.GetSize();
			if (sz>=3)
			{
				v1=*lowerHull[sz-2] - *lowerHull[sz-3];
				v2=*lowerHull[sz-1] - *lowerHull[sz-2];
			}
			else
			{
				break;
			}
		}
	}

	sz=lowerHull.GetSize();
	for (int i=0;i<sz;i++ )
		convexHull.Add(lowerHull[i]);
	lowerHull.RemoveAll();

	sz=upperHull.GetSize();
	for (int i=sz-2;i>=0;i--)
		convexHull.Add(upperHull[i]);
	delete upperHull[sz-1];
	upperHull[sz-1]=NULL;
	upperHull.RemoveAll();

	for (int i=sz0-1;i>=0;i--)
	{
		delete ptList[i];
		ptList[i]=NULL;
	}
	ptList.RemoveAll();
}

COBBox CPointSet::BoundingBox()
{
	int sz = m_pts.GetSize();
	if (sz = 0)
		return COBBox();

	double xMin, xMax, yMin, yMax, zMin, zMax;
	xMin=xMax=m_pts[0]->x;
	yMin=yMax=m_pts[0]->y;
	zMin=zMax=m_pts[0]->z;
	for (int i=1;i<sz;i++)
	{
		if (m_pts[i]->x < xMin)
		{
			xMin = m_pts[i]->x;
		}
		if (m_pts[i]->x > xMax)
		{
			xMax = m_pts[i]->x;
		}
		if (m_pts[i]->y < yMin)
		{
			yMin = m_pts[i]->y;
		}
		if (m_pts[i]->y > yMax)
		{
			yMax = m_pts[i]->y;
		}
		if (m_pts[i]->z < zMin)
		{
			zMin = m_pts[i]->z;
		}
		if (m_pts[i]->z > zMax)
		{
			zMax = m_pts[i]->z;
		}
	}
	m_obb.Init(xMin,xMax,yMin,yMax,zMin,zMax);
	return m_obb;
}

void CPointSet::GetMEC(CPoint3D& ptCenter_out, double& radius_out, BOOL bRandomize/* =TRUE */)
{
	if (bRandomize)
	{
		Randomize();
	}

	ptCenter_out=*m_pts.GetAt(0);
	radius_out=0;

	int nCount=m_pts.GetSize();
	for (int i=1; i<nCount; i++)
		if (GetDistance(*m_pts.GetAt(i),ptCenter_out)>radius_out+POINT_RESOLUTION)
		{
			ptCenter_out=*m_pts.GetAt(i);
			radius_out=0;
			for (int j=0; j<i; j++)
				if(GetDistance(*m_pts.GetAt(j),ptCenter_out)>radius_out+POINT_RESOLUTION)
				{
					ptCenter_out.x=(m_pts.GetAt(i)->x+m_pts.GetAt(j)->x)/2;
					ptCenter_out.y=(m_pts.GetAt(i)->y+m_pts.GetAt(j)->y)/2;
					radius_out=GetDistance(ptCenter_out,*m_pts.GetAt(j));
					for (int k=0; k<j; k++)
						if (GetDistance(*m_pts.GetAt(k),ptCenter_out)>radius_out+POINT_RESOLUTION)
						{
							ptCenter_out=GetCenter(*m_pts.GetAt(i),*m_pts.GetAt(j),*m_pts.GetAt(k));
							radius_out=GetDistance(ptCenter_out,*m_pts.GetAt(k));
						}
				}	
		}
}

CPoint3D CPointSet::GetCenter( const CPoint3D &ptA, const CPoint3D &ptB, const CPoint3D &ptC )
{
	double   xAB=ptB.x-ptA.x; 
	double   yAB=ptB.y-ptA.y; 
	double   xAC=ptC.x-ptA.x; 
	double   yAC=ptC.y-ptA.y; 
	double   zB=xAB*(ptA.x+ptB.x)+yAB*(ptA.y+ptB.y); 
	double   zC=xAC*(ptA.x+ptC.x)+yAC*(ptA.y+ptC.y); 
	double   d=2.0*(xAB*(ptC.y-ptB.y)-yAB*(ptC.x-ptB.x)); 
	if (abs(d)<POINT_RESOLUTION)
		return FALSE;
	CPoint3D ptCenter;
	ptCenter.x=(yAC*zB-yAB*zC)/d; 
	ptCenter.y=(xAB*zC-xAC*zB)/d; 
	return ptCenter;
}

void CPointSet::QSortL2R(Point3DAry& ptList, int s, int e)
{
	if (s >= e) return;

	int i=s, j=e-1;
	CPoint3D tmp;
	do
	{     
		while(        IsP1LeftOfP2   (*ptList[i],*ptList[e])) i++;  // From left to right
		while(j>=0 && IsP1NotLeftOfP2(*ptList[j],*ptList[e])) j--;     // the "=" must be put, or mistake happens.

		if (i<j)
		{
			tmp = *ptList[i];
			*ptList[i] = *ptList[j];
			*ptList[j] = tmp;
		}
		else 
		{
			tmp = *ptList[i];
			*ptList[i] = *ptList[e];
			*ptList[e] = tmp;
		}
	} while(i<j);

	QSortL2R(ptList,s, i-1);
	QSortL2R(ptList,i+1, e);
}

// Compare two points according to dictionary order. If a point is on left
// of the other point, return true. If their x values are the same, then 
// compare their y value, consider the point with bigger y is on the left.
// Here P1 and P2 must not be the same.
BOOL CPointSet::IsP1LeftOfP2(const CPoint3D& P1, const CPoint3D& P2)
{
	if (P1.x<P2.x) return TRUE;
	if (P1.x>P2.x) return FALSE;
	if (P1.y>P2.y) return TRUE;
	return FALSE;
}

// On the right or the same
BOOL CPointSet::IsP1NotLeftOfP2(const CPoint3D& P1, const CPoint3D& P2)
{
	if (P1.x>P2.x) return TRUE;
	if (P1.x<P2.x) return FALSE;
	if (P1.y>P2.y) return FALSE;
	return TRUE;
}

void CPointSet::RemoveSamePts(Point3DAry& ptList, BOOL HasBeenSorted/* =TRUE */) const
{
	int sz=ptList.GetSize();
	if (FALSE==HasBeenSorted)
		QSortL2R(ptList,0,sz-1);
	
	int k=1;
	while (k<ptList.GetSize())
	{
		if (*ptList[k-1]==*ptList[k])
		{
			delete ptList[k];
			ptList[k]=NULL;
			ptList.RemoveAt(k);
		}
		else
		{
			k++;
		}
	}
}

int CPointSet::FindExtremPt(int nWhichDir)
{
	int sz=m_pts.GetSize();
	if (0==sz) 
	{
		AfxMessageBox(_T("There is no point in the point-cloud. Check it again."));
		return -1;
	}

	int nExt=0;
	int i;
	double dExt;

	switch (nWhichDir)
	{
	case MIN_X:
		dExt=m_pts[0]->x;
		for (i=1;i<sz;i++)
		{
			if (m_pts[i]->x < dExt)
			{
				dExt=m_pts[i]->x;
				nExt=i;
			}
		}
		break;

	case MAX_X:
		dExt=m_pts[0]->x;
		for (i=1;i<sz;i++)
		{
			if (m_pts[i]->x > dExt)
			{
				dExt=m_pts[i]->x;
				nExt=i;
			}
		}
		break;

	case MIN_Y:
		dExt=m_pts[0]->y;
		for (i=1;i<sz;i++)
		{
			if (m_pts[i]->y < dExt)
			{
				dExt=m_pts[i]->y;
				nExt=i;
			}
		}
		break;

	case MAX_Y:
		dExt=m_pts[0]->y;
		for (i=1;i<sz;i++)
		{
			if (m_pts[i]->y > dExt)
			{
				dExt=m_pts[i]->y;
				nExt=i;
			}
		}
		break;

	case MIN_Z:
		dExt=m_pts[0]->z;
		for (i=1;i<sz;i++)
		{
			if (m_pts[i]->z < dExt)
			{
				dExt=m_pts[i]->z;
				nExt=i;
			}
		}
		break;

	case MAX_Z:
		dExt=m_pts[0]->z;
		for (i=1;i<sz;i++)
		{
			if (m_pts[i]->z > dExt)
			{
				dExt=m_pts[i]->z;
				nExt=i;
			}
		}
		break;
	}

	return nExt;
}

double CPointSet::FindExtreme(int nWhichDir) const 
{
	int sz=m_pts.GetSize();
	if (0==sz) 
	{
		AfxMessageBox(_T("There is no point in the point-cloud. Check it again."));
		return -1;
	}

	int i;
	double dExt;

	switch (nWhichDir)
	{
	case MIN_X:
		dExt=m_pts[0]->x;
		for (i=1;i<sz;i++)
		{
			if (m_pts[i]->x < dExt)
			{
				dExt=m_pts[i]->x;
			}
		}
		break;

	case MAX_X:
		dExt=m_pts[0]->x;
		for (i=1;i<sz;i++)
		{
			if (m_pts[i]->x > dExt)
			{
				dExt=m_pts[i]->x;
			}
		}
		break;

	case MIN_Y:
		dExt=m_pts[0]->y;
		for (i=1;i<sz;i++)
		{
			if (m_pts[i]->y < dExt)
			{
				dExt=m_pts[i]->y;
			}
		}
		break;

	case MAX_Y:
		dExt=m_pts[0]->y;
		for (i=1;i<sz;i++)
		{
			if (m_pts[i]->y > dExt)
			{
				dExt=m_pts[i]->y;
			}
		}
		break;

	case MIN_Z:
		dExt=m_pts[0]->z;
		for (i=1;i<sz;i++)
		{
			if (m_pts[i]->z < dExt)
			{
				dExt=m_pts[i]->z;
			}
		}
		break;

	case MAX_Z:
		dExt=m_pts[0]->z;
		for (i=1;i<sz;i++)
		{
			if (m_pts[i]->z > dExt)
			{
				dExt=m_pts[i]->z;
			}
		}
		break;
	}
	return dExt;
}

void CPointSet::GntRandomPts(int nHowMany)
{
	int sz=m_pts.GetSize();
	for (int i=sz-1;i>=0;i--)
	{
		delete m_pts[i];
		m_pts[i]=NULL;
	}
	m_pts.RemoveAll();

	for (int i=0;i<nHowMany;i++)
	{
		CPoint3D* pPt=new CPoint3D;
		pPt->x=rand()%1500+(double)rand()/(double)RAND_MAX;
		pPt->y=rand()%1000+(double)rand()/(double)RAND_MAX;
		pPt->z=0;
		m_pts.Add(pPt);
	}
}

//// Please refer to Chapter 9 of "Computaional Geometry__Algorithm and Application__Third Edition" //
///*void CPointSet::Triangulate(CTriangleSet* pTriSetOut, BOOL bRandomize/* =TRUE */)
//{
//	if (m_pts.GetSize()<3) return;
//
//	if (bRandomize) Randomize();
//
//	CTypedPtrArray<CObArray,PTriangle*> triList;
//	PTriangle* pRoot=new PTriangle;
//	GntSuperTriangle(pRoot);
//	triList.Add(pRoot);
//
//	PTriangle* pTri=NULL;
//	PEdge* pEdge=NULL;
//	int nR=0;
//
//	int sz=m_pts.GetSize();
//	for (int i=0;i<sz;i++)
//	{
//		nR=FindTriangle(m_pts[i],pRoot,pTri,pEdge); // 参数传出的时候只有当指针有地址时才能赋值上去
//
//		if (1==nR) 
//			Split_1_to_3(m_pts[i],pTri,triList);
//		else       
//			Split_2_to_4(m_pts[i],pEdge,triList);
//	}
//
//	RemoveBadTriangles(triList,pTriSetOut);
//}
//
//void CPointSet::GntSuperTriangle(PTriangle* pRoot)
//{
//	double minX=m_pts[FindExtremPt(MIN_X)]->x;
//	double maxX=m_pts[FindExtremPt(MAX_X)]->x;
//	double minY=m_pts[FindExtremPt(MIN_Y)]->y;
//	double maxY=m_pts[FindExtremPt(MAX_Y)]->y;
//	double z=m_pts[0]->z;
//
//	double w=maxX-minX;
//	double h=maxY-minY;
//
//	double l=w;
//	if (h>l) l=h;
//
//	double x=(minX+maxX)/2;
//	double y=minY;
//
//    double sqrt3=sqrt(3.0);
//	CPoint3D  M(x,y+sqrt3*l/3,z);
//	CPoint3D  A(x-l,y,z);
//	CPoint3D  B(x+l,y,z);
//	CPoint3D  C(x,y+sqrt3*l,z);
//	CVector3D v_MA=A-M;
//	CVector3D v_MB=B-M;
//	CVector3D v_MC=C-M;
//	double factor=1.0e2;
//
//	CPoint3D* pA=new CPoint3D(M+v_MA*factor);
//	CPoint3D* pB=new CPoint3D(M+v_MB*factor);
//	CPoint3D* pC=new CPoint3D(M+v_MC*factor);
//
//	pRoot->Create(pA,pB,pC,new PEdge(pRoot,NULL),new PEdge(pRoot,NULL),new PEdge(pRoot,NULL));
//}
//
//void CPointSet::Split_1_to_3(CPoint3D* pAddedPt, PTriangle* pTri, PTRILIST& triList)
//{
//	pTri->m_bDeleted=TRUE;
//
//	pTri->m_pTri1=new PTriangle;
//	pTri->m_pTri2=new PTriangle;
//	pTri->m_pTri3=new PTriangle;
//
//	triList.Add(pTri->m_pTri1);
//	triList.Add(pTri->m_pTri2);
//	triList.Add(pTri->m_pTri3);
//
//	pTri->m_pTri1->Create(pTri->m_pA,pTri->m_pB,pAddedPt);
//	pTri->m_pTri2->Create(pAddedPt,pTri->m_pB,pTri->m_pC);
//	pTri->m_pTri3->Create(pTri->m_pA,pAddedPt,pTri->m_pC);
//
//	pTri->m_pTri1->m_pEdgeAB=pTri->m_pEdgeAB;
//	if (pTri==pTri->m_pEdgeAB->m_pLTri)
//		pTri->m_pTri1->m_pEdgeAB->m_pLTri=pTri->m_pTri1;
//	else
//		pTri->m_pTri1->m_pEdgeAB->m_pRTri=pTri->m_pTri1;
//	pTri->m_pTri1->m_pEdgeBC=new PEdge(pTri->m_pTri1,pTri->m_pTri2);
//	pTri->m_pTri1->m_pEdgeCA=new PEdge(pTri->m_pTri1,pTri->m_pTri3);
//
//	pTri->m_pTri2->m_pEdgeAB=pTri->m_pTri1->m_pEdgeBC;
//	pTri->m_pTri2->m_pEdgeBC=pTri->m_pEdgeBC;
//	if (pTri==pTri->m_pEdgeBC->m_pLTri)
//		pTri->m_pTri2->m_pEdgeBC->m_pLTri=pTri->m_pTri2;
//	else
//		pTri->m_pTri2->m_pEdgeBC->m_pRTri=pTri->m_pTri2;
//	pTri->m_pTri2->m_pEdgeCA=new PEdge(pTri->m_pTri2,pTri->m_pTri3);
//
//	pTri->m_pTri3->m_pEdgeAB=pTri->m_pTri1->m_pEdgeCA;
//	pTri->m_pTri3->m_pEdgeBC=pTri->m_pTri2->m_pEdgeCA;
//	pTri->m_pTri3->m_pEdgeCA=pTri->m_pEdgeCA;
//	if (pTri==pTri->m_pEdgeCA->m_pLTri)
//		pTri->m_pTri3->m_pEdgeCA->m_pLTri=pTri->m_pTri3;
//	else
//		pTri->m_pTri3->m_pEdgeCA->m_pRTri=pTri->m_pTri3;
//
//	LegalizeEdge(pAddedPt,pTri->m_pEdgeAB,triList); // Based on new edge
//	LegalizeEdge(pAddedPt,pTri->m_pEdgeBC,triList);
//	LegalizeEdge(pAddedPt,pTri->m_pEdgeCA,triList);
//}
//
//void CPointSet::Split_2_to_4(CPoint3D* pAddedPt, PEdge* pEdge, PTRILIST& triList)
//{
//	PTriangle* pTri1=pEdge->m_pLTri;
//	PTriangle* pTri2=pEdge->m_pRTri;
//
//	pTri1->m_bDeleted=TRUE;
//	pTri2->m_bDeleted=TRUE;
//
//	CPoint3D* pA1=pTri1->m_pA;
//	CPoint3D* pB1=pTri1->m_pB;
//	CPoint3D* pC1=pTri1->m_pC;
//	CPoint3D* pA2=pTri2->m_pA;
//	CPoint3D* pB2=pTri2->m_pB;
//	CPoint3D* pC2=pTri2->m_pC;
//	PEdge* pEdge_A1B1=pTri1->m_pEdgeAB;
//	PEdge* pEdge_C1A1=pTri1->m_pEdgeCA;
//	PEdge* pEdge_A2B2=pTri2->m_pEdgeAB;
//	PEdge* pEdge_C2A2=pTri2->m_pEdgeCA;
//
//	if (pEdge==pTri1->m_pEdgeAB)
//	{
//		pA1=pTri1->m_pC;
//		pB1=pTri1->m_pA;
//		pC1=pTri1->m_pB;
//		pEdge_A1B1=pTri1->m_pEdgeCA;
//		pEdge_C1A1=pTri1->m_pEdgeBC;
//	}
//	if (pEdge==pTri1->m_pEdgeCA)
//	{
//		pA1=pTri1->m_pB;
//		pB1=pTri1->m_pC;
//		pC1=pTri1->m_pA;
//		pEdge_A1B1=pTri1->m_pEdgeBC;
//		pEdge_C1A1=pTri1->m_pEdgeAB;
//	}
//
//	if (pEdge==pTri2->m_pEdgeAB)
//	{
//		pA2=pTri2->m_pC;
//		pB2=pTri2->m_pA;
//		pC2=pTri2->m_pB;
//		pEdge_A2B2=pTri2->m_pEdgeCA;
//		pEdge_C2A2=pTri2->m_pEdgeBC;
//	}
//	if (pEdge==pTri2->m_pEdgeCA)
//	{
//		pA2=pTri2->m_pB;
//		pB2=pTri2->m_pC;
//		pC2=pTri2->m_pA;
//		pEdge_A2B2=pTri2->m_pEdgeBC;
//		pEdge_C2A2=pTri2->m_pEdgeAB;
//	}
//
//	pTri1->m_pTri1=new PTriangle;
//	pTri1->m_pTri2=new PTriangle;
//	pTri2->m_pTri1=new PTriangle;
//	pTri2->m_pTri2=new PTriangle;
//
//	triList.Add(pTri1->m_pTri1);
//	triList.Add(pTri1->m_pTri2);
//	triList.Add(pTri2->m_pTri1);
//	triList.Add(pTri2->m_pTri2);
//
//	PEdge* pNewEdge1=new PEdge(pTri1->m_pTri1,pTri1->m_pTri2);
//	PEdge* pNewEdge2=new PEdge(pTri1->m_pTri2,pTri2->m_pTri2);
//	PEdge* pNewEdge3=new PEdge(pTri2->m_pTri1,pTri2->m_pTri2);
//	PEdge* pNewEdge4=new PEdge(pTri1->m_pTri1,pTri2->m_pTri1);
//
//	if (pEdge_A1B1->m_pLTri==pTri1)
//		pEdge_A1B1->m_pLTri=pTri1->m_pTri2;
//	else
//		pEdge_A1B1->m_pRTri=pTri1->m_pTri2;
//
//	if (pEdge_C1A1->m_pLTri==pTri1)
//		pEdge_C1A1->m_pLTri=pTri1->m_pTri1;
//	else
//		pEdge_C1A1->m_pRTri=pTri1->m_pTri1;
//
//	if (pEdge_A2B2->m_pLTri==pTri2)
//		pEdge_A2B2->m_pLTri=pTri2->m_pTri1;
//	else
//		pEdge_A2B2->m_pRTri=pTri2->m_pTri1;
//
//	if (pEdge_C2A2->m_pLTri==pTri2)
//		pEdge_C2A2->m_pLTri=pTri2->m_pTri2;
//	else
//		pEdge_C2A2->m_pRTri=pTri2->m_pTri2;
//
//	pTri1->m_pTri1->Create(pA1,pAddedPt,pC1,pNewEdge1,pNewEdge4,pEdge_C1A1);
//	pTri1->m_pTri2->Create(pA1,pB1,pAddedPt,pEdge_A1B1,pNewEdge2,pNewEdge1);
//	pTri2->m_pTri1->Create(pA2,pB2,pAddedPt,pEdge_A2B2,pNewEdge4,pNewEdge3);
//	pTri2->m_pTri2->Create(pA2,pAddedPt,pC2,pNewEdge3,pNewEdge2,pEdge_C2A2);
//
//	LegalizeEdge(pAddedPt,pEdge_C1A1,triList);
//	LegalizeEdge(pAddedPt,pEdge_A1B1,triList);
//	LegalizeEdge(pAddedPt,pEdge_A2B2,triList);
//	LegalizeEdge(pAddedPt,pEdge_C2A2,triList);
//}
//
////	This function may still have problem, but I cannot find it!
//void CPointSet::LegalizeEdge(CPoint3D *pPt, PEdge *pEdge, PTRILIST& triList) 
//{
//	if (!IsEdgeLegal(pPt,pEdge))
//	{
//		PTriangle* pTri1=pEdge->m_pRTri;  // Suppose *pTri1 contains point *pPt
//		PTriangle* pTri2=pEdge->m_pLTri;
//
//		if (NULL==pTri1 || NULL==pTri2) return;
//
//		pTri1->m_bDeleted=TRUE;  
//		pTri2->m_bDeleted=TRUE;
//
//		if (pEdge->m_pLTri->m_pA==pPt || pEdge->m_pLTri->m_pB==pPt || pEdge->m_pLTri->m_pC==pPt)
//		{
//			pTri1=pEdge->m_pLTri;  // Make *pTri1 contains point *pPt
//			pTri2=pEdge->m_pRTri;
//		}
//
//		CPoint3D* pA1=pTri1->m_pA;
//		CPoint3D* pB1=pTri1->m_pB;
//		CPoint3D* pC1=pTri1->m_pC;
//		CPoint3D* pA2=pTri2->m_pA;
//		CPoint3D* pB2=pTri2->m_pB;
//		CPoint3D* pC2=pTri2->m_pC;
//		PEdge* pEdge_A1B1=pTri1->m_pEdgeAB;
//		PEdge* pEdge_C1A1=pTri1->m_pEdgeCA;
//		PEdge* pEdge_A2B2=pTri2->m_pEdgeAB;
//		PEdge* pEdge_C2A2=pTri2->m_pEdgeCA;
//
//		if (pEdge==pTri1->m_pEdgeAB)  
//		{
//			pA1=pTri1->m_pC;
//			pB1=pTri1->m_pA;
//			pC1=pTri1->m_pB;
//			pEdge_A1B1=pTri1->m_pEdgeCA;
//			pEdge_C1A1=pTri1->m_pEdgeBC;
//		}
//		if (pEdge==pTri1->m_pEdgeCA)
//		{
//			pA1=pTri1->m_pB;
//			pB1=pTri1->m_pC;
//			pC1=pTri1->m_pA;
//			pEdge_A1B1=pTri1->m_pEdgeBC;
//			pEdge_C1A1=pTri1->m_pEdgeAB;
//		}
//
//		if (pEdge==pTri2->m_pEdgeAB)
//		{
//			pA2=pTri2->m_pC;
//			pB2=pTri2->m_pA;
//			pC2=pTri2->m_pB;
//			pEdge_A2B2=pTri2->m_pEdgeCA;
//			pEdge_C2A2=pTri2->m_pEdgeBC;
//		}
//		if (pEdge==pTri2->m_pEdgeCA)
//		{
//			pA2=pTri2->m_pB;
//			pB2=pTri2->m_pC;
//			pC2=pTri2->m_pA;
//			pEdge_A2B2=pTri2->m_pEdgeBC;
//			pEdge_C2A2=pTri2->m_pEdgeAB;
//		}
//
//		PTriangle* pNewTri1=new PTriangle;
//		PTriangle* pNewTri2=new PTriangle;
//
//        triList.Add(pNewTri1);
//		triList.Add(pNewTri2);
//
//		if (pEdge_A1B1->m_pLTri==pTri1)
//			pEdge_A1B1->m_pLTri=pNewTri1;
//		else
//			pEdge_A1B1->m_pRTri=pNewTri1;
//
//		if (pEdge_C2A2->m_pLTri==pTri2)
//			pEdge_C2A2->m_pLTri=pNewTri1;
//		else
//			pEdge_C2A2->m_pRTri=pNewTri1;
//
//		if (pEdge_A2B2->m_pLTri==pTri2)
//			pEdge_A2B2->m_pLTri=pNewTri2;
//		else
//			pEdge_A2B2->m_pRTri=pNewTri2;
//
//		if (pEdge_C1A1->m_pLTri==pTri1)
//			pEdge_C1A1->m_pLTri=pNewTri2;
//		else
//			pEdge_C1A1->m_pRTri=pNewTri2;
//
//		PEdge* pNewEdge=new PEdge;         // Here to be changed!
//		pNewEdge->m_pLTri=pNewTri1;
//		pNewEdge->m_pRTri=pNewTri2;
//
//		pNewTri1->Create(pB1,pA2,pA1,pEdge_C2A2,pNewEdge,pEdge_A1B1);
//		pNewTri2->Create(pC1,pA1,pA2,pEdge_C1A1,pNewEdge,pEdge_A2B2);
//
//		pTri1->m_pTri1=pNewTri1;
//		pTri1->m_pTri2=pNewTri2;
//		pTri2->m_pTri1=pNewTri1;
//		pTri2->m_pTri2=pNewTri2;
//
//		LegalizeEdge(pPt,pEdge_C2A2,triList);
//		LegalizeEdge(pPt,pEdge_A2B2,triList);
//	}
//}
//
//BOOL CPointSet::IsEdgeLegal(CPoint3D* pPt, PEdge* pEdge)
//{
//	PTriangle* pTri=NULL;
//	if (pEdge->m_pLTri->m_pA==pPt || pEdge->m_pLTri->m_pB==pPt || pEdge->m_pLTri->m_pC==pPt)
//		pTri=pEdge->m_pRTri;
//	else
//		pTri=pEdge->m_pLTri;
//
//	if (NULL==pTri) return TRUE;
//
//	CPoint3D ptM_AB=(*pTri->m_pA + *pTri->m_pB)*0.5;
//	CPoint3D ptM_BC=(*pTri->m_pB + *pTri->m_pC)*0.5;
//	CVector3D v_n_AB=GetOrthoVectorOnXYPlane(*pTri->m_pA,*pTri->m_pB);
//	CVector3D v_n_BC=GetOrthoVectorOnXYPlane(*pTri->m_pB,*pTri->m_pC);
//
//	CPoint3D ptCenter;
//	GetCrossPoint(ptCenter,ptM_AB,ptM_AB+v_n_AB,ptM_BC,ptM_BC+v_n_BC);
//	double R2=(*pTri->m_pA-ptCenter).GetMod2();
//	double d2=(*pPt-ptCenter).GetMod2();
//
//	if ((d2-R2)>-1.0e-6)
//		return TRUE;
//	else
//		return FALSE;
//}
//
//// return value: 
////				0 means pt is outside trianle
////				1 means pt is in triangle
////				2 means pt is on border AB
////				3 means pt is on border BC
////				4 means pt is on border CA
//int CPointSet::IsPtInTriangle(CPoint3D *pPt, PTriangle *pTri, PEdge*& edgeAddress_out)
//{
//	double ab,bc,ca;
//	CVector3D v1,v2;
//
//	v1=*pTri->m_pA - *pPt;
//	v2=*pTri->m_pB - *pPt;
//	ab=(v1*v2).dz;
//	v1=*pTri->m_pB - *pPt;
//	v2=*pTri->m_pC - *pPt;
//	bc=(v1*v2).dz;
//	v1=*pTri->m_pC - *pPt;
//	v2=*pTri->m_pA - *pPt;
//	ca=(v1*v2).dz;
//
//	if ((ab>0 && bc>0 && ca>0) || (ab<0 && bc<0 && ca<0)) return 1;
//	if (0==ab)
//	{
//		edgeAddress_out=pTri->m_pEdgeAB;
//		return PT_ON_EDGE_AB;
//	}
//	if (0==bc) 
//	{
//		edgeAddress_out=pTri->m_pEdgeBC;
//		return PT_ON_EDGE_BC;
//	}
//	if (0==ca) 
//	{
//		edgeAddress_out=pTri->m_pEdgeCA;
//		return PT_ON_EDGE_CA;
//	}
//	return 0;
//}
//
//int CPointSet::FindTriangle(CPoint3D* pPt, PTriangle* pRoot, PTriangle*& triAddress_out, PEdge*& edgeAddress_out)
//{
//	PTriangle* pCurTri=pRoot;
//	int nR=-100;
//
//    while (TRUE)
//	{
//		if (NULL!=pCurTri->m_pTri3)
//		{
//			nR=IsPtInTriangle(pPt,pCurTri->m_pTri3,edgeAddress_out);
//			if (nR>0)
//			{
//				pCurTri=pCurTri->m_pTri3;
//				if (NULL==pCurTri->m_pTri1)
//				{
//					triAddress_out=pCurTri;
//					return nR;
//				}
//				else continue;
//			}
//		}
//		if (NULL!=pCurTri->m_pTri2)
//		{
//			nR=IsPtInTriangle(pPt,pCurTri->m_pTri2,edgeAddress_out);
//			if (nR>0)
//			{
//				pCurTri=pCurTri->m_pTri2;
//				if (NULL==pCurTri->m_pTri1)
//				{
//					triAddress_out=pCurTri;
//					return nR;
//				}
//				else continue;
//			}
//		}
//		if (NULL!=pCurTri->m_pTri1)
//		{
//			nR=IsPtInTriangle(pPt,pCurTri->m_pTri1,edgeAddress_out);
//			if (nR>0)
//			{
//				pCurTri=pCurTri->m_pTri1;
//				if (NULL==pCurTri->m_pTri1)
//				{
//					triAddress_out=pCurTri;
//					return nR;
//				}
//				else continue;
//			}
//		}
//
//		triAddress_out=pCurTri;
//		return 1;
//	}
//
//	return NULL;
//}
//
//
//void CPointSet::RemoveBadTriangles(PTRILIST& triList, CTriangleSet* pTriSetOut)
//{
//	int sz=triList.GetSize();
//
//	CPoint3D* pA=triList[0]->m_pA;
//	CPoint3D* pB=triList[0]->m_pB;
//	CPoint3D* pC=triList[0]->m_pC;
//
//	for (int i=0;i<sz;i++)
//	{
//		if (pA==triList[i]->m_pA || pA==triList[i]->m_pB || pA==triList[i]->m_pC ||
//			pB==triList[i]->m_pA || pB==triList[i]->m_pB || pB==triList[i]->m_pC ||
//			pC==triList[i]->m_pA || pC==triList[i]->m_pB || pC==triList[i]->m_pC)
//		{
//			triList[i]->m_bDeleted=TRUE;
//		}
//	}
//
//	for (int i=0;i<sz;i++)
//	{
//		if (FALSE==triList[i]->m_bDeleted)
//		{
//			PTriangle* pTri=new PTriangle(triList[i]->m_pA,triList[i]->m_pB,triList[i]->m_pC);
//			pTriSetOut->Add(pTri);
//		}
//	}
//
//	for (int i=sz-1;i>=0;i--)
//	{
//		delete triList[i];
//		triList[i]=NULL;
//	}
//	triList.RemoveAll();
//}
//
void CPointSet::Randomize()
{
	int sz=m_pts.GetSize();

	CPoint3D* ptTmp;   // 用指针比较好
	int r;
	for (int i=sz-1;i>=1;i--)
	{
		r=rand()%i;
		ptTmp=m_pts[i];
		m_pts[i]=m_pts[r];
		m_pts[r]=ptTmp;
	}
}