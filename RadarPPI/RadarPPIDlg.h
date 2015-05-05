
// RadarPPIDlg.h : header file
//

#pragma once
#include "atltypes.h"
#include "afxwin.h"
#include <vector>
#define WM_TARGET_UPDATE WM_USER+1001
typedef struct pos{
	double x;
	double y;
	double z;
}Pos;

typedef struct ThreadData{
	HWND hwnd;
	SOCKET sock;
}ThreadData;

typedef struct ThreadRetData{
	char *buf;
}ThreadRetData;

enum{RADAR_OFF = 0 , RADAR_ON = 1, RADAR_PAUSE = 2};

// CRadarPPIDlg dialog
class CRadarPPIDlg : public CDialogEx
{
// Construction
public:
	CRadarPPIDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_RADARPPI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnTargetUpdate(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
private:
	double m_th;
public:
	void Draw(CDC * pDC);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
private:
	CRect m_canvas;
	std::vector<int> m_targetx;
	std::vector<int> m_targety;
	std::vector<int> m_oldtargetx;
	std::vector<int> m_oldtargety;
	std::vector<double> m_distance;
	std::vector<double> m_theta;
public:
//	int m_iRadarState;
public:
	afx_msg void OnBnClickedStart();
	CButton m_btnPause;
	CButton m_btnStart;
	afx_msg void OnBnClickedPause();
	CString m_sOutput;
	void AddToOutput(const char *str);
	
	CString m_strIPAddr;
	DWORD m_dwIP;
	int m_strPort;
	BOOL m_bUseThreads;
	double m_iLocationX;
	double m_iLocationY;
	void PrepareSock(void);
	SOCKET m_sock;
	HANDLE m_bkgThread;


	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	std::vector<COLORREF> m_clrs;
	int m_clrSelected;
	afx_msg void OnBnClickedCancel();
};
