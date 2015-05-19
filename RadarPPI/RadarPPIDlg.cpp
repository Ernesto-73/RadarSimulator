
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

// Global Variables.
int g_RadarState = RADAR_OFF;
char g_RecvBuf[BUF_SIZE];

void StrToPos(CString str, Pos *pos)
{
	CString cx = str.Left(str.Find(","));
	pos->x = _ttof(cx); 
	str = str.Right(str.Delete(0, str.Find(",") + 1));       
	CString cy = str.Left(str.Find(","));
	pos->y = _ttof(cy); 
	str = str.Right(str.Delete(0, str.Find(",") + 1));
	CString cz = str;
	pos->z = _ttof(cz);
	str = str.Right(str.Delete(0, str.Find(","))); 
}

void SplitTargetsData(CString str, std::vector<Pos*> &ps)
{
	while(!str.IsEmpty())
	{
		CString tmp = str.Left(str.Find(";"));
		str = str.Right(str.Delete(0, str.Find(";") + 1));
		Pos *pos = new Pos;
		StrToPos(tmp, pos);
		ps.push_back(pos);
	}
}

DWORD WINAPI RadarDataAccess(LPVOID lpParameter)
{
	ThreadData *thrData = (ThreadData *)lpParameter;
	HWND hwnd = thrData->hwnd;
	SOCKET sock = thrData->sock;

	listen(sock, 5);
	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);

	SOCKET sockConn = accept(sock, (SOCKADDR *)&addrClient, &len);

THR_SWTITCH:
	switch(g_RadarState)
	{
	case RADAR_ON:
		while(recv(sockConn, g_RecvBuf, BUF_SIZE, 0) > 0 && g_RadarState == RADAR_ON)
		{
			//TODO: PostMessaget or SendMessage？
			//::SendMessage(hwnd, WM_TARGET_UPDATE, NULL, NULL);
			::PostMessageA(hwnd, WM_TARGET_UPDATE, NULL, NULL);
		}
		goto THR_SWTITCH;
	case RADAR_OFF: 
		closesocket(sockConn);
		break;
	case RADAR_PAUSE:
		while(g_RadarState == RADAR_PAUSE);
		goto THR_SWTITCH;
	default:;
	}
	/*
	mFile.Flush();
	mFile.Close();
	*/
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
	, m_sOutput(_T(""))
	, m_strIPAddr(_T(""))
	, m_dwIP(0)
	, m_strPort(6000)
	, m_bUseThreads(FALSE)
	, m_iLocationX(0)
	, m_iLocationY(0)
	, m_distance(0)
	, m_clrSelected(0)
	, m_nElapse(100)
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
	DDX_IPAddress(pDX, IDC_IPADDRESS, m_dwIP);
	DDX_Text(pDX, IDC_PORT, m_strPort);
	DDV_MinMaxInt(pDX, m_strPort, 6000, 8000);
	DDX_Check(pDX, IDC_USE_THREADS, m_bUseThreads);
	DDX_Text(pDX, IDC_LOC_X, m_iLocationX);
	DDV_MinMaxDouble(pDX, m_iLocationX, 0., 500.);
	DDX_Text(pDX, IDC_LOC_Y, m_iLocationY);
	DDV_MinMaxDouble(pDX, m_iLocationY, 0., 500.);
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
	ON_WM_RBUTTONDOWN()
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
	
	// Get host ip address.
	hostent *host;
	char hostName[100];
	gethostname(hostName, 100);
	host = gethostbyname(hostName); 
	m_strIPAddr = inet_ntoa(*(struct in_addr *)host->h_addr_list[0]);
	m_dwIP = ntohl( inet_addr(m_strIPAddr));
	m_clrs.push_back(RGB(200, 100, 0));
	m_clrs.push_back(RGB(0, 200, 0));
	m_clrs.push_back(RGB(0, 0, 200));
	m_clrs.push_back(RGB(200, 0, 0));
/*
	HANDLE bkgThread;
	bkgThread = ::CreateThread(NULL, 0, RadarDataAccess, (void *)(this->m_hWnd), 0, NULL);
*/
	m_bUseThreads = TRUE;

	m_iLocationX = 300.90;
	m_iLocationY = 200.97;

	UpdateData(FALSE);
	
	this->MoveWindow(0,0, 800, 700);
	CenterWindow();
	AddToOutput("Welcome to use radar simulator v1.0 beta.");
	AddToOutput("Tip: You can click right button on the PPI scope to change color.");

	// 改变对话框焦点
	m_btnStart.SetFocus();
	return FALSE;  // return TRUE  unless you set the focus to a control
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
	int m_bClockwise = 1;
	double m_dVelocity = 0.314 * 0.75; // real antenna velocity =  m_dVelocity / m_nElapse
	if(m_bClockwise)
	{
		m_th += m_dVelocity;	//0.1s
		if(m_th > 6.28) 
			m_th -= 6.28;
	}
	else{
		m_th -= m_dVelocity;	
		if(m_th < 0) 
			m_th += 6.28;
	}
	//Invalidate();
	
	InvalidateRect(&m_canvas);
	CDialogEx::OnTimer(nIDEvent);
}

void CRadarPPIDlg::Draw(CDC * pDC)
{
	// Re-draw background.
	CRect r;
	GetClientRect(&r);
	pDC->FillSolidRect(&r,RGB(0,0,0));

	// Initialize color && pen && brush.
	COLORREF clr = m_clrs[m_clrSelected];
	CPen xPen(PS_SOLID, 1, clr);
	CPen *oPen = pDC->SelectObject(&xPen);
	CBrush *brush = CBrush::FromHandle( (HBRUSH)GetStockObject(NULL_BRUSH) );
	CBrush *oBrush = pDC->SelectObject(brush);

	if(g_RadarState == RADAR_ON)
	{
		// Draw scan sector(fan-shaped)
		double ArcScale = 1.024;
		for(double dt = 0.0;dt < ArcScale;dt += 0.004)
		{
			// use variable "dt" as bias.
			double  gc = 200. * dt;
			double  x = 250. + 200. * cos(m_th + dt);
			double  y = 250. + 200. * sin(m_th + dt);

			// decide which color to use.
			xPen.DeleteObject();
			switch(m_clrSelected)
			{
			case 0:	xPen.CreatePen(0, 4, RGB(gc, gc/2, 0)); break;
			case 1:	xPen.CreatePen(0, 4, RGB(0, gc, 0)); break;
			case 2:	xPen.CreatePen(0, 4, RGB(0, 0, gc)); break;
			case 3:	xPen.CreatePen(0, 4, RGB(gc, 0, 0)); break;
			default:;
			}
			pDC->SelectObject(&xPen);
			pDC->MoveTo(250, 250);
			pDC->LineTo((int)x, (int)y);
		}

		// Draw targets(detected).
		int radius = 5; // the radius of the target.
		for(int i = 0;i < (int)m_theta.size();i++)
		{
			if(m_distance[i] >= 100)
				continue;
			
			xPen.DeleteObject();	

			// Make sure the target is in the scoop, otherwise target location won't be updated.
			if(m_theta[i] > m_th && m_theta[i] < m_th + ArcScale)
			{
				CBrush brush(RGB(255, 0, 0));
				pDC->SelectObject(&brush);
				xPen.CreatePen(0, 1, RGB(255, 0, 0));	
				pDC->SelectObject(&xPen);

				pDC->Ellipse((int)m_targetx[i] - radius, 
							(int)m_targety[i] - radius, 
							(int)m_targetx[i] + radius, 
							(int)m_targety[i] + radius);
			
				CString str;
				str.Format("Target Location: (%.2lf, %.2lf)", m_targetx[i], m_targety[i]);
				AddToOutput(str);
			
				m_oldtargetx[i] = m_targetx[i];
				m_oldtargety[i] = m_targety[i];
			}
			else{
				CBrush brush(RGB(150, 0, 0));
				pDC->SelectObject(&brush);
				xPen.CreatePen(0, 1, RGB(150, 0, 0));	
				pDC->SelectObject(&xPen);
				if(m_oldtargetx[i] != -1)
					pDC->Ellipse((int)m_oldtargetx[i] - radius - 1, 
								(int)m_oldtargety[i] - radius - 1, 
								(int)m_oldtargetx[i] + radius - 1, 
								(int)m_oldtargety[i] + radius -1 );
			}
		}
	}

	// Draw circles.
	int mode = 1;
	xPen.DeleteObject();
	xPen.CreatePen(0, 1, clr);
	pDC->SelectObject(&xPen);

	if(mode == 0)
	{
		pDC->Arc(50,50,450,450,0,0,0,0);
		pDC->Arc(100,100,400,400,0,0,0,0);
		pDC->Arc(150,150,350,350,0,0,0,0);
		pDC->Arc(200,200,300,300,0,0,0,0);
		pDC->Arc(240,240,260,260,0,0,0,0);
	}
	else {
		pDC->Arc(50,50,450,450,0,0,0,0);
		pDC->Arc(80,80,420,420,0,0,0,0);
		pDC->Arc(110,110,390,390,0,0,0,0);
		pDC->Arc(140,140,360,360,0,0,0,0);
		pDC->Arc(170,170,330,330,0,0,0,0);
		pDC->Arc(200,200,300,300,0,0,0,0);
		pDC->Arc(230,230,270,270,0,0,0,0);
	}
	
	// Draw the cross line.
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

	// Print radar location and antenna orient.
	CFont font;
	font.CreatePointFont(90, "Verdana", NULL);
	CFont *oFont = pDC->SelectObject(&font);
	
	// Text position.
	int tx = 325;
	int ty = 5;

	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	pDC->SetTextColor(RGB(150, 150, 150));
	
	CString strInfo;
	strInfo.Format("Radar PPI Scope");
	pDC->TextOut(tx, ty, strInfo);
	
	ty += tm.tmHeight;
	strInfo.Format("Location(%.2lf, %.2lf)", m_iLocationX, m_iLocationY);
	pDC->TextOut(tx, ty, strInfo);

	// Antenna orient.
	tx = 10;
	ty = 490 - tm.tmHeight;
	double angle = m_th / 3.1415926 * 180;
	strInfo.Format("Antenna Orient: %.4lf", angle);
	pDC->TextOut(tx, ty, strInfo);

	// Restore DC objects.
	pDC->SelectObject(oPen);
	pDC->SelectObject(oBrush);
	pDC->SelectObject(oFont);
}


BOOL CRadarPPIDlg::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}


afx_msg LRESULT CRadarPPIDlg::OnTargetUpdate(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
/*
	CFile mFile(_T("e:\\user.txt "), CFile::modeWrite|CFile::modeNoTruncate);
	mFile.SeekToEnd();
*/
	CString tmp(g_RecvBuf);
	std::vector<Pos*> ps;
	SplitTargetsData(tmp, ps);
/*
	tmp += "\n";
	mFile.Write(tmp, tmp.GetLength());
	mFile.Flush();
	mFile.Close();
*/
	// clear buffer.
	m_targetx.clear();
	m_targety.clear();
	m_theta.clear();
	m_distance.clear();

	for(int i = 0;i < (int)ps.size();i++)
	{
		// Get the relative coordinates.
		double dx = ps[i]->x - m_iLocationX;
		double dy = ps[i]->y - m_iLocationY;

		// Calculate the polar coordinates.
		double theta = atan(dy / dx);
		if(dx > 0 )
		{
			if(dy > 0)					// 1st sector
				theta = theta;
			else
				theta += 6.28;			// 4th sector
		}
		else{
			if(dy > 0)				
				theta += 3.14;			// 2nd sector
			else
				theta = 3.14 + theta;	// 3rd sector
		}

		m_targetx.push_back(dx * 2. + 250.);
		m_targety.push_back(dy * 2. + 250.);
		m_theta.push_back(theta);
		m_distance.push_back(sqrt(dx * dx + dy * dy));
	}

	// TODO: this may cause core-dump.
	if(m_oldtargetx.size() != m_targetx.size())	
	{
		m_oldtargetx.resize(m_targetx.size(), -1);
		m_oldtargety.resize(m_targety.size(), -1);
	}
	return 0;
}


void CRadarPPIDlg::OnBnClickedStart()
{
	switch(g_RadarState)
	{
	case RADAR_OFF:
		UpdateData(TRUE);
		SetTimer(1, m_nElapse, NULL);
		g_RadarState = RADAR_ON;
		m_btnPause.EnableWindow(TRUE);
		m_btnStart.SetWindowTextA("Stop");
		PrepareSock();

		// Create background thread.
		ThreadData thrData;
		thrData.sock = m_sock;
		thrData.hwnd = m_hWnd;
		m_bkgThread = ::CreateThread(NULL, 0, RadarDataAccess, (LPVOID)&thrData, 0, NULL);
		AddToOutput("Radar State: On");
		break;

	case RADAR_ON:

	case RADAR_PAUSE:
		KillTimer(1);
		closesocket(m_sock);
		m_btnPause.EnableWindow(FALSE);
		m_btnStart.SetWindowTextA("Start");
		AddToOutput("Radar State: Off");

		// Reset the inner varibles;
		m_th = 0;
		m_theta.clear();
		m_distance.clear();
		m_targetx.clear();
		m_targety.clear();
		m_oldtargetx.clear();
		m_oldtargety.clear();
		g_RadarState = RADAR_OFF;
		InvalidateRect(&m_canvas);
		break;

	default:;
	}
}


void CRadarPPIDlg::OnBnClickedPause()
{
	// TODO: Add your control notification handler code here
	switch(g_RadarState)
	{
	case RADAR_OFF:
		break;

	case RADAR_ON:
		g_RadarState = RADAR_PAUSE;
		m_btnPause.SetWindowTextA("Continue");
		AddToOutput("Radar State: Pause");
		KillTimer(1);
		break;

	case RADAR_PAUSE:
		g_RadarState = RADAR_ON;
		m_btnPause.SetWindowTextA("Pause");
		AddToOutput("Radar State: On");
		SetTimer(1, 100, NULL);
		break;

	default:;
	}
}

/*
	Output information to the text area.
*/
void CRadarPPIDlg::AddToOutput(const char *str)
{
	CTime t = CTime::GetCurrentTime();
	m_sOutput += t.Format("[%H:%M:%S]: ") + CString(str) + "\r\n";
	UpdateData(FALSE);

	// Set the cursor to the last line.
	CEdit *edt = (CEdit *)GetDlgItem(IDC_OUTPUT);
	int li = edt->GetWindowTextLength();
	edt->SetSel(li, li, FALSE);
}


void CRadarPPIDlg::PrepareSock(void)
{
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN addrSrv;
//	addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons((u_short)m_strPort);
	bind(m_sock, (SOCKADDR *)&addrSrv, sizeof(SOCKADDR));
}


void CRadarPPIDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// Change PPI background color.
	CPoint ptTopleft = m_canvas.TopLeft();
	CPoint ptBottomRight = m_canvas.BottomRight();
	if(	point.x > ptTopleft.x && point.x < ptBottomRight.x &&
		point.y > ptTopleft.y && point.y < ptBottomRight.y )
	{
		m_clrSelected = (++m_clrSelected % 4);
		InvalidateRect(&m_canvas);
	}
	CDialogEx::OnRButtonDown(nFlags, point);
}

// Exit
void CRadarPPIDlg::OnBnClickedCancel()
{
	if(IDYES == MessageBox("Are you sure to quit?", "Question",MB_YESNO|MB_ICONQUESTION))
	{
		if(g_RadarState != RADAR_OFF)
			SendMessage(WM_COMMAND, MAKEWPARAM(ID_START, BN_CLICKED), NULL);
		CDialogEx::OnCancel();
	}
	else {
		return ;
	}
}
