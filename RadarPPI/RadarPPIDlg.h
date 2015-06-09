
// RadarPPIDlg.h : header file
//

#pragma once
#include "atltypes.h"
#include "afxwin.h"
#include <vector>
#include "afxext.h"
#include "occi.h"
#include "math.h"
#include "TrackRecorderDlg.h"

using namespace oracle::occi;;

#define WM_TARGET_UPDATE WM_USER+1001
#define BUF_SIZE 500

int genRand(int min, int max);

typedef struct TComplex{
	TComplex()
	{
		real = 0;
		im = 0;
	}
	double real;
	double im;
}TComplex;

typedef struct RadarParams{
	double PRI;
	double PW;
	double Amp;
	double AntennaVelocity;
	std::vector<double> AntennaGain;
	double Th;
	double SamplingRate;
	double BW; // Receive Bandwidth;
	int BufferSize;
	int PRISize;
	int Num;
}RadarParams;

static UINT BASED_CODE indicators[] =   
{
	ID_SEPARATOR,
	ID_INDICATOR_TIME,
	ID_INDICATOR_STATUS
};

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
enum{DATABASE_CONNECTED = 0, NUM};

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
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
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
	std::vector<double> m_snr;

	CRect m_canvas;
	CButton m_btnPause;
	CButton m_btnStart;
	CString m_sOutput;
	
	CString m_strIPAddr;
	DWORD m_dwIP;
	int m_port;
	BOOL m_bAutoDispatch;
	double m_LocationX;
	double m_LocationY;
	
	SOCKET m_sock;
	HANDLE m_bkgThread;
	std::vector<COLORREF> m_clrs;
	int m_clrSelected;
	int m_nElapse;
	int m_nBufferSize;
	CScrollBar m_scrollBarPW;
	CFont m_font;
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CScrollBar m_scrollBarThresh;
	CRect m_large;
	CRect m_small;
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	bool m_bIsSmallWindow;
private:
	CMenu m_menu;
	CStatusBar m_StatusBar;
public:
	afx_msg void OnHelpAbout();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDatabaseConnect();
private:
	Environment* m_env;
	Connection* m_conn;
	int m_arrOptions[NUM];
	afx_msg void OnUpdateDatabaseConnect(CCmdUI *pCmdUI);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
public:
	afx_msg void OnDatabaseDisconnect();
	afx_msg void OnUpdateDatabaseDisconnect(CCmdUI *pCmdUI);
	afx_msg void OnDatabaseRegister();
	afx_msg void OnUpdateDatabaseRegister(CCmdUI *pCmdUI);
private:
	int m_iRardarID;
public:
	afx_msg void OnBnClickedAutoDispatch();
	int m_iAngularRate;
private:
	RadarParams m_params;
public:
	int m_pulseCount;
	double *m_ReturnPulseReal;
	double *m_ReturnPulseIm;
	afx_msg void OnDatabaseOptions();
	afx_msg void OnViewWave();
	afx_msg void OnUpdateViewWave(CCmdUI *pCmdUI);
	afx_msg void OnViewTrackrecorder();
	std::vector<Track> m_TrackList;
};
