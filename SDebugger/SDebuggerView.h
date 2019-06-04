
// SDebuggerView.h : CSDebuggerView ��Ľӿ�
//


#pragma once

typedef struct _Selecter
{
	UInt64 address;
	int line;
	BOOL show;
}Selecter,*PSelecter;

class CSDebuggerView : public CView
{
protected: // �������л�����
	CSDebuggerView();
	DECLARE_DYNCREATE(CSDebuggerView)

// ����
public:
	CSDebuggerDoc* GetDocument() const;

// ��д
public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnInitialUpdate();
// ʵ��
public:
	virtual ~CSDebuggerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	COLORREF m_background;
	COLORREF m_adressBG;
	COLORREF m_adressF;
	COLORREF m_binaryBG;
	COLORREF m_binaryF;
	COLORREF m_disasmBG;
	COLORREF m_disasmF;
	COLORREF m_pointerF;
	COLORREF m_pointerBG;
	COLORREF m_selecterBG;
	CFont m_font;
	//��϶
	int m_marge;  
	//��ַ�ַ����
	int m_code_address_width;
	int m_line_height;
	int m_pointer_width;
	int m_binary_width;
	int m_disasm_width;
	//���ѡ���е�ַ
	vector<Selecter> m_SelecterList;
	//ָ���ַ
	ULONG_PTR m_PointerAddress;
	//��ǰҳ���Ƿ���ָ��
	BOOL m_hasPointer;
	//ָ��������
	int m_pointer_line;
	//���д����ַ��
	UINT_PTR m_nAllAddress;
	//��ǰҳ���е�ַ��
	UINT_PTR m_nCurPageAdrCount;
	//��ǰҳ��ʼ��
	UINT_PTR m_nCurrentPageStartAdr;
	//�Ƿ�����������
	BOOL m_isMouseLeftDown;
	UINT_PTR m_dragStartAdr;
protected:
	void DrawBackground(CDC* pDC);
	void DrawCodeAddress(CDC* pDC);
	void DrawDisassemby(CDC* pDC);
	void DrawBinary(CDC* pDC);
	void DrawPointer(CDC* pDC);
	void DrawSelecter(CDC* pDC);
	//��ȡָ��ͼ��
	void GetPointerPoly(POINT* p,int x,int y);
	//�жϵ�ַ�Ƿ�ѡ��
	BOOL IsSelected(UInt64 vAddress);
// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg LRESULT OnDebuggerThread(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT OnDisassemby(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT OnSetPointerAdress(WPARAM wparam,LPARAM lparam);
	afx_msg void OnRun();
	afx_msg void OnStop();
	afx_msg void OnPause();
	afx_msg void OnRestart();
	afx_msg void OnSingleIn();
	afx_msg void OnUpdateDebugUI(CCmdUI* pCui);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	// ����
public:
	void SetVertScrollBar(void);
};

#ifndef _DEBUG  // SDebuggerView.cpp �еĵ��԰汾
inline CSDebuggerDoc* CSDebuggerView::GetDocument() const
   { return reinterpret_cast<CSDebuggerDoc*>(m_pDocument); }
#endif

