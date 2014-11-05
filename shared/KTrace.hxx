#pragma once
#include <windows.h>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>

class KTrace
{
protected:
	/*==============================================================================
	 * vars
	 *=============================================================================*/
	enum Phase
	{
		  Constructor = 0
		, Normal
		, Destructor
	} phase;
private:
	std::string first;
	int defLevel;
public:
	bool remove;
private:
	/*==============================================================================
	 * critical section static variable
	 *=============================================================================*/
	static CRITICAL_SECTION & cs( )
	{
		static CRITICAL_SECTION _cs;
		static bool init = true;

		if( init )
		{
			init = false;
			InitializeCriticalSection( &_cs );
		}
		return( _cs );
	}
private:
	/*==============================================================================
	 * expand file string
	 * ?pid - processid
	 *=============================================================================*/
	static std::string ExpandString( const std::string & str1 )
	{
		std::string str = str1;
		if( str.find( "?pid" ) != std::string::npos )
		{
			std::stringstream s;
			s << GetCurrentProcessId( );
			str.replace( str.find( "?pid" ), 4, s.str( ) );
		}

		return( str );
	}
public:
	/*==============================================================================
	 * file name static variable
	 *=============================================================================*/
	static std::string TraceFile( const std::string & file = "" )
	{
		static std::string _file = "";

		if( file != "" ) 
		{
			_file = KTrace::ExpandString( file );
		}

		return( _file );
	}

public:
	/*==============================================================================
	 * file name static variable
	 *=============================================================================*/
	static std::string LogFile( const std::string & file = "" )
	{
		static std::string _file = "";

		if( file != "" ) _file = KTrace::ExpandString( file );

		return( _file );
	}

public:
	/*==============================================================================
	 * error file name static variable
	 *=============================================================================*/
	static std::string ErrorFile( const std::string & file = "" )
	{
		static std::string _file = "";

		if( file != "" ) _file = KTrace::ExpandString( file );
		return( _file );
	}

public:
	/*==============================================================================
	 * the trace level
	 *=============================================================================*/
	static int TraceLevel( int level = -1 )
	{
		static int _level = 0;

		if( level >= 0 ) _level = level;

		return( _level );
	}

private:
	/*==============================================================================
	 * remove thread data from static sets
	 *=============================================================================*/
	static void RemoveThreadData( )
	{
		KTrace::ThreadTraceLevel( -999 );
		KTrace::ThreadIndent( -999 );
	}

private:
	/*==============================================================================
	 * current thread level
	 *=============================================================================*/
	static int ThreadTraceLevel( int level = -1 )
	{
		EnterCriticalSection( &KTrace::cs( ) );

		static std::map< DWORD, int > _level;
		int i;
		if( level == -999 ) 
		{
			_level.erase( GetCurrentThreadId( ) );
			i = 0;
		}
		else
		{
			std::map< DWORD, int >::iterator l = _level.find( GetCurrentThreadId( ) );

			if( l == _level.end( ) )
			{
				_level[ GetCurrentThreadId( ) ] = KTrace::TraceLevel( );
			}

			if( level >= 0 ) _level[ GetCurrentThreadId( ) ] = level;

			i = _level[ GetCurrentThreadId( ) ];
		}
		LeaveCriticalSection( &KTrace::cs( ) );

		return( i );
	}

private:
	/*==============================================================================
	 * current thread indent level
	 *=============================================================================*/
	static int ThreadIndent( int indent_ = -1 )
	{
		EnterCriticalSection( &KTrace::cs( ) );

		static std::map< DWORD, int > _indent;
		int i;
		if( indent_ == -999 ) 
		{
			_indent.erase( GetCurrentThreadId( ) );
			i = 0;
		}
		else
		{
			std::map< DWORD, int >::iterator l = _indent.find( GetCurrentThreadId( ) );

			if( l == _indent.end( ) )
			{
				_indent[ GetCurrentThreadId( ) ] = 0;
			}

			if( indent_ >= 0 ) _indent[ GetCurrentThreadId( ) ] = indent_;

			i = _indent[ GetCurrentThreadId( ) ];
		}
		LeaveCriticalSection( &KTrace::cs( ) );

		return( i );
	}

private:
	/*==============================================================================
	 * trace history
	 *=============================================================================*/
	static std::string History( const std::string & str = "" )
	{
		static std::vector< std::string > _history;
		std::string history;

		EnterCriticalSection( &KTrace::cs( ) );

		if( _history.size( ) == 0 ) for( int i = 0; i < 10; i++ ) _history.push_back( "" );

		if( str != "" )
		{
			_history.erase( _history.begin( ) );
			_history.push_back( str );
		}
		else for( unsigned i = 0; i < _history.size( ); i++ ) history += _history[ i ];

		LeaveCriticalSection( &KTrace::cs( ) );

		return( history );
	}

public:
	/*==============================================================================
	 * constructor
	 *=============================================================================*/
	KTrace( )
	{
		this->remove = false;
		this->phase = Constructor;
		this->defLevel = KTrace::ThreadTraceLevel( );

		KTrace::ThreadIndent( KTrace::ThreadIndent( ) + 1 );
	}

private:
	/*==============================================================================
	 * get row beautifier
	 *=============================================================================*/
	std::string Beautify( int le )
	{
		std::stringstream s;
		SYSTEMTIME time;
		
		// GetSystemTime( &time );
		GetLocalTime( &time );
		s << std::setw( 5 ) << GetCurrentProcessId( ) << " :" << std::setw( 5 ) << GetCurrentThreadId( ) << " "
			<< time.wYear << "-" << std::setw( 2 ) << time.wMonth << "-" << std::setw( 2 ) << time.wDay << " " 
			<< std::setw( 2 ) << time.wHour << ":" << std::setw( 2 ) << time.wMinute << ":" << std::setw( 2 ) << time.wSecond
			<< " "  << std::setw( 3 ) << time.wMilliseconds << " :" << std::setw( 3 ) << le << ":\t";

		return( s.str( ) );
	}
public:
	/*==============================================================================
	 * check if can trace
	 *=============================================================================*/
	bool CanTrace( )
	{
		return( KTrace::TraceLevel( ) <= KTrace::ThreadTraceLevel( ) );
	}

public:
	/*==============================================================================
	 * trace function
	 *=============================================================================*/
	void Trace( const std::string & str1, int le )
	{

		if( !this->CanTrace( ) ) return;

		int indent = KTrace::ThreadIndent( );

		if( this->phase == Constructor
		||  this->phase == Destructor ) if( indent >0 ) indent--;

		std::string str = this->Beautify( le ) + std::string( indent, ' ' ) + str1;


		if( this->phase == Constructor )
		{
			this->first = str1;
			str += "{";
		}
		if( this->phase == Destructor ) str += "}";

		str += "\r\n";


		this->History( str );
		this->Write( str );

		this->phase = Normal;
	}

public:
	/*==============================================================================
	 * set level
	 * when the class gets out of scope the thread level will be restored to the
	 * original level of the class was creation.
	 *=============================================================================*/
	void SetLevel( int level )
	{
		KTrace::ThreadTraceLevel( level );
	}

private:
	/*==============================================================================
	 * write to file
	 *=============================================================================*/
	void Write( const std::string str )
	{
		if( KTrace::TraceFile( ) == "" ) return;

		EnterCriticalSection( &KTrace::cs( ) );

		std::ofstream f;

		f.open( KTrace::TraceFile( ).c_str( ), std::ios_base::app );

		if( f.good( ) )
		{
			f << str;
			f.close( );
		}

		LeaveCriticalSection( &KTrace::cs( ) );
	}

public:
	/*==============================================================================
	 * write to file
	 *=============================================================================*/
	void Log( const std::string str1 )
	{
		std::string str = this->Beautify( 0 ) + str1 + "\r\n";

		this->History( str );

		if( KTrace::LogFile( ) == "" ) return;

		EnterCriticalSection( &KTrace::cs( ) );

		std::ofstream f;

		f.open( KTrace::LogFile( ).c_str( ), std::ios_base::app );

		if( f.good( ) )
		{
			f << str;
			f.close( );
		}

		LeaveCriticalSection( &KTrace::cs( ) );
	}

public:
	/*==============================================================================
	 * write to file
	 *=============================================================================*/
	void Error( const std::string str1 )
	{
		std::string str = this->Beautify( 0 ) + str1 + "\r\n";
		this->History( str );

		if( KTrace::ErrorFile( ) == "" ) return;

		str = KTrace::History( );

		EnterCriticalSection( &KTrace::cs( ) );

		std::ofstream f;

		f.open( KTrace::ErrorFile( ).c_str( ), std::ios_base::app );

		if( f.good( ) )
		{
			f << str;
			f.close( );
		}

		LeaveCriticalSection( &KTrace::cs( ) );
	}

public:
	/*==============================================================================
	 * destructor
	 *=============================================================================*/
	~KTrace( )
	{
		this->phase = Destructor;
		this->Trace( "", 0 );
		KTrace::ThreadIndent( KTrace::ThreadIndent( ) - 1 );
		KTrace::ThreadTraceLevel( this->defLevel );

		if( this->remove ) KTrace::RemoveThreadData();
	}
};

#define USE( x )	( ( x ) = ( x ) )

// ktrace initial macro
#define ktrace_master_level( X ) KTrace::TraceLevel( X );
#define ktrace_trace_file( X ) KTrace::TraceFile( X );
#define ktrace_error_file( X ) KTrace::ErrorFile( X );
#define ktrace_log_file( X ) KTrace::LogFile( X );

#define ktrace_remove( ) __kts_t.remove = true;

// ktrace in macro
#define ktrace_in( ) \
	int __kts_le = 0; \
	KTrace __kts_t; \
	std::stringstream __kts_s;

// ktrace trace macro
#ifdef _ktrace
# define ktrace( X ) \
	if( __kts_t.CanTrace( ) ) \
	{ \
		__kts_le = GetLastError( ); \
		__kts_s.str( "" ); \
		__kts_s << X; \
		__kts_t.Trace( __kts_s.str( ), __kts_le ); \
	}
#else 
# define ktrace( X )	0;
#endif // _ktrace


// ktrace level
#define ktrace_level( X ) \
{ \
	__kts_t.SetLevel( X ); \
}

#define kerror( X ) \
{ \
	int le = GetLastError( ); \
	__kts_s.str( "" ); \
	__kts_s << X << " le = " << le; \
	__kts_t.Error( __kts_s.str( ) ); \
	klog( X ); \
}

#define klog( X ) \
{ \
		ktrace( X ); \
		__kts_s.str( "" ); \
		__kts_s << X; \
		__kts_t.Log( __kts_s.str( ) ); \
}


void static trans_func( unsigned int u, EXCEPTION_POINTERS* pExp )
{
	USE(pExp);

	KTrace __kts_t;
	std::stringstream __kts_s;

	__kts_s << "EXCEPTION[ " << u << " ]";
	__kts_t.Error( __kts_s.str( ) );
	__kts_t.Log( __kts_s.str( ) );

	throw "exception";
}

#define kbegin_try_block	try{ _set_se_translator( trans_func );


#define kend_try_block		}catch( ... ){kerror( "exception " );ExitProcess( 0 );}

