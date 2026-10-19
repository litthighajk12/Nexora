
#include "pch.h"
#include "framework.h"

#ifndef SHARED_HANDLERS
#include "Nexora.h"
#endif

#include "NexoraDoc.h"
#include "NexoraView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CNexoraView, CView)

BEGIN_MESSAGE_MAP(CNexoraView, CView)
    ON_WM_SIZE()
    ON_WM_RBUTTONUP()
    ON_WM_CONTEXTMENU()
    ON_MESSAGE(WM_APP + 101, &CNexoraView::OnShowDashboard)
END_MESSAGE_MAP()

CNexoraView::CNexoraView() noexcept
{
}

CNexoraView::~CNexoraView()
{
}

BOOL CNexoraView::PreCreateWindow(CREATESTRUCT& cs)
{
    return CView::PreCreateWindow(cs);
}

void CNexoraView::OnInitialUpdate()
{
    CView::OnInitialUpdate();

    if (!m_authPage.GetSafeHwnd())
    {
        if (!m_authPage.CreatePage(this))
        {
            AfxMessageBox(
                _T("Unable to create Nexora login page."),
                MB_ICONERROR);
            return;
        }
    }

    CRect rc;
    GetClientRect(&rc);

    m_authPage.MoveWindow(rc);
    m_authPage.ShowWindow(SW_SHOW);
    m_authPage.SetFocus();
}

void CNexoraView::ShowDashboard()
{
    if (!m_dashboardCreated)
    {
        if (!m_dashboardPage.CreatePage(this))
        {
            AfxMessageBox(
                _T("Unable to create Nexora dashboard."),
                MB_ICONERROR);
            return;
        }

        m_dashboardCreated = true;

        CString userName = m_authPage.GetUserId();
        if (userName.IsEmpty())
            userName = _T("User");

        m_dashboardPage.SetUserName(userName);
    }

    CRect rc;
    GetClientRect(&rc);

    m_authPage.ShowWindow(SW_HIDE);
    m_dashboardPage.MoveWindow(rc);
    m_dashboardPage.ShowWindow(SW_SHOW);
    m_dashboardPage.BringWindowToTop();
    m_dashboardPage.Invalidate(TRUE);
    m_dashboardPage.UpdateWindow();
}

void CNexoraView::OnDraw(CDC* pDC)
{
    if (!pDC)
        return;

    CRect rc;
    GetClientRect(&rc);

    CBrush background(RGB(246, 249, 247));
    pDC->FillRect(&rc, &background);
}

void CNexoraView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);

    if (m_authPage.GetSafeHwnd())
        m_authPage.MoveWindow(0, 0, cx, cy);

    if (m_dashboardPage.GetSafeHwnd())
        m_dashboardPage.MoveWindow(0, 0, cx, cy);
}

void CNexoraView::OnRButtonUp(UINT, CPoint point)
{
    ClientToScreen(&point);
    OnContextMenu(this, point);
}

void CNexoraView::OnContextMenu(CWnd*, CPoint point)
{
#ifndef SHARED_HANDLERS
    if (theApp.GetContextMenuManager())
    {
        theApp.GetContextMenuManager()->ShowPopupMenu(
            IDR_POPUP_EDIT,
            point.x,
            point.y,
            this,
            TRUE);
    }
#endif
}
LRESULT CNexoraView::OnShowDashboard(WPARAM, LPARAM)
{
    ShowDashboard();
    return 0;
}

#ifdef _DEBUG
void CNexoraView::AssertValid() const
{
    CView::AssertValid();
}

void CNexoraView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}
#endif
