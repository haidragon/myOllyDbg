
// SDebuggerView.h : CSDebuggerView 类的接口
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
protected: // 仅从序列化创建
	CSDebuggerView();
	DECLARE_DYNCREATE(CSDebuggerView)

// 属性
public:
	CSDebuggerDoc* GetDocument() const;

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnInitialUpdate();
// 实现
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
	//间隙
	int m_marge;  
	//地址字符宽高
	int m_code_address_width;
	int m_line_height;
	int m_pointer_width;
	int m_binary_width;
	int m_disasm_width;
	//鼠标选中行地址
	vector<Selecter> m_SelecterList;
	//指针地址
	ULONG_PTR m_PointerAddress;
	//当前页面是否有指针
	BOOL m_hasPointer;
	//指针所在行
	int m_pointer_line;
	//所有代码地址数
	UINT_PTR m_nAllAddress;
	//当前页所有地址数
	UINT_PTR m_nCurPageAdrCount;
	//当前页起始行
	UINT_PTR m_nCurrentPageStartAdr;
	//是否鼠标左键按下
	BOOL m_isMouseLeftDown;
	UINT_PTR m_dragStartAdr;
protected:
	void DrawBackground(CDC* pDC);
	void DrawCodeAddress(CDC* pDC);
	void DrawDisassemby(CDC* pDC);
	void DrawBinary(CDC* pDC);
	void DrawPointer(CDC* pDC);
	void DrawSelecter(CDC* pDC);
	//获取指针图形
	void GetPointerPoly(POINT* p,int x,int y);
	//判断地址是否被选中
	BOOL IsSelected(UInt64 vAddress);
// 生成的消息映射函数
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

	// 操作
public:
	void SetVertScrollBar(void);
};

#ifndef _DEBUG  // SDebuggerView.cpp 中的调试版本
inline CSDebuggerDoc* CSDebuggerView::GetDocument() const
   { return reinterpret_cast<CSDebuggerDoc*>(m_pDocument); }
#endif

