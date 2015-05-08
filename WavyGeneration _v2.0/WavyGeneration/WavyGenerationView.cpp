// WavyGenerationView.cpp : CWavyGenerationView 类的实现
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
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
END_MESSAGE_MAP()

// CWavyGenerationView 构造/析构

CWavyGenerationView::CWavyGenerationView()
{
	// TODO: 在此处添加构造代码

}

CWavyGenerationView::~CWavyGenerationView()
{
}

BOOL CWavyGenerationView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CWavyGenerationView 绘制

void CWavyGenerationView::OnDraw(CDC* /*pDC*/)
{
	CWavyGenerationDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
}


// CWavyGenerationView 打印

BOOL CWavyGenerationView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CWavyGenerationView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CWavyGenerationView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}


// CWavyGenerationView 诊断

#ifdef _DEBUG
void CWavyGenerationView::AssertValid() const
{
	CView::AssertValid();
}

void CWavyGenerationView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CWavyGenerationDoc* CWavyGenerationView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWavyGenerationDoc)));
	return (CWavyGenerationDoc*)m_pDocument;
}
#endif //_DEBUG


// CWavyGenerationView 消息处理程序
