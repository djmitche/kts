#pragma once

#include <conio.h>
#include <time.h>
#include <sstream>

#include "..\shared\kts.h"
#include "..\shared\KConsole.hxx"
#include "..\shared\KIni.hxx"
#include "..\shared\KLicense.hxx"


class KRegkey
{
public:
	// =============================================================================
	// check reg key
	// =============================================================================
	KRegkey( )
	{
#ifdef LIC_Z
		KLicense lic( LIC_Z );
#else
		KLicense lic( 211 );
#endif
		KIni ini;
		DWORD registration_key;

		ini.File( KTS_INI_FILE );
		ini.GetKey( "KRegkey", "registration_key", registration_key );

		if( !lic.CheckKey( registration_key ) )
		{
			KConsole console;

			system( "cls" );

			console.Write( "\n" );
			console.SetAttribute( console.BACKGROUND_YELLOW | console.FOREGROUND_BLACK );
			console.Write( "\n ================================================================= " );
			console.Write( "\n  KpyM Telnet/SSH Server - fully functional unregistered version.  " );
			console.Write( "\n  Order registration key at http://www.kpym.com/                   " );
			console.Write( "\n  The registered version does not display this notice.             " );
			console.Write( "\n ================================================================= " );
			console.SetAttribute( console.BACKGROUND_BLACK | console.FOREGROUND_WHITE );
			console.Write( "\n" );

			COORD pos;
			console.GetCursor( pos );

			for( int i = 5; i > 0 ; i-- )
			{
				std::stringstream s;
				s << "\n waiting... " << i << " ";

				console.SetCursor( pos );
				console.Write( s.str() );
				Sleep( 1000 );
			}

			console.SetCursor( pos );
			console.Write( "\n Press any key to continue..." );

			int key = 0;
			console.ReadKey( key );

			system( "cls" );
		}
	}
};
