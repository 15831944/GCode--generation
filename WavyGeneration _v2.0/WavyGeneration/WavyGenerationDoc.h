// WavyGenerationDoc.h : CWavyGenerationDoc ��Ľӿ�
//


#pragma once


class CWavyGenerationDoc : public CDocument
{
protected: // �������л�����
	CWavyGenerationDoc();
	DECLARE_DYNCREATE(CWavyGenerationDoc)

// ����
public:

// ����
public:

// ��д
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// ʵ��
public:
	virtual ~CWavyGenerationDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
};


