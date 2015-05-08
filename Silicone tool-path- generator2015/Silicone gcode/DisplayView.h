#pragma once
#include "glContext.h"
#include "WavyGenerate.h"



// CDisplayView 窗体视图

class CDisplayView : public CFormView
{
	DECLARE_DYNCREATE(CDisplayView)

protected:
	CDisplayView();           // 动态创建所使用的受保护的构造函数
	virtual ~CDisplayView();

public:
	enum { IDD = IDD_VIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;

public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	int        m_nViewAction;
	enum       // View Action
	{ 
		VA_NO_ACTION,
		VA_MOVE_VIEW,
		VA_ROTATE_VIEW,
		VA_MOVE_ENTITY
	};
	CPoint m_ptFrom;

	//	CControlPanel* m_pCtrlPane;
	COpenGLDC* m_pDC;
	void Render();


protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

public:
	CWavyGenerate* m_pWavy;

};


