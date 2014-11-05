#pragma once

#include <Winsock2.h>
#include <string>
#include "..\shared\KTrace.hxx"

class KSocket{
private:
	/*============================================================================
	 * variables
	 *============================================================================*/
	SOCKET m_sock;
public:
	/*============================================================================
	 * constructor
	 *============================================================================*/
	KSocket( SOCKET s )
	{

		ktrace_in( );
		ktrace( "KSocket::KSocket( " << ( int )s << " )" );

		m_sock = s;
		
		
		
	}/* KSocket( SOCKET s ) */

public:
	/*============================================================================
	 * constructor
	 *============================================================================*/
	KSocket( )
	{
		ktrace_in( );
		ktrace( "KSocket::KSocket( )" );

		m_sock = INVALID_SOCKET;

		
	}/* KSocket( ) */
public:
	/*============================================================================
	 * assignment
	 *============================================================================*/
	KSocket & operator = ( SOCKET s )
	{
		ktrace_in( );
		ktrace( "KSocket::operator =( SOCKET " << ( int )s << " )" );

		closesocket( m_sock );

		m_sock = s;

		
		return( *this );
	}/* KSocket & operator = ( SOCKET s ) */

public:
	/*============================================================================
	 * assignment
	 *============================================================================*/
	KSocket & operator = ( std::string str )
	{
		ktrace_in( );
		ktrace( "KSocket::operator =( string " << str << " )" );

		SOCKET sock;
		std::stringstream s;

		s.str( str );
		s >> sock;

		*this = sock;

		
		return( *this );
	}/* KSocket & operator = ( SOCKET s ) */
public:
	/*============================================================================
	 * assignment
	 *============================================================================*/
	KSocket & operator = ( KSocket k )
	{
		ktrace_in( );
		ktrace( "KSocket::operator =( KSocket " << ( int )k.GetSocket( ) << " )" );

		*this = k.GetSocket( );

		
		return( *this );
	}/* KSocket & operator = ( KSocket k ) */
	
public:
	/*============================================================================
	 * get socket descriptor
	 *============================================================================*/
	SOCKET GetSocket( )
	{
		return( m_sock );
	}/* SOCKET GetSocket( ) */
	
public:
	/*============================================================================
	 * init sockets
	 *============================================================================*/
	static int WSAStartup( )
	{

		ktrace_in( );
		ktrace( "KSocket::WSAStartup( )" );

		WSADATA	WSAData;
		int ret;
		
		ret = ::WSAStartup( MAKEWORD( 1, 1 ), &WSAData );
		if( ret != 0 ) kerror("can't start wsa:err " << ret);
		
		return( ret );
		
	}/* int WSAStartup( ) */

public:
	/*============================================================================
	 * clear sockets
	 *============================================================================*/
	static int WSACleanup( )
	{

		ktrace_in( );
		ktrace( "KSocket::WSACleanup( )" );

		int ret = ::WSACleanup();
		if( ret == 0 ) return 0;

		int error = WSAGetLastError( );
		
		if( error != WSANOTINITIALISED ) kerror("can't clean wsa:err " << error );
		
		return( ret );
		
	}/* int WSACleanup( ) */

public:
	/*============================================================================
	 * receive
	 *============================================================================*/
	bool Receive( OUT std::string & str )
	{
		ktrace_in( );
		ktrace( "KSocket::Receive( )" );

		int iBytesReceived;
		char buff[ 20005 ];
		
		str.clear();

		iBytesReceived = recv( m_sock, buff, 20000, 0 );

		ktrace( "iBytesReceived = " << iBytesReceived );

		if( iBytesReceived == SOCKET_ERROR  )
		{
			ktrace( "recv( ) : err " << WSAGetLastError( ) );
			return( false );
		}

		if( iBytesReceived == 0  )
		{
			ktrace( "recv( ) : socket down" );
			return( false );
		}

		str.assign( buff, iBytesReceived );

		
		return( true );
		
	}/* bool Receive( string & str ) */

public:
	/*============================================================================
	 * send
	 *============================================================================*/
	bool Send( std::string str )
	{
		ktrace_in( );
		ktrace( "KSocket::Send( " << ( unsigned int )str.length( ) << " )" );
		
		int iBytesSend;
		unsigned uTotalBytesSent;
		
		if( str.length( ) == 0 )
		{
			ktrace( "no data" );
			return( true );
		}

		uTotalBytesSent = 0;
		while( uTotalBytesSent < str.length( ) )
		{
			iBytesSend = send( m_sock, str.c_str( ) + uTotalBytesSent, ( int ) ( str.length( ) - uTotalBytesSent ), 0 );

			if( iBytesSend == SOCKET_ERROR )
			{
				ktrace( "send( ) : err " << WSAGetLastError( ) );
				return( false );
			}
			uTotalBytesSent += iBytesSend;
		}

		
		return( true );
		
	}/* bool Send( std::string & str ) */

public:
	/*============================================================================
	 * check if can receive
	 *============================================================================*/
	int CanReceive( )
	{
		ktrace_in( );
		ktrace( "KSocket::CanReceive( )" );
		
		int iRet;
		timeval timeout;
		fd_set readfds;

		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		FD_ZERO( &readfds );
		FD_SET( m_sock, &readfds );

		iRet = select( 0, &readfds, NULL, NULL, &timeout );

		FD_CLR(m_sock, &readfds);

		ktrace( "select1( ) = "<< iRet );

		if( iRet == 0 )
		{
			fd_set excpfds;

			FD_ZERO( &excpfds );
			FD_SET( m_sock, &excpfds );

			iRet = select( 0, NULL, NULL, &excpfds, &timeout );

			FD_CLR(m_sock, &excpfds);

			ktrace( "select2( ) = "<< iRet );
		}
		return( iRet );

	}/* int CanReceive( ) */
public:
	/*============================================================================
	 * create listening socket
	 *============================================================================*/
	bool Listen( int iPort, std::string ip = "" )
	{
		ktrace_in( );
		ktrace( "KSocket::Listen( " << iPort << ", " << ip << " )" );

		SOCKADDR_IN	sin;
		int iRet;
		
		sin.sin_family = AF_INET;
		sin.sin_port = htons( ( u_short )iPort );

		if( ip == "" )
		{
			sin.sin_addr.s_addr = htonl( INADDR_ANY );
		}
		else
		{
			sin.sin_addr.s_addr = inet_addr( ip.c_str() );
		}
	
		m_sock = socket( AF_INET, SOCK_STREAM, 0 );
		if( m_sock == INVALID_SOCKET )
		{
			ktrace( "socket( ) : err " << WSAGetLastError( ) );
			return( false );
		}
	
		iRet = bind( m_sock, ( SOCKADDR * )&sin, sizeof( SOCKADDR_IN ) );
		if( iRet == SOCKET_ERROR )
		{
			ktrace( "bind( ) : err " << WSAGetLastError( ) );
			this->Close( false );
			return( false );
		}

		iRet = listen( m_sock, SOMAXCONN );
		if( iRet == SOCKET_ERROR )
		{
			ktrace( "listen( ) : err " << WSAGetLastError( ) );
			this->Close( false );
			return( false );
		}
		
		
		return( true );
	}/* bool Listen( int iPort ) */

public:
	/*============================================================================
	 * connect socket to server and port
	 *============================================================================*/
	SOCKET Connect( std::string server, u_short port )
	{
		ktrace_in( );
		ktrace( "KSocket::Connect( " << server << ", " << port << " )" );

		struct hostent *hostPtr = NULL;
		struct sockaddr_in serverName = { 0 };

		this->m_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

		if( this->m_sock == INVALID_SOCKET  )
		{
			ktrace( "accept( ) : err " << WSAGetLastError( ) );
			return( this->m_sock );
		}

		/*
		* need to resolve the remote server name or 
		* IP address */
		hostPtr = gethostbyname( server.c_str() );
		if( NULL == hostPtr )
		{
			ktrace( "gethostbyname( ) : err " << WSAGetLastError( ) );

			hostPtr = gethostbyaddr( server.c_str(), (int)strlen( server.c_str() ), AF_INET );
			if( NULL == hostPtr )
			{
				ktrace( "gethostbyaddr( ) : err " << WSAGetLastError( ) );
				closesocket( this->m_sock );
				this->m_sock = INVALID_SOCKET;
				return( this->m_sock );
			}
		}

		serverName.sin_family = AF_INET;
		serverName.sin_port = htons( port );
		memcpy( &serverName.sin_addr, hostPtr->h_addr, hostPtr->h_length );

		if( connect( this->m_sock, (struct sockaddr*) &serverName, sizeof(serverName) ) != 0 )
		{
				ktrace( "connect( ) : err " << WSAGetLastError( ) );
				closesocket( this->m_sock );
				this->m_sock = INVALID_SOCKET;
				return( this->m_sock );
		}

		return( this->m_sock );
	}
public:
	/*============================================================================
	 * accept
	 *============================================================================*/
	SOCKET Accept( )
	{
		ktrace_in( );
		ktrace( "KSocket::Accept( )" );

		SOCKET s;

		s = accept( m_sock, NULL, NULL );
		if( s == SOCKET_ERROR )
		{
			ktrace( "accept( ) : err " << WSAGetLastError( ) );
		}

		
		return( s );
	}/* SOCKET Accept( ) */
public:
	/*============================================================================
	 * close connection
	 *============================================================================*/
	void Close( bool bGracefully = true )
	{
		ktrace_in( );
		ktrace( "KSocket::Close( " << bGracefully << " )" );

		char buff[ 1010 ];

		if( bGracefully )
		{
			shutdown( m_sock, 1/*SD_SEND*/ );
			while( recv( m_sock, buff, 1000, 0 ) > 0 );
		}
		closesocket( m_sock );
	}
public:
	/*============================================================================
	 * return connection ip
	 *============================================================================*/
	std::string GetConnectionIP( )
	{
		ktrace_in( );
		ktrace( "KSocket::GetConnectionIP( )" );

		if( this->m_sock == INVALID_SOCKET ) return "INVALID_SOCKET";

		int iSinLen=0;
		SOCKADDR_IN	sinPeer;
		std::stringstream s;

		iSinLen = sizeof( SOCKADDR_IN );
		getpeername( this->m_sock, ( SOCKADDR * )&sinPeer, &iSinLen );
		s << inet_ntoa( sinPeer.sin_addr );

		ktrace( "s = " << s.str( ) );
		return( s.str( ) );

	}

public:
	/*============================================================================
	 * return connection port
	 *============================================================================*/
	std::string GetConnectionPort( )
	{
		ktrace_in( );
		ktrace( "KSocket::GetConnectionPort( )" );

		if( this->m_sock == INVALID_SOCKET ) return "INVALID_SOCKET";

		int iSinLen=0;
		SOCKADDR_IN	sinPeer;
		std::stringstream s;

		iSinLen = sizeof( SOCKADDR_IN );
		getpeername( this->m_sock, ( SOCKADDR * )&sinPeer, &iSinLen );
		s << ntohs( sinPeer.sin_port );

		ktrace( "s = " << s.str( ) );
		return( s.str( ) );

	}
	
};

