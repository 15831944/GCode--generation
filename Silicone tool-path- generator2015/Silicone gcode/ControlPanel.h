#pragma once
#include "DisplayView.h"


// CControlPanel ������ͼ

class CControlPanel : public CFormView
{
	DECLARE_DYNCREATE(CControlPanel)

protected:
	CControlPanel();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
	virtual ~CControlPanel();

public:
	enum { IDD = IDD_DLG_CONTROL };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

public:
	CDisplayView*  m_pDispaly;

	afx_msg void OnBnClickedBtnRead();
	afx_msg void OnBnClickedBtnGenerate();
	double m_distance;
	double m_height;
	double m_FirstlayerHeight;
	double m_LayerHeight;
	double m_LayerNumber;
	double m_pathwidth;
	double m_deltaAngle;
	afx_msg void OnBnClickedBtnSave();
	double m_move_x;
	double m_move_y;
	afx_msg void OnBnClickedBtnMove();
};


