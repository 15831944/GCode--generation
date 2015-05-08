#include "StdAfx.h"
#include "Layer.h"
#include "AzClipper.h"
#include "global.h"
#include "ZigzagPathGen.h"

#define random(x) (rand()%x)


CLayer::CLayer(void)
{
}

CLayer::~CLayer(void)
{
}

CPolyline& CLayer::Bspline2Polyline( CBSpline& spline )
{
	CPolyline* TempPolyline;
	TempPolyline = new CPolyline();
	spline.UnitizeKnot();
	for (int i = 0;i < 10000; i++)
	{
		TempPolyline->AddPt(spline.P((double)i/(double)10000));
	}
	return *TempPolyline;
}


void CLayer::GntEffectPath(double linespace,double angle)
{ 
	if (!m_path.empty())
	{
		for(int i=0; i<m_path.size();i++)
		{
			for (int j=0;j<m_path.at(i)->size();j++)
			{
				delete m_path.at(i)->at(j);
				m_path.at(i)->at(j) = NULL;
			}
			delete m_path.at(i);
			m_path.at(i) = NULL;
		}
		for(vector<PolyLineGroup*>::iterator it = m_path.begin(); it != m_path.end();)
		{
			it = m_path.erase( it ); 
		}
	}

	PolyLineGroup* pathforsingle;
	CPolyline *TempPoly;
	for (int i=0;i<m_InPolylines.size();i++)
	{
		// reference points
		CPoint3D *pt1;
		pt1 = m_InPolylines.at(i)->m_pts.GetAt(0);

		CPoint3D *pt2;
		pt2 = m_InPolylines.at(i)->m_pts.GetAt(1);

		CPoint3D *pt3;
		pt3 = m_InPolylines.at(i)->m_pts.GetAt(2);

		CPoint3D *pt4;
		pt4 = m_InPolylines.at(i)->m_pts.GetAt(3);


		//*********************************************
		//Filling Path
		pathforsingle = new PolyLineGroup;
		TempPoly = new CPolyline();

		CPolyline* TempPolyline;
		PolyLineGroup OriginalPolys;
		PolyLineGroup* OffsetPolys;

		CZigzagPathGen *pZigPath;
		CPoint3D *ptRef;
		CPoint3D *newPt;

		newPt = new CPoint3D(*pt1);
		TempPoly->AddPt(newPt);

		newPt = new CPoint3D(*pt2);
		TempPoly->AddPt(newPt);

		newPt = new CPoint3D(*pt3);
		TempPoly->AddPt(newPt);

		newPt = new CPoint3D(*pt4);
		TempPoly->AddPt(newPt);

		newPt = new CPoint3D(*pt1);
		TempPoly->AddPt(newPt);

		//pathforsingle->push_back(TempPoly);

		TempPolyline = TempPoly;
		TempPolyline->DelColinearPt();

		OriginalPolys.clear();
		TempPolyline->MakePolygonCCW();
		OriginalPolys.push_back(TempPolyline);
		OffsetPolys = new PolyLineGroup;
		OffsetPolys->clear();

		ptRef = TempPolyline->m_pts.GetAt(TempPolyline->m_pts.GetSize()-1);

		double offset_dis = linespace;

		//zigzag path
		pZigPath = new CZigzagPathGen();
		pZigPath->GetInputs(OriginalPolys,offset_dis, angle);
		pZigPath->GntSweepPath();

		PolyLineGroup *TmPPolys = new PolyLineGroup;;
		PolyLineGroup *PolTmPPolys = new PolyLineGroup;
		pZigPath->OutPutPolys(*TmPPolys);
		for (int i=0;i<TmPPolys->size();i++)
		{
			pathforsingle->push_back(TmPPolys->at(i));
		}

		m_path.push_back(pathforsingle);
	}
}



void CLayer::Clear()
{
	// 	if (!m_path.empty())
	// 	{
	// 		for(vector<CPolyline*>::iterator it = m_path.begin(); it != m_path.end();)
	// 		{
	// 			it = m_path.erase( it ); 
	// 
	// 		}
	// 	}

	m_GcodesArray.RemoveAll();
}

void CLayer::SetLayerZValue( double z )
{
	m_z =z;
}

void CLayer::Draw( COpenGLDC* pDC )
{
	int sz = m_path.size();
	if (sz == 0)
	{
		for (int i=0; i<m_InPolylines.size();i++)
		{
			m_InPolylines[i]->Draw(pDC,RGB(255,255,255));
		}
	}

	for (int i=0;i<m_TestPolyLines.size(); i++)
	{

		/*	CString str;
		CPoint3D pt = *(m_TestPolyLines.at(i)->m_pts).GetAt(0);
		str.Format(_T("%.3f, %.3f"), pt.x, pt.y);
		MessageBox(NULL, str,str,0);*/
		pDC->DrawPoint(*(m_TestPolyLines.at(i)->m_pts).GetAt(0),20);
	}

	for (int i=0;i<m_PointsOnOutsideSplines.size();i++)
	{
		for (int j=0; j<m_PointsOnOutsideSplines.at(i)->size();j++)
		{
			pDC->DrawPoint(m_PointsOnOutsideSplines.at(i)->at(j)->pt,10);
		}
	}

	for (int i=0;i<m_PointsOnInsideSplines.size();i++)
	{
		for (int j=0; j<m_PointsOnInsideSplines.at(i)->size();j++)
		{
			pDC->DrawPoint(m_PointsOnInsideSplines.at(i)->at(j)->pt,10);
		}
	}
	for (int i=0; i<m_pWavies.size();i++)
	{
		m_pWavies.at(i)->Draw(pDC,RGB(0,255,0));  
	}

	for (int i=0; i<m_path.size();i = i++)
	{
		for (int j=0; j<m_path.at(i)->size();j++)
		{
			if (i == 0)
			{
				m_path.at(i)->at(j)->Draw(pDC,RGB(255,255,255));
			}
			else
			{
				m_path.at(i)->at(j)->Draw(pDC,RGB(255,255,255));
			}

		}
	}
}

void CLayer::GntLayerGcodes()
{
	CString strOpen = _T("M106 \nG4 P150");
	CString strClose=_T("M107 \nG4 P150");
	CString strDwell=_T("G4 P500");

		m_GcodesArray.RemoveAll();

		m_GcodesArray.Add(strClose);
		CString str;
		str.Format(_T("G1 Z%.3f F900.000"),m_z);
		m_GcodesArray.Add(str);

		int a = 0;
		for (int j = 0; j < m_path.at(a)->size(); j++)
		{
			str.Format(_T("G1 X%.3f Y%.3f F900.000"),m_path.at(a)->at(j)->m_pts[0]->x,m_path.at(a)->at(j)->m_pts[0]->y);
			m_GcodesArray.Add(str);
			m_GcodesArray.Add(strOpen);
			str.Format(_T("G1 X%.3f Y%.3f F900.000"),m_path.at(a)->at(j)->m_pts[1]->x,m_path.at(a)->at(j)->m_pts[1]->y);
			m_GcodesArray.Add(str);
			for (int k=2; k<m_path.at(a)->at(j)->m_pts.GetSize();k++)
			{
				str.Format(_T("G1 X%.3f Y%.3f"),m_path.at(a)->at(j)->m_pts[k]->x,m_path.at(a)->at(j)->m_pts[k]->y);
				m_GcodesArray.Add(str);
			}
			m_GcodesArray.Add(strClose);
		}
}



void CLayer::CalculatePointsOnSplines( double distance,double rate_distance )
{

	CBSpline* pOutSpline;
	CBSpline* pInSpline;

	PointsOnSpline* pPointsOnSplineIn;
	PointsOnSpline* pPointsOnSplineOut;

	pPointsOnSplineIn = new PointsOnSpline;
	pPointsOnSplineOut = new PointsOnSpline;

	pOutSpline = (CBSpline*)m_splines.at(0);
	pOutSpline->UnitizeKnot();
	double Length1 = pOutSpline->GetLength();

	pInSpline = (CBSpline*)m_splines.at(1);
	pInSpline->UnitizeKnot();
	double Length2 = pInSpline->GetLength();

	if (Length1 < Length2)
	{
		int nNumber = Length2/distance;
		double LengthEach = Length2/nNumber;
		for (int i=0; i<=nNumber; i++)
		{
			double u=0;
			u = (double)i/(double)nNumber;
			if (u>=1)
			{
				u=0.9999;
			}
			PointEx * pt;
			pt = new PointEx;
			pt->u = u;
			pt->pt = pInSpline->P(u);
			pPointsOnSplineOut->push_back(pt);

			pt = new PointEx;
			u= ((double)i+0.5)/(double)nNumber;
			pt->u = u;
			if (u<1)
			{
				pt->pt = pOutSpline->P(u);
				pPointsOnSplineIn->push_back(pt);
			}

		}
	}
	else
	{
		int nNumber = Length1/distance;
		double LengthEach = Length1/nNumber;
		for (int i=0; i<=nNumber; i++)
		{
			double u=0;
			u = (double)i/(double)nNumber;
			if (u>=1)
			{
				u=0.9999;
			}
			PointEx * pt;
			pt = new PointEx;
			pt->u = u;
			pt->pt = pOutSpline->P(u);
			pPointsOnSplineOut->push_back(pt);

			pt = new PointEx;
			u= ((double)i+0.5)/(double)nNumber;
			pt->u = u;
			if (u<1)
			{
				pt->pt = pInSpline->P(u);
				pPointsOnSplineIn->push_back(pt);
			}

		}
	}
	m_PointsOnOutsideSplines.push_back(pPointsOnSplineOut);
	m_PointsOnInsideSplines.push_back(pPointsOnSplineIn);
}



void CLayer::GenerateWavySpline()
{
	CBSpline* pWavy;

	pWavy = new CBSpline();
	CPoint3D *pt;
	for (int j=0; j<m_PointsOnOutsideSplines.at(0)->size()-1; j++)
	{
		pt = new CPoint3D(m_PointsOnOutsideSplines.at(0)->at(j)->pt);
		pWavy->AddFitPts(pt);
		pt = new CPoint3D(m_PointsOnInsideSplines.at(0)->at(j)->pt);
		pWavy->AddFitPts(pt);
	}
	int sz = m_PointsOnOutsideSplines.at(0)->size()-1;
	pt = new CPoint3D(m_PointsOnOutsideSplines.at(0)->at(sz)->pt);
	pWavy->AddFitPts(pt);
	pWavy->Interpolate();
	pWavy->Scatter();
	m_pWavies.push_back(pWavy);
}

void CLayer::ClearBuffer()
{
	if (!m_PointsOnOutsideSplines.empty())
	{
		for (int i=0; i<m_PointsOnOutsideSplines.size();i++)
		{
			if (!m_PointsOnOutsideSplines[i]->empty())
			{
				for(vector<PointEx*>::iterator it = m_PointsOnOutsideSplines[i]->begin(); it != m_PointsOnOutsideSplines[i]->end();)
				{
					it = m_PointsOnOutsideSplines[i]->erase( it ); 
				}
			}
		}

	}

	if (!m_PointsOnInsideSplines.empty())
	{
		for (int i=0; i<m_PointsOnInsideSplines.size();i++)
		{
			if (!m_PointsOnInsideSplines[i]->empty())
			{
				for(vector<PointEx*>::iterator it = m_PointsOnInsideSplines[i]->begin(); it != m_PointsOnInsideSplines[i]->end();)
				{
					it = m_PointsOnInsideSplines[i]->erase( it ); 
				}
			}
		}

	}

	if (!m_ExteriorOriginalPointsEx.empty())
	{
		for (int i=0; i<m_ExteriorOriginalPointsEx.size();i++)
		{
			if (!m_ExteriorOriginalPointsEx[i]->empty())
			{
				for(vector<PointEx*>::iterator it = m_ExteriorOriginalPointsEx[i]->begin(); it != m_ExteriorOriginalPointsEx[i]->end();)
				{
					it = m_ExteriorOriginalPointsEx[i]->erase( it ); 
				}
			}
		}

	}

	if (!m_ExteriorOffsetPointsEx.empty())
	{
		for (int i=0; i<m_ExteriorOffsetPointsEx.size();i++)
		{
			if (!m_ExteriorOffsetPointsEx[i]->empty())
			{
				for(vector<PointEx*>::iterator it = m_ExteriorOffsetPointsEx[i]->begin(); it != m_ExteriorOffsetPointsEx[i]->end();)
				{
					it = m_ExteriorOffsetPointsEx[i]->erase( it ); 
				}
			}
		}

	}
}



void CLayer::AddPolyLine( CPolyline* poly )
{
	m_InPolylines.push_back(poly);
}


void CLayer::GntLayerPath (double linespace,double angle)
{
	
	GntEffectPath(linespace, angle);
	GntLayerGcodes();
	ClearBuffer();

}

void CLayer::MovePolyline( double x, double y )
{
	for (int i=0; i<m_InPolylines.size();i++)
	{
		for (int j=0; j<m_InPolylines.at(i)->m_pts.GetSize();j++)
		{
			m_InPolylines.at(i)->m_pts.GetAt(j)->x +=x;
			m_InPolylines.at(i)->m_pts.GetAt(j)->y +=y;

		}
	}
}

void CLayer::InitialProContours(double gap,double overlap)
{
	// if the contour has just one boundary, it need to be seperated
	if (m_InPolylines.size() == 1)
	{

		PolyLineGroup* TmpPolysEx;
		TmpPolysEx = new PolyLineGroup;
		AzClipper::Offset(m_InPolylines,*TmpPolysEx, -gap/2);
		////(m_InPolylines.at(0))->Offset_silly(gap/2,TmpPoly);
		//(m_InPolylines.at(0))->Offset(gap/2);
		CPolyline *TmpPoly;
		TmpPoly = new CPolyline();
		TmpPoly = TmpPolysEx->at(0);


		PolyLineGroup TempPolys;
		double thredshold = 2*PI/3;
		TmpPoly->Sparse();
		Seperate2Polys(*TmpPoly,TempPolys, thredshold);

		for (int i=0; i<TempPolys.size(); i++)
		{
			m_TestPolyLines.push_back(TempPolys.at(i));
		}


		////////////////////////////////////////////////////////////////////////////////////
		// make sure there are only 4 segments for the contour 
		while (TempPolys.size() != 4)
		{
			if (TempPolys.size() >4)
			{
				thredshold = thredshold - 0.001;
				for(vector<CPolyline*>::iterator it = TempPolys.begin(); it != TempPolys.end();)
				{
					it = TempPolys.erase( it ); 
				}
				TempPolys.clear();
				Seperate2Polys(*TmpPoly,TempPolys, thredshold);
			}
			else if (TempPolys.size() <4)
			{
				thredshold = thredshold + 0.001;
				for(vector<CPolyline*>::iterator it = TempPolys.begin(); it != TempPolys.end();)
				{
					it = TempPolys.erase( it ); 
				}
				TempPolys.clear();
				Seperate2Polys(*TmpPoly,TempPolys, thredshold);
			}
			else
			{
				break;
			}
		}
		///////////////////////////////////////////////////////////////////////////////////
		// delete two short segments
		int sz1 = TempPolys.at(0)->m_pts.GetSize();
		int sz2 = TempPolys.at(1)->m_pts.GetSize();
		int sz3 = TempPolys.at(2)->m_pts.GetSize();
		int sz4 = TempPolys.at(3)->m_pts.GetSize();

		if ((sz1>sz2)&&(sz3>sz4))
		{
			TempPolys.erase(TempPolys.begin() + 1);
			TempPolys.erase(TempPolys.begin() + 2);
		}

		else
		{
			TempPolys.erase(TempPolys.begin());
			TempPolys.erase(TempPolys.begin() + 1);
		}
		/////////////////////////////////////////////////////////////////////////////
		//transfer two polylines into splines
		CBSpline* pSline;

		pSline = new CBSpline();
		for (int j=1; j<TempPolys.at(0)->m_pts.GetSize();j++)
		{

			CPoint3D* pt1, *pt2;
			pt1 = TempPolys.at(0)->m_pts.GetAt(j-1);
			pt2 = TempPolys.at(0)->m_pts.GetAt(j);
			double L = GetDistance(*pt1, *pt2);
			if (L >0.01)
			{
				pSline->AddFitPts(TempPolys.at(0)->m_pts.GetAt(j-1));
			}
		} 
		pSline->AddFitPts(TempPolys.at(0)->m_pts.GetAt(TempPolys.at(0)->m_pts.GetSize()-1));
		pSline->Interpolate();
		pSline->Scatter();
		m_splines.push_back(pSline);

		pSline = new CBSpline();
		for (int j=TempPolys.at(1)->m_pts.GetSize()-1; j>1;j--)
		{

			CPoint3D* pt1, *pt2;
			pt1 = TempPolys.at(1)->m_pts.GetAt(j-1);
			pt2 = TempPolys.at(1)->m_pts.GetAt(j);
			double L = GetDistance(*pt1, *pt2);
			if (L >0.01)
			{
				pSline->AddFitPts(TempPolys.at(1)->m_pts.GetAt(j));
			}
		} 
		pSline->AddFitPts(TempPolys.at(1)->m_pts.GetAt(0));
		pSline->Interpolate();
		pSline->Scatter();
		m_splines.push_back(pSline);

	}

	else if ( m_InPolylines.size() == 2)
	{
		CBSpline* pSline;
		for (int i=0; i<m_InPolylines.size();i++)
		{
			pSline = new CBSpline();
			for (int j=0; j<m_InPolylines.at(i)->m_pts.GetSize();j++)
			{
				pSline->AddFitPts(m_InPolylines.at(i)->m_pts.GetAt(j));
			} 
			pSline->Interpolate();
			pSline->Scatter();

			m_splines.push_back(pSline);
		}
	}
	else
	{
		MessageBox(NULL,_T("error"),_T("Number of boundaries is not right"),0);
	}

}

void CLayer::Seperate2Polys(CPolyline& InPoly, PolyLineGroup& OutPoly, double thredshold_angle)
{
	CPolyline* pTmpPoly;
	for (int i=0; i<InPoly.m_pts.GetSize()-1;i++)
	{
		//note: the last point and the first is the same
		CPoint3D pt1, pt2, pt3;
		pt2 = *(InPoly.m_pts.GetAt(i));
		if (i==0)
		{
			pt1 = *(InPoly.m_pts.GetAt(InPoly.m_pts.GetSize()-2));
			pTmpPoly = new CPolyline();
		}
		else
			pt1 = *(InPoly.m_pts.GetAt(i-1));

		pt3 =  *(InPoly.m_pts.GetAt(i+1));

		double Angle = GetAngleBetweenLines(pt1, pt2, pt3);//get the angle at pt2

		if (Angle < thredshold_angle)
		{
			if ((OutPoly.size() == 0)&&pTmpPoly->m_pts.GetSize() == 0)
			{
				pTmpPoly->AddPt(pt2);
			}
			else
			{
				pTmpPoly->AddPt(pt2);
				OutPoly.push_back(pTmpPoly);
				pTmpPoly = new CPolyline();
			}
		}

		//connect the first and the last polyline
		if ( i == InPoly.m_pts.GetSize()-2)
		{
			CPoint3D pt10, pt20, pt30;
			pt10 = *(InPoly.m_pts.GetAt(InPoly.m_pts.GetSize() - 2));
			pt20 = *(InPoly.m_pts.GetAt(InPoly.m_pts.GetSize() - 1));
			pt30 = *(InPoly.m_pts.GetAt(1));

			double Ang = GetAngleBetweenLines(pt10, pt20, pt30);

			pTmpPoly->AddPt(pt20);

			if (Ang > thredshold_angle)
			{
				for (int k=0; k< OutPoly.at(0)->m_pts.GetSize();k++ )
				{
					pTmpPoly->AddPt(OutPoly.at(0)->m_pts.GetAt(k));
				}

				OutPoly.erase(OutPoly.begin());
			}

			OutPoly.push_back(pTmpPoly);
		}
		else
		{
			pTmpPoly->AddPt(pt2);
		}  
	}

}

double CLayer::GetAngleBetweenLines( CPoint3D& pt1,CPoint3D& pt2,CPoint3D& pt3 )
{
	CVector3D v1,v2;
	v1 = pt1 - pt2;
	v2 = pt3 - pt2;
	return GetAngle(v1,v2);
}
