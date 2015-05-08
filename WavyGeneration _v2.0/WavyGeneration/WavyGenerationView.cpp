// WavyGenerationView.cpp : CWavyGenerationView ���ʵ��
//

#include "stdafx.h"
#include "WavyGeneration.h"

#include "WavyGenerationDoc.h"
#include "WavyGenerationView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWavyGenerationView

IMPLEMENT_DYNCREATE(CWavyGenerationView, CView)

BEGIN_MESSAGE_MAP(CWavyGenerationView, CView)
	// ��׼��ӡ����
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
END_MESSAGE_MAP()

// CWavyGenerationView ����/����

CWavyGenerationView::CWavyGenerationView()
{
	// TODO: �ڴ˴���ӹ������

}

CWavyGenerationView::~CWavyGenerationView()
{
}

BOOL CWavyGenerationView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	return CView::PreCreateWindow(cs);
}

// CWavyGenerationView ����

void CWavyGenerationView::OnDraw(CDC* /*pDC*/)
{
	CWavyGenerationDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: �ڴ˴�Ϊ����������ӻ��ƴ���
}


// CWavyGenerationView ��ӡ

BOOL CWavyGenerationView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Ĭ��׼��
	return DoPreparePrinting(pInfo);
}

void CWavyGenerationView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ��Ӷ���Ĵ�ӡǰ���еĳ�ʼ������
}

void CWavyGenerationView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ��Ӵ�ӡ����е��������
}


// CWavyGenerationView ���

#ifdef _DEBUG
void CWavyGenerationView::AssertValid() const
{
	CView::AssertValid();
}

void CWavyGenerationView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWavyGenerationDoc* CWavyGenerationView::GetDocument() const // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWavyGenerationDoc)));
	return (CWavyGenerationDoc*)m_pDocument;
}
#endif //_DEBUG


// CWavyGenerationView ��Ϣ�������
