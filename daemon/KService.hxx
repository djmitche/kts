#pragma once

#include "..\shared\KTrace.hxx"

class KService
{
private:
	/*==============================================================================
	 * get manager
	 *=============================================================================*/
	SC_HANDLE GetManager( )
	{
		ktrace_in( );
		ktrace( "KService::GetManager( )" );

		static SC_HANDLE manager = 0;

		if( !manager ) manager = OpenSCManager( NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS );

		return( manager );
	}

public:
	/*==============================================================================
	 * install service
	 *=============================================================================*/
	DWORD Install( const std::string & module, const std::string & name, const std::string & info )
	{
		ktrace_in( );
		ktrace( "KService::Install( " << module << ", " << name << ", " << info << " )" );

		DWORD error = ERROR_SUCCESS;

		SC_HANDLE service = CreateService( this->GetManager( ), name.c_str( ), name.c_str( )
				, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS
				, SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, module.c_str( )
				, NULL, NULL, NULL, NULL, NULL );
		if( !service )
		{
			error = GetLastError( );

			ktrace( "CreateService( " << this->GetManager( ) << " ):err" );
			return( error );
		}

		
		SERVICE_DESCRIPTION sd;

		char buff[ 1010 ];
		strncpy_s( buff, 1001, info.c_str( ), info.length() );
//		strncpy( buff, info.c_str( ), info.length() );
		buff[ 1000 ] = 0;

		sd.lpDescription = buff;

//		winnt4 incompatible code
//		if( !ChangeServiceConfig2( service, SERVICE_CONFIG_DESCRIPTION, &sd ) )
//		{
//			ktrace( "ChangeServiceConfig2( ):err" );
//		}

//		winnt4 compatible code

		HMODULE advapi32 = LoadLibrary( "Advapi32.dll" );
		if( advapi32 )
		{
			typedef BOOL ( WINAPI * CSC2 )( SC_HANDLE, DWORD, LPVOID );
			CSC2 csc2 = ( CSC2 )GetProcAddress( advapi32, "ChangeServiceConfig2A" );

			if( csc2 )
			{
				if( !csc2( service, SERVICE_CONFIG_DESCRIPTION, &sd ) )
				{
					ktrace( "ChangeServiceConfig2( ):err" );
				}
			}
			CloseHandle( advapi32 );
		}

		CloseServiceHandle( service );
		return( error );
	}

public:
	/*==============================================================================
	 * start service
	 *=============================================================================*/
	DWORD Start( std::string name )
	{
		ktrace_in( );
		ktrace( "KService::Start( " << name << " )" );

		DWORD error = ERROR_SUCCESS;

		SC_HANDLE service = OpenService( this->GetManager( ), name.c_str( ), SERVICE_ALL_ACCESS );
		if( !service )
		{
			error = GetLastError( );

			ktrace( "OpenService( " << this->GetManager( ) << " ):err"  );
			return( error );
		}

		if( !StartService( service, 0, NULL ) )
		{
			error = GetLastError( );

			ktrace( "StartService( ):err"  );

			CloseServiceHandle( service );
			return( error );
		}

		CloseServiceHandle( service );
		return( error );	
	}

public:
	/*==============================================================================
	 * stop service
	 *=============================================================================*/
	DWORD Stop( std::string name )
	{
		ktrace_in( );
		ktrace( "KService::Stop( " << name << " )" );

		DWORD error = ERROR_SUCCESS;

		SC_HANDLE service = OpenService( this->GetManager( ), name.c_str( ), SERVICE_ALL_ACCESS );
		if( !service )
		{
			error = GetLastError( );

			ktrace( "OpenService( " << this->GetManager( ) << " ):err"  );
			return( error );
		}

		SERVICE_STATUS status;
		if( !ControlService( service, SERVICE_CONTROL_STOP, &status ) )
		{
			error = GetLastError( );

			ktrace( "ControlService( ):err"  );

			CloseServiceHandle( service );
			return( error );
		}

		CloseServiceHandle( service );
		return( error );	
	}

public:
	/*==============================================================================
	 * uninstall service
	 *=============================================================================*/
	DWORD Uninstall( std::string name )
	{
		ktrace_in( );
		ktrace( "KService::Uninstall( " << name << " )" );

		DWORD error = ERROR_SUCCESS;

		SC_HANDLE service = OpenService( this->GetManager( ), name.c_str( ), SERVICE_ALL_ACCESS );
		if( !service )
		{
			error = GetLastError( );
			ktrace( "OpenService( " << this->GetManager( ) << " ):err"  );
			return( error );
		}

		SERVICE_STATUS status;
		if( !ControlService( service, SERVICE_CONTROL_STOP, &status ) )
		{
			ktrace( "ControlService( ):err"  );
		}

		if( !DeleteService( service ) )
		{
			error = GetLastError( );
			ktrace( "DeleteService( ):err"  );

			CloseServiceHandle( service );
			return( error );
		}
		CloseServiceHandle( service );
		return( error );	
	}

public:
	/*==============================================================================
	 * run service main
	 *=============================================================================*/
	void RunService( std::string name, LPSERVICE_MAIN_FUNCTION f )
	{
		SERVICE_TABLE_ENTRY service[] =
		{
			{ ( char * )name.c_str( ), f },
			{ 0, 0 }
		};

		StartServiceCtrlDispatcher( service );
	}

};