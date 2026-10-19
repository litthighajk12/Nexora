
#pragma once

#include "NexoraDoc.h"
#include "AuthPage.h"
#include "DashboardPage.h"

class CNexoraView : public CView
{
protected:
    CNexoraView() noexcept;
    DECLARE_DYNCREATE(CNexoraView)

public:
    CNexoraDoc* GetDocument() const;

protected:
    CAuthPage m_authPage;
    CDashboardPage m_dashboardPage;

    bool m_dashboardCreated = false;

    void ShowDashboard();

public:
    virtual void OnDraw(CDC* pDC) override;
    virtual void OnInitialUpdate() override;
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs) override;
    virtual ~CNexoraView();

protected:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg LRESULT OnShowDashboard(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()

#ifdef _DEBUG
public:
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
};

#ifndef _DEBUG
inline CNexoraDoc* CNexoraView::GetDocument() const
{
    return reinterpret_cast<CNexoraDoc*>(m_pDocument);
}
#endif
