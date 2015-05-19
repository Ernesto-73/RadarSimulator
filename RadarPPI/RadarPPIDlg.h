
// RadarPPIDlg.h : header file
//

#pragma once
#include "atltypes.h"
#include "afxwin.h"
#include <vector>
#define WM_TARGET_UPDATE WM_USER+1001

#define BUF_SIZE 500

typedef struct pos{
	double x;
	double y;
	double z;
}Pos;

typedef struct ThreadData{
	HWND hwnd;
	SOCKET sock;
}ThreadData;

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
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedPause();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()

private:
	double m_th;

	void Draw(CDC * pDC);
	void AddToOutput(const char *str);
	void PrepareSock(void);

	std::vector<double> m_targetx;
	std::vector<double> m_targety;
	std::vector<double> m_oldtargetx;
	std::vector<double> m_oldtargety;
	std::vector<double> m_distance;
	std::vector<double> m_theta;
	
	CRect m_canvas;
	CButton m_btnPause;
	CButton m_btnStart;
	CString m_sOutput;
	
	CString m_strIPAddr;
	DWORD m_dwIP;
	int m_strPort;
	BOOL m_bUseThreads;
	double m_iLocationX;
	double m_iLocationY;
	
	SOCKET m_sock;
	HANDLE m_bkgThread;
	std::vector<COLORREF> m_clrs;
	int m_clrSelected;
	int m_nElapse;
};
