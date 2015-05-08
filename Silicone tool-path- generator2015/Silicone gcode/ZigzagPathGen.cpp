#include "StdAfx.h"
#include "ZigzagPathGen.h"
#include "ZigZagPath.h"
#include "global.h"
#include "AzClipper.h"
CZigzagPathGen::CZigzagPathGen(void)
{
}

CZigzagPathGen::~CZigzagPathGen(void)
{
}

void CZigzagPathGen::GetInputs( PolyLineGroup &InPut, double width,double angle)
{
   for (int i=0; i<InPut.size(); i++)
   {
	   m_InputPolys.push_back(InPut.at(i));
   }
   m_width = width;
   m_angle = angle;

}

int CZigzagPathGen::RoughClassifyZone()
{
	//clear m_LinesGroups
	int szTemp1 =m_LinesGroups.size();
	for (int i = 0; i < szTemp1; i++)
	{
		int szTemp2 = m_LinesGroups[i].size();
		for (int j = 0; j<szTemp2; j++)
		{
			m_LinesGroups[i][j].clear();
		}
		m_LinesGroups[i].clear();
	}
	m_LinesGroups.clear();

	int n_GroupNum = 0;
	int szRef  = lineses.size();
	if (szRef == 0)
	{
		return 0 ;
	}

	int tempNum = 0;
	int index1 =0;

	//第一步根据扫描线的交点进行分组
	while(true)
	{
		if (index1 >= szRef-1)
		{
			break;
		} 
		vector<vector< CLine> > pLines;
		for (int i = index1 ; i<szRef; i++)
		{	
			if (i == 0 )
			{
				pLines.push_back(lineses[0]);
				tempNum = lineses[0].size();
				index1++;
			}
			if (i == szRef - 1)
			{
				m_LinesGroups.push_back(pLines);
				index1++;
				break;
			}
			if ((i!=0)&&(i!=szRef-1))
			{
				if (lineses[i].size() == tempNum)
				{
					pLines.push_back(lineses[i]);
				}
				else
				{
					tempNum = lineses[i].size();
					m_LinesGroups.push_back(pLines);

					int se  = pLines.size();
					for (int j = 0; j<se ; j++)
					{
						pLines[j].clear();
					}
					pLines.clear();
					pLines.push_back(lineses[i]);
					break;
				}
				index1++;
			}
		}
	}
	n_GroupNum = m_LinesGroups.size();
	return n_GroupNum;

}


void CZigzagPathGen::GntSweeplines(vector<CPolyline*>& boundaries, double lineInterval, double angle, vector<vector<CLine>>& lineses, bool bBoundaryRotateBack )
{
	::GntSweeplines(boundaries,lineInterval,angle,lineses,bBoundaryRotateBack);
}

void CZigzagPathGen::GntSweeplines(CPolyline* pBoundary, double lineInterval, double angle, vector<vector<CLine>>& lineses, bool bBoundaryRotateBack )
{
	vector<CPolyline*> boundaries;
	boundaries.push_back(pBoundary);
	::GntSweeplines(boundaries,lineInterval,angle,lineses,bBoundaryRotateBack);
}

void CZigzagPathGen::GntSweeplines(CScanArea* pScanArea, double lineInterval, double angle, vector<vector<CLine>>& lineses, bool bBoundaryRotateBack /* = false */ )
{
	vector<CPolyline*> boundaries;
	boundaries.push_back(pScanArea->m_outerBoundary);
	int sz = pScanArea->m_islands.size();
	for (int i=0;i<sz;i++)
	{
		boundaries.push_back(pScanArea->m_islands[i]);
	}
	::GntSweeplines(boundaries,lineInterval,angle,lineses,bBoundaryRotateBack);	
}

int CZigzagPathGen::FineClassifyZone()
{
	//clear m_InitialLineGroups
	int szTemp =m_InitialLineGroups.size();
	for (int i = 0; i < szTemp; i++)
	{
		m_InitialLineGroups[i].clear();
	}
	m_InitialLineGroups.clear();

	int n_GroupNum = 0;
	int index1 =0;

	n_GroupNum = RoughClassifyZone();

	for (int k = 0; k<n_GroupNum ;k++)
	{
		int nLineCount = m_LinesGroups[k].size();    //这组有几条扫描线
		int nLineCounts = m_LinesGroups[k][0].size();//这一组里面每条扫描线有几条元素

		for (int j = 0;j <nLineCounts;j++)
		{
			vector<CLine> n_lines;	         
			for (int m =0; m < nLineCount; m++)
			{
				n_lines.push_back(m_LinesGroups[k][m][j]);
			}
			m_InitialLineGroups.push_back(n_lines);            
		}
	}
	n_GroupNum = m_InitialLineGroups.size();
	return n_GroupNum;

}

void CZigzagPathGen::LinkZone()
{
	//如何连接作为是一个优化的概念，就是把分组之后的结果进行衔接
	m_ZigzagPath.releaseMem();
	int szzigzag = m_InitialLineGroups.size();

	vector<ZigzagPoint*> tmpZigzagpath;
	vector<CLine> tmpLines;


	CPoint3D pt_Ref;//作为添加到m_Ziazagpaths之后的最后一个点
	//把第一组路径加入
	m_ZigzagPath.GntZigzagFRomlines(tmpZigzagpath,m_InitialLineGroups[0],PSSTART);
	m_ZigzagPath.m_Ziazagpaths.push_back(tmpZigzagpath);
	pt_Ref = m_ZigzagPath.GetZigzagEndPoint( m_InitialLineGroups[0],PSSTART);
	m_InitialLineGroups.erase(m_InitialLineGroups.begin());//delete the first part

	vector<vector<CLine> >::iterator it;//作为m_InitialLineGroups的迭代器来遍历
	while(true)
	{
		if (m_InitialLineGroups.empty())
		{
			break;
		}
		vector<ZigzagPoint*> tmpZigzagpathEx;
		double ShortestLen = HUGE; //每次循环找出一个最短的距离
		int n_type;                //找出的那个zigzag的起点
		int nCount = 0;
		vector<struct TargetLine> LineElements;
		for (it = m_InitialLineGroups.begin(); it!= m_InitialLineGroups.end();)
		{
			tmpLines = *it;
			double length;
			n_type = m_ZigzagPath.FindShortestLen(pt_Ref, tmpLines,length);
			struct TargetLine n_targetLine(length, nCount, n_type);
			LineElements.push_back(n_targetLine);
			nCount++;
			++it;
		}
		sort(LineElements.begin(), LineElements.end());//按照里面的shortestdis元素进行升序排列
		ShortestLen = LineElements[0].n_shortdis;
		n_type = LineElements[0].n_type;
		nCount = LineElements[0].n_index;

		m_ZigzagPath.GntZigzagFRomlines(tmpZigzagpathEx,m_InitialLineGroups[nCount],n_type);
		m_ZigzagPath.m_Ziazagpaths.push_back(tmpZigzagpathEx);
		pt_Ref = m_ZigzagPath.GetZigzagEndPoint( m_InitialLineGroups[nCount],n_type);
		m_InitialLineGroups.erase(m_InitialLineGroups.begin()+nCount);
	}

	//把相邻的衔接起来
	m_ZigzagPath.FineProcessingPath(m_width);

	CPolyline* TempPoly;
	for (int j=0; j<m_ZigzagPath.m_FinalZiazagpaths.size();j++)
	{
		TempPoly = new CPolyline();
		CPoint3D* TempPt;
		for (int i =0; i<m_ZigzagPath.m_FinalZiazagpaths.at(j).size();i++)
		{		
			TempPt = new CPoint3D();
			TempPt = &(m_ZigzagPath.m_FinalZiazagpaths.at(j).at(i)->pt);
			TempPoly->AddPt(TempPt);
		}

		double Length = TempPoly->GetLength();
		if (Length > 3)
		{
			m_OutPolys.push_back(TempPoly);
		}
	
	}
	m_ZigzagPath.HermiteLinking(m_width);

}

void CZigzagPathGen::GntSweepPath()
{
	//调用库函数，然后把内轮廓偏置一下计算和直线的交点
	PolyLineGroup  m_offsetedboundriesEx;;

	AzClipper::Offset(m_InputPolys,m_offsetedboundriesEx,-m_width/2);

	//上一次生成之后的容器清空
	int sz = lineses.size();
	for (int i= 0 ; i<sz; i++)
	{
		lineses[i].clear();
	}
	lineses.clear();

	::GntSweeplines(m_offsetedboundriesEx,m_width,m_angle,lineses, true);  // note：最后一个参数要为true，不转回去的话第二次生成就会出错了
	FineClassifyZone();//分区
	LinkZone();//连接
}

void CZigzagPathGen::OutPutPolys( PolyLineGroup& OutPolys )
{
	for (int i=0; i<m_OutPolys.size();i++)
	{
		OutPolys.push_back(m_OutPolys.at(i));
	}

}


