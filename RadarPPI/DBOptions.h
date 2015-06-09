#pragma once


// DBOptions dialog

class DBOptions : public CDialogEx
{
	DECLARE_DYNAMIC(DBOptions)

public:
	DBOptions(CWnd* pParent = NULL);   // standard constructor
	virtual ~DBOptions();

// Dialog Data
	enum { IDD = IDD_DB_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_iPort;
	CString m_strUserName;
	CString m_strPassword;
	CString m_strSID;
	int m_boolSrvNameOrSID;
	CString m_strConfigName;
	CString m_strDataName;
	int m_iSize;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CFont m_font;
	virtual BOOL OnInitDialog();
	BOOL m_bOverwrite;
	BOOL m_bShowProgress;
	DWORD m_ip;
};
