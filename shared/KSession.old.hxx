#pragma once
#include <windows.h>
#include <string>
#include <sstream>

#include "..\shared\KIni.hxx"
#include "..\shared\KTrace.hxx"

class KSession
{
	/*==============================================================================
	 * session states
	 *=============================================================================*/
	#define KSESSION_WAITING		0
	#define KSESSION_CONNECTED		1
	#define KSESSION_DISCONNECTED	2

	#define session_dir				".\\sessions\\"

public:
	/*==============================================================================
	 * constructor
	 *=============================================================================*/
	KSession( )
	{
		ktrace_in( );
		ktrace( "KSession::KSession( )" );

		KSession::SetSessionState( KSESSION_WAITING, "" );
	}
public:
	/*==============================================================================
	 * destructor
	 *=============================================================================*/
	~KSession( )
	{
		ktrace_in( );
		ktrace( "KSession::~KSession( )" );

		KSession::ClearSessionState( );
	}
	
public:
	/*==============================================================================
	 * set session state
	 *=============================================================================*/
	static void SetSessionState( int state, std::string login )
	{
		ktrace_in( );
		ktrace( "KSession::SetSessionState( " << state << ", " << login << " )" );

		KSession::ClearSessionState( );

		std::string name = KSession::GenerateSessionStateString( state, login );
		::CreateFile();

	}

private:
	/*==============================================================================
	 * clear session state
	 *=============================================================================*/
	static void ClearSessionState( )
	{
		ktrace_in( );
		ktrace( "KSession::ClearSessionState( )" );

		WIN32_FIND_DATA file;
		HANDLE find;

		std::stringstream smask;
		smask << session_dir << ::GetCurrentProcessId( ) << ".*";

		find = FindFirstFile( smask.str( ).c_str( ), &file );
		if( find == INVALID_HANDLE_VALUE )
		{
			return;
		} 

		if( !DeleteFile( file.cFileName ) )
		{
		}
		FindClose(find);
	}

private:
	/*==============================================================================
	 * gen session state
	 *=============================================================================*/
	static std::string GenerateSessionStateString( int state, std::string login )
	{
		ktrace_in( );
		ktrace( "KSession::GenerateSessionStateString( " << state << ", " << login << " )" );

		std::stringstream ss;
		ss << session_dir << ::GetCurrentProcessId( ) << "." << state << "." << login;
		
		ktrace( "ss = " << ss.str() );
		return( ss.str( ) );
	}
} ksession__1234;