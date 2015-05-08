#pragma once
#include "Entity.h"

class CPolyline;

class AFX_CLASS_EXPORT CPolylineSet : public CEntity
{
public:
	CPolylineSet(void);
	~CPolylineSet(void);

	virtual void Draw(COpenGLDC* pDC,COLORREF clr);


	CTypedPtrArray<CObArray, CPolyline*> m_polylineList;
};
