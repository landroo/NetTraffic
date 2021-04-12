// ----------------------------------------------------------------------------
// File Name:	    ProcessViewer.cpp
// Contents : 		Implementation of ProcessViewer class.
// Originator: 	    Madhu Raykar.
// Date:		    04.12.05 
// Version:		    1.00.
// ----------------------------------------------------------------------------

#include "stdafx.h"
//#include "AdvancedTaskManager.h"
#include "ProcessEnumerator.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProcessEnumerator::CProcessEnumerator()
{
	m_hInstLib  = NULL;
	m_hInstLib2 = NULL;
	m_OSVersion = 0; //unknown

	m_lpfEnumProcesses				= NULL;
	m_lpfEnumProcessModules			= NULL;
	m_lpfGetModuleFileNameEx		= NULL;
	m_lpfVDMEnumTaskWOWEx			= NULL;
}

CProcessEnumerator::~CProcessEnumerator()
{
	if( m_hInstLib != NULL)
	{
		FreeLibrary( m_hInstLib) ;
	}
	
	if( m_hInstLib2 != NULL)
	{
		FreeLibrary( m_hInstLib2) ;
	}
}

// The EnumProcs function takes a pointer to a callback function
// that will be called once per process in the system providing
// process EXE filename and process ID.
// Callback function definition:
// BOOL CALLBACK Proc( DWORD dw, WORD wdTaskNo, LPCSTR lpstr, LPARAM lParam ) ;
BOOL CProcessEnumerator::EnumProcs( PROCENUMPROC lpProc, LPARAM lParam )
{
	// If Windows NT:
	if( m_OSVersion == VER_PLATFORM_WIN32_NT )
	{
		return EnumWinNTProcs(lpProc, lParam);
	}
	else
	{
		return FALSE ;
	}
	
	return TRUE ;
}

BOOL CProcessEnumerator::EnumWinNTProcs( PROCENUMPROC lpProc, LPARAM lParam )
{
	LPDWORD        lpdwPIDs ;
	DWORD          dwSize, dwSize2, dwIndex ;
	HMODULE        hMod ;
	HANDLE         hProcess ;
	char           szFileName[ MAX_PATH ] ;
	ENUMINFOSTRUCT sInfo ;
	
				
	// Call the PSAPI function EnumProcesses to get all of the
	// ProcID's currently in the system.
	// NOTE: In the documentation, the third parameter of
	// EnumProcesses is named cbNeeded, which implies that you
	// can call the function once to find out how much space to
	// allocate for a buffer and again to fill the buffer.
	// This is not the case. The cbNeeded parameter returns
	// the number of PIDs returned, so if your buffer size is
	// zero cbNeeded returns zero.
	// NOTE: The "HeapAlloc" loop here ensures that we
	// actually allocate a buffer large enough for all the
	// PIDs in the system.
	dwSize2 = 256 * sizeof( DWORD ) ;
	lpdwPIDs = NULL ;
	do
	{
		if( lpdwPIDs )
		{
			HeapFree( GetProcessHeap(), 0, lpdwPIDs ) ;
			dwSize2 *= 2 ;
		}
		lpdwPIDs = (LPDWORD)HeapAlloc( GetProcessHeap(), 0, dwSize2 );
		if( lpdwPIDs == NULL )
		{
			return FALSE ;
		}
		if( !m_lpfEnumProcesses( lpdwPIDs, dwSize2, &dwSize ) )
		{
			HeapFree( GetProcessHeap(), 0, lpdwPIDs ) ;
			return FALSE ;
		}
	}while( dwSize == dwSize2 ) ;
	
	dwSize /= sizeof( DWORD ) ;
	
	// Loop through each ProcID.
	for( dwIndex = 0 ; dwIndex < dwSize ; dwIndex++ )
	{
		szFileName[0] = 0 ;
		// Open the process .
		hProcess = OpenProcess(
			PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
			FALSE, lpdwPIDs[ dwIndex ] ) ;
		if( hProcess != NULL )
		{
			if( m_lpfEnumProcessModules( hProcess, &hMod,
				sizeof( hMod ), &dwSize2 ) )
			{
				// Get Full pathname:
				if( !m_lpfGetModuleFileNameEx( hProcess, hMod, (LPTSTR)szFileName, sizeof( szFileName ) ) )
				{
					szFileName[0] = 0 ;
				}
			}
			CloseHandle( hProcess ) ;
		}
		DWORD lastErr = GetLastError();
		
		if(!lpProc( lpdwPIDs[dwIndex], 0, szFileName, lParam))
			break ;

		if( _stricmp( szFileName+(strlen(szFileName)-9),
			"NTVDM.EXE")==0)
		{
			sInfo.dwPID = lpdwPIDs[dwIndex] ;
			sInfo.lpProc = lpProc ;
			sInfo.lParam = (DWORD)lParam ;
			sInfo.bEnd = FALSE ;
			try
			{
				m_lpfVDMEnumTaskWOWEx( lpdwPIDs[dwIndex],
					(TASKENUMPROCEX) Enum16,
					(LPARAM) &sInfo);
			}
			catch(...)
			{
				continue;
			}
			if(sInfo.bEnd)
				break ;
		}
	}
	
	HeapFree( GetProcessHeap(), 0, lpdwPIDs ) ;
	
	return TRUE ;
}


BOOL CProcessEnumerator::Enum16( 
							DWORD dwThreadId, 
							WORD hMod16, 
							WORD hTask16,
							PSZ pszModName, 
							PSZ pszFileName, 
							LPARAM lpUserDefined )
{
  BOOL bRet ;

  ENUMINFOSTRUCT *psInfo = (ENUMINFOSTRUCT *)lpUserDefined ;

  bRet = psInfo->lpProc( psInfo->dwPID, hTask16, pszFileName,
     psInfo->lParam ) ;

  if(!bRet)
  {
     psInfo->bEnd = TRUE ;
  }

  return !bRet;
} 

BOOL CProcessEnumerator::Initialize(DWORD OSVersion)
{
	m_OSVersion = OSVersion;
	// If Windows NT:
	if( m_OSVersion == VER_PLATFORM_WIN32_NT )
	{
		return InitializeWinNT();
	}
	return FALSE;
}

BOOL CProcessEnumerator::InitializeWinNT()
{
	// Get pointers to the functions in PSAPI.DLL
	// to use them
	m_hInstLib = LoadLibraryA( "PSAPI.DLL" ) ;
	if( m_hInstLib == NULL )
	{
//		AfxMessageBox("Could not load PSAPI.DLL", MB_OK);
		return FALSE ;
	}
	
	m_hInstLib2 = LoadLibraryA( "VDMDBG.DLL" ) ;
	if( m_hInstLib2 == NULL )
	{
//		AfxMessageBox("Could not load VDMDBG.DLL", MB_OK);
		return FALSE ;
	}
	
	// Get procedure addresses.
	m_lpfEnumProcesses = (BOOL(WINAPI *)(DWORD *,DWORD,DWORD*))
		GetProcAddress( m_hInstLib, "EnumProcesses" ) ;
	m_lpfEnumProcessModules = (BOOL(WINAPI *)(HANDLE, HMODULE *,
		DWORD, LPDWORD)) GetProcAddress( m_hInstLib,
		"EnumProcessModules" ) ;
	m_lpfGetModuleFileNameEx =(DWORD (WINAPI *)(HANDLE, HMODULE,
		LPTSTR, DWORD )) GetProcAddress( m_hInstLib,
		"GetModuleFileNameExA" ) ;
	m_lpfVDMEnumTaskWOWEx =(INT(WINAPI *)( DWORD, TASKENUMPROCEX,
		LPARAM))GetProcAddress( m_hInstLib2, "VDMEnumTaskWOWEx" );
	
	if( m_lpfEnumProcesses == NULL ||
		m_lpfEnumProcessModules == NULL ||
		m_lpfGetModuleFileNameEx == NULL ||
		m_lpfVDMEnumTaskWOWEx == NULL)
    {
		return FALSE ;
    }
	return TRUE;
}
