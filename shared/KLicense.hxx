#pragma once

class KLicense
{
private:
	/*==============================================================================
	 * consume ctrl_c input
	 *=============================================================================*/
	static const DWORD a = 123;
	static const DWORD c = 1234;
	static const DWORD m = 65534;
	DWORD z;

public:
	/*==============================================================================
	 * init z
	 *=============================================================================*/
	KLicense( DWORD z )
	{
		this->z = z;
	}

public:
	/*==============================================================================
	 * gen key
	 *=============================================================================*/
	DWORD GenKey( DWORD n )
	{
		DWORD z = this->z;

		for( DWORD i = 0; i < n; i++ ) z = ( z * this->a + this->c ) % this->m;

		DWORD key = 0;
		unsigned char * k = ( unsigned char * )&key;

		k[ 0 ] = ( unsigned char )n;
		k[ 1 ] = ( unsigned char )( n >> 8 );

		k[ 2 ] = ( unsigned char )z;
		k[ 3 ] = ( unsigned char )( z >> 8 );

		if( key > 1000000000 ) return( key );

		return( 0 );
	}

public:
	/*==============================================================================
	 * check key
	 *=============================================================================*/
	bool CheckKey( DWORD key )
	{
		unsigned char * k = ( unsigned char * )&key;
		DWORD n = k[ 0 ] | ( k[ 1 ]  << 8 );

		if( n == 0 ) return( false );

		if( this->GenKey( n ) == key ) return( true );

		return( false );
	}
};