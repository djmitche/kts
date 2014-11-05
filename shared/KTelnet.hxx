#pragma once
#include <windows.h>
#include <string>
#include <sstream>

#include "..\shared\KIni.hxx"
#include "..\shared\KTrace.hxx"
#include "..\shared\KConsole.hxx"

class KTelnet
{
	/*==============================================================================
	 * vars
	 *=============================================================================*/
public:
	COORD screen;

private:
	struct CLIENT_SERVER
	{
		std::string client;
		std::string server;
		bool passed;
	};
	std::vector< CLIENT_SERVER > client_server;
	bool ctrl_c_enabled;
	DWORD export_code_page;
	bool ssh;

	WORD attribute;

	struct Colors
	{
		// foreground intensity
		std::string foreground_intensity_on;
		std::string foreground_intensity_off;

		// foreground colors
		std::string foreground_white;
		std::string foreground_red;
		std::string foreground_green;
		std::string foreground_blue;
		std::string foreground_yellow;
		std::string foreground_black;
		std::string foreground_cyan;
		std::string foreground_magenta;

		// background intensity
		std::string background_intensity_on;
		std::string background_intensity_off;

		// background colors
		std::string background_white;
		std::string background_red;
		std::string background_green;
		std::string background_blue;
		std::string background_yellow;
		std::string background_black;
		std::string background_cyan;
		std::string background_magenta;
	} colors;

public:
	/*==============================================================================
	 * constructor
	 *=============================================================================*/
	KTelnet( bool ssh = false )
	{
		ktrace_in( );
		ktrace( "KTelnet::KTelnet( )" );

		this->ctrl_c_enabled = true;

		this->attribute = 0;

		// foreground intensity
		this->colors.foreground_intensity_on = "[1m";
		this->colors.foreground_intensity_off = "[22m";

		// foreground colors
		this->colors.foreground_white = "[37m";
		this->colors.foreground_red = "[31m";
		this->colors.foreground_green = "[32m";
		this->colors.foreground_blue = "[34m";
		this->colors.foreground_yellow = "[33m";
		this->colors.foreground_black = "[30m";
		this->colors.foreground_cyan = "[35m";
		this->colors.foreground_magenta = "[36m";

		// background intensity
		this->colors.background_intensity_on = "[5m";
		this->colors.background_intensity_off = "[25m";

		// background colors
		this->colors.background_white = "[47m";
		this->colors.background_red = "[41m";
		this->colors.background_green = "[42m";
		this->colors.background_blue = "[44m";
		this->colors.background_yellow = "[43m";
		this->colors.background_black = "[40m";
		this->colors.background_cyan = "[45m";
		this->colors.background_magenta = "[46m";

		this->ssh = ssh;
	}

public:
	/*==============================================================================
	 * load from file
	 *=============================================================================*/
	bool Load( std::string file )
	{
		ktrace_in( );
		ktrace( "KTelnet::Load( " << file << " )" );

		KIni ini;

		ini.File( file );

		// foreground intensity
		ini.GetKey( "KTelnet", "foreground_intensity_on", this->colors.foreground_intensity_on );
		ini.GetKey( "KTelnet", "foreground_intensity_off", this->colors.foreground_intensity_off );

		// foreground colors
		ini.GetKey( "KTelnet", "foreground_white", this->colors.foreground_white );
		ini.GetKey( "KTelnet", "foreground_red", this->colors.foreground_red );
		ini.GetKey( "KTelnet", "foreground_green", this->colors.foreground_green );
		ini.GetKey( "KTelnet", "foreground_blue", this->colors.foreground_blue );
		ini.GetKey( "KTelnet", "foreground_yellow", this->colors.foreground_yellow );
		ini.GetKey( "KTelnet", "foreground_black", this->colors.foreground_black );
		ini.GetKey( "KTelnet", "foreground_cyan", this->colors.foreground_cyan );
		ini.GetKey( "KTelnet", "foreground_magenta", this->colors.foreground_magenta );

		// background intensity
		ini.GetKey( "KTelnet", "background_intensity_on", this->colors.background_intensity_on );
		ini.GetKey( "KTelnet", "background_intensity_off", this->colors.background_intensity_off );

		// background colors
		ini.GetKey( "KTelnet", "background_white", this->colors.background_white );
		ini.GetKey( "KTelnet", "background_red", this->colors.background_red );
		ini.GetKey( "KTelnet", "background_green", this->colors.background_green );
		ini.GetKey( "KTelnet", "background_blue", this->colors.background_blue );
		ini.GetKey( "KTelnet", "background_yellow", this->colors.background_yellow );
		ini.GetKey( "KTelnet", "background_black", this->colors.background_black );
		ini.GetKey( "KTelnet", "background_cyan", this->colors.background_cyan );
		ini.GetKey( "KTelnet", "background_magenta", this->colors.background_magenta );

		// ctrl + c enabled
		ini.GetKey( "KTelnet", "ctrl_c_enabled", this->ctrl_c_enabled );

		// ctrl + c enabled
		ini.GetKey( "KTelnet", "export_code_page", this->export_code_page );

		// client & server
		for( unsigned i = 0; i < 256; i++ )
		{
			CLIENT_SERVER cs;

			std::string client;
			std::string server;
			std::stringstream s;

			s << "client" << i;
			client = s.str( );

			s.str( "" );
			s << "server" << i;
			server = s.str( );

			cs.passed = false;
			if( !ini.GetHexKey( "KTelnet", client, cs.client ) ) break;
			if( !ini.GetHexKey( "KTelnet", server, cs.server ) ) break;

			this->client_server.push_back( cs );

		}

		return( true );
	}

public:
	/*==============================================================================
	 * telnet gotoxy command
	 *=============================================================================*/
	std::string GotoXY( int x, int y )
	{
		std::stringstream s;
		s << "[" << ( y + 1 ) << ";" << ( x + 1 ) << "f";

		return( s.str( ) );
	}

public:
	// =============================================================================
	// telnet are you there command
	// =============================================================================
	std::string AreYouThere( )
	{
		return( "\xff\xf1" );
	}

public:
	/*==============================================================================
	 * telnet "\r\n" command
	 *=============================================================================*/
	std::string CrLf( )
	{
		return( "\r\n" );
	}

public:
	/*==============================================================================
	 * telnet cls command
	 *=============================================================================*/
	std::string Cls( WORD color )
	{
		return( this->Color( color ) + "[1;1f[0J" );
	}

public:
	/*==============================================================================
	 * encode char if needed
	 *=============================================================================*/
	std::string Encode( WCHAR ch )
	{
		char buff[ 15 ];
		int len;

		len = WideCharToMultiByte( this->export_code_page, 0, &ch, 1, buff, 10, NULL, NULL );
	
		if( len <= 0 )
		{
			ktrace_in( );
			kerror( "WideCharToMultiByte( ):err" );
		}

		if( len == 1 )
		{
			unsigned char ch = buff[ 0 ];
			unsigned char encode[ 33 ] = " ...............><......II<>..II";

			if( ch < 32 ) return( std::string( 1, encode[ ch ] ) );
			if( ch == 255 && this->ssh == false ) return( std::string( 2, buff[ 0 ] ) );
			return( std::string( 1, buff[ 0 ] ) );
		}

		std::string str;
		str.assign( buff, len );
		return( str );
	}

public:
	/*==============================================================================
	 * set color
	 *=============================================================================*/
	std::string Color( WORD attribute )
	{
		if( this->attribute != attribute )
		{
			this->attribute = attribute;

			std::string attr;

			if( attribute & KConsole::FOREGROUND_INTENSITY ) attr += this->colors.foreground_intensity_on;
			else attr += this->colors.foreground_intensity_off;

			switch( attribute & KConsole::FOREGROUND_MASK )
			{
				case KConsole::FOREGROUND_WHITE:
					attr += this->colors.foreground_white;
					break;
				case KConsole::FOREGROUND_RED:
					attr += this->colors.foreground_red;
					break;
				case KConsole::FOREGROUND_GREEN:
					attr += this->colors.foreground_green;
					break;
				case KConsole::FOREGROUND_BLUE:
					attr += this->colors.foreground_blue;
					break;
				case KConsole::FOREGROUND_YELLOW:
					attr += this->colors.foreground_yellow;
					break;
				case KConsole::FOREGROUND_BLACK:
					attr += this->colors.foreground_black;
					break;
				case KConsole::FOREGROUND_CYAN:
					attr += this->colors.foreground_cyan;
					break;
				case KConsole::FOREGROUND_MAGENTA:
					attr += this->colors.foreground_magenta;
					break;
			}

			if( attribute & KConsole::BACKGROUND_INTENSITY ) attr += this->colors.background_intensity_on;
			else attr += this->colors.background_intensity_off;

			switch( attribute & KConsole::BACKGROUND_MASK )
			{
				case KConsole::BACKGROUND_WHITE:
					attr += this->colors.background_white;
					break;
				case KConsole::BACKGROUND_RED:
					attr += this->colors.background_red;
					break;
				case KConsole::BACKGROUND_GREEN:
					attr += this->colors.background_green;
					break;
				case KConsole::BACKGROUND_BLUE:
					attr += this->colors.background_blue;
					break;
				case KConsole::BACKGROUND_YELLOW:
					attr += this->colors.background_yellow;
					break;
				case KConsole::BACKGROUND_BLACK:
					attr += this->colors.background_black;
					break;
				case KConsole::BACKGROUND_CYAN:
					attr += this->colors.background_cyan;
					break;
				case KConsole::BACKGROUND_MAGENTA:
					attr += this->colors.background_magenta;
					break;			
			}

			return( attr );
		}

		return( "" );
	}
private:
	/*==============================================================================
	 * consume ctrl_c input
	 *=============================================================================*/
	bool Consume_ctrl_c( std::string & input, std::string & output )
	{
		USE(output);

		ktrace_in( );

		ktrace( "KTelnet::Consume_ctrl_c( )" );

		if( !this->ctrl_c_enabled )
		{
			ktrace( "disabled" );
			return( false );
		}
		if( input == "" ) return( false );
		if( input[ 0 ] != '\x03' ) return( false );

		input = input.substr( 1 );

		if( !GenerateConsoleCtrlEvent( CTRL_C_EVENT, 0 ) )
		{
			ktrace( "GenerateConsoleCtrlEvent( ):err" );
			return( false );
		}
		return( true );
	}
private:
	/*==============================================================================
	 * consume iac do and iac will that we don't know
	 *=============================================================================*/
	bool Consume_iac_do_will( std::string & input, std::string & output )
	{
		ktrace_in( );
		ktrace( "KTelnet::Consume_iac_do_will( )" );

		if( input.length( ) < 3 ) return( false );

		if( input.substr( 0, 2 ) != "\xFF\xFD" && input.substr( 0, 2 ) != "\xFF\xFB" ) return( false );

		output = "\xFF\xFE" + std::string( 1, input[ 2 ] ); // refuse that option

		input = input.substr( 3 );

		return( true );
	}

private:
	/*==============================================================================
	 * consume iac dont and iac wont that we don't know
	 *=============================================================================*/
	bool Consume_iac_dont_wont( std::string & input, std::string & output )
	{
		ktrace_in( );
		ktrace( "KTelnet::Consume_iac_dont_wont( )" );

		if( input.length( ) < 3 ) return( false );

		if( input.substr( 0, 2 ) != "\xFF\xFC" && input.substr( 0, 2 ) != "\xFF\xFE" ) return( false );

		output.clear();

		input = input.substr( 3 );
		return( true );
	}


private:
	/*==============================================================================
	 * consume rec_resp from input
	 *=============================================================================*/
	bool Consume_client_server( std::string & input, std::string & output )
	{
		ktrace_in( );
		ktrace( "KTelnet::Consume_client_server( " << input << " )" );

		for( unsigned i = 0; i < this->client_server.size( ); i++ )
		{
			if( this->client_server[ i ].passed ) continue;
			if( this->client_server[ i ].client.length( ) > input.length( ) ) continue;
			if( this->client_server[ i ].client == input.substr( 0, this->client_server[ i ].client.length( ) ) )
			{
				output = this->client_server[ i ].server;
				input = input.substr( this->client_server[ i ].client.length( ) );
				this->client_server[ i ].passed = true;
				return( true );
			}
		}
		return( false );
	}

private:
	/*==============================================================================
	 * consume iac sb naws
	 *=============================================================================*/
	bool Consume_iac_sb_naws( std::string & input, std::string & output )
	{
		USE(output);

		ktrace_in( );
		ktrace( "KTelnet::Consume_iac_sb_naws( " << input << " )" );

		if( input.length( ) < 9 ) return( false );
		if( input.substr( 0, 3 ) != "\xff\xfa\x1f" ) return( false );

		this->screen.X = ( unsigned char )input[ 3 ] * 256 + ( unsigned char )input[ 4 ];
		this->screen.Y = ( unsigned char )input[ 5 ] * 256 + ( unsigned char )input[ 6 ];

		input = input.substr( 9 );

		ktrace( "[ " << this->screen.X << ", " << this->screen.Y << " ]" );
		return( true );
	}

public:
	/*==============================================================================
	 * consume input
	 *=============================================================================*/
	bool Consume( std::string & input, std::string & output )
	{
		output.clear();

		if( this->Consume_client_server( input, output ) ) return( true );

		if( this->Consume_iac_do_will( input, output ) ) return( true );
		if( this->Consume_iac_dont_wont( input, output ) ) return( true );
		if( this->Consume_iac_sb_naws( input, output ) ) return( true );
		if( this->Consume_ctrl_c( input, output ) ) return( true );
		return( false );
	}
};