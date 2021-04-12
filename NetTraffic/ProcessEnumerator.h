// ----------------------------------------------------------------------------
// File Name:	    ProcessEnumerator.h
// Contents : 		ProcessEnumerator class definition.
// Originator: 	    Madhu Raykar.
// Date:		    04.12.05 
// Version:		    1.00.
// ----------------------------------------------------------------------------

#if !defined(AFX_PROCESSVIEWER_H__9E53E37B_B6F1_4604_8BA2_21685F61C013__INCLUDED_)
#define AFX_PROCESSVIEWER_H__9E53E37B_B6F1_4604_8BA2_21685F61C013__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <tlhelp32.h>
#include <vdmdbg.h>

typedef BOOL (CALLBACK *PROCENUMPROC)( DWORD, WORD, LPSTR, LPARAM ) ;

typedef BOOL (WINAPI *LPFENUMPROCESSES)( DWORD *, DWORD cb, DWORD * );
typedef BOOL (WINAPI *LPFENUMPROCESSMODULES)( HANDLE, HMODULE *,DWORD, LPDWORD );
typedef DWORD (WINAPI *LPFGETMODULEFILENAMEEX)( HANDLE, HMODULE,LPTSTR, DWORD );	
typedef INT (WINAPI *LPFVDMENUMTASKWOWEX)( DWORD,TASKENUMPROCEX  fp, LPARAM );

class CProcessEnumerator  
{
private :
	
	struct ENUMINFOSTRUCT
	{
	  DWORD          dwPID ;
	  PROCENUMPROC   lpProc ;
	  DWORD          lParam ;
	  BOOL           bEnd ;
	};

	
	HINSTANCE		m_hInstLib ;
	HINSTANCE		m_hInstLib2;
	DWORD			m_OSVersion;

	LPFENUMPROCESSES		m_lpfEnumProcesses;
	LPFENUMPROCESSMODULES	m_lpfEnumProcessModules;
	LPFGETMODULEFILENAMEEX	m_lpfGetModuleFileNameEx;
	LPFVDMENUMTASKWOWEX		m_lpfVDMEnumTaskWOWEx;

private :
	static BOOL Enum16( DWORD dwThreadId, 
						WORD hMod16, 
						WORD hTask16,
						PSZ pszModName, 
						PSZ pszFileName, 
						LPARAM lpUserDefined ) ;
	
	BOOL EnumWinNTProcs( PROCENUMPROC lpProc, LPARAM lParam );
	BOOL InitializeWinNT();

public:
		CProcessEnumerator();
		virtual ~CProcessEnumerator();
		BOOL EnumProcs( PROCENUMPROC lpProc, LPARAM lParam );
		BOOL Initialize(DWORD OSVersion);
};

#endif // !defined(AFX_PROCESSVIEWER_H__9E53E37B_B6F1_4604_8BA2_21685F61C013__INCLUDED_)
