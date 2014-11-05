#pragma once

#include <windows.h>
#include <string>
#include <sstream>
#include "..\shared\KTrace.hxx"

#undef FOREGROUND_INTENSITY
#undef FOREGROUND_RED
#undef FOREGROUND_GREEN
#undef FOREGROUND_BLUE

#undef BACKGROUND_INTENSITY
#undef BACKGROUND_RED
#undef BACKGROUND_GREEN
#undef BACKGROUND_BLUE

class KConsole
{

public:
	/*==============================================================================
	 * color masks
	 *=============================================================================*/
	static const WORD FOREGROUND_INTENSITY = 0x0008;
	static const WORD FOREGROUND_RED = 0x0004;
	static const WORD FOREGROUND_GREEN = 0x0002;
	static const WORD FOREGROUND_BLUE = 0x0001;

	static const WORD BACKGROUND_INTENSITY = 0x0080;
	static const WORD BACKGROUND_RED = 0x0040;
	static const WORD BACKGROUND_GREEN = 0x0020;
	static const WORD BACKGROUND_BLUE = 0x0010;

	static const WORD FOREGROUND_MASK	= ( FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );
	static const WORD FOREGROUND_WHITE	= ( FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );
	static const WORD FOREGROUND_YELLOW	= ( FOREGROUND_GREEN | FOREGROUND_RED );
	static const WORD FOREGROUND_BLACK	= ( 0 );
	static const WORD FOREGROUND_CYAN	= ( FOREGROUND_BLUE | FOREGROUND_RED );
	static const WORD FOREGROUND_MAGENTA= ( FOREGROUND_BLUE | FOREGROUND_GREEN );

	static const WORD BACKGROUND_MASK	= ( BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED );
	static const WORD BACKGROUND_WHITE	= ( BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED );
	static const WORD BACKGROUND_YELLOW	= ( BACKGROUND_GREEN | BACKGROUND_RED );
	static const WORD BACKGROUND_BLACK	= ( 0 );
	static const WORD BACKGROUND_CYAN	= ( BACKGROUND_BLUE | BACKGROUND_RED );
	static const WORD BACKGROUND_MAGENTA= ( BACKGROUND_BLUE | BACKGROUND_GREEN );

private:
	/*==============================================================================
	 * handle variables
	 *=============================================================================*/
	HANDLE output;
	HANDLE input;
	HANDLE error;
	HWND window;

public:
	// =============================================================================
	// constructor
	// =============================================================================
	KConsole( )
	{
		ktrace_in( );
		ktrace( "KConsole::KConsole( )" );

		ZeroMemory( &output, sizeof( output ) );
		ZeroMemory( &input, sizeof( input ) );
		ZeroMemory( &error, sizeof( error ) );
		ZeroMemory( &window, sizeof( window ) );

	}

public:
	/*==============================================================================
	 * get screen size
	 *=============================================================================*/
	bool GetScreenSize( COORD & screen, CONSOLE_SCREEN_BUFFER_INFO * pcsbi = NULL )
	{
		ktrace_in( );
		ktrace( "KConsole::GetScreenSize( " << pcsbi << " )" );

		CONSOLE_SCREEN_BUFFER_INFO csbi;

		if( pcsbi ) csbi = *pcsbi;
		else if( !this->GetConsoleInfo( csbi ) )
		{
			ktrace( "GetConsoleInfo( ):err" );
			return( false );
		}

		screen.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		screen.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

		ktrace( "[ " << screen.X << ", " << screen.Y << " ]" );
		return( true );

	}

public:
	/*==============================================================================
	 * get buff size
	 *=============================================================================*/
	bool GetBuffSize( COORD & buff, CONSOLE_SCREEN_BUFFER_INFO * pcsbi = NULL )
	{
		ktrace_in( );
		ktrace( "KConsole::GetBuffSize( " << pcsbi << " )" );

		CONSOLE_SCREEN_BUFFER_INFO csbi;

		if( pcsbi ) csbi = *pcsbi;
		else if( !this->GetConsoleInfo( csbi ) )
		{
			ktrace( "GetConsoleInfo( ):err" );
			return( false );
		}

		buff.X = csbi.dwSize.X;
		buff.Y = csbi.dwSize.Y;

		ktrace( "[ " << buff.X << ", " << buff.Y << " ]" );
		return( true );
	}

public:
	/*==============================================================================
	 * set screen size
	 *=============================================================================*/
	bool SetScreenSize( COORD screen )
	{
		ktrace_in( )
		ktrace( "KConsole::SetScreenSize( " << screen.X << ", " << screen.Y << " )" );

		CONSOLE_SCREEN_BUFFER_INFO csbi;

		if( !this->GetConsoleInfo( csbi ) )
		{
			ktrace( "GetConsoleInfo( ):err" );
			return( false );
		}

		if( csbi.srWindow.Right - csbi.srWindow.Left + 1 == screen.X
		&&  csbi.srWindow.Bottom - csbi.srWindow.Top + 1 == screen.Y )
		{
			ktrace( "screen size ok" );
			return( true );
		}

		if( csbi.dwSize.X < screen.X || csbi.dwSize.Y < screen.Y )
		{
			if( csbi.dwSize.X < screen.X ) csbi.dwSize.X = screen.X;
			if( csbi.dwSize.Y < screen.Y ) csbi.dwSize.Y = screen.Y;

			if( !SetConsoleScreenBufferSize( GetOutputHandle( ), csbi.dwSize ) )
			{
				ktrace( "SetConsoleScreenBufferSize( ):err" );
				return( false );
			}
		}

		if( csbi.srWindow.Left + screen.X > csbi.dwSize.X )
		{
			csbi.srWindow.Right = csbi.dwSize.X - 1;
			csbi.srWindow.Left = csbi.dwSize.X - screen.X;
		}
		else
		{
			csbi.srWindow.Right = csbi.srWindow.Left + screen.X - 1;
		}


		if( csbi.srWindow.Top + screen.Y > csbi.dwSize.Y )
		{
			csbi.srWindow.Bottom = csbi.dwSize.Y - 1;
			csbi.srWindow.Top = csbi.dwSize.Y - screen.Y;
		}
		else
		{
			csbi.srWindow.Bottom = csbi.srWindow.Top + screen.Y - 1;
		}

		if( !SetConsoleWindowInfo( GetOutputHandle( ), true, &csbi.srWindow ) )
		{
			ktrace( "SetConsoleWindowInfo( ):err" );
			return( false );
		}

		return( true );
	}

public:
	/*==============================================================================
	 * set buff size
	 *=============================================================================*/
	bool SetBuffSize( COORD buff )
	{
		ktrace_in( );
		ktrace( "KConsole::SetBuffSize( " << buff.X << ", " << buff.Y << " )" );


		CONSOLE_SCREEN_BUFFER_INFO csbi;
		bool fixScreenSize = false;

		if( !this->GetConsoleInfo( csbi ) )
		{
			ktrace( "GetConsoleInfo( ):err" );
			return( false );
		}

		if( csbi.dwSize.X == buff.X && csbi.dwSize.Y == buff.Y )
		{
			ktrace( "buff size ok" );
			return( true );
		}

		if( buff.Y < csbi.srWindow.Bottom - csbi.srWindow.Top + 1 )
		{
			COORD tmp;

			fixScreenSize = true;

			tmp.X = csbi.dwSize.X;
			tmp.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

			if( !SetConsoleScreenBufferSize( GetOutputHandle( ), tmp ) )
			{
				ktrace( "SetConsoleScreenBufferSize( " << tmp.X << ", " << tmp.Y << " ):err" );
				return( false );
			}

			csbi.srWindow.Top = 0;
			csbi.srWindow.Bottom = buff.Y - 1;
		}

		if( buff.X < csbi.srWindow.Right - csbi.srWindow.Left + 1 )
		{
			fixScreenSize = true;

			csbi.srWindow.Right = buff.X - 1;
			csbi.srWindow.Left = 0;
		}

		if( fixScreenSize )
		{
			if( !SetConsoleWindowInfo( GetOutputHandle( ), true, &csbi.srWindow ) )
			{
				ktrace( "SetConsoleWindowInfo( "
					<< csbi.srWindow.Left << ", " << csbi.srWindow.Top << ", "
					<< csbi.srWindow.Right << ", " << csbi.srWindow.Bottom << " ) : err" );
				return( false );
			}
		}

		if( !SetConsoleScreenBufferSize( GetOutputHandle( ), buff ) )
		{

			ktrace( "SetConsoleScreenBufferSize2( ):err" );
			return( false );
		}

		return( true );
	}

public:
	/*==============================================================================
	 * get console info
	 *=============================================================================*/
	bool GetConsoleInfo( CONSOLE_SCREEN_BUFFER_INFO &csbi )
	{
		ktrace_in( );
		ktrace( "KConsole::GetConsoleInfo( )" );

		if( !GetConsoleScreenBufferInfo( this->GetOutputHandle( ), &csbi ) )
		{
			ktrace( "GetConsoleScreenBufferInfo( ):err" );
			return( false );
		}

		return( true );
	}


public:
	/*==============================================================================
	 * write in console string
	 *=============================================================================*/
	bool Write( std::string strText )
	{
		ktrace_in( );
		ktrace( "KConsole::Write( string[ " << strText << " ] )" );

		DWORD dw;

		if( !WriteConsole( this->GetOutputHandle( ), strText.c_str( ), ( DWORD )strText.length( ), &dw, NULL ) )
		{
			ktrace( "WriteConsole( ):err" );
			return( false );
		}

		return( true );
	}

public:
	/*==============================================================================
	 * write in console string unicode
	 *=============================================================================*/
	bool WriteW( std::wstring strText )
	{
		ktrace_in( );
		ktrace( "KConsole::WriteW( string[ " << strText.c_str( ) << " ] )" );

		DWORD dw;

		if( !WriteConsoleW( this->GetOutputHandle( ), strText.c_str( ), ( DWORD )strText.length( ), &dw, NULL ) )
		{
			ktrace( "WriteConsoleW( ):err" );
			return( false );
		}

		return( true );
	}

public:
	/*==============================================================================
	* write in console char
	*=============================================================================*/
	bool Write( char ch )
	{
		ktrace_in( );
		ktrace( "KConsole::Write( char[ " << ch << " ] )" );

		std::string str;

		str = ch;
		if( !this->Write( str ) )
		{
			ktrace( "Write : err" );
			return( false );
		}

		return( true );
	}

public:
	/*==============================================================================
	* write in console char unicode
	*=============================================================================*/
	bool WriteW( WCHAR ch )
	{
		ktrace_in( );
		ktrace( "KConsole::WriteW( WCHAR[ " << ch << " ] )" );

		std::wstring str;

		str = ch;
		if( !this->WriteW( str ) )
		{
			ktrace( "WriteW : err" );
			return( false );
		}

		return( true );
	}

public:
	/*==============================================================================
	 * read from console
	 *=============================================================================*/
	bool Read( std::string & str )
	{
		ktrace_in( );
		ktrace( "KConsole::Read( )" );

		char buff[ 255 ];
		unsigned long len;

		bool ret = ( TRUE == ReadConsole( this->GetInputHandle( ), buff, 250, &len, NULL ) );
		if( ret ) str.assign( buff, len - 1 );

		ktrace( "[ " << ret << " ] [ " << str << " ] " );

		return( ret );
	}

public:
	/*==============================================================================
	 * get cursor position
	 *=============================================================================*/
	bool GetCursor( COORD & pos, bool absolute = true, CONSOLE_SCREEN_BUFFER_INFO * pcsbi = NULL )
	{
		ktrace_in( );
		ktrace( "KConsole::GetCursor( " << absolute << ", " << pcsbi << " )" );

		CONSOLE_SCREEN_BUFFER_INFO csbi;

		if( pcsbi ) csbi = *pcsbi;
		else if( !this->GetConsoleInfo( csbi ) )
		{
			ktrace( "GetConsoleInfo( ):err" );
			return( false );
		}

		if( absolute )
		{
			pos.X = csbi.dwCursorPosition.X;
			pos.Y = csbi.dwCursorPosition.Y;
		}
		else
		{
			pos.X = csbi.dwCursorPosition.X - csbi.srWindow.Left;
			pos.Y = csbi.dwCursorPosition.Y - csbi.srWindow.Top;
		}

		ktrace( "[ " << pos.X << ", " << pos.Y << " ]" );
		return( true );
	}

public:
	/*==============================================================================
	 * set cursor position
	 *=============================================================================*/
	bool SetCursor( COORD pos, bool absolute = true )
	{
		ktrace_in( );
		ktrace( "KConsole::SetCursor( " << pos.X << ", " << pos.Y << ", " << absolute << " )" );

		CONSOLE_SCREEN_BUFFER_INFO csbi;

		if( !absolute )
		{
			if( !this->GetConsoleInfo( csbi ) )
			{
				ktrace( "GetConsoleInfo( ):err" );
				return( false );
			}
			pos.X = csbi.srWindow.Left + pos.X;
			pos.Y = csbi.srWindow.Top + pos.Y;
		}

		if( !SetConsoleCursorPosition( this->GetOutputHandle( ), pos ) )
		{
			ktrace( "SetConsoleCursorPosition( " << pos.X << ", " << pos.Y << " ):err" );
			return( false );
		}
		return( true );
	}

public:
	/*==============================================================================
	 * get console title
	 *=============================================================================*/
	bool GetTitle( std::string &title )
	{
		ktrace_in( );
		ktrace( "KConsole::GetTitle( )" );

		char buff[ 1000 + 1 ];

		if( !GetConsoleTitle( buff, 1000 ) )
		{
			ktrace( "GetConsoleTitle( ):err" );
			return( false );
		}

		title = buff;

		ktrace( "[ " << title << " ]" );
		return( true );
	}

public:
	/*==============================================================================
	 * set console title
	 *=============================================================================*/
	bool SetTitle( std::string title )
	{
		ktrace_in( );
		ktrace( "KConsole::SetTitle( " << title << " )" );

		if( !SetConsoleTitle( title.c_str( ) ) )
		{
			ktrace( "SetConsoleTitle( ):err" );
			return( false );
		}
		return( true );
	}

public:
	/*==============================================================================
	 * get output handle
	 *=============================================================================*/
	HANDLE GetOutputHandle( )
	{
		ktrace_in( );
		ktrace( "KConsole::GetOutputHandle( );" );

		if( !this->output ) this->output = GetStdHandle( STD_OUTPUT_HANDLE );

		return( this->output );
	}

public:
	/*==============================================================================
	* get input handle
	*=============================================================================*/
	HANDLE GetInputHandle( )
	{
		ktrace_in( );
		ktrace( "KConsole::GetInputHandle( );" );

		if( !this->input ) this->input = GetStdHandle( STD_INPUT_HANDLE );

		return( this->input );
	}


public:
	/*==============================================================================
	* get error handle
	*=============================================================================*/
	HANDLE GetErrorHandle( );


public:
	/*==============================================================================
	* get window handle
	*=============================================================================*/
	HWND GetWindowHandle( )
	{
		ktrace_in( );
		ktrace( "KConsole::GetWindowHandle( )" );

		if( this->window )
		{
			ktrace( "done handle ok" );
			return( this->window );
		}

		std::stringstream s;
		std::string str;

		s.str( "" );
		s << "KConsole::GetWindowHandle( " << GetCurrentProcessId( ) << " )";

		if( !this->GetTitle( str ) )
		{
			ktrace( "GetTitle( ):err" );
			return( NULL );
		}

		if( !this->SetTitle( s.str( ) ) )
		{
			ktrace( "SetTitle( ):err" );
			return( NULL );
		}

		for( int i = 0; i < 10; i++ )
		{
			this->window = FindWindow( NULL, s.str( ).c_str( ) );
			if( this->window ) break;
			Sleep( 100 );
		}

		if( !this->window )
		{
			ktrace( "FindWindow( ):err" );

			if( !this->SetTitle( str ) ) ktrace( "SetTitle( ):err" );

			return( NULL );
		}

		if( !this->SetTitle( str ) ) ktrace( "SetTitle : err" );

		ktrace( "[ " << this->window << " ]" );
		return( this->window );

	}

public:
	/*==============================================================================
	 * get a row from the output returns the number of chars in the row
	 *=============================================================================*/
	int GetOutputRow(  CHAR_INFO *pciRow,  SHORT iRowLen,  SHORT iRow, CONSOLE_SCREEN_BUFFER_INFO * pcsbi = NULL )
	{
		ktrace_in( );
		ktrace( "KConsole::GetOutputRow( " << iRow << ", " << pcsbi << " )" );

		COORD c1;
		COORD c2;
		SMALL_RECT sr;
		CONSOLE_SCREEN_BUFFER_INFO csbi;

		if( pcsbi ) csbi = *pcsbi;
		else if( !this->GetConsoleInfo( csbi ) )
		{
			ktrace( "GetConsoleInfo( ):err" );
			return( false );
		}

		if( csbi.srWindow.Right - csbi.srWindow.Left > iRowLen )
		{
			ktrace( "err row too wide" );
			return( false );
		}

		c1.X = iRowLen;
		c1.Y = 1;

		c2.X = 0;
		c2.Y = 0;

		sr.Left = csbi.srWindow.Left;
		sr.Right = csbi.srWindow.Right;
		sr.Top = iRow;
		sr.Bottom = iRow;

		if( !ReadConsoleOutput( GetOutputHandle( ), pciRow, c1, c2 , &sr ) )
		{
			ktrace( "ReadConsoleOutput( ):err" );
			return( false );
		}

		return( true );
	}

public:
	/*==============================================================================
	 * get a row from the output unicode
	 *=============================================================================*/
	bool GetOutputRowW(  CHAR_INFO *pciRow,  SHORT iRowLen,  SHORT iRow, CONSOLE_SCREEN_BUFFER_INFO * pcsbi = NULL )
	{
		ktrace_in( );
		ktrace( "KConsole::GetOutputRowW( " << iRow << ", " << pcsbi << " )" );

		COORD c1;
		COORD c2;
		SMALL_RECT sr;
		CONSOLE_SCREEN_BUFFER_INFO csbi;

		if( pcsbi ) csbi = *pcsbi;
		else if( !this->GetConsoleInfo( csbi ) )
		{
			ktrace( "GetConsoleInfo( ):err" );
			return( false );
		}

		if( csbi.srWindow.Right - csbi.srWindow.Left > iRowLen )
		{
			ktrace( "err row too wide" );
			return( false );
		}

		c1.X = iRowLen;
		c1.Y = 1;

		c2.X = 0;
		c2.Y = 0;

		sr.Left = csbi.srWindow.Left;
		sr.Right = csbi.srWindow.Right;
		sr.Top = iRow;
		sr.Bottom = iRow;

		if( !ReadConsoleOutputW( this->GetOutputHandle( ), pciRow, c1, c2 , &sr ) )
		{
			ktrace( "ReadConsoleOutputW( ):err" );
			return( false );
		}

		return( true );
	}

public:
	// =============================================================================
	// Check if console has scroll back
	// =============================================================================
	bool HasScroll( CONSOLE_SCREEN_BUFFER_INFO csbi )
	{
		ktrace_in( );
		ktrace( "KConsole::HasScroll( )" );

		if( csbi.srWindow.Bottom + 1 == csbi.dwSize.Y && csbi.srWindow.Top == 0 )
		{

			ktrace( "[ true ]" );
			return( false );
		}

		ktrace( "[ false ]" );
		return( true );
	}

public:
	// =============================================================================
	// Set text attribute
	// =============================================================================
	bool SetAttribute( WORD attr )
	{
		ktrace_in( );
		ktrace( "KConsole::SetAttribute( " << attr << " )" );

		bool ret = ( TRUE == SetConsoleTextAttribute( this->GetOutputHandle( ), attr ) );

		ktrace( "[ " << ret << " ]" );
		return( ret );
	}

public:
	/*==============================================================================
	 * set console mode
	 *=============================================================================*/
	bool SetConsoleMode( DWORD mode )
	{
		ktrace_in( );
		ktrace( "KConsole::SetConsoleMode( )" );

		bool ret = ( TRUE == ::SetConsoleMode( this->GetInputHandle( ), mode ) );

		ktrace( "[ " << ret << " ]" );
		return( ret );
	}
public:
	/*==============================================================================
	 * set console mode
	 *=============================================================================*/
	bool ReadKey( int & key )
	{
		ktrace_in( );
		ktrace( "KConsole::ReadKey( )" );

		INPUT_RECORD ir;
        DWORD dwRead;

//        FlushConsoleInputBuffer(hConIn); 

		while( true )
		{
			if( !ReadConsoleInput(this->GetInputHandle( ), &ir, 1, &dwRead) ) return false; 
			if ( ir.EventType == KEY_EVENT ) break;
		}

		key = ir.Event.KeyEvent.uChar.AsciiChar;
		return true;
	}
};
