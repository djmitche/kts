#pragma once

#include <windows.h>
#include <vector>

#include "..\shared\KTrace.hxx"

class KTermSessions
{
private:
	/*==============================================================================
	 * stored pids
	 *=============================================================================*/
	std::vector< DWORD > pids;

	/*==============================================================================
	 * check if pid is active
	 *=============================================================================*/
private:
	bool Active( DWORD pid )
	{
		ktrace_in( );
		ktrace( "KTermSessions::Active( " << pid << " )" );

		HANDLE process = OpenProcess( PROCESS_QUERY_INFORMATION , false, pid );

		if( !process )
		{
			return( false );
		}

		DWORD exitcode;

		if( !GetExitCodeProcess( process, &exitcode ) )
		{
			CloseHandle( process );
			ktrace( "GetExitCodeProcess( " << pid << " ):err " );
			kerror( "GetExitCodeProcess( " << pid << " ):err " );
			return( false );
		}

		CloseHandle( process );
		ktrace( "[ " << ( BOOL )( exitcode == STILL_ACTIVE ) << " ]" )
		return( exitcode == STILL_ACTIVE );

	}

public:
	/*==============================================================================
	 * add pid to sessions
	 *=============================================================================*/
	void Add( DWORD pid )
	{
		ktrace_in( );
		ktrace( "KTermSessions::Add( " << pid << " );" );

		this->pids.push_back( pid );

	}

public:
	/*==============================================================================
	 * get active pids count
	 * as a side effect it will remove the last non active pid from the vector
	 * thus keeping unused level low
	 *=============================================================================*/
	int Count( )
	{
		ktrace_in( );
		ktrace( "KTermSessions::Count( );" );

		int count = 0;
		std::vector< DWORD >::iterator dead = this->pids.end( );
		for( std::vector< DWORD >::iterator i = this->pids.begin( ); i != this->pids.end( ); i++ )
		{
			if( this->Active( *i ) ) count++;
			else dead = i;
		}

		if( dead != this->pids.end( ) ) this->pids.erase( dead );

		ktrace( "count = " << count );

		return( count );
	}

};