#pragma once

#include <windows.h>
#include <string>
#include <map>

#include "..\shared\kts.h"
class KFlags
{
private:
	/*==============================================================================
	 * inner struct
	 *=============================================================================*/
	struct Flag
	{
		std::string name;
		DWORD sec;
		DWORD ticks;
		bool active;
		bool raised;

		Flag( std::string name, long sec, long ticks, bool active, bool raised )
		{
			this->name = name;
			this->sec = sec;
			this->ticks = ticks;
			this->active = active;
			this->raised = raised;
		}

		Flag( )
		{
			this->name = "";
			this->sec = 0;
			this->ticks = 0;
			this->active = false;
			this->raised = false;
		}
	};
private:
	/*==============================================================================
	 * vars
	 *=============================================================================*/
	std::map< std::string, Flag > flags;
	CRITICAL_SECTION cs;

public:
	// =============================================================================
	// constructor / private!
	// =============================================================================
	KFlags( )
	{
		ktrace_in( );
		ktrace( "KFlags::KFlags( )" );
		InitializeCriticalSection( &cs );
	}

public:
	// =============================================================================
	// create flag / with timeout
	// =============================================================================
	void Create( std::string name, long sec = 0 )
	{
		ktrace_in( );
		ktrace( "KFlags::Create( " << name << ", " << sec << " )" );

		Flag flg( name, sec, 0, false, false );

		EnterCriticalSection( &this->cs );
		this->flags[ name ] = flg;
		LeaveCriticalSection( &this->cs );
	}

public:
	// =============================================================================
	// enable flag
	// =============================================================================
	void Enable( std::string name )
	{
		ktrace_in( );
		ktrace( "KFlags::Enable( " << name << " )" );

		EnterCriticalSection( &this->cs );
		this->flags[ name ].ticks = GetTickCount( );
		this->flags[ name ].active = true;
		LeaveCriticalSection( &this->cs );
	}

public:
	// =============================================================================
	// disable flag
	// =============================================================================
	void Disable( std::string name )
	{
		ktrace_in( );
		ktrace( "KFlags::Disable( " << name << " )" );

		EnterCriticalSection( &this->cs );
		this->flags[ name ].ticks = GetTickCount( );
		this->flags[ name ].active = false;
		this->flags[ name ].raised = false;
		LeaveCriticalSection( &this->cs );
	}

public:
	// =============================================================================
	// reset flag
	// =============================================================================
	void Reset( std::string name )
	{
		ktrace_in( );
		ktrace( "KFlags::Reset( " << name << " )" );

		EnterCriticalSection( &this->cs );
		this->flags[ name ].ticks = GetTickCount( );
		this->flags[ name ].raised = false;
		LeaveCriticalSection( &this->cs );
	}

public:
	// =============================================================================
	// raise flag
	// =============================================================================
	void Raise( std::string name )
	{
		ktrace_in( );
		ktrace( "KFlags::Raise( " << name << " )" );

		EnterCriticalSection( &this->cs );
		this->flags[ name ].active = true;
		this->flags[ name ].raised = true;
		LeaveCriticalSection( &this->cs );
	}

public:
	// =============================================================================
	// is flag raised
	// =============================================================================
	bool IsRaised( std::string name )
	{
		ktrace_in( );
		ktrace( "KFlags::IsRaised( " << name << " )" );


		EnterCriticalSection( &this->cs );
		Flag flg = this->flags[ name ];
		LeaveCriticalSection( &this->cs );

		if( flg.active == false ) return false;
		if( flg.raised == true ) return true;
		if( flg.sec == 0 ) return false;
		if( ( GetTickCount( ) - flg.ticks ) > ( flg.sec * 1000 ) ) return true;

		return false;
	}

};