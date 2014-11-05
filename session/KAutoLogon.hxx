#pragma once
#include <algorithm>
#include "..\shared\KTrace.hxx"
#include "..\shared\Kini.hxx"
#include "..\shared\KSlre.hxx"

class KAutoLogon
{
private:
	/*==============================================================================
	 * var
	 *=============================================================================*/
	struct Pattern
	{
		std::string address;
		std::string username;
		std::string password;
		std::string domain;
	};
	std::vector < Pattern > patterns;

public:
	/*==============================================================================
	 * load from file
	 *=============================================================================*/
	void Load( std::string file )
	{
		ktrace_in( );
		ktrace( "KAutoLogon::Load( " << file << " )" );

		KIni ini;

		ini.File( file );

		for( unsigned i = 0; i < 1024; i++ )
		{
			Pattern p;
			std::stringstream s;

			s << "address" << i;
			if( !ini.GetKey( "KAutoLogon", s.str( ), p.address, "eof" ) ) break;

			s.str( "" );
			s << "username" << i;
			ini.GetKey( "KAutoLogon", s.str( ), p.username );

			s.str( "" );
			s << "password" << i;
			ini.GetKey( "KAutoLogon", s.str( ), p.password );

			s.str( "" );
			s << "domain" << i;
			ini.GetKey( "KAutoLogon", s.str( ), p.domain );

			this->patterns.push_back( p );
		}
	}

	bool GetAutoCredentials( const std::string& ipaddress, std::string& username, std::string& password, std::string& domain )
	{
		ktrace_in( );
		ktrace( "KAutoLogon::GetAutoCredentials( " << ipaddress << ", <out> )" );

		for( unsigned i = 0; i < this->patterns.size( ); i++ )
		{
			KSlre kslre;
			if(kslre.Match(ipaddress, patterns[i].address))
			{
				username = patterns[i].username;
				password = patterns[i].password;
				domain = patterns[i].domain;
				return true;
			}
		}

		return false;
	}
};

