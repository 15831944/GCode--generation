#include "StdAfx.h"
#include "BSpline.h"
//#include <boost/numeric/mtl/mtl.hpp>
#include <afxmt.h>
#include <iostream>
using namespace std;
//using namespace mtl;


CMutex g_mutex;

CBSpline::CBSpline(void)
{
	m_degree=3;
	m_bCanDraw=TRUE;
	m_bCanFit=TRUE;
}

CBSpline::~CBSpline(void)
{
	m_u.RemoveAll();

	int sz=m_ctrls.GetSize();
	for (int i=0;i<sz;i++)
	{
		delete m_ctrls[i];
		m_ctrls[i]=NULL;
	}
	m_ctrls.RemoveAll();

	sz=m_fits.GetSize();
	for (int i=0;i<sz;i++)
	{
		delete m_fits[i];
		m_fits[i]=NULL;
	}
	m_fits.RemoveAll();

	sz=m_pts.GetSize();
	for (int i=0;i<sz;i++)
	{
		delete m_pts[i];
		m_pts[i]=NULL;
	}
	m_pts.RemoveAll();
}

CBSpline::CBSpline(const CBSpline& spline)
{
// 	numOfCtrlPt=spline.numOfCtrlPt;
// 	numOfFitPt=spline.numOfFitPt;
// 	u=new double[numOfCtrlPt+5];
// 	for(int i=0;i<=numOfCtrlPt+5;i++)
// 		u[i]=spline.u[i];
// 	 int sz=m_ctrlPts.GetSize();
// 	 for(int i=0;i<sz;i++)
// 	 {
// 		 CPoint3D* pPt=new CPoint3D(*spline.m_ctrlPts[i]);
// 		 m_ctrlPts.Add(pPt);
// 	 }
}

const CBSpline& CBSpline::operator =(const CBSpline& spline)
{
// 	numOfCtrlPt=spline.numOfCtrlPt;
// 	numOfFitPt=spline.numOfFitPt;
// 	u=new double[numOfCtrlPt+5];
// 	for(int i=0;i<=numOfCtrlPt+4;i++)
// 		u[i]=spline.u[i];
// 	int sz=m_ctrlPts.GetSize();
// 	for(int i=0;i<sz;i++)
// 	{
// 		CPoint3D* pPt=new CPoint3D(*spline.m_ctrlPts[i]);
// 		m_ctrlPts.Add(pPt);
// 	}

	return *this;
}

void CBSpline::SetDegree(int degree)
{
	m_degree=degree;
}

int CBSpline::AddCtrlPts(CPoint3D* pPt)
{
	return m_ctrls.Add(pPt);
}

int CBSpline::AddKnot(double uu)
{
	return m_u.Add(uu);
}

int CBSpline::AddFitPts(CPoint3D *pPt)
{
	return m_fits.Add(pPt);
}

void CBSpline::SetLastFitPt(const CPoint3D& pt)
{
	int sz=m_fits.GetSize();
	if (sz>0)
		*m_fits[sz-1]=pt;
}

BOOL CBSpline::Interpolate()
{
	m_u.RemoveAll();
	int sz=m_ctrls.GetSize();
	for (int i=0;i<sz;i++)
	{
		delete m_ctrls[i];
		m_ctrls[i]=NULL;
	}
	m_ctrls.RemoveAll();

	int szFitPts=m_fits.GetSize();
	int m,n;
	if (3==m_degree)  // 3次的情况
	{
		CPoint3D* pPt=NULL;
		m=szFitPts-1;               // 型值点   q0,q1...qm
		n=m+2;                      // 控制顶点 d0,d1...dn  // n=m+k-1

		if (-1==m)
		{
			AfxMessageBox(_T("No fit point!"));

			return FALSE;
		}
		if (0==m)  // 点
		{
			pPt=new CPoint3D(*m_fits[0]);
			m_ctrls.Add(pPt);
			return TRUE;
		}
		if (1==m)  // 直线
		{
			pPt=new CPoint3D(*m_fits[0]);
			m_ctrls.Add(pPt);
			pPt=new CPoint3D(*m_fits[1]);
			m_ctrls.Add(pPt);
			return TRUE;
		}
		if (2==m)  // 抛物线
		{
			pPt=new CPoint3D(*m_fits[0]);
			m_ctrls.Add(pPt);

			double len01=(*m_fits[1]-*m_fits[0]).GetLength();
			double len12=(*m_fits[2]-*m_fits[1]).GetLength();
			double t=len01/(len01+len12);
			CPoint3D ptOrg(0,0,0);

			pPt=new CPoint3D(ptOrg+((*m_fits[1]-ptOrg)-(*m_fits[0]-ptOrg)*(1-t)*(1-t)
				-(*m_fits[2]-ptOrg)*t*t)*(2/t/(1-t)));
			m_ctrls.Add(pPt);

			pPt=new CPoint3D(*m_fits[2]);
			m_ctrls.Add(pPt);

			return TRUE;
		}
		if (m>=3)
		{
			m_u.SetSize(n+5);

			m_u[0]=m_u[1]=m_u[2]=m_u[3]=0;
			m_u[n+4]=m_u[n+3]=m_u[n+2]=1;

			double *len=new double[m];
			CVector3D vec;
			vec=*m_fits[1]-*m_fits[0];
			len[0]=vec.GetLength();
			for (int i=1;i<m;i++)
			{
				vec=*m_fits[i+1]-*m_fits[i];
				len[i]=len[i-1]+vec.GetLength();	
			}
			for (int i=4;i<=n+1;i++)
			{
				m_u[i]=len[i-4]/len[m-1];
			}
			delete []len;

			CPoint3D* ctrPoint=new CPoint3D[n+1]; // 控制顶点

			double *a=new double[n];
			double *b=new double[n];
			double *c=new double[n];
			double *e_x=new double[n];
			double *e_y=new double[n];
			double *e_z=new double[n];
			for(int i=2;i<=n-2;i++)
			{
				double delta0=m_u[i+1]-m_u[i];
				double delta1=m_u[i+2]-m_u[i+1];
				double delta2=m_u[i+3]-m_u[i+2];
				double delta3=m_u[i+4]-m_u[i+3];
				double delta012=m_u[i+3]-m_u[i];
				double delta123=m_u[i+4]-m_u[i+1];
				a[i]=delta2*delta2/delta012;
				b[i]=delta2*(m_u[i+2]-m_u[i])/delta012+delta1*(m_u[i+4]-m_u[i+2])/delta123;
				c[i]=delta1*delta1/delta123;
				e_x[i]=m_fits[i-1]->x*(m_u[i+3]-m_u[i+1]);
				e_y[i]=m_fits[i-1]->y*(m_u[i+3]-m_u[i+1]);
				e_z[i]=m_fits[i-1]->z*(m_u[i+3]-m_u[i+1]);
			}

			// 添加边界条件---自由端条件 // 参见施法中的那本书
			double delta34=m_u[5]-m_u[3];
			double delta345=m_u[6]-m_u[3];
			a[1]=2-Delta(3)*Delta(4)/delta34/delta34;
			b[1]=Delta(3)*(Delta(4)/delta34-Delta(3)/delta345)/delta34;
			c[1]=Delta(3)*Delta(3)/delta34/delta345;
			e_x[1]=m_fits[0]->x+m_fits[1]->x;
			e_y[1]=m_fits[0]->y+m_fits[1]->y;
			e_z[1]=m_fits[0]->z+m_fits[1]->z;

			double delta_n2=m_u[n+1]-m_u[n-1];
			double delta_n3=m_u[n+1]-m_u[n-2];
			a[n-1]=-Delta(n)*Delta(n)/delta_n2/delta_n3;
			b[n-1]=Delta(n)*(Delta(n)/delta_n3-Delta(n-1)/delta_n2)/delta_n2;
			c[n-1]=Delta(n-1)*Delta(n)/delta_n2/delta_n2-2;
			e_x[n-1]=-m_fits[m]->x-m_fits[m-1]->x;
			e_y[n-1]=-m_fits[m]->y-m_fits[m-1]->y;
			e_z[n-1]=-m_fits[m]->z-m_fits[m-1]->z;
			// 添加边界条件

			double *p=new double[n-1];
			double *q_x=new double[n-1];
			double *q_y=new double[n-1];
			double *q_z=new double[n-1];
			double temp;
			temp=b[2]/a[2]-b[1]/a[1];
			p[2]=(c[1]/a[1]-c[2]/a[2])/temp;
			q_x[2]=(e_x[2]/a[2]-e_x[1]/a[1])/temp;   
			q_y[2]=(e_y[2]/a[2]-e_y[1]/a[1])/temp;
			q_z[2]=(e_z[2]/a[2]-e_z[1]/a[1])/temp;

			for (int i=3;i<=n-2;i++)
			{
				temp=a[i]*p[i-1]+b[i];
				p[i]=-c[i]/temp;
				q_x[i]=(e_x[i]-a[i]*q_x[i-1])/temp;
				q_y[i]=(e_y[i]-a[i]*q_y[i-1])/temp;
				q_z[i]=(e_z[i]-a[i]*q_z[i-1])/temp;
			}
			temp=(a[n-1]*p[n-3]+b[n-1])*p[n-2]+c[n-1];
			ctrPoint[n-1].x=(e_x[n-1]-a[n-1]*q_x[n-3]-(a[n-1]*p[n-3]+b[n-1])*q_x[n-2])/temp;
			ctrPoint[n-1].y=(e_y[n-1]-a[n-1]*q_y[n-3]-(a[n-1]*p[n-3]+b[n-1])*q_y[n-2])/temp;
			ctrPoint[n-1].z=(e_z[n-1]-a[n-1]*q_z[n-3]-(a[n-1]*p[n-3]+b[n-1])*q_z[n-2])/temp;

			for (int i=n-2;i>=2;i--)
			{
				ctrPoint[i].x=p[i]*ctrPoint[i+1].x+q_x[i];
				ctrPoint[i].y=p[i]*ctrPoint[i+1].y+q_y[i];
				ctrPoint[i].z=p[i]*ctrPoint[i+1].z+q_z[i];
			}
			ctrPoint[1].x=(e_x[1]-b[1]*ctrPoint[2].x-c[1]*ctrPoint[3].x)/a[1];
			ctrPoint[1].y=(e_y[1]-b[1]*ctrPoint[2].y-c[1]*ctrPoint[3].y)/a[1];
			ctrPoint[1].z=(e_z[1]-b[1]*ctrPoint[2].z-c[1]*ctrPoint[3].z)/a[1];

			pPt=new CPoint3D(*m_fits[0]);
			m_ctrls.Add(pPt);
			for (int i=1;i<=n-1;i++)
			{
				pPt=new CPoint3D(ctrPoint[i]);
				m_ctrls.Add(pPt);
			}
			pPt=new CPoint3D(*m_fits[m]);
			m_ctrls.Add(pPt);

			delete []a;
			delete []b;
			delete []c;
			delete []e_x;
			delete []e_y;
			delete []e_z;
			delete []p;
			delete []q_x;
			delete []q_y;
			delete []q_z;
			delete []ctrPoint;

			return TRUE;
		}
	}

	if (1==m_degree)  // 当曲线是1次的情况, 还没写好
	{
		int sz=m_fits.GetSize();
		m=sz-1;   // 型值点 P0,P1...Pm
		n=m;

		if (sz>=2)
		{
			//u=new double[n+1];
			m_u.SetSize(n+1);

			m_u[0]=0;
			m_u[n]=1;

			double *len=new double[n];
			CVector3D vec;
			vec=*m_fits[1]-*m_fits[0];
			len[0]=vec.GetLength();
			for (int i=1;i<=n-1;i++)
			{
				vec=*m_fits[i+1]-*m_fits[i];
				len[i]=len[i-1]+vec.GetLength();	
			}
			for (int i=1;i<=n-1;i++)
			{
				m_u[i]=len[i-1]/len[n-1];
			}
			delete []len;

			int sz=m_fits.GetSize();
			for (int i=0;i<sz;i++)
			{
				CPoint3D* pPt=new CPoint3D(*m_fits[i]);
				m_ctrls.Add(pPt);
			}

			return TRUE;
		}

		if (1==sz)
		{
			CPoint3D* pPt=new CPoint3D(*m_fits[0]);
			m_ctrls.Add(pPt);

			return TRUE;
		}

		if (0==sz)
		{
			AfxMessageBox(_T("No fit point!"));

			return FALSE;
		} 
	}
	return FALSE;
}
//
//BOOL CBSpline::Approximate(int numOfCtrlPts, int degree/* =3 */)
//{
//	int upperIndexCtrlPts=numOfCtrlPts-1;
//	int upperIndexKnots=upperIndexCtrlPts+degree+1;
//
//	SetKnotUpperIndex(upperIndexCtrlPts+degree+1);
//
//	// 均匀细分节点矢量
//	for (int i=0;i<=degree;i++)
//		m_u[i]=0;
//	for (int i=upperIndexCtrlPts+1;i<=upperIndexCtrlPts+degree+1;i++)
//		m_u[i]=1;
//	double knotStep=1.0/(upperIndexCtrlPts-degree+1.0);
//	for (int i=degree+1;i<=upperIndexCtrlPts;i++)
//		m_u[i]=m_u[i-1]+knotStep;
//
//	int szFitPts=m_fits.GetSize();
//	double* u=new double[szFitPts];
//
//	u[0]=0.0;
//	for (int i=1;i<szFitPts;i++)
//		u[i]=sqrt(GetDistance(*m_fits[i],*m_fits[i-1]))+u[i-1];
//	for (int i=0;i<szFitPts;i++)
//		u[i]/=u[szFitPts-1];
//	u[szFitPts-1]=1.0;
//
//	// 找到每个型值点所在节点矢量的位置
//	int* knotIndex=new int[szFitPts];
//	for (int i=0;i<szFitPts;i++)
//	{
//		for(int j=degree;j<=upperIndexCtrlPts+1;j++)
//		{
//			if (u[i]>=m_u[j] && u[i]<=m_u[j+1])
//			{
//				knotIndex[i]=j;
//				break;
//			}
//		}
//	}
//
//	typedef dense2D<double> SparseMatrix;
//	SparseMatrix M(szFitPts,upperIndexCtrlPts+1),Mt,M1,M2,denseMat;
//	M=0.0;
//	{
//		matrix::inserter<SparseMatrix> ins(M);
//		ins(0,0)<<1.0;
//		for (int row=1;row<szFitPts-1;row++) // consider only degree=3
//		{
//			for (int j=0;j<=degree;j++)
//			{
//				ins(row,knotIndex[row]-j) << N3(knotIndex[row]-j,u[row]);
//			}
//		}
//		ins(szFitPts-1,upperIndexCtrlPts)<<1.0;
//	}
//	Mt=trans(M);
//	M1=Mt*M;
//	M2=inv(M1);
//	denseMat=M2*Mt;
//
//	for (int i=0;i<=upperIndexCtrlPts;i++)
//	{
//		CPoint3D* pPt=new CPoint3D(0,0,0);
//		for (int j=0;j<szFitPts;j++)
//		{
//			*pPt = (*m_fits[j])*denseMat[i][j]+ (*pPt);
//		}
//		m_ctrls.Add(pPt);
//	}
//	delete []u;
//	delete []knotIndex;
//	return TRUE;
//}


/*  // 用于测试矩阵
    CStdioFile file(_T("D:\\matrix.txt"),CFile::modeCreate|CFile::modeWrite);
	CString str;
	for (int i=0;i<szFitPts;i++)
	{
		str=_T("{");
		for (int j=0;j<=upperIndexCtrlPts;j++)
		{
			CString str1;
			str1.Format(_T("%.2f"),NM[i][j]);
			if (j==upperIndexCtrlPts)
			{
				str=str+_T("\"")+str1+_T("\"")+_T("}\n");
			}
			else
			{
				str=str+_T("\"")+str1+_T("\"")+_T(", ");
			}
		}
		file.WriteString(str);
	}
	file.Close();
*/

// void CBSpline::SetKnotUpperIndex(int upperIndex)
// {
// 	m_u.SetSize(upperIndex+1);
// }

double CBSpline::Delta(int i)
{
	return m_u[i+1]-m_u[i];
}

// int CBSpline::GetKnotIndex(double uu) const
// {
// 
// 	return 1;
// }

CPoint3D CBSpline::P(double t) const
{
	int szCtrlPts=m_ctrls.GetSize();
	int n=szCtrlPts-1;    // d0, d1, d2, ... , dn
	int szFitPts=m_fits.GetSize();
	int m=szFitPts-1;     // q0, q1, q2, ... , qm

	if (3==m_degree)
	{
		if (m>=3 || -1==m)   // when m==-1, it suggests that the data is from file
		{
			if (0==t) return *m_ctrls[0];
			if (1==t) return *m_ctrls[n]; //

			int iStart;
			for(int i=3;i<=n+1;i++)
			{
				if (t>=m_u[i] && t<m_u[i+1])
				{
					iStart=i;
					break;
				}
			}

			CVector3D vec_i,vec_i1,vec_i2,vec_i3;
			CPoint3D ptOrg(0,0,0);
			vec_i=*m_ctrls[iStart]-ptOrg;
			vec_i1=*m_ctrls[iStart-1]-ptOrg;
			vec_i2=*m_ctrls[iStart-2]-ptOrg;
			vec_i3=*m_ctrls[iStart-3]-ptOrg;

			return ptOrg+vec_i3*N3(iStart-3,t)+vec_i2*N3(iStart-2,t)+vec_i1*N3(iStart-1,t)+vec_i*N3(iStart,t);
		}
		if (0==m)
		{
			return *m_ctrls[0];
		}
		if (1==m)
		{
			CVector3D vec=*m_ctrls[1]-*m_ctrls[0];
			return *m_ctrls[0]+vec*t;
		}
		if (2==m)
		{
			CPoint3D ptOrg(0,0,0);
			CVector3D vec0=*m_ctrls[0]-ptOrg;
			CVector3D vec1=*m_ctrls[1]-ptOrg;
			CVector3D vec2=*m_ctrls[2]-ptOrg;
			return ptOrg+vec0*(1-t)*(1-t)+vec1*0.5*t*(1-t)+vec2*t*t;
		}
	}

	if (1==m_degree)
	{
		if (0==m)
		{
			return *m_ctrls[0];
		}
		if (1==t)
		{
			return *m_ctrls[m];
		}
		int iStart;
		for (int i=0;i<=m-1;i++)
		{
			if (t>=m_u[i] && t<m_u[i+1])
			{
				iStart=i;
				break;
			}
		}
		CPoint3D ptOrg(0,0,0);
		double beta=(t-m_u[iStart])/(m_u[iStart+1]-m_u[iStart]);
		return ptOrg+(*m_ctrls[iStart]-ptOrg)*(1-beta)+(*m_ctrls[iStart+1]-ptOrg)*beta;
	}

	return CPoint3D(0,0,0);
}

CVector3D CBSpline::dP(double t) const
{
	int szCtrlPts=m_ctrls.GetSize();
	int m=szCtrlPts-1;

	int s=0;
	for(int i=3;i<=m+1;i++)
	{
		if (t>=m_u[i] && t<m_u[i+1])
		{
			s=i;
			break;
		}
	}

	return (*m_ctrls[s-3])*dN3(s-3,t)
		 + (*m_ctrls[s-2])*dN3(s-2,t)
		 + (*m_ctrls[s-1])*dN3(s-1,t)
		 + (*m_ctrls[s])*dN3(s,t)
		 - CPoint3D(0,0,0);
}

// 求3次B样条基
double CBSpline::N3(int i,double uu) const
{
	if (uu>=m_u[i] && uu<m_u[i+1])
	{
		double temp=uu-m_u[i];
		return temp*temp*temp/((m_u[i+3]-m_u[i])*(m_u[i+2]-m_u[i])*(m_u[i+1]-m_u[i]));
	}

	if (uu>=m_u[i+1] && uu<m_u[i+2])
	{
		return (uu-m_u[i])*(uu-m_u[i])*(m_u[i+2]-uu)/(m_u[i+3]-m_u[i])/(m_u[i+2]-m_u[i])/(m_u[i+2]-m_u[i+1])
			  +(uu-m_u[i])*(m_u[i+3]-uu)*(uu-m_u[i+1])/(m_u[i+3]-m_u[i])/(m_u[i+3]-m_u[i+1])/(m_u[i+2]-m_u[i+1])
			  +(uu-m_u[i+1])*(uu-m_u[i+1])*(m_u[i+4]-uu)/(m_u[i+4]-m_u[i+1])/(m_u[i+3]-m_u[i+1])/(m_u[i+2]-m_u[i+1]);
	}

	if (uu>=m_u[i+2] && uu<m_u[i+3])
	{
		return (m_u[i+3]-uu)*(m_u[i+3]-uu)*(uu-m_u[i])/(m_u[i+3]-m_u[i])/(m_u[i+3]-m_u[i+1])/(m_u[i+3]-m_u[i+2])
			+(uu-m_u[i+1])*(m_u[i+3]-uu)*(m_u[i+4]-uu)/(m_u[i+3]-m_u[i+1])/(m_u[i+3]-m_u[i+2])/(m_u[i+4]-m_u[i+1])
			+(m_u[i+4]-uu)*(m_u[i+4]-uu)*(uu-m_u[i+2])/(m_u[i+4]-m_u[i+1])/(m_u[i+4]-m_u[i+2])/(m_u[i+3]-m_u[i+2]);
	}

	if (uu>=m_u[i+3] && uu<=m_u[i+4]                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            )
	{
		double temp=m_u[i+4]-uu;
		return temp*temp*temp/(m_u[i+4]-m_u[i+1])/(m_u[i+4]-m_u[i+2])/(m_u[i+4]-m_u[i+3]);
	}

	return 0;
}

// 2次B样条基
double CBSpline::N2(int i, double uu) const 
{
	if (uu>=m_u[i] && uu<m_u[i+1])
	{
		return (uu-m_u[i])*(uu-m_u[i])/(m_u[i+2]-m_u[i])/(m_u[i+1]-m_u[i]);
	}

	if (uu>=m_u[i+1] && uu<m_u[i+2])
	{
		return (uu-m_u[i])*(m_u[i+2]-uu)/(m_u[i+2]-m_u[i])/(m_u[i+2]-m_u[i+1])
			  +(m_u[i+3]-uu)*(uu-m_u[i+1])/(m_u[i+3]-m_u[i+1])/(m_u[i+2]-m_u[i+1]);
	}

	if (uu>=m_u[i+2] && uu<m_u[i+3])
	{
		return (m_u[i+3]-uu)*(m_u[i+3]-uu)/(m_u[i+3]-m_u[i+1])/(m_u[i+3]-m_u[i+2]);
	}

	return 0;
}

// 求3次B样条基的导数
double CBSpline::dN3(int i, double uu) const // k=3
{
	double t1,t2;
	if (0==m_u[i+3]-m_u[i])
		t1=0;
	else
		t1=N2(i,uu)/(m_u[i+3]-m_u[i]);

	if (0==m_u[i+4]-m_u[i+1])
		t2=0;
	else
		t2=N2(i+1,uu)/(m_u[i+4]-m_u[i+1]);

	return 3*(t1-t2);
}

void CBSpline::UnitizeKnot()
{
	int szKnot=m_u.GetSize();
	for (int i=0;i<szKnot;i++)
	{
		m_u[i]=m_u[i]/m_u[szKnot-1];
	}
}

// 注：
// 在Scatter函数和draw函数里用了线程锁，因为两个函数会分别在
// 两个进程里对m_pts进行了操作或访问
void CBSpline::Scatter()
{
	g_mutex.Lock();

	int sz=m_pts.GetSize();
	for (int i=0;i<sz;i++)
	{
		delete m_pts[i];
		m_pts[i]=NULL;
	}
	m_pts.RemoveAll();   // 先要清除内存

	if (3==m_degree)
	{
		double step=1/(double)m_ctrls.GetSize()/10;
		for (double u=0;u<1;u=u+step)
		{
			CPoint3D* pPt=new CPoint3D(P(u));
			m_pts.Add(pPt);
		}
		CPoint3D* pPt=new CPoint3D(P(1));
		m_pts.Add(pPt);
	}
	if (1==m_degree)
	{
		int sz=m_ctrls.GetSize();
		for (int i=0;i<sz;i++)
		{
			m_pts.Add(new CPoint3D(*m_ctrls[i]));
		}
	}

	g_mutex.Unlock();
}

void CBSpline::Draw(COpenGLDC* pDC,COLORREF clr)
{
	g_mutex.Lock();

	int sz=m_pts.GetSize();

	COLORREF oldclr;
	if (GetIsSelected())
		oldclr=pDC->SetMaterialColor(clr);
	else
		oldclr=pDC->SetMaterialColor(clr);

	for (int i=0;i<=sz-2;i++)
	{
		pDC->DrawLine(*m_pts[i],*m_pts[i+1],m_nStyle,3,clr);
	}

	pDC->SetMaterialColor(oldclr);

	g_mutex.Unlock();
}

double CBSpline::GetLength()
{
	double L = 0;
	for (int i=0; i<10000;i++)
	{
		CPoint3D pt1 = P((double)i/10000);
		CPoint3D pt2 = P((double)(i+1)/10000);
		L +=  GetDistance(pt1,pt2);
	}
	return L;

}

double CBSpline::Getlength( double u )
{
   double L=0;
   for (int i=0; i<u*10000; i++)
   {
	   CPoint3D pt1 = P((double)i/10000);
	   CPoint3D pt2 = P((double)(i+1)/10000);
	   L +=  GetDistance(pt1,pt2);
   }
   return L;
}

