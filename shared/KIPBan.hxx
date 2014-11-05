#pragma once


#include <windows.h>
#include <string>
#include <sstream>
#include <vector>
#include "..\shared\kts.h"
#include "..\shared\KConsole.hxx"

class KIPBan
{
#define BANNED	99999
	/*==============================================================================
	 * var
	 *=============================================================================*/
private:
	
private:
	/*==============================================================================
	 * params
	 *=============================================================================*/
	struct Params
	{
		int ban_max_connections;
		std::string ban_ip_directory;

		Params( )
		{
			KIni ini;
			ini.File( ".\\kts.ini" );

			
			ini.GetKey( "KSession", "ban_max_connections", this->ban_max_connections );
			ini.GetKey( "KSession", "ban_ip_directory", this->ban_ip_directory );

			KWinsta::ExpandEnvironmentString( this->ban_ip_directory );
		}
	} params;

public:
	/*==============================================================================
	 * IP counter struct
	 *=============================================================================*/
	struct KIPBAN_STATE
	{
		std::string ip;
		int count;
		
		KIPBAN_STATE()
		{
			this->ip = "";
			this->count = 0;
		}

		void FromString( std::string str )
		{
			ktrace_in( );
			ktrace( "KIPBan::KIPBAN_STATE::FromString( " << str << " )" );

			std::vector<int> sep;
			for( unsigned i = 0; i < str.length( ); i++ )
			{
				if( str[ i ] == '-' ) sep.push_back( i );
			}

			if( sep.size( ) != 1 )
			{
				kerror( "wrong seps number" );
				return;
			}

			// count
			this->count = atoi( str.substr( 0, sep[ 0 ] ).c_str( ) );
			ktrace( "count = " << this->count );
			// ip
			this->ip = str.substr( sep[ 0 ] + 1 );
			ktrace( "ip = " << this->ip );
		}
	} kipban_state;

public:
	/*==============================================================================
	 * add IP connection counter
	 *=============================================================================*/
	void AddIPBanConnection( std::string ip )
	{
		ktrace_in( );
		ktrace( "KIPBan::AddIPConnection( " << ip << " )" );

		if( this->params.ban_max_connections == 0 ) return;

		int count = this->FindIPConnectionState( ip ).count + 1;
		if( count > this->params.ban_max_connections ) count = BANNED;

		this->ResetIPState( ip );
		this->CreateIPState( ip, count );
	}

public:
	/*==============================================================================
	 * check if IP is banned
	 *=============================================================================*/
	bool IsBanned( std::string ip )
	{
		ktrace_in( );
		ktrace( "KIPBan::IsBanned( " << ip << " )" );

		if( this->params.ban_max_connections == 0 ) return false;
		if( this->FindIPConnectionState( ip ).count == BANNED ) return true;
		return false;
	}

public:
	/*==============================================================================
	 * reset IP connection counter
	 *=============================================================================*/
	void ResetIPBanConnection( std::string ip )
	{
		ktrace_in( );
		ktrace( "KIPBan::ResetIPConnection( " << ip << " )" );

		if( this->params.ban_max_connections == 0 ) return;

		this->ResetIPState( ip );
	}

private:
	/*==============================================================================
	 * reset IP state
	 *=============================================================================*/
	void ResetIPState( std::string ip )
	{
		ktrace_in( );
		ktrace( "KIPBan::ResetIPState( " << ip << " )" );

		std::stringstream s;
		s << FindIPConnectionState( ip ).count;

		std::string name = this->params.ban_ip_directory + "\\" + s.str() + "-" + ip;

		ktrace( "name = " << name );

		for( int i = 0; i < 10; i++ )
		{
			if( DeleteFile( name.c_str( ) ) ) return;
			Sleep( 100 );
		}
		ktrace( "can't delete ip ban file " << name );
	}

private:
	/*==============================================================================
	 * create IP state
	 *=============================================================================*/
	void CreateIPState( std::string ip, int count )
	{
		ktrace_in( );
		ktrace( "KIPBan::CreateIPState( " << ip << " )" );

		std::stringstream s;
		s << count;

		std::string name = this->params.ban_ip_directory + "\\" + s.str() + "-" + ip;

		ktrace( "name = " << name );

		HANDLE file = CreateFile( name.c_str(), 0, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( file != INVALID_HANDLE_VALUE )
		{
			CloseHandle( file );
		}
		else
		{
			klog( "can't create ip ban file " << name );
		}
	}

private:
	/*==============================================================================
	 * find IP counter
	 *=============================================================================*/
	KIPBAN_STATE FindIPConnectionState( std::string ip )
	{
		ktrace_in( );
		ktrace( "KIPBan::FindIPConnection( " << ip << " )" );

		std::string path = this->params.ban_ip_directory + "\\*-" + ip;

		WIN32_FIND_DATA FindFileData;
		
		HANDLE hFind = FindFirstFile( path.c_str( ), &FindFileData );

		this->kipban_state.count = 0;
		this->kipban_state.ip = ip;

		if( hFind == INVALID_HANDLE_VALUE )
		{
			if( GetLastError( ) == ERROR_FILE_NOT_FOUND ) return( this->kipban_state );

			kerror( "can't FindFirstFile" );
			return( this->kipban_state );
		}

		this->kipban_state.FromString( FindFileData.cFileName );

		FindClose(hFind);

		return( this->kipban_state );
	}
};