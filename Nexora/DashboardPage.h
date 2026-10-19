
#pragma once

#include <afxwin.h>

class CDashboardPage : public CWnd
{
public:
    CDashboardPage();
    virtual ~CDashboardPage();

    BOOL CreatePage(CWnd* pParent);
    void SetUserName(const CString& name);

protected:
    CString m_userName;
    CString m_activePage;

    COLORREF m_background;
    COLORREF m_green;
    COLORREF m_darkGreen;
    COLORREF m_text;
    COLORREF m_muted;
    COLORREF m_border;
    COLORREF m_card;

    CFont m_titleFont;
    CFont m_headingFont;
    CFont m_normalFont;
    CFont m_smallFont;
    CFont m_logoFont;

    void CreateFonts();
    void DrawTextLine(
        CDC& dc, const CString& text, CRect rect,
        COLORREF color, CFont* font,
        UINT format = DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    void DrawCard(
        CDC& dc, CRect rect,
        COLORREF accent,
        const CString& heading,
        const CString& value,
        const CString& subtitle);

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);

    DECLARE_MESSAGE_MAP()
};
