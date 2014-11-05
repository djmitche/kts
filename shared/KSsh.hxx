#pragma once

#include <string>
//#include <Winsock2.h>
#include "..\shared\kts.h"
#include "..\shared\KTrace.hxx"
#define KPYM_HACK
#include "..\shared\cl\cryptlib.h"
#include "..\shared\KWinsta.hxx"

const static char my_pass[] = "klsdfij2SD:LM@)(3kmvei230-ifsd;lf12-=9";
const static char my_file[] = ".\\private.ky";

class KSsh
{
	/*==============================================================================
	 * var
	 *=============================================================================*/
private:
	CRYPT_SESSION cryptSession;
	int login_attempts;
	SOCKET sock;
	bool is_shutdown;
public:
	SHORT screenWidth;
	SHORT screenHeight;
	std::string username;
	std::string password;
	std::string client;
	std::string publickey;

	int status;
	int channel;

public:
	/*==============================================================================
	 * constructor
	 *=============================================================================*/
	KSsh( bool use = true ) : sock(0)
	{
		if( !use ) return;

		ktrace_in( );
		ktrace( "KSsh::KSsh( )" );
		// init cryptlib
		int status = cryptInit( );
		if( status != CRYPT_OK ) 
		{
			kerror( "can't init cryptlib" );
			klog( "can't init cryptlib" );
			LogErrorMessage();
		}

		status = cryptAddRandom( NULL, CRYPT_RANDOM_SLOWPOLL );
		if( status != CRYPT_OK )
		{
			kerror( "can't add cryptlib random" );
			klog( "can't add cryptlib random" );
			LogErrorMessage();
		}
		this->login_attempts = 0;
		this->is_shutdown = false;
	}

public:
	/*==============================================================================
	 * destructor
	 *=============================================================================*/
	~KSsh( )
	{
		ktrace_in( );
		ktrace( "KSsh::~KSsh( )" );

		this->ShutDown( );
	}
public:
	/*==============================================================================
	 * set algo lists
	 *=============================================================================*/
	void SetAlgoLists(std::string kexAlgoList, std::string encrAlgoList, std::string macAlgoList)
	{
		const unsigned max_arr = 20;
		std::vector<std::string> list;
		int arr[max_arr];
		unsigned i = 0;

		// kex algo list
		list = KWinsta::SplitString(kexAlgoList, ",");
		for( i = 0; i < list.size() && i < max_arr - 2; i++ )
		{
			arr[ i ] = cryptAlgoFromString(list[i].c_str());
		}
		cryptSetAlgoList(arr, i, KEYEX_ALGO_LIST );
		
		// encr algo list
		list = KWinsta::SplitString(encrAlgoList, ",");
		for( i = 0; i < list.size() && i < max_arr - 2; i++ )
		{
			arr[ i ] = cryptAlgoFromString(list[i].c_str());
		}
		cryptSetAlgoList(arr, i, ENCR_ALGO_LIST );

		// mac algo list
		list = KWinsta::SplitString(macAlgoList, ",");
		for( i = 0; i < list.size() && i < max_arr - 2; i++ )
		{
			arr[ i ] = cryptAlgoFromString(list[i].c_str());
		}
		cryptSetAlgoList(arr, i, MAC_ALGO_LIST );


	}

public:
	/*==============================================================================
	 * shut down ssh session
	 *=============================================================================*/
	void ShutDown( )
	{
		ktrace_in( );
		ktrace( "KSsh::ShutDown( )" );

		if( this->is_shutdown ) return;
		this->is_shutdown = true;

		this->status = cryptGetAttribute( this->cryptSession, CRYPT_SESSINFO_SSH_CHANNEL, &this->channel );
		if( this->status != CRYPT_OK )
		{
			ktrace( "cryptSetAttribute : err = " << this->status );
			this->channel = 1;
		}

		this->status = cryptSetAttribute( this->cryptSession, CRYPT_SESSINFO_SSH_CHANNEL, this->channel );
		if( this->status != CRYPT_OK )
		{
			ktrace( "cryptSetAttribute : err = " << this->status );
		}
		else
		{
			this->status = cryptSetAttribute( this->cryptSession, CRYPT_SESSINFO_SSH_CHANNEL_ACTIVE, 0 );
		}

		this->status = cryptDestroySession( this->cryptSession );

		if( this->sock != 0 ) 
		{
			// close gracefully
			char buff[ 1010 ];
			shutdown( this->sock, 1/*SD_SEND*/ );
			while( recv( this->sock, buff, 1000, 0 ) > 0 );

			closesocket( this->sock );
		}

		cryptEnd( );
	}
public:
	/*==============================================================================
	 * push data up the ssh chanel
	 *=============================================================================*/
	bool Push( const std::string & buff, const int channel )
	{
		ktrace_in( );
		ktrace( "KSsh::Push( " << channel << ", " << buff << " )" );

		unsigned offset = 0;
		while(true)
		{
			std::string chunk = buff.substr(offset, 2000);
			offset += 2000;
			if( !this->PushChunk(chunk, channel) ) return false;

			if(offset > buff.length()) break;
		}
		return true;
	}
public:
	/*==============================================================================
	 * push data up the ssh chanel
	 *=============================================================================*/
	bool Push( const std::string & buff )
	{
		ktrace_in( );
		ktrace( "KSsh::Push( " << buff << " )" );

		return this->Push( buff, this->channel );
	}

public:
	/*==============================================================================
	 * push chunk of data up the ssh chanel
	 *=============================================================================*/
	bool PushChunk( const std::string & buff, const int channel )
	{
		ktrace_in( );
		ktrace( "KSsh::PushChunk( " << buff << " )" );

		int len;

		this->status = cryptSetAttribute( this->cryptSession, CRYPT_SESSINFO_SSH_CHANNEL, channel );
		if( this->status != CRYPT_OK )
		{
			ktrace( "cryptSetAttribute : err = " << this->status );
			return( false );
		}

		if( buff.length() == 0 ) return( true );

		this->status = cryptPushData( this->cryptSession, buff.c_str( ), ( const int )buff.length( ), &len );
		if( this->status != CRYPT_OK )
		{
			ktrace( "cryptPushData : err = " << this->status );
			return( false );
		}
		this->status = cryptFlushData( this->cryptSession );
		if( this->status != CRYPT_OK )
		{
			ktrace( "cryptFlushData : err = " << this->status );
			return( false );
		}

		return( true );
	}

public:
	/*==============================================================================
	 * log the ssh error message
	 *=============================================================================*/
	void LogErrorMessage( )
	{
		ktrace_in( );
		ktrace( "KSsh::LogErrorMessage( )" );

		char error[1000];
		int len = -1;

		cryptGetAttributeString( this->cryptSession, CRYPT_ATTRIBUTE_INT_ERRORMESSAGE, error, &len);

		if( len < 0 ) return;

		error[len] = '\0';
		klog( "ssh info: " << error )
	}
public:
	/*==============================================================================
	 * pop data from the ssh chanel
	 *=============================================================================*/
	bool Pop( std::string & buff )
	{
		ktrace_in( );
		ktrace( "KSsh::Pop( )" );

		char buffer[ 60001 ];
		int len;

		this->status = cryptPopData( this->cryptSession, buffer, 60000, &len );
		if( this->status != CRYPT_OK && status != CRYPT_ERROR_TIMEOUT )
		{
			ktrace( "cryptPopData : err = " << this->status );
			return( false );
		}
		this->status = cryptGetAttribute( this->cryptSession, CRYPT_SESSINFO_SSH_CHANNEL, &this->channel );

		buff.assign( buffer, len );

		ktrace( "len = " << ( unsigned )buff.length( ) );
		return( true );
	}
public:
	/*==============================================================================
	 * close channel
	 *=============================================================================*/
	bool CloseChannel( int channel )
	{
		ktrace_in( );
		ktrace( "KSsh::CloseChannel( " << channel << " )" );

		this->status = cryptSetAttribute( this->cryptSession, CRYPT_SESSINFO_SSH_CHANNEL, channel );
		if( this->status != CRYPT_OK) 
		{
			ktrace( "cryptSetAttribute 1 : err = " << this->status );
			return false;
		}

		this->status = cryptSetAttribute( this->cryptSession, CRYPT_SESSINFO_SSH_CHANNEL_ACTIVE, 0 );
		if( this->status != CRYPT_OK) 
		{
			ktrace( "cryptSetAttribute 2 : err = " << this->status );
			return false;
		}
		return true;
	}

public:
	/*==============================================================================
	 * get channel type and arg
	 *=============================================================================*/
	bool GetChannelTypeAndArg( std::string & type, std::string & arg )
	{
		ktrace_in( );
		ktrace( "KSsh::GetChannelTypeAndArg( )" );

		// channel stuff
		char type1[ 1000 ];
		char arg1[ 1000 ];
		int typeLen = -1;
		int argLen = -1;

		this->status = cryptGetAttribute( this->cryptSession, CRYPT_SESSINFO_SSH_CHANNEL, &this->channel );
		this->status = cryptGetAttributeString( this->cryptSession, CRYPT_SESSINFO_SSH_CHANNEL_TYPE, type1, &typeLen );
		this->status = cryptGetAttributeString( this->cryptSession, CRYPT_SESSINFO_SSH_CHANNEL_ARG1, arg1, &argLen );
		if( argLen < 0 ) argLen = 0;
		if( typeLen < 0 ) typeLen = 0;
		type1[ typeLen ] = 0;
		arg1[ argLen ] = 0;

		type.assign(type1);
		arg.assign(arg1);

		return( true );
	}

public:
	/*==============================================================================
	 * get screen size
	 *=============================================================================*/
	void ScreenSize( )
	{
#define CRYPT_KPYMINFO_SSH_TERMINAL_WIDTH	60001
#define CRYPT_KPYMINFO_SSH_TERMINAL_HEIGHT	60002
		int width;
		int height;
		cryptGetAttribute( cryptSession, ( CRYPT_ATTRIBUTE_TYPE )CRYPT_KPYMINFO_SSH_TERMINAL_WIDTH, &width );
		cryptGetAttribute( cryptSession, ( CRYPT_ATTRIBUTE_TYPE )CRYPT_KPYMINFO_SSH_TERMINAL_HEIGHT, &height );
		this->screenWidth = (SHORT)width;
		this->screenHeight =(SHORT) height;
	}


public:
	/*==============================================================================
	 * login ssh user - wrapper
	 *=============================================================================*/
	bool Login( bool logged = false )
	{
		ktrace_in( );
		ktrace( "KSsh::Login( " << logged <<" )" );

		if( logged ) 
		{
			// login ok
			this->status = cryptSetAttribute( this->cryptSession, CRYPT_SESSINFO_AUTHRESPONSE,1 );
			if( this->status != CRYPT_OK )
			{
				kerror( "cryptSetAttribute3 = " << status );
				return false;
			}

			// complete handshake
			this->status = cryptSetAttribute( this->cryptSession, CRYPT_SESSINFO_ACTIVE, 1 );
			if( this->status != CRYPT_OK )
			{
				ktrace( "ssh not initialised 0. disconnecting" << status );
				return false;
			}

			// get the client info
			char buff[ 5000 ];
			int len = -1;
			#define CRYPT_KPYMINFO_CLIENT_ID	60003

			this->status = cryptGetAttributeString( this->cryptSession, ( CRYPT_ATTRIBUTE_TYPE )CRYPT_KPYMINFO_CLIENT_ID, buff, &len );
			if( len < 0 || len >= CRYPT_MAX_TEXTSIZE ) len = 0;
			buff[ len ] = '\0';
			this->client.assign(buff);


			this->status = cryptGetAttribute( this->cryptSession, CRYPT_SESSINFO_SSH_CHANNEL, &this->channel );
			
			return true;
		}
		else
		{
			if( this->login_attempts > 0 )
			{
				// login incorrect
				this->status = cryptSetAttribute( this->cryptSession, CRYPT_SESSINFO_AUTHRESPONSE,0 );
				if( status != CRYPT_OK )
				{
					kerror( "cryptSetAttribute3 = " << status );
					return false;
				}
			}
		}

		// server client handshake
		this->status = cryptSetAttribute( this->cryptSession, CRYPT_SESSINFO_ACTIVE, 1 );
		if( status != CRYPT_ENVELOPE_RESOURCE )
		{
			ktrace( "ssh not initialised 1. disconnecting" << status );
			return false;
		}

		// get session user name
		char buff[ 5000 ];
		int len = 0;

		len = -1;
		this->status = cryptGetAttributeString( this->cryptSession, CRYPT_SESSINFO_USERNAME, buff, &len );
		if( status != CRYPT_OK )
		{
			kerror( "cryptGetAttributeString user = " << status );
			return false;
		}
		if(len < 0 || len > 5000 ) len = 0;
		this->username.assign( buff, len );

		ktrace( "username = " << this->username );

		// get session password
		len = -1;
		this->status = cryptGetAttributeString( this->cryptSession, CRYPT_SESSINFO_PASSWORD, buff, &len );
		if( status != CRYPT_OK )
		{
			kerror( "cryptGetAttributeString pass = " << status );
			return false;
		}
		if(len < 0 || len > 5000 ) len = 0;
		this->password.assign( buff, len );

		// get publickey
		#define CRYPT_KPYMINFO_PUBLICKEY 60004
		len = -1;
		this->status = cryptGetAttributeString( this->cryptSession, (CRYPT_ATTRIBUTE_TYPE)CRYPT_KPYMINFO_PUBLICKEY, buff, &len );
		if( status != CRYPT_OK )
		{
			kerror( "cryptGetAttributeString publickey = " << status );
			return false;
		}
		if(len < 0 || len > 5000 ) len = 0;
		this->publickey.assign( buff, len );

		this->login_attempts ++;

		return true;
	}
public:
	/*==============================================================================
	 * check if this is port forward request
	 *=============================================================================*/
	bool GetPortForwardRequest(std::string & request)
	{
		ktrace_in( );
		ktrace( "KSsh::GetPortForwardRequest( )" );

		if( this->status != CRYPT_ENVELOPE_RESOURCE ) return false;

		std::string type;

		this->GetChannelTypeAndArg(type, request);
		if( type != "direct-tcpip" ) return false;

		ktrace("pf request is " << request);
		return true;
	}
public:
	/*==============================================================================
	 * check if session is sftp
	 *=============================================================================*/
	bool IsSftp(std::string packet)
	{
		ktrace_in( );
		ktrace( "KSsh::IsSftp( " << (unsigned int)packet.length() << " )" );

		std::string type;
		std::string arg;

		this->GetChannelTypeAndArg(type, arg);

		if(type == "subsystem" && arg == "sftp" ) return true;

		// bug slashed by drakkar
		if( packet.length() < (unsigned int)9) return false;

		if( (unsigned char)packet[0] == 0
		&&  (unsigned char)packet[1] == 0
		&&  (unsigned char)packet[2] == 0
		&&  (unsigned char)packet[3] == 5 /*len = 5*/
		&&  (unsigned char)packet[4] == 1 /*type = 1*/
		&&  (unsigned char)packet[5] == 0
		&&  (unsigned char)packet[6] == 0
		&&  (unsigned char)packet[7] == 0
		&&  (unsigned char)packet[8] != 0 /*version = any*/) return true;

		return false;
	}

public:
	/*==============================================================================
	 * create ssh session associated with socket
	 *=============================================================================*/
	bool Init( SOCKET sock, std::string file = my_file, std::string pass = my_pass )
	{
		ktrace_in( );
		ktrace( "KSsh::Init( " << ( int )sock << ", " << file << " )" );

		int status = 0;
		this->sock = sock;

		// load private RSA key
		CRYPT_CONTEXT privKey;

		CRYPT_KEYSET keySet;
		status = cryptKeysetOpen(&keySet, CRYPT_UNUSED, CRYPT_KEYSET_FILE, file.c_str( ), CRYPT_KEYOPT_READONLY );
		if( status != CRYPT_OK )
		{
			kerror( "cryptKeysetOpen = " << status );
			return false;
		}

		status = cryptGetPrivateKey( keySet, &privKey, CRYPT_KEYID_NAME, "RSA_KEY", pass.c_str( ) );
		if( status != CRYPT_OK )
		{
			kerror( "cryptGetPrivateKey = " << status );
			return false;
		}

		status = cryptKeysetClose( keySet );
		if( status != CRYPT_OK )
		{
			kerror( "cryptKeysetClose = " << status );
			return false;
		}

		// create the session and add private key
		status = cryptCreateSession( &( this->cryptSession ), CRYPT_UNUSED, CRYPT_SESSION_SSH_SERVER );
		if( status != CRYPT_OK )
		{
			kerror( "cryptCreateSession = " << status );
			return false;
		}

		status = cryptSetAttribute( this->cryptSession, CRYPT_SESSINFO_PRIVATEKEY, privKey );
		if( status != CRYPT_OK )
		{
			kerror( "cryptSetAttribute1 = " << status );
			return false;
		}

		cryptDestroyContext( privKey );

		// add the socket
		status = cryptSetAttribute( this->cryptSession, CRYPT_SESSINFO_NETWORKSOCKET, ( int )sock );
		if( status != CRYPT_OK )
		{
			kerror( "cryptSetAttribute2 = " << status );
			return false;
		}

		return( true );
	}


public:
	/*==============================================================================
	 * create rsa key
	 *=============================================================================*/
	bool CreateRsaKey( std::string file = my_file, std::string pass = my_pass )
	{
		ktrace_in( );
		ktrace( "KSsh::CreateRsaKey( " << my_file << " )" );

		int status;
		// Create a context for RSA
		CRYPT_CONTEXT	privKeyContext;
		int		keyLen	= 128;	// 1024 bit

		status = cryptCreateContext(&privKeyContext, CRYPT_UNUSED, CRYPT_ALGO_RSA);
		if( status != CRYPT_OK )
		{
			kerror( "cryptCreateContext = " << status );
			LogErrorMessage();
			return( false );
		}

		// Set a label for the key
		status = cryptSetAttributeString(privKeyContext, CRYPT_CTXINFO_LABEL, "RSA_KEY", 7 );
		if( status != CRYPT_OK )
		{
			kerror( "cryptSetAttributeString = " << status );
			LogErrorMessage();
			return( false );
		}

		// Set key length
		status = cryptSetAttribute(privKeyContext, CRYPT_CTXINFO_KEYSIZE, keyLen);
		if( status != CRYPT_OK )
		{
			kerror( "cryptSetAttribute = " << status );
			LogErrorMessage();
			return( false );
		}

		// Generate a key
		status = cryptGenerateKey(privKeyContext);
		if( status != CRYPT_OK )
		{
			kerror( "cryptGenerateKey = " << status );
			LogErrorMessage();
			return( false );
		}

		// Open a Keysey file
		CRYPT_KEYSET	keySet;

		status = cryptKeysetOpen(&keySet, CRYPT_UNUSED, CRYPT_KEYSET_FILE, file.c_str( ), CRYPT_KEYOPT_CREATE);
		if( status != CRYPT_OK )
		{
			kerror( "cryptKeysetOpen = " << status );
			LogErrorMessage();
			return( false );
		}

		// Store the private key
		status = cryptAddPrivateKey(keySet, privKeyContext, pass.c_str( ) );
		if( status != CRYPT_OK )
		{
			kerror( "cryptAddPrivateKey = " << status );
			LogErrorMessage();
			return( false );
		}

		status = cryptKeysetClose( keySet );
		if( status != CRYPT_OK )
		{
			kerror( "cryptKeysetClose = " << status );
			LogErrorMessage();
			return( false );
		}

		if( !KWinsta::AddSystemFullControlToFile( file ) )
		{
			kerror( "AddSystemFullControlToFile(" << file << "):err" );
			LogErrorMessage();
			return( false );
		}

		return( true );
	}
};
