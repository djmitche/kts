#pragma once
#include <conio.h>
#include <iostream>
#include <string>
#include <vector>

#include "..\shared\kts.h"
#include "..\shared\KWinsta.hxx"
#include "..\shared\KConsole.hxx"
#include "..\shared\KIni.hxx"
#include "..\shared\KSsh.hxx"
#include "KService.hxx"

class KSetup
{
private:
	/*==============================================================================
	 * vars
	 *=============================================================================*/
	enum Key
	{
		  Unknown = 0
		, Esc
		, Up
		, Down
		, Enter
	} key;


	std::vector< std::string > sections;
	std::vector< std::string > options;
	int top;
	int height;
	int index;
	KConsole console;
	COORD pos;
	std::string file;

private:
	/*==============================================================================
	 * clear screen
	 *=============================================================================*/
	void Cls( )
	{
		this->console.SetAttribute( KConsole::BACKGROUND_BLACK | KConsole::FOREGROUND_INTENSITY | KConsole::FOREGROUND_WHITE );
		system( "cls" );
/*		this->console.SetAttribute( KConsole::BACKGROUND_INTENSITY | KConsole::BACKGROUND_BLACK | KConsole::FOREGROUND_INTENSITY | KConsole::FOREGROUND_WHITE );

		COORD pos = { 0, 0 };
		for( int i = 0; i < 25; i ++ )
		{
			pos.Y = i + 1;
			console.SetCursor( pos, false );
			console.Write( "                                                                            " );
		}
*/
		COORD pos = { 0, 0 };
		pos.X = 0;
		pos.Y = 0;
		console.SetCursor( pos, false );
	}
private:
	/*==============================================================================
	 * process functions
	 *=============================================================================*/
	void ProcessFunction( )
	{
		this->Cls( );
		switch( this->index )
		{
		case 0:
			this->Install( this->file );
			_getch( );
			break;
		case 1:
			this->Uninstall( this->file );
			_getch( );
			break;
		case 2:
			this->Start( this->file );
			_getch( );
			break;
		case 3:
			this->Stop( this->file );
			_getch( );
			break;
		case 4:
			this->RSAKey( this->file );
			_getch( );
			break;
		default:
			KIni ini;
			std::string param;
			
			ini.File( this->file );
			ini.GetKey( this->sections[ this->index ], this->options[ this->index ], param );

			COORD pos = { 0, 0 };
			this->console.SetAttribute( KConsole::BACKGROUND_WHITE | KConsole::FOREGROUND_BLACK );
			this->console.SetCursor( pos, false );

			std::stringstream header;
			header << "  KpyM Telnet/SSH Server - v" << KTS_VERSION << "                <ret> = save   <esc> = cancel   ";
			this->console.Write( header.str( ).substr( 0, 79 ) );

			pos.Y = 2;
			this->console.SetCursor( pos );
			this->console.SetAttribute( KConsole::BACKGROUND_BLACK | KConsole::FOREGROUND_WHITE );
			this->console.Write( "[" + this->sections[ this->index ] + "] : " + this->options[ this->index ] + " = " );

			this->console.SetAttribute( KConsole::BACKGROUND_WHITE | KConsole::FOREGROUND_BLACK );

			if( this->EditParam( 0, 3, param ) )
			{
				while( param.find( "\n" ) != std::string::npos ) param = param.replace( param.find( "\n" ), 1, "\\n" );
				ini.SetKey( this->sections[ this->index ], this->options[ this->index ], param );
			}
			break;
		}
	}

private:
	/*==============================================================================
	 * process functions
	 *=============================================================================*/
	bool EditParam( SHORT x, SHORT y, std::string & param )
	{
		COORD pos;
		pos.X = x;
		pos.Y = y;

		while( true )
		{
			this->console.SetAttribute( KConsole::BACKGROUND_WHITE | KConsole::FOREGROUND_BLACK );
			this->console.SetCursor( pos, false );
			this->console.Write( param );

			int key = _getch( );
			if( key == 0 || key == 224 )
			{
				key = 0;
				if( _getch( ) == 75 ) key = 8;
			}


			switch( key )
			{
			case 0:
				break;
			case 27:
				return( false );
			case 13:
				return( true );
			case 8:
				if( param.length( ) > 0 ) param = param.substr( 0, param.length( ) - 1 );

				this->console.SetCursor( pos, false );
				this->console.SetAttribute( KConsole::BACKGROUND_BLACK | KConsole::FOREGROUND_BLACK );
				this->console.Write( param + " " );

				break;
			default:
				param += (char)key;
				if( param.find( "\\n" ) != std::string::npos )
				{
					this->console.SetCursor( pos, false );
					this->console.SetAttribute( KConsole::BACKGROUND_BLACK | KConsole::FOREGROUND_BLACK );
					this->console.Write( param + " " );

					param = param.replace( param.find( "\\n" ), 2, "\n" );
				}
				break;
			}

		}
	}

private:
	/*==============================================================================
	 * read key
	 *=============================================================================*/
	void ReadKey( )
	{
		int key = _getch( );
		if( key == 0 || key == 224 ) key = _getch( );

		switch( key )
		{
		case 27:
			this->key = Esc;
			break;
		case 13:
			this->key = Enter;
			break;
		case 72:
			this->key = Up;
			break;
		case 80:
			this->key = Down;
			break;
		default:
			this->key = Unknown;
		}
	}

private:
	/*==============================================================================
	 * update screen
	 *=============================================================================*/
	void UpdateScreen( )
	{

		
		COORD pos = { 0, 0 };
		std::string label;

		

		this->console.SetAttribute( KConsole::BACKGROUND_WHITE | KConsole::FOREGROUND_BLACK );
		this->console.SetCursor( pos, false );

		std::stringstream header;
		header << "  KpyM Telnet/SSH Server - v" << KTS_VERSION << "                                <esc> = exit   ";
		this->console.Write( header.str( ).substr( 0, 79 ) );

		

		for( int i = this->top; i < this->top + this->height; i++ )
		{
			pos.X = this->pos.X;
			pos.Y = (SHORT)(i - this->top + this->pos.Y);
			console.SetCursor( pos, false );
			
			if( i == this->index )
			{
				this->console.SetAttribute( KConsole::BACKGROUND_WHITE | KConsole::FOREGROUND_BLACK );
				label = " o  ";
			}
			else
			{
				this->console.SetAttribute( KConsole::BACKGROUND_BLACK | KConsole::FOREGROUND_WHITE );
				label = "    ";
			}

			if( this->sections[ i ] == "" )
			{
				label += "   " + this->options[ i ] + "                                         ";
			}
			else
			{
				label += "[" + this->sections[ i ] + "]                      ";
				label = label.substr( 0, 17 );
				label += " : " + this->options[ i ] + "                                         ";
			}

			this->console.Write( label.substr( 0, 46 ) );
		
		}

		pos.X = this->pos.X;
		pos.Y = (SHORT)(this->index - this->top + this->pos.Y);
		console.SetCursor( pos, false );
	}
private:
	/*==============================================================================
	 * process key
	 *=============================================================================*/
	bool ProcessKey( )
	{
		switch( this->key )
		{
		case Up:
			if( this->index > 0 ) this->index--;
			if( this->index < this->top ) this->top--;

			if( this->options[ this->index ] == "" ) return( this->ProcessKey( ) );

			return( true );
		case Down:
			if( this->index < ( int )this->options.size( ) - 1 ) this->index++;
			if( this->index > this->top + this->height -1 ) this->top++;

			if( this->options[ this->index ] == "" ) return( this->ProcessKey( ) );

			return( true );
		case Enter:
			this->ProcessFunction( );
			this->Cls( );
			return( true );
		case Esc:
			return( false );
		}
		return( true );
	}

private:
	/*==============================================================================
	 * update screen
	 *=============================================================================*/
	bool LoadSetup( )
	{
		std::fstream f;

		f.open( this->file.c_str( ), std::ios_base::in );
		if( !f.good( ) ) return( false );

		this->sections.push_back( "" );
		this->options.push_back( "install service" );
		this->sections.push_back( "" );
		this->options.push_back( "uninstall service" );
		this->sections.push_back( "" );
		this->options.push_back( "start service" );
		this->sections.push_back( "" );
		this->options.push_back( "stop service" );
		this->sections.push_back( "" );
		this->options.push_back( "generate RSA key" );
		this->sections.push_back( "" );
		this->options.push_back( "" );

		std::string section;
		while( !f.eof( ) )
		{
			char buff1[ 2010 ];
			f.getline( buff1, 2000 );
			std::string buff = buff1;

			if( buff == "" ) continue;
			if( buff[ 0 ] == ';' ) continue;

			if( buff[ 0 ] == '[' )
			{
				if( buff.find( "]" ) != std::string::npos ) section = buff.substr( 1, buff.find( "]" ) - 1 );
				continue;
			}

			if( buff.find( "=" ) == std::string::npos ) continue;

			buff = buff.substr( 0, buff.find( "=" ) );

			while( buff[ buff.length( ) - 1 ] == ' '
			||     buff[ buff.length( ) - 1 ] == '\t' ) buff = buff.substr( 0, buff.length( ) - 1 );

			this->sections.push_back( section );
			this->options.push_back( buff );
		}

		this->pos.X = 20;
		this->pos.Y = 3;

		this->top = 0;
		this->index = 0;
		this->height = min( this->options.size( ), 20 );
		
		return( true );
	}

public:
	/*==============================================================================
	 * run
	 *=============================================================================*/
	void Run( std::string file )
	{
		this->file = file;

		if( !this->LoadSetup( ) ) return;

		this->Cls( );
		this->console.SetAttribute( KConsole::BACKGROUND_INTENSITY | KConsole::BACKGROUND_BLACK | KConsole::FOREGROUND_WHITE );

		while( true )
		{
			this->UpdateScreen( );
			this->ReadKey( );
			if( !this->ProcessKey( ) ) break;
		}

		this->Cls( );
	}

public:
	/*==============================================================================
	 * generate rsa key
	 *=============================================================================*/
	bool RSAKey( std::string file )
	{
		ktrace_in( );
		ktrace_level( 10 );
		ktrace( "KSetup::RSAKey( )" );

		std::cout << "generating rsa key..." << std::endl;

		KSsh ssh;
		KIni ini;
		std::string rsakey_file;

		ini.File( file );
		ini.GetKey( "KSession", "rsakey_file", rsakey_file );
		KWinsta::ExpandEnvironmentString( rsakey_file );

		if( !ssh.CreateRsaKey( rsakey_file ) )
		{
			ktrace( "KSetup::RSAKey:err" );
			kerror( "KSetup::RSAKey:err" );
			std::cout << "can't create rsa key." << std::endl;

			return( false );
		}
		std::cout << "done." << std::endl;
		return( true );

	}
public:
	/*==============================================================================
	 * start service
	 *=============================================================================*/
	bool Start( std::string file )
	{
		ktrace_in( );
		ktrace_level( 10 );
		ktrace( "KSetup::Start( )" );

		KService service;
		KIni ini;
		std::string service_name;

		ini.File( file );
		ini.GetKey( "KDaemon", "service_name", service_name );

		std::cout << "starting service \"" << service_name << "\"..." << std::endl;
		DWORD error = service.Start( service_name );
		if( error != ERROR_SUCCESS )
		{
			ktrace( KWinsta::GetErrorMessage( error ) );
			kerror( "KSetup::Start:err" );
			std::cout << KWinsta::GetErrorMessage( error );

			return( false );
		}
		else
		{
			std::cout << "done." << std::endl;

			return( true );
		}
	}

public:
	/*==============================================================================
	 * stop service
	 *=============================================================================*/
	bool Stop( std::string file )
	{
		ktrace_in( );
		ktrace_level( 10 );
		ktrace( "KSetup::Stop( )" );

		KService service;
		KIni ini;
		std::string service_name;

		ini.File( file );
		ini.GetKey( "KDaemon", "service_name", service_name );

		std::cout << "stopping service \"" << service_name << "\"..." << std::endl;
		DWORD error = service.Stop( service_name );
		if( error != ERROR_SUCCESS )
		{
			ktrace( KWinsta::GetErrorMessage( error ) );
			kerror( "KSetup::Stop:err" );
			std::cout << KWinsta::GetErrorMessage( error );

			return( false );
		}
		else
		{
			std::cout << "done." << std::endl;

			return( true );
		}
	}

public:
	/*==============================================================================
	 * install service
	 *=============================================================================*/
	bool Install( std::string file )
	{
		ktrace_in( );
		ktrace_level( 10 );
		ktrace( "KSetup::Install( )" );

		KService service;
		KIni ini;
		std::string service_name;
		std::string service_info;

		ini.File( file );
		ini.GetKey( "KDaemon", "service_name", service_name );
		ini.GetKey( "KDaemon", "service_info", service_info );

		char module[ 2010 ];

		GetModuleFileName( NULL, module, 2000 );

		std::cout << "installing service \"" << service_name << "\"..." << std::endl;

		std::string module1(module);
		module1 = "\"" + module1 + "\"";

		DWORD error = service.Install( module1, service_name, service_info );
		if( error != ERROR_SUCCESS )
		{
			ktrace( KWinsta::GetErrorMessage( error ) );
			kerror( "KSetup::Install:err" );
			std::cout << KWinsta::GetErrorMessage( error );

			return( false );
		}
		else
		{
			std::cout << "done." << std::endl;

			return( true );
		}
	}

public:
	/*==============================================================================
	 * uninstall service
	 *=============================================================================*/
	bool Uninstall( std::string file )
	{
		ktrace_in( );
		ktrace_level( 10 );
		ktrace( "KSetup::Uninstall( )" );

		KService service;
		KIni ini;
		std::string service_name;

		ini.File( file );
		ini.GetKey( "KDaemon", "service_name", service_name );

		std::cout << "uninstalling service \"" << service_name << "\"..." << std::endl;
		DWORD error = service.Uninstall( service_name );
		if( error != ERROR_SUCCESS )
		{
			ktrace( KWinsta::GetErrorMessage( error ) );
			kerror( "KSetup::Uninstall:err" );
			std::cout << KWinsta::GetErrorMessage( error );

			return( false );
		}
		else
		{
			std::cout << "done." << std::endl;

			return( true );
		}
	}

public:
	// =============================================================================
	//  constructor / init trace only
	// =============================================================================
	KSetup( )
	{
		KIni ini;
		
		int trace_level;
		std::string error_file;
		std::string trace_file;
		std::string log_file;

		ini.File( KTS_INI_FILE );

		ini.GetKey( "KDaemon", "trace_level", trace_level );
		ini.GetKey( "KDaemon", "error_file", error_file );
		ini.GetKey( "KDaemon", "trace_file", trace_file );
		ini.GetKey( "KDaemon", "log_file", log_file );

		KWinsta::ExpandEnvironmentString( error_file );
		KWinsta::ExpandEnvironmentString( trace_file );
		KWinsta::ExpandEnvironmentString( log_file );

		ktrace_master_level( trace_level );
		ktrace_error_file( error_file );
		ktrace_trace_file( trace_file );
		ktrace_log_file( log_file );
	}
};