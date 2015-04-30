
// RadarPPIDlg.h : header file
//

#pragma once
#include "atltypes.h"
#include "afxwin.h"

#define WM_TARGET_UPDATE WM_USER+1001
typedef struct pos{
	int x;
	int y;
	int z;
}Pos;

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
	int m_targetx;
	int m_targety;
	int m_oldtargetx;
	int m_oldtargety;
public:
	double m_theta;
	int m_iRadarState;
public:
	afx_msg void OnBnClickedStart();
	CButton m_btnPause;
	CButton m_btnStart;
	afx_msg void OnBnClickedPause();
	CString m_sOutput;
	void AddToOutput(const char *str);
};
