#pragma once
#include <algorithm>
#include "..\shared\KTrace.hxx"
#include "..\shared\Kini.hxx"

class KPublickeyLogon
{
private:
	/*==============================================================================
	 * var
	 *=============================================================================*/
	struct Pattern
	{
		std::string publickey;
		std::string username;
		std::string password;
		std::string domain;
	};
	std::vector < Pattern > patterns;

	static const int min_publickey_len = 50;
public:
	/*==============================================================================
	 * load from file
	 *=============================================================================*/
	void Load( std::string file )
	{
		ktrace_in( );
		ktrace( "KPublickeyLogon::Load( " << file << " )" );

		KIni ini;

		ini.File( file );

		for( unsigned i = 0; i < 1024; i++ )
		{
			Pattern p;
			std::stringstream s;

			s << "publickey" << i;
			if( !ini.GetKey( "KPublickeyLogon", s.str( ), p.publickey, "eof" ) ) break;

			// remove padding
			p.publickey = KWinsta::ReplaceString(p.publickey, "=", "");

			s.str( "" );
			s << "username" << i;
			ini.GetKey( "KPublickeyLogon", s.str( ), p.username );

			s.str( "" );
			s << "password" << i;
			ini.GetKey( "KPublickeyLogon", s.str( ), p.password );

			s.str( "" );
			s << "domain" << i;
			ini.GetKey( "KPublickeyLogon", s.str( ), p.domain );

			if(p.publickey.length() < min_publickey_len ) continue;

			this->patterns.push_back( p );
		}
	}

	bool GetPublickeyCredentials( const std::string& publickey, const std::string& username, std::string& password, std::string& domain )
	{
		ktrace_in( );
		ktrace( "KPublickeyLogon::GetPublickeyCredentials( " << publickey << ", <out> )" );

		// skip short publickeys
		if(publickey.length() < min_publickey_len ) return false;

		for( unsigned i = 0; i < this->patterns.size( ); i++ )
		{
			if(patterns[i].publickey == publickey && username == patterns[i].username)
			{
				password = patterns[i].password;
				domain = patterns[i].domain;
				return true;
			}
		}

		return false;
	}
};

