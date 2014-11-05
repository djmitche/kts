/*==============================================================================
 * KpyM Telnet/SSH Server
 * Copyright (C) 2002-2008  Kroum Grigorov
 * e-mail:  support@kpym.com
 * web:     http://www.kpym.com
 * license: ( see ../license.txt for details )
 *=============================================================================*/
#define _ktrace
#include "KSession.hxx"


// =============================================================================
// app entry point
// =============================================================================
int main( int argc, char **argv )
{
	USE(argc);
	USE(argv);
	ktrace_in( );
	ktrace_level( 10 );

	kbegin_try_block;

	KWinsta::SetKtsHome( );

	if(KWinsta::GetCmdLineParam( "-ssh:" ) == "1" )
	{
		// run ssh mode
		KSession session( true );
		session.Run( );
	}
	else
	{
		// run telnet mode
		KSession session( false );
		session.Run( );
	}

	kend_try_block;
}
