#pragma once


#include <windows.h>
#include <string>
#include <sstream>
#include <vector>
#include "..\shared\kts.h"
#include "..\shared\KConsole.hxx"

//kts_<client_ip>_<kts_port>_<pid>_<state>_<user>_<date>
//kts_127.0.0.1_1234_912_started_
//kts_127.0.0.1_1234_912_logged_kpym
//kts_127.0.0.1_1234_912_shell_kpym
//kts_127.0.0.1_1234_912_proxy_kpym
//kts_127.0.0.1_1234_912_disconnected_kpym
//kts_127.0.0.1_1234_912_close_kpym

class KSessionState
{
#define STATE_STARTED		"started"
#define STATE_LOGGED		"logged"
#define STATE_SHELL			"shell"
#define STATE_SFTP			"sftp"
#define STATE_PROXY			"proxy"
#define STATE_PIPE			"pipe"
#define STATE_DISCONNECTED	"disconnected"
#define STATE_CLOSED		"closed"
#define STATE_ZOMBIE		"zombie"
public:
	/*==============================================================================
	 * session state struct
	 *=============================================================================*/
	struct KSESSION_STATE
	{
		std::string date;
		DWORD pid;
		std::string ip;
		std::string port;

		// load struct from string
		void FromString( const std::string & str )
		{
			ktrace_in( );
			ktrace( "KSESSION_STATE::FromString( )" );

			this->pid = 0;

			//kts_<client_ip>_<kts_port>_<pid>_<state>_<user>_<date>
			std::vector<int> sep;
			for( unsigned i = 0; i < str.length( ); i++ )
			{
				if( str[ i ] == '_' ) sep.push_back( i );
			}

			if( sep.size( ) != 6 )
			{
				kerror( "wrong seps number" );
				return;
			}

			// ip
			this->ip = str.substr( sep[ 0 ] + 1, sep[ 1 ] - sep[ 0 ] - 1 );
			ktrace( "ip = " << this->ip );
			// port
			this->port = str.substr( sep[ 1 ] + 1, sep[ 2 ] - sep[ 1 ] - 1 );
			ktrace( "port = " << this->port );
			// pid
			this->pid = atoi( str.substr( sep[ 2 ] + 1, sep[ 3 ] - sep[ 2 ] - 1 ).c_str( ) );
			ktrace( "pid = " << this->pid );
			// date
			this->date = str.substr( sep[ 5 ] + 1 );
			ktrace( "date = " << this->date );
		}
	};

private:
	/*==============================================================================
	 * vars
	 *=============================================================================*/
	std::string state;
	std::string client_ip;
	std::string client_port;
	std::string user;
	std::string session_dir;
	std::string pid;
	std::string date;
	CRITICAL_SECTION cs;

public:
	/*==============================================================================
	 * choose disconnected session
	 *=============================================================================*/
	std::string ChooseDisconnectedSession( bool auto_reconnect )
	{
		ktrace_in( );
		ktrace( "KSessionState::ChooseDisconnectedSession( )" );

		system( "cls" );

		unsigned i = 0;
		std::vector<std::string> sessions1 = this->FindDisconnectedSessions( this->session_dir, this->user );
		if( sessions1.size() == 0 ) return "";

		// filter zombie states
		std::vector<std::string> sessions;
		for( i = 0; i < sessions1.size( ); i++ )
		{
			KSESSION_STATE ks;
			ks.FromString( sessions1[ i ] );

			if( !this->IsSessionAlive(ks.pid) )
			{
				this->SetStateZombie("kts" + ks.ip + "_" + ks.port);
				continue;
			}
			sessions.push_back(sessions1[i]);
		}

		if( sessions.size() == 0 ) return "";

		if( auto_reconnect )
		{
			// auto reconnect
			if( sessions.size( ) == 1 )
			{
				KSESSION_STATE ks;
				ks.FromString( sessions[ 0 ] );

				return "kts" + ks.ip + "_" + ks.port;
			}
		}

		// we have sessions so print them on screen
		KConsole console;

		console.Write( "\r\n" );
		console.Write( "   ======================================\r\n" );
		console.Write( "   KpyM Telnet/SSH Server Session Manager\r\n" );
		console.Write( "   ======================================\r\n" );
		console.Write( "\r\n" );
		console.Write( "   0\tStart new session\r\n" );
		std::stringstream s;
		for( i = 0; i < sessions.size( ); i++ )
		{
			KSESSION_STATE ks;
			ks.FromString( sessions[ i ] );

			s.str( "" );
			s << "   " << ( char )( '1' + i ) << "\tConnect to session: " << ks.ip << ":" << ks.port << " " << ks.pid << " " << ks.date << "\r\n";
			console.Write( s.str( ) );
		}
		console.Write( "\r\n" );
		s.str( "" );
		s << "   Enter your choice [0-" << ( char )( '0' + i ) << "]: ";
		console.Write( s.str( ) );
		int key = 0;
		while( true )
		{
			console.ReadKey( key );
			if( key >= ( int )'0' && key <= ( int )( '0' + i ) ) break;
		}

		system( "cls" );

		if( key == '0' ) return "";

		KSESSION_STATE ks;
		ks.FromString( sessions[ key - '1' ] );

		return "kts" + ks.ip + "_" + ks.port;
	}

private:
	/*==============================================================================
	 * check if session is alive
	 *=============================================================================*/
	bool IsSessionAlive( DWORD pid )
	{
		ktrace_in( );
		ktrace( "KSessionState::IsSessionAlive( " << pid << " )" );

		HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, false, pid);
		if( process == NULL )
		{
			DWORD err = GetLastError();
			// no such process
			if(err == ERROR_INVALID_PARAMETER ) return false;
			kerror("can't open process " << pid << " err " << err);
			return false;
		}
		DWORD exitCode = 0;
		if( !GetExitCodeProcess( process, &exitCode ) )
		{
			CloseHandle(process);

			kerror( "can't get exit code" );
			return false;
		}
		CloseHandle(process);

		if( exitCode == STILL_ACTIVE )
		{
			return true;
		}
		return false;
	}

private:
	/*==============================================================================
	 * find session files
	 *=============================================================================*/
	std::vector<std::string> FindSessionFiles( )
	{
		ktrace_in( );
		ktrace( "KSessionState::FindSessionFiles( )" );

		std::vector<std::string> res;

		//kts_<client_ip>_<kts_port>_<pid>_<state>_<user>_<date>
		std::string path =
			  this->session_dir + "\\kts_"
			+ this->client_ip + "_"
			+ this->client_port + "_"
			+ this->pid + "_*_" 
			+ this->user + "_*";

		ktrace( "path = " << path );

		WIN32_FIND_DATA FindFileData;
		
		HANDLE hFind = FindFirstFile( path.c_str( ), &FindFileData );

		if( hFind == INVALID_HANDLE_VALUE )
		{
			if( GetLastError( ) == ERROR_FILE_NOT_FOUND ) return( res );

			kerror( "can't FindFirstFile" );
			return( res );
		}

		res.push_back( FindFileData.cFileName );

		while( FindNextFile( hFind, &FindFileData ) != 0 ) res.push_back( FindFileData.cFileName );

		DWORD dwError = GetLastError( );
		FindClose(hFind);
		if( dwError != ERROR_NO_MORE_FILES )
		{
			res.clear( );
			kerror( "can't FindNextFile" );
			return( res );
		}

		return( res );
	}

private:
	/*==============================================================================
	 * find disconnected files for user
	 *=============================================================================*/
	std::vector<std::string> FindDisconnectedSessions( std::string session_dir, std::string user )
	{
		ktrace_in( );
		ktrace( "KSessionState::FindDisconnectedSessions( " << user << " )" );

		std::vector<std::string> res;

		//kts_<client_ip>_<kts_port>_<pid>_<state>_<user>_<date>
		std::string path = session_dir + "\\kts_*_*_*_" + STATE_DISCONNECTED + "_" + user + "_*";

		WIN32_FIND_DATA FindFileData;
		
		HANDLE hFind = FindFirstFile( path.c_str( ), &FindFileData );

		if( hFind == INVALID_HANDLE_VALUE )
		{
			if( GetLastError( ) == ERROR_FILE_NOT_FOUND ) return( res );

			kerror( "can't FindFirstFile" );
			return( res );
		}

		res.push_back( FindFileData.cFileName );

		while( FindNextFile( hFind, &FindFileData ) != 0 ) res.push_back( FindFileData.cFileName );

		DWORD dwError = GetLastError( );
		FindClose(hFind);
		if( dwError != ERROR_NO_MORE_FILES )
		{
			res.clear( );
			kerror( "can't FindNextFile" );
			return( res );
		}

		return( res );
	}

public:
	/*==============================================================================
	 * find disconnected files for user
	 *=============================================================================*/
	void DeleteAllSessions( std::string session_dir )
	{
		ktrace_in( );
		ktrace( "KSessionState::DeleteAllSessions( " << session_dir << " )" );


		//kts_<client_ip>_<kts_port>_<pid>_<state>_<user>_<date>
		std::string path = session_dir + "\\kts_*_*_*_*_*_*";
		std::string file;

		WIN32_FIND_DATA FindFileData;

		HANDLE hFind = FindFirstFile( path.c_str( ), &FindFileData );

		if( hFind == INVALID_HANDLE_VALUE )
		{
			if( GetLastError( ) == ERROR_FILE_NOT_FOUND ) return;

			kerror( "can't FindFirstFile" );
			return;
		}

		file = session_dir + "\\" + std::string( FindFileData.cFileName );
		if( !DeleteFile( file.c_str( ) ) ) kerror( "can't delete " << std::string( FindFileData.cFileName ) );

		while( FindNextFile( hFind, &FindFileData ) != 0 ) 
		{
			file = session_dir + "\\" + std::string( FindFileData.cFileName );
			if( !DeleteFile( file.c_str( ) ) ) kerror( "can't delete " << std::string( FindFileData.cFileName ) );
		}

		DWORD dwError = GetLastError( );
		FindClose(hFind);
		if( dwError != ERROR_NO_MORE_FILES )
		{
			kerror( "can't FindNextFile" );
		}
	}
public:
	/*==============================================================================
	 * constructor
	 *=============================================================================*/
	KSessionState( )
	{
		ktrace_in( );
		ktrace( "KSessionState::KSessionState( )" );

		std::stringstream s;
		s << GetCurrentProcessId( );

		this->pid = s.str( );

		this->state = STATE_STARTED;

		this->GetDate( );

		InitializeCriticalSection( &this->cs );
	}

public:
	/*==============================================================================
	 * set session directory
	 *=============================================================================*/
	void SetSessionDirectory( std::string session_dir )
	{
		ktrace_in( );
		ktrace( "KSessionState::KSessionState( " << session_dir << " )" );

		this->session_dir = session_dir;
	}

private:
	/*==============================================================================
	 * combine the file name
	 *=============================================================================*/
	void GetDate( )
	{
		// get the time
		std::stringstream s;
		SYSTEMTIME time;
		
		// GetSystemTime( &time );
		GetLocalTime( &time );
		s << time.wYear 
			<< "-" << std::setfill( '0' ) << std::setw( 2 ) << time.wMonth 
			<< "-" << std::setfill( '0' ) << std::setw( 2 ) << time.wDay 
			<< " " << std::setfill( '0' ) << std::setw( 2 ) << time.wHour << "h" 
			<< " " << std::setfill( '0' ) << std::setw( 2 ) << time.wMinute << "m" 
			<< " " << std::setfill( '0' ) << std::setw( 2 ) << time.wSecond << "s"; 
		this->date = s.str( );
	}

private:
	/*==============================================================================
	 * combine the file name
	 *=============================================================================*/
	std::string CombineFileName( )
	{
		ktrace_in( );
		ktrace( "KSessionState::CombineFileName( )" );


		// .\active-sessions\kts_<client_ip>_<kts_port>_<pid>_<state>_<user>
		std::string name = this->session_dir + "\\kts_" + this->client_ip 
			+ "_" + this->client_port
			+ "_" + this->pid
			+ "_" + this->state
			+ "_" + this->user
			+ "_" + this->date;

		ktrace( "name = " << name );

		return name;
	}

private:
	/*==============================================================================
	 * create state file
	 *=============================================================================*/
	bool CreateStateFile( )
	{
		ktrace_in( );
		ktrace( "KSessionState::CreateStateFile( )" );

		if( this->state == STATE_CLOSED ) return false;

		std::string name = this->CombineFileName( );

		HANDLE file = CreateFile( name.c_str(), 0, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( file == INVALID_HANDLE_VALUE ) return false;

		CloseHandle( file );
		return true;
	}

private:
	/*==============================================================================
	 * create state file
	 *=============================================================================*/
	bool CreateStateFile( const std::string & name )
	{
		ktrace_in( );
		ktrace( "KSessionState::CreateStateFile( " << name << " )" );

		std::string file = this->session_dir + "\\" + name;
		HANDLE hfile = CreateFile( file.c_str(), 0, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( hfile == INVALID_HANDLE_VALUE ) return false;

		CloseHandle( hfile );
		return true;
	}

private:
	/*==============================================================================
	 * delete state file
	 *=============================================================================*/
	void DeleteStateFile( )
	{
		ktrace_in( );
		ktrace( "KSessionState::DeleteStateFile( )" );

		std::vector<std::string> files = this->FindSessionFiles( );

		for( unsigned int i = 0; i < files.size(); i++ )
		{
			this->DeleteStateFile( files[i] );
		}
	}

private:
	/*==============================================================================
	 * delete state file
	 *=============================================================================*/
	void DeleteStateFile( const std::string & name )
	{
		ktrace_in( );
		ktrace( "KSessionState::DeleteStateFile( " << name << " )" );

		std::string file = this->session_dir + "\\" + name;

		for( int i = 0; i < 10; i++ )
		{
			if( DeleteFile( file.c_str( ) ) ) return;
			else
			{
				// file does not exist
				if( GetLastError() == ERROR_FILE_NOT_FOUND ) return;
			}
			Sleep( 100 );
		}
		kerror( "can't delete file " << name );
	}
public:
	/*==============================================================================
	 * get session state
	 *=============================================================================*/
	std::string GetState( )
	{
		ktrace_in( );
		ktrace( "KSessionState::GetState( )" );

		return this->state;
	}

public:
	/*==============================================================================
	 * set session state to started
	 *=============================================================================*/
	bool SetStateStarted( std::string client_ip, std::string client_port )
	{
		ktrace_in( );
		ktrace( "KSessionState::SetStateStarted( " << client_ip << ", " << client_port << " )" );

		if( this->state == STATE_CLOSED ) return false;

		EnterCriticalSection( &this->cs );

		this->client_ip = client_ip;
		this->client_port = client_port;

		bool ret = this->CreateStateFile( );

		LeaveCriticalSection( &this->cs );

		return ret;
	}

public:
	/*==============================================================================
	 * set session state to logged
	 *=============================================================================*/
	bool SetStateLogged( std::string user )
	{
		ktrace_in( );
		ktrace( "KSessionState::SetStateLogged( )" );

		if( this->state == STATE_CLOSED ) return false;

		EnterCriticalSection( &this->cs );

		this->DeleteStateFile( );

		this->user = this->NormalizeUser( user );
		this->state = STATE_LOGGED;

		bool ret = this->CreateStateFile( );

		LeaveCriticalSection( &this->cs );

		return ret;
	}

public:
	/*==============================================================================
	 * set session state to shell
	 *=============================================================================*/
	bool SetStateShell( )
	{
		ktrace_in( );
		ktrace( "KSessionState::SetStateShell( )" );

		if( this->state == STATE_CLOSED ) return false;

		EnterCriticalSection( &this->cs );

		this->DeleteStateFile( );

		this->state = STATE_SHELL;

		bool ret = this->CreateStateFile( );

		LeaveCriticalSection( &this->cs );

		return ret;
	}

public:
	/*==============================================================================
	 * set session state to sftp
	 *=============================================================================*/
	bool SetStateSftp( )
	{
		ktrace_in( );
		ktrace( "KSessionState::SetStateSftp( )" );

		if( this->state == STATE_CLOSED ) return false;

		EnterCriticalSection( &this->cs );

		this->DeleteStateFile( );

		this->state = STATE_SFTP;

		bool ret = this->CreateStateFile( );

		LeaveCriticalSection( &this->cs );

		return ret;
	}
public:
	/*==============================================================================
	 * set session state to proxy
	 *=============================================================================*/
	bool SetStateProxy( )
	{
		ktrace_in( );
		ktrace( "KSessionState::SetStateProxy( )" );

		if( this->state == STATE_CLOSED ) return false;

		EnterCriticalSection( &this->cs );

		this->DeleteStateFile( );

		this->state = STATE_PROXY;

		bool ret = this->CreateStateFile( );

		LeaveCriticalSection( &this->cs );

		return ret;
	}

private:
	/*==============================================================================
	 * get session by pipe name
	 *=============================================================================*/
	std::string GetSessionByPipeName( std::string pipe_name )
	{
		ktrace_in( );
		ktrace( "KSessionState::SetStateZombie( )" );

		// pipe: kts<client_ip>_<kts_port>
		// sess: kts_<client_ip>_<kts_port>_<pid>_<state>_<user>_<date>

		std::string session = "";
		std::string prefix = pipe_name.insert(3, "_");
		std::vector<std::string> sessions = this->FindDisconnectedSessions(this->session_dir, this->user);

		// no active sessions
		if( sessions.size() == 0 ) return "";

		for(unsigned int i = 0; i < sessions.size(); i++)
		{
			if( sessions[i].length( ) <= prefix.length( ) ) continue;
			if( prefix == sessions[i].substr( 0, prefix.length( ) ) )
			{
				if( sessions[i].find("_"STATE_ZOMBIE"_") != std::string::npos ) continue;

				// we have found the session
				session = sessions[i];
				break;
			}
		}

		return session;
	}

public:
	/*==============================================================================
	 * set to zombie by pipe name
	 *=============================================================================*/
	bool SetStateZombie( std::string pipe_name )
	{
		ktrace_in( );
		ktrace( "KSessionState::SetStateZombie( )" );

		std::string session = this->GetSessionByPipeName( pipe_name );

		ktrace( "session = " << session );

		// can't find zombie session
		if(session == "") return true;


		this->DeleteStateFile( session );

		// .\active-sessions\kts_<client_ip>_<kts_port>_<pid>_<state>_<user>
		std::string zombie =  "";

		if( session.find("_"STATE_DISCONNECTED"_") != std::string::npos )
		{
			zombie = KWinsta::ReplaceString(session, "_"STATE_DISCONNECTED"_", "_"STATE_ZOMBIE"_");
		}
		else if( session.find("_"STATE_STARTED"_") != std::string::npos )
		{
			zombie = KWinsta::ReplaceString(session, "_"STATE_STARTED"_", "_"STATE_ZOMBIE"_");
		}
		else if( session.find("_"STATE_LOGGED"_") != std::string::npos )
		{
			zombie = KWinsta::ReplaceString(session, "_"STATE_LOGGED"_", "_"STATE_ZOMBIE"_");
		}
		else if( session.find("_"STATE_SHELL"_") != std::string::npos )
		{
			zombie = KWinsta::ReplaceString(session, "_"STATE_SHELL"_", "_"STATE_ZOMBIE"_");
		}
		else if( session.find("_"STATE_SFTP"_") != std::string::npos )
		{
			zombie = KWinsta::ReplaceString(session, "_"STATE_SFTP"_", "_"STATE_ZOMBIE"_");
		}
		else if( session.find("_"STATE_PROXY"_") != std::string::npos )
		{
			zombie = KWinsta::ReplaceString(session, "_"STATE_PROXY"_", "_"STATE_ZOMBIE"_");
		}
		else if( session.find("_"STATE_PIPE"_") != std::string::npos )
		{
			zombie = KWinsta::ReplaceString(session, "_"STATE_PIPE"_", "_"STATE_ZOMBIE"_");
		}
		else if( session.find("_"STATE_CLOSED"_") != std::string::npos )
		{
			zombie = KWinsta::ReplaceString(session, "_"STATE_CLOSED"_", "_"STATE_ZOMBIE"_");
		}

		ktrace( "zombie = " << zombie );
		if( zombie == "" ) 
		{
			kerror( "can't combine zombie session" );
			return false;
		}
		return this->CreateStateFile( zombie );
	}

public:
	/*==============================================================================
	 * set session state to disconnected
	 *=============================================================================*/
	bool SetStateDisconnected( )
	{
		ktrace_in( );
		ktrace( "KSessionState::SetStateDisconnected( )" );

		if( this->state == STATE_CLOSED ) return false;

		EnterCriticalSection( &this->cs );

		this->DeleteStateFile( );

		this->state = STATE_DISCONNECTED;

		bool ret = this->CreateStateFile( );

		LeaveCriticalSection( &this->cs );

		return ret;
	}

public:
	/*==============================================================================
	 * set session state to pipe
	 *=============================================================================*/
	bool SetStatePipe( )
	{
		ktrace_in( );
		ktrace( "KSessionState::SetStatePipe( )" );

		if( this->state == STATE_CLOSED ) return false;

		EnterCriticalSection( &this->cs );

		this->DeleteStateFile( );

		this->state = STATE_PIPE;

		bool ret = this->CreateStateFile( );

		LeaveCriticalSection( &this->cs );

		return ret;
	}
public:
	/*==============================================================================
	 * set session state to closed
	 *=============================================================================*/
	bool SetStateClosed( )
	{
		ktrace_in( );
		ktrace( "KSessionState::SetStateClosed( )" );

		EnterCriticalSection( &this->cs );

		this->DeleteStateFile( );

		this->state = STATE_CLOSED;

		LeaveCriticalSection( &this->cs );

		return true;
	}

private:
	/*==============================================================================
	 * convert domain\user to user@domain
	 *=============================================================================*/
	std::string NormalizeUser( std::string user )
	{
		ktrace_in( );
		ktrace( "KSessionState::NormalizeUser( " << user << " )" );

		if( user.find( "@" ) != std::string::npos ) return user;

		size_t pos = user.find("\\");

		if( pos == std::string::npos ) 
		{
			char computer_name[ MAX_COMPUTERNAME_LENGTH + 3 ];
			DWORD len = MAX_COMPUTERNAME_LENGTH + 1;

			if( !GetComputerName( computer_name, &len ) )
			{
				kerror( "can't get computer name" );
				return( user );
			}
			computer_name[ len ] = 0;
			std::string user1 = user + "@" + std::string( computer_name );

			KWinsta::ToLower( user1 );
			return user1;
		}

		std::string user1 = user.substr( pos + 1 ) + "@" + user.substr( 0, pos );

		KWinsta::ToLower( user1 );
		return user1;
	}
};
