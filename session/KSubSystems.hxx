#pragma once
#include <algorithm>
#include "..\shared\KTrace.hxx"

class KSubSystems
{
private:
	/*==============================================================================
	 * vars
	 *=============================================================================*/
	std::string subsystem_dir;

public:
	/*==============================================================================
	 * set subsystem directory
	 *=============================================================================*/
	void SetSubsystemDirectory( std::string subsystem_dir )
	{
		ktrace_in( );
		ktrace( "KSubSystems::SetSubsystemDirectory( " << subsystem_dir << " )" );

		this->subsystem_dir = subsystem_dir;

	}
public:
	/*==============================================================================
	 * check access
	 *=============================================================================*/
	bool CheckAccess(HANDLE token, std::string subsystem)
	{
		ktrace_in( );
		ktrace( "KSubSystems::CheckAccess( " << subsystem << " )" );

		std::string file = this->subsystem_dir + "\\" + subsystem + ".allowed";

		if( !ImpersonateLoggedOnUser( token ) ) 
		{
			kerror( "CheckAccess - ImpersonateLoggedOnUser" );
			return false;
		}
		
		bool allowed = false;

		// try read the file
		HANDLE hFile = CreateFile(file.c_str(),               // file to open
                       GENERIC_READ,          // open for reading
                       FILE_SHARE_READ,       // share for reading
                       NULL,                  // default security
                       OPEN_EXISTING,         // existing file only
                       FILE_ATTRIBUTE_NORMAL, // normal file
                       NULL);

		if ( hFile != INVALID_HANDLE_VALUE )
		{
			allowed = true; 

			CloseHandle(hFile);
		}

		if( !RevertToSelf( ) )
		{
			kerror( "CheckAccess - RevertToSelf" );
			return false;
		}

		return allowed;
	}
};

