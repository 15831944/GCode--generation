#include "StdAfx.h"
#include "AzClipper.h"

AzClipper::AzClipper(void)
{
}

AzClipper::~AzClipper(void)
{
}

bool AzClipper::AddPolygon(CPolyline* pPoly, ClipperLib::PolyType polyType)
{
	int sz=pPoly->m_pts.GetSize();
	ClipperLib::Polygon poly;
	for (int i=0;i<sz-1;i++)
	{
		ClipperLib::long64 x = (ClipperLib::long64) (pPoly->m_pts[i]->x * D2I_TIMES);
		ClipperLib::long64 y = (ClipperLib::long64) (pPoly->m_pts[i]->y * D2I_TIMES);
		ClipperLib::IntPoint pt(x,y);
		m_z = pPoly->m_pts[i]->z;
		poly.push_back(pt);
	}

	ClipperLib::Polygon poly1;
	ClipperLib::CleanPolygon(poly,poly1);

	return ClipperLib::Clipper::AddPolygon(poly1,polyType);
}

bool AzClipper::AddPolygons(Polylines& polygons, ClipperLib::PolyType polyType)
{
	int sz = polygons.size();
	for (int i=0;i<sz;i++)
	{
		if (false == AddPolygon(polygons[i],polyType))
		{
			return false;
		}
	}
	return true;
}


bool AzClipper::Execute(ClipperLib::ClipType clipType, 
						std::vector<CPolyline*> &solution, 
						ClipperLib::PolyFillType subjFillType /* = pftEvenOdd */, 
						ClipperLib::PolyFillType clipFillType /* = pftEvenOdd */,
						bool isSimply /*= true*/)
{
	ClipperLib::Polygons polys0;
	
	bool b = ClipperLib::Clipper::Execute(clipType,polys0,subjFillType,clipFillType);
	if (b)
	{			
		ClipperLib::Polygons polys;
		int sz0 = polys0.size();
		for (int i=0;i<sz0;i++)
		{
			ClipperLib::Polygon out_poly;
			ClipperLib::CleanPolygon(polys0[i],out_poly);
			if (out_poly.size() > 2)
			{
				polys.push_back(out_poly);
			}
		}

		int sz = polys.size();
		for (int i=0;i<sz;i++)
		{
			double a = fabs(ClipperLib::Area(polys[i]));
			if (isSimply && a < 0.1 * D2I_TIMES * D2I_TIMES)
			{
				continue;
			}

			CPolyline* pPolyline = new CPolyline();
			int szPt = polys[i].size();
			for (int j=0;j<szPt;j++)
			{
				double x = (double)polys[i][j].X * I2D_TIMES;
				double y = (double)polys[i][j].Y * I2D_TIMES;
				CPoint3D *pPt = new CPoint3D(x, y, m_z);
				pPolyline->m_pts.Add(pPt);
			}
			pPolyline->AddPt(pPolyline->GetFirstPt());

// 			pPolyline->DelCoincidePt(); 
// 			pPolyline->DelColinearPt();
// 			if (pPolyline->m_pts.GetSize()<4)
// 			{
// 				delete pPolyline;
// 				continue;
// 			} 
// 2013-11-25 调试时注释，全部用 ClipperLib CleanPolygon 函数
			solution.push_back(pPolyline);
		}
	}
	return b;
}

void AzClipper::Offset(CPolyline* in_poly, Polylines& out_polys, double dis, ClipperLib::JoinType jointype /* = jtSquare */, double limit /* = 0 */, bool autoFix /* = true */)
{
	double z = in_poly->m_pts[0]->z;

	ClipperLib::Polygon in_polygon;
	MyPoly2HisPoly(in_poly,in_polygon);
	ClipperLib::Polygons in_polygons;
	in_polygons.push_back(in_polygon);

	ClipperLib::Polygons out_polygons;
	ClipperLib::OffsetPolygons(in_polygons,out_polygons,dis*D2I_TIMES,jointype,limit,autoFix);

	int sz = out_polygons.size();
	for (int i=0;i<sz;i++)
	{
		CPolyline* pPolyline = new CPolyline();
		HisPoly2MyPoly(out_polygons[i],pPolyline,z);
		out_polys.push_back(pPolyline);
	}
}

void AzClipper::Offset(Polylines& in_polys, Polylines& out_polys, double dis, ClipperLib::JoinType jointype /* = jtSquare */, double limit /* = 0 */, bool autoFix /* = true */)
{
	int sz = in_polys.size();
	if (0==sz)
		return;

	double z = in_polys[0]->m_pts[0]->z;

	ClipperLib::Polygons in_polygons;
	for (int i=0;i<sz;i++)
	{
		ClipperLib::Polygon in_polygon;
		MyPoly2HisPoly(in_polys[i],in_polygon);
		in_polygons.push_back(in_polygon);
	}

	ClipperLib::Polygons out_polygons;
	ClipperLib::OffsetPolygons(in_polygons,out_polygons,dis*D2I_TIMES,jointype,limit,autoFix);

	sz = out_polygons.size();
	for (int i=0;i<sz;i++)
	{
		CPolyline* pPolyline = new CPolyline();
		HisPoly2MyPoly(out_polygons[i],pPolyline,z);
		out_polys.push_back(pPolyline);
	}
}

void AzClipper::MyPoly2HisPoly(CPolyline* myPoly, ClipperLib::Polygon& hisPoly)
{
	int sz=myPoly->m_pts.GetSize();
	ClipperLib::Polygon poly;
	for (int i=0;i<sz-1;i++)
	{
		ClipperLib::long64 x = (ClipperLib::long64) (myPoly->m_pts[i]->x * D2I_TIMES);
		ClipperLib::long64 y = (ClipperLib::long64) (myPoly->m_pts[i]->y * D2I_TIMES);
		ClipperLib::IntPoint pt(x,y);
		poly.push_back(pt);
	}
	CleanPolygon(poly,hisPoly);
}

void AzClipper::HisPoly2MyPoly(ClipperLib::Polygon& hisPoly, CPolyline* myPoly, double z /* = 0 */)
{
	int sz = hisPoly.size();
	for (int i=0;i<sz;i++)
	{
		double x = (double)hisPoly[i].X * I2D_TIMES;
		double y = (double)hisPoly[i].Y * I2D_TIMES;
		CPoint3D* pPt = new CPoint3D(x, y, z);
		myPoly->m_pts.Add(pPt);
	}
	myPoly->m_pts.Add(new CPoint3D(*myPoly->m_pts[0]));
	myPoly->DelCoincidePt();
	myPoly->DelColinearPt();
}