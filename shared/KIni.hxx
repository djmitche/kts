#pragma once

#include <windows.h>
#include <string>
#include "..\shared\KTrace.hxx"

class KIni
{
private:
	/*==============================================================================
	 * var
	 *=============================================================================*/
	std::string file;

public:
	/*==============================================================================
	 * set ini file
	 *=============================================================================*/
	void File( const std::string & file )
	{
		ktrace_in( );
		ktrace( "KIni::File( " << file << " )" );

		this->file = file;
	}

public:
	/*==============================================================================
	 * get ini file key
	 *=============================================================================*/
	bool GetKey( const std::string & section, const std::string key, std::string & value, const std::string & def = "" )
	{
		ktrace_in( );
		ktrace( "KIni::GetKey( " << section << ", " << key << " )" );

		char buff[ 1000 ];

		int len = GetPrivateProfileString( section.c_str( ), key.c_str( ), def.c_str( ), buff, 1000, this->file.c_str( ) );
		value.assign( buff, len );

		ktrace( "[ " << value << " ]" );

		value = KWinsta::ReplaceString( value, "\\n", "\n" );

		if( value == "eof" ) return( false );
		return( true );
	}

public:
	/*==============================================================================
	 * get hex ini file key
	 *=============================================================================*/
	bool GetHexKey( const std::string & section, const std::string key, std::string & value )
	{
		ktrace_in( );
		ktrace( "KIni::GetHexKey( " << section << ", " << key << " )" );

		value = "";

		std::string buff;
		if( !this->GetKey( section, key, buff ) )
		{
			ktrace( "eof" );
			return( false );
		}

		if( buff == "" ) return( true );
		std::stringstream s;
		unsigned short ch;
		s.str( buff );
		
		while( true )
		{
			s >> std::hex >> ch;
			value += ( char )ch;
			if( !s.good( ) ) break;
		}

		ktrace( "[ " << value << " ]" );
		return( true );
	}

public:
	/*==============================================================================
	 * get hex ini file key
	 *=============================================================================*/
	bool GetHexKey( const std::string & section, const std::string key, std::vector< DWORD > & value )
	{
		ktrace_in( );
		ktrace( "KIni::GetHexKey( " << section << ", " << key << " )" );

		value.clear( );

		std::string buff;
		if( !this->GetKey( section, key, buff ) )
		{
			ktrace( "eof" );
			return( false );
		}

		if( buff == "" ) return( true );
		std::stringstream s;
		DWORD dw;
		s.str( buff );
		
		while( true )
		{
			s >> std::hex >> dw;
			value.push_back( dw );
			if( !s.good( ) ) break;
		}

		ktrace( "[ " << ( unsigned )value.size( ) << " ]" );
		return( true );
	}
public:
	/*==============================================================================
	 * get ini file key int
	 *=============================================================================*/
	bool GetKey( const std::string & section, const std::string key, int & value, int def = 0 )
	{
		ktrace_in( );
		ktrace( "KIni::GetKey( " << section << ", " << key << ", int )" );

		value = def;

		std::string str;
		if( !this->GetKey( section, key, str ) )
		{
			ktrace( "eof" );
			return( false );
		}

		if( str != "" )
		{
			std::stringstream s;
			s.str( str );

			s >> value;
		}

		ktrace( "[ " << value << " ]" );

		return( true );
	}

public:
	/*==============================================================================
	 * get ini file key unsigned
	 *=============================================================================*/
	bool GetKey( const std::string & section, const std::string key, unsigned & value, unsigned def = 0 )
	{
		ktrace_in( );
		ktrace( "KIni::GetKey( " << section << ", " << key << ", int )" );

		value = def;

		std::string str;
		if( !this->GetKey( section, key, str ) )
		{
			ktrace( "eof" );
			return( false );
		}

		if( str != "" )
		{
			std::stringstream s;
			s.str( str );

			s >> value;
		}

		ktrace( "[ " << value << " ]" );

		return( true );
	}
public:
	/*==============================================================================
	 * get ini file key SHORT
	 *=============================================================================*/
	bool GetKey( const std::string & section, const std::string key, SHORT & value, SHORT def = 0 )
	{
		ktrace_in( );
		ktrace( "KIni::GetKey( " << section << ", " << key << ", int )" );

		value = def;

		std::string str;
		if( !this->GetKey( section, key, str ) )
		{
			ktrace( "eof" );
			return( false );
		}

		if( str != "" )
		{
			std::stringstream s;
			s.str( str );

			s >> value;
		}

		ktrace( "[ " << value << " ]" );

		return( true );
	}

public:
	/*==============================================================================
	 * get ini file key DWORD
	 *=============================================================================*/
	bool GetKey( const std::string & section, const std::string key, DWORD & value, DWORD def = 0 )
	{
		ktrace_in( );
		ktrace( "KIni::GetKey( " << section << ", " << key << ", int )" );

		value = def;

		std::string str;
		if( !this->GetKey( section, key, str ) )
		{
			ktrace( "eof" );
			return( false );
		}

		if( str != "" )
		{
			std::stringstream s;
			s.str( str );

			s >> value;
		}

		ktrace( "[ " << value << " ]" );

		return( true );
	}
public:
	/*==============================================================================
	 * get ini file key bool
	 *=============================================================================*/
	bool GetKey( const std::string & section, const std::string key, bool & value, bool def = 0 )
	{
		ktrace_in( );
		ktrace( "KIni::GetKey( " << section << ", " << key << ", int )" );

		value = def;

		std::string str;
		if( !this->GetKey( section, key, str ) )
		{
			ktrace( "eof" );
			return( false );
		}

		if( str != "" )
		{
			std::stringstream s;
			s.str( str );

			s >> value;
		}

		ktrace( "[ " << value << " ]" );

		return( true );
	}

public:
	/*==============================================================================
	 * set ini file key
	 *=============================================================================*/
	bool SetKey( const std::string & section, const std::string & key, const std::string & value )
	{
		ktrace_in( );
		ktrace( "KIni::GetKey( " << section << ", " << key << ", " << value << " )" );

		bool ret = ( TRUE == WritePrivateProfileString( section.c_str( ), key.c_str( ), value.c_str( ), this->file.c_str( ) ) );

		ktrace( "[ " << ret << " ]" );
		return( ret );
	}
};