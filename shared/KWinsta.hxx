#pragma once

#pragma once
#include <windows.h>
#include <lmcons.h>
#include <aclapi.h>
#include <string>
#include <userenv.h>
#include <lm.h>

#include "..\shared\KTrace.hxx"

#define WINSTA_ALL (WINSTA_ACCESSCLIPBOARD  | WINSTA_ACCESSGLOBALATOMS | WINSTA_CREATEDESKTOP | WINSTA_ENUMDESKTOPS \
		| WINSTA_ENUMERATE | WINSTA_EXITWINDOWS | WINSTA_READATTRIBUTES | WINSTA_READSCREEN | WINSTA_WRITEATTRIBUTES \
		| DELETE | READ_CONTROL | WRITE_DAC | WRITE_OWNER)

#define DESKTOP_ALL (DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW | DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL \
		| DESKTOP_JOURNALPLAYBACK | DESKTOP_JOURNALRECORD | DESKTOP_READOBJECTS | DESKTOP_SWITCHDESKTOP \
		| DESKTOP_WRITEOBJECTS | DELETE | READ_CONTROL | WRITE_DAC | WRITE_OWNER)

#define GENERIC_ACCESS (GENERIC_READ    | GENERIC_WRITE | GENERIC_EXECUTE | GENERIC_ALL)

class KWinsta
{
public:
	// =============================================================================
	// create the winsta and desktop
	// =============================================================================
	static bool CreateWinstaAndDesktop( std::string name )
	{
		ktrace_in( );
		ktrace( "KWinsta::CreateWinstaAndDesktop( " << name << " )" );

		HWINSTA hWinsta;
		SECURITY_ATTRIBUTES sa;
		HDESK hDesk;
		PSECURITY_DESCRIPTOR psd;
		HWINSTA hOriWinsta;

		hOriWinsta = GetProcessWindowStation( );
		if( hOriWinsta==NULL )
		{
			ktrace( "GetProcessWindowStation( ):err" );
			return( false );
		}

		psd = HeapAlloc( GetProcessHeap( ), 0, SECURITY_DESCRIPTOR_MIN_LENGTH );
		if( psd==NULL )
		{
			ktrace( "HeapAlloc( ):err" );
			return( false );
		}

		if( !InitializeSecurityDescriptor( psd, SECURITY_DESCRIPTOR_REVISION ) )
		{
			ktrace( "InitializeSecurityDescriptor( ):err" );
			return( false );
		}

		if( !SetSecurityDescriptorDacl( psd, TRUE, NULL, FALSE ) ){
			ktrace( "SetSecurityDescriptorDacl( ):err" );
			return( false );
		}

		ZeroMemory( &sa, sizeof( sa ) );
		sa.nLength = sizeof( sa );
		sa.lpSecurityDescriptor = psd;
		sa.bInheritHandle = TRUE;

		std::string winsta = name + "WINSTA";

		hWinsta=CreateWindowStation( winsta.c_str( ), 0, GENERIC_ALL, &sa );
		if( hWinsta==NULL )
		{
			ktrace( "CreateWindowStation( ):err" );
			return( false );
		}

		if( SetSecurityInfo( hWinsta, SE_WINDOW_OBJECT 
			, DACL_SECURITY_INFORMATION|PROTECTED_DACL_SECURITY_INFORMATION
			, NULL, NULL, NULL, NULL )!=ERROR_SUCCESS )
		{
			ktrace( "SetSecurityInfo( ):err" );
			return( false );
		}

		if( !SetProcessWindowStation( hWinsta ) )
		{
			ktrace( "SetProcessWindowStation( ):err" );
			return( false );
		}

		std::string desk = name + "DESK";

		hDesk = CreateDesktop( desk.c_str( ), NULL, NULL, 0, GENERIC_ALL, &sa );
		if( hDesk==NULL )
		{
			ktrace( "CreateDesktop( ):err" );
			return( false );
		}

		if( SetSecurityInfo( hDesk, SE_WINDOW_OBJECT 
			, DACL_SECURITY_INFORMATION|PROTECTED_DACL_SECURITY_INFORMATION
			, NULL, NULL, NULL, NULL )!=ERROR_SUCCESS )
		{
			ktrace( "SetSecurityInfo( ):err" );
			return( false );
		}
		
		if( !SetProcessWindowStation( hOriWinsta ) )
		{
			ktrace( "SetProcessWindowStation( ):err" );
			return( false );
		}

		HeapFree( GetProcessHeap( ), 0, ( LPVOID )psd );
//		CloseHandle( hDesk );
//		CloseHandle( hWinsta );
//		CloseHandle( hOriWinsta );

		return( true );
	}
private:
	// =============================================================================
	// handle to kernel32.dll
	// =============================================================================
	static HMODULE hKernel32( )
	{
		static HMODULE h = 0;
		if( h == 0 ) h = LoadLibrary( "kernel32.dll" );
		return h;
	}

public:
	// =============================================================================
	// run command as user
	// =============================================================================
	static bool RunCommandAsUser(HANDLE token, std::string command)
	{
		ktrace_in( );
		ktrace( "KWinsta::RunCommandAsUser( " << command << " )" );

		STARTUPINFO si;
		SECURITY_ATTRIBUTES sa;

		ZeroMemory( &sa, sizeof( sa ));
		sa.nLength = sizeof( sa );
		sa.bInheritHandle = true;

		ZeroMemory( &si, sizeof( si ) );
		si.cb = sizeof( si );
		si.lpDesktop = "ktsWINSTA\\ktsDESK";
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOW;

		ZeroMemory( &sa, sizeof( sa ));
		sa.nLength = sizeof( sa );
		sa.bInheritHandle = true;

		BOOL ret = false;

		PROCESS_INFORMATION pri = {0};

		ret = CreateProcessAsUser( token, NULL, ( char * )command.c_str( ), NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pri );

		if( ret )
		{
			// wait for the script to terminate
			WaitForSingleObject(pri.hProcess, INFINITE );
			CloseHandle(pri.hThread);
			CloseHandle(pri.hProcess);

			return( true );
		}

		return( false );
	}
public:
	// =============================================================================
	// terminate thread NT4 safe
	// =============================================================================
	static bool TerminateThread( HANDLE handle, DWORD exitCode )
	{
		if( handle == 0 ) return false;

		typedef BOOL (WINAPI *PTerminateThread)( HANDLE, DWORD );

		PTerminateThread tt = ( PTerminateThread )GetProcAddress( KWinsta::hKernel32( ), "TerminateThread" );
		if( tt = 0 ) return false;

		tt(handle, exitCode);
		return true;
	}

public:
	// =============================================================================
	// open thread NT4 safe
	// =============================================================================
	static HANDLE OpenThread( DWORD access, BOOL inherit, DWORD tid)
	{
		typedef HANDLE (WINAPI *POpenThread)(DWORD,BOOL,DWORD);

		POpenThread ot = ( POpenThread )GetProcAddress( KWinsta::hKernel32( ), "OpenThread" );
		if( ot == 0 ) return 0;

		return ot( access, inherit, tid );
	}

public:
	// =============================================================================
	// set the winsta and desktop
	// =============================================================================
	static bool SetWinstaAndDesktop( std::string name )
	{

		ktrace_in( );
		ktrace( "KWinsta::SetWinstaAndDesktop( " << name << " )" );

		HWINSTA hWinsta;
		SECURITY_ATTRIBUTES sa;
		HDESK hDesk;
		PSECURITY_DESCRIPTOR psd;

		psd = HeapAlloc( GetProcessHeap( ), 0, SECURITY_DESCRIPTOR_MIN_LENGTH );
		if( !psd )
		{
			ktrace( "HeapAlloc( ):err" );
			return( false );
		}

		if( !InitializeSecurityDescriptor( psd, SECURITY_DESCRIPTOR_REVISION ) )
		{
			ktrace( "InitializeSecurityDescriptor( ):err" );
			return( false );
		}

		if( !SetSecurityDescriptorDacl( psd, TRUE, NULL, FALSE ) )
		{
			ktrace( "SetSecurityDescriptorDacl( ):err" );
			return( false );
		}

		ZeroMemory( &sa, sizeof( sa ) );
		sa.nLength = sizeof( sa );
		sa.lpSecurityDescriptor = psd;
		sa.bInheritHandle = TRUE;

		std::string winsta = name + "WINSTA";
		hWinsta = OpenWindowStation( winsta.c_str( ), TRUE, GENERIC_ALL );
		if( hWinsta==NULL )
		{
			ktrace( "OpenWindowStation( ):err" );
			return( false );
		}
	
		if( SetSecurityInfo( hWinsta, SE_WINDOW_OBJECT 
			, DACL_SECURITY_INFORMATION|PROTECTED_DACL_SECURITY_INFORMATION
			, NULL, NULL, NULL, NULL )!=ERROR_SUCCESS )
		{
			ktrace( "SetSecurityInfo( ):err" );
			return( false );
		}

		if( !SetProcessWindowStation( hWinsta ) )
		{
			ktrace( "SetProcessWindowStation( ):err" );
			return( false );
		}

		std::string desk = name + "DESK";
		hDesk = OpenDesktop( desk.c_str( ), 0, TRUE, GENERIC_ALL | READ_CONTROL | WRITE_DAC |DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS );
		if( !hDesk )
		{
			ktrace( "OpenDesktop( ):err" );
			return( false );
		}

		if( SetSecurityInfo( hDesk, SE_WINDOW_OBJECT 
			, DACL_SECURITY_INFORMATION|PROTECTED_DACL_SECURITY_INFORMATION
			, NULL, NULL, NULL, NULL )!=ERROR_SUCCESS )
		{
			ktrace( "SetSecurityInfo( ):err" );
			return( false );
		}

		if( !SetThreadDesktop( hDesk ) )
		{
			ktrace( "SetThreadDesktop( ):err" );
			return( false );
		}

		HeapFree( GetProcessHeap( ), 0, ( LPVOID )psd );
//		CloseHandle( hDesk );
//		CloseHandle( hWinsta );

		return( true );
	}

public:
	// =============================================================================
	// add full access for system account to file access list
	// =============================================================================
	static bool AddSystemFullControlToFile( std::string file )
	{
		ktrace_in( );
		ktrace( "KWinsta::AddSystemFullControl( " << file << " )" );

		DWORD res = 0;
		PACL dacl = 0;
		PACL dacl1 = 0;
		PSECURITY_DESCRIPTOR psd = 0;
		EXPLICIT_ACCESS ea = {0};
		PSID sidLocalSystem = 0;
		SID_IDENTIFIER_AUTHORITY local = SECURITY_NT_AUTHORITY;
		 
		res = GetNamedSecurityInfo( ( char * )file.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION
				, NULL, NULL, &dacl, NULL, &psd);
		if( res != ERROR_SUCCESS )
		{
			ktrace( "GetNamedSecurityInfo( ):err res = " << res );
			res = false;
			goto done;
		}

		// SID: Local System
		if( !AllocateAndInitializeSid( &local, 1
				, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &sidLocalSystem ) )
		{
			ktrace( "AllocateAndInitializeSid( ):err" );
			res = false;
			goto done;
		}

		ea.grfAccessPermissions = GENERIC_ALL;
		ea.grfAccessMode = SET_ACCESS;
		ea.grfInheritance = NO_INHERITANCE;
		ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea.Trustee.ptstrName = ( LPTSTR )sidLocalSystem;

		res = SetEntriesInAcl( 1, &ea, dacl, &dacl1 );
		if( res != ERROR_SUCCESS )
		{
			ktrace( "SetEntriesInAcl( ):err res = " << res );
			res = false;
			goto done;
		}

		res = SetNamedSecurityInfo( ( char * )file.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION
				, NULL, NULL, dacl1, NULL );
		if( res != ERROR_SUCCESS )
		{
			ktrace( "SetNamedSecurityInfo( ):err res = " << res );
			res = false;
			goto done;
		}

		res = true;
done:
		// this may leek as following code will never be called on error
		if( sidLocalSystem ) FreeSid( sidLocalSystem );
		if( dacl1 ) LocalFree( dacl1 );
		if( psd ) LocalFree( psd );
		return( res != 0 );
	}

public:
	// =============================================================================
	// set current directory to module directory
	// =============================================================================
	static bool SetToModuleDirectory( )
	{
		ktrace_in( );
		ktrace( "KWinsta::SetToModuleDirectory( )" );

		char buff[ 2010 ];

		if( !GetModuleFileName( NULL, buff, 2000 ) ) return( false );

		buff[ strlen( buff ) - strlen( "daemon.exe" ) ] = 0;

		ktrace( "[ " << buff << " ]" );
		if( !SetCurrentDirectory( buff ) ) return( false );
		return( true );
	}

public:
	// =============================================================================
	// set the winsta and desktop
	// =============================================================================
	static std::string GetCurrentDirectory( )
	{
		ktrace_in( );
		ktrace( "KWinsta::GetCurrentDirectory( )" );

		char buff[ 2010 ];

		if( !::GetCurrentDirectory( 2000, buff ) ) return( "" );

		ktrace( "[ " << buff << " ]" );
		return( buff );
	}

public:
	// =============================================================================
	// set kts_home variable
	// =============================================================================
	static void SetKtsHome( )
	{
		ktrace_in( );
		ktrace( "KWinsta::SetKtsHome( )" );

		SetEnvironmentVariable( "KTS_HOME", KWinsta::GetCurrentDirectory( ).c_str( ) );
	}

public:
	// =============================================================================
	// get error message
	// =============================================================================
	static std::string GetErrorMessage( DWORD error )
	{
		ktrace_in( );
		ktrace( "KWinsta::GetErrorMessage( " << error << " )" );

		void * buff;

		if( !FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
							, NULL, error, 0 , ( LPTSTR )&buff, 0, NULL ) )
		{
			return( "" );
		}

		std::string message = ( char * )buff;

		LocalFree( buff );

		ktrace( "[ " << message << " ]" );

		return( message );
	}

public:
	// =============================================================================
	// check if process is still active
	// =============================================================================
	static bool ProcessStillActive( HANDLE hProcess )
	{
		ktrace_in( );
		ktrace( "KWinsta::ProcessStillActive( " << hProcess << " )" )

		DWORD dwExitCode;

		if( !GetExitCodeProcess( hProcess, &dwExitCode ) )
		{
			// can't GetExitCodeProcess
			ktrace( "GetExitCodeProcess( ):err" );
			return( false );
		}

		ktrace( "[ " << ( dwExitCode == STILL_ACTIVE ) << " ]" );

		return( dwExitCode == STILL_ACTIVE );
	}
public:
	// =============================================================================
	// convert pid to process handle
	// =============================================================================
	static HANDLE PidToHandle( DWORD pid )
	{
		ktrace_in( );
		ktrace( "KWinsta::PidToHandle( " << pid << " )" );

		return( OpenProcess( PROCESS_QUERY_INFORMATION , false, pid ) );
	}

public:
	// =============================================================================
	// get param from command line
	// =============================================================================
	static std::string GetCmdLineParam( std::string strParam )
	{

		ktrace_in( );
		ktrace( "KWinsta::GetCmdLineParam( " << strParam << " )" );

		std::string strCommandLine;
		std::string strValue;
		size_t pos;
		
		strCommandLine = GetCommandLine( );
		ktrace( strCommandLine );

		pos = strCommandLine.find( strParam );
		if( pos == std::string::npos )
		{
			ktrace( "strValue = ''" );
			return( "" );
		}

		strValue = strCommandLine.substr( pos + strParam.length( ) );
		strValue = strValue.substr( 0, strValue.find( " " ) );
		ktrace( "strValue = '" << strValue << "'" );

		return( strValue );
	}
public:
	// =============================================================================
	// load user environment variables
	// =============================================================================
	static bool LoadUserEnvironment( HANDLE token, std::string username )
	{
		ktrace_in( );
		ktrace( "KWinsta::LoadUserEnvironment( " << token << ", " << username << " )" );

		PROFILEINFO pi = { 0 };
		pi.dwSize = sizeof( pi );
		pi.lpUserName = ( char * )username.c_str( );

		if( LoadUserProfile( token, &pi ) == 0 ) 
		{
			ktrace( "can't load user profile " << username );
			return( false );
		}

		void * environment = NULL;
		if( !CreateEnvironmentBlock( &environment, token, true ) )
		{
			ktrace( "can't create user environment " << username );
			return( false );
		}


		WCHAR * env = ( WCHAR *  )environment;
		int i = 0;
		while( true )
		{
			std::wstring var = L"";
			std::wstring val = L"";
			bool var_passed = false;
			while( true )
			{
				if( env[ i ] == 0 ) break;
				if( env[ i ] == L'=' && var_passed == false )
				{
					var_passed = true;
				}
				else
				{
					if( var_passed == false ) var += env[ i ];
					else val += env[ i ];
				}
				i++;
			}
			if( ! SetEnvironmentVariableW( var.c_str( ), val.c_str( ) ) )
			{
				ktrace( "can't set env variable " << wstring2string( var ) << " = " << wstring2string( val ) );
			}
			i++;
			if( env[ i ] == 0 ) break;
		}

		DestroyEnvironmentBlock(environment);
		return( true );
	}
public:
	// =============================================================================
	// convert unicode string to char string
	// =============================================================================
	static std::string wstring2string( const std::wstring & wstr )
	{
		ktrace_in( );
		ktrace( "KWinsta::wstring2string( " << wstr.c_str() << " )" );

		std::string str;

		int len = WideCharToMultiByte( CP_ACP, 0, wstr.c_str(), -1, 0, 0, 0, 0 );

		str.resize(len);
		len = WideCharToMultiByte( CP_ACP, 0, wstr.c_str(), -1, ( char * )str.c_str(), len, 0, 0 );
		( ( char * )str.c_str( ) )[ len ] = 0;

		ktrace( "str = " << str.c_str( ) );
		return str.c_str();
	}
public:
	// =============================================================================
	// convert char string to unicode string
	// =============================================================================
	static std::wstring string2wstring( const std::string & str )
	{
		ktrace_in( );
		ktrace( "KWinsta::string2wstring( " << str.c_str() << " )" );

		std::wstring wstr;

		int len = MultiByteToWideChar( CP_ACP, 0, str.c_str(), -1, 0, 0 );
		wstr.resize(len);

		len = MultiByteToWideChar( CP_ACP, 0, str.c_str(), -1, (LPWSTR)wstr.c_str(), len );

		( (LPWSTR)wstr.c_str( ) )[ len ] = 0;

		ktrace( "len = " << wstr.length() );
		return wstr.c_str();
	}
public:
	// =============================================================================
	// convert char string to unicode string
	// =============================================================================
	static NET_API_STATUS ChangePassword( const std::string & domain, const std::string & user, const std::string & oldpass, const std::string & newpass )
	{
		ktrace_in( );
		ktrace( "KWinsta::string2wstring( " << domain.c_str() << ", " << user.c_str() << " )" );

		std::wstring domain1 = L"\\\\.";
		if( domain.length() > 0 ) domain1 = string2wstring( "\\\\" + domain );

		std::wstring user1 = string2wstring( user );
		std::wstring oldpass1 = string2wstring( oldpass );
		std::wstring newpass1 = string2wstring( newpass );

		return NetUserChangePassword( (LPCWSTR)domain1.c_str(), (LPCWSTR)user1.c_str(), (LPCWSTR)oldpass1.c_str(), (LPCWSTR)newpass1.c_str() );

	}
public:
	// =============================================================================
	// expand environment string
	// =============================================================================
	static bool ExpandEnvironmentString( std::string &strEnv )
	{
		ktrace_in( );
		ktrace( "KWinsta::ExpandEnvironmentString( " << strEnv << " )" );

		size_t start;
		size_t end;
		size_t pos;
		std::string strTmp;
		std::string strVariable;
		char buff[ 1024 ];
		
		
		strTmp = "";
		pos = 0;
		while( true )
		{
			start = strEnv.find( "%", pos );
			if( start == std::string::npos ) break;

			end = strEnv.find( "%", start + 1 );
			if( end == std::string::npos ) break;

			strTmp += strEnv.substr( pos, start - pos );

			strVariable = strEnv.substr( start + 1, end - start - 1 );

			buff[ 0 ] = 0;
			GetEnvironmentVariable( strVariable.c_str( ), buff, 1020 );
			strTmp += buff;

			pos = end + 1;
		}
		strTmp += strEnv.substr( pos );
		strEnv = strTmp;
		
		ktrace( "strEnv =" << strEnv );
		return( true );
	}
public:
	// =============================================================================
	// convert string to lowercase
	// =============================================================================
	static void ToLower( std::string & str )
	{
		for( unsigned u = 0; u < str.length( ); u++ ) str[ u ] = (char)tolower( str[ u ] );
	}
public:
	// =============================================================================
	// replace in string 
	// =============================================================================
	static std::string ReplaceString( const std::string & source, const std::string & from, const std::string & to )
	{
		ktrace_in( );
		ktrace( "KWinsta::ReplaceString( " << source << ", " << from << ", " << to << " )" );

		std::string ret = source;
		while( ret.find( from ) != std::string::npos ) ret = ret.replace( ret.find( from ), from.length(), to );

		return ret;
	}

public:
	// =============================================================================
	// split string
	// =============================================================================
	static std::vector<std::string> SplitString( const std::string & str, const std::string & sep )
	{
		std::vector< std::string > l;
		std::string tmp;
		for( unsigned i = 0; i < str.length( ); i++ )
		{
			if( str.find( sep, i ) != i )
			{
				tmp += str[ i ];
			}
			else 
			{
				l.push_back( tmp );
				tmp = "";
				i += (unsigned)sep.length( ) - 1;
			}
		}
		if( tmp != "" ) l.push_back( tmp );

		return l;
	}
};