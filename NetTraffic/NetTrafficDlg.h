// NetTrafficDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


#include "windows.h"
#include "stdio.h"
#include "iprtrmib.h"
#include "tlhelp32.h"

#define LOCALADDRESS 0x0100007f
#define HOSTNAMELEN		256
#define PORTNAMELEN		256
#define ADDRESSLEN		HOSTNAMELEN+PORTNAMELEN

typedef struct {
  DWORD   dwState;        // state of the connection
  DWORD   dwLocalAddr;    // address on local computer
  DWORD   dwLocalPort;    // port number on local computer
  DWORD   dwRemoteAddr;   // address on remote computer
  DWORD   dwRemotePort;   // port number on remote computer
  DWORD	  dwProcessId;
} MIB_TCPEXROW, *PMIB_TCPEXROW;


typedef struct {
	DWORD			dwNumEntries;
	MIB_TCPEXROW	table[ANY_SIZE];
} MIB_TCPEXTABLE, *PMIB_TCPEXTABLE;



typedef struct {
  DWORD   dwLocalAddr;    // address on local computer
  DWORD   dwLocalPort;    // port number on local computer
  DWORD	  dwProcessId;
} MIB_UDPEXROW, *PMIB_UDPEXROW;


typedef struct {
	DWORD			dwNumEntries;
	MIB_UDPEXROW	table[ANY_SIZE];
} MIB_UDPEXTABLE, *PMIB_UDPEXTABLE;


//
// APIs that we link against dynamically in case they aren't 
// present on the system we're running on.
//
typedef UINT (CALLBACK* pAllocateAndGetTcpExTableFromStack)(PMIB_TCPEXTABLE *pTcpTable, BOOL bOrder, HANDLE heap, DWORD zero, DWORD flags);
typedef UINT (CALLBACK* pAllocateAndGetUdpExTableFromStack)(PMIB_UDPEXTABLE *pTcpTable, BOOL bOrder, HANDLE heap, DWORD zero, DWORD flags);
typedef UINT (CALLBACK* pCreateToolhelp32Snapshot)(DWORD dwFlags, DWORD th32ProcessID);
typedef UINT (CALLBACK* pProcess32First)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef UINT (CALLBACK* pProcess32Next)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);

// CNetTrafficDlg dialog
class CNetTrafficDlg : public CDialog
{
// Construction
public:
	CNetTrafficDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_NETTRAFFIC_DIALOG };

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
	DECLARE_MESSAGE_MAP()

private:
	int GetIPAddrTable(int iConn);
	int GetInterfaceTable(int iConn);
	bool Start(void);
	int OnSelchange(LPNMLISTVIEW pnmlv);
	bool LogLine(CString strLine);
	char* GetDateTime(bool bDateOnly);
	void ViewData(int iSel);
	bool GetProcesses(void);
	bool CheckOSVersion(DWORD &OSVersion);
	int CheckState(int iMode);
	BOOLEAN ExApisArePresent(void);
	PCHAR GetIpHostName(BOOL local, UINT ipaddr, PCHAR name, int namelen);
	PCHAR GetPortName(UINT port, PCHAR proto, PCHAR name, int namelen);
	PCHAR ProcessPidToName(HANDLE hProcessSnap, DWORD ProcessId, PCHAR ProcessName);

	CEdit m_cAddr;
	CEdit m_cMask;
	CEdit m_cIdx;
	CEdit m_cBCast;
	CEdit m_cType;
	CEdit m_cMAC;
	CEdit m_cDesc;
	CEdit m_cSent;
	CEdit m_cReceived;
	CEdit m_cLocalPort;
	CEdit m_cSourcePort;
	CEdit m_cDestPort;

	CComboBox m_cAddrCmb;
	CComboBox m_cProtocol;

	CListCtrl m_cPacketList;

	CIPAddressCtrl m_cSource;
	CIPAddressCtrl m_cDest;

	CButton m_cStart;
	CButton m_cStop;
	CButton m_cClear;

	CButton m_cHostCheck;
	CButton m_cLogCheck;
	CButton m_cHexCheck;
	CButton m_cResolve;

	DWORD m_dwIPSource;
	DWORD m_IPHost;

	long m_lSent;
	long m_lReceive;

	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedClear();
	afx_msg void OnBnClickedView();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnBnClickedState();

	pAllocateAndGetTcpExTableFromStack AllocateAndGetTcpExTableFromStack;
	pCreateToolhelp32Snapshot CreateToolhelp32Snapshot;
	pProcess32First Process32First;
	pProcess32Next Process32Next;
	pAllocateAndGetUdpExTableFromStack AllocateAndGetUdpExTableFromStack;

public:
	bool AddToList(CString strLine, int iLen, char* cBuffer);

	SOCKET m_s;
	DWORD m_threadID;
	CEdit m_cStatusEdit;
	CComboBox m_cProcCombo;
};
