#pragma once
#include "..\shared\KTrace.hxx"
#include "..\shared\KScreen.hxx"
#include "..\shared\KTelnet.hxx"

class KScreenExport
{
public:
	bool dumb_client;

private:
	/*==============================================================================
	 * vars
	 *=============================================================================*/
	KTelnet & telnet;
	COORD & screen;
	KScreen nscreen;
	KScreen oscreen;
	bool mismatch;

public:
	/*==============================================================================
	 * constructor
	 *=============================================================================*/
	KScreenExport( KTelnet & telnet1 ) : telnet( telnet1 ), screen( telnet1.screen )
	{
		ktrace_in( );

		ktrace( "KScreenExport::KScreenExport( ... )" );

		this->mismatch = false;

	}
	// KScreenExport( )

private:
	/*==============================================================================
	 * fix nscreen to reflect client screen
	 *=============================================================================*/
	std::string ClientServerMismatch( KScreen & nscreen )
	{
		if( this->screen.X - 1 == nscreen.screen.Right
		&&  this->screen.Y - 1 == nscreen.screen.Bottom - nscreen.screen.Top ) return( "." );

		if( nscreen.screen.Right > this->screen.X - 1 ) 
		{
				this->nscreen.screen.Right = this->screen.X - 1;
				this->nscreen.screen.Left = 0;
		}


		if( nscreen.cursor.X > this->screen.X - 1 ) nscreen.cursor.X = 0;
		if( nscreen.cursor.Y - nscreen.screen.Top > this->screen.Y - 1 ) nscreen.cursor.Y = 0;

		if( nscreen.screen.Bottom - nscreen.screen.Top > this->screen.Y - 1 )
		{
			if( nscreen.screen.Top == nscreen.buff.Top && nscreen.screen.Bottom == nscreen.buff.Bottom )
			{
				// no scroll back so fix the screen
				nscreen.screen.Bottom = nscreen.screen.Top + this->screen.Y - 1;
				nscreen.buff.Bottom = nscreen.screen.Top + this->screen.Y - 1;
			}
		}

		// mismatch
		if( this->screen.X - 1 != this->nscreen.screen.Right - this->nscreen.screen.Left
		||  this->screen.Y - 1 != this->nscreen.screen.Bottom - this->nscreen.screen.Top )
		{

			CONSOLE_SCREEN_BUFFER_INFO csbi;
			KConsole con;
			con.GetConsoleInfo( csbi );
			if( con.HasScroll( csbi ) )
			{
				if( this->mismatch ) return( "" );
				this->mismatch = true;
				return( this->telnet.Cls( KConsole::BACKGROUND_RED | KConsole::FOREGROUND_WHITE | KConsole::FOREGROUND_INTENSITY )
					    + this->telnet.GotoXY( 20, this->screen.Y / 2 )
						+ "KTS error: client screen mismatch" );
			}
		}

		return( "." );
	}

private:
	/*==============================================================================
	 * check if needs flat screen export1
	 *=============================================================================*/
	bool NeedFlatScreen( const KScreen & nscreen, const KScreen & oscreen )
	{
		ktrace_in( );

		ktrace( "KScreenExport::NeedFlatScreen( )" );

		// client server mismatch
		if( this->mismatch )
		{
			this->mismatch = false;
			return( true );
		}

		// whole buff
		if( nscreen.buff.Top == 0 )
		{
			ktrace( "[ true ] - whole buff" );
			return( true );
		}

		// screen size differ
		if( ( nscreen.screen.Bottom - nscreen.screen.Top ) != ( oscreen.screen.Bottom - oscreen.screen.Top ) )
		{
			ktrace( "[ true ] - screen differ" );
			return( true );
		}

		// screen width differ
		if( ( nscreen.screen.Right - nscreen.screen.Left ) != ( oscreen.screen.Right - oscreen.screen.Left ) )
		{
			ktrace( "[ true ] - screen width differ" );
			return( true );
		}

		// screen top
		if( nscreen.screen.Top < oscreen.screen.Top )
		{
			ktrace( "[ true ] - screen top" );
			return( true );
		}

		KConsole console;
		// scrollback
		if( console.HasScroll( nscreen.csbi ) != console.HasScroll( oscreen.csbi ) )
		{
			ktrace( "[ true ] - scrollback differ" );
			return( true );
		}

		ktrace( "[ false ]" );
		return( false );
		
	}
	// bool NeedFlatScreen( const KScreen & nscreen, const KScreen & oscreen )

private:
	/*==============================================================================
	 * get index of tje last visible char of the row
	 *=============================================================================*/
	int GetLastVisible( const KScreen & nscreen, int row )
	{
		std::wstring blank = L" \t\r\n";
		blank += std::wstring( 0, 1 );

		int last;
		for( last = nscreen.screen.Right; last >= 0; last-- )
		{
			if( blank.find( nscreen.data[ row ][ last ].Char.UnicodeChar ) == std::wstring::npos
			|| nscreen.data[ row ][ last ].Attributes != nscreen.csbi.wAttributes ) break;
		}
		return( last );
	}
	// int GetLastVisible( const KScreen & nscreen, int row )

private:
	/*==============================================================================
	 * convert row to oscreen "screen" coords
	 *=============================================================================*/
	int to_oscreen( const KScreen & nscreen, const KScreen & oscreen, int row )
	{
		return( oscreen.screen.Top + ( row - nscreen.buff.Top ) );
	}

private:
	/*==============================================================================
	 * convert row to nscreen "screen" coords
	 *=============================================================================*/
	SHORT to_nscreen( const KScreen & nscreen, const KScreen & oscreen, SHORT row )
	{
		SHORT ret = nscreen.buff.Top + ( row - oscreen.buff.Top );
		if( ret > nscreen.buff.Bottom ) ret = nscreen.buff.Bottom;
		return( ret );
	}

private:
	/*==============================================================================
	 * check if "chars" differ
	 *=============================================================================*/
	bool char_differ( const CHAR_INFO & nci, const CHAR_INFO & oci )
	{
		return( nci.Char.UnicodeChar != oci.Char.UnicodeChar || nci.Attributes != oci.Attributes );
	}

private:
	/*==============================================================================
	 * get row difference
	 *=============================================================================*/
	std::string DiffRow( const KScreen & nscreen, const KScreen & oscreen, int row, int & col )
	{
		ktrace_in( );

		ktrace( "KScreenExport::DiffRow( )"  );

		int oldrow = this->to_oscreen( nscreen, oscreen, row );
		int last = nscreen.screen.Right;

		if( this->dumb_client && row == nscreen.buff.Bottom ) last--;

		int from = -1;
		int to = -2;
		for( int i = 0; i <= last; i++ )
		{
			if( this->char_differ( nscreen.data[ row ][ i ], oscreen.data[ oldrow ][ i ] ) )
			{
				if( from == -1 ) from = i;
				to = i;
			}
		}

		col = from;
		if( oldrow > oscreen.cursor.Y )
		{
			from = 0;
			col = -1;
		}

		std::string export1;
		for( int i = from; i <= to; i++ )
		{
			// !!!
			export1 += this->telnet.Color( nscreen.data[ row ][ i ].Attributes );
			export1 += this->telnet.Encode( nscreen.data[ row ][ i ].Char.UnicodeChar );
		}


		return( export1 );
	}

private:
	/*==============================================================================
	 * get the difference btw screens
	 *=============================================================================*/
	std::string DiffScreen( const KScreen & nscreen, const KScreen & oscreen )
	{
		ktrace_in( );
		
		ktrace( "KScreenExport::DiffScreen( )" );

		COORD cursor;
		std::string export1;
		int col;

		cursor.X = oscreen.cursor.X;
		cursor.Y = this->to_nscreen( nscreen, oscreen, oscreen.cursor.Y );

		// new        old
		// -----      -----
		//  5          
		//  6          4
		// [7  ]      [5  ]
		// [8  ]      [6  ]
		// [9  ]      [7  ]

		// export1 difference
		SHORT i;
		for( i = nscreen.buff.Top; this->to_oscreen( nscreen, oscreen, i ) <= oscreen.buff.Bottom; i++  )
		{
			std::string tmp = this->DiffRow( nscreen, oscreen, i, col );

			if( tmp != "" )
			{
				// we have position
				if( col >= 0 )
				{
					// set posision it differs the oscreen cursor
					if( col != cursor.X || this->to_oscreen( nscreen, oscreen, i ) != cursor.Y )
					{
						export1 += this->telnet.GotoXY( col, i - nscreen.buff.Top );
					}
					export1 += tmp;
					cursor.X = ( SHORT )( col + tmp.length( ) );
				}
				else
				{
					for( int j = cursor.Y; j < i; j++ ) export1 += this->telnet.CrLf( );
					export1 += tmp;
					cursor.X = ( SHORT )tmp.length( );
				}

				cursor.Y = i;
			}
		}

		// export1 flat screen
		export1 += this->FlatScreen( nscreen, i, nscreen.buff.Bottom, &cursor );

		// fix cursor pos
		if( nscreen.cursor.X != cursor.X || nscreen.cursor.Y != cursor.Y )
		{
			if( nscreen.cursor.Y - nscreen.buff.Top >= this->screen.Y && nscreen.cursor.X == 0 )
			{
				export1 += "\n";
//				for( int i = 0; i <= nscreen.cursor.Y - nscreen.buff.Top - this->screen.Y; i++ ) export1 += "\n";
//				export1 += this->telnet.GotoXY( nscreen.cursor.X, this->screen.Y - 1 );
			}
			else
				export1 += this->telnet.GotoXY( nscreen.cursor.X, nscreen.cursor.Y - nscreen.buff.Top );
		}

		// fix cursor pos on blank screen
		if( export1 == "" )
		{
			if( nscreen.cursor.X != oscreen.cursor.X || nscreen.cursor.Y != oscreen.cursor.Y )
			{
				export1 += this->telnet.GotoXY( nscreen.cursor.X, nscreen.cursor.Y - nscreen.buff.Top );
			}
		}
		else export1 += this->telnet.GotoXY( nscreen.cursor.X, nscreen.cursor.Y - nscreen.buff.Top );

		return( export1 );
	}

private:
	/*==============================================================================
	 * export1 a flat screen
	 *=============================================================================*/
	std::string FlatScreen( const KScreen & nscreen, SHORT top = -1, int bottom = -1, COORD * pcursor = 0 )
	{
		ktrace_in( );

		ktrace( "KScreenExport::FlatScreen( )"  );

		std::string export1;
		COORD cursor = { 0, 0 };

		if( top == -1 && bottom == -1 )
		{
			// we export1 the entire screen
			export1 += this->telnet.Cls( nscreen.csbi.wAttributes );
			top = nscreen.buff.Top;
			bottom = nscreen.buff.Bottom;
		}


		cursor.Y = top;
		for( SHORT i = top; i <= bottom; i++ )
		{
			std::string tmp;
			for( int j = 0; j <= this->GetLastVisible( nscreen, i ); j++ )
			{
				// !!!
				tmp += this->telnet.Color( nscreen.data[ i ][ j ].Attributes );
				tmp += this->telnet.Encode( nscreen.data[ i ][ j ].Char.UnicodeChar );
			}

			if( tmp != "" )
			{
				for( int j = cursor.Y; j < i; j++ ) export1 += this->telnet.CrLf( );
				export1 += tmp;
				cursor.X = ( SHORT )tmp.length( );
				cursor.Y = i;
			}
		}

		if( export1 != "" && pcursor != 0 ) *pcursor = cursor;
		if( export1 != "" )
		{
			export1 = this->telnet.CrLf( ) + export1;
			if( pcursor == 0 )
			{
				// fix cursor pos
				if( nscreen.cursor.X != cursor.X || nscreen.cursor.Y != cursor.Y )
				{
					if( nscreen.cursor.Y - nscreen.buff.Top >= this->screen.Y && nscreen.cursor.X == 0 )
					{
						export1 += "\n";
					}
					else
						export1 += this->telnet.GotoXY( nscreen.cursor.X, nscreen.cursor.Y - nscreen.buff.Top );
				}
			}
		}

		return( export1 );
	}

public:
	/*==============================================================================
	 * export1 the screen
	 *=============================================================================*/
	std::string Export( const KScreen & nscreen, const KScreen & oscreen )
	{
		ktrace_in( );
		ktrace_level( 0 );

		ktrace( "KScreenExport::Export2( )" );

		if( this->NeedFlatScreen( nscreen, oscreen ) ) return( this->FlatScreen( nscreen ) );
		else return( this->DiffScreen( nscreen, oscreen ) );

	}

public:
	/*==============================================================================
	 * export1 the screen ( auto refresh )
	 *=============================================================================*/
	std::string Export( )
	{
		ktrace_in( );
		ktrace_level( 0 );
		ktrace( "KScreenExport::Export( )" );

		std::string str;

		this->nscreen.Refresh( );

		str = this->ClientServerMismatch( this->nscreen );
		if( str == "." ) str = this->Export( this->nscreen, this->oscreen );

		this->oscreen = this->nscreen;

		return( str );
	}
};