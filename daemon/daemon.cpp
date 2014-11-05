/*==============================================================================
 * KpyM Telnet/SSH Server
 * Copyright (C) 2002-2008  Kroum Grigorov
 * e-mail:  support@kpym.com
 * web:     http://www.kpym.com
 * license: ( see ../license.txt for details )
 *=============================================================================*/
#define _ktrace
#define NOCRYPT
#include "..\shared\kts.h"
#include "KDaemon.hxx"
#include "KSetup.hxx"

// =============================================================================
// app entry point
// =============================================================================
void main( int argc, char **argv )
{
	ktrace_in( );
	ktrace_level( 10 );

	kbegin_try_block;

	KWinsta::SetToModuleDirectory( );
	KWinsta::SetKtsHome( );

	if( argc == 1 )
	{
		KDaemon::RunService( );
		return;
	}

	if( argc == 2 )
	{
		if( strcmp( "-debug", argv[ 1 ] ) == 0 )
		{
			KDaemon::Debug( );
			return;
		}

		// init trace
		KSetup setup;

		ktrace_in( );
		ktrace_level( 10 );
		ktrace( "setup" );

		if( strcmp( "-install", argv[ 1 ] ) == 0 ) setup.Install( KTS_INI_FILE );
		if( strcmp( "-uninstall", argv[ 1 ] ) == 0 ) setup.Uninstall( KTS_INI_FILE );
		if( strcmp( "-start", argv[ 1 ] ) == 0 ) setup.Start( KTS_INI_FILE );
		if( strcmp( "-stop", argv[ 1 ] ) == 0 ) setup.Stop( KTS_INI_FILE );
		if( strcmp( "-rsakey", argv[ 1 ] ) == 0 ) setup.RSAKey( KTS_INI_FILE );

		if( strcmp( "-setup", argv[ 1 ] ) == 0 ) setup.Run( KTS_INI_FILE );

	}
	return;

	kend_try_block;
}
