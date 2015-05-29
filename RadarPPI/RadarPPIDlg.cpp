
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
			//TODO: PostMessaget or SendMessage£¿
			//::SendMessage(hwnd, WM_TARGET_UPDATE, NULL, NULL);
			::PostMessageA(hwnd, WM_TARGET_UPDATE, NULL, NULL);
		}
		goto THR_SWTITCH;
	case RADAR_OFF: 
		closesocket(sockConn);
		break;
	case RADAR_PAUSE:
		goto THR_SWTITCH;
	default:;
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
	, m_sOutput(_T(""))
	, m_strIPAddr(_T(""))
	, m_dwIP(0)
	, m_port(7000)
	, m_bUseThreads(FALSE)
	, m_LocationX(0)
	, m_LocationY(0)
	, m_distance(0)
	, m_clrSelected(1)
	, m_nElapse(100)
	, m_nBufferSize(0)
	, m_bIsSmallWindow(false)
	, m_env(NULL)
	, m_conn(NULL)
	, m_iRardarID(0)
{
	this->m_canvas = CRect(10, 10, 510, 510);
	m_large = CRect(0,0, 800, 725);
	m_small = CRect(0, 0, 518, 560);
	m_font.CreatePointFont(80, "Verdana", NULL);
	m_hIcon = AfxGetApp()->LoadIcon(IDR_RADAR);

	memset(m_arrOptions, 0, sizeof(int) * NUM);
}

void CRadarPPIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, ID_PAUSE, m_btnPause);
	DDX_Control(pDX, ID_START, m_btnStart);
	DDX_Text(pDX, IDC_OUTPUT, m_sOutput);
	DDX_IPAddress(pDX, IDC_IPADDRESS, m_dwIP);
	DDX_Text(pDX, IDC_PORT, m_port);
	DDV_MinMaxInt(pDX, m_port, 6000, 8000);
	DDX_Check(pDX, IDC_USE_THREADS, m_bUseThreads);
	DDX_Text(pDX, IDC_LOC_X, m_LocationX);
	DDV_MinMaxDouble(pDX, m_LocationX, 0., 50000.);
	DDX_Text(pDX, IDC_LOC_Y, m_LocationY);
	DDV_MinMaxDouble(pDX, m_LocationY, 0., 50000.);
	DDX_Control(pDX, IDC_SCROLLBAR_PW, m_scrollBarPW);
	DDX_Radio(pDX, IDC_RADIO_BUFFER_1, m_nBufferSize);
	DDX_Control(pDX, IDC_SCROLLBAR_THRESH, m_scrollBarThresh);
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
	ON_WM_HSCROLL()
	ON_WM_INITMENUPOPUP()
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_HELP_ABOUT, &CRadarPPIDlg::OnHelpAbout)
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_DATABASE_CONNECT, &CRadarPPIDlg::OnDatabaseConnect)
	ON_UPDATE_COMMAND_UI(ID_DATABASE_CONNECT, &CRadarPPIDlg::OnUpdateDatabaseConnect)
	ON_COMMAND(ID_DATABASE_DISCONNECT, &CRadarPPIDlg::OnDatabaseDisconnect)
	ON_UPDATE_COMMAND_UI(ID_DATABASE_DISCONNECT, &CRadarPPIDlg::OnUpdateDatabaseDisconnect)
	ON_COMMAND(ID_DATABASE_REGISTER, &CRadarPPIDlg::OnDatabaseRegister)
	ON_UPDATE_COMMAND_UI(ID_DATABASE_REGISTER, &CRadarPPIDlg::OnUpdateDatabaseRegister)
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
	m_nElapse = 100;
	//Set up main menu.
	m_menu.LoadMenuA(IDR_MAINMENU);
	SetMenu(&m_menu);

	// Set up status bar.
	m_StatusBar.CreateEx(this,SBT_TOOLTIPS,WS_CHILD | WS_VISIBLE | CBRS_BOTTOM,AFX_IDW_STATUS_BAR );
	m_StatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST,AFX_IDW_CONTROLBAR_LAST,ID_INDICATOR_CAPS);

	m_StatusBar.SetPaneText(0, _T("Ready"));
	CTime t = CTime::GetCurrentTime();
	CString time = t.Format("  %H:%M:%S");
	m_StatusBar.SetPaneText(1, time);
	SetTimer(2, 1000, NULL);

	// Get host ip address.
	hostent *host;
	char hostName[100];
	gethostname(hostName, 100);
	host = gethostbyname(hostName); 
	m_strIPAddr = inet_ntoa(*(struct in_addr *)host->h_addr_list[0]);
	m_dwIP = ntohl( inet_addr(m_strIPAddr));

	// Initial colors list.
	m_clrs.push_back(RGB(200, 100, 0));
	m_clrs.push_back(RGB(0, 200, 0));
	m_clrs.push_back(RGB(0, 0, 200));
	m_clrs.push_back(RGB(200, 0, 0));

	// Whether yo use multi-threads.
	m_bUseThreads = TRUE;
	m_nBufferSize = 2;

	UpdateData(FALSE);

	// Initialize PRI combo-box.
	for(int i = 1;i < 20;i++)
	{
		CString str;
		float pri = i * 0.2f;
		str.Format("%.2f", pri);
		((CComboBox *)GetDlgItem(IDC_PRI))->AddString(str);
	}
	((CComboBox *)GetDlgItem(IDC_PRI))->SelectString(0, "0.80");
	
	// Initialize Stagger combo-box.
	for(int i = 0;i < 6;i++)
	{
		CString str;
		str.Format("%d", i);
		((CComboBox *)GetDlgItem(IDC_STAGGER))->AddString(str);
	}
	((CComboBox *)GetDlgItem(IDC_STAGGER))->SelectString(0, _T("1"));

	// Initialize PW scroll-bar.
	SCROLLINFO si;  
	si.cbSize = sizeof(SCROLLINFO);;  
	si.nMin = 2;  
	si.nPos = 8;
	si.nMax = 21;  
	si.nPage = 2;
	si.fMask = SIF_POS|SIF_RANGE|SIF_PAGE;  
	m_scrollBarPW.SetScrollInfo(&si, TRUE);  
	CString str;
	str.Format("PW(Pulse Width) = %d%% of the PRI", si.nPos);
	SetDlgItemText(IDC_PW, str);

	si.cbSize = sizeof(SCROLLINFO);;  
	si.nMin = 1;  
	si.nMax = 100;  
	si.nPos = 17;
	si.nPage = 2;
	si.fMask = SIF_POS|SIF_RANGE|SIF_PAGE;  
	m_scrollBarThresh.SetScrollInfo(&si, TRUE); 
	str.Format("Absolute Th = %d e-14", si.nPos);
	SetDlgItemText(IDC_THRESH, str);

	// Initialize Amplitude combo-box.
	for(int i = 5;i < 14;i++)
	{
		CString str;
		str.Format("%d", i);
		((CComboBox *)GetDlgItem(IDC_TRANS_AMP))->AddString(str);
	}
	((CComboBox *)GetDlgItem(IDC_TRANS_AMP))->SelectString(0, _T("10"));

	// Initialize Antenna Velocity Combo-box.
	CString strV[7] = {"pi/12", "pi/6", "pi/4", "pi/3", "pi/2", "pi", "pi*3/2"};
	for(int i = 0;i < 7;i++)
	{
		((CComboBox *)GetDlgItem(IDC_ANTENNA_VELOCITY))->AddString(strV[i]);
	}
	((CComboBox *)GetDlgItem(IDC_ANTENNA_VELOCITY))->SetCurSel(5);

	// Initialize Update Rate Velocity Combo-box.
	for(int i = 1;i < 5;i++)
	{
		CString str;
		str.Format("%d", i * 50);
		((CComboBox *)GetDlgItem(IDC_UPDATE_RATE))->AddString(str);
	}
	((CComboBox *)GetDlgItem(IDC_UPDATE_RATE))->SetCurSel(1);

	// Initialize Radar Bandwidth Combo-box.
	for(int i = 1;i <= 20;i++)
	{
		CString str;
		str.Format("%.2f", i * 0.02);
		((CComboBox *)GetDlgItem(IDC_RADAR_BW))->AddString(str);
	}
	((CComboBox *)GetDlgItem(IDC_RADAR_BW))->SetCurSel(1);

	// Initialize Sampling Rate Combo-box.
	for(int i = 1;i <= 20;i++)
	{
		CString str;
		str.Format("%d", i * 1000);
		((CComboBox *)GetDlgItem(IDC_SAMPLING_RATE))->AddString(str);
	}
	((CComboBox *)GetDlgItem(IDC_SAMPLING_RATE))->SetCurSel(0);

	// Initialize Antenna State Combo-box.
	((CComboBox *)GetDlgItem(IDC_ANTENNA_STATE))->AddString(_T("Antenna Connected"));
	((CComboBox *)GetDlgItem(IDC_ANTENNA_STATE))->AddString(_T("Antenna Disonnected"));
	((CComboBox *)GetDlgItem(IDC_ANTENNA_STATE))->SetCurSel(0);

	// Initialize Check-boxes.
	((CButton *)GetDlgItem(IDC_USE_MTI))->SetCheck(0);
	((CButton *)GetDlgItem(IDC_USE_MATCH_FILTER))->SetCheck(1);
	((CButton *)GetDlgItem(IDC_CFAR))->SetCheck(1);
	((CButton *)GetDlgItem(IDC_PESISTENT_DISPLAY))->SetCheck(1);

	((CButton *)GetDlgItem(IDC_USE_MTI))->SetFont(&m_font);
	((CButton *)GetDlgItem(IDC_USE_MATCH_FILTER))->SetFont(&m_font);
	((CButton *)GetDlgItem(IDC_CFAR))->SetFont(&m_font);
	((CButton *)GetDlgItem(IDC_PESISTENT_DISPLAY))->SetFont(&m_font);
	((CButton *)GetDlgItem(IDC_USE_THREADS))->SetFont(&m_font);
	((CButton *)GetDlgItem(IDC_USE_DATABASE))->SetFont(&m_font);

	GetDlgItem(IDC_STATIC_1)->SetFont(&m_font);
	GetDlgItem(IDC_STATIC_2)->SetFont(&m_font);
	
	((CButton *)GetDlgItem(IDC_RADIO_BUFFER_1))->SetFont(&m_font);
	((CButton *)GetDlgItem(IDC_RADIO_BUFFER_2))->SetFont(&m_font);
	((CButton *)GetDlgItem(IDC_RADIO_BUFFER_3))->SetFont(&m_font);
	((CButton *)GetDlgItem(IDC_RADIO_BUFFER_4))->SetFont(&m_font);
	GetDlgItem(IDC_IPADDRESS)->SetFont(&m_font);

	MoveWindow(&m_large);
//	MoveWindow(&m_small);
	CenterWindow();

	// Print welcome information.
	AddToOutput("Welcome to use radar simulator v1.0 beta.");
	AddToOutput("Tip: You can click right button on the PPI scope to change color.");

	// Change the focus of dialog.
	m_btnStart.SetFocus();
	GetDlgItem(IDC_STATIC)->SetFont(&m_font);
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
	if(nIDEvent == 2)
	{
		CTime t = CTime::GetCurrentTime();
		CString time = t.Format("  %H:%M:%S");
		m_StatusBar.SetPaneText(1, time);
	}
	else{
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
		InvalidateRect(&m_canvas);
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CRadarPPIDlg::Draw(CDC * pDC)
{
	// Initialize some variables.
	int GreyScale = 60;
	int ScanFaceDepth = 100;
	int LineWidth = 10;

	// Re-draw background.
	CRect r;
	GetClientRect(&r);
	pDC->FillSolidRect(&r,RGB(GreyScale, GreyScale, GreyScale));

	// Initialize color && pen && brush.
	COLORREF clr = m_clrs[m_clrSelected];
	CPen xPen(PS_SOLID, 1, clr);
	CPen *oPen = pDC->SelectObject(&xPen);
	CBrush *brush = CBrush::FromHandle( (HBRUSH)GetStockObject(NULL_BRUSH) );
	CBrush *oBrush = pDC->SelectObject(brush);

	// Border
	pDC->MoveTo(499, 0);
	pDC->LineTo(499, 499);

	pDC->MoveTo(0, 499);
	pDC->LineTo(499, 499);

	pDC->MoveTo(0, 0);
	pDC->LineTo(0, 499);

	pDC->MoveTo(0, 0);
	pDC->LineTo(499, 0);


	if(g_RadarState == RADAR_ON)
	{
		// Draw scan sector(fan-shaped)
		double ArcScale = 1.024, x, y;
		for(double dt = 0.0;dt <= ArcScale;dt += 0.002)
		{
			// use variable "dt" as bias.
			double  gc = ScanFaceDepth * dt + GreyScale;
			x = 250. + 210. * cos(m_th + dt);
			y = 250. + 210. * sin(m_th + dt);
			if(gc > 255)
				gc = 255;
			// decide which color to use.
			xPen.DeleteObject();

			switch(m_clrSelected)
			{
			case 0:	xPen.CreatePen(PS_SOLID, LineWidth, RGB(gc, gc/2 + GreyScale / 2, GreyScale)); break;
			case 1:	xPen.CreatePen(PS_SOLID, LineWidth, RGB(GreyScale, gc, GreyScale)); break;
			case 2:	xPen.CreatePen(PS_SOLID, LineWidth, RGB(GreyScale, GreyScale, gc)); break;
			case 3:	xPen.CreatePen(PS_SOLID, LineWidth, RGB(gc, GreyScale, GreyScale)); break;
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
			/*
				CString str;
				str.Format("Target Location: (%.2lf, %.2lf)", m_targetx[i], m_targety[i]);
				AddToOutput(str);
			*/
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

	int mode = 0;
	xPen.DeleteObject();
	xPen.CreatePen(0, 1, clr);
	pDC->SelectObject(&xPen);

	if(mode == 1)
	{
		pDC->Arc(50,50,450,450,0,0,0,0);
		pDC->Arc(100,100,400,400,0,0,0,0);
		pDC->Arc(150,150,350,350,0,0,0,0);
		pDC->Arc(200,200,300,300,0,0,0,0);
	//	pDC->Arc(240,240,260,260,0,0,0,0);
	}
	else {
		pDC->Arc(50,50,450,450,0,0,0,0);
		pDC->Arc(80,80,420,420,0,0,0,0);
		pDC->Arc(110,110,390,390,0,0,0,0);
		pDC->Arc(140,140,360,360,0,0,0,0);
		pDC->Arc(170,170,330,330,0,0,0,0);
		pDC->Arc(200,200,300,300,0,0,0,0);
	//	pDC->Arc(230,230,270,270,0,0,0,0);
	}
	
	// Draw the cross line.
	CFont font;
	font.CreatePointFont(85, "Monaco", NULL);
	CFont *oFont = pDC->SelectObject(&font);

	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	pDC->SetTextColor(clr);

	LOGBRUSH logBrush;
	logBrush.lbStyle = BS_SOLID;
	logBrush.lbColor = clr;
	CPen pen;
	pen.CreatePen(PS_SOLID|PS_GEOMETRIC|PS_ENDCAP_ROUND, 1, &logBrush);
	pDC->SelectObject(&pen);

	pDC->SetBkMode(TRANSPARENT);

	pDC->MoveTo(50,250);
	pDC->LineTo(450,250);
	pDC->TextOutA(13, 250, "180.0");
	pDC->TextOutA(455, 250, "0.0");

	pDC->MoveTo(250,50);
	pDC->LineTo(250,450);
	pDC->TextOutA(250, 35, "90.0");
	pDC->TextOutA(250, 450, "270.0");

	pDC->MoveTo(109,109);
	pDC->LineTo(391,391);
	pDC->TextOutA(80, 92, "135.0");
	pDC->TextOutA(391, 391, "315.0");

	pDC->MoveTo(109, 391);
	pDC->LineTo(391,109);
	pDC->TextOutA(80, 395, "225.0");
	pDC->TextOutA(395, 98, "45.0");

	pDC->MoveTo(69,169);
	pDC->LineTo(431,331);
	pDC->TextOutA(30, 160, "157.5");
	pDC->TextOutA(431, 331, "337.5");

	pDC->MoveTo(69,331);
	pDC->LineTo(431,169);
	pDC->TextOutA(30,331, "202.5");
	pDC->TextOutA(435,160, "22.5");

	pDC->MoveTo(169,69);
	pDC->LineTo(331,431);
	pDC->TextOutA(150, 50, "112.5");
	pDC->TextOutA(331, 436, "292.5");

	pDC->MoveTo(169,431);
	pDC->LineTo(331,69);
	pDC->TextOutA(150, 436, "247.5");
	pDC->TextOutA(331, 55, "67.5");

	// Print radar location and antenna orient.
	pDC->SetTextColor(RGB(220, 220, 220));
	pDC->SelectObject(&m_font);

	// Text position.
	int tx = 340;
	int ty = 5;
	
	CString strInfo;
	strInfo.Format("JY-27 Radar-PPI Scope");
	pDC->TextOut(tx, ty, strInfo);
	
	ty += tm.tmHeight;
	strInfo.Format("Location: (%.2lf, %.2lf)", m_LocationX, m_LocationY);
	pDC->TextOut(tx, ty, strInfo);

	// Antenna orient.
	tx = 10;
	ty = 500 - tm.tmHeight;
	double angle = m_th / 3.1415926 * 180;
	strInfo.Format("Antenna Orient: %.4lf", angle);
	pDC->TextOut(tx, ty, strInfo);
	
	strInfo.Format("Radar ID: #%d", m_iRardarID);
	tx = 400;
	pDC->TextOut(tx, ty, strInfo);

	// Scale
	for(int i = 0;i < 40;i++)
	{
		xPen.DeleteObject();
		xPen.CreatePen(PS_SOLID, 4, RGB(200 - i * 4, 0, 0));
		pDC->SelectObject(&xPen);
		pDC->MoveTo(15, 100 - i * 2);
		pDC->LineTo(35, 100-  i * 2);
	}
	xPen.DeleteObject();
	xPen.CreatePen(PS_SOLID, 5, RGB(20, 0, 0));
	pDC->SelectObject(&xPen);
	pDC->MoveTo(15, 20);
	pDC->LineTo(35, 20);

	pDC->MoveTo(15, 20);
	pDC->LineTo(15, 100);

	pDC->MoveTo(35, 20);
	pDC->LineTo(35, 100);

	pDC->MoveTo(15, 100);
	pDC->LineTo(35, 100);

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
		double dx = ps[i]->x - m_LocationX;
		double dy = ps[i]->y - m_LocationY;

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

	// Warning: this may cause core-dump.
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
		SetTimer(1, m_nElapse, NULL);
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
	m_sOutput += t.Format("[%H:%M:%S] ") + CString(str) + "\r\n";
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
	addrSrv.sin_port = htons((u_short)m_port);
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


void CRadarPPIDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	pScrollBar->GetScrollInfo(&si, SIF_POS|SIF_PAGE|SIF_RANGE);
	int nVscroll = si.nPos;
	switch(nSBCode)
	{
	case SB_LINEDOWN:  
		nVscroll += 1; 
		if (nVscroll >= si.nMax)
			nVscroll = si.nMax - 1;
		break;

	case SB_LINEUP:    
		nVscroll -= 1;
		if (nVscroll < si.nMin)
			nVscroll = si.nMin;
		break;

	case SB_PAGEDOWN: 
		nVscroll += si.nPage; 
		if (nVscroll >= si.nMax)
			nVscroll = si.nMax - 1;
		break;

	case SB_PAGEUP:  
		nVscroll -= si.nPage;
		if (nVscroll < si.nMin)
			nVscroll = si.nMin;
		break;

	case SB_THUMBTRACK: 
		nVscroll = nPos - (nPos % 2); 
		break;

	default:;
	}

	si.fMask = SIF_POS;
	si.nPos = nVscroll;
	if(pScrollBar == &m_scrollBarPW)
	{
		CString str;
		str.Format("PW(Pulse Width) = %d%% of the PRI", si.nPos);
		SetDlgItemText(IDC_PW, str);
	}
	else{
		CString str;
		si.nPos++;
		str.Format("Absolute Th = %d e-14", si.nPos);
		SetDlgItemText(IDC_THRESH, str);
	}
	pScrollBar->SetScrollInfo(&si, TRUE);
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


HBRUSH CRadarPPIDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	pDC->SelectObject(&m_font);
	// TODO:  Change any attributes of the DC here

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}


void CRadarPPIDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint ptTopleft = m_canvas.TopLeft();
	CPoint ptBottomRight = m_canvas.BottomRight();
	if(	point.x > ptTopleft.x && point.x < ptBottomRight.x &&
		point.y > ptTopleft.y && point.y < ptBottomRight.y )
	{
		if(!m_bIsSmallWindow)
			MoveWindow(&m_small);
		else
			MoveWindow(&m_large);

		m_bIsSmallWindow = !m_bIsSmallWindow;
	}
	CDialogEx::OnLButtonDblClk(nFlags, point);
}


void CRadarPPIDlg::OnHelpAbout()
{
	// TODO: Add your command handler code here
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}


void CRadarPPIDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint ptTopleft = m_canvas.TopLeft();
	CPoint ptBottomRight = m_canvas.BottomRight();
	if(	point.x > ptTopleft.x && point.x < ptBottomRight.x &&
		point.y > ptTopleft.y && point.y < ptBottomRight.y )
	{
		
	}
	else{
		
	}
	CDialogEx::OnMouseMove(nFlags, point);
}


void CRadarPPIDlg::OnDatabaseConnect()
{
	try
	{
		m_env = Environment::createEnvironment(Environment::THREADED_MUTEXED);
	}catch(SQLException &e)
	{
		MessageBox(e.what());
	}

	std::string name = "scott";
	std::string pass = "zh2348";
	std::string srvName = "10.106.3.128:1521/ORCL";

	try
	{
		m_conn = m_env->createConnection(name, pass, srvName);
		MessageBox(_T("Dadatabse connected!"));
		m_StatusBar.SetPaneText(2, "   Connected");
		m_arrOptions[DATABASE_CONNECTED] = 1;
	}
	catch(SQLException &e)
	{
		CString str;
		str.Format("%s, Failed to connect database",e.what());
		MessageBox(str);
		return ;
	}
}

void CRadarPPIDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT /*nIndex*/, BOOL /*bSysMenu*/)
{
	ENSURE_VALID(pPopupMenu);

	// check the enabled state of various menu items
	CCmdUI state;
	state.m_pMenu = pPopupMenu;
	ASSERT(state.m_pOther == NULL);
	ASSERT(state.m_pParentMenu == NULL);

	// determine if menu is popup in top-level menu and set m_pOther to
	//  it if so (m_pParentMenu == NULL indicates that it is secondary popup)
	HMENU hParentMenu;
	if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
		state.m_pParentMenu = pPopupMenu;    // parent == child for tracking popup
	else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
	{
		CWnd* pParent = GetTopLevelParent();
		// child windows don't have menus -- need to go to the top!
		if (pParent != NULL &&
			(hParentMenu = pParent->GetMenu()->GetSafeHmenu()) != NULL)
		{
			int nIndexMax = ::GetMenuItemCount(hParentMenu);
			for (int nItemIndex = 0; nItemIndex < nIndexMax; nItemIndex++)
			{
				if (::GetSubMenu(hParentMenu, nItemIndex) == pPopupMenu->m_hMenu)
				{
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
			}
		}
	}

	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
		state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if (state.m_nID == 0)
			continue; // menu separator or invalid cmd - ignore it

		ASSERT(state.m_pOther == NULL);
		ASSERT(state.m_pMenu != NULL);
		if (state.m_nID == (UINT)-1)
		{
			// possibly a popup menu, route to first item of that popup
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
			if (state.m_pSubMenu == NULL ||
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				state.m_nID == (UINT)-1)
			{
				continue;       
			}
			state.DoUpdate(this, FALSE);    // pop-ups are never auto disabled
		}
		else
		{
			// normal menu item
			// Auto enable/disable if frame window has 'm_bAutoMenuEnable'
			//    set and command is _not_ a system command.
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, FALSE);
		}

		// adjust for menu deletions and additions
		UINT nCount = pPopupMenu->GetMenuItemCount();
		if (nCount < state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax - nCount);
			while (state.m_nIndex < nCount &&
				pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
			{
				state.m_nIndex++;
			}
		}
		state.m_nIndexMax = nCount;
	}
}

void CRadarPPIDlg::OnUpdateDatabaseConnect(CCmdUI *pCmdUI)
{
	if(m_arrOptions[DATABASE_CONNECTED])
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->Enable(TRUE);
}


void CRadarPPIDlg::OnDatabaseDisconnect()
{
	if(IDOK == MessageBox("Are you sure to disconnect from database?", "Disconnect", MB_OKCANCEL | MB_ICONQUESTION))
	{
		m_env->terminateConnection(m_conn);
		Environment::terminateEnvironment(m_env);
		m_env = NULL;
		m_conn = NULL;
		m_arrOptions[DATABASE_CONNECTED] = 0;
	}
}


void CRadarPPIDlg::OnUpdateDatabaseDisconnect(CCmdUI *pCmdUI)
{
	if(m_arrOptions[DATABASE_CONNECTED])
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}


void CRadarPPIDlg::OnDatabaseRegister()
{
	Statement *stmt = m_conn->createStatement();
	try{
		ResultSet *rs = stmt->executeQuery("select count(*) from user_tables where table_name = 'RADAR_LIST'");
		rs->next();
		if(!rs->getInt(1))
		{
			MessageBox("Table 'RADAR_LIST' doesn't exist.", "Error", MB_ICONERROR);
			return ;
		}
	}
	catch(SQLException &e)
	{
		MessageBox(e.what());	
		return ;
	}

	UpdateData(TRUE);
	CString ip;
	GetDlgItem(IDC_IPADDRESS)->GetWindowTextA(ip);
	CString query;
	query.Format("select * from RADAR_LIST WHERE IP = '%s' and PORT = %d", ip.GetBuffer(), m_port);
	ResultSet *rs = stmt->executeQuery(query.GetBuffer());
	if(rs->next())		// ip and port exist.
	{
		m_iRardarID = rs->getInt(1);
		m_LocationX = rs->getDouble(2);
		m_LocationY = rs->getDouble(3);
		UpdateData(FALSE);
		InvalidateRect(&m_canvas);
	}
	else{		// Auto dispatch.
		query.Format("select * from RADAR_LIST WHERE IP = '0.0.0.0' and PORT = 0");
		rs = stmt->executeQuery(query.GetBuffer());
		if(rs->next())		
		{
			m_iRardarID = rs->getInt(1);
			m_LocationX = rs->getDouble(2);
			m_LocationY = rs->getDouble(3);
			CString sql;
			sql.Format("update RADAR_LIST set IP = '%s', PORT = %d WHERE ID = %d", ip.GetBuffer(), m_port, m_iRardarID);
			stmt->executeUpdate(sql.GetBuffer());
			m_conn->commit();
			UpdateData(FALSE);
			InvalidateRect(&m_canvas);
		}
		else
		{
			MessageBox("There is no interface left.");
		}
	}
}


void CRadarPPIDlg::OnUpdateDatabaseRegister(CCmdUI *pCmdUI)
{
	if(m_arrOptions[DATABASE_CONNECTED])
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}
