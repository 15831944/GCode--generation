#pragma once
#include "afx.h"
#include "Entity.h"
//#include "TriangleSet.h"


class AFX_CLASS_EXPORT CPointSet : public CEntity
{
public:
	CPointSet(void);
	~CPointSet(void);

// Overridable
public:   
	virtual void Draw(COpenGLDC* pDC,COLORREF clr);

	virtual int GetSize();
// Operations
public:

	void AddPt(CPoint3D* pPt);

	void AddPt(const CPoint3D& pt);


	// Write the original profile to a TXT text file. The format is : i=..., X=..., Y=..., Z=... //
	void Write2TXT(CStdioFile& file);

	void Write2TXT(CString sFilePath);

	// Read TXT file //
	void ReadTXT(CStdioFile& file);

	// For planar points, calculate or remove the convex hull //
	// For derived classes, override this virtual function    // 
    virtual void GetConvexHull(Point3DAry& convexHull) const;

	virtual COBBox BoundingBox();

	// Get the min enclosing circle
	// O(n)
	void GetMEC(CPoint3D& ptCenter_out, double& radius_out, BOOL bRandomize=TRUE);

    static CPoint3D GetCenter(const CPoint3D &ptA, 
							  const CPoint3D &ptB, 
							  const CPoint3D &ptC);

	
	// Quick sort left to right //
	static void QSortL2R(Point3DAry& pts, int s, int e);

	// Define point position relationships //
	static BOOL IsP1LeftOfP2(const CPoint3D& P1, const CPoint3D& P2);
	static BOOL IsP1NotLeftOfP2(const CPoint3D& P1, const CPoint3D& P2);

	void RemoveSamePts(Point3DAry& pts, BOOL HasBeenSorted=TRUE) const;

	// Find Extrem Point: the -x, +x, -y, +y, -z and +z of a point cloud //
	int FindExtremPt(int nWhichDir);

	double FindExtreme(int nWhichDir) const;

	void GntRandomPts(int nHowMany);

	// Delaunay triangulation. Parameter [bRandomize] means that should the 
	// point array in m_ptList randomized.
	//void Triangulate(CTriangleSet* pTriSetOut, BOOL bRandomize=TRUE);

// Protected functions of Triangualte()
protected:
	//void GntSuperTriangle(PTriangle* pRoot);

	//void Split_1_to_3(CPoint3D* pAddedPt, PTriangle* pTri, PTRILIST& triList);
	//void Split_2_to_4(CPoint3D* pAddedPt, PEdge* pEdge, PTRILIST& triList);

	//void LegalizeEdge(CPoint3D* pPt, PEdge* pEdge, PTRILIST& triList);

	//BOOL IsEdgeLegal(CPoint3D* pPt, PEdge* pEdge);

	//int IsPtInTriangle(CPoint3D* pPt, PTriangle* pTri, PEdge*& edgeAddress_out);

	// Find in which triangle is the new added point in, the return value is the found triangle //
	//int FindTriangle(CPoint3D* pPt, PTriangle* pRoot, PTriangle*& triAddress_out, PEdge*& edgeAddress_out);

	//void RemoveBadTriangles(PTRILIST& triList, CTriangleSet* pTriSetOut);

	// Make the point array in m_ptList random
	void Randomize();    // 

	
public:
	Point3DAry m_pts;   // Basic data of the point cloud
	//PTRILIST m_triList;

	enum{MIN_X,MAX_X,MIN_Y,MAX_Y,MIN_Z,MAX_Z};
};
