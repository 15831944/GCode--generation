#pragma once
#include "entity.h"
#include <vector>
#include "Line.h"
#include "Hermite.h"
using namespace std;

enum{UNMARK, MARKED};
enum{PSSTART, PSEND, PESTART, PEEND};



struct ZigzagPoint 
{
	CPoint3D pt;      //点的坐标
	double angle;     //该点对应的角度
	double Feedrate;  //该点对应的进给速度
	bool   b_isIn;    //该点是进入内部区域还是离开区域
	bool   b_isEndPoint;//该点是不是某段zigzag的最后一个点

	ZigzagPoint(){}
	ZigzagPoint(CPoint3D& pt)
	{
		this->pt = pt;
	}
	ZigzagPoint(ZigzagPoint& _pt)
	{
		pt = _pt.pt;
		angle = _pt.angle;
		Feedrate = _pt.Feedrate;
		b_isIn = _pt.b_isIn;
		b_isEndPoint = _pt.b_isEndPoint;
	}
	ZigzagPoint(CPoint3D& n_pt,double n_angle , double n_Feedrate, bool n_b_isIn, bool n_b_isEndPoint)
	{
         this->pt = n_pt;
		 this->angle = n_angle;
		 this-> Feedrate = n_Feedrate;
		 this-> b_isIn = n_b_isIn;
		 this->b_isEndPoint = n_b_isEndPoint;
	}

	void operator =(CPoint3D _pt)
	{
		pt =_pt;
	}

	void operator =(ZigzagPoint& _pt)
	{
		pt = _pt.pt;
		angle = _pt.angle;
		Feedrate = _pt.Feedrate;
		b_isIn = _pt.b_isIn;
		b_isEndPoint = _pt.b_isEndPoint;
	}
};

typedef vector<ZigzagPoint*> ZigzagPath;//一条zigzag路径
typedef vector<ZigzagPath> ZigzagPaths; //一个layer所有的路径

//这个结构体就是在排序用的
struct TargetLine 
{
	double n_shortdis; //距离
	int    n_index;    //下标
	int    n_type;     //是该条线段上那个点作为起点  PSSTART....

	TargetLine(){}

	TargetLine(double _n_shortdis, int _n_index,int _n_type)
	{
		n_shortdis = _n_shortdis;
		n_index = _n_index;
		n_type  = _n_type;

	}

	bool operator < (const TargetLine& _n_targetline)
	{
		if (n_shortdis<_n_targetline.n_shortdis)
			return true;
		else
			return false;
	}
};



class AFX_CLASS_EXPORT CZigZagPath :
	public CEntity
{
public:
	CZigZagPath(void);
	~CZigZagPath(void);
public:
	virtual void Draw( COpenGLDC* pDC, COLORREF clr);


public:
	ZigzagPath  m_tmpZiazag;
	ZigzagPaths m_Ziazagpaths;
	ZigzagPaths m_FinalZiazagpaths;//最终分组之后得到的结果
	vector<CHermite*> m_Hermite;  //连接的Hermite曲线



	void push_up_Point(ZigzagPoint* _zigzagpoint);
	void push_up_Path(ZigzagPath _zigzagpath);
	void clear_tmppath();
	void releaseMem();

    /*从一组平行的线条生成
	  n_lines是一组平行线
	  n_type是路径的起点 
	  PSSTART:第一条的起点作为起点
      PSEND  : 第一条的终点作为起点
	  PESTART：最后一条的起点作为起点
	  PEEND  ：最后一条的终点作为起点
	*/
     void GntZigzagFRomlines(ZigzagPath& n_zigzagPath ,vector<CLine> n_lines, int n_type);


	 /*从一个zigzag路径以及起点的情况获得最后一个点的坐标*/
	 CPoint3D GetZigzagEndPoint(vector<CLine> n_lines, int n_type);
	 /*获取一个点到一个zigzag路径最短的路径,并且返回是哪个点*/
	 int FindShortestLen(CPoint3D pt,vector<CLine> n_lines, double &len);
     /*把路径ZigzagPaths中的路径进行连接*/
	 void FineProcessingPath(double n_width);
	 /*用Hermite曲线连接*/
	 void HermiteLinking(double n_width);


public:
	
};
