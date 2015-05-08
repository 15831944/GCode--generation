/* Class CPolyline : public CPointSet.
   The CPolyline class is defined here. 
   Bear in mind that the 0th point and the last point of a polygon is the same.
*/

// History:
// 2011/4/26: 修改了 GetNormalVector() 函数，去掉了normalize() 函数减小计算量。
// ----------------------------------------------------------------------------------------
// 2011/4/27: 
// 增加了 CRect3D 类。写了 CalAllInsPts() 函数，在三角面片数位400时将计算时间从280降到0ms
// 修改 CalDis2PtToProfile() 加了每个点和原始轮廓每条边包络矩形的判断。时间从280-〉31ms

// 2013/9/12:
// 添加了一些函数，如Rotate，SetZLevel
#pragma once
#include "Line.h"
#include "PointSet.h"
#include <vector>
using namespace std;

class CPolylineSet;
class CPolyline;
typedef  CTypedPtrArray<CObArray, CPolyline*> PolylineAry; 

typedef  vector<CPolyline*> Polylines;

class AFX_CLASS_EXPORT CPolyline : public CPointSet  
{
public:
	CPolyline();
	virtual ~CPolyline();

	CPolyline(int n, CPoint3D pt,...);

	void Init();


	const CPolyline& operator=(const CPolyline& polyline);



	void SetIsDrawPt(bool bIsDrawPt);

	// Draw the polygon
	virtual void Draw(COpenGLDC* pDC,COLORREF clr);

	virtual void Move(const CVector3D& vec);

	// Rotate around the origin, ccw, in rad
	virtual void Rotate(double angle);

	virtual void Rotate(const CMatrix3D& m);

	void SetZLevel(double z);

	bool Sparse(double err  = 0.001 );

	void Reverse();

	CPoint3D GetFirstPt() const;

	CPoint3D GetLastPt() const;  

	bool IsCircle(CPoint3D& ptCenter, double& radius) const;   // Decide whether the points are on a circle in XY plane

	bool IsRect(CRect3D& rt, double precision=0.03) const;

	bool GetMinEnclosingRect(CRect3D& rt) const;

	// Get the length of the polyline. The length is stored in m_length
	// or can be returned in the return value of this function
	double GetLength();

	double GetArea();

	// Rearrangle the point order of the polygon to make the 0th point at the original I //
	void MakeFirstAt(int at_i);
	static void MakeFirstAt(Point3DAry& pts, int at_i);

	// Judge if a line segment intersects with the polygon 
	// Return TRUE if the line cross with the polygon, else False
	BOOL IsCrossWith(const CPoint3D& pt1, const CPoint3D& pt2);

	BOOL IsCrossWith(const CLine& line);

	// Judge if a point pt is in side the polygon // 
	bool IsPtInPolygon(const CPoint3D& pt) const;

	// Judge if a point pt is on the boundary of the polygon //
	bool IsPtOnPolygon(const CPoint3D& pt) const;

	// Judge of a polygon is counterclockwise //
	BOOL IsPolygonCCW();   

	// Make polygon counterclockwise //
	void MakePolygonCCW();

	// Make polygon clockwise //
	void MakePolygonCW();

	// Delete the neigbor points that have indentical coordinates
	void DelCoincidePt(double dist_resolution = POINT_RESOLUTION);  // Call firstly

	// Merge colinear Segments // 
	void DelColinearPt(double angle_resolution = 1.0e-5 /*rad*/);  // Call secondly

	// Full offset of a polygon, can only do inner offset //
    void FullOffset(const double& dis);

	// Single offset of a polygon. The original profile is counterclockwise //
	void Offset(const double& dis);

	// Only deal with the local offset problem, the global problem is not handled
	bool RawOffset(double dis, CPolyline *& pOffsetPolygon);

	// Export the offset polyline. 
	// [dis_in]             is the input offset distance;
	// [pPolylineSet]       is the output polyline array;  
	void Offset(double dis_in, CPolylineSet* pPolylineSet);

	// Silly single offset, only use bisector to get the very initial offset curve, for debug purpose //
	void Offset_silly(const double& dis); // version 2

	void Offset_silly(double dis, CPolyline *& pPolyline);

	// Write to DXF file //
	void Write2DXF(CStdioFile& file);

	// Link all offset curve into one //
	void LinkToolPath();

	// Insert a point that is not on the boundary of the polyline to the polyline.
	// If the min distance point of the polyline is are points, then return the point
	// index; otherwise if the min distance is on the edge, insert pt into the edge 
	// and return the corresponding point index
	int InsertPt(const CPoint3D& pt);


	void SetOPsUnused();

	// Test for uneven offset
	// The input param [dR]
	void UnevenOffset(double dR, CPolyline* & pPl);

	// 计算多边形点列的中心点，用的是包络矩形的中心，包络矩形是横平竖直的矩形 //
	void FindCenterPt();

	// Judge if a pt on a line segment //
	static BOOL IsPtOnSeg(const CPoint3D& pt, const CPoint3D& ptS_seg, const CPoint3D& ptE_seg);

	// Calculate the min distance from a point to the original profile //
	double CalcDis2PtToProfile(const CPoint3D& ptTest);

	//double GetDistToPt



protected:
	// ---------------------Inside classes--------------------- //
	class CPos;          // Position class
	class CIstPt;        // Intersection class
	class CEvt;          // Event class
	class CEvtSpace;     // Event Space class
	class CStatus;       // Status class
	class CStatusSpace;  // Status space class
	class CBisector;     // Bisector class
	typedef  CTypedPtrArray<CObArray,CIstPt*> IstPtAry;


	// Get Raw offset curve. In the raw offset curve, there's no local problem //
	BOOL RawOffset(Point3DAry &pts_raw);



	// Judge if there is stuck circle for four points //
	BOOL IsThereStuckCircle(const CPoint3D& S1, const CPoint3D& E1, const CPoint3D& S2, const CPoint3D& E2, CLine& seg_out);

	// Calculate stuck circle for four point. If there is no stuck circle, simply return FALSE //
	BOOL CalStuckCircle(const CPoint3D& S1, const CPoint3D& E1, const CPoint3D& S2, const CPoint3D& E2, CLine& seg_out);

	// Judge if the test point is on right of a polyline //
	BOOL IsPtOnRightOf3Segs(const CPoint3D& ptTest,  const CPoint3D& pt1, 
		                    const CPoint3D& pt2, const CPoint3D& pt3, const CPoint3D& pt4);

	BOOL IsPtOnRightOf4Segs(const CPoint3D& ptTest,  const CPoint3D& pt1, const CPoint3D& pt2, 
		                    const CPoint3D& pt3, const CPoint3D& pt4, const CPoint3D& pt5);

	BOOL IsPtOnRightAtCorner(const CPoint3D& ptTest, const CPoint3D& pt1, const CPoint3D& pt2, const CPoint3D& pt3);

	BOOL IsPtOnRightOfSeg(const CPoint3D& ptTest, const CPoint3D& pt1, const CPoint3D& pt2);

	// Trim Raw offset curve with segment //
	void TrimRawOffset(Point3DAry& pts);

	// Tree analysis procedre //
	void TreeAnalyse(Point3DAry& pts);

	// Bubble sort //
	void SortSelfInsts(IstPtAry& istPts);
	// Quick sort  //
	void SortSelfInsts(IstPtAry& istPts, const int& s, const int& e);

	// Remove intersecting self intersections //
	void RemoveInstSelfInsts(IstPtAry& istPts);

	void ConstructTree(IstPtAry& istPts, CIstPt* pRoot);

	// Calculte the AABB rect for the original polygon //
	CXAlignRect CalAABBRect();

	// Calculate axis-aligned rect of each edge of the original polygon & Release the rects' memory //
	void CalEveRects();
	void ReleaseEveRects();

	// Use axis-aligned rects to report self-intersection of the offset polygon //
	int CalAllIstPts(Point3DAry &pts_in, IstPtAry& istPts_out);

	// Use the well know BOA to report self-intersection of the offset polygon //
	int ReportIntersection(Point3DAry& pts, IstPtAry& istPts_out);

	// Cycle functions //
	int    O(int i, int sz);
	double O(const double& i, const int& nOffset, const int& sz);

    int FindFathers(const int& iWhich2Find, IstPtAry& istPts);

	// Functions supporting DXF writting
	void WriteHeader(CStdioFile& file);
	void WriteClasses(CStdioFile& myFile);
	void WriteTables(CStdioFile& myFile);
	void WriteBlocks(CStdioFile& myFile);
	void WriteEntities(CStdioFile& myFile, BOOL bSaveMinRectLayout=FALSE);
	void WriteObjects(CStdioFile& myFile);

	// 判断多边形上某一条线段时候和其他线段有相交，相邻的不算 //
	BOOL IsSegIstOtherSegs(const int& iWhichSeg, Point3DAry& pts);

	// Find Deepest layer, return the pointer to the deepest layer //
	CPolyline* FindDeepestLayer();

	// Find the point on the polyline which has a minimum distance //
	// to the test point, return the sequece of the point          //
	int FindClosestToPt(const CPoint3D& pt);


	// -------------------------Members------------------------ //
public:
	CPolyline*  m_pFather;         // pointer to father polyline, its direct outer polyline
	PolylineAry m_treeOPs;         // pointers to offset child polylines, in tree stucture
	PolylineAry m_sequOPs;         // Full offset list, in sequencetial struture
	int         m_level;           // The depth of the polyline in the tree structure
	CXAlignRect*    m_eveRt;           // The axis-aligned rect of each edge of the original polygon
	CPoint3D    m_ptCenter;        // The center point of the envelop rect of the original polygon
	BOOL        m_bIsClosed;       // 0 for open，1 for close，-1 for initial
	double      m_offsetDist;
	double      m_length;
	BOOL        m_bIsUsed;

private:
	bool m_bIsDrawPt;



public:
	// Test *******************************************
	// The test code can be delete after testing
	BOOL RawOffset_Demo(double offset_dis, int nStopAt, BOOL bOutputOffset);
	Point3DAry m_ostPts;
	// Test *******************************************


	// For generating Straight Skeleton algorithm of a simple polygon
};