#include "stdafx.h"
#include "global.h"


double g_baseLevel=-10;
double g_supportSectionInterval=10;
double g_sawTriangleLength = 3;
double g_sawTriangleInterval = g_sawTriangleLength * 4/3;

double g_maxLayerHeight=0.2;
double g_minLayerHeight=0.05;
double g_stairError=0.1;

double g_contourSpotDia=0.1;
double g_scanlineInterval=1;
double g_scanlineSpotDia=0.2;
double g_feedrate = 100;
double g_feedrate_air = 200;
double g_biggestSpotDia = 2.0;
double g_smallestSpotDia = 0.1;
double g_overlapRatio = 0.01;   // 1%


bool IsTriInTris(LTriangle* pTri, vector<LTriangle*>& tris, bool bDel/* =true */)
{
	int sz=tris.size();
	for (int i=0;i<sz;i++)  // need to be optimized
	{
		if (pTri == tris[i])
		{
			if (bDel)
			{
				tris.erase(tris.begin()+i);
			}
			return true;
		}
	}
	return false;
}


struct LinkPoint : public CPoint3D
{
	LinkPoint()
	{
		pOther=NULL;
		idx=-1;
		bUsed=false;
	}
	~LinkPoint(){}
	LinkPoint(const CPoint3D& pt)
	{
		x=pt.x;
		y=pt.y;
		z=pt.z;
		bUsed=false;
	}
	const LinkPoint& operator=(const LinkPoint& ptLink)
	{
		x=ptLink.x;
		y=ptLink.y;
		z=ptLink.z;
		pOther=ptLink.pOther;
		idx=ptLink.idx;
		bUsed=ptLink.bUsed;
		return *this;
	}

	LinkPoint* pOther;  // the other point of the segment
	bool bUsed;
	int  idx;
};


bool smaller(LinkPoint* sp1, LinkPoint* sp2)
{
	if (((float)sp1->x <  (float)sp2->x)  || 
		(((float)sp1->x == (float)sp2->x) && ((float)sp1->y <  (float)sp2->y)) ||
		(((float)sp1->x == (float)sp2->x) && ((float)sp1->y == (float)sp2->y)  && ((float)sp1->z < (float)sp2->z)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool smaller_planar(LinkPoint* sp1, LinkPoint* sp2)
{
	if ((sp1->x < sp2->x)  || 
		((sp1->x == sp2->x) && (sp1->y < sp2->y)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

int FindAnUnusedLP(vector<LinkPoint*> LPs)
{
	int sz=LPs.size();
	for (int i=0;i<sz;i++)
	{
		if (false==LPs[i]->bUsed)
		{
			return i;
		}
	}

	return -1;
}

int SearchInLps(LinkPoint* pLP, vector<LinkPoint*> LPs)
{
	int sz=LPs.size();
	for (int i=0;i<sz;i++)
	{
		if (LPs[i]->bUsed)
		{
			continue;
		}

		if (*pLP ^= *LPs[i])
		{
			return i;
		}
	}

	return -1;
}


void LinesToPolylines(vector<CLine>& lines, vector<CPolyline*>& boundaries, bool bPlanar /* = false */)
{
	vector<LinkPoint*> LPs;

	int szLine=lines.size();
	for (int i=0;i<szLine;i++)
	{
		if (lines[i].m_pt1 != lines[i].m_pt2)
		{
			LinkPoint* pLP1=new LinkPoint(lines[i].m_pt1);
			LinkPoint* pLP2=new LinkPoint(lines[i].m_pt2);
			pLP1->pOther=pLP2;
			pLP2->pOther=pLP1;
			LPs.push_back(pLP1);
			LPs.push_back(pLP2);
		}
	}

	bPlanar ? sort(LPs.begin(),LPs.end(),smaller_planar) : sort(LPs.begin(),LPs.end(),smaller);

	int szLP=LPs.size();
	for (int i=0;i<szLP;i++)
	{
		LPs[i]->idx=i;
	}

	LinkPoint* pCurLP;
	int initIdx=0,curIdx;
	bool bClosed=false;
	vector<CPoint3D*> tmp_pts;
	while (true)
	{
		bClosed=false;
		curIdx=initIdx;
		pCurLP=LPs[curIdx];
		tmp_pts.push_back(new CPoint3D(*pCurLP));
		pCurLP->bUsed=true;
		pCurLP=LPs[curIdx]->pOther;
		tmp_pts.push_back(new CPoint3D(*pCurLP));
		curIdx=pCurLP->idx;
		pCurLP->bUsed=true;

		while (true)
		{
			if ((curIdx-1>=0) && false==LPs[curIdx-1]->bUsed && (*pCurLP ^= *LPs[curIdx-1]))
			{
				pCurLP=LPs[curIdx-1]->pOther;
				tmp_pts.push_back(new CPoint3D(*pCurLP));
				LPs[curIdx-1]->bUsed=true;
				pCurLP->bUsed=true;
				curIdx=pCurLP->idx;
			}
			else if((curIdx+1<szLP) && false==LPs[curIdx+1]->bUsed && (*pCurLP ^= *LPs[curIdx+1])) // shouldn't have problem
			{
				pCurLP=LPs[curIdx+1]->pOther;
				tmp_pts.push_back(new CPoint3D(*pCurLP));
				LPs[curIdx+1]->bUsed=true;
				pCurLP->bUsed=true;
				curIdx=pCurLP->idx;
			}
			else
			{
				int idx = SearchInLps(pCurLP,LPs);
				if (-1 == idx)
				{
					break;
				}
				else
				{
					pCurLP=LPs[idx]->pOther;
					tmp_pts.push_back(new CPoint3D(*pCurLP));
					LPs[idx]->bUsed=true;
					pCurLP->bUsed=true;
					curIdx=pCurLP->idx;
				}
			}

			if (*pCurLP ^= *tmp_pts[0])
			{
				bClosed=true;
				break;
			}
		}   // If the curve is closed, means stretch to one end can meet the other end, eventually
		// But if the curve is not closed, must stretch to the other end, too

		if (false==bClosed)
		{
			curIdx=initIdx;        // To make the algorithm more robot, I have to consider the intersecting curves -
			pCurLP=LPs[curIdx];	   // - that are not closed.
			while (true)
			{
				if ((curIdx-1>=0) && false==LPs[curIdx-1]->bUsed && (*pCurLP ^= *LPs[curIdx-1]))
				{
					pCurLP=LPs[curIdx-1]->pOther;
					tmp_pts.insert(tmp_pts.begin(),new CPoint3D(*pCurLP));
					LPs[curIdx-1]->bUsed=true;
					pCurLP->bUsed=true;
					curIdx=pCurLP->idx;
				}
				else if ((curIdx+1<szLP) && false==LPs[curIdx+1]->bUsed && (*pCurLP ^= *LPs[curIdx+1]))
				{
					pCurLP=LPs[curIdx+1]->pOther;
					tmp_pts.insert(tmp_pts.begin(),new CPoint3D(*pCurLP));
					LPs[curIdx+1]->bUsed=true; 
					pCurLP->bUsed=true;
					curIdx=pCurLP->idx;
				}
				else
				{
					int idx = SearchInLps(pCurLP,LPs);
					if (-1 == idx)
					{
						break;
					}
					else
					{
						pCurLP=LPs[idx]->pOther;
						tmp_pts.insert(tmp_pts.begin(),new CPoint3D(*pCurLP));
						LPs[idx]->bUsed=true;
						pCurLP->bUsed=true;
						curIdx=pCurLP->idx;
					}
				}
			}
		}


		int szPt=tmp_pts.size();
		if (2==szPt && bClosed)   // 清除只有两个顶点且闭合的线，只有在线段非常短时会发生这种情况
		{
			for (int i=0;i<szPt;i++)
			{
				delete tmp_pts[i];
				tmp_pts[i]=NULL;
			}
			tmp_pts.clear();
		}
		else
		{
			CPolyline* pBoundary=new CPolyline();
			pBoundary->m_bIsClosed=bClosed;
			for (int i=0;i<szPt;i++)
				pBoundary->AddPt(tmp_pts[i]);
			tmp_pts.clear();
			pBoundary->DelColinearPt();
			boundaries.push_back(pBoundary);
		}

		initIdx=FindAnUnusedLP(LPs);
		if (-1==initIdx)
			break;
	}

	int sz=LPs.size();  
	for (int i=sz-1;i>=0;i--)
	{
		delete LPs[i];
		LPs[i]=NULL;
	}
	LPs.clear();
}

CPolyline* LinesToPolyline(vector<CLine>& lines)
{
	vector<CPolyline*> polylines;
	LinesToPolylines(lines,polylines);
	return polylines[0];
}

struct Segment
{
	Segment(){}
	Segment(const Segment& seg)
	{
		pt1 = seg.pt1;
		pt2 = seg.pt2;
	}
	Segment(const CPoint3D& _pt1, const CPoint3D& _pt2)
	{
		pt1=_pt1;
		pt2=_pt2;

		if (pt2.y>pt1.y)   //在segment内部也做一个排序
			swap(pt1,pt2);
	}

	double GetMaxY()
	{
		return pt1.y;
	}

	double GetMinY()
	{
		return pt2.y;
	}

	bool operator < (const Segment& seg)//重载<是拿来排序用的，降序
	{
		if (pt1.y>seg.pt1.y)
			return true;
		else 
			return false;
	}

	int CalcCrossPtWithYLine(double y, CPoint3D& ptCross)  // 和y扫描线计算交点，前提是有交点,
	{                                                       // 和y线重合认为没交点
		if (pt1.y == pt2.y)
		{
			return 0;      // 重合，没交点
		}

		if (pt1.y == y)
		{
			ptCross = pt1;
			return 1;      // 有交点，交点在pt1
		}

		if (pt2.y == y)
		{
			ptCross = pt2;
			return 2;      // 有交点，交点在pt2
		}

		double k=(pt1.y - y)/(y-pt2.y);
		double x=(pt1.x+k*pt2.x)/(1+k);
		ptCross=CPoint3D(x,y,pt1.z);
		return 3;          // 有交点，交点在pt1、pt2中间某点
	}

	CPoint3D pt1, pt2;  // pt1.y >= pt2.y must
};

// 点和线段放在一起的数据结构
// 用于 GntSweepLinesForBoundaries() 里存放数据结构
struct Pt_Seg
{
	Pt_Seg(){}
	Pt_Seg(const CPoint3D& _pt, const Segment& _seg, int _nState)
	{
		pt=_pt;
		seg=_seg;
		nState=_nState;
	}
	bool operator < (const Pt_Seg& ptseg)
	{
		if (pt.x < ptseg.pt.x)
			return true;
		else
			return false;
	}
	CPoint3D pt;
	Segment  seg;
	int      nState;
};


// 扫描线算法，时间复杂度为接近线性
void GntSweeplines(vector<CPolyline*>&		boundaries,							// 多边形起点和末尾点是重合的
				   double					lineInterval,						// 间距
				   double					angle,								// 倾斜角度
				   vector<vector<CLine>>&	lineses,							// 输出扫描线
				   bool						bBoundaryRotateBack/* = false*/)    // angle不为0时是否要对输入boundary旋转回来
{
	int szBoundary=boundaries.size();
	if (0==lineInterval || 0==szBoundary)
		return;

	CMatrix3D mf,mb;
	if (0!=angle)
	{
		mf=CMatrix3D::CreateRotateMatrix(-angle,AXIS_Z);
		mb=CMatrix3D::CreateRotateMatrix(angle,AXIS_Z);
		for (int i=0;i<szBoundary;i++)
		{
			boundaries[i]->Rotate(mf);
		}
	}

	double yMin=HUGE;
	vector<Segment> segs;                               // 这里令我比较纠结的一件事情是：我必须先对轮廓进行旋转， 
	for (int i=0;i<szBoundary;i++)                      // 在得到的旋转多边形上取得一条条线段；而不能够先获得线段
	{													// 然后对线段进行旋转。原因是我一段直接旋转线段，得到的旋
		Point3DAry& pts=boundaries[i]->m_pts;			// 转后的线段集竟然可能不是首尾相接的，这可能是计算机截断
		int szPt=pts.GetSize();							// 误差造成的。为了保证线段集首尾相接，我不得不先旋转轮廓											
		for (int j=0;j<=szPt-3;j++)						// 多边形。
		{
			Segment seg(*pts[j],*pts[j+1]);
			segs.push_back(seg);
		}												 // Debug 2013-9-23
		Segment seg(*pts[szPt-2],*pts[0]);               // 旋转后，这里pt[0]和pt[sz-1]可能不重合，所以用回pt[0]
		segs.push_back(seg);

		double yMin_ = boundaries[i]->FindExtreme(CPointSet::MIN_Y);
		if (yMin_ < yMin)
		{
			yMin = yMin_;
		}
	}

	//把轮廓给旋转回去了
	if (bBoundaryRotateBack && 0!=angle)
	{
		for (int i=0;i<szBoundary;i++)
		{
			boundaries[i]->Rotate(mb);
		}
	}

	int szSeg=segs.size();
	for (int i=szSeg-1; i>=1; i--)
	{
		swap(segs[i],segs[rand()%i]);                     // 将线段顺序打乱，有利于加快下面的排序速度
	}
	sort(segs.begin(),segs.end());                        //按照y的值从大到小排序，因为segment中重载了〈

	vector<Segment> status;
	double y=segs[0].GetMaxY() - lineInterval/2.0;       // y线从上到下扫描，第一条扫描线
	int cnt=0, szStatus=0;
	CPoint3D ptCross;

	while (true)
	{

		if (y<yMin)   // I debugged this bug on 2013-10-29
		{             // Just to fullfill the whole rigion
			if (y+lineInterval/2.0 > yMin)
			{
				y = y + lineInterval/2.0;
			}
			else
			{
				break;
			}
		}

		szStatus=status.size();
		for (int i=szStatus-1;i>=0;i--)//把原先其中不可能与y相交的部分内容删掉，仍然留下其中的
		{
			if (status[i].GetMinY() > y)
			{
				status.erase(status.begin()+i);
			}
		}

		while (true)//这个while把y扫描线上方的segments都处理了一遍
		{
			if (cnt < szSeg && segs[cnt].GetMaxY() >= y)//最大部分在扫描线上方的segments
			{
				if (segs[cnt].GetMinY() <= y)           //最小部分在扫描线下方
				{
					status.push_back(segs[cnt]);//存在交点的seg
				}
				cnt++;			
			}
			else
			{
				break;
			}
		}  // 此时第cnt条seg的MaxY < y

		vector<Pt_Seg> ptsegs;//保存扫描线与轮廓的交点信息
		szStatus=status.size();
		for (int i=0;i<szStatus;i++)
		{
			int nState=status[i].CalcCrossPtWithYLine(y,ptCross);
			if (0==nState)
			{									 // Debug 2013-9-24
				y = y - lineInterval*1.0e-10;    // 为了避免线段和扫描线重合引发的难以判断的局面，该情况下将扫描线稍微下移
				continue;
			}
			else if (nState>0)
			{
				Pt_Seg ptseg(ptCross,status[i],nState);
				ptsegs.push_back(ptseg);
			}
		}

		sort(ptsegs.begin(),ptsegs.end());//按照交点x的大小进行排序
		int szPtsegs=ptsegs.size();
		for (int i=szPtsegs-1;i>=1;i--)//把重合的交点处理掉？与两个segment是同一个交点？是\/ 还是/\,或者一个最下，一个最上
		{
			if (ptsegs[i].pt == ptsegs[i-1].pt)//
			{
				if (ptsegs[i].nState == ptsegs[i-1].nState/*(ptsegs[i].nState == 1 && ptsegs[i-1].nState == 1) || (ptsegs[i].nState == 2 && ptsegs[i-1].nState == 2)*/)
				{ // here there is still bug
					ptsegs.erase(ptsegs.begin()+i);
					ptsegs.erase(ptsegs.begin()+i-1);
					i--;
				}
				else
				{
					ptsegs.erase(ptsegs.begin()+i);
				}
			}
		}

		szPtsegs=ptsegs.size();
		if (szPtsegs>0 && szPtsegs%2==0)//交点肯定是偶数个的
		{
			vector<CLine> lines;  //一条扫描线对应了一个lines。。。
			for (int i=0;i<szPtsegs;i=i+2)//保存下来的线全都是有效线条，包括每个交点的信息，是进去的还是出来的
			{
				CLine line(ptsegs[i].pt,ptsegs[i+1].pt);
				/////////////////////////////////
				if (0!=angle)                  //我先不旋转，因为我后面还有一些操作还是针对平行于x轴的，最后再旋转
					line.Rotate(mb);             //
				/////////////////////////////////
				if (HALFPI == angle)
					line.m_pt1.x = line.m_pt2.x;
				lines.push_back(line);
			}
			lineses.push_back(lines);
		}

		y=y-lineInterval;
	}
}

bool CalcCrossPt(const CPoint3D& P0, const CVector3D& v,    // line1: point-direction pattern
				 const CPoint3D& A,  const CPoint3D& B,     // line2: line segment, two end points
				 CPoint3D& ptCross)                         // output: cross point if exists; 
{
	CPoint3D A_ = P0 + v*((A-P0)|v);
	CPoint3D B_ = P0 + v*((B-P0)|v);

	CVector3D AA_ = A_ - A;
	CVector3D BB_ = B_ - B;
	double dp = AA_ | BB_;

	if (dp < 0)
	{
		double k=AA_.GetLength()/BB_.GetLength();
		ptCross = A + (B-A)*(k/(1+k));
		return true;
	}
	else if (dp == 0)
	{
		if (AA_.GetLength()==0)
		{
			ptCross = A;
		}
		else
		{
			ptCross = B;
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool CalcTwoTrisInst(const CPoint3D& A1, const CPoint3D& B1, const CPoint3D& C1, 
					 const CPoint3D& A2, const CPoint3D& B2, const CPoint3D& C2, 
					 CLine& line)
{
	CVector3D n1=(A1-B1)*(C1-B1);
	CVector3D n2=(A2-B2)*(C2-B2);
	n1.Normalize();
	n2.Normalize();
	CVector3D v=n1*n2;
	v.Normalize();

	double a1, b1, c1, d1, a2, b2, c2, d2;
	a1 = n1.dx;	  
	b1 = n1.dy;   
	c1 = n1.dz;
	d1 = -(a1*A1.x + b1*A1.y + c1*A1.z);
	a2 = n2.dx;	  
	b2 = n2.dy;   
	c2 = n2.dz;
	d2 = -(a2*A2.x + b2*A2.y + c2*A2.z);

	double x, y, z;

	if (0==a2)
	{
		if (0==b2)
		{
			if (0==a1)
			{
				x=0; y=(c1*d2/c2-d1)/b1; z=-d2/c2;
			}
			else
			{
				y=0; x=(c1*d2/c2-d1)/a1; z=-d2/c2;
			}
		}
		else  // b2!=0
		{
			if (0==a1)
			{
				x=0; z=(d2*b1-d1*b2)/(b2*c1-b1*c2); y=-c2*z/b2-d2/b2;
			}
			else
			{
				z=0; x=(d2*b1/b2-d1)/a1; y=-d2/b2;
			}
		}
	}
	else
	{
		double tmp=a2*b1-a1*b2;
		if (0==tmp)
		{
			y=0; z=(a1*d2-a2*d1)/(a2*c1-a1*c2); x=(-c2*z-d2)/a2;
		}
		else
		{
			z=0; y=(a1*d2-a2*d1)/tmp; x=(-b2*y-d2)/a2;
		}
	}
	CPoint3D P0(x,y,z);

	typedef pair<CPoint3D, CPoint3D> Seg;
	Seg seg1, seg2;
	CPoint3D I1, I2, I3, J1, J2, J3;
	bool p1, p2, p3, q1, q2, q3;

	p1=CalcCrossPt(P0,v,A1,B1,I1);
	p2=CalcCrossPt(P0,v,B1,C1,I2);
	p3=CalcCrossPt(P0,v,C1,A1,I3);
	if (p1 && p2 && !p3)
	{
		seg1.first=I1;
		seg1.second=I2;
	}
	else if (p2 && p3 && !p1)
	{
		seg1.first = I2;
		seg1.second = I3;
	}
	else if (p1 && p3 && !p2)
	{
		seg1.first = I1;
		seg1.second = I3;
	}
	else
	{
		return false;
	}

	q1=CalcCrossPt(P0,v,A2,B2,J1);
	q2=CalcCrossPt(P0,v,B2,C2,J2);
	q3=CalcCrossPt(P0,v,C2,A2,J3);
	if (q1 && q2 && !q3)
	{
		seg2.first=J1;
		seg2.second=J2;
	}
	else if (q2 && q3 && !q1)
	{
		seg2.first = J2;
		seg2.second = J3;
	}
	else if (q1 && q3 && !q2)
	{
		seg2.first = J1;
		seg2.second = J3;
	}
	else
	{
		return false;
	}


	CPoint3D ptFar = P0 + v*100000000;

	if (GetDistance(seg1.first,ptFar) > GetDistance(seg1.second,ptFar))
	{
		swap(seg1.first, seg1.second);   // first 在左边，second 在右边
	}

	if (GetDistance(seg2.first,ptFar) > GetDistance(seg2.second,ptFar))
	{
		swap(seg2.first, seg2.second);
	}

	if (GetDistance(seg1.first, seg1.second) < GetDistance(seg2.first,seg2.second))
	{
		swap(seg1,seg2);  // 让 seg1 比 seg2 长
	}

	bool on1, on2;
	on1 = IsPtOnSeg(seg2.first,seg1.first,seg1.second);
	on2 = IsPtOnSeg(seg2.second,seg1.first,seg1.second);

	if ( !on1 && !on2)
	{
		return false;
	}
	else if (on1 && !on2)
	{
		line = CLine(seg2.first, seg1.second);
	}
	else if (!on1 && on2)
	{
		line = CLine(seg1.first, seg2.second);
	}
	else
	{
		line = CLine(seg2.first, seg2.second);
	}

	if (line.m_pt1 == line.m_pt2)
	{
		return false;
	}
	else
	{
		return true;
	}
}


bool CalcTriXYPlaneInst(const CPoint3D& A, const CPoint3D& B, const CPoint3D& C, double z, CLine& line_out)
{
	if (A.z>=z && B.z>=z && C.z>=z)
		return false;
	if (A.z<=z && B.z<=z && C.z<=z)
		return false;

	CPoint3D A_,B_,C_;
	CVector3D vec1,vec2;
	double lamda1,lamda2;

	if ((A.z-z)*(B.z-z)>0)
	{
		A_=C;
		B_=A;
		C_=B;
		goto NO_POINT_ON_SURFACE;
	}
	if ((B.z-z)*(C.z-z)>0)
	{
		A_=A;
		B_=B;
		C_=C;
		goto NO_POINT_ON_SURFACE;
	}
	if ((A.z-z)*(C.z-z)>0)
	{
		A_=B;
		B_=A;
		C_=C;
		goto NO_POINT_ON_SURFACE;
	}

	if (A.z==z)
	{
		A_=A;
		B_=B;
		C_=C;
		goto ONE_POINT_ON_SURFACE;
	}
	if (B.z==z)
	{
		A_=B;
		B_=A;
		C_=C;
		goto ONE_POINT_ON_SURFACE;
	}
	if (C.z==z)
	{
		A_=C;
		B_=B;
		C_=A;
		goto ONE_POINT_ON_SURFACE;
	}

NO_POINT_ON_SURFACE:
	lamda1=(z-A_.z)/(B_.z-A_.z);
	lamda2=(z-A_.z)/(C_.z-A_.z);
	vec1=B_-A_;
	vec2=C_-A_;
	line_out.m_pt1=A_+vec1*lamda1;
	line_out.m_pt2=A_+vec2*lamda2;
	line_out.m_pt1.z = z;  // 修正z
	line_out.m_pt2.z = z;
	if (line_out.m_pt1 == line_out.m_pt2)
	{
		return false;
	}
	else 
	{
		return true;
	}


ONE_POINT_ON_SURFACE:
	lamda1=(z-B_.z)/(C_.z-B_.z);
	vec1=C_-B_;
	line_out.m_pt1=A_;
	line_out.m_pt2=B_+vec1*lamda1;
	line_out.m_pt1.z = z;  // 修正z
	line_out.m_pt2.z = z;
	if (line_out.m_pt1 == line_out.m_pt2)
	{
		return false;
	}
	else 
	{
		return true;
	}
}


double CalcTriangleArea(const CPoint3D& A, const CPoint3D& B, const CPoint3D& C)
{
	double h=GetDistPtToLine(A,B,C);
	double l=GetDistance(B,C);
	return 0.5*h*l;
}


// bak codes
/*

void GntSweepLinesForBoundaries(vector<CPolyline*>& boundaries,  // 多边形起点和末尾点是重合的
double lineInterval,             // 间距
double angle,                    // 倾斜角度
vector<vector<CLine>>& liness)   // 输出线段集合
{
CMatrix3D mf,mb;
int szBoundary=boundaries.size();
if (0!=angle)
{
mf=CMatrix3D::CreateRotateMatrix(-angle,AXIS_Z);
mb=CMatrix3D::CreateRotateMatrix(angle,AXIS_Z);
for (int i=0;i<szBoundary;i++)
{
boundaries[i]->Rotate(mf);
}
}

vector<Segment> segs;                               // 这里令我比较纠结的一件事情是：我必须先对轮廓进行旋转， 
for (int i=0;i<szBoundary;i++)                      // 在得到的旋转多边形上取得一条条线段；而不能够先获得线段
{													// 然后对线段进行旋转。原因是我一段直接旋转线段，得到的旋
Point3DAry& pts=boundaries[i]->m_pts;			// 转后的线段集尽然可能不是首尾相接的，这可能是计算机截断
int szPt=pts.GetSize();							// 误差造成的。为了保证线段集首尾相接，我不得不先旋转轮廓
// 多边形。
for (int j=0;j<szPt-1;j++)
{
Segment seg(*pts[j],*pts[j+1]);
segs.push_back(seg);
}
}

if (0!=angle)
{
for (int i=0;i<szBoundary;i++)
{
boundaries[i]->Rotate(mb);
}
}

int szSeg=segs.size();
for (int i=szSeg-1; i>=1; i--)
{
swap(segs[i],segs[rand()%i]);
}
sort(segs.begin(),segs.end());


vector<Segment> status;
int cnt=0;
double y=segs[cnt].GetMaxY();   // y线从上到下扫描
status.push_back(segs[cnt]);

cnt++;
while (true)
{
if (segs[cnt].GetMaxY() >= y)
{
if (segs[cnt].GetMinY() <= y)
{
status.push_back(segs[cnt]);
}
cnt++;

if (cnt == szSeg)
{
break;
}
}
else
{
break;
}

}  // 此时第i条seg的MaxY < y


double yMin=segs[szSeg-1].GetMinY();
while (true)
{
y=y-lineInterval;
if (y<yMin)
{
break;
}

int szStatus=status.size();
for (int i=szStatus-1;i>=0;i--)
{
if (status[i].GetMinY() > y)
{
status.erase(status.begin()+i);
}
}

while (true)
{
if (segs[cnt].GetMaxY() >= y)
{
if (segs[cnt].GetMinY() <= y)
{
status.push_back(segs[cnt]);
}
cnt++;

if (cnt == szSeg)
{
break;
}				
}
else
{
break;
}

}  // 此时第i条seg的MaxY < y

szStatus=status.size();
CPoint3D ptCross;
vector<Pt_Seg> ptsegs;
for (int i=0;i<szStatus;i++)
{
int nState=status[i].CalcCrossPtWithYLine(y,ptCross);
if (nState>0)
{
Pt_Seg ptseg(ptCross,status[i],nState);
ptsegs.push_back(ptseg);
}
}

sort(ptsegs.begin(),ptsegs.end());

int szPtsegs=ptsegs.size();
for (int i=szPtsegs-1;i>=1;i--)
{
if (ptsegs[i].pt == ptsegs[i-1].pt)
{
if ((ptsegs[i].nState == 1 && ptsegs[i-1].nState == 1) || (ptsegs[i].nState == 2 && ptsegs[i-1].nState == 2))
{
ptsegs.erase(ptsegs.begin()+i);
ptsegs.erase(ptsegs.begin()+i-1);
}
else
{
ptsegs.erase(ptsegs.begin()+i);
}
}
}

szPtsegs=ptsegs.size();
vector<CLine> lines;
for (int i=0;i<szPtsegs;i=i+2)
{
CLine line(ptsegs[i].pt,ptsegs[i+1].pt);
if (0!=angle)
line.Rotate(mb);
lines.push_back(line);
}
liness.push_back(lines);

}
}


*/