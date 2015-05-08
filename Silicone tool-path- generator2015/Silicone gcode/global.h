// History
// 
// 2013-10-23
// �޸� GntSweeplines ������yMinѰ�ҵ�bug

#include <vector>
#include <algorithm>
#include "Polyline.h"
#include "LVertex.h"

using namespace std;


// ע����h�ļ��� extern ��������cpp�ļ��ж��塣��ʹ�õĵط�����h�ļ�����

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
// �͹�����ص��ز���
extern double g_contourSpotDia;
extern double g_scanlineSpotDia;
extern double g_scanlineInterval;
extern double g_feedrate;       // �̻��ӹ�ʱɨ���ٶ�
extern double g_feedrate_air;   // �����ٶ�
extern double g_biggestSpotDia; // �����
extern double g_smallestSpotDia;// ��С��� 0.1mm
extern double g_overlapRatio;   // ��������ɨ��·���ص��ʣ�����ƽ��ɨ���� in range of (0,1)


bool IsTriInTris(LTriangle* pTri, vector<LTriangle*>& tris, bool bDel =true);


void LinesToPolylines(vector<CLine>&		lines, 
					  vector<CPolyline*>&	boundaries,
					  bool					bPlanar = false);   // ָ�����ڸú�������new��������Ҫ���ⲿ��������ɾ��

CPolyline* LinesToPolyline(vector<CLine>& lines);


void AFX_API_EXPORT
GntSweeplines( 
			  vector<CPolyline*>& boundaries,    // ����߽�
			  double lineInterval,               // �߼��
			  double angle,                      // ��б��
			  vector<vector<CLine>>& lineses,	  // ���ɨ����
			  bool bBoundaryRotateBack = false); // angle��Ϊ0ʱ�Ƿ�Ҫ������boundary��ת����


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