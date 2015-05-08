#include "StdAfx.h"
#include "WavyGenerate.h"
#include "AzClipper.h"

CWavyGenerate::CWavyGenerate()
{

}

CWavyGenerate::~CWavyGenerate(void)
{
}


BOOL CWavyGenerate::GenerateOriginRects( int layerNumber,double layerHeight, int width, int length )
{
	for (int i=0; i<layerNumber; i++)
	{
		CLayer* pLayer=new CLayer();
		double z;
		z= (i+1)*layerHeight;
		pLayer->SetLayerZValue(z);
		CPolyline* pBoundary=new CPolyline();
		 int orginX, orginY;
		 orginX = orginY = 60;

		CPoint3D* pPt=new CPoint3D(orginX,orginY,z);
		pBoundary->AddPt(pPt);
		pPt=new CPoint3D(orginX,orginY+width,z);
		pBoundary->AddPt(pPt);
		pPt=new CPoint3D(orginX+length,orginY+width,z);
		pBoundary->AddPt(pPt);
		pPt=new CPoint3D(orginX+length,orginY,z);
		pBoundary->AddPt(pPt);
		pPt=new CPoint3D(orginX,orginY,z);
		pBoundary->AddPt(pPt);
		pBoundary->DelColinearPt();
		pLayer->AddPolyLine(pBoundary);

		m_layers.push_back(pLayer);

	}
	return TRUE;
}


void CWavyGenerate::Draw( COpenGLDC* pDC )
{
   if (!m_layers.empty())
   {
	   //int a = m_layers.size() -44;
	   //m_layers[a]->Draw(pDC);

	  
 	   for (int i=0; i<m_layers.size(); i = i+1)
 	   {
		
         m_layers[i]->Draw(pDC);
 		
 	   }
   }
}

void CWavyGenerate::Write2Gcode( CStdioFile& file )
{
	file.WriteString(_T("M107   ; turn off temperature\n"));
	file.WriteString(_T("G21 ; set units to millimeters\n"));
	file.WriteString(_T("G28 ; home all axes\n"));
	file.WriteString(_T("G1 Z5 F5000 ; lift nozzl\n"));
	file.WriteString(_T("G90 ; use absolute coordinates\n"));
	file.WriteString(_T("M83 ; use relative distances for extrusion\n"));

	for (int i=0; i<m_layers.size(); i++)
	{
		for (int j=0;j<m_layers.at(i)->m_GcodesArray.GetSize();j++)
		{
			file.WriteString(m_layers.at(i)->m_GcodesArray[j]);
			file.WriteString(_T("\n"));
		}

	}
	file.WriteString(_T("M107\n"));
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


void CWavyGenerate::GenerateGcode(double linespace, double delta_angle)
{
	for (int a=0; a<m_layers.size(); a= a+1)
	{
		double angle_0 =0;
        double angle_1 = delta_angle;
		double angle = ((a%2 == 0)?angle_0:angle_1);
		m_layers.at(a)->GntLayerPath(linespace,angle);


	}
}


