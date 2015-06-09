// TrackRecorderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RadarPPI.h"
#include "TrackRecorderDlg.h"

#include "afxdialogex.h"


// CTrackRecorderDlg dialog

IMPLEMENT_DYNAMIC(CTrackRecorderDlg, CDialogEx)

CTrackRecorderDlg::CTrackRecorderDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTrackRecorderDlg::IDD, pParent)
	, m_iCount(0)
{

}

CTrackRecorderDlg::~CTrackRecorderDlg()
{
}

void CTrackRecorderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TRACK, m_list);
}


BEGIN_MESSAGE_MAP(CTrackRecorderDlg, CDialogEx)
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDCANCEL, &CTrackRecorderDlg::OnBnClickedCancel)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_TRACK, &CTrackRecorderDlg::OnNMCustomdrawTrack)
END_MESSAGE_MAP()


// CTrackRecorderDlg message handlers


void CTrackRecorderDlg::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class
	delete this;
	CDialogEx::PostNcDestroy();
}


BOOL CTrackRecorderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	m_list.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT ); // list control style: report
	m_font.CreatePointFont(80, "Verdana", NULL);
	GetDlgItem(IDCANCEL)->SetFont(&m_font);
	m_list.SetFont(&m_font);
	// Add columns to list control.
	m_list.InsertColumn(0, "Time", 0, 80);
	m_list.InsertColumn(1, "Code", 0, 50);
	m_list.InsertColumn(2, "Coordinates", 0, 110);
	m_list.InsertColumn(3, "SNR", 0, 60);
	m_list.InsertColumn(4, "FAR", 0, 60);
	m_list.InsertColumn(5, "Distance", 0, 80);
	m_list.InsertColumn(6, "Theta", 0, 60);

	SetTimer(1, 500, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CTrackRecorderDlg::SetTrack(std::vector<Track> * _track)
{
	m_TrackList = _track;
}


void CTrackRecorderDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	UpdateList();
	CDialogEx::OnTimer(nIDEvent);
}


void CTrackRecorderDlg::UpdateList(void)
{
	int sz = 0;
	double ldistance = 0;
	double lcode = 0;
	double ltheta = 0;
	for(std::size_t i = m_iCount;i < m_TrackList->size();i++)
	{
		sz++;
		if(sz % 5 != 0)
			continue;
	
		int pos = m_list.GetItemCount();
		Track &track = m_TrackList->at(i);
		if(track.distance == ldistance && track.code == lcode && track.theta == ltheta)
			continue;

		ldistance = track.distance;
		lcode = track.code;
		ltheta = track.theta;
		CString str;
		int nRow = m_list.InsertItem(pos, track.time);
		str.Format("#%d",track.code);
		m_list.SetItemText(nRow, 1, str);
		str.Format("(%.2f, %.2f)",track.x, track.y);
		m_list.SetItemText(nRow, 2, str);
		str.Format("%e",track.snr);
		m_list.SetItemText(nRow, 3, str);
		str.Format("%.5f",track.f);
		m_list.SetItemText(nRow, 4, str);
		str.Format("%.5f",track.distance);
		m_list.SetItemText(nRow, 5, str);
		str.Format("%.5f",track.theta);
		m_list.SetItemText(nRow, 6, str);
	}
	m_iCount = m_TrackList->size();
	CString str;
	str.Format("Total: %d    Update Rate: 500 ms", m_iCount);
	GetDlgItem(IDC_TEXT)->SetWindowText(str);
}


HBRUSH CTrackRecorderDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	pDC->SelectObject(&m_font);
	// TODO:  Return a different brush if the default is not desired
	return hbr;
}


void CTrackRecorderDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	DestroyWindow();
}


void CTrackRecorderDlg::OnNMCustomdrawTrack(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );
	// TODO: Add your control notification handler code here
	if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage )
	{
		COLORREF clrText = RGB(50, 50, 50), clrTextBk = RGB(255, 255, 255);
		int nItem = static_cast<int> (pLVCD->nmcd.dwItemSpec);
		if(nItem % 2 == 0)
			clrTextBk = RGB(210, 210, 255);
		else
			clrTextBk = RGB(230, 230, 255);
		
		pLVCD->clrText = clrText;
		pLVCD->clrTextBk = clrTextBk;
		*pResult = CDRF_DODEFAULT;
	}
}
