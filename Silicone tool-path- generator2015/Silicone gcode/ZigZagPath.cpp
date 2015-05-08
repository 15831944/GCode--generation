#include "StdAfx.h"
#include "ZigZagPath.h"

CZigZagPath::CZigZagPath(void)
{
}

CZigZagPath::~CZigZagPath(void)
{
}

void CZigZagPath::Draw( COpenGLDC* pDC, COLORREF clr )
{
   int szPaths = m_FinalZiazagpaths.size();
   if (szPaths<=0)
   {
	   return;
   }

   COLORREF oldclr= pDC->SetMaterialColor(clr);
   for (int i = 0 ; i<szPaths -1; i++)
   {
	   int sz_tmpPoints = m_FinalZiazagpaths[i].size();
	   for (int j = 0;j<sz_tmpPoints -1; j++)
	   {
		   CPoint3D pts(m_FinalZiazagpaths[i][j]->pt);
		   CPoint3D pte(m_FinalZiazagpaths[i][j+1]->pt);
		   pDC->DrawLine(pts, pte,LS_SOLID,2,RGB(255,255,255));
	   }

// 	   CPoint3D pts(m_FinalZiazagpaths[i][sz_tmpPoints-1]->pt);
// 	   CPoint3D pte(m_FinalZiazagpaths[i+1][0]->pt);
// 
// 	   pDC->DrawLine(pts, pte, LS_SOLID,2,RGB(255,255,0));
   }

   int sz_tmpPoints = m_FinalZiazagpaths[szPaths-1].size();
   for (int j = 0;j<sz_tmpPoints -1; j++)
   {
	   CPoint3D pts(m_FinalZiazagpaths[szPaths-1][j]->pt);
	   CPoint3D pte(m_FinalZiazagpaths[szPaths-1][j+1]->pt);
	   pDC->DrawLine(pts, pte,LS_SOLID,2,RGB(255,255,255));
   }

   int sz_hermite = m_Hermite.size();
   for (int m = 0; m<sz_hermite; m++)
   {
	   m_Hermite[m]->Draw(pDC, RGB(0,0,0));
   }

   pDC->SetMaterialColor(oldclr);

}

void CZigZagPath::push_up_Point( ZigzagPoint* _zigzagpoint)
{
	m_tmpZiazag.push_back(_zigzagpoint);
}


void CZigZagPath::push_up_Path( ZigzagPath _zigzagpath )
{
    m_Ziazagpaths.push_back(_zigzagpath);
}


void CZigZagPath::clear_tmppath()
{
	int szPoints = m_tmpZiazag.size();
	for (int i = 0; i<szPoints ;i++)
	{
		delete m_tmpZiazag[i];
		m_tmpZiazag[i] = NULL;
	}
	m_tmpZiazag.clear();
}

void CZigZagPath::releaseMem()
{
	int szPaths = m_Ziazagpaths.size();
	for (int i = 0; i<szPaths ;i++)
	{
		int szPoints = m_Ziazagpaths[i].size();
		for (int j = 0; j<szPoints ;j++)
		{
			delete m_Ziazagpaths[i][j];
			m_Ziazagpaths[i][j] = NULL;
		}
		 m_Ziazagpaths[i].clear();
	}
	m_Ziazagpaths.clear();

	int szFinalPaths = m_FinalZiazagpaths.size();
	for (int k =0 ; k<szFinalPaths; k++)
	{
		int szPoints = m_FinalZiazagpaths[k].size();
		for (int m = 0; m < szPoints; m++)
// 		{
// 			delete m_FinalZiazagpaths[k][m];
// 			m_FinalZiazagpaths[k][m] = NULL;
// 		}
		m_FinalZiazagpaths[k].clear();
	}
	m_FinalZiazagpaths.clear();

	int szHermite = m_Hermite.size();
	for (int m = 0; m<szHermite; m++)
	{
		delete m_Hermite[m];
		m_Hermite[m] = NULL;
	}
	m_Hermite.clear();
}

 void CZigZagPath::GntZigzagFRomlines(ZigzagPath& n_zigzagPath, vector<CLine> n_lines,int n_type )
{
	for (int i = 0; i<n_zigzagPath.size(); i++)
	{
         delete n_zigzagPath[i];
		 n_zigzagPath[i] = NULL;
	}
    n_zigzagPath.clear();

	int szLines = n_lines.size();//条数
	ZigzagPoint* p = NULL;
	switch (n_type)
	{
	case PSSTART: 	    
		for (int i=0;i<szLines;i++)
		{
			if (i%2 == 0)//从头开始数数，奇数条，从左到右
			{
				p = new ZigzagPoint(n_lines[i].m_pt1);	
				p->b_isIn = true;
				n_zigzagPath.push_back(p);
				p = new ZigzagPoint(n_lines[i].m_pt2);
				p->b_isIn = false;
				n_zigzagPath.push_back(p);
			}
			else        //偶数条，从右到左
			{
				p = new ZigzagPoint(n_lines[i].m_pt2);	
				n_zigzagPath.push_back(p);
				p->b_isIn = true;
				p = new ZigzagPoint(n_lines[i].m_pt1);
				p->b_isIn = false;
				n_zigzagPath.push_back(p);
			}
		}
		 break;
	case PSEND:  
		for (int i=0;i<szLines;i++)
		{
			if (i%2 == 1)//从头开始数数，偶数条，从左到右
			{
				p = new ZigzagPoint(n_lines[i].m_pt1);	
				p->b_isIn = true;
				n_zigzagPath.push_back(p);
				p = new ZigzagPoint(n_lines[i].m_pt2);
				p->b_isIn = false;
				n_zigzagPath.push_back(p);
			}
			else        //奇数条，从右到左
			{
				p = new ZigzagPoint(n_lines[i].m_pt2);	
				p->b_isIn = true;
				n_zigzagPath.push_back(p);
				p = new ZigzagPoint(n_lines[i].m_pt1);
				p->b_isIn = false;
				n_zigzagPath.push_back(p);
			}
		}
		 break;
	case PESTART:
		for (int i=szLines-1; i>=0; i--)
		{
			if ((szLines-1-i)%2 == 0)//从尾开始数数，奇数条，从左到右
			{
				p = new ZigzagPoint(n_lines[i].m_pt1);	
				n_zigzagPath.push_back(p);
				p->b_isIn = true;
				p = new ZigzagPoint(n_lines[i].m_pt2);
				p->b_isIn = false;
				n_zigzagPath.push_back(p);
			}
			else        //偶数条，从右到左
			{
				p = new ZigzagPoint(n_lines[i].m_pt2);	
				n_zigzagPath.push_back(p);
				p->b_isIn = true;
				p = new ZigzagPoint(n_lines[i].m_pt1);
				n_zigzagPath.push_back(p);
				p->b_isIn =false;
			}
		}
		break;
	case PEEND: 
		for (int i=szLines-1; i>=0; i--)
		{
			if ((szLines-1-i)%2 == 1)//从尾开始数数，偶数条，从左到右
			{
				p = new ZigzagPoint(n_lines[i].m_pt1);	
				p->b_isIn =true;
				n_zigzagPath.push_back(p);
				p = new ZigzagPoint(n_lines[i].m_pt2);
				p->b_isIn =false;
				n_zigzagPath.push_back(p);
			}
			else        //奇数条，从右到左
			{
				p = new ZigzagPoint(n_lines[i].m_pt2);
				p->b_isIn = true;
				n_zigzagPath.push_back(p);
				p = new ZigzagPoint(n_lines[i].m_pt1);
				p->b_isIn =false;
				n_zigzagPath.push_back(p);
			}
		}
		break;
	}
}

 CPoint3D CZigZagPath::GetZigzagEndPoint( vector<CLine> n_lines, int n_type )
 {
	 ZigzagPoint n_zigzagPoint;
	 int sz = n_lines.size();
     switch (n_type)
     {
     case PSSTART:
		 if (sz%2 == 0)	 n_zigzagPoint = n_lines[sz -1].m_pt1;
		 else            n_zigzagPoint = n_lines[sz-1].m_pt2;   		
    	break;
	 case PSEND:
		 if (sz%2 == 0)	 n_zigzagPoint = n_lines[sz -1].m_pt2;
		 else            n_zigzagPoint = n_lines[sz-1].m_pt1;   		
		 break;
	 case PESTART:
		 if (sz%2 == 0)	 n_zigzagPoint = n_lines[0].m_pt1;
		 else            n_zigzagPoint = n_lines[0].m_pt2;   		
		 break;
	 case PEEND:
		 if (sz%2 == 0)	 n_zigzagPoint = n_lines[0].m_pt2;
		 else            n_zigzagPoint = n_lines[0].m_pt1;   		
		 break;
     }

	 return n_zigzagPoint.pt;
 }


 int CZigZagPath::FindShortestLen(CPoint3D pt,vector<CLine> n_lines, double &len)
 {
	 double dis1, dis2, dis3, dis4;
	 CPoint3D _pt;
	  
	 _pt = n_lines[0].m_pt1;
	 dis1 = GetDistance(pt, _pt);

	 _pt = n_lines[0].m_pt2;
	 dis2 = GetDistance(pt, _pt);

	 _pt = n_lines[n_lines.size()-1].m_pt1;
	 dis3 = GetDistance(pt, _pt);

	 _pt = n_lines[n_lines.size()-1].m_pt2;
	 dis4 = GetDistance(pt, _pt);

	 if ((dis1<=dis2)&&(dis1<=dis3)&&(dis1<=dis4))
	 {
        len = dis1;
		return PSSTART;
	 }

	 if ((dis2<=dis1)&&(dis2<=dis3)&&(dis2<=dis4))
	 {
		 len = dis1;
		 return PSEND;
	 }


	 if ((dis3<=dis1)&&(dis3<=dis2)&&(dis3<=dis4))
	 {
		 len = dis3;
		 return PESTART;
	 }

	 if ((dis4<=dis1)&&(dis4<=dis2)&&(dis4<=dis3))
	 {
		 len = dis4;
		 return PEEND;
	 } 
	 else
		 return 4;
 }

 void CZigZagPath::FineProcessingPath( double n_width )
 {
	 int sz = m_Ziazagpaths.size();

	 //先把第一条放进去
     m_FinalZiazagpaths.push_back(m_Ziazagpaths[0]);

	 for (int i = 1; i<sz; i++)
	 {
		
		 int szFinal = m_FinalZiazagpaths.size();
		 int szLines = m_FinalZiazagpaths[szFinal-1].size();
		 ZigzagPoint beforepts(*m_FinalZiazagpaths[szFinal-1][szLines-2]);
		 ZigzagPoint pts(*m_FinalZiazagpaths[szFinal-1][szLines-1]);
	     ZigzagPoint pte(*m_Ziazagpaths[i][0]);
		 ZigzagPoint afterpte(*m_Ziazagpaths[i][1]);

		 /***********************
		 这里的判断条件有两个，第一个是上下相邻，
		 //另外一个就是这两个点在连接起来之后，不会与他们成为一个z形状,这点不能解决所有问题的
         另外一个条件就是这两个点是因为与同一个轮廓的交点
		 **************************/

	     CPoint3D pt1, pt2;
		 pt1 = pts.pt;
		 pt2 = pte.pt;
		 double L = GetDistance(pt1,pt2);
		 if (((fabs(pte.pt.x - pts.pt.x) - n_width) < 0.000001)&&((pts.pt.y-beforepts.pt.y)*(pte.pt.y-afterpte.pt.y)>0)&&(L < 2*n_width))
		 {
			 int szpoints = m_Ziazagpaths[i].size();
			 for (int j=0; j<szpoints; j++)
			 {
				 m_FinalZiazagpaths[szFinal-1].push_back(m_Ziazagpaths[i][j]);
			 }
		 }
		 else
		 {
			 m_FinalZiazagpaths.push_back(m_Ziazagpaths[i]);
		 }
	 }
 }

 void CZigZagPath::HermiteLinking(double n_width)
 {
     CHermite* p_Hermite;
	 int sz = m_FinalZiazagpaths.size();
	 for (int i = 1; i<sz; i++)
	 {	
		 int szLines = m_FinalZiazagpaths[i-1].size();
		 ZigzagPoint beforepts(*m_FinalZiazagpaths[i-1][szLines-2]);
		 ZigzagPoint pts(*m_FinalZiazagpaths[i-1][szLines-1]);
	     ZigzagPoint pte(*m_FinalZiazagpaths[i][0]);
		 ZigzagPoint Afterpte(*m_FinalZiazagpaths[i][1]);

		 CPoint3D StartPt(pts.pt);
		 CPoint3D EndPt(pte.pt);
		 CPoint3D BeforePt(beforepts.pt);
		 CPoint3D AfterPt(Afterpte.pt);

		 p_Hermite = new CHermite(BeforePt, StartPt, EndPt, AfterPt);
		 p_Hermite->Interpolate();
		 m_Hermite.push_back(p_Hermite);
	 }

 }




