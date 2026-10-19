
#include "pch.h"
#include "framework.h"
#include "AuthPage.h"

#include <afxsock.h>
#include <atlconv.h>
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
    const COLORREF BG_COLOR =
        RGB(242, 249, 246);

    const COLORREF CARD_COLOR =
        RGB(255, 255, 255);

    const COLORREF GREEN =
        RGB(0, 153, 102);

    const COLORREF DARK_TEXT =
        RGB(30, 41, 40);

    const COLORREF MUTED_TEXT =
        RGB(105, 122, 120);

    const COLORREF BORDER =
        RGB(210, 229, 218);

    const UINT ID_AUTH_PRIMARY = 5101;
    const UINT ID_AUTH_SWITCH = 5102;
    const UINT ID_AUTH_SHOW = 5103;
    const UINT ID_AUTH_REMEMBER = 5104;
}

BEGIN_MESSAGE_MAP(CAuthPage, CWnd)
    ON_WM_PAINT()
    ON_WM_SIZE()
    ON_BN_CLICKED(ID_AUTH_PRIMARY, &CAuthPage::OnSubmit)
    ON_BN_CLICKED(ID_AUTH_SWITCH, &CAuthPage::OnSwitchMode)
    ON_BN_CLICKED(ID_AUTH_SHOW, &CAuthPage::OnShowPassword)
    ON_BN_CLICKED(ID_AUTH_REMEMBER, &CAuthPage::OnRememberUser)
END_MESSAGE_MAP()


CAuthPage::CAuthPage()
    : m_registrationMode(false),
    m_authenticated(false)
{
    m_serverAddress = _T("127.0.0.1");
    m_serverPort = _T("5000");

    m_font.CreateFont(
        14, 0, 0, 0, FW_NORMAL,
        FALSE, FALSE, 0, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS,
        _T("Segoe UI"));

    m_headingFont.CreateFont(
        25, 0, 0, 0, FW_BOLD,
        FALSE, FALSE, 0, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS,
        _T("Segoe UI"));

    m_logoFont.CreateFont(
        27, 0, 0, 0, FW_BOLD,
        FALSE, FALSE, 0, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS,
        _T("Segoe UI"));
}

CAuthPage::~CAuthPage()
{
}

BOOL CAuthPage::CreatePage(CWnd* pParent)
{
    if (pParent == nullptr)
        return FALSE;

    CString className = AfxRegisterWndClass(
        CS_HREDRAW | CS_VREDRAW,
        ::LoadCursor(nullptr, IDC_ARROW),
        (HBRUSH)(COLOR_WINDOW + 1),
        nullptr);

    CRect rect;
    pParent->GetClientRect(&rect);

    if (!CreateEx(
        0,
        className,
        _T("Nexora Authentication"),
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
        rect,
        pParent,
        0))
    {
        return FALSE;
    }

    CreateControls();
    LayoutControls();

    return TRUE;
}

BOOL CAuthPage::IsAuthenticated() const
{
    return m_authenticated ? TRUE : FALSE;
}

CString CAuthPage::GetUserId() const
{
    return m_userId;
}


void CAuthPage::CreateControls()
{
    DWORD editStyle =
        WS_CHILD | WS_VISIBLE | WS_TABSTOP |
        WS_BORDER | ES_AUTOHSCROLL;

    m_nameEdit.Create(
        editStyle, CRect(0, 0, 0, 0),
        this, 5111);

    m_userEdit.Create(
        editStyle, CRect(0, 0, 0, 0),
        this, 5112);

    m_emailEdit.Create(
        editStyle, CRect(0, 0, 0, 0),
        this, 5113);

    m_passwordEdit.Create(
        editStyle | ES_PASSWORD,
        CRect(0, 0, 0, 0),
        this, 5114);

    m_confirmEdit.Create(
        editStyle | ES_PASSWORD,
        CRect(0, 0, 0, 0),
        this, 5115);

    m_primaryButton.Create(
        _T("Sign In"),
        WS_CHILD | WS_VISIBLE | WS_TABSTOP |
        BS_PUSHBUTTON,
        CRect(0, 0, 0, 0),
        this, ID_AUTH_PRIMARY);

    m_switchButton.Create(
        _T("New to Nexora? Create an account"),
        WS_CHILD | WS_VISIBLE | WS_TABSTOP |
        BS_PUSHBUTTON,
        CRect(0, 0, 0, 0),
        this, ID_AUTH_SWITCH);

    m_showPassword.Create(
        _T("Show password"),
        WS_CHILD | WS_VISIBLE | WS_TABSTOP |
        BS_AUTOCHECKBOX,
        CRect(0, 0, 0, 0),
        this, ID_AUTH_SHOW);

    m_rememberUser.Create(
        _T("Remember email"),
        WS_CHILD | WS_VISIBLE | WS_TABSTOP |
        BS_AUTOCHECKBOX,
        CRect(0, 0, 0, 0),
        this, ID_AUTH_REMEMBER);

    CWnd* controls[] =
    {
        &m_nameEdit,
        &m_userEdit,
        &m_emailEdit,
        &m_passwordEdit,
        &m_confirmEdit,
        &m_primaryButton,
        &m_switchButton,
        &m_showPassword,
        &m_rememberUser
    };

    for (CWnd* control : controls)
    {
        if (::IsWindow(control->GetSafeHwnd()))
            control->SetFont(&m_font);
    }

    m_nameEdit.SetLimitText(100);
    m_userEdit.SetLimitText(40);
    m_emailEdit.SetLimitText(254);
    m_passwordEdit.SetLimitText(128);
    m_confirmEdit.SetLimitText(128);

    m_nameEdit.ShowWindow(SW_HIDE);
    m_emailEdit.ShowWindow(SW_HIDE);
    m_confirmEdit.ShowWindow(SW_HIDE);
}


void CAuthPage::LayoutControls()
{
    if (!::IsWindow(m_userEdit.GetSafeHwnd()))
        return;

    CRect client;
    GetClientRect(&client);

    int width = (std::min)(540, client.Width() - 32);
    width = (std::max)(320, width);

    int height = m_registrationMode ? 690 : 570;

    int left = (client.Width() - width) / 2;
    int top = (std::max)(12, (client.Height() - height) / 2);

    const int pad = 42;
    const int x = left + pad;
    const int w = width - pad * 2;
    const int editHeight = 36;
    const int rowGap = 66;

    int y = top + 178;

    if (m_registrationMode)
    {
        m_nameEdit.ShowWindow(SW_SHOW);
        m_userEdit.ShowWindow(SW_SHOW);
        m_emailEdit.ShowWindow(SW_SHOW);
        m_confirmEdit.ShowWindow(SW_SHOW);

        m_nameEdit.MoveWindow(x, y, w, editHeight);
        y += rowGap;

        m_userEdit.MoveWindow(x, y, w, editHeight);
        y += rowGap;

        m_emailEdit.MoveWindow(x, y, w, editHeight);
        y += rowGap;

        m_passwordEdit.MoveWindow(x, y, w, editHeight);
        y += rowGap;

        m_confirmEdit.MoveWindow(x, y, w, editHeight);
    }
    else
    {
        m_nameEdit.ShowWindow(SW_HIDE);
        m_emailEdit.ShowWindow(SW_HIDE);
        m_confirmEdit.ShowWindow(SW_HIDE);

        m_userEdit.ShowWindow(SW_SHOW);
        m_userEdit.MoveWindow(x, y, w, editHeight);

        y += rowGap;

        m_passwordEdit.MoveWindow(x, y, w, editHeight);
    }

    int optionsY = top + height - 145;

    m_showPassword.MoveWindow(
        x, optionsY, 145, 27);

    m_rememberUser.MoveWindow(
        x + 155, optionsY, 180, 27);

    m_primaryButton.MoveWindow(
        x, top + height - 105, w, 40);

    m_switchButton.MoveWindow(
        x, top + height - 57, w, 36);

    m_primaryButton.SetWindowText(
        m_registrationMode
        ? _T("Create Account")
        : _T("Sign In"));

    m_switchButton.SetWindowText(
        m_registrationMode
        ? _T("Already have an account? Sign In")
        : _T("New to Nexora? Create an account"));

    Invalidate(FALSE);
}


void CAuthPage::OnPaint()
{
    CPaintDC dc(this);

    CRect client;
    GetClientRect(&client);
    dc.FillSolidRect(client, BG_COLOR);

    int width = (std::min)(540, client.Width() - 32);
    width = (std::max)(320, width);

    int height = m_registrationMode ? 690 : 570;
    int left = (client.Width() - width) / 2;
    int top = (std::max)(12, (client.Height() - height) / 2);

    CRect card(left, top, left + width, top + height);

    CBrush cardBrush(CARD_COLOR);
    CPen borderPen(PS_SOLID, 1, BORDER);

    CBrush* oldBrush = dc.SelectObject(&cardBrush);
    CPen* oldPen = dc.SelectObject(&borderPen);

    dc.RoundRect(card, CPoint(12, 12));

    dc.SelectObject(oldBrush);
    dc.SelectObject(oldPen);

    dc.SetBkMode(TRANSPARENT);

    // Logo
    CRect logo(left + 42, top + 35,
        left + 90, top + 83);

    CBrush logoBrush(GREEN);
    CPen logoPen(PS_SOLID, 1, GREEN);

    oldBrush = dc.SelectObject(&logoBrush);
    oldPen = dc.SelectObject(&logoPen);

    dc.RoundRect(logo, CPoint(8, 8));

    dc.SelectObject(oldBrush);
    dc.SelectObject(oldPen);

    CFont* oldFont = dc.SelectObject(&m_logoFont);

    dc.SetTextColor(RGB(255, 255, 255));
    dc.DrawText(
        _T("N"), logo,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Brand
    dc.SelectObject(&m_headingFont);
    dc.SetTextColor(GREEN);

    CRect brand(left + 102, top + 34,
        left + width - 24, top + 65);

    dc.DrawText(
        _T("NEXORA"), brand,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // Tagline
    dc.SelectObject(&m_font);
    dc.SetTextColor(MUTED_TEXT);

    CRect tagline(left + 102, top + 67,
        left + width - 24, top + 94);

    dc.DrawText(
        _T("YOUR PEOPLE. YOUR WORLD."),
        tagline,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // Page heading
    dc.SelectObject(&m_headingFont);
    dc.SetTextColor(DARK_TEXT);

    CRect title(left + 42, top + 105,
        left + width - 30, top + 140);

    dc.DrawText(
        m_registrationMode
        ? _T("Create your account")
        : _T("Welcome back"),
        title,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // Subtitle
    dc.SelectObject(&m_font);
    dc.SetTextColor(MUTED_TEXT);

    CRect subtitle(left + 42, top + 143,
        left + width - 25, top + 168);

    dc.DrawText(
        m_registrationMode
        ? _T("Join your people and communities.")
        : _T("Sign in to continue to your workspace."),
        subtitle,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // Field labels aligned above their corresponding edits
    dc.SetTextColor(DARK_TEXT);

    int labelX = left + 42;
    int labelY = top + 153;

    if (m_registrationMode)
    {
        dc.TextOut(labelX, labelY, _T("FULL NAME"));
        labelY += 66;

        dc.TextOut(labelX, labelY, _T("USERNAME"));
        labelY += 66;

        dc.TextOut(labelX, labelY, _T("EMAIL ADDRESS"));
        labelY += 66;

        dc.TextOut(labelX, labelY, _T("PASSWORD"));
        labelY += 66;

        dc.TextOut(labelX, labelY, _T("CONFIRM PASSWORD"));
    }
    else
    {
        dc.TextOut(labelX, labelY, _T("EMAIL ADDRESS"));
        labelY += 66;

        dc.TextOut(labelX, labelY, _T("PASSWORD"));
    }

    // Footer stays inside the card
    dc.SetTextColor(MUTED_TEXT);

    CRect footer(
        left + 25, top + height - 20,
        left + width - 25, top + height - 5);

    dc.DrawText(
        _T("NEXORA  |  CONNECT. SHARE. EXPERIENCE."),
        footer,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    dc.SelectObject(oldFont);
}


void CAuthPage::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);

    if (::IsWindow(m_userEdit.GetSafeHwnd()))
        LayoutControls();
}


void CAuthPage::OnSwitchMode()
{
    m_registrationMode = !m_registrationMode;

    m_showPassword.SetCheck(BST_UNCHECKED);
    UpdatePasswordVisibility();

    m_passwordEdit.SetWindowText(_T(""));
    m_confirmEdit.SetWindowText(_T(""));

    if (m_registrationMode)
    {
        m_userEdit.SetWindowText(_T(""));
        m_emailEdit.SetWindowText(_T(""));
    }
    else
    {
        CString email;
        m_emailEdit.GetWindowText(email);

        if (!email.IsEmpty())
            m_userEdit.SetWindowText(email);
    }

    LayoutControls();
    Invalidate(FALSE);
}


void CAuthPage::OnShowPassword()
{
    UpdatePasswordVisibility();
}


void CAuthPage::UpdatePasswordVisibility()
{
    BOOL show = (m_showPassword.GetCheck() == BST_CHECKED);

    m_passwordEdit.SetPasswordChar(show ? 0 : _T('*'));
    m_confirmEdit.SetPasswordChar(show ? 0 : _T('*'));

    m_passwordEdit.Invalidate();
    m_confirmEdit.Invalidate();
}


void CAuthPage::OnRememberUser()
{
    if (m_rememberUser.GetCheck() != BST_CHECKED)
        return;

    CString email;

    if (m_registrationMode)
        m_emailEdit.GetWindowText(email);
    else
        m_userEdit.GetWindowText(email);

    if (!email.IsEmpty())
    {
        AfxMessageBox(
            _T("Email entered. Persistent remember-email storage ")
            _T("will be added with session settings."));
    }
}


BOOL CAuthPage::SendAuthRequest(
    const CString& request,
    CString& response)
{
    response.Empty();

    CSocket client;

    if (!client.Create())
    {
        response = _T("Unable to create TCP socket.");
        return FALSE;
    }

    UINT port = (UINT)_ttoi(m_serverPort);

    if (port == 0)
    {
        response = _T("Invalid server port.");
        client.Close();
        return FALSE;
    }

    if (!client.Connect(m_serverAddress, port))
    {
        response =
            _T("Cannot connect to the Nexora server.\n\n")
            _T("Check that NexoraServer.exe is running and ")
            _T("the server address and port are correct.");

        client.Close();
        return FALSE;
    }

    CString message = request + _T("\n");
    CStringA bytes;

    {
        CT2A converted(message, CP_UTF8);
        bytes = converted;
    }

    int sent = client.Send(
        (LPCSTR)bytes, bytes.GetLength());

    if (sent == SOCKET_ERROR || sent != bytes.GetLength())
    {
        response = _T("Failed to send the request to the server.");
        client.Close();
        return FALSE;
    }

    char buffer[2048] = {};
    int received = client.Receive(buffer, sizeof(buffer) - 1);

    if (received <= 0)
    {
        response = _T("The server did not return a response.");
        client.Close();
        return FALSE;
    }

    buffer[received] = '\0';

    {
        CA2T converted(buffer, CP_UTF8);
        response = converted;
    }

    response.Trim();
    client.Close();

    return TRUE;
}


void CAuthPage::SubmitForm()
{
    CString name;
    CString username;
    CString email;
    CString password;
    CString confirm;

    if (m_registrationMode)
    {
        m_nameEdit.GetWindowText(name);
        m_userEdit.GetWindowText(username);
        m_emailEdit.GetWindowText(email);
    }
    else
    {
        m_userEdit.GetWindowText(email);
    }

    m_passwordEdit.GetWindowText(password);

    if (m_registrationMode)
        m_confirmEdit.GetWindowText(confirm);

    name.Trim();
    username.Trim();
    email.Trim();

    if (m_registrationMode)
    {
        if (name.IsEmpty() || username.IsEmpty() ||
            email.IsEmpty() || password.IsEmpty() ||
            confirm.IsEmpty())
        {
            AfxMessageBox(_T("Please fill in every field."));
            return;
        }

        if (username.Find(_T('|')) >= 0 ||
            email.Find(_T('|')) >= 0 ||
            username.Find(_T('\n')) >= 0 ||
            email.Find(_T('\n')) >= 0)
        {
            AfxMessageBox(
                _T("Username and email cannot contain protocol separators."));
            return;
        }

        int at = email.Find(_T('@'));

        if (at <= 0 ||
            email.Find(_T('.'), at) == -1 ||
            email.Find(_T(' ')) >= 0)
        {
            AfxMessageBox(_T("Enter a valid email address."));
            return;
        }

        if (username.GetLength() < 3 ||
            username.GetLength() > 40)
        {
            AfxMessageBox(
                _T("Username must contain 3 to 40 characters."));
            return;
        }

        if (password.GetLength() < 8 ||
            password.GetLength() > 128)
        {
            AfxMessageBox(
                _T("Password must contain 8 to 128 characters."));
            return;
        }

        bool upper = false;
        bool lower = false;
        bool digit = false;
        bool special = false;

        for (int i = 0; i < password.GetLength(); ++i)
        {
            TCHAR ch = password[i];

            if (ch >= _T('A') && ch <= _T('Z'))
                upper = true;
            else if (ch >= _T('a') && ch <= _T('z'))
                lower = true;
            else if (ch >= _T('0') && ch <= _T('9'))
                digit = true;
            else
                special = true;
        }

        if (!upper || !lower || !digit || !special)
        {
            AfxMessageBox(
                _T("Password must contain uppercase and lowercase ")
                _T("letters, a number, and a special character."));
            return;
        }

        if (password != confirm)
        {
            AfxMessageBox(_T("Passwords do not match."));
            return;
        }
    }
    else
    {
        if (email.IsEmpty() || password.IsEmpty())
        {
            AfxMessageBox(
                _T("Enter your email address and password."));
            return;
        }

        if (email.Find(_T('@')) <= 0 ||
            email.Find(_T('.'), email.Find(_T('@'))) == -1)
        {
            AfxMessageBox(_T("Enter a valid email address."));
            return;
        }
    }

    // Compatible with the current prototype server protocol.
    // The current server accepts REGISTER|email|password
    // and LOGIN|email|password.
    CString request;

    request.Format(
        _T("%s|%s|%s"),
        m_registrationMode ? _T("REGISTER") : _T("LOGIN"),
        (LPCTSTR)email,
        (LPCTSTR)password);

    CString response;

    m_primaryButton.EnableWindow(FALSE);
    BOOL connected = SendAuthRequest(request, response);
    m_primaryButton.EnableWindow(TRUE);

    if (!connected)
    {
        AfxMessageBox(response, MB_ICONWARNING);
        return;
    }

    if (response.Left(3).CompareNoCase(_T("OK|")) == 0 ||
        response.CompareNoCase(_T("OK")) == 0)
    {
        if (m_registrationMode)
        {
            AfxMessageBox(
                _T("Account created successfully. Please sign in."));

            m_registrationMode = false;
            m_userEdit.SetWindowText(email);
            m_passwordEdit.SetWindowText(_T(""));
            m_confirmEdit.SetWindowText(_T(""));
            m_showPassword.SetCheck(BST_UNCHECKED);

            UpdatePasswordVisibility();
            LayoutControls();
            Invalidate(FALSE);
            return;
        }

        m_userId = email;
        m_authenticated = true;

        CWnd* pView = GetParent();

        ShowWindow(SW_HIDE);

        if (pView != nullptr)
        {
            pView->PostMessage(WM_APP + 101);
        }

        return;
    }

    AfxMessageBox(
        response.IsEmpty()
        ? _T("The server rejected the request.")
        : response,
        MB_ICONWARNING);
}

void CAuthPage::OnSubmit()
{
    SubmitForm();
}
