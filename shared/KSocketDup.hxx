#pragma once

#include <Winsock2.h>
#include "..\shared\KPipe.hxx"
#include "..\shared\KTrace.hxx"

/*==============================================================================
 * duplicate process between 2 running processes
 *=============================================================================*/
class KSocketDup
{
public:
	/*==============================================================================
	 * client pid
	 *=============================================================================*/
	DWORD pid;

private:
	/*==============================================================================
	 * fil the data with the string content
	 *=============================================================================*/
	void string2data( const std::string & s, char * d, unsigned l )
	{
		for( unsigned i = 0; i < l; i++ )
		{
			d[ i ] = s[ i ];
		}
	}
	// void string2data( const std::string & s, char * d, unsigned l )

private:
	/*==============================================================================
	 * fil the string with the data content
	 *=============================================================================*/
	void data2string( std::string & s, char * d, unsigned l )
	{
		s.clear( );
		for( unsigned i = 0; i < l; i++ )
		{
			s.push_back( d[ i ] );
		}
	}
	// void data2string( std::string & s, char * d, unsigned l )

public:
	/*==============================================================================
	 * send the socket to child process
	 *=============================================================================*/
	bool SendSocket( SOCKET s, const std::string & pipeName )
	{
		ktrace_in( );
		ktrace( "KSocketDup::SendSocket( " << ( int )s << ", " << pipeName << " )" );

		WSAPROTOCOL_INFO pi;
		DWORD pid;
		std::string buff;
		KPipe pipe;

		if( !pipe.Connect( pipeName, 30 ) )
		{
			ktrace( "pipe.Connect( " << pipeName << " ):err" );
			return( false );
		}

		if( !pipe.Read( buff ) )
		{
			ktrace( "pipe.Read( " << pipeName << " ):err" );
			return( false );
		}

		this->string2data( buff, ( char * )&pid, sizeof( pid ) );
		ktrace( "pid = " << pid );
		this->pid = pid;

		if( WSADuplicateSocket( s, pid, &pi ) != 0 )
		{
			ktrace( "WSADuplicateSocket( ):err" );
			return( false );
		}

		closesocket( s );

		this->data2string( buff, ( char * )&pi, sizeof( pi ) );

		if( !pipe.Write( buff ) )
		{
			ktrace( "pipe.Write( " << pipeName << " ):err" );
			return( false );
		}

		pipe.Read( buff );
		pipe.Close( );

		return( true );
	}

public:
	/*==============================================================================
	 * get the socket ( from the child process )
	 *=============================================================================*/
	SOCKET GetSocket( const std::string & pipeName )
	{
		ktrace_in( );
		ktrace( "KSocketDup::GetSocket( " << pipeName << " )" );

		DWORD pid = GetCurrentProcessId( );
		KPipe pipe;
		std::string buff;
		WSAPROTOCOL_INFO pi;
		SOCKET s;

		if( !pipe.Create( pipeName ) )
		{
			ktrace( "pipe.Create( " << pipeName << " ):err" );
			return( INVALID_SOCKET );
		}

		pipe.Accept( );

		ktrace( "pid = " << pid );
		this->data2string( buff, ( char * )&pid, sizeof( pid ) );

		if( !pipe.Write( buff ) )
		{
			ktrace( "pipe.Write( " << pipeName << " ):err" );
			return( INVALID_SOCKET );
		}


		if( !pipe.Read( buff ) )
		{
			ktrace( "pipe.Read( " << pipeName << " ):err" );
			return( INVALID_SOCKET );
		}

		pipe.Write( "." );
		pipe.Close( );

		this->string2data( buff, ( char * )&pi, sizeof( pi ) );

		s = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, &pi, 0, 0 );
		if( s == INVALID_SOCKET )
		{
			ktrace( "WSASocket( ):err" );
			return( INVALID_SOCKET );
		}

		return( s );
	}
};