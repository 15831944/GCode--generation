// DisplayView.cpp : 实现文件
//

#include "stdafx.h"
#include "WavyGeneration.h"
#include "DisplayView.h"


// CDisplayView

IMPLEMENT_DYNCREATE(CDisplayView, CFormView)

CDisplayView::CDisplayView()
	: CFormView(CDisplayView::IDD)
{
   m_pWavy = NULL;
}

CDisplayView::~CDisplayView()
{
}

void CDisplayView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDisplayView, CFormView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEWHEEL()

END_MESSAGE_MAP()


// CDisplayView 诊断

#ifdef _DEBUG
void CDisplayView::AssertValid() const
{
	CFormView::AssertValid();
}

void CDisplayView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here

	Render();
}

#ifndef _WIN32_WCE
void CDisplayView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CDisplayView 消息处理程序

int CDisplayView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_pDC = new COpenGLDC(this);

	return 0;
}

void CDisplayView::Render()
{
	m_pDC->SetBkColor(RGB(0,0,0));
	m_pDC->Ready();
	m_pDC->DrawCoord();

    if (m_pWavy != NULL)
    {
 	   m_pWavy->Draw(m_pDC);
    }
// 	if (m_pCode!=NULL)
// 	{
// 		m_pCode->Draw(m_pDC,m_CurrentLayer);
// 	}
	m_pDC->Finish();
}

void CDisplayView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	m_pDC->GLResize(cx,cy);
}

void CDisplayView::OnDestroy()
{
	CView::OnDestroy();

	m_pDC->GLRelease();
}

BOOL CDisplayView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return true;
}

BOOL CDisplayView::PreTranslateMessage(MSG* pMsg)
{
	if (WM_MBUTTONUP==pMsg->message || WM_RBUTTONUP==pMsg->message)
	{
		m_nViewAction=VA_NO_ACTION;
	}
	else if (WM_MBUTTONDOWN==pMsg->message)
	{
		m_nViewAction=VA_MOVE_VIEW;
		m_ptFrom=pMsg->pt;
	}
	else if (WM_RBUTTONDOWN==pMsg->message)
	{
		m_nViewAction=VA_ROTATE_VIEW;
		m_ptFrom=pMsg->pt;
	}
	else if (WM_MOUSEMOVE==pMsg->message)
	{
		if (VA_MOVE_VIEW==m_nViewAction)
		{
			m_pDC->m_camera.MoveView(m_ptFrom,pMsg->pt);
			m_ptFrom=pMsg->pt;
			Render();
		}

		if (VA_ROTATE_VIEW==m_nViewAction)
		{
			m_pDC->m_camera.RotateView(m_ptFrom,pMsg->pt);
			m_ptFrom=pMsg->pt;
			Render();
		}
	}

	return CView::PreTranslateMessage(pMsg);
}

BOOL CDisplayView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if(zDelta>0)
		m_pDC->m_camera.Zoom(1/1.1);
	else
		m_pDC->m_camera.Zoom(1.1);

	Render();

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

