#pragma once
#include "afx.h"
#include "GeomBase.h"
#include "glContext.h"

typedef    int  ObjectID;
#define  Original_PART 0
#define  First_PART    1
#define  Wavy_PART     2
#define  Offset_PART   3

enum{TYPE_LINE, TYPE_POLYLINE, TYPE_CIRCLE, TYPE_ELLIPSE, TYPE_SPLINE,
TYPE_POLYLINESET};

class AFX_CLASS_EXPORT CEntity : public CObject
{
public:
	CEntity();
	~CEntity();

// Attributes
	COLORREF SetColor(COLORREF clr);
	COLORREF SetColor(BYTE r, BYTE g, BYTE b);
	COLORREF GetColor();
	void     SetWidth(int nWidth);
	int      GetWidth();
	void     SetStyle(int nStyle);
	int      GetStyle();
	void     SetIsDraw(BOOL bDraw);  
	BOOL     GetIsDraw();
	void     SetIsSelected(BOOL bSel);
	BOOL     GetIsSelected();
	void     SetID(int ID);
	int      GetID();
	int      GetType();
	void     SetType(int nType);



// Overridable
	virtual void Draw(COpenGLDC* pDC, COLORREF clr)=0;
	virtual	void Move(const CVector3D& vec);
	virtual COBBox BoundingBox();




private:
	CEntity(const CEntity& entitySrc);              // no implementation
	void operator=(const CEntity& entitySrc);       // no implementation
	static int m_IDCount;

protected:
	COLORREF m_clr;       // Color of the entity
	int      m_nWith;     // The width of the line
	int      m_nStyle;    // The style of the line: solid, dash ...
	BOOL     m_bDraw;     // Should or not draw this entity
	BOOL     m_bSelected; // Is the entity selected or not
	int      m_ID;        // The ID of the entity, not used currently
	int      m_nType;     // The type of the derived class, like CLine, CPolyline...

public:
	COBBox   m_obb;

public:
	int part_ID; //three parts
};

typedef CTypedPtrArray<CObArray,CEntity*> EntityAry;