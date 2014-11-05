#pragma once

#include <windows.h>
#include <string>
#include "..\shared\KTrace.hxx"


class KPipe
{
private:
	/*============================================================================
	 * variables
	 *============================================================================*/
	HANDLE m_hReadPipe;
	HANDLE m_hWritePipe;
	std::string pipeName;
	int timeout;
	bool connected;
public:
	/*==============================================================================
	* create the server end of the pipe and wait for client to connect
	*=============================================================================*/
	bool Create( const std::string & strPipeName ){

		ktrace_in( );
		ktrace( "KPipe::Create( " << strPipeName << " )" );

		this->connected = false;
		this->pipeName = strPipeName;
		std::string strTmp;


	 
		strTmp = "\\\\.\\pipe\\KTS_sr_" + strPipeName;

		m_hReadPipe = CreateNamedPipe( strTmp.c_str( ), PIPE_ACCESS_INBOUND
			, PIPE_TYPE_BYTE|PIPE_WAIT,PIPE_UNLIMITED_INSTANCES, 1000, 1000, 10000, NULL );
		if( m_hReadPipe == INVALID_HANDLE_VALUE ){
			ktrace( "m_hReadPipe == INVALID_HANDLE_VALUE : " << GetLastError( ) );
			return( false );
		}

		strTmp = "\\\\.\\pipe\\KTS_sw_" + strPipeName;

		m_hWritePipe = CreateNamedPipe( strTmp.c_str( ), PIPE_ACCESS_OUTBOUND
			, PIPE_TYPE_BYTE|PIPE_WAIT,PIPE_UNLIMITED_INSTANCES, 1000, 1000, 10000, NULL );

		if( m_hWritePipe == INVALID_HANDLE_VALUE ){
			ktrace( "m_hWritePipe == INVALID_HANDLE_VALUE : err " << GetLastError );
			return( false );
		}

		return( true );
	}/* bool Create( const std::string & strPipeName ) */

public:
	/*==============================================================================
	* create the client end of pipe and connect to the server (for given time)
	*=============================================================================*/
	int Connect( const std::string & strPipeName, int sec )
	{
		ktrace_in( );
		ktrace( "KPipe::Connect( " << strPipeName << ", " << sec << " )" );

		if( sec < 0 ) return this->Connect( strPipeName );

		for(int i = 0; i < sec * 10; i++ )
		{
			if( this->Connect( strPipeName ) ) return true;

			Sleep( 100 );
		}

		return false;
	}

public:
	/*==============================================================================
	* create the client end of pipe and connect to the server
	*=============================================================================*/
	bool Connect( const std::string & strPipeName ){

		ktrace_in( );
		ktrace( "KPipe::Connect( " << strPipeName << " )" );
		
		this->pipeName = strPipeName;
		std::string strTmp;

		strTmp = "\\\\.\\pipe\\KTS_sr_" + strPipeName;

		m_hWritePipe = CreateFile ( strTmp.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );
		if( m_hWritePipe == INVALID_HANDLE_VALUE ){
			ktrace( "KPipe::Connect( ) : err m_hWritePipe" );

			return( false );
		}

		strTmp = "\\\\.\\pipe\\KTS_sw_" + strPipeName;

		m_hReadPipe = CreateFile ( strTmp.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );
		if( m_hReadPipe == INVALID_HANDLE_VALUE ){
			ktrace( "KPipe::Connect( ) : err m_hReadPipe" );

			return( false );
		}

		return( true );
	}

private:
	/*==============================================================================
	* timeout thread proc
	*=============================================================================*/
	static DWORD WINAPI TimeoutThreadProc(LPVOID lpParameter)
	{
		ktrace_in( );
		ktrace_remove();
		ktrace_level( 10 );

		kbegin_try_block;

		ktrace( "KPipe::TimeoutThreadProc( )" );

		KPipe * p = (KPipe *)lpParameter;

		for( int i = 0; i < p->timeout * 10; i++ )
		{
			if( p->connected ) return 0;
			Sleep(100);
		}
		if( !p->connected )
		{
			kerror("pipe connect timedout");
			KPipe p1;
			p1.Connect(p->pipeName);
			p1.Close();
		}

		return 0;
		kend_try_block;
	}
public:
	/*==============================================================================
	* wait for client connection (for a period in sec, for each internal pipe)
	*=============================================================================*/
	void Accept( int timeout )
	{
		// there's some nasty memory leak in here
		// stringstream /and/ seh are both leaking when used with threads
		// we will NOT use this method ever
		// wtf?

		ktrace_in( );
		ktrace( "KPipe::Accept( " << timeout << " )" );
		this->timeout = timeout;

		HANDLE thread = CreateThread( NULL, 0, KPipe::TimeoutThreadProc, this, 0, NULL);
		ConnectNamedPipe( m_hReadPipe, NULL );
		ConnectNamedPipe( m_hWritePipe, NULL );

		this->connected = true;

		WaitForSingleObject(thread, INFINITE);
		CloseHandle(thread);
	}

public:
	/*==============================================================================
	* wait for client connection
	*=============================================================================*/
	void Accept( void ){

		ktrace_in( );
		ktrace( "KPipe::Accept( )" );

		DWORD dwRead;
		OVERLAPPED olTmp;

		ZeroMemory( &olTmp, sizeof( OVERLAPPED ) );
		olTmp.hEvent = CreateEvent( NULL, TRUE, TRUE, NULL );
		ConnectNamedPipe( m_hReadPipe, &olTmp );

		if( GetLastError( ) != ERROR_PIPE_CONNECTED ){
			GetOverlappedResult( m_hReadPipe, &olTmp, &dwRead, TRUE );
		}
		CloseHandle( olTmp.hEvent );


		ZeroMemory( &olTmp, sizeof( OVERLAPPED ) );
		olTmp.hEvent = CreateEvent( NULL, TRUE, TRUE, NULL );
		ConnectNamedPipe( m_hWritePipe, &olTmp );

		if( GetLastError( ) != ERROR_PIPE_CONNECTED ){
			GetOverlappedResult( m_hWritePipe, &olTmp, &dwRead, TRUE );
		}
		CloseHandle( olTmp.hEvent );
	}

public:
	/*==============================================================================
	* peek read pipe
	*=============================================================================*/
	bool PeekData( OUT PDWORD pdwBytesToRead ){

		ktrace_in( );
		ktrace( "KPipe::PeekData( )" );

		*pdwBytesToRead = 0;
		if( !PeekNamedPipe( m_hReadPipe, NULL, 0, NULL, pdwBytesToRead, NULL) ){
			ktrace( "PeekNamedPipe( ) : err " << GetLastError( ) );

			return( false );
		}

		

		return( true );
	}

public:
	/*==============================================================================
	* read from pipe
	*=============================================================================*/
	bool Read( OUT std::string & s ){

		ktrace_in( );
		ktrace( "KPipe::Read( )" );

		int iLen=0;
		DWORD dwRead;
		std::string buff;
		
		if( !ReadFile( m_hReadPipe, &iLen, sizeof(int), &dwRead, NULL ) ){
			ktrace( "ReadFile( ) : err " << GetLastError( ) );

			return( false );
		}

		ktrace( "KPipe::Read iLen = " << iLen << " dwRead = " << dwRead );
		buff.resize(iLen,'\x00');

		if( !ReadFile( m_hReadPipe, ( void * )buff.c_str( ), iLen, &dwRead, NULL ) ){
			ktrace( "ReadFile2( ) : err " << GetLastError( ) );

			return( false );
		}

		s.assign( buff.c_str( ), iLen );

		

		return( true );
	}/* BOOL Read( OUT string &s ) */

public:
	/*==============================================================================
	* read from pipe Async
	*=============================================================================*/
	bool ReadA( OUT std::string & s ){

		ktrace_in( );
		ktrace( "KPipe::ReadA( )" );

		s = "";

		DWORD data;
		if( !this->PeekData( &data ) ) return false;
		if( data == 0 ) return true;

		return( this->Read( s ) );

	}/* bool ReadA( OUT std::string & s ) */

public:
	/*==============================================================================
	* write to pipe
	*=============================================================================*/
	bool Write( const std::string & s ){

		ktrace_in( );
		ktrace( "KPipe::Write( " << ( int )s.length( ) << " )" );

		DWORD dwWrite;
		int iLen;
		
		if( s.length( ) == 0 ){
			ktrace( "len == 0" );

			return( true );
		}

		iLen = ( int )s.length();

		if( !WriteFile( m_hWritePipe, &iLen, sizeof( int ), &dwWrite, NULL ) ){
			ktrace( "WriteFile( ) : err " << GetLastError( ) );
			
			return( false );
		}
		
		if( !WriteFile( m_hWritePipe, s.c_str(), iLen, &dwWrite, NULL ) ){
			ktrace( "WriteFile1( ) : err " << GetLastError( ) );

			return( false );
		}

		ktrace( "length = " << ( int )s.length( ) );
		return( true );

	}

public:
	/*==============================================================================
	* shut down pipes
	*=============================================================================*/
	void Close( void ){

		ktrace_in( );
		ktrace( "KPipe::Close( );" );

		if( m_hReadPipe != NULL ){
			DisconnectNamedPipe( m_hReadPipe );
			CancelIo( m_hReadPipe );
			CloseHandle( m_hReadPipe );
			m_hReadPipe = NULL;
		}

		if( m_hWritePipe != NULL ){
			DisconnectNamedPipe( m_hWritePipe );
			CancelIo( m_hWritePipe );
			CloseHandle( m_hWritePipe );
			m_hWritePipe = NULL;
		}
	}

public:
	/*==============================================================================
	* constructor
	*=============================================================================*/
	KPipe( void ){
		ktrace_in( );
		ktrace( "KPipe::KPipe( )" );

		m_hReadPipe = NULL;
		m_hWritePipe = NULL;
	}

public:
	/*==============================================================================
	* destructor
	*=============================================================================*/
	~KPipe( void ){

		ktrace_in( );
		ktrace( "KPipe::~KPipe( )" );

		Close( );
		
	}

};
