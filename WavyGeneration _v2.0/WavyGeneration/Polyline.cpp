#include "StdAfx.h"
#include "Polyline.h"
#include "PolylineSet.h"
#include "AzClipper.h"

// There must be a rule, it's upon to us to find it. //


// The marco TINY is defined to make distance a bit greater than the calculated value //
#define TINY             0.0000000001  // 10 '0's
#define LARGE            9999999999999999999.9

#define TOP_END          1    // 上端点
#define BOTTOM_END       2

// 位置类
class CPolyline::CPos : public CObject
{
public:
	CPos(){}
	CPos(const int& i_in, const int& sz_in)
	{
		i=i_in;
		sz=sz_in;
	}
	~CPos(){}
	CPos(const CPos& pos)
	{
		i=pos.i;
		sz=pos.sz;
	}
	const CPos& operator=(const CPos& pos)
	{
		i=pos.i;
		sz=pos.sz;
		return *this;
	}

public:
	int i;
	int sz;
};

// 线段交点类,记录线段交点的坐标以及线段的序号 // 
class CPolyline::CIstPt : public CObject
{
public:
	CIstPt()
	{
		m_s=m_b=0.0;
		pFather=NULL;
	}
	~CIstPt()
	{
		int sz=pChildList.GetSize();
		for (int i=sz-1;i>=0;i--)
			delete pChildList[i];
		pChildList.RemoveAll();
	}
	CIstPt(const CPoint3D& pt, const double& s, const double& b)
	{
		m_pt=pt;
		m_s=s;
		m_b=b;
	}
	CIstPt(const CIstPt& istPt)
	{
		m_pt=istPt.m_pt;
		m_s=istPt.m_s;
		m_b=istPt.m_b;
	}

	const CIstPt& operator=(const CIstPt& istPt)
	{
		m_pt=istPt.m_pt;
		m_s=istPt.m_s;
		m_b=istPt.m_b;
		return *this;
	}

	BOOL operator==(const CIstPt& istPt) const
	{
		if (istPt.m_pt==m_pt)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	BOOL operator<(const CIstPt& istPt) const // belong
	{
		if (*this==istPt)
		{
			return FALSE;
		}
		else if (m_s>=istPt.m_s && m_b<=istPt.m_b)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	BOOL operator>(const CIstPt& istPt) const // contain
	{
		if (*this==istPt)
		{
			return FALSE;
		}
		else if (m_s<=istPt.m_s && m_b>=istPt.m_b)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}		
	}

	BOOL operator*(const CIstPt& istPt) const // intersection
	{
		if ((istPt.m_s>m_s && istPt.m_s<m_b && istPt.m_b>m_b) ||
			(istPt.m_b>m_s && istPt.m_b<m_b && istPt.m_s<m_s))
			return TRUE;
		else return FALSE;
	}

	BOOL Add(CIstPt* pIstPt);

	CPoint3D m_pt;
	double m_s;  // 小的一个 small
	double m_b;  // 大的一个 big

	CIstPt* pFather;
	CTypedPtrArray<CObArray,CIstPt*> pChildList;
};

// 事件类 2011-6-24 //
class CPolyline::CEvt : public CPoint3D
{
public:
	CEvt(){}
	~CEvt(){}
	CEvt(const CEvt& evt)
	{
		x=evt.x;
		y=evt.y;
		z=evt.z;
		i=evt.i;
		type_b=evt.type_b;
		type_f=evt.type_f;
	}

	CEvt(const CPoint3D& pt, const int& i_i)
	{
		x=pt.x;
		y=pt.y;
		z=pt.z;
		i=i_i;
	}

	const CEvt& operator=(const CEvt& evt)
	{
		x=evt.x;
		y=evt.y;
		z=evt.z;
		i=evt.i;
		type_b=evt.type_b;
		type_f=evt.type_f;
		return *this;
	}

	BOOL operator>(const CEvt& evt) const
	{
		if (y < evt.y) return TRUE;
		if (y > evt.y) return FALSE;
		if (x>evt.x) return TRUE;
		return FALSE;
	}

	BOOL operator<(const CEvt& evt) const
	{
		if (y > evt.y) return TRUE;
		if (y < evt.y) return FALSE;
		if (x<evt.x) return TRUE;
		return FALSE;
	}

	int i;       // i>=0 时这点是原始多边形上的哪个点， -1 表示是一个交点
	int type_b;  // 前面这条线段在该点是上端点还是下端点 i==-1时表示交点的两条线段
	int type_f;  // 后面这条线段在该点是上端点还是下端点
};

// 事件空间，所有事件 //
class CPolyline::CEvtSpace
{
public:
	CEvtSpace(){}
	~CEvtSpace()
	{
		int sz=evtList.GetSize();
		for (int i=sz-1;i>=0;i--)
			delete evtList[i];
		evtList.RemoveAll();
	}

	void LoadEvts(Point3DAry &ptList);
	void SortEvts(const int& s, const int& e);  //
	int  InsertEvt(CEvt* pEvt);
	void RemoveEvt(int iWhich);


	CTypedPtrArray<CObArray,CEvt*> evtList;
};

class CPolyline::CStatus : public CLine
{
public:
	CStatus(){}
	~CStatus(){}

	CStatus(const CLine& seg, const int& i_i)
	{
		m_pt1=seg.m_pt1;
		m_pt2=seg.m_pt2;
		iStatus=i_i;
	}

	CStatus(const CPoint3D& pt_s, const CPoint3D& pt_e, const int& i_i)
	{
		m_pt1=pt_s;
		m_pt2=pt_e;
		iStatus=i_i;
	}

	CStatus(const CStatus& status)  // ptS 保存线段上端点，ptE保存线段下端点
	{
		m_pt1=status.m_pt1;
		m_pt2=status.m_pt2;
		iStatus=status.iStatus;
	}

	BOOL operator>(const CStatus& status) const
	{
		if (m_pt1.x>status.m_pt1.x) return TRUE;
		if (m_pt1.x<status.m_pt1.x) return FALSE;
		if (m_pt2.x>status.m_pt2.x) return TRUE;
		return FALSE;
	}

	BOOL operator<(const CStatus& status) const
	{
		if (m_pt1.x<status.m_pt1.x) return TRUE;
		if (m_pt1.x>status.m_pt1.x) return FALSE;
		if (m_pt2.x<status.m_pt2.x) return TRUE;
		return FALSE;
	}

	BOOL IsRightOf(const CStatus& status, const double& y_line) const // >
	{
		double lamda=(status.m_pt1.y - y_line)/(status.m_pt1.y - status.m_pt2.y); // InsertStatus() 已经讨论了平行的情况
		double x=(1-lamda)*status.m_pt1.x+lamda*status.m_pt2.x;
		if (m_pt1.x > x) return TRUE;
		if (m_pt1.x < x) return FALSE;
		if (m_pt2.x > status.m_pt2.x) 
			return TRUE;
		else
			return FALSE;
	}

	BOOL IsLeftOf(const CStatus& status, const double& y_line) const  // <
	{
		double lamda=(status.m_pt1.y - y_line)/(status.m_pt1.y - status.m_pt2.y); // InsertStatus() 已经讨论了平行的情况
		double x=(1-lamda)*status.m_pt1.x+lamda*status.m_pt2.x;
		if (m_pt1.x < x) return TRUE;
		if (m_pt1.x > x) return FALSE;
		if (m_pt2.x < status.m_pt2.x) 
			return TRUE;
		else
			return FALSE;
	}

	int iStatus;    // 线段序号
};

// 状态空间类
class CPolyline::CStatusSpace 
{
public:
	CStatusSpace(){}
	~CStatusSpace()
	{
		int sz=statusList.GetSize();
		for (int i=sz-1;i>=0;i--)
			delete statusList[i];
		statusList.RemoveAll();
	}

	void SetYLine(const double& y_line) {yLine=y_line;} // Set 
	void SetSize(const int& sz) {iSzStutus=sz;}
	int  Add(CStatus* pStatus) {return statusList.Add(pStatus);}
	BOOL RemoveStatus(const int& iWhichSeg, IstPtAry &istPtList); // evt 返回可能存在的交点
	BOOL InsertStatus(CStatus* pStatus, IstPtAry &istPtList);
	int  SwapStatuses(const int& iSeg1, const int& iSeg2, IstPtAry &istPtList);

public:
	CEvtSpace evtSpace;
	CTypedPtrArray<CObArray,CStatus*> statusList;
	int iSzStutus; // 总的线段条数
	double yLine;
};

/*
// 角平分线类 //
class CPolyline::CPtBisector : public CPoint3D
{
public:
	CPtBisector(){}
	~CPtBisector(){}
	CPtBisector(const CPtBisector& ptBis)
	{
		bis=ptBis.bis;
		x=ptBis.x;
		y=ptBis.y;
		z=ptBis.z;
	}

	CPtBisector(const CPoint3D& pt)
	{
		x=pt.x;
		y=pt.y;
		z=pt.z;
	}

	const CPtBisector& operator=(const CPtBisector& bisPt_in)
	{
		bis=bisPt_in.bis;
		x=bisPt_in.x;
		y=bisPt_in.y;
		z=bisPt_in.z;

		return *this;
	}

	BOOL operator*(const CPtBisector& bisPt_in)  // Bisector intersection
	{
		return bis*bisPt_in.bis;
	}

public:
	CLine bis; // short for bisector
};
*/

class CPolyline::CBisector : public CPoint3D
{
public:
	CBisector(){}
	~CBisector(){}
	CBisector(const CBisector& bis)
	{
		x=bis.x;
		y=bis.y;
		z=bis.z;
		ptO=bis.ptO;
	}

	CBisector(const CPoint3D& pt)
	{
		x=pt.x;
		y=pt.y;
		z=pt.z;
	}

	void CreateO(CPoint3D* pPrev, CPoint3D* pNext, double offsetDist)
	{
		CVector3D vec1,vec2,vec;
		double angle;
		double dotProduct;

		vec1 = *pPrev - *this;
		vec2 = *pNext - *this;
		vec1.Normalize();
		vec2.Normalize();
		vec=vec1+vec2;
		vec.Normalize();
		if ((vec1*vec2).dz>0)
			vec=-vec;
		dotProduct=vec1|vec2;
		if (dotProduct>1)
			dotProduct=1;
		if (dotProduct<-1)
			dotProduct=-1;
		angle=acos(dotProduct);

		if (angle < 0.0000001)  // This if is added on 2013-6-14
		{
			vec=-vec1;
			ptO = (*this) + vec*offsetDist;
			return;
		}

		if (fabs(PI-angle)<TINY)
		{
			vec=GetOrthoVectorOnXYPlane(vec2);
			vec.Normalize();
			vec=vec*offsetDist;
		}
		else
		{
			vec=vec*(offsetDist/sin(angle/2));
		}

		ptO = (*this) + vec;
	}

	const CBisector& operator=(const CBisector& bis)
	{
		x=bis.x;
		y=bis.y;
		z=bis.z;
		ptO=bis.ptO;

		return *this;
	}

	BOOL operator*(const CBisector& bis)  // Bisector intersection
	{
		CVector3D v1 = ptO - (*this);
		CVector3D v2 = bis.ptO - bis;
		CPoint3D p1 = (*this);
		CPoint3D p2 = bis;
		double lamda1=((p2.x-p1.x)*v2.dy-(p2.y-p1.y)*v2.dx)/(v1.dx*v2.dy-v1.dy*v2.dx);
		double lamda2;
		if (0!=v2.dx)
			lamda2=(p1.x-p2.x+lamda1*v1.dx)/v2.dx;
		else
			lamda2=(p1.y-p2.y+lamda1*v1.dy)/v2.dy;

		if (lamda1>=ZERO && lamda1<=ONE && lamda2>=ZERO && lamda2<=ONE)  // 得到贺博指点，将这里改了 2011-8-2
			return TRUE;
		else
			return FALSE;
	}

public:
	CPoint3D ptO;  // this is the offset point
};


void CPolyline::CEvtSpace::LoadEvts(Point3DAry &ptList)
{
	int sz=ptList.GetSize();
	CEvt* pEvt=NULL;

	for (int i=0;i<sz-1;i++)
	{
		pEvt=new CEvt(*ptList[i],i);
        evtList.Add(pEvt);
	}

	sz=evtList.GetSize();

	if (*evtList[sz-1] < *evtList[0])
		evtList[0]->type_b=BOTTOM_END;
	else
		evtList[0]->type_b=TOP_END;
	if (*evtList[1] < *evtList[0])
		evtList[0]->type_f=BOTTOM_END;
	else
		evtList[0]->type_f=TOP_END;

	for (int i=1;i<sz-1;i++)
	{
		if (*evtList[i-1] < *evtList[i])
			evtList[i]->type_b=BOTTOM_END;
		else
			evtList[i]->type_b=TOP_END;
		if (*evtList[i+1] < *evtList[i])
			evtList[i]->type_f=BOTTOM_END;
		else
			evtList[i]->type_f=TOP_END;
	}

	if (*evtList[sz-2] < *evtList[sz-1])
		evtList[sz-1]->type_b=BOTTOM_END;
	else
		evtList[sz-1]->type_b=TOP_END;
	if (*evtList[0] < *evtList[sz-1])
		evtList[sz-1]->type_f=BOTTOM_END;
	else
		evtList[sz-1]->type_f=TOP_END;

	sz=evtList.GetSize();
	SortEvts(0,sz-1);

	// Debug
// 	CString str;
// 	CStdioFile file(_T("evt.txt"),CFile::modeCreate|CFile::modeWrite);
// 	for (int i=0;i<sz;i++)
// 	{
// 		str.Format(_T("i=%d  which point=%d x=%f,y=%f\n"),i,evtList[i]->i,evtList[i]->x,evtList[i]->y);
// 		file.WriteString(str);
// 	}
// 	file.Close();

	// Debug
}

void CPolyline::CEvtSpace::SortEvts(const int& s, const int& e) // QuickSort Algorithm can be found on Internet //
{
	if (s >= e) return;
	int i=s, j=e-1, sz=evtList.GetSize();
	CEvt tmp;

	do
	{     
		while(i<sz && *evtList[i] > *evtList[e]) i++; // DESCENDING_ORDER
		while(j>=0 && *evtList[j] < *evtList[e]) j--; // j>=0 to ensure Not to exceed bound
		if (i<j)
		{
			tmp = *evtList[i];
            *evtList[i] = *evtList[j];
			*evtList[j] = tmp;
		}
		else 
		{
			tmp = *evtList[i];
			*evtList[i] = *evtList[e];
			*evtList[e] = tmp;
		}
	} while(i<j);

	SortEvts(s, i-1);
	SortEvts(i+1, e);
}

// 二分法搜索插入，返回插入的位置 //
int CPolyline::CEvtSpace::InsertEvt(CEvt* pEvt)
{
	int sz=evtList.GetSize();
	int iBottom=0;     // 底部的点，最大的那个evt
	int iTop=sz-1; // 顶端的点，最小的那个点

	if (*pEvt < *evtList[iTop])  // 2011-7-13 注意 evt 数组顶端的是最小的
	{
		evtList.Add(pEvt);
		return sz;
	}

	if (*pEvt > *evtList[iBottom])
	{
		evtList.InsertAt(0,pEvt);
		return 0;
	}

	int k=(iBottom+iTop)/2;
	while (1!=iTop-iBottom)
	{
		if (*pEvt < *evtList[k])
		{
			iBottom=k;
			k=(iBottom+iTop)/2;
		}
		else
		{
			iTop=k;
			k=(iBottom+iTop)/2;
		}
	}

	evtList.InsertAt(iTop,pEvt);
	return iTop;
}

void CPolyline::CEvtSpace::RemoveEvt(int iWhich)
{
	delete evtList[iWhich];
	evtList.RemoveAt(iWhich);
}

// 移除掉线段后相邻线段有交点，返回 True，否则返回 False。有交点交点在 istPt 中，并且更新 evtSpace //
BOOL CPolyline::CStatusSpace::RemoveStatus(const int &iWhichSeg, IstPtAry &istPtList)  // iWhichSeg 是指要移除的线段在原始图形上的线段序号
{   // 这里是暴力搜索，
	int sz=statusList.GetSize();
	int i;
	for (i=0;i<sz;i++)
	{
		if (iWhichSeg==statusList[i]->iStatus) break;
	}

	if (i==sz) return FALSE;
	
	delete statusList[i];
	statusList.RemoveAt(i);

	if (0==i || statusList.GetSize()==i)
	{ // no need to calculate intersection //
        return FALSE;
	}
	else
	{
		int iS1=statusList[i-1]->iStatus; // iS1 and iS2 are the two intersecting segments' sequences //
		int iS2=statusList[i]->iStatus;
		if ( abs(iS1-iS2)==1 || abs(iS1-iS2)==iSzStutus-1) return FALSE;

		CPoint3D ptCross;
		if (::Is2SegsCross(ptCross,statusList[i-1]->m_pt1,statusList[i-1]->m_pt2,statusList[i]->m_pt1,statusList[i]->m_pt2))
		{
			if (ptCross.y >= yLine) return FALSE;

			CEvt* pEvt=new CEvt(ptCross,-1);
			pEvt->type_b=iS1;
			pEvt->type_f=iS2;
			evtSpace.InsertEvt(pEvt);

			CIstPt* pIstPt=new CIstPt(ptCross,pEvt->type_b,pEvt->type_f);
			if (pIstPt->m_s > pIstPt->m_b)
			{
				double tmp=pIstPt->m_s;
				pIstPt->m_s=pIstPt->m_b;
				pIstPt->m_b=tmp;
			}
			istPtList.Add(pIstPt);

			return TRUE;
		}
	}
	return FALSE;
}

// 返回值 TRUE 表示 pStatus 是和 扫描线平行的，否则不平行 //
BOOL CPolyline::CStatusSpace::InsertStatus(CStatus* pStatus, IstPtAry &istPtList) 
{
	int sz=statusList.GetSize();
	if (0==sz) return FALSE;
	CPoint3D ptCross;
	CIstPt* pIstPt=NULL;
	int iS1,iS2;  // 记录两条求交的线段在原始多边形上的序号

    // 处理插入线段平行于（或大致平行于）扫描线的情况
	if ( (float)pStatus->m_pt1.y == (float)pStatus->m_pt2.y) 
	{
		for (int i=0;i<sz;i++)
		{
			iS1=pStatus->iStatus;
			iS2=statusList[i]->iStatus;
			if ( abs(iS1-iS2)==1 || abs(iS1-iS2)==iSzStutus-1)  continue;
			if (::Is2SegsCross(ptCross,pStatus->m_pt1,pStatus->m_pt2,statusList[i]->m_pt1,statusList[i]->m_pt2))
			{
				pIstPt=new CIstPt(ptCross,pStatus->iStatus,statusList[i]->iStatus);
				if (pIstPt->m_s > pIstPt->m_b)
				{
					double tmp=pIstPt->m_s;
					pIstPt->m_s=pIstPt->m_b;
					pIstPt->m_b=tmp;
				}
				istPtList.Add(pIstPt);
			}
		}
		if (NULL!=pStatus)
		{
			delete pStatus; // 这种情况下要删除这个指针 //
			pStatus=NULL;
		}
		return TRUE;
	}

	// 以下是不平行的情况
	int iLeft=0;
	int iRight=sz-1;

	if (pStatus->IsRightOf(*statusList[iRight],yLine))  // 插入到最右边
	{
		statusList.Add(pStatus);
		iS1=statusList[iRight]->iStatus;
		iS2=statusList[iRight+1]->iStatus;
		if ( abs(iS1-iS2)==1 || abs(iS1-iS2)==iSzStutus-1)  return 0;
		if (::Is2SegsCross(ptCross,statusList[iRight]->m_pt1,statusList[iRight]->m_pt2,statusList[iRight+1]->m_pt1,statusList[iRight+1]->m_pt2))
		{
			if (ptCross.y>=yLine) return FALSE;

			CEvt* pEvt=new CEvt(ptCross,-1);
			pEvt->type_b=iS1;
			pEvt->type_f=iS2;
			evtSpace.InsertEvt(pEvt);

			pIstPt=new CIstPt(ptCross,iS1,iS2);
			if (pIstPt->m_s > pIstPt->m_b)
			{
				double tmp=pIstPt->m_s;
				pIstPt->m_s=pIstPt->m_b;
				pIstPt->m_b=tmp;
			}
			istPtList.Add(pIstPt);
		}
		return FALSE;
	}

	if (pStatus->IsLeftOf(*statusList[iLeft],yLine))  // 插入到最左边
	{
		statusList.InsertAt(0,pStatus);
		iS1=statusList[0]->iStatus;
		iS2=statusList[1]->iStatus;
		if ( abs(iS1-iS2)==1 || abs(iS1-iS2)==iSzStutus-1) return 0;
		if (::Is2SegsCross(ptCross,statusList[0]->m_pt1,statusList[0]->m_pt2,statusList[1]->m_pt1,statusList[1]->m_pt2))
		{
			if (ptCross.y>=yLine) return FALSE;

			CEvt* pEvt=new CEvt(ptCross,-1);
			pEvt->type_b=iS1;
			pEvt->type_f=iS2;
			evtSpace.InsertEvt(pEvt);

			pIstPt=new CIstPt(ptCross,iS1,iS2);
			if (pIstPt->m_s > pIstPt->m_b)
			{
				double tmp=pIstPt->m_s;
				pIstPt->m_s=pIstPt->m_b;
				pIstPt->m_b=tmp;
			}
			istPtList.Add(pIstPt);
		}
		return FALSE;
	}

	int k=(iLeft+iRight)/2;
	while (1!=iRight-iLeft)  // 1
	{
		if (pStatus->IsRightOf(*statusList[k],yLine))
		{
			iLeft=k;
			k=(iLeft+iRight)/2;
		}
		else
		{
			iRight=k;
			k=(iLeft+iRight)/2;
		}
	}

    statusList.InsertAt(iRight,pStatus);
	int state=0;
	iS1=statusList[iRight-1]->iStatus;
	iS2=statusList[iRight]->iStatus;
	if ( abs(iS1-iS2)!=1 && abs(iS1-iS2)!=iSzStutus-1 )  // 靠左边的
	{
		if (::Is2SegsCross(ptCross,statusList[iRight-1]->m_pt1,statusList[iRight-1]->m_pt2,statusList[iRight]->m_pt1,statusList[iRight]->m_pt2))
		{
			if (ptCross.y>=yLine) return FALSE;

			CEvt* pEvt=new CEvt(ptCross,-1);
			pEvt->type_b=iS1;
			pEvt->type_f=iS2;
			evtSpace.InsertEvt(pEvt);

			pIstPt=new CIstPt(ptCross,iS1,iS2);
			if (pIstPt->m_s > pIstPt->m_b)
			{
				double tmp=pIstPt->m_s;
				pIstPt->m_s=pIstPt->m_b;
				pIstPt->m_b=tmp;
			}
			istPtList.Add(pIstPt);
		}	
	}
	iS1=statusList[iRight]->iStatus;
	iS2=statusList[iRight+1]->iStatus;
	if ( abs(iS1-iS2)!=1 && abs(iS1-iS2)!=iSzStutus-1 )  // 靠右边的
	{
		if (::Is2SegsCross(ptCross,statusList[iRight]->m_pt1,statusList[iRight]->m_pt2,statusList[iRight+1]->m_pt1,statusList[iRight+1]->m_pt2))
		{
			if (ptCross.y>=yLine) return FALSE;

			CEvt* pEvt=new CEvt(ptCross,-1);
			pEvt->type_b=iS1;
			pEvt->type_f=iS2;
			evtSpace.InsertEvt(pEvt);

			pIstPt=new CIstPt(ptCross,iS1,iS2);
			if (pIstPt->m_s > pIstPt->m_b)
			{
				double tmp=pIstPt->m_s;
				pIstPt->m_s=pIstPt->m_b;
				pIstPt->m_b=tmp;
			}
			istPtList.Add(pIstPt);
		}	
	}

	return FALSE;
}

int CPolyline::CStatusSpace::SwapStatuses(const int& iSeg1, const int& iSeg2, IstPtAry &istPtList) // 输入的是在原始轮廓上的直线的序号，
{                                                                   // 先要查找出在状态空间对应的序号
	int sz=statusList.GetSize(); 
	int i,j;
	for (i=0;i<sz;i++)
	{
		if (iSeg1==statusList[i]->iStatus) break;
	}
	for (j=0;j<sz;j++)
	{
		if (iSeg2==statusList[j]->iStatus) break;
	}

	if (i==sz || j==sz) return 0;   // This is set to avoid mistake when the line is parallel to the sweep line //

    // To make i < j
	if (i>j)
	{
		int tmp=i;
		i=j;
		j=tmp;
	}

	CStatus sTmp=*statusList[i];
	*statusList[i]=*statusList[j];
	*statusList[j]=sTmp;

	CPoint3D ptCross;
	if (sz>=3)
	{
		if (0==i)
		{
			int iS1=statusList[j]->iStatus;
			int iS2=statusList[j+1]->iStatus;
			if ( abs(iS1-iS2)==1 || abs(iS1-iS2)==iSzStutus-1) return 0;
			if (::Is2SegsCross(ptCross,statusList[j]->m_pt1,statusList[j]->m_pt2,statusList[j+1]->m_pt1,statusList[j+1]->m_pt2))
			{
				if (ptCross.y >= yLine) return 0;
	
				CEvt* pEvt=new CEvt(ptCross,-1);
				pEvt->type_b=iS1;
				pEvt->type_f=iS2;
				evtSpace.InsertEvt(pEvt);

				CIstPt* pIstPt=new CIstPt(ptCross,iS1,iS2);
				if (pIstPt->m_s > pIstPt->m_b)
				{
					double tmp=pIstPt->m_s;
					pIstPt->m_s=pIstPt->m_b;
					pIstPt->m_b=tmp;
				}
				istPtList.Add(pIstPt);
				return 1;
			}
			return 0;
		}
		else if (sz-1==j)
		{
			int iS1=statusList[i-1]->iStatus;
			int iS2=statusList[i]->iStatus;
			if ( abs(iS1-iS2)==1 || abs(iS1-iS2)==iSzStutus-1) return 0;
			if (::Is2SegsCross(ptCross,statusList[i-1]->m_pt1,statusList[i-1]->m_pt2,statusList[i]->m_pt1,statusList[i]->m_pt2))
			{
				if (ptCross.y >= yLine) return 0;

				CEvt* pEvt=new CEvt(ptCross,-1);
				pEvt->type_b=iS1;
				pEvt->type_f=iS2;
				evtSpace.InsertEvt(pEvt);

				CIstPt* pIstPt=new CIstPt(ptCross,iS1,iS2);
				if (pIstPt->m_s > pIstPt->m_b)
				{
					double tmp=pIstPt->m_s;
					pIstPt->m_s=pIstPt->m_b;
					pIstPt->m_b=tmp;
				}
				istPtList.Add(pIstPt);
				return 1;
			}
			return 0;
		}
		else
		{
			int szR=istPtList.GetSize();
			int iS1=statusList[i-1]->iStatus;
			int iS2=statusList[i]->iStatus;
			if ( abs(iS1-iS2)!=1 && abs(iS1-iS2)!=iSzStutus-1 ) 
			{
				if (::Is2SegsCross(ptCross,statusList[i-1]->m_pt1,statusList[i-1]->m_pt2,statusList[i]->m_pt1,statusList[i]->m_pt2))
				{
					if (ptCross.y < yLine)
					{
						CEvt* pEvt=new CEvt(ptCross,-1);
						pEvt->type_b=iS1;
						pEvt->type_f=iS2;
						evtSpace.InsertEvt(pEvt);

						CIstPt* pIstPt=new CIstPt(ptCross,iS1,iS2);
						if (pIstPt->m_s > pIstPt->m_b)
						{
							double tmp=pIstPt->m_s;
							pIstPt->m_s=pIstPt->m_b;
							pIstPt->m_b=tmp;
						}
						istPtList.Add(pIstPt);
					}
				}	
			}
			iS1=statusList[j]->iStatus;
			iS2=statusList[j+1]->iStatus;
			if ( abs(iS1-iS2)!=1 && abs(iS1-iS2)!=iSzStutus-1 ) 
			{
				if (::Is2SegsCross(ptCross,statusList[j]->m_pt1,statusList[j]->m_pt2,statusList[j+1]->m_pt1,statusList[j+1]->m_pt2))
				{
					if (ptCross.y < yLine)
					{
						CEvt* pEvt=new CEvt(ptCross,-1);
						pEvt->type_b=iS1;
						pEvt->type_f=iS2;
						evtSpace.InsertEvt(pEvt);

						CIstPt* pIstPt=new CIstPt(ptCross,iS1,iS2);
						if (pIstPt->m_s > pIstPt->m_b)
						{
							double tmp=pIstPt->m_s;
							pIstPt->m_s=pIstPt->m_b;
							pIstPt->m_b=tmp;
						}
						istPtList.Add(pIstPt);
					}
				}	
			}
			return istPtList.GetSize()-szR;
		}
	}
	else
	{
		return 0;
	}

	return 0;
}

// ---------------自动添加到合适的子节点上---------------- //
// 递归遍历所有子节点，并且将 pIstPt添加到直系的父子节点上 //
BOOL CPolyline::CIstPt::Add(CIstPt* pIstPt)
{
	// 往包含 pIstPt 的节点上添加
	CArray<CPos,CPos&> posAry;
 	int curLayer=0;
 	CIstPt* pCur=NULL;
	CPos pos(0,1);
	posAry.Add(pos);
	int sz;
	BOOL bHasAdded=FALSE;
	pCur=this;

EXPAND:
	sz=pCur->pChildList.GetSize();
	if (0==sz)
	{   // Do some compare //
		if (*pIstPt<*pCur)
		{
			CIstPt* p=new CIstPt(*pIstPt);
			p->pFather=pCur;
			pCur->pChildList.Add(p);
			bHasAdded=TRUE;
		}
		if (*pIstPt>*pCur && *pIstPt<*pCur->pFather)
		{
			CIstPt* p=new CIstPt(*pCur);
			p->pFather=pCur;
			*pCur=*pIstPt;            // 这里是什么意思，看不懂了？？？ fuck 自己写的东西都看不懂
			pCur->pChildList.Add(p);  // 在 2011-6-3 修改过了，现在应该是完美了。
			bHasAdded=TRUE;
		}

LAST_LAYER:
		if (posAry[curLayer].i==posAry[curLayer].sz-1)
		{   // 返回上一层
			if (0==curLayer)
			{
				goto BREAK_OUT;
			}
			posAry.RemoveAt(curLayer);
			curLayer--;
			pCur=pCur->pFather;

			// Test(ed) to be OK, oh ,ahaha
			if (FALSE==bHasAdded)
			{
				if (*pIstPt<*pCur)
				{
					CIstPt* p=new CIstPt(*pIstPt);
					p->pFather=pCur;
					pCur->pChildList.Add(p);
					bHasAdded=TRUE;
				}
			}
			// Test

			goto  LAST_LAYER;
		}
		else
		{
			posAry[curLayer].i+=1;
			pCur=pCur->pFather->pChildList[posAry[curLayer].i];
		 	goto EXPAND;
		}
	}
	else
	{
		pos.i=0;
		pos.sz=sz;
		posAry.Add(pos);
		curLayer++;
		pCur=pCur->pChildList[0];
		goto EXPAND;
	}

BREAK_OUT:
    posAry.RemoveAll();
	if (bHasAdded)
	{
		return TRUE;
	}
	else
	{
        return FALSE;
	}
}


CPolyline::CPolyline()
{
	Init();
}

CPolyline::CPolyline(int n, CPoint3D pt,...)
{
	Init();

	CPoint3D* p = &pt;
	int sz=sizeof(CPoint3D);
	for (int i=0;i<n;i++)
	{
		AddPt(*p);
		p += 1;
	}	
}

void CPolyline::Init()
{
	m_nType=TYPE_POLYLINE;
	m_bIsClosed=-1;
	m_eveRt=NULL;
	m_pFather=NULL;
	m_offsetDist=10;
	m_level=0;
	m_length=-1.0;
	m_bIsUsed=FALSE;
	m_bIsDrawPt=false;
}

CPolyline::~CPolyline()
{
	m_sequOPs.RemoveAll();	

	if (NULL!=m_eveRt)
	{
		delete []m_eveRt;
		m_eveRt=NULL;
	}

	// Test
	int sz=m_ostPts.GetSize();
	for (int i=sz-1;i>=0;i--)
		delete m_ostPts[i];
	m_ostPts.RemoveAll();
	// Test
}

const CPolyline& CPolyline::operator=(const CPolyline& polyline)
{
	int sz=polyline.m_pts.GetSize();

	CPoint3D* p=NULL;
	for(int i=0;i<sz;i++)
	{
		p=new CPoint3D();
		*p=*polyline.m_pts[i];
		m_pts.Add(p);		
	}
	m_bIsClosed=polyline.m_bIsClosed;  

	return *this;
}

void CPolyline::SetIsDrawPt(bool bIsDrawPt)
{
	m_bIsDrawPt = bIsDrawPt;
}

void CPolyline::Draw(COpenGLDC* pDC,COLORREF clr)
{
	int sz=m_pts.GetSize();

	COLORREF oldClr;
	if (GetIsSelected())
		oldClr=pDC->SetMaterialColor(CLR_SELECTED);
	else
		oldClr=pDC->SetMaterialColor(clr);

	if (1==sz)
	{
		pDC->DrawPoint(*m_pts[0]);
		return;
	}

	m_bIsDrawPt = true;

	for (int i=0;i<sz-1;i++)
	{
		pDC->DrawLine(*m_pts[i],*m_pts[i+1],0,1,clr);

		//  if (m_bIsDrawPt)
		//pDC->DrawSelPoint(*m_pts[i]);
	}

	// 	if (m_bIsDrawPt)
	// 	{
	// 		pDC->DrawSelPoint(GetFirstPt(),ST_CIRCLE);
	// 		pDC->DrawSelPoint(*m_pts[1],ST_TRIANGLE);
	// 	}


	//pDC->DrawSelPoint(*m_pts[1823],ST_TRIANGLE,RGB(255,0,0));


	pDC->SetMaterialColor(oldClr);
}

void CPolyline::Move(const CVector3D &vec)
{
	int sz=m_pts.GetSize();
	for (int i=0;i<sz;i++)
	{
		(*m_pts[i]) += vec;
	}
}

void CPolyline::Rotate(double angle)
{
	int sz=m_pts.GetSize();
	CMatrix3D m=CMatrix3D::CreateRotateMatrix(angle,AXIS_Z);
	for (int i=0;i<sz;i++)
	{
		(*m_pts[i]) *= m;
	}
}

void CPolyline::Rotate(const CMatrix3D& m)
{
	int sz=m_pts.GetSize();
	for (int i=0;i<sz;i++)
	{
		(*m_pts[i]) *= m;
	}
}

void CPolyline::SetZLevel(double z)
{
	int sz=m_pts.GetSize();
	for (int i=0;i<sz;i++)
	{
		m_pts[i]->z=z;
	}
}

void CPolyline::Reverse()
{
	int sz=m_pts.GetSize();
	CPoint3D tmp;
	for (int i=0;i<sz/2;i++)
	{
		tmp = *m_pts[i];
		*m_pts[i] = *m_pts[sz-1-i];
		*m_pts[sz-1-i] = tmp;
	}
}

CPoint3D CPolyline::GetFirstPt() const
{
	return *m_pts[0];
}

CPoint3D CPolyline::GetLastPt() const
{
	int sz=m_pts.GetSize();
	return *m_pts[sz-1];
}

bool CPolyline::IsCircle(CPoint3D& ptCenter, double& radius) const
{
	int sz = m_pts.GetSize();
	if (sz<10)
	{
		return false;
	}

	double factor = 0.05;
	double x_min = FindExtreme(CPointSet::MIN_X);
	double x_max = FindExtreme(CPointSet::MAX_X);
	double y_min = FindExtreme(CPointSet::MIN_Y);
	double y_max = FindExtreme(CPointSet::MAX_Y);

	double zLevel = m_pts[0]->z;

	ptCenter=CPoint3D((x_min+x_max)/2, (y_min+y_max)/2, zLevel);

	radius = (*m_pts[0] - ptCenter).GetLength();
	double leni;
	for (int i=1;i<sz;i++)
	{
		leni = (*m_pts[i] - ptCenter).GetLength();
		if (fabs(leni - radius)/radius > factor)
		{
			return false;
		}
	}
	return true;
}

bool CPolyline::IsRect(CRect3D& rt, double precision/* =0.03 */) const
{
	if (!GetMinEnclosingRect(rt))
	{
		return false;
	}
	rt.MakeABLength();

	CRect3D MinRt = rt.Shrink(precision);
	CRect3D MaxRt = rt.Shrink(-precision);

	int sz=m_pts.GetSize();
	int cnt=0;
	for (int i=0;i<sz;i++)
	{
		if (!(MaxRt.IsPtInRect(*m_pts[i]) && MinRt.IsPtOutRect(*m_pts[i])))
		{
			return false;
		}
	}

	return true;
}

bool CPolyline::GetMinEnclosingRect(CRect3D& rt) const
{
	int sz=m_pts.GetSize();
	if (sz<4)
	{
		return false;
	}

	Point3DAry pts;
	GetConvexHull(pts);

	// 以下代码从我以前写的排样算法中截取
	sz=pts.GetSize();
	CPoint3D pt;     	                             // 求最小包络矩形并存储 S
	CPoint3D ptOrigin(0,0,0);
	double xMin,xMax,yMin,yMax;	//       考察该点和下一点组成的边,标记为当前点下标
	double S, SMin=HUGE, angle, anglePassToRect;
	CVector3D vecMove, vecPassToRect;
	CPoint3D ptLeftTop,ptLeftBottom,ptRightBottom,ptRightTop;
	CMatrix3D matrixRotate;

	CVector3D vec=*pts[1]-*pts[0];      // 第一条边，是点 sz-1 到 0 点。。。主要作用是给SMin初值
	for(int j=0;j<sz-1;j++)
	{
		vec=*pts[j+1]-*pts[j];
		angle=GetAngle(vec);  // 0-360
		vecMove=*pts[j]-ptOrigin;
		matrixRotate=CMatrix3D::CreateRotateMatrix(-angle,AXIS_Z);
		xMin=yMin=HUGE;
		xMax=yMax=-HUGE;
		for(int k=0;k<sz;k++)
		{
			pt=*pts[k]-vecMove;
			pt*=matrixRotate;
			if(pt.x<xMin) xMin=pt.x;
			if(pt.x>xMax) xMax=pt.x;
			if(pt.y<yMin) yMin=pt.y;
			if(pt.y>yMax) yMax=pt.y;
		}
		S=(yMax-yMin)*(xMax-xMin);
		if(S<SMin) 
		{
			SMin=S;
			anglePassToRect=angle;
			vecPassToRect=vecMove;
			ptLeftTop=CPoint3D(xMin,yMax,0);
			ptLeftBottom=CPoint3D(xMin,yMin,0);
			ptRightTop=CPoint3D(xMax,yMax,0);
			ptRightBottom=CPoint3D(xMax,yMin,0);
		}
	}
	matrixRotate=CMatrix3D::CreateRotateMatrix(anglePassToRect,AXIS_Z);
	rt.A=ptLeftBottom*matrixRotate+vecPassToRect;
	rt.B=ptRightBottom*matrixRotate+vecPassToRect;
	rt.C=ptRightTop*matrixRotate+vecPassToRect;
	rt.D=ptLeftTop*matrixRotate+vecPassToRect;

	for (int i=0;i<sz;i++)
	{
		delete pts[i];
		pts[i]=NULL;
	}
	pts.RemoveAll();

	return true;
}

double CPolyline::GetLength()
{
	m_length=0.0;
	CVector3D vec;
	int sz=GetSize();
	for (int i=0;i<sz-1;i++)
	{
		vec= *m_pts[i+1] - *m_pts[i];
		m_length+=vec.GetMod();
	}
	return m_length;
}

double CPolyline::GetArea()
{
	double area=0;
	int sz = m_pts.GetSize();
	for(int i=0;i<=sz-2;i++)
	{
		area+=0.5*(m_pts[i]->x*m_pts[i+1]->y - m_pts[i+1]->x*m_pts[i]->y);
	}
	return area;
}

void CPolyline::MakeFirstAt(int at_i)
{
	if (0==at_i) return;
	int sz=m_pts.GetSize();
	if (at_i<0 || at_i>sz-1) return;

	CPoint3D* pt=new CPoint3D[sz];
	for (int i=at_i;i<sz;i++)
		pt[i-at_i]=*m_pts[i];
	for (int i=1;i<=at_i;i++)
		pt[sz-1+i-at_i]=*m_pts[i];
	for (int i=0;i<sz;i++)
		*m_pts[i]=pt[i];
	delete []pt;
}

void CPolyline::MakeFirstAt(Point3DAry &pts, int at_i)
{
	if (0==at_i) return;
	int sz=pts.GetSize();
	if (at_i<0 || at_i>sz-1) return;

	CPoint3D* pt=new CPoint3D[sz];
	for (int i=at_i;i<sz;i++)
		pt[i-at_i]=*pts[i];
	for (int i=1;i<=at_i;i++)
		pt[sz-1+i-at_i]=*pts[i];
	for (int i=0;i<sz;i++)
		*pts[i]=pt[i];
	delete []pt;
}

BOOL CPolyline::IsCrossWith(const CPoint3D& pt1, const CPoint3D& pt2)
{
	CLine line1(pt1,pt2);
	int sz=m_pts.GetSize();
	for (int i=0;i<sz-1;i++)
	{
		CLine line2(*m_pts[i],*m_pts[i+1]);
		if (line1*line2)
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CPolyline::IsCrossWith(const CLine& line)
{
	int sz=m_pts.GetSize();
	for (int i=0;i<sz-1;i++)
	{
		CLine line2(*m_pts[i],*m_pts[i+1]);
		if (line*line2)
		{
			return TRUE;
		}
	}

	return FALSE;
}

bool CPolyline::IsPtInPolygon(const CPoint3D& pt) const
{
	if (IsPtOnPolygon(pt))
	{
		return false;
	}

	CPoint3D ptFaraway(LARGE,pt.y,pt.z);
	int countCrossPoint=0;

    int sz=m_pts.GetSize();
	for (int i=0;i<=sz-2;i++)
	{
		if ((m_pts[i]->y - pt.y)*(m_pts[i+1]->y - pt.y)<0)
		{
			if (Is2SegsCross(pt,ptFaraway,*m_pts[i],*m_pts[i+1]))
			{
				countCrossPoint++;
			}
		}
	}
 
	double prev_y, next_y;
	for (int i=0;i<=sz-2;i++)
	{
		if (m_pts[i]->y==pt.y && m_pts[i]->x > pt.x)
		{
			if (0==i)
			{
				prev_y = m_pts[sz-2]->y;
				next_y = m_pts[1]->y;
			}
			else
			{
				prev_y = m_pts[i-1]->y;
				next_y = m_pts[i+1]->y;
			}

			if ((prev_y - pt.y)*(next_y - pt.y)<0)
			{
				countCrossPoint++;
			}
		}
	}
	
	if(1==countCrossPoint%2)
		return true;
	else
		return false;
}

bool CPolyline::IsPtOnPolygon(const CPoint3D& pt) const
{
	CVector3D v1, v2;
	CVector3D zero_vector(0,0,0);
	int sz = m_pts.GetSize();
	for (int i=0;i<sz-1;i++)
	{
		v1 = *m_pts[i] - pt;
		v2 = *m_pts[i+1] - pt;

		if (v1*v2 == zero_vector)
		{
			return true;
		}
	}
	return false;
}

BOOL CPolyline::IsPolygonCCW()
{
	int sz=m_pts.GetSize();

	if (sz<=3)      // 一条线的时候（3个点）认为是顺时针的，啊哈哈哈...
	{
		return FALSE;
	}

	int ibtm=0;
	double yMin=m_pts[0]->y;
	for (int j=0;j<sz;j++)
	{
		if (m_pts[j]->y<yMin)
		{
			ibtm=j;
			yMin=m_pts[j]->y;
		}
	}

	CVector3D vec1;
	CVector3D vec2;
	double r=0;
	while (TRUE)
	{
		if (sz-1==ibtm || 0==ibtm)
		{
			vec1=*m_pts[0]-*m_pts[sz-2];
			vec2=*m_pts[1]-*m_pts[0];
		}
		else
		{
			vec1=*m_pts[ibtm]-*m_pts[ibtm-1];
			vec2=*m_pts[ibtm+1]-*m_pts[ibtm];
		}

		r=(vec1*vec2).dz;
		if (0==r)
		{
			ibtm++;
			if (sz==ibtm)
			{
				ibtm=1;
			}
			continue;
		}
		else
		{
			if (r>0)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}
}

void CPolyline::MakePolygonCCW()
{
	if (!IsPolygonCCW())
	{
		int sz=m_pts.GetSize();
		CPoint3D* pt=new CPoint3D[sz];
		for (int i=0;i<sz;i++)
		{
			pt[i]=*m_pts[sz-1-i];
		}
		for (int i=0;i<sz;i++)
		{
			*m_pts[i]=pt[i];
		}
		delete []pt;
	}
}

void CPolyline::MakePolygonCW()
{
	if (IsPolygonCCW())
	{
		int sz=m_pts.GetSize();
		CPoint3D* pt=new CPoint3D[sz];
		for (int i=0;i<sz;i++)
		{
			pt[i]=*m_pts[sz-1-i];
		}
		for (int i=0;i<sz;i++)
		{
			*m_pts[i]=pt[i];
		}
		delete []pt;
	}
}

void CPolyline::DelCoincidePt(double dist_resolution /* = POINT_RESOLUTION */)
{
	int i=0;
	CVector3D vec;
	double dist2;
	while (i<m_pts.GetSize()-1)
	{
		vec = *m_pts[i] - *m_pts[i+1];
		dist2 = vec.GetMod2();
		if (dist2 < dist_resolution)
		{
			delete m_pts[i+1];
			m_pts.RemoveAt(i+1);
		}
		else
		{
			i++;
		}
	}
}


void CPolyline::DelColinearPt(double angle_resolution /* = 1.0e-5 */ )
{
	if (GetFirstPt() ^= GetLastPt())
	{	
		CVector3D vec1,vec2;
		double angle = 0.0;
		int sz0 = m_pts.GetSize();
		delete m_pts[sz0-1];
		m_pts.RemoveAt(sz0-1);

		int prev_i, next_i, i=0, cnt=0, pos=0, sz;
		while (true)
		{
			cnt++;

			sz = m_pts.GetSize();

			if (0==sz)
			{
				return;
			}

			i = O(i,sz);
			prev_i = O(i-1,sz);
			next_i = O(i+1,sz);

			if (*m_pts[prev_i] ^= *m_pts[i])
			{
				delete m_pts[prev_i];
				m_pts.RemoveAt(prev_i);
				continue;
			}

			vec1 = *m_pts[prev_i] - *m_pts[i];
			vec2 = *m_pts[next_i] - *m_pts[i];
			angle = GetAngle(vec1,vec2);
			if (angle < angle_resolution || fabs(angle-PI) < angle_resolution)
			{
				delete m_pts[i];
				m_pts.RemoveAt(i);

				pos = cnt;
			}
			else
			{
				i++;

				if (cnt - pos >= 2*m_pts.GetSize())
				{
					break;
				}
			}
		}
		AddPt(GetFirstPt());
	}
	else
	{
		int i=0; 
		CVector3D vec1,vec2;
		double z;

		i=1;
		while (i<m_pts.GetSize()-1)  
		{	// 不要去处理第一个点和最后一点是直线的情况
			vec1=*m_pts[i-1]-*m_pts[i];
			vec2=*m_pts[i+1]-*m_pts[i];
			z=(vec1*vec2).dz;
			if (z<0) z=-z;	
			if (z<TINY)
			{
				delete m_pts[i];
				m_pts.RemoveAt(i);
			}
			else
			{
				i++;
			}
		}
	}
}

int CPolyline::IsThereStuckCircle(const CPoint3D& S1, const CPoint3D& E1, const CPoint3D& S2, const CPoint3D& E2, CLine& seg_out)
{ 
	if (CalStuckCircle(S1,E1,S2,E2,seg_out))
	{
		if (GetDist2PtToSeg(seg_out.m_pt2,E1,S2)+TINY>=m_offsetDist*m_offsetDist)
	    	return TRUE;
	}

	CLine seg1,seg2;
	CalStuckCircle(S1,E1,E1,S2,seg1);
	CalStuckCircle(E1,S2,S2,E2,seg2);
	seg_out.m_pt1=seg1.m_pt1;
	seg_out.m_pt2=seg2.m_pt1;
	return FALSE;
}

BOOL CPolyline::CalStuckCircle(const CPoint3D& S1, const CPoint3D& E1, const CPoint3D& S2, const CPoint3D& E2, CLine& seg_out)
{
	CVector3D vec1,vec2,v1,v2,v;
	CPoint3D ptCross,ptO;
	double dotProduct,angle,lamda1,lamda2,dis2;
	dis2=m_offsetDist*m_offsetDist;
	v1=vec1=S1-E1;
	v2=vec2=E2-S2;

	// (1) both points are tangtial circle
	if (E1==S2) // 
	{
		if ((v1*v2).dz>=0)        // If the inner angle is > Pi, then return false. Here false means
		{                         // these two segments need not to update.If it needs to be updated,
			seg_out.m_pt1=E1; // return true.
			return FALSE;   
		}
		else
		{
		    ptCross=E1;
		}
	}
	else
	{
        if (0==GetCrossPoint(ptCross,S1,E1,S2,E2)) 
			goto Situations2_4;
	}
	v1=S1-ptCross;
	v2=E2-ptCross;
	v1.Normalize();
	v2.Normalize();
	v=v1+v2;
	v.Normalize();
	dotProduct=v1|v2;
	if (dotProduct>1)  dotProduct=1;
	if (dotProduct<-1) dotProduct=-1;
	angle=acos(dotProduct);
	ptO=ptCross+v*(m_offsetDist/sin(angle/2));
	if (GetDist2PtToSeg(ptO,E1,S2)+TINY>dis2)
	{
		lamda1=((ptO-E1)|vec1)/vec1.GetMod2();
		lamda2=((ptO-S2)|vec2)/vec2.GetMod2();
		if (lamda1>ZERO && lamda1<=ONE && lamda2>ZERO && lamda2<=ONE)
		{
			seg_out.m_pt1=ptCross;
			seg_out.m_pt2=ptO;
			return TRUE;
		}
	}

Situations2_4:
	CVector3D n,t,vec;
	double lamda,a,b,c,delta;

	// (2) tangent to segment Pi-1Pi，supported by point Pi+2
	n=GetOrthoVectorOnXYPlane(S1,E1);
	if (IsPtOnRightOfSeg(E2,S1,E1)) n=-n;
	n.Normalize();
	t=E1-E2+n*m_offsetDist;
	a=vec1.GetMod2();
	b=2*(t|vec1);
	c=t.GetMod2()-dis2;
	delta=b*b-4*a*c;
	if (delta>=0)
	{
		lamda=(-b+sqrt(delta))/(2.0*a);
		if (lamda>ZERO && lamda<=ONE)
		{
			ptO=E1+vec1*lamda+n*m_offsetDist;
			if (GetDist2PtToSeg(ptO,S2,E2)+TINY>=dis2) 
			{
				vec=GetOrthoVectorOnXYPlane(ptO,E2);
				CPoint3D ptT=E2+vec;
				GetCrossPoint(seg_out.m_pt1,E2,ptT,S1,E1);
				seg_out.m_pt2=ptO;
				return TRUE;
			}
		}
	}

	// (3) tangent to segment Pi+1Pi+2，supported by point Pi-1
	n=GetOrthoVectorOnXYPlane(S2,E2);
	if (IsPtOnRightOfSeg(S1,S2,E2)) n=-n;
	n.Normalize();
	t=S2-S1+n*m_offsetDist;
	a=vec2.GetMod2();
	b=2.0*(t|vec2);
	c=t.GetMod2()-dis2;
	delta=b*b-4*a*c;
	if (delta>=0)
	{
		lamda=(-b+sqrt(delta))/(2.0*a);
		if (lamda>ZERO && lamda<=ONE)
		{
			ptO=S2+vec2*lamda+n*m_offsetDist;
			if (GetDist2PtToSeg(ptO,S1,E1)+TINY>=dis2)
			{
				vec=GetOrthoVectorOnXYPlane(ptO,S1);
				CPoint3D ptT=S1+vec;
				GetCrossPoint(seg_out.m_pt1,S1,ptT,S2,E2);
				seg_out.m_pt2=ptO;
				return TRUE;
			}
		}
	}

	// (4) supported both by the two points
	vec=E2-S1;
	double len=vec.GetMod2();
	if (len<4*dis2)
	{
		n=GetOrthoVectorOnXYPlane(S1,E2);
		n.Normalize();
		ptO=(S1+E2)*0.5+n*sqrt(dis2-len/4);
		if (GetDist2PtToSeg(ptO,S1,E1)+TINY<dis2) return FALSE;
		if (GetDist2PtToSeg(ptO,S2,E2)+TINY<dis2) return FALSE;
		vec1=GetOrthoVectorOnXYPlane(ptO,S1);
		vec2=GetOrthoVectorOnXYPlane(ptO,E2);
		GetCrossPoint(seg_out.m_pt1,S1,S1+vec1,E2,E2+vec2);
		seg_out.m_pt2=ptO;
		return TRUE;
	}

	return FALSE;
}

BOOL CPolyline::IsSegIstOtherSegs(const int& iWhichSeg, Point3DAry &pts)
{
	int szPt=pts.GetSize();

    if (0==iWhichSeg || szPt-1==iWhichSeg)
    {
		for (int i=2;i<=szPt-3;i++)
		{
			if (Is2SegsCross(*pts[0],*pts[1],*pts[i],*pts[i+1]))
			{
				return TRUE;
			}
		}
    }
	else if (szPt-2==iWhichSeg)
	{
		for (int i=1;i<=szPt-4;i++)
		{
			if (Is2SegsCross(*pts[iWhichSeg],*pts[iWhichSeg+1],*pts[i],*pts[i+1]))
			{
				return TRUE;
			}
		}
	}
	else
	{
		for (int i=0;i<=szPt-2;i++)
		{
			if (iWhichSeg-1==i || iWhichSeg==i || iWhichSeg+1==i)
				continue;
			if (Is2SegsCross(*pts[iWhichSeg],*pts[iWhichSeg+1],*pts[i],*pts[i+1]))
			{
				return TRUE;
			}
		}

	}

	return FALSE;
}

void CPolyline::FullOffset(const double& dis)
{
	if (dis<=0 || Round(dis)==0) return;

	int sz;
	CArray<CPos,CPos&> posAry;
	int curLayer=0;
	CPolyline* pTemp=NULL;
	CPolyline* pCur=this;
	pCur->m_level=0;      // store the depth of the current polyline
	pCur->m_pFather=NULL;   // root
	CPos pos(0,1);
	posAry.Add(pos);

EXPAND_FULL_OFFSET:
	pCur->Offset(dis);

	sz=pCur->m_treeOPs.GetSize();    // 线性的记录所有的偏置曲线
	if (0==sz)
	{
		pCur->m_level=-pCur->m_level;
	}
	else
	{
		for (int i=0;i<sz;i++)
		{
			pCur->m_treeOPs[i]->m_level=curLayer+1;
			m_sequOPs.Add(pCur->m_treeOPs[i]);
		}
	}

	if (sz>0)
	{ // Go down
		pos.i=0;
		pos.sz=sz;
		posAry.Add(pos);
		curLayer++;
		pTemp=pCur;
		pCur=pCur->m_treeOPs[0];
		pCur->m_pFather=pTemp;
		pTemp=NULL;
		goto EXPAND_FULL_OFFSET;
	}
	else
	{
LAST_LAYER_FULL_OFFSET:
		if (posAry[curLayer].i==posAry[curLayer].sz-1)
		{ // Back to last layer
			if (0==curLayer)
			{
				posAry.RemoveAll();
				return;
			}
			posAry.RemoveAt(curLayer);
			curLayer--;
			pCur=pCur->m_pFather;
			goto LAST_LAYER_FULL_OFFSET;
		}
		else
		{
			posAry[curLayer].i++;
			pTemp=pCur->m_pFather;
			pCur=pCur->m_pFather->m_treeOPs[posAry[curLayer].i];
			pCur->m_pFather=pTemp;
			pTemp=NULL;
			goto EXPAND_FULL_OFFSET;
		}
	}
}

void CPolyline::Offset_silly(const double& dis)
{
	// Test
	int sz=m_ostPts.GetSize();
	for (int i=sz-1;i>=0;i--)
		delete m_ostPts[i];
	m_ostPts.RemoveAll();
	// Test

	//MergeNbSegs();

	sz=m_pts.GetSize();
	CTypedPtrArray<CObArray,CPoint3D*> ptList;
	CPoint3D* pPt=NULL;
	CVector3D vec1,vec2,vec;
	double dotProduct=0;
	double angle;

	for(int i=0;i<sz;i++)
	{
		if (0==i)
		{
			vec1=*m_pts[sz-2]-*m_pts[0];
			vec2=*m_pts[1]-*m_pts[0];
		}
		else if (sz-1==i)
		{
			vec1=*m_pts[sz-2]-*m_pts[sz-1];
			vec2=*m_pts[1]-*m_pts[sz-1];
		}
		else
		{
			vec1=*m_pts[i-1]-*m_pts[i];
			vec2=*m_pts[i+1]-*m_pts[i];
		}
		vec1.Normalize();
		vec2.Normalize();
		vec=vec1+vec2;
		vec.Normalize();


		dotProduct=vec1|vec2;
		if (dotProduct>1)
			dotProduct=1;
		if (dotProduct<-1)
			dotProduct=-1;

		angle=acos(dotProduct);

		if (Round(angle)==Round(PI))
		{
			vec=GetOrthoVectorOnXYPlane(vec2).Normalize()*dis;
		}
		else if (angle<0.0000001)
		{
			vec=-vec1;
			vec=vec*dis;
		}
		else
		{
			if ((vec2*vec1).dz<0)
				vec=vec*(-1);
			vec=vec*(dis/sin(angle/2));
		}

		pPt=new CPoint3D(*m_pts[i]+vec);
		m_ostPts.Add(pPt);
	}
}

void CPolyline::Offset_silly(double dis, CPolyline *& pPolyline)
{
	int sz=m_pts.GetSize();
	CPoint3D* pPt=NULL;
	CVector3D vec1,vec2,vec;
	double dotProduct=0;
	double angle;

	for(int i=0;i<sz-1;i++)
	{
		if (0==i)
		{
			vec1=*m_pts[sz-2]-*m_pts[0];
			vec2=*m_pts[1]-*m_pts[0];
		}
		else
		{
			vec1=*m_pts[i-1]-*m_pts[i];
			vec2=*m_pts[i+1]-*m_pts[i];
		}
		vec1.Normalize();
		vec2.Normalize();
		vec=vec1+vec2;
		vec.Normalize();
		angle = Angle(vec1,vec2);

		if (Round(angle)==Round(PI))
		{
			vec=GetOrthoVectorOnXYPlane(vec2).Normalize()*dis;
		}
		else if (angle<0.0000001)   // ???
		{
			vec=-vec1;
			vec=vec*dis;
		}
		else
		{
			if ((vec2*vec1).dz<0)
				vec=vec*(-1);
			vec=vec*(dis/sin(angle/2));
		}

		pPt=new CPoint3D(*m_pts[i]+vec);
		pPolyline->AddPt(pPt);
	}
	pPolyline->AddPt(pPolyline->GetFirstPt());

	pPolyline->DelCoincidePt();
	pPolyline->DelColinearPt();
}

void CPolyline::Offset(const double& dist)
{
	std::vector<CPolyline*> polylines;
	AzClipper::Offset(this,polylines,-dist);

	int sz = polylines.size();
	for (int i=0;i<sz;i++)
	{
		m_treeOPs.Add(polylines[i]);
	}


/*
	if (dist==0)
	{
		return;
	}

	m_offsetDist=dist;

	DelColinearPt();

	if (m_pts.GetSize()<4)
		return;

	CXAlignRect rt=CalAABBRect();
	if (rt.GetMinBorder()<2*dist)
		return;

	Point3DAry pts;                 // Temp array

	if (FALSE==RawOffset(pts))
		goto Release_mem_Offset;     // RawOffset 

	if (pts.GetSize()<4)
		goto Release_mem_Offset;

	TrimRawOffset(pts);

	TreeAnalyse(pts);                  // 树结构分析去除全局环

	int szOstLoopList=m_treeOPs.GetSize();
	for (int i=szOstLoopList-1;i>=0;i--)  // 删除顺时针的圈
	{
		if (!m_treeOPs[i]->IsPolygonCCW())
		{
			delete m_treeOPs[i];
			m_treeOPs[i]=NULL;
			m_treeOPs.RemoveAt(i);
		}
		else
		{
			m_treeOPs[i]->DelCoincidePt();
			m_treeOPs[i]->DelColinearPt();
		}
	}

Release_mem_Offset:
	int sz=pts.GetSize();  // 清除内存
	for (int i=sz-1;i>=0;i--)
	{
		delete pts[i];
		pts[i]=NULL;
	}
	pts.RemoveAll();
	*/
}

bool CPolyline::RawOffset(double dist, CPolyline *& pOffsetPolygon)
{
	if (dist==0)
	{
		return false;
	}

	m_offsetDist=dist;

	DelColinearPt();

	if (m_pts.GetSize()<4)
		return false;

	CXAlignRect rt=CalAABBRect();
	if (rt.GetMinBorder()<2*dist)
		return false;

	Point3DAry pts;                 // Temp array
	if (FALSE==RawOffset(pts) || pts.GetSize()<4)
	{
		int sz=pts.GetSize();  // 清除内存
		for (int i=sz-1;i>=0;i--)
		{
			delete pts[i];
			pts[i]=NULL;
		}
		pts.RemoveAll();
		return false;
	}
	else
	{
		int sz = pts.GetSize();
		for (int i=0;i<sz;i++)
		{
			pOffsetPolygon->AddPt(pts[i]);
		}
		pts.RemoveAll();
		return true;
	}
}

void CPolyline::Offset(double dis_in, CPolylineSet* pPolylineSet)
{
	Offset(dis_in);
	int sz=m_treeOPs.GetSize();
	for (int i=0;i<sz;i++)
	{
		pPolylineSet->m_polylineList.Add(m_treeOPs[i]);
	}
}

BOOL CPolyline::RawOffset(Point3DAry &ptList_raw)
{
	int sz=m_pts.GetSize(); // sz>=3
	if (sz<4) return FALSE;

	CTypedPtrArray<CObArray,CBisector*> bises;        // 复制数组并计算角平分线 //
	for (int i=0;i<sz-1;i++)                          // 注意，最后一个点没有复制，复制出来的东西第一点和最后一点是不重合的
	bises.Add(new CBisector(*m_pts[i]));

	sz=bises.GetSize();
	bises[0]->CreateO(bises[sz-1],bises[1],m_offsetDist);
	bises[sz-1]->CreateO(bises[sz-2],bises[0],m_offsetDist);
	for (int i=1;i<=sz-2;i++)
	bises[i]->CreateO(bises[i-1],bises[i+1],m_offsetDist);

	int nCount=0;
	int nLastCount=0;

	CLine bis;
	CPoint3D ptMem; // The memorized point of Pi+1
	int k=0;
	while (TRUE)
	{
		nCount++;
		sz=bises.GetSize();
		if (sz<=2) break;
		if (nCount-nLastCount>=sz+10) break;

		k=O(k,sz);
		if ((*bises[O(k,sz)])*(*bises[O(k+1,sz)]))
		{
			ptMem=*bises[O(k+1,sz)];
			if (IsThereStuckCircle(*bises[O(k-1,sz)],*bises[O(k,sz)],*bises[O(k+1,sz)],*bises[O(k+2,sz)],bis))
			{
				*bises[O(k+1,sz)]=bis.m_pt1;
				bises[O(k+1,sz)]->ptO=bis.m_pt2;
				bises[O(k-1,sz)]->CreateO(bises[O(k-2,sz)],bises[O(k+1,sz)],m_offsetDist);
				delete bises[O(k,sz)];
				bises.RemoveAt(O(k,sz));
				sz=bises.GetSize();
				if (sz<=2) break;
				while (sz>=4 && IsPtOnRightOf3Segs(*bises[O(k+2,sz)],*bises[O(k-1,sz)],*bises[O(k,sz)],*bises[O(k+1,sz)],ptMem))
				{
					delete bises[O(k+2,sz)];
					bises.RemoveAt(O(k+2,sz));
					sz=bises.GetSize();
					if (sz==k) k--;
				}
				bises[O(k+1,sz)]->CreateO(bises[O(k,sz)],bises[O(k+2,sz)],m_offsetDist);
			}
			else
			{
				*bises[O(k,sz)]=bis.m_pt1;
				*bises[O(k+1,sz)]=bis.m_pt2;
				bises[O(k-1,sz)]->CreateO(bises[O(k-2,sz)],bises[O(k,sz)],m_offsetDist);
				bises[O(k,sz)]->CreateO(bises[O(k-1,sz)],bises[O(k+1,sz)],m_offsetDist);
				bises[O(k+1,sz)]->CreateO(bises[O(k,sz)],bises[O(k+2,sz)],m_offsetDist);
				while (sz>=4 && IsPtOnRightOf4Segs(*bises[O(k+3,sz)],*bises[O(k-1,sz)],*bises[O(k,sz)],*bises[O(k+1,sz)],*bises[O(k+2,sz)],ptMem))
				{
					delete bises[O(k+3,sz)];
					bises.RemoveAt(O(k+3,sz));
					sz=bises.GetSize();
					if (sz==k) k--;
				}
				bises[O(k+2,sz)]->CreateO(bises[O(k+1,sz)],bises[O(k+3,sz)],m_offsetDist);
			}
			k=k-2;
			nLastCount=nCount;
		}
		else
		{
			k++;
		}
	}


	sz=bises.GetSize();
	if (sz>2)
	{
		CPoint3D* pPt=NULL;
		for (int i=0;i<sz;i++)
		{
			pPt=new CPoint3D(bises[i]->ptO);
			ptList_raw.Add(pPt);
		}

		pPt=new CPoint3D(*ptList_raw[0]);
		ptList_raw.Add(pPt);

		for (int i=sz-1;i>=0;i--)
			delete bises[i];
		bises.RemoveAll();

		return TRUE;
	}
	else
	{
		for (int i=sz-1;i>=0;i--)
			delete bises[i];
		bises.RemoveAll();

		return FALSE;
	}
}


BOOL CPolyline::RawOffset_Demo(double offsetDist, int nStopAt, BOOL bOutputOffset)
{
	m_offsetDist=offsetDist;
	if (offsetDist<0)
	{
		MakePolygonCW();
		offsetDist=-offsetDist;
	}
	if (offsetDist==0)
	{
		return FALSE;
	}

	int sz=m_pts.GetSize(); // sz>=3
	if (sz<4) return FALSE;

	CTypedPtrArray<CObArray,CBisector*> bises;        // 复制数组并计算角平分线 //
	for (int i=0;i<sz-1;i++)                          // 注意，最后一个点没有复制，复制出来的东西第一点和最后一点是不重合的
		bises.Add(new CBisector(*m_pts[i]));

	MakePolygonCCW();

	sz=bises.GetSize();
	bises[0]->CreateO(bises[sz-1],bises[1],offsetDist);
	bises[sz-1]->CreateO(bises[sz-2],bises[0],offsetDist);
	for (int i=1;i<=sz-2;i++)
		bises[i]->CreateO(bises[i-1],bises[i+1],offsetDist);

	// 精华部分，重组轮廓 //
	int nCount=0;
	int nLastCount=0;
	CLine bis;
	CPoint3D ptMem; // The memorized point of Pi+1
	int k=0;
	while (TRUE)
	{
		nCount++;
		sz=bises.GetSize();
		if (sz<=2) break;
		if (nCount-nLastCount>=sz+10) break;

		if (nStopAt==nCount) break;

		if (nCount==nStopAt-1)
		{
			int x=1+1;
		}

		//////////////////////////////////////////////////////////////////////////
		k=O(k,sz);
		if ((*bises[O(k,sz)])*(*bises[O(k+1,sz)]))
		{
			ptMem=*bises[O(k+1,sz)];
			if (IsThereStuckCircle(*bises[O(k-1,sz)],*bises[O(k,sz)],*bises[O(k+1,sz)],*bises[O(k+2,sz)],bis))
			{
				*bises[O(k+1,sz)]=bis.m_pt1;
				bises[O(k+1,sz)]->ptO=bis.m_pt2;
				bises[O(k-1,sz)]->CreateO(bises[O(k-2,sz)],bises[O(k+1,sz)],offsetDist);
				delete bises[O(k,sz)];
				bises.RemoveAt(O(k,sz));
				sz=bises.GetSize();
				if (sz<=2) break;
				while (sz>=4 && IsPtOnRightOf3Segs(*bises[O(k+2,sz)],*bises[O(k-1,sz)],*bises[O(k,sz)],*bises[O(k+1,sz)],ptMem))
				{
					delete bises[O(k+2,sz)];
					bises.RemoveAt(O(k+2,sz));
					sz=bises.GetSize();
					if (sz==k) k--;
				}
				bises[O(k+1,sz)]->CreateO(bises[O(k,sz)],bises[O(k+2,sz)],offsetDist);
			}
			else
			{
				*bises[O(k,sz)]=bis.m_pt1;
				*bises[O(k+1,sz)]=bis.m_pt2;
				bises[O(k-1,sz)]->CreateO(bises[O(k-2,sz)],bises[O(k,sz)],offsetDist);
				bises[O(k,sz)]->CreateO(bises[O(k-1,sz)],bises[O(k+1,sz)],offsetDist);
				bises[O(k+1,sz)]->CreateO(bises[O(k,sz)],bises[O(k+2,sz)],offsetDist);
				while (sz>=4 && IsPtOnRightOf4Segs(*bises[O(k+3,sz)],*bises[O(k-1,sz)],*bises[O(k,sz)],*bises[O(k+1,sz)],*bises[O(k+2,sz)],ptMem))
				{
					delete bises[O(k+3,sz)];
					bises.RemoveAt(O(k+3,sz));
					sz=bises.GetSize();
					if (sz==k) k--;
				}
				bises[O(k+2,sz)]->CreateO(bises[O(k+1,sz)],bises[O(k+3,sz)],offsetDist);
			}
			k=k-2;
			nLastCount=nCount;
		}
		else
		{
			k++;
		}

		//////////////////////////////////////////////////////////////////////////
	}


	sz=bises.GetSize();
	if (sz>2)
	{
		CPoint3D* pPt=NULL;
		for (int i=0;i<sz;i++)
		{
			if (bOutputOffset)
			{
				pPt=new CPoint3D(bises[i]->ptO);
			}
			else
			{
				pPt=new CPoint3D(*bises[i]);
			}
			m_ostPts.Add(pPt);
		}

		pPt=new CPoint3D(*m_ostPts[0]);
		m_ostPts.Add(pPt);

		for (int i=sz-1;i>=0;i--)
			delete bises[i];
		bises.RemoveAll();

		//TrimRawOffset(m_ostPtList);

		return TRUE;
	}
	else
	{
		for (int i=sz-1;i>=0;i--)
			delete bises[i];
		bises.RemoveAll();

		return FALSE;
	}
}

BOOL CPolyline::IsPtOnRightOf3Segs(const CPoint3D& ptTest, const CPoint3D& pt1, 
								   const CPoint3D& pt2, const CPoint3D& pt3, const CPoint3D& pt4)
{
	CVector3D v,v1,v2;

	v1=pt2-pt4;
	v2=pt3-pt4;
	double z=(v1*v2).dz;
	if (z<0) z=-z;
	if (z<TINY) return FALSE;

	//////////////////////////////////////////////////////////////////////////
	double lamda,dMin,d;
	int nPos,nSeg;
	nSeg=0;
	CPoint3D pt0_seg=pt1+(pt1-pt3);

	v=pt1-pt0_seg;
	v1=ptTest-pt0_seg;
	lamda=(v|v1)/v.GetMod2();
	if (lamda<=ONE)
	{
		nPos=0;
		v2=pt0_seg+v*lamda-ptTest;
		dMin=v2.GetMod2();
	}
	else
	{
		nPos=1;
		v2=pt1-ptTest;
		dMin=v2.GetMod2();
	}

	v=pt2-pt1;
	v1=ptTest-pt1;
	lamda=(v|v1)/v.GetMod2();
	if (lamda<ZERO)
	{
		d=v1.GetMod2();
		if (d<dMin)
		{
			nPos=-1;
			nSeg=1;
			dMin=d;
		}
	}
	if (lamda>=ZERO && lamda<=ONE)
	{
		v2=pt1+v*lamda-ptTest;
		d=v2.GetMod2();
		if (d<dMin)
		{
			nPos=0;
			nSeg=1;
			dMin=d;
		}
	}
	if (lamda>ONE)
	{
		v2=pt2-ptTest;
		d=v2.GetMod2();
		if (d<dMin)
		{
			nPos=1;
			nSeg=1;
			dMin=d;
		}
	}

	v=pt3-pt2;
	v1=ptTest-pt2;
	lamda=(v|v1)/v.GetMod2();
	if (lamda<ZERO)
	{
		d=v1.GetMod2();
		if (d<dMin)
		{
			nPos=-1;
			nSeg=2;
			dMin=d;
		}
	}
	if (lamda>=ZERO && lamda<=ONE)
	{
		v2=pt2+v*lamda-ptTest;
		d=v2.GetMod2();
		if (d<dMin)
		{
			nPos=0;
			nSeg=2;
			dMin=d;
		}
	}
	if (lamda>ONE)
	{
		v2=pt3-ptTest;
		d=v2.GetMod2();
		if (d<dMin)
		{
			nPos=1;
			nSeg=2;
			dMin=d;
		}
	}

	v=pt4-pt3;
	v1=ptTest-pt3;
	lamda=(v|v1)/v.GetMod2();
	if (lamda<ZERO)
	{
		d=v1.GetMod2();
		if (d<dMin)
		{
			nPos=-1;
			nSeg=3;
			dMin=d;
		}
	}
	else
	{
		v2=pt3+v*lamda-ptTest;
		d=v2.GetMod2();
		if (d<dMin)
		{
			nPos=0;
			nSeg=3;
			dMin=d;
		}
	}
// 	if (lamda>1)  // 2011/8/2
// 	{
// 		v2=pt4-ptTest;
// 		d=v2.GetMold2();
// 		if (d<dMin)
// 		{
// 			nPos=1;
// 			nSeg=3;
// 			dMin=d;
// 		}
// 	}

	if (d<TINY) return TRUE;

	switch (nSeg)
	{
	case 0:
		switch (nPos)
		{
		case 0:  return IsPtOnRightOfSeg(ptTest,pt0_seg,pt1);
		case 1:  return IsPtOnRightAtCorner(ptTest,pt0_seg,pt1,pt2);
		}

	case 1: 
		switch (nPos)
		{
		case -1: return IsPtOnRightAtCorner(ptTest,pt0_seg,pt1,pt2);
		case 0:  return IsPtOnRightOfSeg(ptTest,pt1,pt2);
		case 1:  return IsPtOnRightAtCorner(ptTest,pt1,pt2,pt3);
		}

	case 2:
		switch (nPos)
		{
		case -1: return IsPtOnRightAtCorner(ptTest,pt1,pt2,pt3);
		case 0:  return IsPtOnRightOfSeg(ptTest,pt2,pt3);
		case 1:  return IsPtOnRightAtCorner(ptTest,pt2,pt3,pt4);
		}

	case 3:
		switch (nPos)
		{
		case -1: return IsPtOnRightAtCorner(ptTest,pt2,pt3,pt4);
		case 0:
		case 1:  return IsPtOnRightOfSeg(ptTest,pt3,pt4);
		}
	}

	return FALSE;
}

BOOL CPolyline::IsPtOnRightOf4Segs(const CPoint3D& ptTest, const CPoint3D& pt1, const CPoint3D& pt2, 
								   const CPoint3D& pt3, const CPoint3D& pt4, const CPoint3D& pt5)
{
	CVector3D v,v1,v2;

	v1=pt3-pt5;
	v2=pt4-pt5;
	double z=(v1*v2).dz;
	if (z<0) z=-z;
	if (z<TINY) return FALSE;

	//////////////////////////////////////////////////////////////////////////
	double lamda,dMin,d;
	int nPos,nSeg;
	nSeg=0;
	dMin=LARGE;

	v=pt1-pt4;
	CPoint3D pt0=pt1+v;
	v1=ptTest-pt1;
	lamda=(v|v1)/v.GetMod2();
	if (lamda>0)
	{
		nPos=0;
		v2=pt1+v*lamda-ptTest;
		dMin=v2.GetMod2();
	}
	else
	{
		nPos=1;
		dMin=v1.GetMod2();
	}
	
	
	v=pt2-pt1;
	v1=ptTest-pt1;
	lamda=(v|v1)/v.GetMod2();
	if (lamda<0)
	{
		d=v1.GetMod2();
		if (d<dMin)
		{
			nPos=-1;
			nSeg=1;
			dMin=d;
		}
	}
	if (lamda>=ZERO && lamda<=ONE)
	{
		v2=pt1+v*lamda-ptTest;
		d=v2.GetMod2();
		if (d<dMin)
		{
			nPos=0;
			nSeg=1;
			dMin=d;
		}
	}
	if (lamda>ONE)
	{
		v2=pt2-ptTest;
		d=v2.GetMod2();
		if (d<dMin)
		{
			nPos=1;
			nSeg=1;
			dMin=d;
		}
	}

	v=pt3-pt2;
	v1=ptTest-pt2;
	lamda=(v|v1)/v.GetMod2();
	if (lamda<ZERO)
	{
		d=v1.GetMod2();
		if (d<dMin)
		{
			nPos=-1;
			nSeg=2;
			dMin=d;
		}
	}
	if (lamda>=ZERO && lamda<=ONE)
	{
		v2=pt2+v*lamda-ptTest;
		d=v2.GetMod2();
		if (d<dMin)
		{
			nPos=0;
			nSeg=2;
			dMin=d;
		}
	}
	if (lamda>ONE)
	{
		v2=pt3-ptTest;
		d=v2.GetMod2();
		if (d<dMin)
		{
			nPos=1;
			nSeg=2;
			dMin=d;
		}
	}

	v=pt4-pt3;
	v1=ptTest-pt3;
	lamda=(v|v1)/v.GetMod2();
	if (lamda<ZERO)
	{
		d=v1.GetMod2();
		if (d<dMin)
		{
			nPos=-1;
			nSeg=3;
			dMin=d;
		}
	}
	if (lamda>=ZERO && lamda<=ONE)
	{
		v2=pt3+v*lamda-ptTest;
		d=v2.GetMod2();
		if (d<dMin)
		{
			nPos=0;
			nSeg=3;
			dMin=d;
		}
	}
	if (lamda>ONE)
	{
		v2=pt4-ptTest;
		d=v2.GetMod2();
		if (d<dMin)
		{
			nPos=1;
			nSeg=3;
			dMin=d;
		}
	}

	v=pt5-pt4;
	v1=ptTest-pt4;
	lamda=(v|v1)/v.GetMod2();
	if (lamda<ZERO)
	{
		d=v1.GetMod2();
		if (d<dMin)
		{
			nPos=-1;
			nSeg=4;
			dMin=d;
		}
	}
	if (lamda>=ZERO && lamda<=ONE)
	{
		v2=pt4+v*lamda-ptTest;
		d=v2.GetMod2();
		if (d<dMin)
		{
			nPos=0;
			nSeg=4;
			dMin=d;
		}
	}
	if (lamda>ONE)
	{
		v2=pt5-ptTest;
		d=v2.GetMod2();
		if (d<dMin)
		{
			nPos=1;
			nSeg=4;
			dMin=d;
		}
	}

	if (d<TINY) return TRUE;

	switch (nSeg)
	{
	case 0:
		switch (nPos)
		{
		case 0:  return IsPtOnRightOfSeg(ptTest,pt0,pt1); 
		case 1:  return IsPtOnRightAtCorner(ptTest,pt0,pt1,pt2);
		}
	case 1:
		switch (nPos)
		{
		case -1: return IsPtOnRightAtCorner(ptTest,pt0,pt1,pt2);
		case 0:  return IsPtOnRightOfSeg(ptTest,pt1,pt2);
		case 1:  return IsPtOnRightAtCorner(ptTest,pt1,pt2,pt3);
		}

	case 2:
		switch (nPos)
		{
		case -1: return IsPtOnRightAtCorner(ptTest,pt1,pt2,pt3);
		case 0:  return IsPtOnRightOfSeg(ptTest,pt2,pt3);
		case 1:  return IsPtOnRightAtCorner(ptTest,pt2,pt3,pt4);
		}

	case 3:
		switch (nPos)
		{
		case -1: return IsPtOnRightAtCorner(ptTest,pt2,pt3,pt4);
		case 0:  return IsPtOnRightOfSeg(ptTest,pt3,pt4);
		case 1:  return IsPtOnRightAtCorner(ptTest,pt3,pt4,pt5);
		}

	case 4:
		switch (nPos)
		{
		case -1: return IsPtOnRightAtCorner(ptTest,pt3,pt4,pt5);
		case 0:
		case 1:  return IsPtOnRightOfSeg(ptTest,pt4,pt5);
		}
	}

	return FALSE;
}

BOOL CPolyline::IsPtOnRightAtCorner(const CPoint3D& ptTest,  const CPoint3D& pt1, const CPoint3D& pt2, const CPoint3D& pt3)
{
	CVector3D v1,v2,v;
	v1=pt1-pt2;
	v2=pt3-pt2;
	v1.Normalize();
	v2.Normalize();
	v=v1+v2;
	if ((v1*v2).dz<0) v=-v;

	v=GetOrthoVectorOnXYPlane(v);
	CPoint3D pt=pt2+v;

	v1=pt2-ptTest;
	v2=pt-ptTest;
	if ((v1*v2).dz<=0) 
		return TRUE; 
	else 
		return FALSE;
}

BOOL CPolyline::IsPtOnRightOfSeg(const CPoint3D& ptTest, const CPoint3D& pt1, const CPoint3D& pt2)
{
	CVector3D v1,v2;
	v1=pt1-ptTest;
	v2=pt2-ptTest;
	if ((v1*v2).dz<=0)
		return TRUE;
	else
		return FALSE;
}

void CPolyline::TrimRawOffset(Point3DAry &pts)
{
	double angle,dotProduct,d,sina,cosa,len1,len2,d2;
	CVector3D v1,v2;
	CPoint3D pt1,pt2;
	int k=1;
	while (k<=pts.GetSize()-2)
	{
		v1=*pts[k-1]-*pts[k];
		v2=*pts[k+1]-*pts[k];
		if ((v1*v2).dz<=0)
		{
			k++;
		}
		else
		{
			len1=v1.GetMod2();
			len2=v2.GetMod2();
			v1.Normalize();
			v2.Normalize();
			dotProduct=v1|v2;
			if (dotProduct>1) dotProduct=1;
			if (dotProduct<-1) dotProduct=-1;
			angle=acos(dotProduct)/2.0;
			if (angle>PI/3.0)  // only angles those are less than 120 degree are to be trimmed //
			{
				k=k+1;
				continue;
			}
			sina=sin(angle);
			cosa=cos(angle);
			d=(1-sina)*m_offsetDist/(sina*cosa);
			d2=d*d;

			if (len1>d2 && len2>d2)
			{
				pt1=*pts[k]+v1*d;
				pt2=*pts[k]+v2*d;
				*pts[k]=pt1;
				CPoint3D* pPt=new CPoint3D(pt2);
				pts.InsertAt(k+1,pPt);
				k=k+2;
				continue;
			}

			if (len1<=d2 && len2>d2)
			{
				*pts[k]=*pts[k]+v2*d;
				k++;
				continue;
			}

			if (len2<=d2 && len1>d2)
			{
				*pts[k]=*pts[k]+v1*d;	
				k++;
				continue;
			}

			if (len1<=d2 && len2<=d2)
			{
				delete pts[k];
				pts.RemoveAt(k);
				k++;
				continue;
			}
		}
	}
	
	int sz=pts.GetSize();
	v1=*pts[sz-2]-*pts[sz-1];
	v2=*pts[1]-*pts[0];
	if ((v1*v2).dz>0)
	{		
		v1.Normalize();
		v2.Normalize();
		dotProduct=v1|v2;
		if (dotProduct>1) dotProduct=1;
		if (dotProduct<-1) dotProduct=-1;
		angle=acos(dotProduct)/2.0;
		if (angle>PI/3.0) return;  
		sina=sin(angle);
		cosa=cos(angle);
		d=(1-sina)*m_offsetDist/(sina*cosa);
		pt1=*pts[0]+v1*d;
		pt2=*pts[0]+v2*d;
		*pts[0]=pt2;
		*pts[sz-1]=pt2;
		CPoint3D* pPt=new CPoint3D(pt1);
		pts.InsertAt(sz-1,pPt);
	}
}

void CPolyline::TreeAnalyse(Point3DAry &pts)
{
	int sz=pts.GetSize();
	double dis2=m_offsetDist*m_offsetDist;
	CPoint3D* pPt=NULL;
	CPolyline* pPolyline=NULL;
	CIstPt* pIstPt=NULL;
	CTypedPtrArray<CObArray,CIstPt*> istPts;

	int szIstPts=ReportIntersection(pts,istPts);
    //int szIstPtList=CalAllIstPts(ptList,istPtList);        // 计算出所有的自相交

	if (0==szIstPts)    // If there's no intersection, just save the raw offset curve to the output array //
	{   
		pPolyline=new CPolyline;
		for (int i=0;i<sz;i++)
		{
			pPt=new CPoint3D(*pts[i]);
			pPolyline->m_pts.Add(pPt);
		}
		m_treeOPs.Add(pPolyline);
	}
	else                       // But if there is intersection, carry out the TA procedure // 
	{  
		SortSelfInsts(istPts,0,szIstPts-1);

		for (int i=szIstPts-1;i>=1;i--) // Delete the same intersection, this is caused by the failure of reporting self-intersection //
		{
			if (istPts[i]->m_s==istPts[i-1]->m_s)
			{
				delete istPts[i];
				istPts.RemoveAt(i);
			}
		}

		RemoveInstSelfInsts(istPts);
        
		CalEveRects();     // Calculate envelope rect for each segment, stores in m_eveRect //
		double d1,d2;
		int nSafe=0;
		szIstPts=istPts.GetSize();
		for (int i=0;i<szIstPts;i++)
		{
			d1=CalcDis2PtToProfile(*pts[(int)istPts[i]->m_s])+TINY-dis2;   // TINY is set to avoid trunk errors when
			d2=CalcDis2PtToProfile(*pts[(int)istPts[i]->m_s+1])+TINY-dis2; // calculating the distance.
			if (d1>0 && d2<0)
			{
				nSafe=(int)istPts[i]->m_s;
				break;
			}
			if (d1<0 && d2>0)
			{
				nSafe=(int)istPts[i]->m_s+1;
				break;
			}
			d1=CalcDis2PtToProfile(*pts[(int)istPts[i]->m_b])+TINY-dis2;
			d2=CalcDis2PtToProfile(*pts[(int)istPts[i]->m_b+1])+TINY-dis2;
			if (d1>0 && d2<0)
			{
				nSafe=(int)istPts[i]->m_b;
				break;
			}
			if (d1<0 && d2>0)
			{
				nSafe=(int)istPts[i]->m_b+1;
				break;
			}

			if (szIstPts-1==i)
			{
				ReleaseEveRects();
				for (int i=szIstPts-1;i>=0;i--)
					delete istPts[i];
				istPts.RemoveAll();
				return;
			}
		}
        ReleaseEveRects(); // Release m_eveRect //

		for (int i=0;i<szIstPts;i++)
		{
			istPts[i]->m_s=O(istPts[i]->m_s,nSafe,sz);
			istPts[i]->m_b=O(istPts[i]->m_b,nSafe,sz);
			if (istPts[i]->m_s>istPts[i]->m_b)
			{
				double temp=istPts[i]->m_s;
				istPts[i]->m_s=istPts[i]->m_b;
				istPts[i]->m_b=temp;
			}
		}
		MakeFirstAt(pts,nSafe);

		SortSelfInsts(istPts,0,szIstPts-1);

		CIstPt* pRoot=new CIstPt(*pts[0],0,pts.GetSize()-2);  // need to be delete
		ConstructTree(istPts,pRoot);   // Conduct all the intersection points to a tree stucture

		//////////////////////////////////////////////////////////////////////////
		// Tree analysis to collect data //
		CIstPt* pCur=NULL;
		CArray<CPos,CPos&> posAry;
		CPos pos(0,1);
		int curLayer=0;
		int szChildList;
		posAry.Add(pos);
		pCur=pRoot;

EXPAND_OFFSET:	
		szChildList=pCur->pChildList.GetSize();

		// safe data  收集所有直系子节点
		if (0==curLayer%2)
		{
			pPolyline=new CPolyline;
			if (szChildList>0)  // 这里是有问题的 pCur 上的点没有考虑进去//
			{
				pPt=new CPoint3D(pCur->m_pt);
				pPolyline->m_pts.Add(pPt);

				for (int i=pCur->m_s+1;i<=pCur->pChildList[0]->m_s;i++)
				{
					pPt=new CPoint3D(*pts[i]);
					pPolyline->m_pts.Add(pPt);
				}
				pPt=new CPoint3D(pCur->pChildList[0]->m_pt);
				pPolyline->m_pts.Add(pPt);

				for (int j=1;j<szChildList;j++)
				{
					for (int i=pCur->pChildList[j-1]->m_b+1;i<=pCur->pChildList[j]->m_s;i++)
					{
						pPt=new CPoint3D(*pts[i]);
						pPolyline->m_pts.Add(pPt);
					}
					pPt=new CPoint3D(pCur->pChildList[j]->m_pt);
					pPolyline->m_pts.Add(pPt);
				}

				for (int i=pCur->pChildList[szChildList-1]->m_b+1;i<=pCur->m_b;i++)
				{
					pPt=new CPoint3D(*pts[i]);
					pPolyline->m_pts.Add(pPt);
				}

				pPt=new CPoint3D(pCur->m_pt);
				pPolyline->m_pts.Add(pPt);
				m_treeOPs.Add(pPolyline);
			}
			else
			{
				pPt=new CPoint3D(pCur->m_pt);
				pPolyline->m_pts.Add(pPt);
				for (int i=pCur->m_s+1;i<=pCur->m_b;i++)
				{
					pPt=new CPoint3D(*pts[i]);
					pPolyline->m_pts.Add(pPt);
				}
				pPt=new CPoint3D(pCur->m_pt);
				pPolyline->m_pts.Add(pPt);
				m_treeOPs.Add(pPolyline);
			}

		}
		// safe data  收集所有直系子节点

		if (szChildList>0)
		{
			pCur=pCur->pChildList[0];
			pos.i=0;
			pos.sz=szChildList;
			curLayer++;
			posAry.Add(pos);
			goto EXPAND_OFFSET;
		}
		else
		{
LAST_LAYER_OFFSET:
			if (posAry[curLayer].i==posAry[curLayer].sz-1) // 回上一层
			{
				if (0==curLayer)
				{
					goto BREAK_OUT_OFFSET;
				}
				posAry.RemoveAt(curLayer);
				curLayer--;
				pCur=pCur->pFather;
				goto LAST_LAYER_OFFSET;
			}
			else
			{
				posAry[curLayer].i++;
				pCur=pCur->pFather->pChildList[posAry[curLayer].i]; // 去旁边
				goto EXPAND_OFFSET;
			}

		}

BREAK_OUT_OFFSET:
		posAry.RemoveAll();
		delete pRoot;
	}
}

// Use the bubble method to sort istPtList in ascending order //
void CPolyline::SortSelfInsts(IstPtAry &istPtList)
{
	CIstPt istPtTemp;   
	int sz=istPtList.GetSize();
 	for (int i=sz-2;i>=0;i--)
	{
		for (int j=0;j<=i;j++)
		{
		    if (istPtList[j]->m_s > istPtList[j+1]->m_s)
	 		{
	 			istPtTemp=*istPtList[j];
				*istPtList[j]=*istPtList[j+1];
	 			*istPtList[j+1]=istPtTemp;
	 		}
	 	}
	 }
}

// Use quick sort to sort, the quicksort algorithm can be found on internet //
void CPolyline::SortSelfInsts(IstPtAry &istPtList, const int& s, const int& e)
{
	if (s >= e) return;

	int i=s, j=e-1;
	CIstPt tmp;
	do
	{     
		while(        istPtList[i]->m_s <  istPtList[e]->m_s) i++;  // In ascending order
		while(j>=0 && istPtList[j]->m_s >= istPtList[e]->m_s) j--;  // the "=" must be put, or mistake happens.

		if (i<j)
		{
			tmp = *istPtList[i];
			*istPtList[i] = *istPtList[j];
			*istPtList[j] = tmp;
		}
		else 
		{
			tmp = *istPtList[i];
			*istPtList[i] = *istPtList[e];
			*istPtList[e] = tmp;
		}
	} while(i<j);

	SortSelfInsts(istPtList,s, i-1);
	SortSelfInsts(istPtList,i+1, e);
}

void CPolyline::RemoveInstSelfInsts(IstPtAry &istPtList)
{
	int i,j,k;
	int s,m,b;
	i=0;

RemoveInstSelfInst_srestart:
	while (i<istPtList.GetSize())
	{
		j=i+1;
		while (j<istPtList.GetSize())
		{
			if ((*istPtList[i])*(*istPtList[j]))
			{
				for (k=0;k<istPtList.GetSize();k++) // check k with i
				{
					if (k==i || k==j) continue;

					if ((*istPtList[k])*(*istPtList[i]) && (1==j-i || 1==abs(j-k) || 1==abs(k-i))) // the other has found, check the three off them
					{
						if ((*istPtList[k])*(*istPtList[j])) // remove the three of them
						{
							if (k>j)
							{ s=i;m=j;b=k;}
							else if (k<i)
							{ s=k;m=i;b=j;}
							else
							{ s=i;m=k;b=j;}
							delete istPtList[b];
							istPtList.RemoveAt(b);
							delete istPtList[m];
							istPtList.RemoveAt(m);
							delete istPtList[s];
							istPtList.RemoveAt(s);
						}
						else  // remove two of the three. k and j
						{
							if (k>j)
							{ s=j;b=k;}
							else
							{ s=k;b=j;}
							delete istPtList[b];
							istPtList.RemoveAt(b);
							delete istPtList[s];
							istPtList.RemoveAt(s);
						}
						goto RemoveInstSelfInst_srestart;
					}
				}

				for (k=0;k<istPtList.GetSize();k++)
				{
					if (k==i || k==j) continue;

					if ((*istPtList[k])*(*istPtList[j]) && (1==j-i || 1==abs(j-k) || 1==abs(k-i))) // the other has found, check the three off them
					{
						if ((*istPtList[k])*(*istPtList[i])) // remove the three of them
						{
							if (k>j)
							{ s=i;m=j;b=k;}
							else if (k<i)
							{ s=k;m=i;b=j;}
							else
							{ s=i;m=k;b=j;}
							delete istPtList[b];
							istPtList.RemoveAt(b);
							delete istPtList[m];
							istPtList.RemoveAt(m);
							delete istPtList[s];
							istPtList.RemoveAt(s);
						}
						else  // remove two of the three
						{
							if (k>i)
							{ s=i;b=k;}
							else
							{ s=k;b=i;}
							delete istPtList[b];
							istPtList.RemoveAt(b);
							delete istPtList[s];
							istPtList.RemoveAt(s);
						}
						goto RemoveInstSelfInst_srestart;
					}
				}
			}
			j++;
		}
		i++;
	}
}

void CPolyline::ConstructTree(IstPtAry &istPtList,CIstPt* pRoot)
{
	int sz=istPtList.GetSize();
	CIstPt* pIstPt=NULL;
	BOOL* bRemove=new BOOL[sz];
	for (int i=0;i<sz;i++)
		bRemove[i]=FALSE;

	for (int i=0;i<sz;i++)  // 找出root下的第一级子
	{
		if (0==FindFathers(i,istPtList))
		{
			pIstPt=new CIstPt(*istPtList[i]);
			pIstPt->pFather=pRoot;
			pRoot->pChildList.Add(pIstPt);
			bRemove[i]=TRUE;
			continue;
		}
	}
	for (int i=sz-1;i>=0;i--)
	{
		if (TRUE==bRemove[i])
		{
			delete istPtList[i];
			istPtList.RemoveAt(i);
		}
	}
	delete []bRemove;

	sz=istPtList.GetSize();
	for (int i=0;i<sz;i++)
	{
		if (pRoot->Add(istPtList[0]))
		{
			delete istPtList[0];
			istPtList.RemoveAt(0);
		}
	}

	sz=istPtList.GetSize();
	for (int i=sz-1;i>=0;i--)
		delete istPtList[i];
	istPtList.RemoveAll();
}

CXAlignRect CPolyline::CalAABBRect()
{
	int sz=m_pts.GetSize();
	CXAlignRect rt;
	if (0==sz)
		return rt;

	double x_min, x_max, y_min, y_max;
	x_min=x_max=m_pts[0]->x;
	y_min=y_max=m_pts[0]->y;

	for (int i=1;i<sz;i++)
	{
		if (m_pts[i]->x<x_min)
			x_min=m_pts[i]->x;

		if (m_pts[i]->x>x_max)
			x_max=m_pts[i]->x;

		if (m_pts[i]->y<y_min)
			y_min=m_pts[i]->y;

		if (m_pts[i]->y>y_max)
			y_max=m_pts[i]->y;
	}

	rt.left=x_min;
	rt.right=x_max;
	rt.bottom=y_min;
	rt.top=y_max;

    return rt;
}

void CPolyline::CalEveRects()
{
	double left, right, bottom, top;
	double temp;
	int szPt=m_pts.GetSize();
	m_eveRt=new CXAlignRect[szPt-1];

	for (int i=0;i<=szPt-2;i++)
	{
		left=m_pts[i]->x;
		right=m_pts[i+1]->x;
		if (left>right)
		{
			temp=left;
			left=right;
			right=temp;
		}
		bottom=m_pts[i]->y;
		top=m_pts[i+1]->y;
		if (bottom>top)
		{
			temp=bottom;
			bottom=top;
			top=temp;
		}
		m_eveRt[i].left=left;
		m_eveRt[i].right=right;
		m_eveRt[i].bottom=bottom;
		m_eveRt[i].top=top;
	}
}

void CPolyline::ReleaseEveRects()
{
	if (NULL!=m_eveRt)
	{
		delete []m_eveRt;
		m_eveRt=NULL;
	}
}

// ------------------------------------- CalAllIstPts 原理-------------------------------------------------- //
// 写了这个函数可将多边形自交的的时间大大减小，280 -> 0ms。原理是先找出每条边的包络矩形，判断包络矩形是否相- // 
// 交，如果相交再进行判断，不相交的话直接进入下一条,-------------------------------------------------------- //
// --------------------------------------------------------------------------------------------------------- //
int CPolyline::CalAllIstPts(Point3DAry &ptList_in, IstPtAry &istPtList_out)
{
	CPoint3D ptCross;
	CIstPt* pIstPt=NULL;
	double left, right, bottom, top;
	double temp;
	int szPt=ptList_in.GetSize();
	CXAlignRect* rt=new CXAlignRect[szPt-1];
	for (int i=0;i<=szPt-2;i++)
	{
		left=ptList_in[i]->x;
		right=ptList_in[i+1]->x;
		if (left>right)
		{
			temp=left;
			left=right;
			right=temp;
		}
		bottom=ptList_in[i]->y;
		top=ptList_in[i+1]->y;
		if (bottom>top)
		{
			temp=bottom;
			bottom=top;
			top=temp;
		}
		rt[i].left=left;
		rt[i].right=right;
		rt[i].bottom=bottom;
		rt[i].top=top;
	}

	double lamda1,lamda2;

	for (int i=2;i<=szPt-3;i++)    // 处理第0条，有点特殊 //
	{
		if (rt[0]*rt[i])
		{
			if (Is2SegsCross(ptCross,*ptList_in[0],*ptList_in[1],*ptList_in[i],*ptList_in[i+1]))
			{
				lamda1=(ptCross-*ptList_in[0]).GetMod2()/(*ptList_in[1]-*ptList_in[0]).GetMod2();
				lamda2=(ptCross-*ptList_in[i]).GetMod2()/(*ptList_in[i+1]-*ptList_in[i]).GetMod2();
				lamda2=(double)i+lamda2;
				pIstPt=new CIstPt(ptCross,lamda1,lamda2);
				istPtList_out.Add(pIstPt);
			}
		}
	}

	for (int k=1;k<=szPt-3;k++)  // <=sz-4 是可以的
	{
		for (int i=k+2;i<=szPt-2;i++)
		{
			if (rt[k]*rt[i])
			{
				if (Is2SegsCross(ptCross,*ptList_in[k],*ptList_in[k+1],*ptList_in[i],*ptList_in[i+1]))
				{
					lamda1=(ptCross-*ptList_in[k]).GetMod2()/(*ptList_in[k+1]-*ptList_in[k]).GetMod2();
					lamda2=(ptCross-*ptList_in[i]).GetMod2()/(*ptList_in[i+1]-*ptList_in[i]).GetMod2();
					lamda1=(double)k+lamda1;
					lamda2=(double)i+lamda2;
					pIstPt=new CIstPt(ptCross,lamda1,lamda2);
					istPtList_out.Add(pIstPt);
				}
			}
		}
	}

	delete []rt;
	return istPtList_out.GetSize();
}

// 用 Sweep Line 方法来
int CPolyline::ReportIntersection(Point3DAry &ptList, IstPtAry &istPtList_out)
{
	int sz=ptList.GetSize();
	CPoint3D pt_ymax=*ptList[0];
	int n_ymax=0;

	for (int i=1;i<sz;i++)
	{
		if (ptList[i]->y > pt_ymax.y)
		{
			pt_ymax=*ptList[i];
			n_ymax=i;
			continue;
		}
		if (ptList[i]->y == pt_ymax.y)
		{
			if (ptList[i]->x < pt_ymax.x)
			{
				pt_ymax=*ptList[i];
				n_ymax=i;
			}
		}
	}
	MakeFirstAt(ptList,n_ymax);   // 将图形的最高点设为第一个点，这样在下面用扫面线法计算交线时可以减少很多讨论

	// Debug
// 	CPoint3D pt0=*ptList[0];
// 	CPoint3D pt1=*ptList[1];
// 	CPoint3D ptLast=*ptList[sz-1];

	// Debug

	CStatusSpace statusSpace;
	CStatus* pStatus=NULL;
	CPoint3D ptCross;
	CEvt evt;

	statusSpace.evtSpace.LoadEvts(ptList);      // Load evts and Sort them quickly
	statusSpace.SetSize(sz-1);

	pStatus=new CStatus(*ptList[0],*ptList[1],0);
	statusSpace.Add(pStatus);
	pStatus=new CStatus(*ptList[0],*ptList[sz-2],sz-2);
	statusSpace.Add(pStatus);

	int szEvtList=statusSpace.evtSpace.evtList.GetSize();

	// Debug
	//CEvt evtTest=*statusSpace.evtSpace.evtList[szEvtList-1];

	// Debug

	statusSpace.evtSpace.RemoveEvt(szEvtList-1);

	int nR=0;   // 保存返回值
	int nPt;    // 原始多边形 ptList 上的第 nPt 点

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	while (statusSpace.evtSpace.evtList.GetSize()>0)
	{
		szEvtList=statusSpace.evtSpace.evtList.GetSize();
		evt=*(statusSpace.evtSpace.evtList[szEvtList-1]);
	    statusSpace.evtSpace.RemoveEvt(szEvtList-1);
		statusSpace.SetYLine(evt.y);
		nPt=evt.i;

		// Debug 
// 		if (246==nPt)
// 		{
// 			int x=1+1;
// 		}
// 		if (-1==nPt)
// 		{
// 			int x=1+1;
// 		}
		// Debug

		if (-1==nPt) // 表示是一个交点，交换涉及到得两条线段
		{
			statusSpace.SwapStatuses(evt.type_b,evt.type_f,istPtList_out);
		}
		else         // 表示是一个端点的事件点
		{
			if (BOTTOM_END==evt.type_b && TOP_END==evt.type_f)
			{
				statusSpace.RemoveStatus(nPt-1,istPtList_out);
				pStatus=new CStatus(*ptList[nPt],*ptList[nPt+1],nPt);
				statusSpace.InsertStatus(pStatus,istPtList_out);
				continue;
			}

			if (TOP_END==evt.type_b && BOTTOM_END==evt.type_f)
			{
				statusSpace.RemoveStatus(nPt,istPtList_out);
				pStatus=new CStatus(*ptList[nPt],*ptList[nPt-1],nPt-1);
				statusSpace.InsertStatus(pStatus,istPtList_out);
				continue;
			}

			if (BOTTOM_END==evt.type_b && BOTTOM_END==evt.type_f)
			{
				statusSpace.RemoveStatus(nPt-1,istPtList_out);
				statusSpace.RemoveStatus(nPt,istPtList_out);
				continue;
			}

			if (TOP_END==evt.type_b && TOP_END==evt.type_f)
			{
				pStatus=new CStatus(*ptList[nPt],*ptList[nPt-1],nPt-1);
				statusSpace.InsertStatus(pStatus,istPtList_out);
				pStatus=new CStatus(*ptList[nPt],*ptList[nPt+1],nPt);
				statusSpace.InsertStatus(pStatus,istPtList_out);
				continue;
			}
		}
	}

	int szIstPtList=istPtList_out.GetSize();
	int s,b;
	double ds,db;
	CPoint3D pt;
	for (int i=0;i<szIstPtList;i++)
	{
		s=istPtList_out[i]->m_s;
		b=istPtList_out[i]->m_b;
		ds=(double)s+sqrt((istPtList_out[i]->m_pt-*ptList[s]).GetMod2()/(*ptList[s+1]-*ptList[s]).GetMod2());
		db=(double)b+sqrt((istPtList_out[i]->m_pt-*ptList[b]).GetMod2()/(*ptList[b+1]-*ptList[b]).GetMod2());
		istPtList_out[i]->m_s=ds;
		istPtList_out[i]->m_b=db;
	}

    return istPtList_out.GetSize();
}

double CPolyline::CalcDis2PtToProfile(const CPoint3D& ptTest)
{
	CXAlignRect eveRt(ptTest,m_offsetDist);
	CVector3D vec_P1P2;
	CVector3D vec_PP1;
	CVector3D vec_PP2;
	CVector3D vec_PPp;  // PP'向量
	double lamda=0;
	double squar_actual_dis=0;
	double squar_PP1=0;
	double squar_PP2=0;
	double min_squar_dis=LARGE;  // 

	int sz=m_pts.GetSize();
	for (int i=0;i<sz-1;i++)
	{
		// New
 		if (eveRt*m_eveRt[i]) // New
 		{
			vec_P1P2=*m_pts[i+1]-*m_pts[i];
			vec_PP1=*m_pts[i]-ptTest;

			lamda=-(vec_PP1|vec_P1P2)/(vec_P1P2|vec_P1P2);
			if (lamda<0 || lamda >1)
			{
				vec_PP2=*m_pts[i+1]-ptTest;
				squar_PP1=vec_PP1.GetMod2();
				squar_PP2=vec_PP2.GetMod2();
				squar_actual_dis=squar_PP1;
				if (squar_PP2<squar_PP1)
					squar_actual_dis=squar_PP2;
			}
			else
			{
				vec_PPp=vec_PP1+vec_P1P2*lamda;
				squar_actual_dis=vec_PPp.GetMod2();
			}

			if (squar_actual_dis<min_squar_dis)
				min_squar_dis=squar_actual_dis;
		}
	}

	return min_squar_dis;
}

int CPolyline::FindFathers(const int& iWhich2Find, IstPtAry &istPtList)
{
	int sz=istPtList.GetSize();
	if (iWhich2Find>sz-1)
	{
		return -1;  // 错误
	}
	int count=0;
	for (int i=0;i<sz;i++)
	{
		if (*istPtList[iWhich2Find]<*istPtList[i])
		{
			count++;
		}
	}

	return count;
}


// 起点和顶点不重合的情况下的 O(i) 函数
int CPolyline::O(int i, int sz)
{
	if (i<0)
	{
		int n = -i/sz + 1;
		i = i + n*sz;
	}
	return i%sz;
}

// int CPolyline::O(const int& i, const int& nOffset, const int& sz)
// {
// 	if (i>=nOffset && i<=sz-1)
// 		return i-nOffset;
// 	else
// 		return sz-1-nOffset+i;
// }

double CPolyline::O(const double &i, const int &nOffset, const int &sz)
{
	double delta=i-floor(i);
	int j=(int)i;

	if (j>=nOffset && j<=sz-1)
		return double(j-nOffset)+delta;
	else
		return double(sz-1-nOffset+j)+delta;
}

/* 判断点是否在线段上，这里的点是在线段所在直线方向上的，所以不用距离来判断而用向量点乘和0比较大小 */
BOOL CPolyline::IsPtOnSeg(const CPoint3D& pt, const CPoint3D& ptS_seg, const CPoint3D& ptE_seg)
{
	CVector3D vec1=pt-ptS_seg;
	CVector3D vec2=pt-ptE_seg;

	if ((vec1|vec2)<=0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}	
}

void CPolyline::FindCenterPt()
{
	int sz=m_pts.GetSize();
	if (0==sz)
		return;

	double left, right, bottom, top;

	if (sz>0)
	{
         left=right=m_pts[0]->x;
		 bottom=top=m_pts[0]->y;
	}

	for (int i=1;i<sz;i++)
	{
		if (m_pts[i]->x<left)
		{
			left=m_pts[i]->x;
		}
		if (m_pts[i]->x>right)
		{
			right=m_pts[i]->x;
		}
		if (m_pts[i]->y<bottom)
		{
			bottom=m_pts[i]->y;
		}
		if (m_pts[i]->y>top)
		{
			top=m_pts[i]->y;
		}
	}

	m_ptCenter.x=(left+right)/2;
	m_ptCenter.y=(top+bottom)/2;
	m_ptCenter.z=m_pts[0]->z;
}


void CPolyline::LinkToolPath()
{
	CPolyline* pD=NULL; // Deepest layer
	CPolyline* pF=NULL; // Father of deepest layer
	CPoint3D* pPt=NULL;

	pD=FindDeepestLayer();
	if (NULL==pD) return;
	pF=pD->m_pFather;
	pF->MakeFirstAt(pF->InsertPt(*pD->m_pts[0]));
	int szD=pD->m_pts.GetSize();
	for (int i=0;i<szD;i++)
	{
		pPt=new CPoint3D(*pD->m_pts[i]);
		pF->m_pts.Add(pPt);
	}
	pPt=new CPoint3D(*pF->m_pts[0]);
	pF->m_pts.Add(pPt);
	delete pF->m_treeOPs[0];
	pF->m_treeOPs.RemoveAt(0);
	LinkToolPath();  // Recursively call this function
}

int CPolyline::InsertPt(const CPoint3D& pt)
{
	double d_pt_min=HUGE;
	double d_seg_min=HUGE;
	int n_pt_min, n_seg_min;
	double d=0;

	int sz=GetSize();
	for (int i=0;i<sz;i++)
	{
		d=GetDistance(pt,*m_pts[i]);
		if (d < d_pt_min)
		{
			d_pt_min=d;
			n_pt_min=i;
		}
	}

	for (int i=0;i<=sz-2;i++)
	{
		d=GetDistPtToSeg(pt,*m_pts[i],*m_pts[i+1]);
		if (d < d_seg_min)
		{
			d_seg_min=d;
			n_seg_min=i;
		}
	}
 
	if (d_pt_min <= d_seg_min)
	{
		return n_pt_min;
	}
	else
	{
		CPoint3D pt1=*m_pts[n_seg_min];
		CPoint3D pt2=*m_pts[n_seg_min+1];
		CVector3D vec_P1P=pt-pt1;
		CVector3D vec_P1P2=pt2-pt1;
		double lamda=(vec_P1P|vec_P1P2)/vec_P1P2.GetMod2();
		CPoint3D pt_shadow=pt1+vec_P1P2*lamda;		
		CPoint3D* pPt=new CPoint3D(pt_shadow);
		m_pts.InsertAt(n_seg_min+1,pPt);
		return n_seg_min+1;
	}
}

void CPolyline::SetOPsUnused()
{
	int sz=m_sequOPs.GetSize();
	for (int i=0;i<sz;i++)
	{
		m_sequOPs[i]->m_bIsUsed=FALSE;
	}
}

CPolyline* CPolyline::FindDeepestLayer() // [0] of the depest layer
{
	CPolyline* pCur=this;
	CPolyline* pMax=NULL;
	CArray<CPos,CPos&> posAry;
	CPos pos(0,1);
	int curLayer=0, maxLayer=0;
	int szChildList;
	posAry.Add(pos);

EXPAND:	
	szChildList=pCur->m_treeOPs.GetSize();
	if (szChildList>0)
	{
		pCur=pCur->m_treeOPs[0];
		pos.i=0;
		pos.sz=szChildList;
		curLayer++;
		posAry.Add(pos);
		if (curLayer>maxLayer)
		{
			maxLayer=curLayer;
			pMax=pCur;
		}
		goto EXPAND;
	}
	else
	{
LAST_LAYER:
		if (posAry[curLayer].i==posAry[curLayer].sz-1) // 回上一层
		{
			if (0==curLayer) goto BREAK_OUT;
			posAry.RemoveAt(curLayer);
			curLayer--;
			pCur=pCur->m_pFather;
			goto LAST_LAYER;
		}
		else
		{
			posAry[curLayer].i++;
			pCur=pCur->m_pFather->m_treeOPs[posAry[curLayer].i]; // 去旁边
			goto EXPAND;
		}
	}

BREAK_OUT:
	posAry.RemoveAll();

	if (pMax==this)
		return NULL;
	else
	    return pMax;
}

int CPolyline::FindClosestToPt(const CPoint3D& pt)
{
	int sz=m_pts.GetSize();
	if (0==sz) return -1;

	CVector3D vec;
	vec=*m_pts[0]-pt;
	int nMin=0;
	double d=0;
	double dMin=vec.GetMod2();
	for (int i=1;i<sz;i++)
	{
		vec=*m_pts[i]-pt;
		d=vec.GetMod2();
		if (d<dMin)
		{
			dMin=d;
			nMin=i;
		}
	}

	return nMin;
}

void CPolyline::UnevenOffset(double dR, CPolyline* & pPl)
{
	//CTypedPtrArray<CObArray,CPoint3D*> ptList;
	if (NULL==pPl)
		pPl=new CPolyline;
	CPoint3D* pPt=NULL;

	CVector3D v1,v2,v;
	double theta=0.0;
	double len=0.0;

	int sz=m_pts.GetSize();
	for (int i=0;i<sz;i++)
	{
		if (0==i || sz-1==i)
		{
			v1 = *m_pts[sz-2] - *m_pts[0];
			v2 = *m_pts[1] - *m_pts[0];
		}
		else
		{
			v1 = *m_pts[i-1] - *m_pts[i];
			v2 = *m_pts[i+1] - *m_pts[i];
		}

		v1.Normalize();
		v2.Normalize();
		v=v1+v2;
		v.Normalize();
		double dotProduct=v1|v2;
		if (dotProduct<-1)
			dotProduct=-1;
		if (dotProduct>1)
			dotProduct=1;
		theta=acos(dotProduct);
		if ((v1*v2).dz>0)
		{
			theta=2*PI-theta;
			v=-v;
		}
		len=(1+1/sin(theta/2))*dR;
		pPt=new CPoint3D(*m_pts[i]+v*len);
		pPl->m_pts.Add(pPt);
	}
}

void CPolyline::Write2DXF(CStdioFile& file)
{
	WriteHeader(file);
	WriteClasses(file);
	WriteTables(file);
	WriteBlocks(file);
	WriteEntities(file,TRUE);
	WriteObjects(file);
}

void CPolyline::WriteHeader(CStdioFile& myFile)
{
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("SECTION\n"));
	myFile.WriteString(_T("2\n"));
	myFile.WriteString(_T("HEADER\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$ACADVER\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("AC1015\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$ACADMAINTVER\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DWGCODEPAGE\n"));
	myFile.WriteString(_T("3\n"));
	myFile.WriteString(_T("ANSI_936\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$INSBASE\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$EXTMIN\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("392.1799027\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("120.4502685\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$EXTMAX\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("1167.575029\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("895.8453947\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$LIMMIN\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$LIMMAX\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("420\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("297\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$ORTHOMODE\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$REGENMODE\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$FILLMODE\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$QTEXTMODE\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$MIRRTEXT\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$LTSCALE\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$ATTMODE\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$TEXTSIZE\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("2.5\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$TRACEWID\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$TEXTSTYLE\n"));
	myFile.WriteString(_T("7\n"));
	myFile.WriteString(_T("Standard\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$CLAYER\n"));
	myFile.WriteString(_T("8\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$CELTYPE\n"));
	myFile.WriteString(_T("6\n"));
	myFile.WriteString(_T("ByLayer\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$CECOLOR\n"));
	myFile.WriteString(_T("62\n"));
	myFile.WriteString(_T("256\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$CELTSCALE\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DISPSILH\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMSCALE\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMASZ\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("2.5\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMEXO\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0.625\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMDLI\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("3.75\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMRND\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMDLE\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMEXE\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("1.25\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMTP\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMTM\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMTXT\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("2.5\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMCEN\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("2.5\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMTSZ\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMTOL\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMLIM\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMTIH\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMTOH\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMSE1\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMSE2\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMTAD\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMZIN\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("8\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMBLK\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMASO\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMSHO\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMPOST\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMAPOST\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMALT\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMALTD\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("3\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMALTF\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0.039370079\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMLFAC\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMTOFL\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMTVP\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMTIX\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMSOXD\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMSAH\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMBLK1\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMBLK2\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMSTYLE\n"));
	myFile.WriteString(_T("2\n"));
	myFile.WriteString(_T("ISO-25\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMCLRD\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMCLRE\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMCLRT\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMTFAC\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMGAP\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0.625\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMJUST\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMSD1\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMSD2\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMTOLJ\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMTZIN\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("8\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMALTZ\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMALTTZ\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMUPT\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMDEC\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("2\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMTDEC\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("2\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMALTU\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("2\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMALTTD\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("3\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMTXSTY\n"));
	myFile.WriteString(_T("7\n"));
	myFile.WriteString(_T("Standard\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMAUNIT\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMADEC\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMALTRND\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMAZIN\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMDSEP\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("44\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMATFIT\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("3\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMFRAC\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMLDRBLK\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMLUNIT\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("2\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMLWD\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("-2\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMLWE\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("-2\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$DIMTMOVE\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$LUNITS\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("2\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$LUPREC\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("4\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$SKETCHINC\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$FILLETRAD\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$AUNITS\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$AUPREC\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$MENU\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T(".\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$ELEVATION\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PELEVATION\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$THICKNESS\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$LIMCHECK\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$CHAMFERA\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$CHAMFERB\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$CHAMFERC\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$CHAMFERD\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$SKPOLY\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$TDCREATE\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("2453462.437\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$TDUCREATE\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("2453462.104\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$TDUPDATE\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("2453462.437\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$TDUUPDATE\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("2453462.104\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$TDINDWG\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0.000275741\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$TDUSRTIMER\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0.000268669\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$USRTIMER\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$ANGBASE\n"));
	myFile.WriteString(_T("50\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$ANGDIR\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PDMODE\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PDSIZE\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PLINEWID\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$SPLFRAME\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$SPLINETYPE\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("6\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$SPLINESEGS\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("8\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$HANDSEED\n"));
	myFile.WriteString(_T("5\n"));
	myFile.WriteString(_T("F4\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$SURFTAB1\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("6\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$SURFTAB2\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("6\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$SURFTYPE\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("6\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$SURFU\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("6\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$SURFV\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("6\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$UCSBASE\n"));
	myFile.WriteString(_T("2\n"));
	myFile.WriteString(_T("\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$UCSNAME\n"));
	myFile.WriteString(_T("2\n"));
	myFile.WriteString(_T("\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$UCSORG\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$UCSXDIR\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$UCSYDIR\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$UCSORTHOREF\n"));
	myFile.WriteString(_T("2\n"));
	myFile.WriteString(_T("\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$UCSORTHOVIEW	\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$UCSORGTOP\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$UCSORGBOTTOM\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$UCSORGLEFT\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$UCSORGRIGHT\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$UCSORGFRONT\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$UCSORGBACK\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PUCSBASE\n"));
	myFile.WriteString(_T("2\n"));
	myFile.WriteString(_T("\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PUCSNAME\n"));
	myFile.WriteString(_T("2\n"));
	myFile.WriteString(_T("\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PUCSORG\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PUCSXDIR\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PUCSYDIR\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PUCSORTHOREF\n"));
	myFile.WriteString(_T("2\n"));
	myFile.WriteString(_T("\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PUCSORTHOVIEW\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PUCSORGTOP\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PUCSORGBOTTOM\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PUCSORGLEFT\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PUCSORGRIGHT\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PUCSORGFRONT\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PUCSORGBACK\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$USERI1\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$USERI2\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$USERI3\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$USERI4\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$USERI5\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$USERR1\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$USERR2\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$USERR3\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$USERR4\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$USERR5\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$WORLDVIEW\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$SHADEDGE\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("3\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$SHADEDIF\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$TILEMODE\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$MAXACTVP\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("64\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PINSBASE\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PLIMCHECK\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PEXTMIN\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PEXTMAX\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("30\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PLIMMIN\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PLIMMAX\n"));
	myFile.WriteString(_T("10\n"));
	myFile.WriteString(_T("12\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$UNITMODE\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$VISRETAIN\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PLINEGEN\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PSLTSCALE\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$TREEDEPTH\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("3020\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$CMLSTYLE\n"));
	myFile.WriteString(_T("2\n"));
	myFile.WriteString(_T("Standard\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$CMLJUST\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$CMLSCALE\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("20\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PROXYGRAPHICS\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$MEASUREMENT\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$CELWEIGHT\n"));
	myFile.WriteString(_T("370\n"));
	myFile.WriteString(_T("-1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$ENDCAPS\n"));
	myFile.WriteString(_T("280\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$JOINSTYLE\n"));
	myFile.WriteString(_T("280\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$LWDISPLAY\n"));
	myFile.WriteString(_T("290\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$INSUNITS\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("4\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$HYPERLINKBASE\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$STYLESHEET\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$XEDIT\n"));
	myFile.WriteString(_T("290\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$CEPSNTYPE\n"));
	myFile.WriteString(_T("380\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PSTYLEMODE\n"));
	myFile.WriteString(_T("290\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$FINGERPRINTGUID\n"));
	myFile.WriteString(_T("2\n"));
	myFile.WriteString(_T("{CBDA57BF-9F61-4D07-B641-3F7BEA51C3E9}\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$VERSIONGUID\n"));
	myFile.WriteString(_T("2\n"));
	myFile.WriteString(_T("{D30780FE-E679-40F2-9B1E-30809D1A6F71}\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$EXTNAMES\n"));
	myFile.WriteString(_T("290\n"));
	myFile.WriteString(_T("1\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$PSVPSCALE\n"));
	myFile.WriteString(_T("40\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("9\n"));
	myFile.WriteString(_T("$OLESTARTUP\n"));
	myFile.WriteString(_T("290\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("ENDSEC\n"));
}


void CPolyline::WriteClasses(CStdioFile& myFile)
{
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("SECTION\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("CLASSES\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("CLASS\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("ACDBDICTIONARYWDFLT\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("AcDbDictionaryWithDefault\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("ObjectDBX Classes\n"));
	myFile.WriteString(
		_T("90\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("280\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("CLASS\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("ACDBPLACEHOLDER\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("AcDbPlaceHolder\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("ObjectDBX Classes\n"));
	myFile.WriteString(
		_T("90\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("280\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("CLASS\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("LAYOUT\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("AcDbLayout\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("ObjectDBX Classes\n"));
	myFile.WriteString(
		_T("90\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("280\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("CLASS\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("DICTIONARYVAR\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("AcDbDictionaryVar\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("ObjectDBX Classes\n"));
	myFile.WriteString(
		_T("90\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("280\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("CLASS\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("TABLESTYLE\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("AcDbTableStyle\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("ObjectDBX Classes\n"));
	myFile.WriteString(
		_T("90\n"));
	myFile.WriteString(
		_T("2047\n"));
	myFile.WriteString(
		_T("280\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("ENDSEC\n"));
}


void CPolyline::WriteTables(CStdioFile& myFile)
{
	//	CStdioFile myFile;
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("SECTION\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("TABLES\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("TABLE\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("VPORT\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("8\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTable\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("VPORT\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("EB\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("8\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTableRecord\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbViewportTableRecord\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("*Active\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("10\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("20\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("11\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("21\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("12\n"));
	myFile.WriteString(
		_T("965.8789\n"));
	myFile.WriteString(
		_T("22\n"));
	myFile.WriteString(
		_T("482.9395\n"));
	myFile.WriteString(
		_T("13\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("23\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("14\n"));
	myFile.WriteString(
		_T("10\n"));
	myFile.WriteString(
		_T("24\n"));
	myFile.WriteString(
		_T("10\n"));
	myFile.WriteString(
		_T("15\n"));
	myFile.WriteString(
		_T("10\n"));
	myFile.WriteString(
		_T("25\n"));
	myFile.WriteString(
		_T("10\n"));
	myFile.WriteString(
		_T("16\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("26\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("36\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("17\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("27\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("37\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("40\n"));
	myFile.WriteString(
		_T("4000.0\n"));
	myFile.WriteString(
		_T("41\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("42\n"));
	myFile.WriteString(
		_T("50\n"));
	myFile.WriteString(
		_T("43\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("44\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("50\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("51\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("71\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("72\n"));
	myFile.WriteString(
		_T("1000\n"));
	myFile.WriteString(
		_T("73\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("74\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("75\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("76\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("77\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("78\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("65\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("110\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("120\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("130\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("111\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("121\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("131\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("112\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("122\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("132\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("79\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("146\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("ENDTAB\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("TABLE\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("LTYPE\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTable\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("LTYPE\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("14\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTableRecord\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbLinetypeTableRecord\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("ByBlock\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T(" \n"));
	myFile.WriteString(
		_T("72\n"));
	myFile.WriteString(
		_T("65\n"));
	myFile.WriteString(
		_T("73\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("40\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("LTYPE\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("15\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTableRecord\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbLinetypeTableRecord\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("ByLayer\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T(" \n"));
	myFile.WriteString(
		_T("72\n"));
	myFile.WriteString(
		_T("65\n"));
	myFile.WriteString(
		_T("73\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("40\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("LTYPE\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("16\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTableRecord\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbLinetypeTableRecord\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("Continuous\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("Solid line\n"));
	myFile.WriteString(
		_T("72\n"));
	myFile.WriteString(
		_T("65\n"));
	myFile.WriteString(
		_T("73\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("40\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("ENDTAB\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("TABLE\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("LAYER\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTable\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("LAYER\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("10\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTableRecord\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbLayerTableRecord\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("62\n"));
	myFile.WriteString(
		_T("7\n"));
	myFile.WriteString(
		_T("6\n"));
	myFile.WriteString(
		_T("Continuous\n"));
	myFile.WriteString(
		_T("370\n"));
	myFile.WriteString(
		_T("-3\n"));
	myFile.WriteString(
		_T("390\n"));
	myFile.WriteString(
		_T("F\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("ENDTAB\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("TABLE\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("STYLE\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTable\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("STYLE\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("11\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTableRecord\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbTextStyleTableRecord\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("Standard\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("40\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("41\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("50\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("71\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("42\n"));
	myFile.WriteString(
		_T("2.5\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("txt\n"));
	myFile.WriteString(
		_T("4\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("ENDTAB\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("TABLE\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("VIEW\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("6\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTable\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("ENDTAB\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("TABLE\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("UCS\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("7\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTable\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("ENDTAB\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("TABLE\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("APPID\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("9\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTable\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("APPID\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("12\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("9\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTableRecord\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbRegAppTableRecord\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("ACAD\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("APPID\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("9E\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("9\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTableRecord\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbRegAppTableRecord\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("ACAD_PSEXT\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("ENDTAB\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("TABLE\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("DIMSTYLE\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("A\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTable\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbDimStyleTable\n"));
	myFile.WriteString(
		_T("71\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("340\n"));
	myFile.WriteString(
		_T("27\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("DIMSTYLE\n"));
	myFile.WriteString(
		_T("105\n"));
	myFile.WriteString(
		_T("27\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("A\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTableRecord\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbDimStyleTableRecord\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("ISO-25\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("41\n"));
	myFile.WriteString(
		_T("2.5\n"));
	myFile.WriteString(
		_T("42\n"));
	myFile.WriteString(
		_T("0.625\n"));
	myFile.WriteString(
		_T("43\n"));
	myFile.WriteString(
		_T("3.75\n"));
	myFile.WriteString(
		_T("44\n"));
	myFile.WriteString(
		_T("1.25\n"));
	myFile.WriteString(
		_T("73\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("74\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("77\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("78\n"));
	myFile.WriteString(
		_T("8\n"));
	myFile.WriteString(
		_T("140\n"));
	myFile.WriteString(
		_T("2.5\n"));
	myFile.WriteString(
		_T("141\n"));
	myFile.WriteString(
		_T("2.5\n"));
	myFile.WriteString(
		_T("143\n"));
	myFile.WriteString(
		_T("0.03937\n"));
	myFile.WriteString(
		_T("147\n"));
	myFile.WriteString(
		_T("0.625\n"));
	myFile.WriteString(
		_T("171\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("172\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("271\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("272\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("274\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("278\n"));
	myFile.WriteString(
		_T("44\n"));
	myFile.WriteString(
		_T("283\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("284\n"));
	myFile.WriteString(
		_T("8\n"));
	myFile.WriteString(
		_T("340\n"));
	myFile.WriteString(
		_T("11\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("ENDTAB\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("TABLE\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("BLOCK_RECORD\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTable\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("BLOCK_RECORD\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("1F\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTableRecord\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbBlockTableRecord\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("*Model_Space\n"));
	myFile.WriteString(
		_T("340\n"));
	myFile.WriteString(
		_T("22\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("BLOCK_RECORD\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("D2\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTableRecord\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbBlockTableRecord\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("*Paper_Space\n"));
	myFile.WriteString(
		_T("340\n"));
	myFile.WriteString(
		_T("D3\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("BLOCK_RECORD\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("D6\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbSymbolTableRecord\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbBlockTableRecord\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("*Paper_Space0\n"));
	myFile.WriteString(
		_T("340\n"));
	myFile.WriteString(
		_T("D7\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("ENDTAB\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("ENDSEC\n"));
}


void CPolyline::WriteBlocks(CStdioFile& myFile)
{
	//	CStdioFile myFile;
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("SECTION\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("BLOCKS\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("BLOCK\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("20\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("1F\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbEntity\n"));
	myFile.WriteString(
		_T("8\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbBlockBegin\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("*Model_Space\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("10\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("20\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("30\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("*Model_Space\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("ENDBLK\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("21\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("1F\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbEntity\n"));
	myFile.WriteString(
		_T("8\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbBlockEnd\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("BLOCK\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("D4\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("D2\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbEntity\n"));
	myFile.WriteString(
		_T("67\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("8\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbBlockBegin\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("*Paper_Space\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("10\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("20\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("30\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("*Paper_Space\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("ENDBLK\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("D5\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("D2\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbEntity\n"));
	myFile.WriteString(
		_T("67\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("8\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbBlockEnd\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("BLOCK\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("D8\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("D6\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbEntity\n"));
	myFile.WriteString(
		_T("8\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbBlockBegin\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("*Paper_Space0\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("10\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("20\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("30\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("*Paper_Space0\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("ENDBLK\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("D9\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("D6\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbEntity\n"));
	myFile.WriteString(
		_T("8\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbBlockEnd\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("ENDSEC\n"));
}


void CPolyline::WriteObjects(CStdioFile& myFile)
{
	//	CStdioFile myFile;
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("SECTION\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("OBJECTS\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("DICTIONARY\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbDictionary\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("ACAD_COLOR\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("6B\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("ACAD_GROUP\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("D\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("ACAD_LAYOUT\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("1A\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("ACAD_MATERIAL\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("6A\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("ACAD_MLINESTYLE\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("17\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("ACAD_PLOTSETTINGS\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("19\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("ACAD_PLOTSTYLENAME\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("E\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("ACAD_TABLESTYLE\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("7E\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("ACDBHEADERROUNDTRIPXREC\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("ED\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("AcDbVariableDictionary\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("5E\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("DWGPROPS\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("EC\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("DICTIONARY\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("6B\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbDictionary\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("DICTIONARY\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("D\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbDictionary\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("DICTIONARY\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("1A\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbDictionary\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("Layout1\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("D3\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("Layout2\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("D7\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("Model\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("22\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("DICTIONARY\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("6A\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbDictionary\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("DICTIONARY\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("17\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbDictionary\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("Standard\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("18\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("DICTIONARY\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("19\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbDictionary\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("ACDBDICTIONARYWDFLT\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("E\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbDictionary\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("Normal\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("F\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbDictionaryWithDefault\n"));
	myFile.WriteString(
		_T("340\n"));
	myFile.WriteString(
		_T("F\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("DICTIONARY\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("7E\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbDictionary\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("Standard\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("7F\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("DICTIONARY\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("ED\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbDictionary\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("DICTIONARY\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("5E\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbDictionary\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("CTABLESTYLE\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("84\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("DIMASSOC\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("5F\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("HIDETEXT\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("63\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("PROJECTNAME\n"));
	myFile.WriteString(
		_T("350\n"));
	myFile.WriteString(
		_T("EA\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("XRECORD\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("EC\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("C\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbXrecord\n"));
	myFile.WriteString(
		_T("280\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("DWGPROPS COOKIE\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("4\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("6\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("7\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("8\n"));
	myFile.WriteString(
		_T("Autodesk\n"));
	myFile.WriteString(
		_T("9\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("300\n"));
	myFile.WriteString(
		_T("=\n"));
	myFile.WriteString(
		_T("301\n"));
	myFile.WriteString(
		_T("=\n"));
	myFile.WriteString(
		_T("302\n"));
	myFile.WriteString(
		_T("=\n"));
	myFile.WriteString(
		_T("303\n"));
	myFile.WriteString(
		_T("=\n"));
	myFile.WriteString(
		_T("304\n"));
	myFile.WriteString(
		_T("=\n"));
	myFile.WriteString(
		_T("305\n"));
	myFile.WriteString(
		_T("=\n"));
	myFile.WriteString(
		_T("306\n"));
	myFile.WriteString(
		_T("=\n"));
	myFile.WriteString(
		_T("307\n"));
	myFile.WriteString(
		_T("=\n"));
	myFile.WriteString(
		_T("308\n"));
	myFile.WriteString(
		_T("=\n"));
	myFile.WriteString(
		_T("309\n"));
	myFile.WriteString(
		_T("=\n"));
	myFile.WriteString(
		_T("40\n"));
	myFile.WriteString(
		_T("0.000274\n"));
	myFile.WriteString(
		_T("41\n"));
	myFile.WriteString(
		_T("2453462\n"));
	myFile.WriteString(
		_T("42\n"));
	myFile.WriteString(
		_T("2453462\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("90\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("LAYOUT\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("D3\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_XDICTIONARY\n"));
	myFile.WriteString(
		_T("360\n"));
	myFile.WriteString(
		_T("F0\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("1A\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("1A\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbPlotSettings\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("C:\\Documents and Settings\\basas\\Application Data\\Autodesk\\AutoCAD 2005\\R16.1\\enu\\plotters\\Default Windows System Printer.pc3\n"));
	myFile.WriteString(
		_T("4\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("6\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("40\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("41\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("42\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("43\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("44\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("45\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("46\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("47\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("48\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("49\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("140\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("141\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("142\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("143\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("688\n"));
	myFile.WriteString(
		_T("72\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("73\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("74\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("7\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("75\n"));
	myFile.WriteString(
		_T("16\n"));
	myFile.WriteString(
		_T("147\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("148\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("149\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbLayout\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("Layout1\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("71\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("10\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("20\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("11\n"));
	myFile.WriteString(
		_T("12\n"));
	myFile.WriteString(
		_T("21\n"));
	myFile.WriteString(
		_T("9\n"));
	myFile.WriteString(
		_T("12\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("22\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("32\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("14\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("24\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("34\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("15\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("25\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("35\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("146\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("13\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("23\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("33\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("16\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("26\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("36\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("17\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("27\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("37\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("76\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("D2\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("LAYOUT\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("D7\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_XDICTIONARY\n"));
	myFile.WriteString(
		_T("360\n"));
	myFile.WriteString(
		_T("F2\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("1A\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("1A\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbPlotSettings\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("C:\\Documents and Settings\\basas\\Application Data\\Autodesk\\AutoCAD 2005\\R16.1\\enu\\plotters\\Default Windows System Printer.pc3\n"));
	myFile.WriteString(
		_T("4\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("6\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("40\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("41\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("42\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("43\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("44\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("45\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("46\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("47\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("48\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("49\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("140\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("141\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("142\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("143\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("688\n"));
	myFile.WriteString(
		_T("72\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("73\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("74\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("7\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("75\n"));
	myFile.WriteString(
		_T("16\n"));
	myFile.WriteString(
		_T("147\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("148\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("149\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbLayout\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("Layout2\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("71\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("10\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("20\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("11\n"));
	myFile.WriteString(
		_T("12\n"));
	myFile.WriteString(
		_T("21\n"));
	myFile.WriteString(
		_T("9\n"));
	myFile.WriteString(
		_T("12\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("22\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("32\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("14\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("24\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("34\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("15\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("25\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("35\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("146\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("13\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("23\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("33\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("16\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("26\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("36\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("17\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("27\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("37\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("76\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("D6\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("LAYOUT\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("22\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_XDICTIONARY\n"));
	myFile.WriteString(
		_T("360\n"));
	myFile.WriteString(
		_T("EE\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("1A\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("1A\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbPlotSettings\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("none_device\n"));
	myFile.WriteString(
		_T("4\n"));
	myFile.WriteString(
		_T("ISO_A4_(210.00_x_297.00_MM)\n"));
	myFile.WriteString(
		_T("6\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("40\n"));
	myFile.WriteString(
		_T("7.5\n"));
	myFile.WriteString(
		_T("41\n"));
	myFile.WriteString(
		_T("20\n"));
	myFile.WriteString(
		_T("42\n"));
	myFile.WriteString(
		_T("7.5\n"));
	myFile.WriteString(
		_T("43\n"));
	myFile.WriteString(
		_T("20\n"));
	myFile.WriteString(
		_T("44\n"));
	myFile.WriteString(
		_T("210\n"));
	myFile.WriteString(
		_T("45\n"));
	myFile.WriteString(
		_T("297\n"));
	myFile.WriteString(
		_T("46\n"));
	myFile.WriteString(
		_T("11.55\n"));
	myFile.WriteString(
		_T("47\n"));
	myFile.WriteString(
		_T("-13.65\n"));
	myFile.WriteString(
		_T("48\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("49\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("140\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("141\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("142\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("143\n"));
	myFile.WriteString(
		_T("8.704085\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("11952\n"));
	myFile.WriteString(
		_T("72\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("73\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("74\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("7\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("75\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("147\n"));
	myFile.WriteString(
		_T("0.114889\n"));
	myFile.WriteString(
		_T("148\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("149\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbLayout\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("Model\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("71\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("10\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("20\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("11\n"));
	myFile.WriteString(
		_T("12\n"));
	myFile.WriteString(
		_T("21\n"));
	myFile.WriteString(
		_T("9\n"));
	myFile.WriteString(
		_T("12\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("22\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("32\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("14\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("24\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("34\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("15\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("25\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("35\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("146\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("13\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("23\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("33\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("16\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("26\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("36\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("17\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("27\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("37\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("76\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("1F\n"));
	myFile.WriteString(
		_T("1001\n"));
	myFile.WriteString(
		_T("ACAD_PSEXT\n"));
	myFile.WriteString(
		_T("1000\n"));
	myFile.WriteString(
		_T("None\n"));
	myFile.WriteString(
		_T("1000\n"));
	myFile.WriteString(
		_T("None\n"));
	myFile.WriteString(
		_T("1000\n"));
	myFile.WriteString(
		_T("Not applicable\n"));
	myFile.WriteString(
		_T("1000\n"));
	myFile.WriteString(
		_T("The layout will not be plotted unless PatchArray new plotter configuration name is selected.\n"));
	myFile.WriteString(
		_T("1070\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("MLINESTYLE\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("18\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("17\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("17\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbMlineStyle\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("STANDARD\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("62\n"));
	myFile.WriteString(
		_T("256\n"));
	myFile.WriteString(
		_T("51\n"));
	myFile.WriteString(
		_T("90\n"));
	myFile.WriteString(
		_T("52\n"));
	myFile.WriteString(
		_T("90\n"));
	myFile.WriteString(
		_T("71\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("49\n"));
	myFile.WriteString(
		_T("0.5\n"));
	myFile.WriteString(
		_T("62\n"));
	myFile.WriteString(
		_T("256\n"));
	myFile.WriteString(
		_T("6\n"));
	myFile.WriteString(
		_T("BYLAYER\n"));
	myFile.WriteString(
		_T("49\n"));
	myFile.WriteString(
		_T("-0.5\n"));
	myFile.WriteString(
		_T("62\n"));
	myFile.WriteString(
		_T("256\n"));
	myFile.WriteString(
		_T("6\n"));
	myFile.WriteString(
		_T("BYLAYER\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("ACDBPLACEHOLDER\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("F\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("E\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("E\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("TABLESTYLE\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("7F\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("7E\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("7E\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbTableStyle\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("Standard\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("71\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("40\n"));
	myFile.WriteString(
		_T("1.5\n"));
	myFile.WriteString(
		_T("41\n"));
	myFile.WriteString(
		_T("1.5\n"));
	myFile.WriteString(
		_T("280\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("7\n"));
	myFile.WriteString(
		_T("Standard\n"));
	myFile.WriteString(
		_T("140\n"));
	myFile.WriteString(
		_T("4.5\n"));
	myFile.WriteString(
		_T("170\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("62\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("63\n"));
	myFile.WriteString(
		_T("7\n"));
	myFile.WriteString(
		_T("283\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("274\n"));
	myFile.WriteString(
		_T("-2\n"));
	myFile.WriteString(
		_T("284\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("64\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("275\n"));
	myFile.WriteString(
		_T("-2\n"));
	myFile.WriteString(
		_T("285\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("65\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("276\n"));
	myFile.WriteString(
		_T("-2\n"));
	myFile.WriteString(
		_T("286\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("66\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("277\n"));
	myFile.WriteString(
		_T("-2\n"));
	myFile.WriteString(
		_T("287\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("67\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("278\n"));
	myFile.WriteString(
		_T("-2\n"));
	myFile.WriteString(
		_T("288\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("68\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("279\n"));
	myFile.WriteString(
		_T("-2\n"));
	myFile.WriteString(
		_T("289\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("69\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("7\n"));
	myFile.WriteString(
		_T("Standard\n"));
	myFile.WriteString(
		_T("140\n"));
	myFile.WriteString(
		_T("6\n"));
	myFile.WriteString(
		_T("170\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("62\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("63\n"));
	myFile.WriteString(
		_T("7\n"));
	myFile.WriteString(
		_T("283\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("274\n"));
	myFile.WriteString(
		_T("-2\n"));
	myFile.WriteString(
		_T("284\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("64\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("275\n"));
	myFile.WriteString(
		_T("-2\n"));
	myFile.WriteString(
		_T("285\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("65\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("276\n"));
	myFile.WriteString(
		_T("-2\n"));
	myFile.WriteString(
		_T("286\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("66\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("277\n"));
	myFile.WriteString(
		_T("-2\n"));
	myFile.WriteString(
		_T("287\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("67\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("278\n"));
	myFile.WriteString(
		_T("-2\n"));
	myFile.WriteString(
		_T("288\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("68\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("279\n"));
	myFile.WriteString(
		_T("-2\n"));
	myFile.WriteString(
		_T("289\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("69\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("7\n"));
	myFile.WriteString(
		_T("Standard\n"));
	myFile.WriteString(
		_T("140\n"));
	myFile.WriteString(
		_T("4.5\n"));
	myFile.WriteString(
		_T("170\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("62\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("63\n"));
	myFile.WriteString(
		_T("7\n"));
	myFile.WriteString(
		_T("283\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("274\n"));
	myFile.WriteString(
		_T("-2\n"));
	myFile.WriteString(
		_T("284\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("64\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("275\n"));
	myFile.WriteString(
		_T("-2\n"));
	myFile.WriteString(
		_T("285\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("65\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("276\n"));
	myFile.WriteString(
		_T("-2\n"));
	myFile.WriteString(
		_T("286\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("66\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("277\n"));
	myFile.WriteString(
		_T("-2\n"));
	myFile.WriteString(
		_T("287\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("67\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("278\n"));
	myFile.WriteString(
		_T("-2\n"));
	myFile.WriteString(
		_T("288\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("68\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("279\n"));
	myFile.WriteString(
		_T("-2\n"));
	myFile.WriteString(
		_T("289\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("69\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("DICTIONARYVAR\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("84\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("5E\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("5E\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("DictionaryVariables\n"));
	myFile.WriteString(
		_T("280\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("STANDARD\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("DICTIONARYVAR\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("5F\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("5E\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("5E\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("DictionaryVariables\n"));
	myFile.WriteString(
		_T("280\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("DICTIONARYVAR\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("63\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("5E\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("5E\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("DictionaryVariables\n"));
	myFile.WriteString(
		_T("280\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("DICTIONARYVAR\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("EA\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("5E\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("5E\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("DictionaryVariables\n"));
	myFile.WriteString(
		_T("280\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("DICTIONARY\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("F0\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("D3\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbDictionary\n"));
	myFile.WriteString(
		_T("280\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("ACAD_XREC_ROUNDTRIP\n"));
	myFile.WriteString(
		_T("360\n"));
	myFile.WriteString(
		_T("F1\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("DICTIONARY\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("F2\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("D7\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbDictionary\n"));
	myFile.WriteString(
		_T("280\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("ACAD_XREC_ROUNDTRIP\n"));
	myFile.WriteString(
		_T("360\n"));
	myFile.WriteString(
		_T("F3\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("DICTIONARY\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("EE\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("22\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbDictionary\n"));
	myFile.WriteString(
		_T("280\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("281\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("3\n"));
	myFile.WriteString(
		_T("ACAD_XREC_ROUNDTRIP\n"));
	myFile.WriteString(
		_T("360\n"));
	myFile.WriteString(
		_T("EF\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("XRECORD\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("F1\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("F0\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("F0\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbXrecord\n"));
	myFile.WriteString(
		_T("280\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("SHADEPLOT\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("SHADEPLOTRESLEVEL\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("SHADEPLOTCUSTOMDPI\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("300\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("XRECORD\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("F3\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("F2\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("F2\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbXrecord\n"));
	myFile.WriteString(
		_T("280\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("SHADEPLOT\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("SHADEPLOTRESLEVEL\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("SHADEPLOTCUSTOMDPI\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("300\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("XRECORD\n"));
	myFile.WriteString(
		_T("5\n"));
	myFile.WriteString(
		_T("EF\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("{ACAD_REACTORS\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("EE\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("}\n"));
	myFile.WriteString(
		_T("330\n"));
	myFile.WriteString(
		_T("EE\n"));
	myFile.WriteString(
		_T("100\n"));
	myFile.WriteString(
		_T("AcDbXrecord\n"));
	myFile.WriteString(
		_T("280\n"));
	myFile.WriteString(
		_T("1\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("SHADEPLOT\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("SHADEPLOTRESLEVEL\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("102\n"));
	myFile.WriteString(
		_T("SHADEPLOTCUSTOMDPI\n"));
	myFile.WriteString(
		_T("70\n"));
	myFile.WriteString(
		_T("300\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("ENDSEC\n"));
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("EOF\n"));
}



void CPolyline::WriteEntities(CStdioFile &myFile, BOOL bSaveMinRectLayout/* =FALSE */)
{
	myFile.WriteString(
		_T("0\n"));
	myFile.WriteString(
		_T("SECTION\n"));
	myFile.WriteString(
		_T("2\n"));
	myFile.WriteString(
		_T("ENTITIES\n"));
	// ******************************************
	CString str;
	int e=5;

	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("LWPOLYLINE\n"));
	myFile.WriteString(_T("5\n"));
	str.Format(_T("E%d"),e); e++;
	myFile.WriteString(str+_T("\n"));
	myFile.WriteString(_T("330\n"));
	myFile.WriteString(_T("1F\n"));
	myFile.WriteString(_T("100\n"));
	myFile.WriteString(_T("AcDbEntity\n"));
	myFile.WriteString(_T("8\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("100\n"));
	myFile.WriteString(_T("AcDbPolyline\n"));
	myFile.WriteString(_T("90\n"));
	int number_point=m_pts.GetSize();
	str.Format(_T("%d"),number_point);
	myFile.WriteString(str+_T("\n"));
	myFile.WriteString(_T("70\n"));
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("43\n"));
	myFile.WriteString(_T("0.0\n"));
	for(int j=0;j<number_point;j++)
	{
		myFile.WriteString(_T("10\n"));
		str.Format(_T("%f\n"),m_pts[j]->x);
		myFile.WriteString(str);
		myFile.WriteString(_T("20\n"));
		str.Format(_T("%f\n"),m_pts[j]->y);
		myFile.WriteString(str);
	}	


	// ******************************************
	myFile.WriteString(_T("0\n"));
	myFile.WriteString(_T("ENDSEC\n"));
}


