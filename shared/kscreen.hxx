#pragma once

#include "..\shared\KTrace.hxx"
#include "..\shared\KConsole.hxx"

class KScreen
{
public:
	WCHAR replacedChar;

private:
	const static SHORT buff_width = 200;
	const static SHORT buff_height = 500;
	const static WCHAR magic_mark = 0x2663;

	KConsole con;

public:
	SMALL_RECT screen;
	SMALL_RECT buff;
	COORD cursor;

	CHAR_INFO data[ KScreen::buff_height ][ KScreen::buff_width ];

	CONSOLE_SCREEN_BUFFER_INFO csbi;

public:
	/*==============================================================================
	 * constructor
	 *=============================================================================*/
	KScreen( )
	{
		ktrace_in( );
		ktrace( "KScreen::KScreen( )" );

		ZeroMemory( &screen, sizeof( screen ) );
		ZeroMemory( &buff, sizeof( buff ) );
		ZeroMemory( &cursor, sizeof( cursor ) );
		ZeroMemory( &data, sizeof( data ) );
		ZeroMemory( &csbi, sizeof( csbi ) );
	}

private:
	/*==============================================================================
	 * set/get replaced char value
	 *=============================================================================*/
	static WCHAR replaced_char( WCHAR set = KScreen::magic_mark )
	{
		static WCHAR ch = 0;

		if( set == KScreen::magic_mark ) return( ch );
		return( ch = set );
	}

private:
	/*==============================================================================
	 * convert console coord to internal data coord
	 *=============================================================================*/
	SHORT to_internal( int i )
	{
		return(SHORT)( i - ( this->csbi.srWindow.Bottom - ( KScreen::buff_height - 1 ) ) );
	}

private:
	/*==============================================================================
	 * set the magic mark
	 *=============================================================================*/
	bool SetMagicMark( )
	{
		ktrace_in( );
		ktrace_level( 0 );

		ktrace( "KScreen::SetMagicMark( )" );

		CONSOLE_SCREEN_BUFFER_INFO csbi;
		CHAR_INFO ci[ KScreen::buff_width ];

		if( !this->con.GetConsoleInfo( csbi ) )
		{
			ktrace( "this->con.GetConsoleInfo( ):err" );
			return( false );
		}

		if( !this->con.HasScroll( csbi ) ) 
		{
			ktrace( "mark not needed" );
			return( true );
		}

		if( !this->con.GetOutputRowW( ci, KScreen::buff_width, csbi.srWindow.Top, &csbi ) )
		{
			ktrace( "this->con.GetOutputRowW( ):err" );
			return( false );
		}

		COORD cursor = { 0, csbi.srWindow.Top };
		if( !this->con.SetCursor( cursor ) )
		{
			ktrace( "this->con.SetCursor( ):err" );
			return( false );
		}

		if( !this->con.WriteW( KScreen::magic_mark ) )
		{
			ktrace( "this->con.WriteW( ):err" );
			return( false );
		}

		if( !this->con.SetCursor( csbi.dwCursorPosition ) )
		{
			ktrace( "this->con.SetCursor( ):err" );
			return( false );
		}

		// save replaced char
		KScreen::replaced_char( ci[ 0 ].Char.UnicodeChar );

		return( true );
	}
private:
	/*==============================================================================
	 * fetch the screen
	 *=============================================================================*/
	bool FetchScreen( )
	{
		ktrace_in( );
		ktrace_level( 0 );

		ktrace( "KScreen::Fetch( )" );

		SHORT rowcnt = 0;

		if( !this->con.GetConsoleInfo( this->csbi ) )
		{
			ktrace( "this->con.GetConsoleInfo( ):err" );
			return( false );
		}

		this->buff.Left = this->csbi.srWindow.Left;
		this->buff.Right = this->csbi.srWindow.Right;
		this->buff.Bottom = KScreen::buff_height - 1;
		this->buff.Top = 0;

		for( SHORT i = this->csbi.srWindow.Bottom; this->to_internal( i ) >= 0 && i >=0 ; i-- )
		{
			rowcnt++;

			if( !this->con.GetOutputRowW( this->data[ this->to_internal( i ) ], KScreen::buff_width, i, &this->csbi ) )
			{
				ktrace( "this->con.GetOutputRowW( " << i << " ):err" );
				return( false );
			}

			if( i <= this->csbi.srWindow.Top
			&&  this->data[ this->to_internal( i ) ][ 0 ].Char.UnicodeChar == KScreen::magic_mark )
			{
				this->buff.Top = this->to_internal( i );
				this->data[ this->buff.Top ][ 0 ].Char.UnicodeChar = KScreen::replaced_char( );
				break;
			}

		}

		this->buff.Top = this->buff.Bottom - rowcnt + 1;


		if( !this->SetMagicMark( ) )
		{
			ktrace( "this->SetMagicMark( ):err" );
			return( false );
		}

		// fill data coords
		this->cursor.X = this->csbi.dwCursorPosition.X;
		this->cursor.Y = this->to_internal( this->csbi.dwCursorPosition.Y );

		this->screen.Left = this->csbi.srWindow.Left;
		this->screen.Right = this->csbi.srWindow.Right;
		this->screen.Top = this->to_internal( this->csbi.srWindow.Top );
		this->screen.Bottom = this->to_internal( this->csbi.srWindow.Bottom );

		return( true );
	}

public:
	/*==============================================================================
	 * refresh the screen
	 *=============================================================================*/
	bool Refresh( )
	{
		ktrace_in( );
		ktrace_level( 0 );

		ktrace( "KScreen::Refresh( )" );

		if( !this->FetchScreen( ) )
		{
			ktrace( "this->FetchScreen( ):err" );
			return( false );
		}

		return( true );
	}

};