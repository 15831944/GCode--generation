// WavyGenerationView.h : CWavyGenerationView ��Ľӿ�
//


#pragma once


class CWavyGenerationView : public CView
{
protected: // �������л�����
	CWavyGenerationView();
	DECLARE_DYNCREATE(CWavyGenerationView)

// ����
public:
	CWavyGenerationDoc* GetDocument() const;

// ����
public:

// ��д
public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// ʵ��
public:
	virtual ~CWavyGenerationView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // WavyGenerationView.cpp �еĵ��԰汾
inline CWavyGenerationDoc* CWavyGenerationView::GetDocument() const
   { return reinterpret_cast<CWavyGenerationDoc*>(m_pDocument); }
#endif

