// WavePanel.cpp : implementation file
//

#include "stdafx.h"
#include "RadarPPI.h"
#include "WavePanel.h"
#include "afxdialogex.h"
#include <cmath>

// WavePanel dialog

IMPLEMENT_DYNAMIC(WavePanel, CDialogEx)

WavePanel::WavePanel(CWnd* pParent /*=NULL*/)
	: CDialogEx(WavePanel::IDD, pParent)
	, m_real(NULL)
	, m_im(NULL)
	, m_size(0)
	, m_fEnergy(0)
	, m_iScale(0)
	, m_bAutoDNR(TRUE)
	, m_bInherent(TRUE)
{
	m_canvas = CRect(10, 10, 400, 210);
	m_Size = CRect(0, 0, 415, 500);
	m_font.CreatePointFont(80, "Verdana", NULL);
}

WavePanel::~WavePanel()
{
}

void WavePanel::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_AUTO_DNR, m_bAutoDNR);
	DDX_Check(pDX, IDC_INHERENT, m_bInherent);
	DDX_Text(pDX, IDC_SCALE, m_iScale);
}


BEGIN_MESSAGE_MAP(WavePanel, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &WavePanel::OnBnClickedCancel)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// WavePanel message handlers


void WavePanel::PostNcDestroy()
{
	delete this;
	CDialogEx::PostNcDestroy();
}


void WavePanel::OnBnClickedCancel()
{
	DestroyWindow();
}


void WavePanel::OnPaint()
{
	CPaintDC dc(this);
	CDC xDC;
	dc.SetBkColor(RGB(100,50,0));
	CBitmap xBMP;
	xDC.CreateCompatibleDC(&dc);
	xBMP.CreateCompatibleBitmap(&dc,500,500);
	xDC.SelectObject(&xBMP);
	Draw(&xDC);

	CDC yDC;
	CBitmap yBMP;
	CRect r;
	GetClientRect(&r);
	yDC.CreateCompatibleDC(&dc);

	yBMP.CreateCompatibleBitmap(&dc,r.Width(),r.Height());
	yDC.SelectObject(&yBMP);

	yDC.FillSolidRect(r, GetSysColor(COLOR_3DFACE)); //Get the system color of dialog background

	yDC.BitBlt(m_canvas.left, m_canvas.top, m_canvas.Width(), m_canvas.Height(), &xDC, 0, 0, SRCCOPY);
	dc.BitBlt(0, 0,r.Width(), r.Height(), &yDC, 0, 0, SRCCOPY);

	xBMP.DeleteObject();
	xDC.DeleteDC();
	yBMP.DeleteObject();
	yDC.DeleteDC();
}

void WavePanel::Draw(CDC * pDC)
{
	CRect r;
	GetClientRect(&r);
	pDC->FillSolidRect(&r,RGB(0, 0, 0));
	CPen pen(PS_SOLID, 1, RGB(0, 255, 0));
	CPen *oPen = pDC->SelectObject(&pen);

	for(int k = 0;k < 32;k++)
	{
		for(int i = 0 ;i < 40;i++)
		{
			double val = 150 - 20 * (log10(m_abs[k * 40 + i])  + 25);

			if(i == 0)
			{
				pDC->MoveTo(i * 10, (int)val);
				continue;
			}

			pDC->LineTo(i * 10, (int)val);
			pDC->MoveTo(i * 10, (int)val);
		}	
	}

	pen.DeleteObject();
	pen.CreatePen(PS_DOT, 1, RGB(190, 190, 190));
	pDC->SelectObject(&pen);

	CFont font;
	font.CreateFont(14, 7, -100, 0, 10, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_ROMAN, "Verdana");	
	CFont *oFont = pDC->SelectObject(&font);
	pDC->SetTextColor(RGB(210, 210, 210));
	CString str;
	pDC->TextOutA(2, 181, "0");
	for(int i = 1;i < 10;i++)
	{
		str.Format("%d", i * 2);
		pDC->TextOutA(i * 40 + 2, 181, str);
		pDC->MoveTo(i * 40, 0);
		pDC->LineTo(i * 40, 210);
	}

	pDC->TextOutA(2, 0, "0");
	for(int i = 1;i < 6;i++)
	{
		str.Format("-%d", i * 5);
		pDC->TextOutA(0, i * 40 - 5, str);
		pDC->MoveTo(0, i * 40);
		pDC->LineTo(400, i * 40);
	}

	font.DeleteObject();
	font.CreatePointFont(90, "Verdana", NULL);
	pDC->SelectObject(&font);
	pDC->TextOutA(230, 5, "Volt(dB), Distance(5Km)");

	pDC->SelectObject(oFont);
	pDC->SelectObject(oPen);
}

BOOL WavePanel::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	GetDlgItem(IDC_AUTO_DNR)->SetFont(&m_font);
	GetDlgItem(IDC_CONFIG)->SetFont(&m_font);
	GetDlgItem(IDC_INHERENT)->SetFont(&m_font);
	GetDlgItem(IDC_SUMMARY)->SetFont(&m_font);
	GetDlgItem(IDCANCEL)->SetFont(&m_font);
	for(int i = 0;i < 6;i++)
	{
		CString str;
		str.Format("%d", i * 20);
		((CComboBox *)GetDlgItem(IDC_COMBO_RATE))->AddString(str);
	}
	((CComboBox *)GetDlgItem(IDC_COMBO_RATE))->SetCurSel(5);

	MoveWindow(&m_Size);
	CenterWindow();
	SetTimer(1, 120, NULL);
	UpdateData(FALSE);
	return TRUE;
}


void WavePanel::SetDataPointer(double * real, double * im, int size)
{
	m_real = real;
	m_im = im;
	m_size = size;
	m_abs.resize(m_size);
}


void WavePanel::CalculateAbs(void)
{
	CString str;
	double sum = 0;
	for(int i = 0;i < m_size;i++)
	{
		m_abs[i] = m_real[i] * m_real[i] + m_im[i] * m_im[i];
		sum += m_abs[i];
	}
	m_fEnergy = log10(sum) + 4;
	str.Format("%f", m_fEnergy);
	GetDlgItem(IDC_ENERGY)->SetWindowTextA(str);
}


void WavePanel::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	InvalidateRect(&m_canvas);
	CalculateAbs();	
	CDialogEx::OnTimer(nIDEvent);
}


BOOL WavePanel::OnEraseBkgnd(CDC* /*pDC*/)
{
	// TODO: Add your message handler code here and/or call default

	return FALSE;
}


HBRUSH WavePanel::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	pDC->SelectObject(&m_font);
	// TODO:  Return a different brush if the default is not desired
	return hbr;
}
