#pragma once
#include <algorithm>
#include "..\shared\KTrace.hxx"
#include "..\shared\Kini.hxx"

class KKey
{
private:
	/*==============================================================================
	 * var
	 *=============================================================================*/
	struct Key
	{
		std::string client;
		// 0 - message; 1 - wparam; 2 - lparam;
		std::vector < DWORD > key;
	};
	std::vector < Key > keys;
	HWND window;
	HANDLE handle;
	std::string lastClient;
	int send_same_key_delay;
	int send_key_delay;

public:
	/*==============================================================================
	 * set window & handle internaly
	 *=============================================================================*/
	void SetWindow( HWND window )
	{
		this->window = window;

		this->handle = CreateFile(
			  "CONIN$"
			, GENERIC_READ | GENERIC_WRITE
			, FILE_SHARE_READ
			, NULL
			, OPEN_EXISTING
			, FILE_ATTRIBUTE_NORMAL
			, 0);
	}


public:
	/*==============================================================================
	 * load from file
	 *=============================================================================*/
	void Load( std::string file )
	{
		ktrace_in( );
		ktrace( "KKey::Load( " << file << " )" );

		KIni ini;

		// get key send delay params from kts.ini file
		ini.File( ".\\kts.ini" );

		this->send_key_delay = 1;
		this->send_same_key_delay = 50;
		ini.GetKey( "KKey", "send_key_delay", this->send_key_delay );
		ini.GetKey( "KKey", "send_same_key_delay", this->send_same_key_delay );


		// get key values from the specific key file
		ini.File( file );

		for( unsigned i = 0; i < 1024; i++ )
		{
			Key k;
			std::stringstream s;

			s << "input" << i;
			if( !ini.GetHexKey( "KKey", s.str( ), k.client ) ) break;

			s.str( "" );
			s << "key" << i;
			ini.GetHexKey( "KKey", s.str( ), k.key );

			this->keys.push_back( k );
		}
		
		this->SortKeys( );
	}

private:
	/*==============================================================================
	 * sort key
	 *=============================================================================*/
	struct comparer
	{
		bool operator()( const KKey::Key& left, const KKey::Key& right) const
		{
			return left.client.length() > right.client.length();
		}
	};

	void SortKeys( )
	{
		std::sort(this->keys.begin(), this->keys.end(), KKey::comparer());
	}
	
private:
	/*==============================================================================
	 * send key
	 *=============================================================================*/
	void SendKey( std::vector < DWORD > key )
	{
		ktrace_in( );
		ktrace( "KKey::SendKey( " << this->window << ", " << ( unsigned )key.size( ) << " )" );

		for( unsigned i = 0; i < ( key.size( ) / 3 ); i++ )
		{
			ktrace( " m = " << key[ i ] << " w = " << key[ i + 1 ] << " l = " << key[ i + 2 ] );
			PostMessageW( this->window
					, key[ 3 * i ]
					, key[ 3 * i + 1 ]
					, key[ 3 * i + 2 ] );
		}
	}

private:
	void SendKey1( std::vector < DWORD > key )
	{
		ktrace_in( );
		ktrace( "KKey::SendKey1( " << this->handle << ", " << ( unsigned )key.size( ) << " )" );

		for( unsigned i = 0; i < ( key.size( ) / 5 ); i++ )
		{
			INPUT_RECORD ir;
			DWORD dwTmp;

			ir.EventType = KEY_EVENT;
			ir.Event.KeyEvent.bKeyDown =			( BOOL ) key[ 5 * i + 1 ];
			ir.Event.KeyEvent.dwControlKeyState =	( DWORD )key[ 5 * i + 2 ];
			ir.Event.KeyEvent.uChar.UnicodeChar =	( WCHAR )key[ 5 * i + 3 ];
			ir.Event.KeyEvent.wRepeatCount =		( WORD ) key[ 5 * i + 4 ];
			ir.Event.KeyEvent.wVirtualKeyCode =		( WORD ) key[ 5 * i + 5 ];
			ir.Event.KeyEvent.wVirtualScanCode =	(WORD)MapVirtualKey( ir.Event.KeyEvent.wVirtualKeyCode, 0/*MAPVK_VK_TO_VSC*/ );

			WriteConsoleInputW( this->handle, &ir, 1, &dwTmp );
		}
	}
public:
	/*==============================================================================
	 * consume input
	 *=============================================================================*/
	bool Consume( std::string & input )
	{
		ktrace_in( );
		ktrace( "KKey::Consume( " << input << " )" );

		if( input == "" ) return( false );
		for( unsigned i = 0; i < this->keys.size( ); i++ )
		{
			const Key & k = this->keys[ i ];
			if( k.client == "" ) continue;
			if( k.client.length( ) > input.length( ) ) continue;

			if( input.substr( 0, k.client.length( ) ) == k.client )
			{
				if( k.key.size() != 0 )
				{
					if( k.key[0] == 0xFFFFFFFF) this->SendKey1( k.key );
					else 
					{
						this->SendKey( k.key );

						if( this->lastClient == k.client ) Sleep( this->send_same_key_delay );
						else Sleep( this->send_key_delay );

						this->lastClient = k.client;
					}
				}

				input = input.substr( k.client.length( ) );
				return( true );
			}
		}

		return( false );
	}
};