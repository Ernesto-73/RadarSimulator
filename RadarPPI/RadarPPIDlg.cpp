
// RadarPPIDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RadarPPI.h"
#include "RadarPPIDlg.h"
#include "afxdialogex.h"
#include <cmath>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DWORD WINAPI RadarDataAccess(LPVOID lpParameter)
{
//	::MessageBox((HWND)lpParameter, L"this is a test.", NULL, NULL);
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(1, 1);

	err = WSAStartup(wVersionRequested, &wsaData);
	if(err !=0 )
		return 0;

	if( LOBYTE(wsaData.wVersion) != 1 ||
		HIBYTE(wsaData.wVersion) != 1 )
	{
		WSACleanup();
		return 0;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(6000);

	bind(sock, (SOCKADDR *)&addrSrv, sizeof(SOCKADDR));
	listen(sock, 5);

	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);
	while(1)
	{
		SOCKET sockConn = accept(sock, (SOCKADDR *)&addrClient, &len);
		char sendBuf[100];
		sprintf_s(sendBuf, "Hello, %s. Welcome to javier.net.", inet_ntoa(addrClient.sin_addr));
		send(sockConn, sendBuf, strlen(sendBuf) + 1, 0);
		char recvBuf[100];

		while(recv(sockConn, recvBuf, 100, 0) > 0)
		{
		
			CString str(recvBuf);
		//	str.Format("[%s]: %s\n", inet_ntoa(addrClient.sin_addr), recvBuf);
		//	::MessageBox((HWND)lpParameter, str, NULL, NULL);
			Pos pos;
			CString cx = str.Left(str.Find(","));
			pos.x = _ttoi(cx); 
			str = str.Right(str.Delete(0, str.Find(",") + 1));       
			CString cy = str.Left(str.Find(","));
			pos.y = _ttoi(cy); 
			str = str.Right(str.Delete(0, str.Find(",") + 1));
			CString cz = str;
			pos.z = _ttoi(cz); 

			//str = str.Right(str.Delete(0, str.Find(","))); 
			//::MessageBox((HWND)lpParameter, cx + cy+ cz, NULL, NULL);
			::SendMessage((HWND)lpParameter, WM_TARGET_UPDATE, NULL, (LPARAM)&pos);
		}
		closesocket(sockConn);
	}
	return 0;
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRadarPPIDlg dialog
CRadarPPIDlg::CRadarPPIDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRadarPPIDlg::IDD, pParent)
	, m_th(0)
	, m_targetx(0)
	, m_targety(0)
	, m_oldtargetx(0)
	, m_oldtargety(0)
	, m_theta(0)
	, m_iRadarState(0)
	, m_sOutput(_T(""))
	, m_clrSelected(0)
{
	this->m_canvas = CRect(10, 10, 500, 500);
	m_hIcon = AfxGetApp()->LoadIcon(IDR_RADAR);
}

void CRadarPPIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, ID_PAUSE, m_btnPause);
	DDX_Control(pDX, ID_START, m_btnStart);
	DDX_Text(pDX, IDC_OUTPUT, m_sOutput);
}

BEGIN_MESSAGE_MAP(CRadarPPIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_TARGET_UPDATE, &CRadarPPIDlg::OnTargetUpdate)
	ON_BN_CLICKED(ID_START, &CRadarPPIDlg::OnBnClickedStart)
	ON_BN_CLICKED(ID_PAUSE, &CRadarPPIDlg::OnBnClickedPause)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDCANCEL, &CRadarPPIDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CRadarPPIDlg message handlers

BOOL CRadarPPIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_clrs.push_back(RGB(200, 100, 0));
	m_clrs.push_back(RGB(0, 200, 0));
	m_clrs.push_back(RGB(0, 0, 200));
	m_clrs.push_back(RGB(200, 0, 0));

	HANDLE bkgThread;
	bkgThread = ::CreateThread(NULL, 0, RadarDataAccess, (void *)(this->m_hWnd), 0, NULL);

	this->MoveWindow(0,0, 800, 700);
	CenterWindow();
	AddToOutput("Welcome to use radar simulator v1.0 beta.");
	AddToOutput("Tip: You can click right button on the PPI scope to change color.");
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRadarPPIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRadarPPIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);		
	}
	else
	{
		// Re-Paint the canvas
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRadarPPIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CRadarPPIDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	int m_bClockwise = 1;
	if(m_bClockwise)
	{
		m_th += 0.314;	
		if(m_th > 6.28) 
			m_th -= 6.28;
	}
	else{
		m_th -= 0.314;	
		if(m_th < 0) 
			m_th += 6.28;
	}
	//Invalidate();
	
	InvalidateRect(&m_canvas);
	CDialogEx::OnTimer(nIDEvent);
}

void CRadarPPIDlg::Draw(CDC * pDC)
{
/*
	CRect r;
	GetClientRect(&r);
	pDC->FillSolidRect(&r,RGB(10,10,10));
*/
	COLORREF clr = m_clrs[m_clrSelected];
	CPen xPen(1, 1, clr);
	CPen *oPen = pDC->SelectObject(&xPen);
	CBrush *brush = CBrush::FromHandle( (HBRUSH)GetStockObject(NULL_BRUSH) );
	CBrush *oBrush = pDC->SelectObject(brush);

	if(this->m_iRadarState == RADAR_ON)
	{
		double dt;
		for(dt = 0.0;dt < 1.024;dt += 0.004)
		{
			double  gc = 200 * dt;
			double  x = 250 + 200 * cos(dt + m_th);
			double  y = 250 + 200 * sin(dt + m_th);
			pDC->MoveTo(250,250);
			xPen.DeleteObject();
			switch(m_clrSelected)
			{
			case 0:
				xPen.CreatePen(0, 4, RGB(gc, gc/2, 0)); break;
			case 1:
				xPen.CreatePen(0, 4, RGB(0, gc, 0)); break;
			case 2:
				xPen.CreatePen(0, 4, RGB(0, 0, gc)); break;
			case 3:
				xPen.CreatePen(0, 4, RGB(gc, 0, 0)); break;
			default:;
			}
			
			pDC->SelectObject(&xPen);
			pDC->LineTo(static_cast<int>(x), static_cast<int>(y));
		}

		xPen.DeleteObject();	
		if(m_theta > m_th && m_theta < m_th + 1.024)
		{
			CBrush brush(RGB(200,0,0));
			pDC->SelectObject(&brush);
			xPen.CreatePen(0,1,RGB(200, 0, 0));	
			pDC->SelectObject(&xPen);
			pDC->Ellipse(this->m_targetx - 6, this->m_targety - 6,this->m_targetx + 6, this->m_targety + 6);
			m_oldtargetx = m_targetx;
			m_oldtargety = m_targety;
		}
		else{
			CBrush brush(RGB(100,0,0));
			pDC->SelectObject(&brush);
			xPen.CreatePen(0,1,RGB(100,0,0));	
			pDC->SelectObject(&xPen);
			pDC->Ellipse(this->m_oldtargetx - 5, this->m_oldtargety - 5,this->m_oldtargetx + 5, this->m_oldtargety + 5);
		}
		
	}
	xPen.DeleteObject();
	xPen.CreatePen(0,1,clr);
	pDC->SelectObject(&xPen);
/*
	pDC->Arc(50,50,450,450,0,0,0,0);
	pDC->Arc(100,100,400,400,0,0,0,0);
	pDC->Arc(150,150,350,350,0,0,0,0);
	pDC->Arc(200,200,300,300,0,0,0,0);
	pDC->Arc(240,240,260,260,0,0,0,0);
*/
	pDC->Arc(50,50,450,450,0,0,0,0);
	pDC->Arc(80,80,420,420,0,0,0,0);
	pDC->Arc(110,110,390,390,0,0,0,0);
	pDC->Arc(140,140,360,360,0,0,0,0);
	pDC->Arc(170,170,330,330,0,0,0,0);
	pDC->Arc(200,200,300,300,0,0,0,0);
	pDC->Arc(230,230,270,270,0,0,0,0);

	xPen.DeleteObject();
	xPen.CreatePen(PS_DASH,1,clr);
	pDC->SelectObject(&xPen);
	pDC->SetBkMode(TRANSPARENT);
	pDC->MoveTo(50,250);
	pDC->LineTo(450,250);

	pDC->MoveTo(250,50);
	pDC->LineTo(250,450);

	pDC->MoveTo(109,109);
	pDC->LineTo(391,391);

	pDC->MoveTo(109,391);
	pDC->LineTo(391,109);

	pDC->MoveTo(69,169);
	pDC->LineTo(431,331);

	pDC->MoveTo(69,331);
	pDC->LineTo(431,169);

	pDC->MoveTo(169,69);
	pDC->LineTo(331,431);

	pDC->MoveTo(169,431);
	pDC->LineTo(331,69);

	CFont font;
	font.CreatePointFont(90, "Courier New", NULL);
	CFont *oFont = pDC->SelectObject(&font);

	// Print radar location and antena orient.
	int tx = 350;
	int ty = 5;
	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	pDC->SetTextColor(RGB(150,150,150));
	pDC->TextOut(tx, ty, "Radar #1 PPI Scope");
	ty += tm.tmHeight;
	pDC->TextOut(tx, ty, "Location(267, 300)");

	double angle = m_th / 3.1415926 * 180;
	CString str;
	str.Format("Antena Orient: %.4lf", angle);
	pDC->TextOut(320, 490 - tm.tmHeight, str);

	pDC->SelectObject(oPen);
	pDC->SelectObject(oBrush);
	pDC->SelectObject(&oFont);
}


BOOL CRadarPPIDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	return TRUE;
	//return CDialogEx::OnEraseBkgnd(pDC);
}


afx_msg LRESULT CRadarPPIDlg::OnTargetUpdate(WPARAM wParam, LPARAM lParam)
{
//	AddToOutput("Target spot.");
	Pos *pos = (Pos *)lParam; 
	this->m_targetx = 250 + pos->x;
	this->m_targety = 250 + pos->y;
	this->m_theta = atan((double)pos->y / (double)pos->x );
	return 0;
}


void CRadarPPIDlg::OnBnClickedStart()
{
	// TODO: Add your control notification handler code here

	switch(this->m_iRadarState)
	{
	case RADAR_OFF:
		SetTimer(1,100,NULL);
		m_iRadarState = RADAR_ON;
		m_btnPause.EnableWindow(TRUE);
		m_btnStart.SetWindowTextA("Stop");
		AddToOutput("Radar State: On");
		break;
	case RADAR_ON:
	case RADAR_PAUSE:
		KillTimer(1);
		m_iRadarState = RADAR_OFF;
		m_btnPause.EnableWindow(FALSE);
		m_btnStart.SetWindowTextA("Start");
		AddToOutput("Radar State: Off");
		// Reset the inner varibles;
		m_th = 0;
		m_theta = 0;
		m_oldtargetx = 0;
		m_oldtargety = 0;

		InvalidateRect(&m_canvas);
		break;
	default:;
	}
}


void CRadarPPIDlg::OnBnClickedPause()
{
	// TODO: Add your control notification handler code here
	switch(this->m_iRadarState)
	{
	case RADAR_OFF:break;
	case RADAR_ON:
		m_iRadarState = RADAR_PAUSE;
		m_btnPause.SetWindowTextA("Continue");
		AddToOutput("Radar State: Pause");
		KillTimer(1);
		break;
	case RADAR_PAUSE:
		m_iRadarState = RADAR_ON;
		m_btnPause.SetWindowTextA("Pause");
		AddToOutput("Radar State: On");
		SetTimer(1,100,NULL);
		break;
	default:;
	}
}

/*
	输出到output文本域, 打印系统消息
*/
void CRadarPPIDlg::AddToOutput(const char *str)
{
	CTime t = CTime::GetCurrentTime();
	m_sOutput += t.Format("[%H:%M:%S]: ") + CString(str) + "\r\n";
	UpdateData(FALSE);
}

void CRadarPPIDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint ptTopleft = m_canvas.TopLeft();
	CPoint ptBottomRight = m_canvas.BottomRight();
	if(	point.x > ptTopleft.x && 
		point.x < ptBottomRight.x &&
		point.y > ptTopleft.y && 
		point.y < ptBottomRight.y 
		)
	{
		m_clrSelected = (++m_clrSelected % 4);
		InvalidateRect(&m_canvas);
	}
	CDialogEx::OnRButtonDown(nFlags, point);
}



void CRadarPPIDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	if(IDYES == MessageBox("Are you certain to exit?", "Warning",MB_YESNO|MB_ICONQUESTION))
		CDialogEx::OnCancel();
	else 
		return ;
}
