// NetTrafficDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NetTraffic.h"
#include "NetTrafficDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "Iphlpapi.h"
#include "Winsock2.h"
#include "MSTcpIP.h"

#include "Iprtrmib.h"

#define PROTOCOL_NO  11 

typedef struct _PROTN2T
{ 
	int  proto;
	char *pprototext;
}PROTN2T; 

//
// Possible TCP endpoint states
//
static char TcpState[][32] = {
	"???",
	"CLOSED",
	"LISTENING",
	"SYN_SENT",
	"SYN_RCVD",
	"ESTABLISHED",
	"FIN_WAIT1",
	"FIN_WAIT2",
	"CLOSE_WAIT",
	"CLOSING",
	"LAST_ACK",
	"TIME_WAIT",
	"DELETE_TCB"
};



PROTN2T aOfProto [PROTOCOL_NO + 1] = 
{  
	{IPPROTO_IP   , "IP"},
	{IPPROTO_ICMP , "ICMP"},  
	{IPPROTO_IGMP , "IGMP"}, 
	{IPPROTO_GGP  , "GGP"},  
	{IPPROTO_TCP  , "TCP"},  
	{IPPROTO_PUP  , "PUP"},  
	{IPPROTO_UDP  , "UDP"},  
	{IPPROTO_IDP  , "IDP"},  
	{IPPROTO_ND   , "NP" },  
	{IPPROTO_RAW  , "RAW"},  
	{IPPROTO_MAX  , "MAX"},
	{NULL , "" } 
} ;  

typedef struct _IPHEADER
{
	unsigned char  header_len:4;
	unsigned char  version:4;   
	unsigned char  tos;            // type of service
	unsigned short total_len;      // length of the packet
	unsigned short ident;          // unique identifier
	unsigned short flags;          
	unsigned char  ttl;            
	unsigned char  proto;          // protocol ( IP , TCP, UDP etc)
	unsigned short checksum;       
	unsigned int   sourceIP;
	unsigned int   destIP;
}IPHEADER;

typedef struct _TCPHEADER
{
	unsigned short	sourcePort;
	unsigned short	destPort;
	unsigned int	sequenceNum;
	unsigned int	aqunoledNum;
	unsigned char	dataOffset;
	unsigned char	flags;
	unsigned short	Window;
	unsigned short	checkSum;
	unsigned short	urgentPnt;
}TCPHEADER;

typedef struct _UDPHEADER
{
	unsigned short	sourcePort;
	unsigned short	destPort;
	unsigned short	udpLength;
	unsigned short	checkSum;
}UDPHEADER;

typedef struct _ICMPHEADER
{
	unsigned char	type;
	unsigned char	code;
	unsigned short	checkSum;
}ICMPHEADER;


typedef struct _ITEMDATA
{ 
	int  iLen;
	char *pData;
}ITEMDATA; 


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
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

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CNetTrafficDlg dialog



CNetTrafficDlg::CNetTrafficDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNetTrafficDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNetTrafficDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_cAddrCmb);
	DDX_Control(pDX, IDC_COMBO2, m_cProtocol);
	DDX_Control(pDX, IDC_LIST1, m_cPacketList);
	DDX_Control(pDX, IDC_EDIT1, m_cStatusEdit);
	DDX_Control(pDX, IDC_EDIT2, m_cAddr);
	DDX_Control(pDX, IDC_EDIT3, m_cMask);
	DDX_Control(pDX, IDC_EDIT4, m_cIdx);
	DDX_Control(pDX, IDC_EDIT5, m_cBCast);
	DDX_Control(pDX, IDC_EDIT6, m_cType);
	DDX_Control(pDX, IDC_EDIT7, m_cMAC);
	DDX_Control(pDX, IDC_EDIT8, m_cDesc);
	DDX_Control(pDX, IDC_EDIT9, m_cReceived);
	DDX_Control(pDX, IDC_EDIT12, m_cSent);
	DDX_Control(pDX, IDC_CHECK1, m_cHostCheck);
	DDX_Control(pDX, IDC_CHECK2, m_cLogCheck);
	DDX_Control(pDX, IDC_IPADDRESS1, m_cSource);
	DDX_Control(pDX, IDC_IPADDRESS2, m_cDest);
	DDX_Control(pDX, ID_START, m_cStart);
	DDX_Control(pDX, ID_STOP, m_cStop);
	DDX_Control(pDX, ID_CLEAR, m_cClear);
	DDX_Control(pDX, IDC_CHECK3, m_cHexCheck);
	DDX_Control(pDX, IDC_EDIT10, m_cLocalPort);
	DDX_Control(pDX, IDC_EDIT11, m_cSourcePort);
	DDX_Control(pDX, IDC_EDIT13, m_cDestPort);
	DDX_Control(pDX, IDC_COMBO3, m_cProcCombo);
	DDX_Control(pDX, IDC_CHECK4, m_cResolve);
}

BEGIN_MESSAGE_MAP(CNetTrafficDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_COMBO1, &CNetTrafficDlg::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(ID_START, &CNetTrafficDlg::OnBnClickedStart)
	ON_BN_CLICKED(ID_STOP, &CNetTrafficDlg::OnBnClickedStop)
	ON_BN_CLICKED(ID_CLEAR, &CNetTrafficDlg::OnBnClickedClear)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CNetTrafficDlg::OnLvnItemchangedList1)
	ON_BN_CLICKED(IDC_CHECK3, &CNetTrafficDlg::OnBnClickedCheck3)
	ON_BN_CLICKED(ID_STATE, &CNetTrafficDlg::OnBnClickedState)
END_MESSAGE_MAP()


// CNetTrafficDlg message handlers

BOOL CNetTrafficDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
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

	this->m_cPacketList.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
	USES_CONVERSION;
	CString strBuff = _T("SourceIP, Port, DestinationIP, Port, Length, Protocol, Flag, Process, TTL, ID");
	int iBuff[] = {90, 50, 90, 50, 50, 50, 50, 50, 50};

	wchar_t *next_token;
	wchar_t *token = wcstok_s(strBuff.GetBuffer(), _T(", "), &next_token);
	int iCnt = 0;
	while(token != NULL)
	{
		this->m_cPacketList.InsertColumn(iCnt, token, LVCFMT_LEFT, iBuff[iCnt]);
		token = wcstok_s(NULL, _T(", "), &next_token);
		iCnt++;
	}

	this->m_cProtocol.AddString(_T("All"));
	for(int i = 0; i < PROTOCOL_NO; i++)
	{
		this->m_cProtocol.AddString(A2W(aOfProto[i].pprototext));
	}
	this->m_cProtocol.SetCurSel(0);

	this->m_threadID = 0;

	this->m_lSent = 0;
	this->m_lReceive = 0;

	this->m_cHostCheck.SetCheck(true);

	int iNo = GetIPAddrTable(-1);
	GetIPAddrTable(iNo);

	GetProcesses();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CNetTrafficDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CNetTrafficDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

//
CString GetProc(LPVOID p, DWORD dSrcPort, DWORD dDesPort)
{
	CString strProc = _T("-");

	CNetTrafficDlg *pDlg = static_cast<CNetTrafficDlg *>(p);

	DWORD dPort = 0;
	for(int i = 0; i < pDlg->m_cProcCombo.GetCount(); i++)
	{
		dPort = (DWORD)pDlg->m_cProcCombo.GetItemData(i);
		if(dPort == dSrcPort || dPort == dDesPort)
		{
			pDlg->m_cProcCombo.GetLBText(i, strProc);
			break;
		}
	}

	return strProc;
}
// Get TCP Flag
CString GetFlag(unsigned char flags)
{
	CString strRet;

	if(flags & 1)
		strRet.Append(_T("finish "));
	if(flags & 2)
		strRet.Append(_T("synthesis "));
	if(flags & 4)
		strRet.Append(_T("reset "));
	if(flags & 8)
		strRet.Append(_T("push "));
	if(flags & 16)
		strRet.Append(_T("acknowledgement "));
	if(flags & 32)
		strRet.Append(_T("urgent "));
	if(flags & 64)
		strRet.Append(_T("reserved "));
	if(flags & 128)
		strRet.Append(_T("reserved "));

	return strRet;
}
// Naming protocol
char *GetProtoName(unsigned char strProto)
{
	bool bFound = false;
	int i;
	for(i = 0; i < PROTOCOL_NO; i++)
	{
		if(aOfProto[i].proto == strProto)
		{
			bFound = true;
			break ;
		}	
	}
	if(bFound)
		return aOfProto[i].pprototext;

	return aOfProto[PROTOCOL_NO].pprototext;
}

// Packet recever proc from Arkady Frankel 
UINT threadFunc(LPVOID p)
{
	USES_CONVERSION;

 	CNetTrafficDlg *pDlg = static_cast<CNetTrafficDlg *>(p);

	char buf[2048];
	char *bufIP, *bufTCP, *bufUDP;
	MSG msg;
	int iRet;
	DWORD dwErr;
    char *pSource;
	char *pDest;

	IPHEADER *pIpHeader;
	TCPHEADER *pTcpHeader;
	UDPHEADER *pUdpHeader;
	ICMPHEADER *pIcmpHeader;

	in_addr ina;
	char szSource[16], szDest[16], szErr[128];
	char *pLastBuf = NULL;

	CString strLine;

    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE); // Force to make the queue
 	pDlg->m_threadID = GetCurrentThreadId();

	while(true)
	{
        if(PeekMessage(&msg, 0, WM_CLOSE, WM_CLOSE,PM_NOREMOVE))
		{
 	        closesocket(pDlg->m_s);
			break;
		}

		memset(buf, 0, sizeof(buf));

		iRet = recv(pDlg->m_s, buf, sizeof(buf), 0);
		if(iRet == SOCKET_ERROR )
		{
			dwErr = WSAGetLastError();
			sprintf_s(szErr, 128, "Error recv(threadFunc-1) = %ld ", dwErr);
			pDlg->m_cStatusEdit.SetWindowTextW(A2W(szErr));
			continue;
		}
		else
		{
			if(*buf)
			{ 
				// check IP header
				bufIP = buf;
        		pIpHeader = (IPHEADER*)bufIP;
				if(pIpHeader->proto == IPPROTO_TCP)
				{
					bufTCP = buf  + sizeof(IPHEADER);
					pTcpHeader = (TCPHEADER*)bufTCP;
				}
				if(pIpHeader->proto == IPPROTO_UDP)
				{
					bufUDP = buf  + sizeof(IPHEADER);
					pUdpHeader = (UDPHEADER*)bufTCP;
				}
				if(pIpHeader->proto == IPPROTO_ICMP)
				{
					bufUDP = buf  + sizeof(IPHEADER);
					pIcmpHeader = (ICMPHEADER*)bufTCP;
				}
				
	            WORD iLen = ntohs(pIpHeader->total_len);
				while(true)
				{
					if(iLen <= iRet)
					{
						// Source IP from header
						ina.S_un.S_addr = pIpHeader->sourceIP;
						pSource = inet_ntoa(ina);
						strcpy_s(szSource, 16, pSource);

						// Dest IP from header
						ina.S_un.S_addr = pIpHeader->destIP;
						pDest = inet_ntoa(ina);
						strcpy_s(szDest, 16, pDest);
						
						// SourceIP, Port, DestIP, Port, Length, Protocol, Flag, Process, TTL, ID
						if(pIpHeader->proto == IPPROTO_TCP)
						{
							strLine.Format(_T("%s\t%d\t%s\t%d\t%d\t%s\t%s\t%s\t%d\t%d"), 
								A2W(szSource), 
								pTcpHeader->sourcePort, 
								A2W(szDest), 
								pTcpHeader->destPort, 
								iLen, 
								A2W(GetProtoName(pIpHeader->proto)), 
								GetFlag(pTcpHeader->flags), 
								GetProc(p, pTcpHeader->sourcePort, pTcpHeader->destPort), 
								pIpHeader->ttl, 
								pIpHeader->ident);
							pDlg->AddToList(strLine, iRet - sizeof(IPHEADER) - sizeof(TCPHEADER), buf + sizeof(IPHEADER) + sizeof(TCPHEADER));
						}
						else
							if(pIpHeader->proto == IPPROTO_UDP && !IsBadReadPtr(pUdpHeader, sizeof(pUdpHeader)))
							{
								strLine.Format(_T("%s\t%d\t%s\t%d\t%d\t%s\t-\t%s\t%d\t%d"), 
									A2W(szSource), 
									pUdpHeader->sourcePort, 
									A2W(szDest), 
									pUdpHeader->destPort, 
									iLen, 
									A2W(GetProtoName(pIpHeader->proto)), 
									GetProc(p, 
									pUdpHeader->sourcePort, 
									pUdpHeader->destPort), 
									pIpHeader->ttl, 
									pIpHeader->ident);
								pDlg->AddToList(strLine, iRet - sizeof(IPHEADER) - sizeof(UDPHEADER), buf + sizeof(IPHEADER) + sizeof(UDPHEADER));
							}
							else
								if(pIpHeader->proto == IPPROTO_ICMP)
								{
									strLine.Format(_T("%s\t-\t%s\t-\t%d\t%s\t-\t-\t%d\t%d"), 
										A2W(szSource), 
										A2W(szDest), 
										iLen, 
										A2W(GetProtoName(pIpHeader->proto)), 
										pIpHeader->ttl, 
										pIpHeader->ident);
									pDlg->AddToList(strLine, iRet - sizeof(IPHEADER) - sizeof(ICMPHEADER), buf + sizeof(IPHEADER) + sizeof(ICMPHEADER));
								}
								else
								{
									strLine.Format(_T("%s\t-\t%s\t-\t%d\t%s\t-\t%d\t%d"), 
										A2W(szSource), 
										A2W(szDest), 
										iLen, 
										A2W(GetProtoName(pIpHeader->proto)), 
										pIpHeader->ttl, 
										pIpHeader->ident);
									pDlg->AddToList(strLine, iRet - sizeof(IPHEADER), buf + sizeof(IPHEADER));
								}

						// Sleep 50 stabilize work of list, otherwise sometimes pressing on scroll cased close of program
						Sleep(10); 

						if(iLen < iRet)
						{
							iRet -= iLen;
							bufIP += iLen;
							bufTCP += iLen + sizeof(IPHEADER);
        					pIpHeader = (IPHEADER *)bufIP;
							pTcpHeader = (TCPHEADER*)bufTCP;
						}
						else
							break ; // pIpHeader->total_len == iRet and go out
					}
					else
					{ 
						// Read last part of buf. I wrote it , but always recv() read exactly the lenght of the packet
						int iLast = iLen - iRet;
						pLastBuf = new char [iLen];
						int iReaden = iRet;
						memcpy(pLastBuf, bufIP, iReaden);
						iRet = recv(pDlg->m_s, pLastBuf + iReaden, iLast, 0);
						if(iRet == SOCKET_ERROR)
						{
							dwErr = WSAGetLastError() ;
							sprintf_s(szErr, 128, "Error recv(threadFunc-2) = %ld ", dwErr);
							pDlg->m_cStatusEdit.SetWindowTextW(A2W(szErr));
							break ;
						}
						else
						{
							bufIP = pLastBuf;	
							bufTCP = pLastBuf + sizeof(IPHEADER);
       						pIpHeader = (IPHEADER *)bufIP;
							pTcpHeader = (TCPHEADER*)bufTCP;
							if(iRet == iLast)
							{
								iRet = iLen;
							}
							else
							{ 
								// read all last data
								iReaden += iRet;
								iLast -= iRet;
								while(true)
								{
									iRet = recv(pDlg->m_s, pLastBuf + iReaden, iLast, 0);
									if(iRet == SOCKET_ERROR)
									{
										dwErr = WSAGetLastError() ;
										sprintf_s(szErr, 128, "Error recv(threadFunc-3) = %ld ", dwErr);
										pDlg->m_cStatusEdit.SetWindowTextW(A2W(szErr));
										break ;
									}
									else
									{
								        iReaden += iRet;
								        iLast -= iRet;
									    if(iLast <= 0) 
										    break;
									}	
								} // while
							}
						}
					}	
				}   // while
				if(pLastBuf)
					delete [] pLastBuf;
			}
			else
			{
				pDlg->m_cStatusEdit.SetWindowTextW(_T("No data on network"));
				continue ;
			}
			//  Polling each 100 millisecond
			Sleep(10); 
		}
	}

	return TRUE;
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CNetTrafficDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// Select Network
void CNetTrafficDlg::OnCbnSelchangeCombo1()
{
	// TODO: Add your control notification handler code here
	int iSel = this->m_cAddrCmb.GetCurSel();
	GetIPAddrTable(iSel);
}

// Get Network list
int CNetTrafficDlg::GetIPAddrTable(int iConn)
{
	// Before calling AddIPAddress we use GetIpAddrTable to get
	// an adapter to which we can add the IP.
	PMIB_IPADDRTABLE pIPAddrTable;
	DWORD dwSize = 0;
	int iNo = 0;

	USES_CONVERSION;

	pIPAddrTable = (MIB_IPADDRTABLE*) GlobalAlloc(0, sizeof( MIB_IPADDRTABLE));

	if(pIPAddrTable)
	{
		// Make an initial call to GetIpAddrTable to get the
		// necessary size into the dwSize variable
		if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER)
		{
			GlobalFree(pIPAddrTable);
			pIPAddrTable = (MIB_IPADDRTABLE *) GlobalAlloc (0, dwSize );
		}
	}
	else
	{
		this->m_cStatusEdit.SetWindowTextW(_T("Memory allocation failed."));
	}


	if(pIPAddrTable)
	{
		// Make a second call to GetIpAddrTable to get the
		// actual data we want
		DWORD dwRetVal;
		if((dwRetVal = GetIpAddrTable( pIPAddrTable, &dwSize, 0 )) != NO_ERROR)
		{
			this->m_cStatusEdit.SetWindowTextW(_T("Call to GetIpAddrTable failed."));
			return 0;
		}

		CString strLine;
		for(int i = 0; i < (int)pIPAddrTable->dwNumEntries; i++)
		{
			if(iConn == -1)
			{
				strLine.Format(_T("%d.%d.%d.%d"), (unsigned char)(pIPAddrTable->table[i].dwAddr), (unsigned char)(pIPAddrTable->table[i].dwAddr >> 8), (unsigned char)(pIPAddrTable->table[i].dwAddr >> 16), (unsigned char)(pIPAddrTable->table[i].dwAddr >> 24));
				this->m_cAddrCmb.AddString(strLine);
				this->m_cAddrCmb.SetItemData(i, pIPAddrTable->table[i].dwAddr);
			}

			if(iConn == i)
			{
				strLine.Format(_T("%d.%d.%d.%d"), (unsigned char)(pIPAddrTable->table[i].dwAddr), (unsigned char)(pIPAddrTable->table[i].dwAddr >> 8), (unsigned char)(pIPAddrTable->table[i].dwAddr >> 16), (unsigned char)(pIPAddrTable->table[i].dwAddr >> 24));
				this->m_cAddr.SetWindowTextW(strLine);

				strLine.Format(_T("%d.%d.%d.%d"), (unsigned char)(pIPAddrTable->table[i].dwMask), (unsigned char)(pIPAddrTable->table[i].dwMask >> 8), (unsigned char)(pIPAddrTable->table[i].dwMask >> 16), (unsigned char)(pIPAddrTable->table[i].dwMask >> 24));
				this->m_cMask.SetWindowTextW(strLine);

				strLine.Format(_T("%d"), pIPAddrTable->table[i].dwIndex);
				this->m_cIdx.SetWindowTextW(strLine);

				strLine.Format(_T("%d.%d.%d.%d"), (unsigned char)(pIPAddrTable->table[i].dwBCastAddr), (unsigned char)(pIPAddrTable->table[i].dwBCastAddr >> 8), (unsigned char)(pIPAddrTable->table[i].dwBCastAddr >> 16), (unsigned char)(pIPAddrTable->table[i].dwBCastAddr >> 24));
				this->m_cBCast.SetWindowTextW(strLine);

				CString strType;
				if(pIPAddrTable->table[i].wType & MIB_IPADDR_PRIMARY)
					strType.Append(_T("Primary "));

				if(pIPAddrTable->table[i].wType & MIB_IPADDR_DYNAMIC)
					strType.Append(_T("Dynamic "));

				if(pIPAddrTable->table[i].wType & MIB_IPADDR_DISCONNECTED)
					strType.Append(_T("Disconnected "));

				if(pIPAddrTable->table[i].wType & MIB_IPADDR_DELETED)
					strType.Append(_T("Deleted "));
				
				this->m_cType.SetWindowTextW(strType);

				GetInterfaceTable(pIPAddrTable->table[i].dwIndex);

				iNo = i;
			}
		}
		this->m_cAddrCmb.SetCurSel(iNo);
	}

	if(pIPAddrTable)
		GlobalFree(pIPAddrTable);

	return iNo;
}

// Get Interface Info
int CNetTrafficDlg::GetInterfaceTable(int iConn)
{
	USES_CONVERSION;

	// Declare and initialize variables
	PMIB_IFTABLE ifTable;
	DWORD dwSize = 0;
	DWORD dwRetVal = 0;
	int iNo = 0;

	// Allocate memory for our pointers
	ifTable = (MIB_IFTABLE*) GlobalAlloc(0, sizeof(MIB_IFTABLE));

	// Make an initial call to GetIfTable to get the necessary size into the dwSize variable
	if (GetIfTable(ifTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER)
	{
		GlobalFree(ifTable);
		ifTable = (MIB_IFTABLE *) GlobalAlloc(0, dwSize);
	}

	// Make a second call to GetIfTable to get the actual data we want
	if ((dwRetVal = GetIfTable(ifTable, &dwSize, 0)) == NO_ERROR)
	{
		CString strLine;
		for(int i = 0; i < (int)ifTable->dwNumEntries; i++)
		{
			if(ifTable->table[i].dwIndex == iConn)
			{
				strLine.Format(_T("%02x-%02x-%02x-%02x-%02x-%02x"), ifTable->table[i].bPhysAddr[0], ifTable->table[i].bPhysAddr[1], ifTable->table[i].bPhysAddr[2], ifTable->table[i].bPhysAddr[3], ifTable->table[i].bPhysAddr[4], ifTable->table[i].bPhysAddr[5]);
				this->m_cMAC.SetWindowTextW(strLine);

				this->m_cDesc.SetWindowTextW(A2W((char*)ifTable->table[i].bDescr));
			}
		}
	}
	else
	{
		this->m_cStatusEdit.SetWindowTextW(_T("GetIfTable failed."));
	}

	if(ifTable)
		GlobalFree(ifTable);

	return iNo;
}

// Strat listening
void CNetTrafficDlg::OnBnClickedStart()
{
	// TODO: Add your control notification handler code here
	this->Start();
}

// Start thread
bool CNetTrafficDlg::Start(void) 
{
	USES_CONVERSION;

	char szErr[128];

	DWORD dwErr;
    SOCKADDR_IN sa;

	int iSel = this->m_cAddrCmb.GetCurSel();
	this->m_dwIPSource = (DWORD)this->m_cAddrCmb.GetItemData(iSel);

	CString str;

	// Initialize socket system
	WSADATA wsadata;
	WORD version = MAKEWORD(1, 1);
	WSAStartup(version, &wsadata);

	DWORD dwBufferLen[10];
	DWORD dwBufferInLen = 1;
	DWORD dwBytesReturned = 0;

	// Create Socket
	this->m_s = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	if(this->m_s == INVALID_SOCKET)
	{
		dwErr = WSAGetLastError();
		sprintf_s(szErr, 128, "Error socket(Start) = %ld ", dwErr);
		this->m_cStatusEdit.SetWindowTextW(A2W(szErr));
		closesocket(this->m_s);
		return false;
	}

	int iRcvTimeOut = 9000; 
	// Set Socket timeout to 5 sec insteadof 45 as default
    if(setsockopt(this->m_s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&iRcvTimeOut, sizeof(iRcvTimeOut)) == SOCKET_ERROR)
	{
		dwErr = WSAGetLastError();
		sprintf_s(szErr, 128, "Error WSAIoctl(SO_RCVTIMEO) = %ld ", dwErr);
		this->m_cStatusEdit.SetWindowTextW(A2W(szErr));
		closesocket(this->m_s);
		return false;
	}

	sa.sin_family = AF_INET;
	sa.sin_port = htons(7000);
	sa.sin_addr.s_addr = this->m_dwIPSource;

	// Bining ths socket
    if(bind(this->m_s, (PSOCKADDR)&sa, sizeof(sa)) == SOCKET_ERROR)
	{
		dwErr = WSAGetLastError();
		sprintf_s(szErr, 128, "Error bind(Start) = %ld ", dwErr);
		this->m_cStatusEdit.SetWindowTextW(A2W(szErr));
		closesocket(this->m_s);
		return false;
	} 

	// Socket with SIO_RCVALL option
    if(SOCKET_ERROR != WSAIoctl(this->m_s, SIO_RCVALL, &dwBufferInLen, sizeof(dwBufferInLen), &dwBufferLen, sizeof(dwBufferLen), &dwBytesReturned, NULL, NULL))
	{
        AfxBeginThread(threadFunc, (LPVOID)this);
	}
	else
	{
		dwErr = WSAGetLastError();
		sprintf_s(szErr, 128, "Error WSAIoctl(SIO_RCVALL) = %ld ", dwErr);
		this->m_cStatusEdit.SetWindowTextW(A2W(szErr));
		closesocket(this->m_s);
		return false;
	}

	this->m_cStart.EnableWindow(false);
	this->m_cAddrCmb.EnableWindow(false);
	this->m_cStop.EnableWindow(true);

	return true;
}

// Add items to listcontrol
// Source, Port, Destination, Port, Length, Protocol, Flag, Process, TTL, ID
bool CNetTrafficDlg::AddToList(CString strLine, int iLen, char* cBuffer) 
{
	// Dependencies
	bool bOK = false;
	bool bClient = false;
	bool bServer = false;
	bool bProtocol = false;
	bool bSource = false;
	bool bDest = false;
	bool bLocPort = false;
	bool bScrPort = false;
	bool bDesPort = false;

	CString strTmp1 = strLine;
	CString strTmp2 = strLine;

	USES_CONVERSION;

	// Source
	in_addr Addr;
	this->m_cSource.GetAddress(Addr.S_un.S_addr);
	Addr.S_un.S_addr = ntohl(Addr.S_un.S_addr);
	CString strSource = A2W(inet_ntoa(Addr));

	// Dest
	this->m_cDest.GetAddress(Addr.S_un.S_addr);
	Addr.S_un.S_addr = ntohl(Addr.S_un.S_addr);
	CString strDest = A2W(inet_ntoa(Addr));

	// Local
	int iSel1 = this->m_cAddrCmb.GetCurSel();
	CString strLocal;
	this->m_cAddrCmb.GetLBText(iSel1, strLocal);

	// Protocol
	int iSel2 = this->m_cProtocol.GetCurSel();
	CString strProtocol;
	this->m_cProtocol.GetLBText(iSel2, strProtocol);

	// Ports
	CString strLocalPort, strSourcePort, strDestPort;
	this->m_cLocalPort.GetWindowTextW(strLocalPort);
	this->m_cSourcePort.GetWindowTextW(strSourcePort);
	this->m_cDestPort.GetWindowTextW(strDestPort);

	long lNo = 0;

	wchar_t *next_token1;
	wchar_t *token1 = wcstok_s(strTmp1.GetBuffer(), _T("\t"), &next_token1);

	int iCnt = 0;
	while(token1 != NULL)
	{
		// Source
		if(iCnt == 0 && wcscmp(token1, strLocal) == 0)
			bServer = true;
		if(iCnt == 0 && (wcscmp(token1, strSource) == 0 || strSource == _T("0.0.0.0")))
			bSource = true;

		// Source Port
		if(iCnt == 1 && (wcscmp(token1, strLocalPort) == 0 || strLocalPort.GetLength() == 0))
			bLocPort = true;
		if(iCnt == 1 && (wcscmp(token1, strSourcePort) == 0 || strSourcePort.GetLength() == 0))
			bScrPort = true;

		// Destination
		if(iCnt == 2  && wcscmp(token1, strLocal) == 0)
			bClient = true;
		if(iCnt == 2  && (wcscmp(token1, strDest) == 0 || strDest == _T("0.0.0.0")))
			bDest = true;

		// Destination Port
		if(iCnt == 3 && (wcscmp(token1, strLocalPort) == 0 || strLocalPort.GetLength() == 0))
			bLocPort = true;
		if(iCnt == 3 && (wcscmp(token1, strDestPort) == 0 || strDestPort.GetLength() == 0))
			bDesPort = true;

		// Length
		if(iCnt == 4 && (bClient || bServer))
			lNo = _wtol(token1);

		// Protocol
		if(iCnt == 5 && wcscmp(token1, strProtocol) == 0)
			bProtocol = true;

		token1 = wcstok_s(NULL, _T("\t"), &next_token1);
		iCnt++;
	}

	// Local or clien or server or all
	if(this->m_cHostCheck.GetCheck())
	{
		if(bServer && bLocPort)
			bOK = true;
		if(bClient && bLocPort)
			bOK = true;
	}
	else
	{
		if(bSource && bDest)
		{
			if(bSource && bScrPort)
				bOK = true;
			if(bDest && bDesPort)
				bOK = true;
		}
	}

	// Check protocol
	if(!this->m_cProtocol.GetCurSel() == 0 && !bProtocol)
		bOK = false;

	if(bOK)
	{
		// Increase counters
		wchar_t strBuff[32];
		if(this->m_cHostCheck.GetCheck())
		{
			if(bClient)
			{
				this->m_lReceive += lNo;
				_ltow_s(this->m_lReceive, strBuff, 32, 10);
				this->m_cReceived.SetWindowTextW(strBuff);
			}
			if(bServer)
			{
				this->m_lSent += lNo;
				_ltow_s(this->m_lSent, strBuff, 32, 10);
				this->m_cSent.SetWindowTextW(strBuff);
			}
		}

		// Add line to the listcontrol
		wchar_t *next_token2;
		wchar_t *token2 = wcstok_s(strTmp2.GetBuffer(), _T("\t"), &next_token2);
		iCnt = 0;		
		int iNo = this->m_cPacketList.GetItemCount();
		while(token2 != NULL)
		{
			LV_ITEM lvItem;
			if(iCnt == 0)
			{
				lvItem.mask = LVIF_TEXT; 
				lvItem.iItem = iNo; 
				lvItem.iSubItem = 0;
				lvItem.pszText = token2;
				this->m_cPacketList.InsertItem(&lvItem);

				ITEMDATA *iDat = new ITEMDATA;

				iDat->iLen = iLen;
				if(iLen < 2048)
				{
					iDat->pData = new char [iLen + 1];
					memset(iDat->pData, 0, iLen + 1);
					memcpy_s(iDat->pData, iLen , cBuffer, iLen);
				}

				this->m_cPacketList.SetItemData(iNo, (DWORD_PTR)iDat);
			}
			else
			{
				lvItem.mask = LVIF_TEXT;
				lvItem.iSubItem = iCnt;
				lvItem.pszText = token2;
				this->m_cPacketList.SetItem(&lvItem);
			}

			token2 = wcstok_s(NULL, _T("\t"), &next_token2);
			iCnt++;
		}
		// Flile Logging
		if(this->m_cLogCheck.GetCheck())
		{
			this->LogLine(strLine);
		}
	}

	return bOK;
}

// Stop Thread
void CNetTrafficDlg::OnBnClickedStop()
{
	// TODO: Add your control notification handler code here
	PostThreadMessage(this->m_threadID, WM_CLOSE, 0, 0);
	this->m_threadID = 0;
	this->m_cStart.EnableWindow(true);
	this->m_cAddrCmb.EnableWindow(true);
	this->m_cStop.EnableWindow(false);
}

// Clear Packet list
void CNetTrafficDlg::OnBnClickedClear()
{
	// TODO: Add your control notification handler code here

	this->m_lSent = 0;
	this->m_lReceive = 0;

	ITEMDATA *dat;
	int iCol = this->m_cPacketList.GetItemCount();
	for(int i = 0;i < iCol; i++)
	{
		dat = (ITEMDATA*)this->m_cPacketList.GetItemData(i);
		if(dat != NULL)
		{
			delete [] dat->pData;
			delete dat;
		}
	}

	this->m_cPacketList.DeleteAllItems();

}

// Select a packet from the list
void CNetTrafficDlg::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if(OnSelchange(pNMLV) == 1)
	{
		int iSel = pNMLV->iItem;
//		CString strSourceIP = this->m_cPacketList.GetItemText(iSel, 0);
		ViewData(iSel);
	}
}

// Aquire what happend in lsitbox
int CNetTrafficDlg::OnSelchange(LPNMLISTVIEW pnmlv)
{
    if(pnmlv->uChanged & LVIF_STATE)
    {
        // We compare the old state vs. the new state to see how
        // the item is changing.

        if(((pnmlv->uOldState & LVIS_FOCUSED) == 0) && (pnmlv->uNewState & LVIS_FOCUSED))
        {
            // This item is getting the focus.
            return 1;
        }
        if(((pnmlv->uNewState & LVIS_FOCUSED) == 0) && (pnmlv->uOldState & LVIS_FOCUSED))
        {
            // This item is losting focus.
            return 2;
        }
        if(((pnmlv->uNewState & LVIS_SELECTED) == 0) && (pnmlv->uOldState & LVIS_SELECTED))
        {
            // This item is being selected
            return 3;
        }
        if(((pnmlv->uNewState & LVIS_SELECTED) == 0) && (pnmlv->uNewState & LVIS_SELECTED))
        {
            // This item is being unselected
            return 4;
        }
    }
	return 0;
}

// Log a packet header
bool CNetTrafficDlg::LogLine(CString strLine)
{

	char cBuffer[2048];

	USES_CONVERSION;

	CString strFileName = _T("NetTarfic.log");

	DWORD dwnNumberOfBytesToWrite = 0;
	DWORD dwNumberOfBytesWritten = 0;

	char* strDateTime = GetDateTime(false);

	sprintf_s(cBuffer, 2048, "%s\t%s\r\n", strDateTime, W2A(strLine));

	delete [] strDateTime;

	HANDLE hFile = CreateFile(strFileName, FILE_APPEND_DATA, FILE_SHARE_WRITE, 0,OPEN_ALWAYS, FILE_FLAG_WRITE_THROUGH, 0);

	dwnNumberOfBytesToWrite = (int)strlen(cBuffer);
	BOOL bOK = WriteFile(hFile, cBuffer, dwnNumberOfBytesToWrite, &dwNumberOfBytesWritten, NULL);

	CloseHandle(hFile);

	return true;
}

// DateTime to String
char* CNetTrafficDlg::GetDateTime(bool bDateOnly)
{

    time_t tNow;
    struct tm sWhen;

    time(&tNow);
    localtime_s(&sWhen, &tNow);

	char cBuff[32];

    // Set the time format to yyyy-mm-dd hh:mm:ss
	if(bDateOnly)
	{
		sprintf_s(cBuff, 32, "%04d-%02d-%02d", sWhen.tm_year + 1900, sWhen.tm_mon + 1, sWhen.tm_mday);
	}
	else
	{
		sprintf_s(cBuff, 32, "%04d-%02d-%02d %02d:%02d:%02d", sWhen.tm_year + 1900, sWhen.tm_mon + 1, sWhen.tm_mday, sWhen.tm_hour, sWhen.tm_min, sWhen.tm_sec);
	}

	char *strRes = new char[strlen(cBuff) + 1];
	strcpy_s(strRes, strlen(cBuff) + 1, cBuff);

	return strRes;
}

//
void CNetTrafficDlg::OnBnClickedCheck3()
{
	// TODO: Add your control notification handler code here

	int iSel = this->m_cPacketList.GetSelectionMark();
	if(iSel > -1)
		this->ViewData(iSel);
}

//
void CNetTrafficDlg::ViewData(int iSel)
{
	ITEMDATA *iDat = (ITEMDATA*)this->m_cPacketList.GetItemData(iSel);

	if(iDat->iLen > 0)
	{

		CString strDat;
		CString strTmp;
		int iNo = 0;
		int iWidth = 80;
		if(this->m_cHexCheck.GetCheck())
			iWidth = 32;
		for(int i = 0; i < iDat->iLen; i++)
		{
			if(this->m_cHexCheck.GetCheck())
			{
//					strTmp.Format(_T("%02x "), (unsigned char)iDat->pData[i]);
				strTmp.Format(_T("%03d "), (unsigned char)iDat->pData[i]);
				strDat.Append(strTmp);
			}
			else
			{
				if(iDat->pData[i] == 0)
					strDat.Append(_T("•"));
				else
					strDat.AppendChar(iDat->pData[i]);

				if(iDat->pData[i] == 13 || iDat->pData[i] == 10)
					iNo = 0;
			}

			iNo++;
			if(iNo >= iWidth)
			{
				strDat.Append(_T("\r\n"));
				iNo = 0;
			}

		}

		this->m_cStatusEdit.SetWindowTextW(strDat);
	}
	else
	{
		this->m_cStatusEdit.SetWindowTextW(_T("Header only"));
	}
}

//
bool CNetTrafficDlg::GetProcesses(void)
{
	this->m_cProcCombo.AddString(_T("All"));
	this->CheckState(1);
	this->m_cProcCombo.SetCurSel(0);

	return true;
}

//
bool CNetTrafficDlg::CheckOSVersion(DWORD &OSVersion)
{
	OSVERSIONINFO osver;
	
	// Check to see if were running under Windows95 or Windows NT.
	osver.dwOSVersionInfoSize = sizeof(osver);
	if(!GetVersionEx(&osver))
	{
		return false;
	}
	OSVersion = osver.dwPlatformId;

	if((OSVersion == VER_PLATFORM_WIN32_NT ) || (OSVersion == VER_PLATFORM_WIN32_WINDOWS))
	{
		return true;
	}		

	return false;
}

//
void CNetTrafficDlg::OnBnClickedState()
{
	// TODO: Add your control notification handler code here
	this->CheckState(0);
}

//
// Copyright (C) 1998-2002 Mark Russinovich
int CNetTrafficDlg::CheckState(int iMode)
{
	USES_CONVERSION;

	DWORD		error, dwSize;
	WORD		wVersionRequested;
	WSADATA		wsaData;
	HANDLE		hProcessSnap;
	PMIB_TCPEXTABLE tcpExTable;
	PMIB_TCPTABLE tcpTable;
	PMIB_UDPEXTABLE udpExTable;
	PMIB_UDPTABLE udpTable;
	BOOLEAN		exPresent;
	DWORD		i;
	CHAR		processName[MAX_PATH];
	CHAR		localname[HOSTNAMELEN], remotename[HOSTNAMELEN];
	CHAR		remoteport[PORTNAMELEN], localport[PORTNAMELEN];
	CHAR		localaddr[ADDRESSLEN], remoteaddr[ADDRESSLEN];

	CString strLines;

	// 
	// Check for NT
	//
	if( GetVersion() >= 0x80000000 )
	{
		this->m_cStatusEdit.SetWindowTextW(_T("Requres Windows NT/2K/XP."));
		return -1;
	}

	//
	// Initialize winsock
	//
	wVersionRequested = MAKEWORD( 1, 1 );
	if( WSAStartup(  wVersionRequested, &wsaData ) )
	{
		this->m_cStatusEdit.SetWindowTextW(_T("Could not initialize Winsock."));
		return -1;
	}
	//
	// Get options
	//
	exPresent = ExApisArePresent();

	//
	// Determine if extended query is available (it's only present
	// on XP and higher).
	//
	if( exPresent ) {

		//
		// Get the tables of TCP and UDP endpoints with process IDs
		//
		error = this->AllocateAndGetTcpExTableFromStack( &tcpExTable, TRUE, GetProcessHeap(), 2, 2 );
		if( error ) 
		{
			this->m_cStatusEdit.SetWindowTextW(_T("Failed to snapshot TCP endpoints."));
			return -1;
		}
		error = this->AllocateAndGetUdpExTableFromStack( &udpExTable, TRUE, GetProcessHeap(), 2, 2 );
		if( error ) 
		{
			this->m_cStatusEdit.SetWindowTextW(_T("Failed to snapshot UDP endpoints."));
			return -1;
		}

		//
		// Get a process snapshot. Note that we won't be guaranteed to 
		// exactly match a PID against a process name because a process could have exited 
		// and the PID gotten reused between our endpoint and process snapshots.
		//
		hProcessSnap = (HANDLE)this->CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
		if( hProcessSnap == INVALID_HANDLE_VALUE ) 
		{
			this->m_cStatusEdit.SetWindowTextW(_T("Failed to take process snapshot. Process names will not be shown."));
		}

		//
		// Dump the TCP table
		//
		CString strTmp;
		for( i = 0; i < tcpExTable->dwNumEntries; i++ ) 
		{

			sprintf( localaddr, "%s:%s", 
				GetIpHostName(TRUE, tcpExTable->table[i].dwLocalAddr, localname, HOSTNAMELEN), 
				GetPortName(tcpExTable->table[i].dwLocalPort, "tcp", localport, PORTNAMELEN ));

			sprintf( remoteaddr, "%s:%s",
				GetIpHostName(FALSE, tcpExTable->table[i].dwRemoteAddr, remotename, HOSTNAMELEN), 
				tcpExTable->table[i].dwRemoteAddr ? 
					GetPortName(tcpExTable->table[i].dwRemotePort, "tcp", remoteport, PORTNAMELEN ):
					"0" );

			strTmp.Format(_T("%-5s %s:%d      State:   %s"), _T("[TCP]"), 
				A2W(ProcessPidToName( hProcessSnap, tcpExTable->table[i].dwProcessId, processName )),
				tcpExTable->table[i].dwProcessId,
				A2W(TcpState[ tcpExTable->table[i].dwState ]));
			strLines.Append(strTmp);

			strTmp.Format(_T("      Local:   %s      Remote:  %s\r\n"),
				A2W(localaddr), A2W(remoteaddr));
			strLines.Append(strTmp);

			if(iMode == 1)
			{
				strTmp.Format(_T("%s:%d - %d"),
					A2W(ProcessPidToName( hProcessSnap, tcpExTable->table[i].dwProcessId, processName )),
					tcpExTable->table[i].dwProcessId, tcpExTable->table[i].dwLocalPort);

				this->m_cProcCombo.AddString(strTmp);
				this->m_cProcCombo.SetItemData(this->m_cProcCombo.GetCount() - 1, tcpExTable->table[i].dwLocalPort);
			}
		}

		//
		// Dump the UDP table
		//
		for( i = 0; i < udpExTable->dwNumEntries; i++ ) {

			sprintf( localaddr, "%s:%s", 
				GetIpHostName(TRUE, udpExTable->table[i].dwLocalAddr, localname, HOSTNAMELEN), 
				GetPortName(udpExTable->table[i].dwLocalPort, "tcp", localport, PORTNAMELEN ));

			strTmp.Format(_T("%-5s %s:%d"), _T("[UDP]"), 
				A2W(ProcessPidToName( hProcessSnap, udpExTable->table[i].dwProcessId, processName )),
				udpExTable->table[i].dwProcessId);
			strLines.Append(strTmp);

			strTmp.Format(_T("      Local:   %s      Remote:  %s\r\n"),
				A2W(localaddr), _T("*.*.*.*:*"));
			strLines.Append(strTmp);

			if(iMode == 1)
			{
				strTmp.Format(_T("%s:%d - %d"),
					A2W(ProcessPidToName( hProcessSnap, tcpExTable->table[i].dwProcessId, processName )),
					tcpExTable->table[i].dwProcessId, udpExTable->table[i].dwLocalPort);

				this->m_cProcCombo.AddString(strTmp);
				this->m_cProcCombo.SetItemData(this->m_cProcCombo.GetCount() - 1, udpExTable->table[i].dwLocalPort);
			}

		}

	} 
	else
	{

		//
		// Get the table of TCP endpoints
		//
		dwSize = 0;
		error = GetTcpTable( NULL, &dwSize, TRUE );
		if( error != ERROR_INSUFFICIENT_BUFFER ) 
		{
			this->m_cStatusEdit.SetWindowTextW(_T("Failed to snapshot TCP endpoints."));
			return -1;
		}
		tcpTable = (PMIB_TCPTABLE) malloc( dwSize );
		error = GetTcpTable( tcpTable, &dwSize, TRUE );
		if( error ) 
		{
			this->m_cStatusEdit.SetWindowTextW(_T("Failed to snapshot TCP endpoints."));
			return -1;
		}

		//
		// Get the table of UDP endpoints
		//
		dwSize = 0;
		error = GetUdpTable( NULL, &dwSize, TRUE );
		if( error != ERROR_INSUFFICIENT_BUFFER ) 
		{
			this->m_cStatusEdit.SetWindowTextW(_T("Failed to snapshot UDP endpoints."));
			return -1;
		}
		udpTable = (PMIB_UDPTABLE) malloc( dwSize );
		error = GetUdpTable( udpTable, &dwSize, TRUE );
		if( error ) 
		{
			this->m_cStatusEdit.SetWindowTextW(_T("Failed to snapshot UDP endpoints."));
			return -1;
		}

		//
		// Dump the TCP table
		//
		CString strTmp;
		for( i = 0; i < tcpTable->dwNumEntries; i++ ) 
		{

			sprintf( localaddr, "%s:%s", 
				GetIpHostName(TRUE, tcpTable->table[i].dwLocalAddr, localname, HOSTNAMELEN), 
				GetPortName(tcpTable->table[i].dwLocalPort, "tcp", localport, PORTNAMELEN ));

			sprintf( remoteaddr, "%s:%s",
				GetIpHostName(FALSE, tcpTable->table[i].dwRemoteAddr, remotename, HOSTNAMELEN), 
				tcpTable->table[i].dwRemoteAddr ? 
					GetPortName(tcpTable->table[i].dwRemotePort, "tcp", remoteport, PORTNAMELEN ):
					"0" );

			strTmp.Format(_T("%4s\tState:   %s"), _T("[TCP]"), 
				A2W(TcpState[ tcpTable->table[i].dwState ]));
			strLines.Append(strTmp);

			strTmp.Format(_T("       Local:   %s       Remote:  %s\r\n"),
				A2W(localaddr), A2W(remoteaddr));
			strLines.Append(strTmp);

		}

		//
		// Dump the UDP table
		//
		for( i = 0; i < udpTable->dwNumEntries; i++ ) {

			sprintf( localaddr, "%s:%s", 
				GetIpHostName(TRUE, udpTable->table[i].dwLocalAddr, localname, HOSTNAMELEN), 
				GetPortName(udpTable->table[i].dwLocalPort, "tcp", localport, PORTNAMELEN ));

			strTmp.Format(_T("[TCP]       Local:   %s       Remote:  %s\r\n"),
				A2W(localaddr), _T("*.*.*.*:*"));
			strLines.Append(strTmp);

		}
	}	

	if(iMode == 0)
		this->m_cStatusEdit.SetWindowTextW(strLines);

	return 0;

}

// ExApisArePresent
//
// Determines if Ex APIs (the XP version) are present, and
// if so, gets the function entry points.
//
//------------------------------------------------------------
BOOLEAN CNetTrafficDlg::ExApisArePresent(void)
{
	this->AllocateAndGetTcpExTableFromStack = (pAllocateAndGetTcpExTableFromStack) GetProcAddress( LoadLibrary( _T("iphlpapi.dll")), "AllocateAndGetTcpExTableFromStack");
	if(this->AllocateAndGetTcpExTableFromStack == NULL) return FALSE;

	this->AllocateAndGetUdpExTableFromStack = (pAllocateAndGetUdpExTableFromStack) GetProcAddress( LoadLibrary( _T("iphlpapi.dll")), "AllocateAndGetUdpExTableFromStack" );
	if(this->AllocateAndGetUdpExTableFromStack == NULL) return FALSE;

	this->CreateToolhelp32Snapshot = (pCreateToolhelp32Snapshot) GetProcAddress( GetModuleHandle( _T("kernel32.dll" )), "CreateToolhelp32Snapshot" );
	if(this->CreateToolhelp32Snapshot == NULL) return FALSE;

	this->Process32First = (pProcess32First) GetProcAddress( GetModuleHandle( _T("kernel32.dll" )), "Process32First" );
	if(this->Process32First == NULL) return FALSE;

	this->Process32Next = (pProcess32Next)GetProcAddress(GetModuleHandle( _T("kernel32.dll" )), "Process32Next");
	if(this->Process32Next == NULL) return FALSE;

	return TRUE;
}


//------------------------------------------------------------
//
// GetIpHostName
//
// Translate IP addresses into their name-resolved form
// if possible.
//
//------------------------------------------------------------
PCHAR CNetTrafficDlg::GetIpHostName(BOOL local, UINT ipaddr, PCHAR name, int namelen ) 
{
	struct hostent			*phostent;
	UINT					nipaddr;

	//
	// Try to translate to a name
	//
	nipaddr = htonl( ipaddr );
	if( !ipaddr  ) {

		if( !local ) {

			sprintf( name, "%d.%d.%d.%d", 
				(nipaddr >> 24) & 0xFF,
				(nipaddr >> 16) & 0xFF,
				(nipaddr >> 8) & 0xFF,
				(nipaddr) & 0xFF);

		} else {

			gethostname(name, namelen);
		}

	} else if( ipaddr == LOCALADDRESS ) {

		if( local ) {

			gethostname(name, namelen);
		} else {

			strcpy( name, "localhost" );
		}

	} else if( phostent = gethostbyaddr( (char *) &ipaddr,
		sizeof( nipaddr ), PF_INET )) {

		strcpy( name, phostent->h_name );

	} else {

		sprintf( name, "%d.%d.%d.%d", 
			(nipaddr >> 24) & 0xFF,
			(nipaddr >> 16) & 0xFF,
			(nipaddr >> 8) & 0xFF,
			(nipaddr) & 0xFF);
	}
	return name;
}

//------------------------------------------------------------
//
// GetPortName
//
// Translate port numbers into their text equivalent if 
// there is one
//
//------------------------------------------------------------
PCHAR CNetTrafficDlg::GetPortName(UINT port, PCHAR proto, PCHAR name, int namelen) 
{
	struct servent *psrvent;

	//
	// Try to translate to a name
	//
	if( psrvent = getservbyport( port, proto )) {

		strcpy( name, psrvent->s_name );

	} else {
	
		sprintf( name, "%d", htons( (WORD) port));
	}		
	return name;
}


//------------------------------------------------------------
//
// ProcessPidToName
//
// Translates a PID to a name.
//
//------------------------------------------------------------
PCHAR CNetTrafficDlg::ProcessPidToName(HANDLE hProcessSnap, DWORD ProcessId, PCHAR ProcessName)
{
	PROCESSENTRY32 processEntry;

	processEntry.dwSize = sizeof( processEntry );
	strcpy( ProcessName, "???" );
	if( !this->Process32First( hProcessSnap, &processEntry )) {

		return ProcessName;
	}
	do {

		if( processEntry.th32ProcessID == ProcessId ) {

			strcpy( ProcessName, (const char *)processEntry.szExeFile );
			return ProcessName;
		}

	} while( this->Process32Next( hProcessSnap, &processEntry ));

	return ProcessName;
}
