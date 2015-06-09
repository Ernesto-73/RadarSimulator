#pragma once
#include "afxcmn.h"
#include <vector>
#include "afxwin.h"

typedef struct Track{
	CString time;
	int code;
	double x;
	double y;
	double z;
	double distance;
	double theta;
	double snr;
	double f;
}Track;

// CTrackRecorderDlg dialog

class CTrackRecorderDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTrackRecorderDlg)

public:
	CTrackRecorderDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTrackRecorderDlg();

// Dialog Data
	enum { IDD = IDD_TRACK_RECORDER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual void PostNcDestroy();
public:
	virtual BOOL OnInitDialog();
	CListCtrl m_list;
	void SetTrack(std::vector<Track> * _track);
	std::vector<Track> *m_TrackList;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void UpdateList(void);
	std::size_t m_iCount;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CFont m_font;
	afx_msg void OnBnClickedCancel();
	afx_msg void OnNMCustomdrawTrack(NMHDR *pNMHDR, LRESULT *pResult);
};
