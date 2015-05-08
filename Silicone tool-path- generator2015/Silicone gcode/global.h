// History
// 
// 2013-10-23
// 修复 GntSweeplines 函数里yMin寻找的bug

#include <vector>
#include <algorithm>
#include "Polyline.h"
#include "LVertex.h"

using namespace std;


// 注：在h文件中 extern 声名，在cpp文件中定义。在使用的地方包含h文件即可

// Support
extern double g_baseLevel;
extern double g_supportSectionInterval;
extern double g_sawTriangleLength;
extern double g_sawTriangleInterval;

// Slicing
extern double g_maxLayerHeight;
extern double g_minLayerHeight;
extern double g_stairError;

// Path generation
// 和工艺相关的重参数
extern double g_contourSpotDia;
extern double g_scanlineSpotDia;
extern double g_scanlineInterval;
extern double g_feedrate;       // 固化加工时扫描速度
extern double g_feedrate_air;   // 空走速度
extern double g_biggestSpotDia; // 最大光斑
extern double g_smallestSpotDia;// 最小光斑 0.1mm
extern double g_overlapRatio;   // 两条相邻扫描路径重叠率，适用平行扫描线 in range of (0,1)


bool IsTriInTris(LTriangle* pTri, vector<LTriangle*>& tris, bool bDel =true);


void LinesToPolylines(vector<CLine>&		lines, 
					  vector<CPolyline*>&	boundaries,
					  bool					bPlanar = false);   // 指针有在该函数里有new出来，需要在外部函数管理删除

CPolyline* LinesToPolyline(vector<CLine>& lines);


void AFX_API_EXPORT
GntSweeplines( 
			  vector<CPolyline*>& boundaries,    // 输入边界
			  double lineInterval,               // 线间距
			  double angle,                      // 倾斜角
			  vector<vector<CLine>>& lineses,	  // 输出扫描线
			  bool bBoundaryRotateBack = false); // angle不为0时是否要对输入boundary旋转回来


bool CalcTwoTrisInst(const CPoint3D& A1, const CPoint3D& B1, const CPoint3D& C1,  // points of the 1st triangle
					 const CPoint3D& A2, const CPoint3D& B2, const CPoint3D& C2,  // points of the 2nd triangle
					 CLine& line);// output line segment if exists

bool CalcTriXYPlaneInst(const CPoint3D& A, 
						const CPoint3D& B, 
						const CPoint3D& C, 
						double z, 
						CLine& line_out);

double CalcTriangleArea(const CPoint3D& A, const CPoint3D& B, const CPoint3D& C);


template<class T>
void DeletePointerArray(vector<T*>& Ts)
{
	int sz = Ts.size();
	for (int i=0;i<sz;i++)
	{
		delete Ts[i];
		Ts[i] = NULL;
	}
	Ts.clear();
}