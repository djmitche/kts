#pragma once
#define _WINSOCKAPI_
#include <string>

#include "..\shared\KWinsta.hxx"
#include "..\shared\KIni.hxx"
#include "..\shared\KTrace.hxx"
#include "..\shared\KSocketDup.hxx"
#include "..\shared\KSocket.hxx"
#include "..\shared\KSessionState.hxx"
#include "..\shared\KIPBan.hxx"
#include ".\KTermSessions.hxx"


class KDaemon
{
private:
	// =============================================================================
	// vars
	// =============================================================================
	KTermSessions sessions;
	SERVICE_STATUS status;
	SERVICE_STATUS_HANDLE statusHandle;
	HANDLE worker;

	// =============================================================================
	// params
	// =============================================================================
	struct Params
	{
		std::string service_name;
		std::string service_info;

		unsigned trace_level;
		std::string error_file;
		std::string trace_file;
		std::string log_file;

		std::string active_sessions_dir;

		int port;
		std::string ip;
		int max_sessions;
		bool debug_flag;
		bool use_ssh;

		Params( )
		{
			KIni ini;
			ini.File( ".\\kts.ini" );

			ini.GetKey( "KDaemon", "service_name", this->service_name );
			ini.GetKey( "KDaemon", "service_info", this->service_info );

			ini.GetKey( "KDaemon", "trace_level", this->trace_level );
			ini.GetKey( "KDaemon", "error_file", this->error_file );
			ini.GetKey( "KDaemon", "trace_file", this->trace_file );
			ini.GetKey( "KDaemon", "log_file", this->log_file );

			ini.GetKey( "KDaemon", "port", this->port );
			ini.GetKey( "KDaemon", "ip", this->ip );
			ini.GetKey( "KDaemon", "max_sessions", this->max_sessions );

			ini.GetKey( "KDaemon", "debug_flag", this->debug_flag );

			ini.GetKey( "KDaemon", "use_ssh", this->use_ssh );

			ini.GetKey( "KSession", "active_sessions_dir", this->active_sessions_dir );

			KWinsta::ExpandEnvironmentString( this->active_sessions_dir );
			KWinsta::ExpandEnvironmentString( this->log_file );
			KWinsta::ExpandEnvironmentString( this->trace_file );
			KWinsta::ExpandEnvironmentString( this->error_file );
		}
	} params;

private:
	// =============================================================================
	// event handler so we don't die on console events
	// =============================================================================
	static BOOL WINAPI event_handler( DWORD event1 )
	{
		ktrace_in( );
		ktrace( "KDaemon::event_handler( " << event1 << " )" )
		return( true );
	}

private:
	// =============================================================================
	// constructor / private!
	// =============================================================================
	KDaemon( )
	{
		ktrace_master_level( this->params.trace_level );
		ktrace_error_file( this->params.error_file );
		ktrace_trace_file( this->params.trace_file );
		ktrace_log_file( this->params.log_file );

		ktrace_in( );
		ktrace_level( 10 );

		ktrace( "KDaemon::KDaemon( )" );

		klog( "daemon.exe started" );

		SetConsoleCtrlHandler( KDaemon::event_handler, true );
	}

private:
	// =============================================================================
	// instance
	// =============================================================================
	static KDaemon * instance( )
	{
		static KDaemon * _instance = 0;

		if( _instance ) return( _instance );

		_instance = new( KDaemon );

		return( _instance );
	}

private:
	// =============================================================================
	// service control handler
	// =============================================================================
	static void _stdcall ServiceCtrlHandler( DWORD opcode )
	{
		ktrace_in( );
		ktrace( "KDaemon::ServiceCtrlHandler( " << opcode << " )" );

		switch( opcode )
		{
		case SERVICE_CONTROL_SHUTDOWN:
		case SERVICE_CONTROL_STOP:
			klog( "stopping service" );

			// kill the main thread
			if( KDaemon::instance( )->worker == 0 )
			{
				kerror( "no handle to worker thread/disregard if running on NT4/: err" );
				ExitProcess(0);
			}
			else
			{
				if( !TerminateThread( KDaemon::instance( )->worker, 0 ) )
				{
					kerror( "can't terminate worker thread[" << KDaemon::instance( )->worker << "]: err" );
				}
			}

			KDaemon::instance( )->status.dwWin32ExitCode = 0;
			KDaemon::instance( )->status.dwCurrentState = SERVICE_STOPPED;
			KDaemon::instance( )->status.dwCheckPoint = 0;
			KDaemon::instance( )->status.dwWaitHint = 0;

			if( !SetServiceStatus( KDaemon::instance( )->statusHandle, &KDaemon::instance( )->status ) )
			{
				ktrace( "SetServiceStatus( ):err" );
			}
			return;

		case SERVICE_CONTROL_INTERROGATE: 
			// Fall through to send current status. 
			break;
		default: 
			ktrace( "unsupported " << opcode );
			break;
		}

		if( !SetServiceStatus( KDaemon::instance( )->statusHandle, &KDaemon::instance( )->status ) )
		{
			ktrace( "SetServiceStatus( ):err" );
		}
		return; 
	}

private:
	// =============================================================================
	// init service
	// =============================================================================
	bool InitService( )
	{
		ktrace_in( );
		ktrace( "KDaemon::InitService( )" );
		
		KSessionState ss;
		ss.DeleteAllSessions( this->params.active_sessions_dir );

		if( this->params.debug_flag ) return( true );

		this->worker = KWinsta::OpenThread( THREAD_ALL_ACCESS, false, GetCurrentThreadId( ) );
		this->status.dwServiceType = SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS;
		this->status.dwCurrentState = SERVICE_START_PENDING;
		this->status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
		this->status.dwWin32ExitCode = 0;
		this->status.dwServiceSpecificExitCode = 0;
		this->status.dwCheckPoint = 0;
		this->status.dwWaitHint = 0;

		this->statusHandle = RegisterServiceCtrlHandler( this->params.service_name.c_str( ), KDaemon::ServiceCtrlHandler );
		
		if( !this->statusHandle )
		{
			ktrace( "RegisterServiceCtrlHandler( ):err" );
			return( false );
		}

		// set running status
		this->status.dwCurrentState = SERVICE_RUNNING;
		this->status.dwCheckPoint = 0;
		this->status.dwWaitHint = 0;

		if( !SetServiceStatus( this->statusHandle, &this->status ) )
		{
			ktrace( "SetServiceStatus( ):err" );
			return( false );
		}
		return( true );
	}

private:
	// =============================================================================
	// service main
	// =============================================================================
	static void _stdcall ServiceMain( unsigned long /*argc*/, char ** /*argv*/ )
	{

		ktrace_in( );
		ktrace_level( 10 );

		kbegin_try_block;

		ktrace( "KDaemon::ServiceMain( )" );

		klog( "starting service" );

		if( !KDaemon::instance( )->InitService( ) )
		{
			kerror( "KDaemon::instance( )->InitService( ):err" );
			return;
		}

		if( KSocket::WSAStartup( ) != 0 )
		{
			kerror( "KSocket::WSAStartup( ):err" );
			return;
		}

		// create the kts desktop
		if( !KWinsta::CreateWinstaAndDesktop( "kts" ) )
		{
			kerror( "KWinsta::CreateWinstaAndDesktop( kts ):err" );

			if( !KDaemon::instance( )->params.debug_flag ) return;
		}

		KDaemon::instance( )->AcceptConnections( );

		kend_try_block;
	}

private:
	// =============================================================================
	// accept connections
	// =============================================================================
	void AcceptConnections( )
	{
		ktrace_in( );
		ktrace( "KDaemon::AcceptConnections( )" );

		KSocket sock;
		KIPBan ip_ban;

		if( !sock.Listen( KDaemon::instance( )->params.port, KDaemon::instance( )->params.ip ) )
		{
			kerror( "sock.Listen( " << KDaemon::instance( )->params.port << " ):err" );
			return;
		}

		while( true )
		{
			KSocket tmp = sock.Accept( );

			if( tmp.GetSocket( ) == INVALID_SOCKET )
			{
				kerror( "sock.Accept( ):err" );
				return;
			}

			klog( "KTS connected to " << tmp.GetConnectionIP( ) << ":" << tmp.GetConnectionPort( ) );

			if( ip_ban.IsBanned( tmp.GetConnectionIP( ) ) )
			{
				klog( "ip is banned" );
				tmp.Close( );
				continue;
			}

			if( this->sessions.Count( ) >= this->params.max_sessions )
			{
				klog( "max_sessions reached" );
				tmp.Close( );
				continue;
			}


			STARTUPINFO si = {0};
			PROCESS_INFORMATION pi = {0};

			std::stringstream s;

			s << "shlex.exe \"session.exe.lnk\" "
				<< "\" -ppid:" << GetCurrentProcessId( ) 
				<< " -ip:" << tmp.GetConnectionIP( )
				<< " -port:" << tmp.GetConnectionPort( );

			if( this->params.debug_flag ) s << " -debug:1";
			if( this->params.use_ssh ) s << " -ssh:1";
			else s << " -ssh:0";
			s << "\"";

			si.cb = sizeof(si);

			ktrace( "CreateProcess( " << s.str( ) << " )" );
			if( !CreateProcess( NULL, ( char * )s.str( ).c_str( ), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) )
			{
				kerror( "CreateProcess( ):err" );
				return;
			}

			WaitForSingleObject(pi.hProcess, INFINITE);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);


			//transfer the socket to spawned process
			KSocketDup ksd;

			ksd.SendSocket( tmp.GetSocket( ), "kts" + tmp.GetConnectionIP( ) + "_" + tmp.GetConnectionPort( ) );

			this->sessions.Add( ksd.pid );

			tmp.Close( false );
		}
	}

public:
	// =============================================================================
	// run service
	// =============================================================================
	static void RunService( )
	{
		ktrace_in( );
		ktrace( "KDaemon::RunService( )" );

		SERVICE_TABLE_ENTRY service[] =
		{
			{ ( char * )KDaemon::instance( )->params.service_name.c_str( ), KDaemon::ServiceMain },
			{ 0, 0 }
		};
		if( !StartServiceCtrlDispatcher( service ) )
		{
			kerror( "StartServiceCtrlDispatcher( ):err" );
		}
	}

public:
	// =============================================================================
	// debug
	// =============================================================================
	static void Debug( )
	{
		ktrace_in( );
		ktrace( "KDaemon::Debug( )" );
		KDaemon::instance( )->params.debug_flag = true;

		KDaemon::ServiceMain( 0, 0 );
	}
};