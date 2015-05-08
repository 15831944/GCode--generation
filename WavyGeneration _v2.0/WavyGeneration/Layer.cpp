#include "StdAfx.h"
#include "Layer.h"
#include "AzClipper.h"

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

void CLayer::GntBrimPath( double distance,int n_laps )
{
   if (m_path.empty())
   {
	   MessageBox(NULL,_T("Interior first!!"), _T("Error"),0);
   }

   for (int i=0;i<m_pWavies.size();i++)
   {
	   PolyLineGroup OriginalPolys;
	   PolyLineGroup* OffsetPolys;

	   CPolyline* TempPolyline;
	   TempPolyline = m_path.at(i)->at(m_path.at(i)->size()-1);
	   TempPolyline->DelColinearPt();


	   OriginalPolys.push_back(TempPolyline);

	   for (int j=0; j<n_laps; j++)
	   {
		   OffsetPolys = new vector<CPolyline*>;
		   AzClipper::Offset(OriginalPolys,*OffsetPolys,j*distance);
		   if (OffsetPolys->size()>0)
		   {
			    m_path.at(i)->push_back(OffsetPolys->at(0));
		   }
	   }
   }
}

void CLayer::GntEffctPath()
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

	//actually here we can use the interpolation of BSpline, that is another topic
	PolyLineGroup* pathforsingle;
	for (int i=0;i<m_pWavies.size();i++)
	{
		pathforsingle = new PolyLineGroup;
	   ////////////wavy path//////////////
		CPolyline *TempPoly;
		TempPoly = new CPolyline();
		double L = m_pWavies.at(i)->GetLength();
		m_pWavies.at(i)->UnitizeKnot();
		int nNumber = L/0.2;
		for (int j=0; j<nNumber;j++)
		{
			double u;
			u = (double)j/(double)nNumber;
			if (u>=1)
			{
				u=0.99999;
			}
			CPoint3D* pt;
			pt = new CPoint3D(m_pWavies.at(i)->P(u));
			TempPoly->AddPt(pt);
		}
		pathforsingle->push_back(TempPoly);

		//////////////////////wall #1///////////////////////////
		TempPoly = new CPolyline();
		L = m_splines[i]->GetLength();
		m_splines[i]->UnitizeKnot();
		nNumber = L/0.2;
		for (int j=0; j<nNumber; j++)
		{
			double u;
			u = (double)j/(double)nNumber;
			if (u>=1)
			{
				u=0.99999;
			}
			CPoint3D* pt;
			pt = new CPoint3D(m_splines[i]->P(u));
			TempPoly->AddPt(pt);
		}

		bool isClosed = false; //need to judge whether the wall is closed in advance
		CPoint3D pt1 = m_splines[i]->P(0);
		CPoint3D pt2 = m_splines[i]->P(0.9999);
		if ((fabs(pt1.x-pt2.x)<0.1)&&(fabs(pt1.y-pt2.y)<0.1))
		{
			isClosed = true;
		}

		if (isClosed)
		{
			TempPoly->AddPt(m_splines[i]->P(0.99999));//connect the start and the end of the first wall
			TempPoly->AddPt(m_splines[i]->P(0));//connect the start and the end of the first wall
			//then restart
			pathforsingle->push_back(TempPoly);
			TempPoly = new CPolyline();


		}
		else
		{
			TempPoly->AddPt(m_splines[i]->P(0.99999));//connect these two parts
			TempPoly->AddPt(m_ExteriorOffsetSpline[i]->P(0.99999));//connect these two parts
		}
		

		L = m_ExteriorOffsetSpline[i]->GetLength();
		m_ExteriorOffsetSpline[i]->UnitizeKnot();
		nNumber = L/0.2;
		for (int j=0; j<nNumber; j++)
		{
			double u;
			u = 1-(double)j/(double)nNumber;
			if (u>=1)
			{
				u=0.99999;
			}
			if (u<=0)
			{
				u=0;
			}
			CPoint3D* pt;
			pt = new CPoint3D(m_ExteriorOffsetSpline[i]->P(u));
			TempPoly->AddPt(pt);
		}

		if (isClosed)
		{
			TempPoly->AddPt(m_ExteriorOffsetSpline[i]->P(0));//connect the start and the end of the first wall
			TempPoly->AddPt(m_ExteriorOffsetSpline[i]->P(0.99999));//connect the start and the end of the first wall
		}
		else
		{
			TempPoly->AddPt(m_ExteriorOffsetSpline[i]->P(0));//connect these two parts
			TempPoly->AddPt(m_splines[i]->P(0));//connect these two parts
		}

		pathforsingle->push_back(TempPoly);

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
	int sz = m_splines.size();
	if (sz == 0)
	{
		m_InPolylines[0]->Draw(pDC,RGB(255,0,0));

		if (m_InPolylines.size() > 1)
		{
            m_InPolylines[1]->Draw(pDC,RGB(255,0,255));
		}
// 		for (int i=0; i<m_InPolylines.size();i++)
// 		{
// 			m_InPolylines[i]->Draw(pDC,RGB(255,0,0));
// 		}
	}
	else
	{
		for (int i=0;i<m_splines.size(); i++)
		{
			m_splines.at(i)->Draw(pDC,RGB(255,0,0));
		}

		for (int i=0;i<m_PointsOnOriginalSplines.size();i++)
		{
			for (int j=0; j<m_PointsOnOriginalSplines.at(i)->size();j++)
			{
				pDC->DrawPoint(m_PointsOnOriginalSplines.at(i)->at(j)->pt,10);
			}
		}

		for (int i=0;i<m_PointsOnOffsetSplines.size();i++)
		{
			for (int j=0; j<m_PointsOnOffsetSplines.at(i)->size();j++)
			{
				pDC->DrawPoint(m_PointsOnOffsetSplines.at(i)->at(j)->pt,10);
			}
		}
		for (int i=0; i<m_pWavies.size();i++)
		{
			m_pWavies.at(i)->Draw(pDC,RGB(0,255,0));  
		}

	}
	
	for (int i=0; i<m_path.size();i++)
	{
       for (int j=0; j<m_path.at(i)->size();j++)
       {
		   m_path.at(i)->at(j)->Draw(pDC,RGB(100,100,100));
       }
	}

}

void CLayer::GntFirstLayerGcodes(double Eratio)
{ 
    m_GcodesArray.RemoveAll();
	CString str;
	double Len;

	str = _T("G21");
	m_GcodesArray.Add(str);

	str = _T("G28");
	m_GcodesArray.Add(str);

	str = _T("M203 X192 Y208 Z3");
	m_GcodesArray.Add(str);

	str = _T("G1 Z5 F5000");
	m_GcodesArray.Add(str);

	str = _T("G90");
	m_GcodesArray.Add(str);

	str = _T("M83");
	m_GcodesArray.Add(str);

	str = _T("G1 E-1.00000 F1800");
	m_GcodesArray.Add(str);

	str.Format(_T("G1 Z%.3f F6000.000"),m_z);
	m_GcodesArray.Add(str);

	for (int i=0; i<m_path.size();i++)
	{
		for (int j=0; j<m_path.at(i)->size(); j++)
		{
			str.Format(_T("G1 X%3f Y%.3f F6000.000"),m_path.at(i)->at(j)->m_pts[0]->x,m_path.at(i)->at(j)->m_pts[0]->y);
			m_GcodesArray.Add(str);
			str = _T("G1 E1.00000 F1800");
			m_GcodesArray.Add(str);
			Len = GetDistance(*(m_path.at(i)->at(j)->m_pts[0]),*(m_path.at(i)->at(j)->m_pts[1]));
			str.Format(_T("G1 X%.3f Y%.3f E%.5f F600.000"),m_path.at(i)->at(j)->m_pts[1]->x,m_path.at(i)->at(j)->m_pts[1]->y,Len*Eratio);
			m_GcodesArray.Add(str);
			for (int k=2; k<m_path.at(i)->at(j)->m_pts.GetSize();k++)
			{
				Len = GetDistance(*(m_path.at(i)->at(j)->m_pts[k]),*(m_path.at(i)->at(j)->m_pts[k-1]));
				str.Format(_T("G1 X%.3f Y%.3f E%.5f"),m_path.at(i)->at(j)->m_pts[k]->x,m_path.at(i)->at(j)->m_pts[k]->y,Len*Eratio);
				m_GcodesArray.Add(str);
			}

			m_GcodesArray.Add(str);

			str = _T("G1 E-1.00000 F1800");
			m_GcodesArray.Add(str);
		}
 		
	}
}

void CLayer::GntCommonLayerGcodes( double Eratio)
{
	m_GcodesArray.RemoveAll();
	CString str;
	double Len;
	str.Format(_T("G1 Z%.3f F6000.000"),m_z);
	m_GcodesArray.Add(str);

	for (int i=0; i<m_path.size();i++)
	{
		for (int j=0; j<m_path[i]->size();j++)
		{
			str.Format(_T("G1 X%.3f Y%.3f F6000.000"),m_path.at(i)->at(j)->m_pts[0]->x,m_path.at(i)->at(j)->m_pts[0]->y);
			m_GcodesArray.Add(str);
			str = _T("G1 E1.00000 F1800.00");
			m_GcodesArray.Add(str);
			Len = GetDistance(*(m_path.at(i)->at(j)->m_pts[0]),*(m_path.at(i)->at(j)->m_pts[1]));
			str.Format(_T("G1 X%.3f Y%.3f E%.5f F600.000"),m_path.at(i)->at(j)->m_pts[1]->x,m_path.at(i)->at(j)->m_pts[1]->y,Len*Eratio);
			m_GcodesArray.Add(str);
			for (int k=2; k<m_path.at(i)->at(j)->m_pts.GetSize();k++)
			{
				Len = GetDistance(*(m_path.at(i)->at(j)->m_pts[k]),*(m_path.at(i)->at(j)->m_pts[k-1]));
				str.Format(_T("G1 X%.3f Y%.3f E%.5f"),m_path.at(i)->at(j)->m_pts[k]->x,m_path.at(i)->at(j)->m_pts[k]->y,Len*Eratio);
				m_GcodesArray.Add(str);
			}
			m_GcodesArray.Add(str);
			str = _T("G1 E-1.00000 F1800");
			m_GcodesArray.Add(str);
		}
	}
}

void CLayer::TransfertoSpline()
{
	CBSpline* pSline;
	for (int i=0; i<m_InPolylines.size();i++)
	{
		pSline = new CBSpline();
		CPolyline* polyline = (CPolyline *)m_InPolylines.at(i);

		if ((polyline->m_pts[0]->x !=polyline->m_pts[polyline->m_pts.GetSize()-1]->x)&&
			(polyline->m_pts[0]->y !=polyline->m_pts[polyline->m_pts.GetSize()-1]->y))
		{
			for (int j=0; j<polyline->m_pts.GetSize();j++)
			{
				CPoint3D *pt = polyline->m_pts[j];
				pSline->AddFitPts(polyline->m_pts[j]);
			}
		}
		else
		{
			for (int j=0; j<polyline->m_pts.GetSize();j++)
			{
				CPoint3D *pt = polyline->m_pts[j];
				pSline->AddFitPts(polyline->m_pts[j]);
			}
		}
	
		pSline->Interpolate();
		pSline->Scatter();

		pSline->SetType(TYPE_SPLINE);
		m_splines.push_back(pSline);
	}
}


void CLayer::CalculatePointsOnSplines( double distance,double rate_distance )
{
	CBSpline* pSpline;
	PointsOnSpline* pPointsOnSpline;
	PointsOnSpline* pPointsOnSplineEx;
	for (int i=0; i<m_splines.size();i++)
	{
		pPointsOnSpline = new PointsOnSpline;
		pPointsOnSplineEx = new PointsOnSpline;
		pSpline = (CBSpline*)m_splines.at(i);
		pSpline->UnitizeKnot();
		double Length = pSpline->GetLength();
		int nNumber = Length/distance;
		double LengthEach = Length/nNumber;
		for (int i=0; i<=nNumber; i++)
		{
			double L = 0;
			double u=0;
			u = (double)i/(double)nNumber;
			if (u>=1)
			{
				u=0.9999;
			}
			PointEx * pt;
			pt = new PointEx;
			pt->u = u;
			CVector3D v1 = pSpline->dP(u);
			double angle1 = atan2(v1.dy, v1.dx);
			CVector3D v2;
			v2.dx = -v1.dy;
			v2.dy = v1.dx;
			v2.dz = v1.dz;
			v2.Normalize();
			double angle2 = atan2(v2.dy, v2.dx);
			pt->vec = v2;
			pt->pt = pSpline->P(u);
			pPointsOnSpline->push_back(pt);

			PointEx* pt1;
			pt1 = new PointEx;
			pt1->pt = pt->pt+v2*rate_distance;
			pPointsOnSplineEx->push_back(pt1);
		}
		//m_splines.push_back(pSpline);
		m_PointsOnOriginalSplines.push_back(pPointsOnSpline);
		m_ExteriorOriginalPointsEx.push_back(pPointsOnSplineEx);

	}
}

void CLayer::GetPointsforanotherSpline( double height,double rate_distance )
{
	CBSpline* pSpline;
	PointsOnSpline* pPointsOnSpline;
	PointsOnSpline* pPointsOnSplineEx;
	for (int i=0; i<m_splines.size();i++)
	{
		PointEx* pt1;
		pPointsOnSpline = new PointsOnSpline;
		pPointsOnSplineEx = new PointsOnSpline;
		pSpline = (CBSpline*)m_splines.at(i);
		pSpline->UnitizeKnot();
		PointEx* pt;
		pt=new PointEx;
		pt->u = m_PointsOnOriginalSplines.at(i)->at(0)->u;
		pt->vec = m_PointsOnOriginalSplines.at(i)->at(0)->vec;
		pt->vec.Normalize();
		double N = height/(pt->vec.GetLength());
		pt->pt = pSpline->P(pt->u)+(pt->vec)*N;
		pPointsOnSpline->push_back(pt);

		for (int j=1; j<m_PointsOnOriginalSplines.at(i)->size();j++)
		{
			pt=new PointEx;
			CVector3D vec;
			double u =  (m_PointsOnOriginalSplines.at(i)->at(j-1)->u+m_PointsOnOriginalSplines.at(i)->at(j)->u)/2;
			pt->u = u;
			vec =m_PointsOnOriginalSplines.at(i)->at(j-1)->vec+m_PointsOnOriginalSplines.at(i)->at(j)->vec;
			vec.Normalize();
			pt->vec = vec;
			N = height/vec.GetLength();
			pt->pt = pSpline->P(u)+vec*N;
			pPointsOnSpline->push_back(pt);

			pt1 = new PointEx;
			pt1->pt = pt->pt-vec*rate_distance ;
			pPointsOnSplineEx->push_back(pt1);
		}

		int sz = m_PointsOnOriginalSplines.at(i)->size();
		pt=new PointEx;
		pt->u =m_PointsOnOriginalSplines.at(i)->at(sz-1)->u;
		pt->vec =m_PointsOnOriginalSplines.at(i)->at(sz-1)->vec;
		pt->vec.Normalize();
		N = height/(pt->vec.GetLength());
		pt->pt = pSpline->P(pt->u)+(pt->vec)*N;
		pPointsOnSpline->push_back(pt);

		m_PointsOnOffsetSplines.push_back(pPointsOnSpline);
		m_ExteriorOffsetPointsEx.push_back(pPointsOnSplineEx);
	}

	for (int i=0;i<m_splines.size();i++)
	{
		CBSpline *TempSpline;
		TempSpline = new CBSpline();
		for (int j=0;j<m_PointsOnOffsetSplines.at(i)->size();j++)
		{
			TempSpline->AddFitPts(&m_PointsOnOffsetSplines.at(i)->at(j)->pt);
		}
		TempSpline->Interpolate();
		TempSpline->Scatter();
		TempSpline->UnitizeKnot();
		m_ExteriorOffsetSpline.push_back(TempSpline);
	}
}

void CLayer::GenerateWavySpline()
{
	CBSpline* pWavy;
	for (int i=0; i<m_splines.size();i++)
	{
		pWavy = new CBSpline();
		CPoint3D *pt;
		for (int j=0; j<m_ExteriorOriginalPointsEx.at(i)->size()-1; j++)
		{
			pt = new CPoint3D(m_ExteriorOriginalPointsEx.at(i)->at(j)->pt);
			pWavy->AddFitPts(pt);
			pt = new CPoint3D(m_ExteriorOffsetPointsEx.at(i)->at(j)->pt);
			pWavy->AddFitPts(pt);
		}
		int sz = m_ExteriorOriginalPointsEx.at(i)->size()-1;
		pt = new CPoint3D(m_ExteriorOriginalPointsEx.at(i)->at(sz)->pt);
		pWavy->AddFitPts(pt);
		pWavy->Interpolate();
		pWavy->Scatter();
		m_pWavies.push_back(pWavy);
	}
}

void CLayer::ClearBuffer()
{
	if (!m_PointsOnOriginalSplines.empty())
	{
		for (int i=0; i<m_PointsOnOriginalSplines.size();i++)
		{
			if (!m_PointsOnOriginalSplines[i]->empty())
			{
				for(vector<PointEx*>::iterator it = m_PointsOnOriginalSplines[i]->begin(); it != m_PointsOnOriginalSplines[i]->end();)
				{
					it = m_PointsOnOriginalSplines[i]->erase( it ); 
				}
			}
		}

	}

	if (!m_PointsOnOriginalSplines.empty())
	{
		for (int i=0; i<m_PointsOnOffsetSplines.size();i++)
		{
			if (!m_PointsOnOffsetSplines[i]->empty())
			{
				for(vector<PointEx*>::iterator it = m_PointsOnOffsetSplines[i]->begin(); it != m_PointsOnOffsetSplines[i]->end();)
				{
					it = m_PointsOnOffsetSplines[i]->erase( it ); 
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



void CLayer::GntFirstLayerPath( double width, int nLoop,double FirstLayerHeight )
{
	double ERatio;
	ERatio = 1.2*(width*FirstLayerHeight)*4/PI/FILIMENT_DIAMETER/FILIMENT_DIAMETER;
	GntEffctPath();
	GntBrimPath(width,nLoop);
	GntFirstLayerGcodes(ERatio);
}

void CLayer::GntCommonLayerPath( double width, double layerHight )
{
	double ERatio;
	ERatio = 1.2*(width*layerHight)*4/PI/FILIMENT_DIAMETER/FILIMENT_DIAMETER;
	GntEffctPath();
	GntCommonLayerGcodes(ERatio);
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

