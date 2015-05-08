#pragma once
#include "Polyline.h"
#include "ScanArea.h"
#include "ZigZagPath.h"
#include <vector>
using namespace std;

class CZigzagPathGen
{

public:
		typedef vector<CPolyline*> PolyLineGroup;
public:
	CZigzagPathGen(void);
	~CZigzagPathGen(void);

	// input: the innermost contours
	void GetInputs(PolyLineGroup &InPut, double width, double angle);
	void GntSweepPath();
	void OutPutPolys(PolyLineGroup& OutPolys);


public:
	

		/////generate zigzag tool path
		void GntSweeplines(vector<CPolyline*>& boundaries, 
			double lineInterval, 
			double angle, 
			vector<vector<CLine>>& lineses, 
			bool bBoundaryRotateBack  = false );

		void GntSweeplines(CPolyline* pBoundary, 
			double lineInterval, 
			double angle, 
			vector<vector<CLine>>& lineses, 
			bool bBoundaryRotateBack = false );

		void GntSweeplines(CScanArea* pScanArea,
			double lineInterval, 
			double angle, 
			vector<vector<CLine>>& lineses, 
			bool bBoundaryRotateBack = false );

public:
	vector<vector<CLine> > lineses;  //利用GntSweeplines得到的线条
	vector<vector<vector<CLine> > > m_LinesGroups;//RoughClassifyZone分组
	vector<vector<CLine> > m_InitialLineGroups;//利用FineClassifyZone分组
	vector<vector<CLine> > m_FinalLineGroups;//利用LinkZone

private:
	int  RoughClassifyZone();
	int  FineClassifyZone();//返回组数
	void LinkZone();

private:
	PolyLineGroup m_InputPolys;
	double m_width;
	double m_angle;
	CZigZagPath m_ZigzagPath;             //zigzag path

	PolyLineGroup m_OutPolys;


};
