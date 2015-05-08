#pragma once
#include "afx.h"
#include "glContext.h"
#include "Entity.h"
#include "Line.h"
#include "Polyline.h"
#include "Circle.h"
#include "Ellipse.h"
#include "Arc.h"
#include "BSpline.h"
#include "Layer.h"
#include "vector"
using namespace std;


struct Entry
{
	Entry(float z, float thick, float width, float rd=0)
	{
		min_z=z;
		thickness=thick;
		lineWidth=width;
		reserved=rd;
	}
	float min_z;
	float thickness;
	float lineWidth;
	float reserved;
};

class CWavyGenerate :
	public CObject
{
public:
	CWavyGenerate(void);
	~CWavyGenerate(void);

	void Draw(COpenGLDC* pDC);
	BOOL ReadSLCFile(CFile& file);
	BOOL ReadSLCFile(CString& sFile);

public:
	void GenerateWavy(double step,
		              double amplitude,
					  double overlap_distance,
					  double pathgap,
					  int    nLoop,
					  double FirstLayerHeight,
					  double LayerHeight);

	void Write2Gcode(CStdioFile& file);
	void MoveEntity(double x, double y);
	void Clear();


	typedef vector<CLayer*> CLayers;
	vector<Entry>   m_entries;

private:
	CLayers          m_layers;
};
