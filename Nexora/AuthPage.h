
#pragma once

#include <afxwin.h>

class CAuthPage : public CWnd
{
public:
    CAuthPage();
    virtual ~CAuthPage();

    BOOL CreatePage(CWnd* pParent);
    BOOL IsAuthenticated() const;
    CString GetUserId() const;

protected:
    CEdit m_nameEdit;
    CEdit m_userEdit;
    CEdit m_emailEdit;
    CEdit m_passwordEdit;
    CEdit m_confirmEdit;

    CButton m_primaryButton;
    CButton m_switchButton;
    CButton m_showPassword;
    CButton m_rememberUser;

    CFont m_font;
    CFont m_headingFont;
    CFont m_logoFont;

    CString m_userId;
    CString m_serverAddress;
    CString m_serverPort;

    bool m_registrationMode;
    bool m_authenticated;

    void CreateControls();
    void LayoutControls();
    void SubmitForm();
    void SwitchMode();
    void UpdatePasswordVisibility();

    BOOL SendAuthRequest(
        const CString& request,
        CString& response);

    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSubmit();
    afx_msg void OnSwitchMode();
    afx_msg void OnShowPassword();
    afx_msg void OnRememberUser();

    DECLARE_MESSAGE_MAP()
};
