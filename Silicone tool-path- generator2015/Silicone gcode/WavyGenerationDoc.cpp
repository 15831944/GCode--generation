// WavyGenerationDoc.cpp : CWavyGenerationDoc 类的实现
//

#include "stdafx.h"
#include "WavyGeneration.h"

#include "WavyGenerationDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWavyGenerationDoc

IMPLEMENT_DYNCREATE(CWavyGenerationDoc, CDocument)

BEGIN_MESSAGE_MAP(CWavyGenerationDoc, CDocument)
END_MESSAGE_MAP()


// CWavyGenerationDoc 构造/析构

CWavyGenerationDoc::CWavyGenerationDoc()
{
	// TODO: 在此添加一次性构造代码

}

CWavyGenerationDoc::~CWavyGenerationDoc()
{
}

BOOL CWavyGenerationDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)

	return TRUE;
}




// CWavyGenerationDoc 序列化

void CWavyGenerationDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}


// CWavyGenerationDoc 诊断

#ifdef _DEBUG
void CWavyGenerationDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWavyGenerationDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CWavyGenerationDoc 命令
