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
	CPoint3D pt;      //�������
	double angle;     //�õ��Ӧ�ĽǶ�
	double Feedrate;  //�õ��Ӧ�Ľ����ٶ�
	bool   b_isIn;    //�õ��ǽ����ڲ��������뿪����
	bool   b_isEndPoint;//�õ��ǲ���ĳ��zigzag�����һ����

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

typedef vector<ZigzagPoint*> ZigzagPath;//һ��zigzag·��
typedef vector<ZigzagPath> ZigzagPaths; //һ��layer���е�·��

//����ṹ������������õ�
struct TargetLine 
{
	double n_shortdis; //����
	int    n_index;    //�±�
	int    n_type;     //�Ǹ����߶����Ǹ�����Ϊ���  PSSTART....

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
	ZigzagPaths m_FinalZiazagpaths;//���շ���֮��õ��Ľ��
	vector<CHermite*> m_Hermite;  //���ӵ�Hermite����



	void push_up_Point(ZigzagPoint* _zigzagpoint);
	void push_up_Path(ZigzagPath _zigzagpath);
	void clear_tmppath();
	void releaseMem();

    /*��һ��ƽ�е���������
	  n_lines��һ��ƽ����
	  n_type��·������� 
	  PSSTART:��һ���������Ϊ���
      PSEND  : ��һ�����յ���Ϊ���
	  PESTART�����һ���������Ϊ���
	  PEEND  �����һ�����յ���Ϊ���
	*/
     void GntZigzagFRomlines(ZigzagPath& n_zigzagPath ,vector<CLine> n_lines, int n_type);


	 /*��һ��zigzag·���Լ��������������һ���������*/
	 CPoint3D GetZigzagEndPoint(vector<CLine> n_lines, int n_type);
	 /*��ȡһ���㵽һ��zigzag·����̵�·��,���ҷ������ĸ���*/
	 int FindShortestLen(CPoint3D pt,vector<CLine> n_lines, double &len);
     /*��·��ZigzagPaths�е�·����������*/
	 void FineProcessingPath(double n_width);
	 /*��Hermite��������*/
	 void HermiteLinking(double n_width);


public:
	
};
