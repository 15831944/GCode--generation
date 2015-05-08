// ControlPanel.cpp : 实现文件
//

#include "stdafx.h"
#include "WavyGeneration.h"
#include "ControlPanel.h"


// CControlPanel

IMPLEMENT_DYNCREATE(CControlPanel, CFormView)

CControlPanel::CControlPanel()
	: CFormView(CControlPanel::IDD)
	, m_distance(50)
	, m_height(20)
	, m_FirstlayerHeight(0.1)
	, m_LayerHeight(0.35)
	, m_LayerNumber(7)
	, m_pathwidth(0.4)
	, m_deltaAngle(0)
	, m_move_x(0)
	, m_move_y(0)
{

}

CControlPanel::~CControlPanel()
{
}

void CControlPanel::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_distance);
	DDX_Text(pDX, IDC_EDIT2, m_height);
	DDX_Text(pDX, IDC_EDIT3, m_FirstlayerHeight);
	DDX_Text(pDX, IDC_EDIT4, m_LayerHeight);
	DDX_Text(pDX, IDC_EDIT5, m_LayerNumber);
	DDX_Text(pDX, IDC_EDIT6, m_pathwidth);
	DDX_Text(pDX, IDC_EDIT7, m_deltaAngle);
	DDX_Text(pDX, IDC_EDIT8, m_move_x);
	DDX_Text(pDX, IDC_EDIT9, m_move_y);
}

BEGIN_MESSAGE_MAP(CControlPanel, CFormView)
	ON_BN_CLICKED(IDC_BTN_READ, &CControlPanel::OnBnClickedBtnRead)
	ON_BN_CLICKED(IDC_BTN_GENERATE, &CControlPanel::OnBnClickedBtnGenerate)
	ON_BN_CLICKED(IDC_BTN_SAVE, &CControlPanel::OnBnClickedBtnSave)
	ON_BN_CLICKED(IDC_BTN_MOVE, &CControlPanel::OnBnClickedBtnMove)
END_MESSAGE_MAP()


// CControlPanel 诊断

#ifdef _DEBUG
void CControlPanel::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CControlPanel::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CControlPanel 消息处理程序

void CControlPanel::OnBnClickedBtnRead()
{
    if (m_pDispaly->m_pWavy)
    {
		delete m_pDispaly->m_pWavy;
		m_pDispaly->m_pWavy = NULL;
    }
	
	m_pDispaly->m_pWavy = new CWavyGenerate();
	UpdateData(TRUE);
	m_pDispaly->m_pWavy->GenerateOriginRects(m_LayerNumber,m_LayerHeight,m_height,m_distance);

	m_pDispaly->Invalidate(TRUE);

	UpdateData(FALSE);
}

void CControlPanel::OnBnClickedBtnGenerate()
{
 	UpdateData(TRUE);
 	m_pDispaly->m_pWavy->GenerateGcode(m_pathwidth,m_deltaAngle);
	m_pDispaly->Invalidate(TRUE);
}

void CControlPanel::OnBnClickedBtnSave()
{
	CFileDialog dlg(FALSE,NULL,_T(".gcode"),OFN_NOCHANGEDIR|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,_T("Gcode(*.gcode)|*.gcode"));
	dlg.DoModal();
	CStdioFile file(dlg.GetPathName(),CFile::modeCreate|CFile::modeWrite);
	CString str;
	m_pDispaly->m_pWavy->Write2Gcode(file);
	file.Close();
}

void CControlPanel::OnBnClickedBtnMove()
{
	UpdateData(TRUE);
	m_pDispaly->m_pWavy->MoveEntity(m_move_x, m_move_y);
	m_pDispaly->Invalidate(TRUE);
}
