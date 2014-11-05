#pragma once
#include "..\shared\slre\slre.c"
#include "..\shared\KTrace.hxx"

class KSlre
{
public:
	/*==============================================================================
	* just a wrapper around the slre library
	*=============================================================================*/
	bool Match( const std::string & s, const std::string & expr )
	{

		ktrace_in( );
		ktrace( "KSlre::Match( " << s << ", " << expr << " )" );

		struct slre        slre;
		struct cap         captures[255];

		int res = slre_compile(&slre, expr.c_str());
		if( res == 0 )
		{
			kerror( "can't compile expression [" << expr << "]" );
			return false;
		}
		res = slre_match(&slre, s.c_str(), s.length(), captures);
		if(res == 0) return false;

		// check if this is a whole match
		if((unsigned)captures[0].len != s.length()) return false;

		return true;
	}
};
