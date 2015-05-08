#include "StdAfx.h"
#include "Entity.h"


int CEntity::m_IDCount=-1;  // All the IDs starts from 0

CEntity::CEntity(void)
{
	m_clr=RGB(128,128,128);
	m_nWith=1;
	m_nStyle=LS_SOLID;
	m_bDraw=TRUE;
	m_bSelected=FALSE;   // Initially, not selected
	m_nType=-1;
	m_IDCount++;
	m_ID=m_IDCount;
}

CEntity::~CEntity(void)
{
	m_IDCount--;
}

COLORREF CEntity::SetColor(COLORREF clr)
{
	COLORREF oldclr = m_clr;
	m_clr=clr;
	return oldclr;
}

COLORREF CEntity::SetColor(BYTE r, BYTE g, BYTE b)
{
	COLORREF oldclr = m_clr;
	m_clr=RGB(r,g,b);
	return oldclr;
}

COLORREF CEntity::GetColor()
{
	return m_clr;
}

void CEntity::SetWidth(int nWidth)
{
	m_nWith=nWidth;
}

int CEntity::GetWidth()
{
	return m_nWith;
}

void CEntity::SetStyle(int nStyle)
{
	m_nStyle=nStyle;
}

int CEntity::GetStyle()
{
	return m_nStyle;
}

void CEntity::SetIsDraw(BOOL bDraw)
{
	m_bDraw=bDraw;
}

void CEntity::SetType(int nType)
{
   m_nType = nType;
}

BOOL CEntity::GetIsDraw()
{
	return m_bDraw;
}

void CEntity::SetIsSelected(BOOL bSel)
{
	m_bSelected=bSel;
}

BOOL CEntity::GetIsSelected()
{
	return m_bSelected;
}

void CEntity::SetID(int ID)
{
	m_ID=ID;
}

int CEntity::GetID()
{
	return m_ID;
}

int CEntity::GetType()
{
	return m_nType;
}

void CEntity::Move(const CVector3D &vec)
{

}

COBBox CEntity::BoundingBox()
{
	return COBBox();
}


