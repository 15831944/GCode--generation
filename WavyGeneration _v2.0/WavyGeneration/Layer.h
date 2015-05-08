
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
using namespace std;

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

	void GntEffctPath();// First layer has some path, other layers just have one path

	void SetLayerZValue(double z); //

	void Clear();

	void GntFirstLayerPath(double width, int nLoop,double FirstLayerHeight);
	void GntCommonLayerPath(double width, double layerHight);

	//Eration pi*r*r*Eratio = height*width
	void GntFirstLayerGcodes(double Eratio);
	void GntCommonLayerGcodes(double Eratio);



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
	/*********trasfer polyline to splines**************/
	void TransfertoSpline();
    /**************get points on original splines based on distance and overlapping rate*********************/
	void  CalculatePointsOnSplines(double distance,double rate_distance);//original splines
	/**************get points on another spline*****************/
    void GetPointsforanotherSpline(double height,double rate_distance);//get points on the offset curves
    /**********generate wavy based on points***********/
	void GenerateWavySpline();

	/*******move polyline**************/
	void MovePolyline(double x, double y);

private:

	double   m_z;
	int      m_Count;//number of layer from 0....
	double   m_feedrate;
	double   m_extrusion_per_mm;

private:
	PolyLineGroup m_InPolylines;// polylines read in
	Splines       m_splines;//tanslate polylines to splines
	PointsOnSplines m_PointsOnOriginalSplines;//
	PointsOnSplines m_PointsOnOffsetSplines;
	//Splines         m_ExteriorOriginalSpline;//original splines 
	Splines         m_ExteriorOffsetSpline;//another splines
	Splines         m_pWavies;//wavy structure for each spline

	PointsOnSplines m_ExteriorOriginalPointsEx;//These points is on for wavy structure generation 
	PointsOnSplines m_ExteriorOffsetPointsEx;//
};
