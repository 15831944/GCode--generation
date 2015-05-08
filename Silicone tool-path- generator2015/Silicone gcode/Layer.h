
///////////////////////////////////
//This class is mainly to contain 
//information of each layer, including
//Bspline,z height, Count of layer
//maybe some other parameters for printing
//input is the z value,and polypline
//then step by step to generate
///////////////////////////////////


#pragma once
#include "afx.h"
#include "Polyline.h"
#include "BSpline.h"
#include "glContext.h"
#include "vector"
#include "ScanArea.h"
using namespace std;


#define SQUARE 0
#define CIRCLE 1

#define FILIMENT_DIAMETER 2.84

struct PointEx
{
	double u;
	CPoint3D pt;
	CVector3D vec;
};
typedef vector<PointEx*> PointsOnSpline;
typedef vector<PointsOnSpline*> PointsOnSplines;
typedef vector<CBSpline*> Splines;

class CLayer :
	public CObject
{
public:
	CLayer(void);
	~CLayer(void);

public:
	void AddPolyLine(CPolyline* poly);

	void Draw(COpenGLDC* pDC);

	//generate toolpath
public:
	void GntBrimPath(double distance,int n_laps);//This is just for the first layer

	void GntEffectPath(double linespace,double angle);
	void GntMoirePath(int nType, double angle);

	void SetLayerZValue(double z); //

	void Clear();


    //nType: 0: squre,1:circle,  
	void GntLayerPath(double linespace, double angle);

    	
	void GntLayerGcodes();

public:
	typedef vector<CPolyline*> PolyLineGroup;

	typedef vector<PolyLineGroup*> Path;

	Path m_path;//filling tool-path
	CStringArray m_GcodesArray;
private:
	CPolyline& Bspline2Polyline(CBSpline& spline);

public:
	/***********cleear all buffer*********************/
	void ClearBuffer();


    /******************divide the contour into two splines************************/
	void InitialProContours(double gap,double overlap);
    /**************get points on original splines based on distance and overlapping rate*********************/
	void  CalculatePointsOnSplines(double distance,double rate_distance);//original splines
    /**********generate wavy based on points***********/
	void GenerateWavySpline();

	/*******move polyline**************/
	void MovePolyline(double x, double y);

private:
	void Seperate2Polys(CPolyline& InPoly, PolyLineGroup& OutPoly, double thredshold_angle);
	double GetAngleBetweenLines(CPoint3D& pt1,CPoint3D& pt2,CPoint3D& pt3);



private:

	double   m_z;
	int      m_Count;//number of layer from 0....
	double   m_feedrate;
	double   m_extrusion_per_mm;

private:
	PolyLineGroup m_InPolylines;// polylines read in
	PolyLineGroup m_TestPolyLines;
    PolyLineGroup m_MediaAxises; //Medial axis


	Splines       m_splines;//transfer to splines

	PointsOnSplines m_PointsOnOutsideSplines;//
	PointsOnSplines m_PointsOnInsideSplines;
	//Splines         m_ExteriorOriginalSpline;//original splines 
	Splines         m_ExteriorOffsetSpline;//another splines
	Splines         m_pWavies;//wavy structure for each spline

	PointsOnSplines m_ExteriorOriginalPointsEx;//These points is on for wavy structure generation 
	PointsOnSplines m_ExteriorOffsetPointsEx;//
};
