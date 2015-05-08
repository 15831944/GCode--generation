#include "StdAfx.h"
#include "WavyGenerate.h"
#include "AzClipper.h"

CWavyGenerate::CWavyGenerate()
{

}

CWavyGenerate::~CWavyGenerate(void)
{
}


BOOL CWavyGenerate::ReadSLCFile( CFile& file )
{
	byte b;

	// Read Head and Reserved section
	while (file.Read(&b,1))
	{
		if (0x0d==b)
		{
			for (int i=0;i<258;i++)
			{
				file.Read(&b,1);
			}
			break;
		}
	}

	// Read Sampling Table section
	byte szEntry;
	file.Read(&szEntry,1);
	float min_z, thickness, lineWidth, reserved;
	for (int i=0;i<szEntry;i++)
	{
		file.Read(&min_z,4);
		file.Read(&thickness,4);
		file.Read(&lineWidth,4);
		file.Read(&reserved,4);
		Entry ey(min_z,thickness,lineWidth);
		m_entries.push_back(ey);
	}

	// Read Counter Data section
	float x, y;
	unsigned int szBoundary, szVertex, szGap; 

	while (true)
	{
		file.Read(&min_z,4);  // min z
		file.Read(&szBoundary,4);

		if (0xFFFFFFFF==szBoundary)
		{
			break;
		}

		CLayer* pLayer=new CLayer();
		pLayer->SetLayerZValue(min_z);

		for (int i=0;i<szBoundary;i++)
		{
			file.Read(&szVertex,4);  // number of vertices for the i th boundary
			file.Read(&szGap,4);     // number of gaps for the i th boundary

			CPolyline* pBoundary=new CPolyline();
			for (int j=0;j<szVertex;j++)
			{
				file.Read(&x,4);
				file.Read(&y,4);
				CPoint3D* pPt=new CPoint3D(x,y,min_z);
				pBoundary->AddPt(pPt);
			}

			//pBoundary->DelColinearPt();
			pLayer->AddPolyLine(pBoundary);
		}

		m_layers.push_back(pLayer);
	}
	return TRUE;

}

BOOL CWavyGenerate::ReadSLCFile( CString& strFilePath )
{
	CFile file;
	if (file.Open(strFilePath,CFile::modeRead))
	{
		if (ReadSLCFile(file))
		{
			file.Close();
			return TRUE;
		}
		file.Close();
	}

	return FALSE;
}

void CWavyGenerate::Draw( COpenGLDC* pDC )
{
   if (!m_layers.empty())
   {
	   m_layers[400]->Draw(pDC);
	   //for (int i=0; i<m_layers.size();)
	   //{
		  //m_layers[i]->Draw(pDC);
		  //i= i+10;
	   //}
   }
}

void CWavyGenerate::Write2Gcode( CStdioFile& file )
{

	for (int i=0; i<m_layers.size(); i++)
	{
		for (int j=0;j<m_layers.at(i)->m_GcodesArray.GetSize();j++)
		{
			file.WriteString(m_layers.at(i)->m_GcodesArray[j]);
			file.WriteString(_T("\n"));
		}

	}
	file.WriteString(_T("M107\n"));
	file.WriteString(_T("M104 S0   ; turn off temperature\n"));
	file.WriteString(_T("G28 X0  ; home X axis\n"));
	file.WriteString(_T("M84  ;disable motors\n"));
}

void CWavyGenerate::MoveEntity( double x, double y )
{
	for (int i=0; i<m_layers.size();i++)
	{
        m_layers.at(i)->MovePolyline(x,y);
	}
}
///*************************************////
///double step distance between wavy peak
///double amplitude  height beteen two walls
///double overlapping_distance  distance between wall and peak
void CWavyGenerate::GenerateWavy(double step,
								  double amplitude,
								  double overlap_distance,
								  double pathgap,
								  int    nLoop,
								  double FirstLayerHeight,
								  double LayerHeight)
{
	//m_layers.at(0)->TransfertoSpline();
	//m_layers.at(0)->CalculatePointsOnSplines(step, overlap_distance);
	//m_layers.at(0)->GetPointsforanotherSpline(amplitude,overlap_distance);
	//m_layers.at(0)->GenerateWavySpline();
	//m_layers.at(0)->GntFirstLayerPath(pathgap,nLoop,FirstLayerHeight);

	int a = 400;
	m_layers.at(a)->TransfertoSpline();
	m_layers.at(a)->CalculatePointsOnSplines(step, overlap_distance);
	m_layers.at(a)->GetPointsforanotherSpline(amplitude,overlap_distance);
	m_layers.at(a)->GenerateWavySpline();
	m_layers.at(a)->GntCommonLayerPath(pathgap, LayerHeight);

	//for (int i=0; i<m_layers.size();)
	//{
	//	m_layers.at(i)->TransfertoSpline();
	//	m_layers.at(i)->CalculatePointsOnSplines(step, overlap_distance);
	//	m_layers.at(i)->GetPointsforanotherSpline(amplitude,overlap_distance);
	//	m_layers.at(i)->GenerateWavySpline();
	//	m_layers.at(i)->GntCommonLayerPath(pathgap, LayerHeight);
	//	i= i+10;
	//}
}

