#pragma once

#include <windows.h>
#include "..\shared\KTrace.hxx"

class KPrcsGroup{
private:
	/*============================================================================
	 * variables
	 *============================================================================*/

	HINSTANCE m_hKernel32;
	HANDLE m_hJob;
	HANDLE m_hParent;
	DWORD m_dwParentId;

public:
	/*============================================================================= 
	 * constructor
	 *=============================================================================*/
	KPrcsGroup( )
	{
		ktrace_in( );
		ktrace( "KPrcsGroup::KPrcsGroup( )" );
	
		m_dwParentId = 0;
		m_hParent = NULL;
		m_hJob = NULL;

	
		m_hKernel32 = LoadLibrary( "KERNEL32.DLL" );
		if( !m_hKernel32 )
		{
			ktrace( "LoadLibrary( ):err" );
			return;
		}
	}

public:
	/*============================================================================= 
	 * destructor
	 *=============================================================================*/
	~KPrcsGroup( )
	{
		ktrace_in( );
		ktrace( "KPrcsGroup::~KPrcsGroup( )" );

		this->Terminate( );
		if( m_hKernel32 ) FreeLibrary( m_hKernel32 );
	}

public:
	/*============================================================================= 
	 * create the job and set its parent
	 *=============================================================================*/
	bool CreateJob( DWORD dwParentId )
	{
		ktrace_in( );
		ktrace( "KPrcsGroup::CreateJob( " << dwParentId << " )" );
	
		HANDLE ( WINAPI * fCreateJobObject )( LPSECURITY_ATTRIBUTES, LPCSTR );
		BOOL ( WINAPI * fAssignProcessToJobObject )( HANDLE, HANDLE );

		if( m_dwParentId )
		{
			ktrace( "parent already set" );
			return( false );
		}

		m_dwParentId = dwParentId;

		m_hParent = OpenProcess( PROCESS_ALL_ACCESS , FALSE, m_dwParentId );
		if( !m_hParent )
		{
			ktrace( "OpenProcess( ):err" );
			return( false );
		}

		fCreateJobObject = ( HANDLE ( WINAPI * )( LPSECURITY_ATTRIBUTES, LPCSTR ) )GetProcAddress( m_hKernel32
																								 , "CreateJobObjectA" );
		if( !fCreateJobObject )
		{
			ktrace( "GetProcAddress( ):err" );
			return( false );
		}

		fAssignProcessToJobObject = ( BOOL ( WINAPI * )( HANDLE, HANDLE ) )GetProcAddress( m_hKernel32
																						 , "AssignProcessToJobObject" );
		if( !fAssignProcessToJobObject )
		{
			ktrace( "GetProcAddress1( ):err" );
			return( false );
		}

		m_hJob = fCreateJobObject( NULL, NULL );
		if( !m_hJob )
		{
			ktrace( "fCreateJobObject( ):err " << GetLastError( ) );
			return( false );
		}
	
		if( !fAssignProcessToJobObject( m_hJob, m_hParent ) )
		{
			ktrace( "fAssignProcessToJobObject( ):err "  << GetLastError( ) );
			return( false );
		}
		return( true );
	}

public:
	/*============================================================================= 
	 * terminate group
	 *=============================================================================*/
	bool Terminate( ){

		ktrace_in( );
		ktrace( "KPrcsGroup::Terminate( )" );

		if( !this->TerminateNT5( ) ) this->TerminateNT4( );

		if( m_hParent != NULL )
		{
			TerminateProcess( m_hParent, 0 );
			CloseHandle( m_hParent );
			m_hParent = NULL;
			m_dwParentId = 0;
		}
	
		return( true );
	}

private:
	/*============================================================================= 
	 * terminate group win2000/xp
	 *=============================================================================*/
	bool TerminateNT5( ){
		ktrace_in( );
		ktrace( "KPrcsGroup::TerminateNT5( )" );

		BOOL ( WINAPI * fTerminateJobObject )( HANDLE, UINT );

		if( !m_hJob )
		{
			ktrace( "m_hJob is null" );
			return( false );
		}

		fTerminateJobObject = ( BOOL ( WINAPI * )( HANDLE, UINT ) )GetProcAddress( m_hKernel32, "TerminateJobObject" );
		if( !fTerminateJobObject )
		{
			ktrace( "GetProcAddress( ):err" );
			return( false );
		}
		
		if( !fTerminateJobObject( m_hJob, 0 ) )
		{
			ktrace( "fTerminateJobObject( ):err" );
			return( false );
		}

		m_hJob = NULL;

		return( true );
	}

private:
	/*============================================================================= 
	 * terminate group nt4
	 *=============================================================================*/
	bool TerminateNT4( ){

		ktrace_in( );
		ktrace( "KPrcsGroup::TerminateNT4( )" );

		HINSTANCE hPsapi=NULL;
		HINSTANCE hNtdll=NULL;
		unsigned long aulPID[5000];
		unsigned long ulProcessCount=0;
		BOOL (WINAPI *lpfEnumProcesses)(DWORD *, DWORD, DWORD *); // PSAPI Function EnumProcesses
		BOOL (WINAPI *lpfNtQueryInformationProcess)(HANDLE, unsigned long, void *,unsigned long,unsigned long *); // NTDLL Function NtQueryInformationProcess
		HANDLE hProcess=NULL;
		unsigned int i=0;
		unsigned long ulRetLen=0;
		struct PROCESS_BASIC_INFORMATION
		{
			DWORD ExitStatus;
			DWORD PebBaseAddress;
			DWORD AffinityMask;
			DWORD BasePriority;
			ULONG UniqueProcessId;
			ULONG InheritedFromUniqueProcessId;
		} pbi;
	
		if( !m_dwParentId )
		{
			ktrace( "m_dwParentId is 0" );
			return( false );
		}
	
		hPsapi = LoadLibrary( "PSAPI.DLL" );
		if( !hPsapi )
		{
			ktrace( "LoadLibrary( ):err" );
			return( false );
		}
	
		// Get procedure addresses.
		lpfEnumProcesses = ( BOOL ( WINAPI * )( DWORD *, DWORD, DWORD * ) )GetProcAddress( hPsapi, "EnumProcesses" );
		if( !lpfEnumProcesses )
		{
			ktrace( "GetProcAddress( ):err" );
			return( false );
		}
	
		hNtdll = LoadLibrary( "NTDLL.DLL" );
		if( !hNtdll )
		{
			ktrace( "LoadLibrary1( ):err" );
			return( false );
		}
	
		// Get procedure addresses.
		lpfNtQueryInformationProcess = ( BOOL ( WINAPI * )( HANDLE, unsigned long, void *, unsigned long, unsigned long * ) )
													GetProcAddress( hNtdll, "NtQueryInformationProcess" );
		if( !lpfNtQueryInformationProcess )
		{
			ktrace( "GetProcAddress1( ):err" );
			return( false );
		}
	
		if( !lpfEnumProcesses( aulPID, 4990, &ulProcessCount ) )
		{
			ktrace( "lpfEnumProcesses( ):err" );
			return( false );
		}
		ulProcessCount = ulProcessCount / sizeof( unsigned long );

		for( i = 0; i < ulProcessCount; i++ )
		{
			hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, aulPID[ i ] );
			if( !hProcess )
			{
				ktrace( "OpenProcess( " << aulPID[ i ] << " ):err" );
				continue;
			}
			lpfNtQueryInformationProcess( hProcess
					,0
					, ( void* )&pbi
					, sizeof( PROCESS_BASIC_INFORMATION )
					, &ulRetLen );
			if( m_dwParentId == pbi.InheritedFromUniqueProcessId ) TerminateProcess( hProcess, 0 );
			CloseHandle( hProcess );
		}

		FreeLibrary( hPsapi );
		FreeLibrary( hNtdll );
		
		return( true );
	}
};
