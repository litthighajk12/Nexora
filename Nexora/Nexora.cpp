
#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "Nexora.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "NexoraDoc.h"
#include "NexoraView.h"

#include <afxsock.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Automatically launched server process.
// Only this application's own server process is tracked.
static HANDLE g_hNexoraServerProcess = nullptr;


// CNexoraApp

BEGIN_MESSAGE_MAP(CNexoraApp, CWinAppEx)
    ON_COMMAND(ID_APP_ABOUT, &CNexoraApp::OnAppAbout)
    ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()


CNexoraApp::CNexoraApp() noexcept
{
    m_bHiColorIcons = TRUE;
    m_nAppLook = 0;

    m_dwRestartManagerSupportFlags =
        AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;

#ifdef _MANAGED
    System::Windows::Forms::Application::
        SetUnhandledExceptionMode(
            System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

    SetAppID(_T("Nexora.AppID.NoVersion"));
}


CNexoraApp theApp;


// Check whether a TCP service accepts connections on localhost:5000.
static bool IsNexoraServerAvailable()
{
    CSocket testSocket;

    if (!testSocket.Create())
        return false;

    BOOL connected = testSocket.Connect(
        _T("127.0.0.1"), 5000);

    testSocket.Close();

    return connected == TRUE;
}


// Start the server only when it is not already listening.
static BOOL StartNexoraServer()
{
    // Do not launch another server if port 5000 is already responding.
    if (IsNexoraServerAvailable())
        return TRUE;

    TCHAR executablePath[MAX_PATH] = {};

    DWORD length = GetModuleFileName(
        nullptr, executablePath, MAX_PATH);

    if (length == 0 || length >= MAX_PATH)
    {
        AfxMessageBox(
            _T("Unable to determine the Nexora executable path."));
        return FALSE;
    }

    CString folder(executablePath);
    int slash = folder.ReverseFind(_T('\\'));

    if (slash < 0)
    {
        AfxMessageBox(_T("Unable to determine the application folder."));
        return FALSE;
    }

    folder = folder.Left(slash + 1);

    CString serverPath = folder + _T("NexoraServer.exe");

    if (GetFileAttributes(serverPath) == INVALID_FILE_ATTRIBUTES)
    {
        AfxMessageBox(
            _T("NexoraServer.exe was not found.\n\n")
            _T("Build both projects using the same configuration ")
            _T("(Debug x64 or Release x64)."));
        return FALSE;
    }

    CString commandLine;
    commandLine.Format(_T("\"%s\""), (LPCTSTR)serverPath);

    STARTUPINFO startupInfo = {};
    startupInfo.cb = sizeof(startupInfo);

    PROCESS_INFORMATION processInfo = {};

    // A new console keeps server output visible for assessment demos.
    BOOL created = CreateProcess(
        serverPath.GetString(),
        commandLine.GetBuffer(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NEW_CONSOLE,
        nullptr,
        folder,
        &startupInfo,
        &processInfo);

    commandLine.ReleaseBuffer();

    if (!created)
    {
        DWORD error = GetLastError();

        CString message;
        message.Format(
            _T("Could not start NexoraServer.exe.\n")
            _T("Windows error: %lu"),
            error);

        AfxMessageBox(message);
        return FALSE;
    }

    g_hNexoraServerProcess = processInfo.hProcess;

    CloseHandle(processInfo.hThread);

    // Give the server a short startup window without blocking indefinitely.
    for (int attempt = 0; attempt < 20; ++attempt)
    {
        if (IsNexoraServerAvailable())
            return TRUE;

        DWORD processState =
            WaitForSingleObject(g_hNexoraServerProcess, 0);

        if (processState == WAIT_OBJECT_0)
        {
            AfxMessageBox(
                _T("NexoraServer.exe exited before accepting connections.\n")
                _T("Check the server console for its error message."));
            return FALSE;
        }

        Sleep(250);
    }

    AfxMessageBox(
        _T("NexoraServer.exe was launched, but port 5000 ")
        _T("did not become available in time.\n\n")
        _T("Check the server console before continuing."));

    return FALSE;
}


// CNexoraApp initialization

BOOL CNexoraApp::InitInstance()
{
    INITCOMMONCONTROLSEX InitCtrls = {};
    InitCtrls.dwSize = sizeof(InitCtrls);
    InitCtrls.dwICC = ICC_WIN95_CLASSES;

    InitCommonControlsEx(&InitCtrls);

    CWinAppEx::InitInstance();

    if (!AfxSocketInit())
    {
        AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
        return FALSE;
    }

    // Initialize OLE libraries.
    if (!AfxOleInit())
    {
        AfxMessageBox(IDP_OLE_INIT_FAILED);
        return FALSE;
    }

    AfxEnableControlContainer();
    EnableTaskbarInteraction(FALSE);

    SetRegistryKey(
        _T("Nexora Application"));

    LoadStdProfileSettings(4);

    InitContextMenuManager();
    InitKeyboardManager();
    InitTooltipManager();

    CMFCToolTipInfo ttParams;
    ttParams.m_bVislManagerTheme = TRUE;

    GetTooltipManager()->SetTooltipParams(
        AFX_TOOLTIP_TYPE_ALL,
        RUNTIME_CLASS(CMFCToolTipCtrl),
        &ttParams);

    // Start the TCP server automatically.
    if (!StartNexoraServer())
    {
        // Do not silently continue when the server failed to start.
        if (g_hNexoraServerProcess)
        {
            CloseHandle(g_hNexoraServerProcess);
            g_hNexoraServerProcess = nullptr;
        }

        return FALSE;
    }

    // Register the MDI document template.
    CMultiDocTemplate* pDocTemplate =
        new CMultiDocTemplate(
            IDR_NexoraTYPE,
            RUNTIME_CLASS(CNexoraDoc),
            RUNTIME_CLASS(CChildFrame),
            RUNTIME_CLASS(CNexoraView));

    if (pDocTemplate == nullptr)
        return FALSE;

    AddDocTemplate(pDocTemplate);

    // Create the main MDI frame.
    CMainFrame* pMainFrame = new CMainFrame;

    if (!pMainFrame ||
        !pMainFrame->LoadFrame(IDR_MAINFRAME))
    {
        delete pMainFrame;
        return FALSE;
    }

    m_pMainWnd = pMainFrame;

    // Process command-line options.
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

    if (!ProcessShellCommand(cmdInfo))
        return FALSE;

    pMainFrame->ShowWindow(m_nCmdShow);
    pMainFrame->UpdateWindow();

    return TRUE;
}


// Shut down the server process launched by this client.
int CNexoraApp::ExitInstance()
{
    if (g_hNexoraServerProcess != nullptr)
    {
        DWORD state =
            WaitForSingleObject(g_hNexoraServerProcess, 0);

        if (state == WAIT_TIMEOUT)
        {
            // Request graceful shutdown in a future server implementation.
            // For this prototype, terminate only the process we launched.
            TerminateProcess(g_hNexoraServerProcess, 0);

            WaitForSingleObject(
                g_hNexoraServerProcess, 2000);
        }

        CloseHandle(g_hNexoraServerProcess);
        g_hNexoraServerProcess = nullptr;
    }

    AfxOleTerm(FALSE);

    return CWinAppEx::ExitInstance();
}


// About dialog

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg() noexcept;

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() noexcept
    : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


void CNexoraApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}


// Customization state

void CNexoraApp::PreLoadState()
{
    CString strName;
    BOOL bNameValid = strName.LoadString(IDS_EDIT_MENU);

    ASSERT(bNameValid);

    GetContextMenuManager()->AddMenu(
        strName, IDR_POPUP_EDIT);
}

void CNexoraApp::LoadCustomState()
{
}

void CNexoraApp::SaveCustomState()
{
}
