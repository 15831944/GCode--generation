#pragma once
#include "clipper.hpp"
#include "Polyline.h"

#define  D2I_TIMES   1.0e6
#define  I2D_TIMES   1/D2I_TIMES


class AFX_CLASS_EXPORT AzClipper : public ClipperLib::Clipper
{
public:
	AzClipper(void);
	~AzClipper(void);

	bool AddPolygon(CPolyline* pPoly, ClipperLib::PolyType polyType);
	bool AddPolygons(Polylines& polygons, ClipperLib::PolyType polyType);

	bool AddOffsetPolygon(CPolyline* pPolyline);

	bool Execute(ClipperLib::ClipType clipType,
				  std::vector<CPolyline*> &solution,
				 ClipperLib::PolyFillType subjFillType = ClipperLib::pftNonZero,
				 ClipperLib::PolyFillType clipFillType = ClipperLib::pftNonZero,
				 bool isSimply = true);

	static void Offset(CPolyline* in_poly, 
					   Polylines& out_polys, 
					   double dis, 
					   ClipperLib::JoinType jointype = ClipperLib::jtSquare, 
					   double limit = 0, 
					   bool autoFix = true);

	static void Offset(Polylines& in_polys, 
					   Polylines& out_polys, 
					   double dis,
					   ClipperLib::JoinType jointype = ClipperLib::jtSquare, 
					   double limit = 0, 
					   bool autoFix = true);



	static void MyPoly2HisPoly(CPolyline* myPoly, ClipperLib::Polygon& hisPoly);
	static void HisPoly2MyPoly(ClipperLib::Polygon& hisPoly, CPolyline* myPoly, double z = 0);

private:
	double m_z;
};
