#pragma once

#define NOCRYPT        /* cryptlib nedds this to work with windows headers */
#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */

#include "..\shared\kts.h"
#include "..\shared\KSocket.hxx"
#include "..\shared\KSsh.hxx"
#include "..\shared\KFlags.hxx"

class KCommUtils
{
private:
	bool is_ssh;
	KSocket * sock;
	KSsh * ssh;
	KFlags * flags;

public:
	// =============================================================================
	// constructor
	// =============================================================================
	KCommUtils(KSocket * sock, KSsh * ssh, KFlags * flags, bool is_ssh)
	{
		ktrace_in( );
		ktrace( "KCommUtils::KCommUtils( sock )" );

		this->sock = sock;
		this->ssh = ssh;
		this->flags = flags;
		this->is_ssh = is_ssh;
		
	}

public:
	// =============================================================================
	// receive
	// =============================================================================
	bool Receive( std::string & buff )
	{
		ktrace_in( );
		ktrace( "KCommUtils::Receive( )" );

		buff.clear();

		bool ret;

		this->flags->Enable( "sock_io_timeout" );

		if( this->is_ssh ) 
		{
			ret = this->ssh->Pop( buff );
			if( !ret ) this->ssh->LogErrorMessage();
		}
		else if( this->sock->CanReceive( ) ) ret = this->sock->Receive( buff );
		else ret = true;

		this->flags->Disable( "sock_io_timeout" );

		return ret;
	}

public:
	// =============================================================================
	// send
	// =============================================================================
	bool Send( const std::string & buff )
	{
		ktrace_in( );
		ktrace( "KCommUtils::Send( )" );

		bool ret;

		this->flags->Enable( "sock_io_timeout" );

		if( this->is_ssh ) 
		{
			ret = this->ssh->Push( buff );
			if( !ret ) this->ssh->LogErrorMessage();
		}
		else ret = this->sock->Send( buff );

		this->flags->Disable( "sock_io_timeout" );

		return ret;
	}

public:
	// =============================================================================
	// send to channel / ssh only
	// =============================================================================
	bool Send( const std::string & buff, int channel )
	{
		ktrace_in( );
		ktrace( "KCommUtils::Send( )" );

		bool ret;

		this->flags->Enable( "sock_io_timeout" );

		ret = this->ssh->Push( buff, channel );

		this->flags->Disable( "sock_io_timeout" );

		if( !ret ) this->ssh->LogErrorMessage();

		return ret;
	}
public:
	// =============================================================================
	// connect
	// =============================================================================
	bool Connect( std::string server, u_short port )
	{
		ktrace_in( );
		ktrace( "KCommUtils::Connect( )" );

		if( this->is_ssh ) return false;

		this->flags->Enable( "sock_io_timeout" );

		SOCKET ret = this->sock->Connect( server, port );

		this->flags->Disable( "sock_io_timeout" );

		if( ret == INVALID_SOCKET ) return false;

		return true;
	}

public:
	// =============================================================================
	// close
	// =============================================================================
	void Close( bool full = false )
	{
		ktrace_in( );
		ktrace( "KCommUtils::Close( )" );

		this->flags->Enable( "sock_io_timeout" );

		if( this->is_ssh ) 
		{
			if(this->ssh != NULL) this->ssh->ShutDown( );
		}
		else 
		{
			if(this->sock != NULL) this->sock->Close( );
		}

		if( full ) KSocket::WSACleanup();

		this->flags->Disable( "sock_io_timeout" );
	}
};