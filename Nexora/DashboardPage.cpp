
#include "pch.h"
#include "framework.h"
#include "DashboardPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CDashboardPage, CWnd)
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

CDashboardPage::CDashboardPage()
{
    m_userName = _T("User");
    m_activePage = _T("Dashboard");

    m_background = RGB(245, 249, 247);
    m_green = RGB(0, 153, 102);
    m_darkGreen = RGB(0, 105, 75);
    m_text = RGB(35, 49, 44);
    m_muted = RGB(112, 130, 121);
    m_border = RGB(222, 234, 227);
    m_card = RGB(255, 255, 255);
}

CDashboardPage::~CDashboardPage()
{
    m_titleFont.DeleteObject();
    m_headingFont.DeleteObject();
    m_normalFont.DeleteObject();
    m_smallFont.DeleteObject();
    m_logoFont.DeleteObject();
}

BOOL CDashboardPage::CreatePage(CWnd* pParent)
{
    CString className = AfxRegisterWndClass(
        CS_HREDRAW | CS_VREDRAW,
        LoadCursor(nullptr, IDC_ARROW),
        (HBRUSH)GetStockObject(WHITE_BRUSH),
        nullptr);

    return CreateEx(
        0,
        className,
        _T("Nexora Dashboard"),
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        CRect(0, 0, 1000, 700),
        pParent,
        0);
}

void CDashboardPage::SetUserName(const CString& name)
{
    m_userName = name.IsEmpty() ? _T("User") : name;

    if (::IsWindow(GetSafeHwnd()))
        Invalidate(FALSE);
}

int CDashboardPage::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    CreateFonts();
    return 0;
}

void CDashboardPage::CreateFonts()
{
    m_titleFont.CreateFont(
        27, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));

    m_headingFont.CreateFont(
        17, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));

    m_normalFont.CreateFont(
        14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));

    m_smallFont.CreateFont(
        12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));

    m_logoFont.CreateFont(
        23, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));
}

BOOL CDashboardPage::OnEraseBkgnd(CDC* /*pDC*/)
{
    return TRUE;
}

void CDashboardPage::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    Invalidate(FALSE);
}

void CDashboardPage::DrawTextLine(
    CDC& dc,
    const CString& text,
    CRect rect,
    COLORREF color,
    CFont* font,
    UINT format)
{
    CFont* oldFont = nullptr;

    if (font && font->GetSafeHandle())
        oldFont = dc.SelectObject(font);

    dc.SetBkMode(TRANSPARENT);
    dc.SetTextColor(color);
    dc.DrawText(text, rect, format);

    if (oldFont)
        dc.SelectObject(oldFont);
}

void CDashboardPage::DrawCard(
    CDC& dc,
    CRect rect,
    COLORREF accent,
    const CString& heading,
    const CString& value,
    const CString& subtitle)
{
    CBrush whiteBrush(m_card);
    CPen borderPen(PS_SOLID, 1, m_border);

    CBrush* oldBrush = dc.SelectObject(&whiteBrush);
    CPen* oldPen = dc.SelectObject(&borderPen);

    dc.RoundRect(rect, CPoint(12, 12));

    dc.SelectObject(oldBrush);
    dc.SelectObject(oldPen);

    CRect stripe(
        rect.left + 16, rect.top + 19,
        rect.left + 21, rect.top + 49);

    dc.FillSolidRect(stripe, accent);

    DrawTextLine(dc, heading,
        CRect(rect.left + 32, rect.top + 15,
            rect.right - 14, rect.top + 40),
        m_muted, &m_normalFont);

    DrawTextLine(dc, value,
        CRect(rect.left + 24, rect.top + 43,
            rect.right - 14, rect.top + 82),
        m_text, &m_titleFont);

    DrawTextLine(dc, subtitle,
        CRect(rect.left + 24, rect.top + 83,
            rect.right - 12, rect.top + 108),
        accent, &m_smallFont);
}

void CDashboardPage::OnPaint()
{
    CPaintDC dc(this);

    CRect client;
    GetClientRect(&client);

    if (client.Width() <= 0 || client.Height() <= 0)
        return;

    dc.FillSolidRect(client, m_background);
    dc.SetBkMode(TRANSPARENT);

    const int sidebarWidth = 220;
    const int headerHeight = 86;
    const int margin = 22;

    // Sidebar
    CRect sidebar(0, 0, sidebarWidth, client.Height());
    dc.FillSolidRect(sidebar, RGB(255, 255, 255));

    CPen borderPen(PS_SOLID, 1, m_border);
    CPen* oldPen = dc.SelectObject(&borderPen);

    dc.MoveTo(sidebarWidth - 1, 0);
    dc.LineTo(sidebarWidth - 1, client.Height());

    dc.SelectObject(oldPen);

    // Brand logo
    CRect logo(22, 23, 65, 66);
    CBrush greenBrush(m_green);
    CPen greenPen(PS_SOLID, 1, m_green);

    CBrush* oldBrush = dc.SelectObject(&greenBrush);
    oldPen = dc.SelectObject(&greenPen);

    dc.RoundRect(logo, CPoint(9, 9));

    dc.SelectObject(oldBrush);
    dc.SelectObject(oldPen);

    DrawTextLine(dc, _T("N"), logo, RGB(255, 255, 255),
        &m_logoFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    DrawTextLine(dc, _T("NEXORA"),
        CRect(76, 20, 200, 48),
        m_darkGreen, &m_headingFont);

    DrawTextLine(dc, _T("YOUR SOCIAL WORKSPACE"),
        CRect(76, 44, 205, 66),
        m_muted, &m_smallFont);

    DrawTextLine(dc, _T("WORKSPACE"),
        CRect(20, 93, 195, 119),
        m_muted, &m_smallFont);

    const TCHAR* menuItems[] = {
        _T("Dashboard"),
        _T("Messages"),
        _T("Contacts"),
        _T("Social Feed"),
        _T("Stories"),
        _T("Groups"),
        _T("History")
    };

    const TCHAR* menuSymbols[] = {
        _T("D"), _T("M"), _T("C"), _T("F"),
        _T("S"), _T("G"), _T("H")
    };

    int navY = 126;
    const int navHeight = 43;

    for (int i = 0; i < 7; ++i)
    {
        CRect item(10, navY, sidebarWidth - 10,
            navY + navHeight);

        bool selected = (m_activePage == menuItems[i]);

        if (selected)
        {
            CBrush selectedBrush(RGB(231, 247, 239));
            CPen selectedPen(PS_SOLID, 1, RGB(231, 247, 239));

            oldBrush = dc.SelectObject(&selectedBrush);
            oldPen = dc.SelectObject(&selectedPen);

            dc.RoundRect(item, CPoint(8, 8));

            dc.SelectObject(oldBrush);
            dc.SelectObject(oldPen);

            dc.FillSolidRect(
                CRect(item.left, item.top + 8,
                    item.left + 3, item.bottom - 8),
                m_green);
        }

        DrawTextLine(dc, menuSymbols[i],
            CRect(25, navY, 53, navY + navHeight),
            selected ? m_green : m_muted,
            &m_headingFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        DrawTextLine(dc, menuItems[i],
            CRect(58, navY, sidebarWidth - 18, navY + navHeight),
            selected ? m_darkGreen : m_text,
            &m_normalFont);

        navY += navHeight + 3;
    }

    DrawTextLine(dc, _T("PREFERENCES"),
        CRect(20, navY + 8, 190, navY + 32),
        m_muted, &m_smallFont);

    navY += 39;

    const TCHAR* preferenceItems[] = {
        _T("Settings"), _T("Diagnostics")
    };

    for (int i = 0; i < 2; ++i)
    {
        bool selected = (m_activePage == preferenceItems[i]);

        DrawTextLine(dc, preferenceItems[i],
            CRect(26, navY, sidebarWidth - 15, navY + 39),
            selected ? m_green : m_text,
            &m_normalFont);

        navY += 42;
    }

    // Profile card at bottom of sidebar
    int profileTop = max(navY + 10, client.Height() - 78);

    if (profileTop < client.Height() - 10)
    {
        CRect profile(12, profileTop,
            sidebarWidth - 12, client.Height() - 12);

        CBrush profileBrush(RGB(244, 249, 246));
        CPen profilePen(PS_SOLID, 1, m_border);

        oldBrush = dc.SelectObject(&profileBrush);
        oldPen = dc.SelectObject(&profilePen);

        dc.RoundRect(profile, CPoint(9, 9));

        dc.SelectObject(oldBrush);
        dc.SelectObject(oldPen);

        DrawTextLine(dc, _T("A"),
            CRect(22, profileTop + 10, 53, profileTop + 42),
            m_green, &m_headingFont,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        DrawTextLine(dc, m_userName,
            CRect(60, profileTop + 7, sidebarWidth - 20,
                profileTop + 30),
            m_text, &m_headingFont);

        DrawTextLine(dc, _T("Your profile"),
            CRect(60, profileTop + 30, sidebarWidth - 20,
                profileTop + 50),
            m_muted, &m_smallFont);
    }

    // Main workspace header
    const int contentLeft = sidebarWidth + margin;
    const int contentRight = client.Width() - margin;
    const int contentWidth = max(100, contentRight - contentLeft);

    DrawTextLine(dc, _T("Good day, ") + m_userName + _T("!"),
        CRect(contentLeft, 19, contentRight - 150, 54),
        m_text, &m_titleFont);

    DrawTextLine(dc,
        _T("Your conversations, community and activity in one place."),
        CRect(contentLeft, 53, contentRight - 100, 77),
        m_muted, &m_normalFont);

    CRect statusPill(contentRight - 142, 27, contentRight, 59);

    CBrush pillBrush(RGB(231, 247, 239));
    CPen pillPen(PS_SOLID, 1, RGB(205, 234, 218));

    oldBrush = dc.SelectObject(&pillBrush);
    oldPen = dc.SelectObject(&pillPen);

    dc.RoundRect(statusPill, CPoint(10, 10));

    dc.SelectObject(oldBrush);
    dc.SelectObject(oldPen);

    DrawTextLine(dc, _T("●  WORKSPACE"),
        statusPill, m_darkGreen, &m_smallFont,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Page heading
    DrawTextLine(dc, m_activePage,
        CRect(contentLeft, headerHeight + 8,
            contentRight, headerHeight + 40),
        m_text, &m_headingFont);

    if (m_activePage != _T("Dashboard"))
    {
        DrawTextLine(dc,
            _T("Your ") + m_activePage +
            _T(" workspace is ready for integration."),
            CRect(contentLeft, headerHeight + 44,
                contentRight, headerHeight + 75),
            m_muted, &m_normalFont);

        CRect panel(contentLeft, headerHeight + 95,
            contentRight, client.Height() - 25);

        CBrush panelBrush(m_card);
        CPen panelPen(PS_SOLID, 1, m_border);

        oldBrush = dc.SelectObject(&panelBrush);
        oldPen = dc.SelectObject(&panelPen);

        dc.RoundRect(panel, CPoint(12, 12));

        dc.SelectObject(oldBrush);
        dc.SelectObject(oldPen);

        DrawTextLine(dc, m_activePage,
            CRect(panel.left + 28, panel.top + 30,
                panel.right - 20, panel.top + 70),
            m_darkGreen, &m_titleFont);

        DrawTextLine(dc,
            _T("This section will display its live data and controls."),
            CRect(panel.left + 28, panel.top + 78,
                panel.right - 20, panel.top + 110),
            m_muted, &m_normalFont);

        return;
    }

    DrawTextLine(dc, _T("Your workspace"),
        CRect(contentLeft, headerHeight + 43,
            contentRight, headerHeight + 70),
        m_text, &m_headingFont);

    DrawTextLine(dc, _T("A snapshot of your communication activity"),
        CRect(contentLeft, headerHeight + 69,
            contentRight, headerHeight + 92),
        m_muted, &m_smallFont);

    // Four responsive status cards
    int cardsTop = headerHeight + 103;
    int cardGap = 14;
    int cardWidth = (contentWidth - 3 * cardGap) / 4;

    DrawCard(dc,
        CRect(contentLeft, cardsTop,
            contentLeft + cardWidth, cardsTop + 125),
        m_green, _T("CONNECTION"), _T("Ready"),
        _T("Waiting for server"));

    DrawCard(dc,
        CRect(contentLeft + cardWidth + cardGap, cardsTop,
            contentLeft + 2 * cardWidth + cardGap,
            cardsTop + 125),
        RGB(25, 174, 190), _T("MESSAGES"), _T("0"),
        _T("No messages loaded"));

    DrawCard(dc,
        CRect(contentLeft + 2 * (cardWidth + cardGap), cardsTop,
            contentLeft + 3 * cardWidth + 2 * cardGap,
            cardsTop + 125),
        RGB(130, 100, 215), _T("CONTACTS"), _T("0"),
        _T("No contacts loaded"));

    DrawCard(dc,
        CRect(contentLeft + 3 * (cardWidth + cardGap), cardsTop,
            contentRight, cardsTop + 125),
        RGB(224, 155, 55), _T("PRESENCE"), _T("Offline"),
        _T("Awaiting connection"));

    // Activity and conversations panels
    int panelsTop = cardsTop + 145;
    int bottomMargin = 22;
    int panelHeight = max(160, client.Height() - panelsTop - bottomMargin);
    int leftPanelWidth = (contentWidth - 14) * 56 / 100;

    CRect activityPanel(
        contentLeft, panelsTop,
        contentLeft + leftPanelWidth, panelsTop + panelHeight);

    CRect conversationsPanel(
        activityPanel.right + 14, panelsTop,
        contentRight, panelsTop + panelHeight);

    CBrush whitePanel(m_card);
    CPen panelBorder(PS_SOLID, 1, m_border);

    oldBrush = dc.SelectObject(&whitePanel);
    oldPen = dc.SelectObject(&panelBorder);

    dc.RoundRect(activityPanel, CPoint(12, 12));
    dc.RoundRect(conversationsPanel, CPoint(12, 12));

    dc.SelectObject(oldBrush);
    dc.SelectObject(oldPen);

    DrawTextLine(dc, _T("Activity overview"),
        CRect(activityPanel.left + 20, panelsTop + 15,
            activityPanel.right - 15, panelsTop + 43),
        m_text, &m_headingFont);

    DrawTextLine(dc, _T("Your recent workspace activity"),
        CRect(activityPanel.left + 20, panelsTop + 42,
            activityPanel.right - 15, panelsTop + 65),
        m_muted, &m_smallFont);

    const TCHAR* activities[] = {
        _T("Workspace created"),
        _T("Messaging"),
        _T("Community"),
        _T("System monitoring")
    };

    const TCHAR* descriptions[] = {
        _T("Nexora is ready for configuration."),
        _T("Connect the server to enable live chat."),
        _T("Your social spaces will appear here."),
        _T("Live service indicators are awaiting data.")
    };

    COLORREF activityColors[] = {
        RGB(0, 153, 102),
        RGB(25, 174, 190),
        RGB(130, 100, 215),
        RGB(224, 155, 55)
    };

    int activityY = panelsTop + 88;

    for (int i = 0; i < 4; ++i)
    {
        CRect dot(activityPanel.left + 20, activityY + 5,
            activityPanel.left + 28, activityY + 13);

        CBrush dotBrush(activityColors[i]);
        CPen dotPen(PS_SOLID, 1, activityColors[i]);

        oldBrush = dc.SelectObject(&dotBrush);
        oldPen = dc.SelectObject(&dotPen);

        dc.Ellipse(dot);

        dc.SelectObject(oldBrush);
        dc.SelectObject(oldPen);

        DrawTextLine(dc, activities[i],
            CRect(activityPanel.left + 39, activityY,
                activityPanel.right - 15, activityY + 23),
            m_text, &m_headingFont);

        DrawTextLine(dc, descriptions[i],
            CRect(activityPanel.left + 39, activityY + 22,
                activityPanel.right - 15, activityY + 43),
            m_muted, &m_smallFont);

        activityY += 62;
    }

    DrawTextLine(dc, _T("Recent conversations"),
        CRect(conversationsPanel.left + 20, panelsTop + 15,
            conversationsPanel.right - 15, panelsTop + 43),
        m_text, &m_headingFont);

    DrawTextLine(dc, _T("Your latest chats will appear here"),
        CRect(conversationsPanel.left + 20, panelsTop + 42,
            conversationsPanel.right - 15, panelsTop + 65),
        m_muted, &m_smallFont);

    int centerX = (conversationsPanel.left +
        conversationsPanel.right) / 2;

    int centerY = panelsTop + panelHeight / 2 - 12;

    CRect messageIcon(centerX - 24, centerY - 44,
        centerX + 24, centerY + 4);

    CBrush iconBrush(RGB(231, 247, 239));
    CPen iconPen(PS_SOLID, 1, RGB(231, 247, 239));

    oldBrush = dc.SelectObject(&iconBrush);
    oldPen = dc.SelectObject(&iconPen);

    dc.Ellipse(messageIcon);

    dc.SelectObject(oldBrush);
    dc.SelectObject(oldPen);

    DrawTextLine(dc, _T("N"), messageIcon,
        m_green, &m_headingFont,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    DrawTextLine(dc, _T("No conversations yet"),
        CRect(conversationsPanel.left + 10, centerY + 10,
            conversationsPanel.right - 10, centerY + 40),
        m_text, &m_headingFont,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    DrawTextLine(dc, _T("Connect to the server to start chatting."),
        CRect(conversationsPanel.left + 10, centerY + 40,
            conversationsPanel.right - 10, centerY + 65),
        m_muted, &m_smallFont,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void CDashboardPage::OnLButtonDown(
    UINT nFlags, CPoint point)
{
    const int sidebarWidth = 220;

    if (point.x >= 10 && point.x < sidebarWidth)
    {
        const TCHAR* menuItems[] = {
            _T("Dashboard"), _T("Messages"), _T("Contacts"),
            _T("Social Feed"), _T("Stories"), _T("Groups"),
            _T("History")
        };

        int navY = 126;

        for (int i = 0; i < 7; ++i)
        {
            CRect item(10, navY, sidebarWidth - 10,
                navY + 43);

            if (item.PtInRect(point))
            {
                m_activePage = menuItems[i];
                Invalidate(FALSE);
                return;
            }

            navY += 46;
        }

        navY = 126 + 7 * 46 + 8;

        if (point.y >= navY && point.y < navY + 39)
        {
            m_activePage = _T("Settings");
            Invalidate(FALSE);
            return;
        }

        if (point.y >= navY + 39 &&
            point.y < navY + 81)
        {
            m_activePage = _T("Diagnostics");
            Invalidate(FALSE);
            return;
        }
    }

    CWnd::OnLButtonDown(nFlags, point);
}

void CDashboardPage::OnMouseMove(
    UINT nFlags, CPoint point)
{
    if (point.x < 220)
        ::SetCursor(::LoadCursor(nullptr, IDC_HAND));
    else
        ::SetCursor(::LoadCursor(nullptr, IDC_ARROW));

    CWnd::OnMouseMove(nFlags, point);
}
