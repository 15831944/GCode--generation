// WavyGenerationDoc.cpp : CWavyGenerationDoc ���ʵ��
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


// CWavyGenerationDoc ����/����

CWavyGenerationDoc::CWavyGenerationDoc()
{
	// TODO: �ڴ����һ���Թ������

}

CWavyGenerationDoc::~CWavyGenerationDoc()
{
}

BOOL CWavyGenerationDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: �ڴ�������³�ʼ������
	// (SDI �ĵ������ø��ĵ�)

	return TRUE;
}




// CWavyGenerationDoc ���л�

void CWavyGenerationDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: �ڴ���Ӵ洢����
	}
	else
	{
		// TODO: �ڴ���Ӽ��ش���
	}
}


// CWavyGenerationDoc ���

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


// CWavyGenerationDoc ����
