/****************************************************************************
*																			*
*						cryptlib SSHv2 Server Management					*
*						Copyright Peter Gutmann 1998-2008					*
*																			*
****************************************************************************/

#if defined( INC_ALL )
  #include "crypt.h"
  #include "misc_rw.h"
  #include "session.h"
  #include "ssh.h"
#else
  #include "crypt.h"
  #include "misc/misc_rw.h"
  #include "session/session.h"
  #include "session/ssh.h"
#endif /* Compiler-specific includes */

#ifdef USE_SSH

/****************************************************************************
*																			*
*								Utility Functions							*
*																			*
****************************************************************************/
#ifdef KPYM_HACK
int typeSize(const CRYPT_ALGO_TYPE t[]);
static CRYPT_ALGO_TYPE algoKeyexList[] = {
CRYPT_PSEUDOALGO_DHE, CRYPT_ALGO_DH, 
	CRYPT_ALGO_NONE, CRYPT_ALGO_NONE };
static CRYPT_ALGO_TYPE algoEncrList[] = {
	CRYPT_ALGO_3DES, /*CRYPT_ALGO_AES,*/ CRYPT_ALGO_BLOWFISH,
	CRYPT_ALGO_CAST, CRYPT_ALGO_IDEA, CRYPT_ALGO_RC4, 
	CRYPT_ALGO_NONE, CRYPT_ALGO_NONE };
static CRYPT_ALGO_TYPE algoMACList[] = {
	CRYPT_ALGO_HMAC_SHA, CRYPT_ALGO_HMAC_MD5, 
	CRYPT_ALGO_NONE, CRYPT_ALGO_NONE };

ALGO_STRING_INFO FAR_BSS algoStringKeyexTbl[] = {
{ "diffie-hellman-group-exchange-sha1", 34, CRYPT_PSEUDOALGO_DHE },
	{ "diffie-hellman-group-exchange-sha256", 36, CRYPT_PSEUDOALGO_DHE_ALT },
	{ "diffie-hellman-group1-sha1", 26, CRYPT_ALGO_DH },
	{ NULL, 0, CRYPT_ALGO_NONE }, { NULL, 0, CRYPT_ALGO_NONE }
	};

ALGO_STRING_INFO FAR_BSS algoStringEncrTblClient[] = {
	{ "3des-cbc", 8, CRYPT_ALGO_3DES },
	{ "aes128-cbc", 10, CRYPT_ALGO_AES },
	{ "blowfish-cbc", 12, CRYPT_ALGO_BLOWFISH },
	{ "cast128-cbc", 11, CRYPT_ALGO_CAST },
	{ "idea-cbc", 8, CRYPT_ALGO_IDEA },
	{ "arcfour", 7, CRYPT_ALGO_RC4 },
	{ NULL, 0, CRYPT_ALGO_NONE }, { NULL, 0, CRYPT_ALGO_NONE }
	};
ALGO_STRING_INFO FAR_BSS algoStringEncrTblServer[] = {
	{ "3des-cbc", 8, CRYPT_ALGO_3DES },
	{ "blowfish-cbc", 12, CRYPT_ALGO_BLOWFISH },
	{ "cast128-cbc", 11, CRYPT_ALGO_CAST },
	{ "idea-cbc", 8, CRYPT_ALGO_IDEA },
	{ "arcfour", 7, CRYPT_ALGO_RC4 },
	{ NULL, 0, CRYPT_ALGO_NONE }, { NULL, 0, CRYPT_ALGO_NONE }
	};

ALGO_STRING_INFO FAR_BSS algoStringMACTbl[] = {
	{ "hmac-sha1", 9, CRYPT_ALGO_HMAC_SHA },
	{ "hmac-md5", 8, CRYPT_ALGO_HMAC_MD5 },
	{ NULL, 0, CRYPT_ALGO_NONE }, { NULL, 0, CRYPT_ALGO_NONE }
	};

const ALGO_STRING_INFO cryptAlgoInfoFromString( const char * str )
{
	static const ALGO_STRING_INFO FAR_BSS table[] = {
		{ "ssh-rsa", 7, CRYPT_ALGO_RSA },
		{ "ssh-dss", 7, CRYPT_ALGO_DSA },
		{ "3des-cbc", 8, CRYPT_ALGO_3DES },
		{ "aes128-cbc", 10, CRYPT_ALGO_AES },
		{ "blowfish-cbc", 12, CRYPT_ALGO_BLOWFISH },
		{ "cast128-cbc", 11, CRYPT_ALGO_CAST },
		{ "idea-cbc", 8, CRYPT_ALGO_IDEA },
		{ "arcfour", 7, CRYPT_ALGO_RC4 },
		{ "diffie-hellman-group-exchange-sha1", 34, CRYPT_PSEUDOALGO_DHE },
		{ "diffie-hellman-group-exchange-sha256", 36, CRYPT_PSEUDOALGO_DHE_ALT },
		{ "diffie-hellman-group1-sha1", 26, CRYPT_ALGO_DH },
		{ "hmac-sha1", 9, CRYPT_ALGO_HMAC_SHA },
		{ "hmac-md5", 8, CRYPT_ALGO_HMAC_MD5 },
		{ "password", 8, CRYPT_PSEUDOALGO_PASSWORD },
		{ NULL, 0, CRYPT_ALGO_NONE }
		};
	static const ALGO_STRING_INFO none = { NULL, 0, CRYPT_ALGO_NONE };
	int i = 0;

	for( i = 0; table[ i ].algo != CRYPT_ALGO_NONE; i++ )
	{
		if( strcmp(str, table[i].name) == 0 ) return table[ i ];
	}
	return none;
}

const ALGO_STRING_INFO cryptAlgoInfoFromAlgo( int algo )
{
	static const ALGO_STRING_INFO FAR_BSS table[] = {
		{ "ssh-rsa", 7, CRYPT_ALGO_RSA },
		{ "ssh-dss", 7, CRYPT_ALGO_DSA },
		{ "3des-cbc", 8, CRYPT_ALGO_3DES },
		{ "aes128-cbc", 10, CRYPT_ALGO_AES },
		{ "blowfish-cbc", 12, CRYPT_ALGO_BLOWFISH },
		{ "cast128-cbc", 11, CRYPT_ALGO_CAST },
		{ "idea-cbc", 8, CRYPT_ALGO_IDEA },
		{ "arcfour", 7, CRYPT_ALGO_RC4 },
		{ "diffie-hellman-group-exchange-sha1", 34, CRYPT_PSEUDOALGO_DHE },
		{ "diffie-hellman-group1-sha1", 26, CRYPT_ALGO_DH },
		{ "hmac-sha1", 9, CRYPT_ALGO_HMAC_SHA },
		{ "hmac-md5", 8, CRYPT_ALGO_HMAC_MD5 },
		{ "password", 8, CRYPT_PSEUDOALGO_PASSWORD },
		{ NULL, 0, CRYPT_ALGO_NONE }
		};
	static const ALGO_STRING_INFO none = { NULL, 0, CRYPT_ALGO_NONE };
	int i = 0;

	for( i = 0; table[ i ].algo != CRYPT_ALGO_NONE; i++ )
	{
		if( algo == table[ i ].algo ) return table[ i ];
	}
	return none;
}


C_RET cryptAlgoFromString( const char * str )
{
	return cryptAlgoInfoFromString( str ).algo;
}

C_RET cryptSetAlgoList( const int * list, int len, int type )
{
	int i = 0;
	if( type == KEYEX_ALGO_LIST )
	{
		//algoKeyexList
		for( i = 0; i < len && i < 2; i++ )
		{
			algoKeyexList[i] = list[i];
			algoStringKeyexTbl[i] = cryptAlgoInfoFromAlgo( list[i] );
		}
		algoKeyexList[i] = CRYPT_ALGO_NONE;
		algoKeyexList[i + 1] = CRYPT_ALGO_NONE;
		algoStringKeyexTbl[i] = cryptAlgoInfoFromAlgo( CRYPT_ALGO_NONE );
		algoStringKeyexTbl[i + 1] = cryptAlgoInfoFromAlgo( CRYPT_ALGO_NONE );
	}
	if( type == ENCR_ALGO_LIST )
	{
		//algoEncrList
		for( i = 0; i < len && i < 5; i++ )
		{
			algoEncrList[i] = list[i];
			algoStringEncrTblClient[i] = cryptAlgoInfoFromAlgo( list[i] );
			algoStringEncrTblServer[i] = cryptAlgoInfoFromAlgo( list[i] );
		}
		algoEncrList[i] = CRYPT_ALGO_NONE;
		algoEncrList[i + 1] = CRYPT_ALGO_NONE;
		algoStringEncrTblClient[i] = cryptAlgoInfoFromAlgo( CRYPT_ALGO_NONE );
		algoStringEncrTblClient[i + 1] = cryptAlgoInfoFromAlgo( CRYPT_ALGO_NONE );
		algoStringEncrTblServer[i] = cryptAlgoInfoFromAlgo( CRYPT_ALGO_NONE );
		algoStringEncrTblServer[i + 1] = cryptAlgoInfoFromAlgo( CRYPT_ALGO_NONE );
	}
	if( type == MAC_ALGO_LIST )
	{
		//algoMACList
		for( i = 0; i < len && i < 2; i++ )
		{
			algoMACList[i] = list[i];
			algoStringMACTbl[i] = cryptAlgoInfoFromAlgo( list[i] );
		}
		algoMACList[i] = CRYPT_ALGO_NONE;
		algoMACList[i + 1] = CRYPT_ALGO_NONE;
		algoStringMACTbl[i] = cryptAlgoInfoFromAlgo( CRYPT_ALGO_NONE );
		algoStringMACTbl[i + 1] = cryptAlgoInfoFromAlgo( CRYPT_ALGO_NONE );
	}

	return 0;
}

static const CRYPT_ALGO_TYPE FAR_BSS algoStringUserauthentList[] = {
	  CRYPT_PSEUDOALGO_PASSWORD
	, CRYPT_PSEUDOALGO_PUBLICKEY
	, CRYPT_ALGO_NONE, CRYPT_ALGO_NONE };

#else
/* SSH algorithm names sent to the client, in preferred algorithm order.
   Since we have a fixed algorithm for our public key (determined by the key
   type) we only send a single value for this that's evaluated at runtime, 
   so there's no list for public-key algorithms.

   Note that the values in these lists must be present in the algorithm-name 
   mapping tables in ssh2.c */

static const CRYPT_ALGO_TYPE FAR_BSS algoKeyexList[] = {
	CRYPT_PSEUDOALGO_DHE, CRYPT_ALGO_DH, 
	CRYPT_ALGO_NONE, CRYPT_ALGO_NONE };
static const CRYPT_ALGO_TYPE FAR_BSS algoEncrList[] = {
	/* We can't list AES as an option because the peer can pick up anything
	   it wants from the list as its preferred choice which means that if
	   we're talking to any non-cryptlib implementation they always go for
	   AES even though it doesn't yet have the full provenance of 3DES */
	CRYPT_ALGO_3DES, /*CRYPT_ALGO_AES,*/ CRYPT_ALGO_BLOWFISH,
	CRYPT_ALGO_CAST, CRYPT_ALGO_IDEA, CRYPT_ALGO_RC4, 
	CRYPT_ALGO_NONE, CRYPT_ALGO_NONE };
static const CRYPT_ALGO_TYPE FAR_BSS algoMACList[] = {
	CRYPT_ALGO_HMAC_SHA, CRYPT_ALGO_HMAC_MD5, 
	CRYPT_ALGO_NONE, CRYPT_ALGO_NONE };

#endif

#ifdef KPYM_HACK
static SSH_HANDSHAKE_INFO * ke_handshakeInfo = NULL;
char ke_client_id[1000] = {0};
BYTE ke_sessionID[ CRYPT_MAX_HASHSIZE + 16 ];
int ke_sessionIDLength = -1;
char ke_publickey[ 5000 + 2 ];
int ke_publickey_len = -1;
#endif
/* Encode a list of available algorithms */

CHECK_RETVAL STDC_NONNULL_ARG( ( 1, 2 ) ) \
static int writeAlgoList( INOUT STREAM *stream, 
						  IN_ARRAY( algoListLength ) \
								const CRYPT_ALGO_TYPE *algoList,
						  IN_RANGE( 1, 16 ) const int algoListLength )
	{
	static const ALGO_STRING_INFO FAR_BSS algoStringMapTbl[] = {
		{ "ssh-rsa", 7, CRYPT_ALGO_RSA },
		{ "ssh-dss", 7, CRYPT_ALGO_DSA },
		{ "3des-cbc", 8, CRYPT_ALGO_3DES },
		{ "aes128-cbc", 10, CRYPT_ALGO_AES },
		{ "blowfish-cbc", 12, CRYPT_ALGO_BLOWFISH },
		{ "cast128-cbc", 11, CRYPT_ALGO_CAST },
		{ "idea-cbc", 8, CRYPT_ALGO_IDEA },
		{ "arcfour", 7, CRYPT_ALGO_RC4 },
		{ "diffie-hellman-group-exchange-sha1", 34, CRYPT_PSEUDOALGO_DHE },
		{ "diffie-hellman-group-exchange-sha256", 36, CRYPT_PSEUDOALGO_DHE_ALT },
		{ "diffie-hellman-group1-sha1", 26, CRYPT_ALGO_DH },
		{ "hmac-sha1", 9, CRYPT_ALGO_HMAC_SHA },
		{ "hmac-md5", 8, CRYPT_ALGO_HMAC_MD5 },
		{ "password", 8, CRYPT_PSEUDOALGO_PASSWORD },
#ifdef KPYM_HACK
        { "publickey", 9, CRYPT_PSEUDOALGO_PUBLICKEY },
#endif
		{ NULL, 0, CRYPT_ALGO_NONE }, { NULL, 0, CRYPT_ALGO_NONE }
		};
	int availAlgoIndex[ 16 + 8 ];
	int noAlgos = 0, length = 0, algoIndex, status;

	assert( isWritePtr( stream, sizeof( STREAM ) ) );
	assert( isReadPtr( algoList, sizeof( CRYPT_ALGO_TYPE ) * \
								 algoListLength ) );

	REQUIRES( algoListLength >= 1 && algoListLength <= 16 );

	/* Walk down the list of algorithms remembering the encoded name of each
	   one that's available for use */
	for( algoIndex = 0; \
		 algoList[ algoIndex ] != CRYPT_ALGO_NONE && \
			algoIndex < algoListLength && \
			algoIndex < FAILSAFE_ITERATIONS_SMALL;
		 algoIndex++ )
		{
		const CRYPT_ALGO_TYPE cryptAlgo = algoList[ algoIndex ];

		if( isPseudoAlgo( cryptAlgo ) || algoAvailable( cryptAlgo ) )
			{
			int i;

			for( i = 0; 
				 algoStringMapTbl[ i ].algo != CRYPT_ALGO_NONE && \
					algoStringMapTbl[ i ].algo != cryptAlgo && \
					i < FAILSAFE_ARRAYSIZE( algoStringMapTbl, ALGO_STRING_INFO ); 
				 i++ );
			ENSURES( i < FAILSAFE_ARRAYSIZE( algoStringMapTbl, \
											 ALGO_STRING_INFO ) );
			ENSURES( algoStringMapTbl[ i ].algo != CRYPT_ALGO_NONE && \
					 noAlgos >= 0 && noAlgos < 16 );
			availAlgoIndex[ noAlgos++ ] = i;
			length += algoStringMapTbl[ i ].nameLen;
			if( noAlgos > 1 )
				length++;			/* Room for comma delimiter */
			}
		}
	ENSURES( algoIndex < FAILSAFE_ITERATIONS_SMALL );

	/* Encode the list of available algorithms into a comma-separated string */
	status = writeUint32( stream, length );
	for( algoIndex = 0; cryptStatusOK( status ) && algoIndex < noAlgos; 
		 algoIndex++ )
		{
		const ALGO_STRING_INFO *algoStringInfo = \
				&algoStringMapTbl[ availAlgoIndex[ algoIndex ] ];

		if( algoIndex > 0 )
			sputc( stream, ',' );	/* Add comma delimiter */
		status = swrite( stream, algoStringInfo->name,
						 algoStringInfo->nameLen );
		}
	return( status );
	}

/* Handle an ephemeral DH key exchange */

CHECK_RETVAL STDC_NONNULL_ARG( ( 1, 2 ) ) \
static int processDHE( INOUT SESSION_INFO *sessionInfoPtr,
					   INOUT SSH_HANDSHAKE_INFO *handshakeInfo )
	{
	MESSAGE_DATA msgData;
	STREAM stream;
	BYTE keyData[ ( CRYPT_MAX_PKCSIZE * 2 ) + 16 + 8 ];
	void *keyexInfoPtr = DUMMY_INIT_PTR;
	int keyexInfoLength, keyDataStart, keyDataLength, length;
	int keySize, status;

	assert( isWritePtr( sessionInfoPtr, sizeof( SESSION_INFO ) ) );
	assert( isWritePtr( handshakeInfo, sizeof( SSH_HANDSHAKE_INFO ) ) );

	/* Get the keyex key request from the client:

		byte	type = SSH2_MSG_KEXDH_GEX_REQUEST_OLD
		uint32	n (bits)

	   or:

		byte	type = SSH2_MSG_KEXDH_GEX_REQUEST_NEW
		uint32	min (bits)
		uint32	n (bits)
		uint32	max (bits)

	   Portions of the the request information are hashed later as part of 
	   the exchange hash so we have to save a copy for then.  We save the
	   original encoded form because some clients send non-integral lengths
	   that don't survive the conversion from bits to bytes */
	status = length = \
		readHSPacketSSH2( sessionInfoPtr, SSH2_MSG_KEXDH_GEX_REQUEST_OLD,
						  ID_SIZE + UINT32_SIZE );
	if( cryptStatusError( status ) )
		return( status );
	sMemConnect( &stream, sessionInfoPtr->receiveBuffer, length );
	streamBookmarkSet( &stream, keyexInfoLength );
	if( sessionInfoPtr->sessionSSH->packetType == SSH2_MSG_KEXDH_GEX_REQUEST_NEW )
		{
		/* It's a { min_length, length, max_length } sequence, save a copy
		   and get the length value */
		readUint32( &stream );
		keySize = readUint32( &stream );
		status = readUint32( &stream );
		}
	else
		{
		/* It's a straight length, save a copy and get the length value */
		status = keySize = readUint32( &stream );
		}
	if( !cryptStatusError( status ) )
		status = streamBookmarkComplete( &stream, &keyexInfoPtr, 
										 &keyexInfoLength, keyexInfoLength );
	sMemDisconnect( &stream );
	if( cryptStatusError( status ) )
		{
		retExt( status,
				( status, SESSION_ERRINFO, 
				  "Invalid ephemeral DH key data request packet" ) );
		}
	if( keySize < bytesToBits( MIN_PKCSIZE ) || \
		keySize > bytesToBits( CRYPT_MAX_PKCSIZE ) )
		{
		retExt( CRYPT_ERROR_BADDATA, 
				( CRYPT_ERROR_BADDATA, SESSION_ERRINFO, 
				  "Client requested invalid ephemeral DH key size %d bits, "
				  "should be %d...%d", keySize, 
				  bytesToBits( MIN_PKCSIZE ), 
				  bytesToBits( CRYPT_MAX_PKCSIZE ) ) );
		}
	ENSURES( rangeCheckZ( 0, keyexInfoLength, MAX_ENCODED_KEYEXSIZE ) );
	memcpy( handshakeInfo->encodedReqKeySizes, keyexInfoPtr,
			keyexInfoLength );
	handshakeInfo->encodedReqKeySizesLength = keyexInfoLength;
	handshakeInfo->requestedServerKeySize = bitsToBytes( keySize );

	/* If the requested key size differs too much from the built-in default 
	   one, destroy the existing default DH key and load a new one of the 
	   appropriate size.  Things get quite confusing here because the spec 
	   is a schizophrenic mix of two different documents, one that specifies 
	   the behaviour for the original message format which uses a single 
	   length value and a second one that specifies the behaviour for the 
	   { min, n, max } combination (multi sunt, qui ad id, quod non 
	   proposuerant scribere, alicuius verbi placentis decore vocentur).
	   
	   The range option was added as an attempted fix for implementations 
	   that couldn't handle the single size option but the real problem is 
	   that the server knows what key sizes are appropriate but the client 
	   has to make the choice, without any knowledge of what the server can 
	   actually handle.  Because of this the spec (in its n-only mindset, 
	   which also applies to the min/n/max version since it's the same 
	   document) contains assorted weasel-words that allow the server to 
	   choose any key size it feels like if the client sends a range 
	   indication that's inappropriate.  Although the spec ends up saying 
	   that the server can do anything it feels like ("The server should 
	   return the smallest group it knows that is larger than the size the 
	   client requested.  If the server does not know a group that is 
	   larger than the client request, then it SHOULD return the largest 
	   group it knows"), we use a least-upper-bound interpretation of the 
	   above, mostly because we store a range of fixed keys of different 
	   sizes and can always find something reasonably close to any 
	   (sensible) requested length */
	if( handshakeInfo->requestedServerKeySize < \
										SSH2_DEFAULT_KEYSIZE - 16 || \
		handshakeInfo->requestedServerKeySize > \
										SSH2_DEFAULT_KEYSIZE + 16 )
		{
		krnlSendNotifier( handshakeInfo->iServerCryptContext,
						  IMESSAGE_DECREFCOUNT );
		handshakeInfo->iServerCryptContext = CRYPT_ERROR;
		status = initDHcontextSSH( &handshakeInfo->iServerCryptContext,
								   &handshakeInfo->serverKeySize, NULL, 0,
								   handshakeInfo->requestedServerKeySize );
		if( cryptStatusError( status ) )
			return( status );
		}

	/* Send the DH key values to the client:

		byte	type = SSH2_MSG_KEXDH_GEX_GROUP
		mpint	p
		mpint	g

	   Since this phase of the key negotiation exchanges raw key components
	   rather than the standard SSH public-key format we have to rewrite
	   the public key before we can send it to the client.  What this 
	   involves is stripping the:

		uint32	length
		string	"ssh-dh"

	   header from the start of the datab and then writing what's left to the 
	   packet.  First we export the key data and figure out the location of
	   the payload that we need to send */
	setMessageData( &msgData, keyData, ( CRYPT_MAX_PKCSIZE * 2 ) + 16 );
	status = krnlSendMessage( handshakeInfo->iServerCryptContext, 
							  IMESSAGE_GETATTRIBUTE_S, &msgData,
							  CRYPT_IATTRIBUTE_KEY_SSH );
	if( cryptStatusError( status ) )
		return( status );
	sMemConnect( &stream, keyData, msgData.length );
	readUint32( &stream );
	status = readUniversal32( &stream );
	ENSURES( cryptStatusOK( status ) );
	keyDataStart = stell( &stream );
	keyDataLength = sMemDataLeft( &stream );
	sMemDisconnect( &stream );

	/* Then we create and send the SSH packet using as the payload the key
	   data content of the SSH public key */
	status = openPacketStreamSSH( &stream, sessionInfoPtr, 
								  SSH2_MSG_KEXDH_GEX_GROUP );
	if( cryptStatusError( status ) )
		return( status );
	status = swrite( &stream, keyData + keyDataStart, keyDataLength );
	if( cryptStatusOK( status ) )
		status = sendPacketSSH2( sessionInfoPtr, &stream, FALSE );
	sMemDisconnect( &stream );
	return( status );
	}

/****************************************************************************
*																			*
*							Server-side Connect Functions					*
*																			*
****************************************************************************/

/* Perform the initial part of the handshake with the client */

CHECK_RETVAL STDC_NONNULL_ARG( ( 1, 2 ) ) \
static int beginServerHandshake( INOUT SESSION_INFO *sessionInfoPtr,
								 INOUT SSH_HANDSHAKE_INFO *handshakeInfo )
	{
	static const ALGO_STRING_INFO FAR_BSS algoStringPubkeyRSATbl[] = {
		{ "ssh-rsa", 7, CRYPT_ALGO_RSA },
		{ NULL, CRYPT_ALGO_NONE }, { NULL, CRYPT_ALGO_NONE }
		};
	static const ALGO_STRING_INFO FAR_BSS algoStringPubkeyDSATbl[] = {
		{ "ssh-dss", 7, CRYPT_ALGO_DSA },
		{ NULL, CRYPT_ALGO_NONE }, { NULL, CRYPT_ALGO_NONE }
		};
	STREAM stream;
	BOOLEAN skipGuessedKeyex = FALSE;
	void *serverHelloPtr = DUMMY_INIT_PTR;
	int length, serverHelloLength, clientHelloLength, status;

	assert( isWritePtr( sessionInfoPtr, sizeof( SESSION_INFO ) ) );
	assert( isWritePtr( handshakeInfo, sizeof( SSH_HANDSHAKE_INFO ) ) );

	/* Get the public-key algorithm that we'll be advertising to the client
	   and set the algorithm table used for processing the client hello to
	   match the one that we're offering */
	status = krnlSendMessage( sessionInfoPtr->privateKey,
							  IMESSAGE_GETATTRIBUTE,
							  &handshakeInfo->pubkeyAlgo,
							  CRYPT_CTXINFO_ALGO );
	if( cryptStatusError( status ) )
		return( status );
	switch( handshakeInfo->pubkeyAlgo )
		{
		case CRYPT_ALGO_RSA:
			handshakeInfo->algoStringPubkeyTbl = algoStringPubkeyRSATbl;
			handshakeInfo->algoStringPubkeyTblNoEntries = \
				FAILSAFE_ARRAYSIZE( algoStringPubkeyRSATbl, ALGO_STRING_INFO );
			break;

		case CRYPT_ALGO_DSA:
			handshakeInfo->algoStringPubkeyTbl = algoStringPubkeyDSATbl;
			handshakeInfo->algoStringPubkeyTblNoEntries = \
				FAILSAFE_ARRAYSIZE( algoStringPubkeyDSATbl, ALGO_STRING_INFO );
			break;

		default:
			retIntError();
		}

	/* SSH hashes parts of the handshake messages for integrity-protection
	   purposes so before we start we hash the ID strings (first the client
	   string that we read previously, then our server string) encoded as SSH
	   string values */
	status = hashAsString( handshakeInfo->iExchangeHashcontext,
						   sessionInfoPtr->receiveBuffer,
						   strlen( sessionInfoPtr->receiveBuffer ) );
#ifdef KPYM_HACK
	if( strlen( sessionInfoPtr->receiveBuffer ) < sizeof( ke_client_id ) )
		strcpy_s( ke_client_id, sizeof( ke_client_id ), sessionInfoPtr->receiveBuffer );
#endif
	if( cryptStatusOK( status ) )
		status = hashAsString( handshakeInfo->iExchangeHashcontext, 
							   SSH2_ID_STRING, SSH_ID_STRING_SIZE );
	if( cryptStatusError( status ) )
		return( status );

	/* Send the server hello packet:

		byte		type = SSH2_MSG_KEXINIT
		byte[16]	cookie
		string		keyex algorithms
		string		pubkey algorithms
		string		client_crypto algorithms
		string		server_crypto algorithms
		string		client_mac algorithms
		string		server_mac algorithms
		string		client_compression algorithms = "none"
		string		server_compression algorithms = "none"
		string		client_language = ""
		string		server_language = ""
		boolean		first_keyex_packet_follows = FALSE
		uint32		reserved = 0

	   The SSH spec leaves the order in which things happen ambiguous, in
	   order to save a while round trip it has provisions for both sides
	   shouting at each other and then a complex interlock process where
	   bits of the initial exchange can be discarded and retried if necessary.
	   This is ugly and error-prone.  The client code solves this by waiting
	   for the server hello, choosing known-good algorithms, and then sending
	   the client hello immediately followed by the client key exchange data.
	   Since it waits for the server to speak first it can choose parameters
	   that are accepted the first time.

	   Unfortunately this doesn't work if we're the server since we'd end up 
	   waiting for the client to speak first while it waits for us to speak 
	   first, so we have to send the server hello in order to prevent 
	   deadlock.  This works fine with most clients, which take the same
	   approach and wait for the server to speak first.  The message flow is
	   then:

		server hello;
		client hello;
		client keyex;
		server keyex;

	   There are one or two exceptions to this, the worst of which is the
	   F-Secure client, which has the client speak first choosing as its
	   preference the incompletely specified "x509v3-sign-dss" format (see
	   the comment in exchangeServerKeys() below) that we can't use since no-
	   one's quite sure what the format is (this was fixed in mid-2004 when
	   the x509v3-* schemes were removed from the spec since no-one could
	   figure out what they were.  F-Secure still specifies them, but has
	   moved them down so that they follow the standard ssh-* schemes).  In 
	   this case the message flow is:

		server hello;
		client hello;
		client keyex1;
		client keyex2;
		server keyex;

	   This is handled by having the code that reads the client hello return
	   OK_SPECIAL to indicate that the next packet should be skipped.  An
	   alternative (and simpler) strategy would be to always throw away the
	   client's first keyex sent by older versions of the F-Secure client
	   since they're using an algorithm choice that's impossible to use but 
	   that implementation-specific approach doesn't generalise well to 
	   other versions or other clients */
	status = openPacketStreamSSH( &stream, sessionInfoPtr, SSH2_MSG_KEXINIT );
	if( cryptStatusError( status ) )
		return( status );
	streamBookmarkSetFullPacket( &stream, serverHelloLength );
	status = exportVarsizeAttributeToStream( &stream, SYSTEM_OBJECT_HANDLE,
											 CRYPT_IATTRIBUTE_RANDOM_NONCE,
											 SSH2_COOKIE_SIZE );
	status = writeAlgoList( &stream, algoKeyexList,
#ifdef KPYM_HACK
							typeSize(algoKeyexList));
#else
							FAILSAFE_ARRAYSIZE( algoKeyexList, \
												CRYPT_ALGO_TYPE ) );
#endif
	if( cryptStatusOK( status ) )
		status = writeAlgoString( &stream, handshakeInfo->pubkeyAlgo );
	if( cryptStatusOK( status ) )
		status = writeAlgoList( &stream, algoEncrList,
#ifdef KPYM_HACK
								typeSize(algoEncrList));
#else
								FAILSAFE_ARRAYSIZE( algoEncrList, \
													CRYPT_ALGO_TYPE ) );
#endif
	if( cryptStatusOK( status ) )
		status = writeAlgoList( &stream, algoEncrList,
#ifdef KPYM_HACK
								typeSize(algoEncrList));
#else
								FAILSAFE_ARRAYSIZE( algoEncrList, \
													CRYPT_ALGO_TYPE ) );
#endif
	if( cryptStatusOK( status ) )
		status = writeAlgoList( &stream, algoMACList,
#ifdef KPYM_HACK
								typeSize(algoMACList));
#else
								FAILSAFE_ARRAYSIZE( algoMACList, \
													CRYPT_ALGO_TYPE ) );
#endif
	if( cryptStatusOK( status ) )
		status = writeAlgoList( &stream, algoMACList,
#ifdef KPYM_HACK
								typeSize(algoMACList));
#else
								FAILSAFE_ARRAYSIZE( algoMACList, \
													CRYPT_ALGO_TYPE ) );
#endif
	if( cryptStatusOK( status ) )
		status = writeAlgoString( &stream, CRYPT_PSEUDOALGO_COPR );
	if( cryptStatusOK( status ) )
		status = writeAlgoString( &stream, CRYPT_PSEUDOALGO_COPR );
	if( cryptStatusOK( status ) )
		{
		writeUint32( &stream, 0 );			/* No language tag */
		writeUint32( &stream, 0 );
		sputc( &stream, 0 );				/* Don't try and guess the keyex */
		status = writeUint32( &stream, 0 );	/* Reserved */
		}
	if( cryptStatusOK( status ) )
		{
		status = streamBookmarkComplete( &stream, &serverHelloPtr, 
										 &serverHelloLength, 
										 serverHelloLength );
		}
	if( cryptStatusOK( status ) )
		status = sendPacketSSH2( sessionInfoPtr, &stream, FALSE );
	sMemDisconnect( &stream );
	if( cryptStatusError( status ) )
		return( status );

	/* While we wait for the client to digest our hello and send back its
	   response, create the context with the DH key */
	status = initDHcontextSSH( &handshakeInfo->iServerCryptContext,
							   &handshakeInfo->serverKeySize, NULL, 0,
							   CRYPT_USE_DEFAULT );
	if( cryptStatusError( status ) )
		return( status );

	/* Process the client hello packet and hash the client and server
	   hello.  Since the entire encoded packet (including the type value) 
	   is hashed we have to reconstruct this at the start of the client
	   hello packet */
	status = processHelloSSH( sessionInfoPtr, handshakeInfo,
							  &clientHelloLength, TRUE );
	if( cryptStatusError( status ) )
		{
		if( status != OK_SPECIAL )
			return( status );

		/* There's a guessed keyex following the client hello that we have
		   to skip later (we can't process it at this point because we still
		   need to hash the data sitting in the receive buffer) */
		skipGuessedKeyex = TRUE;
		}
	REQUIRES( rangeCheck( 1, clientHelloLength, 
						  sessionInfoPtr->receiveBufSize ) );
	memmove( sessionInfoPtr->receiveBuffer + 1, 
			 sessionInfoPtr->receiveBuffer, clientHelloLength );
	sessionInfoPtr->receiveBuffer[ 0 ] = SSH2_MSG_KEXINIT;
	status = hashAsString( handshakeInfo->iExchangeHashcontext,
						   sessionInfoPtr->receiveBuffer,
						   clientHelloLength + 1 );
	if( cryptStatusOK( status ) && skipGuessedKeyex )
		{
		/* There's an incorrectly-guessed keyex following the client hello, 
		   skip it */
		status = readHSPacketSSH2( sessionInfoPtr,
							( handshakeInfo->requestedServerKeySize > 0 ) ? \
								SSH2_MSG_KEXDH_GEX_INIT : SSH2_MSG_KEXDH_INIT,
							ID_SIZE + sizeofString32( "", MIN_PKCSIZE ) );
		}
	if( !cryptStatusError( status ) )	/* readHSPSSH2() returns a length */
		status = hashAsString( handshakeInfo->iExchangeHashcontext,
							   serverHelloPtr, serverHelloLength );
	if( cryptStatusError( status ) )
		return( status );

	/* If we're using a nonstandard DH key value, negotiate a new key with
	   the client */
	if( handshakeInfo->requestedServerKeySize > 0 )
		{
		status = processDHE( sessionInfoPtr, handshakeInfo );
		if( cryptStatusError( status ) )
			return( status );
		}

	/* Process the client keyex:

		byte	type = SSH2_MSG_KEXDH_INIT / SSH2_MSG_KEXDH_GEX_INIT
		mpint	y */
	status = length = \
		readHSPacketSSH2( sessionInfoPtr,
						  ( handshakeInfo->requestedServerKeySize > 0 ) ? \
							SSH2_MSG_KEXDH_GEX_INIT : SSH2_MSG_KEXDH_INIT,
						  ID_SIZE + sizeofString32( "", MIN_PKCSIZE ) );
	if( cryptStatusError( status ) )
		return( status );
	sMemConnect( &stream, sessionInfoPtr->receiveBuffer, length );
	status = readRawObject32( &stream, handshakeInfo->clientKeyexValue,
							  MAX_ENCODED_KEYEXSIZE,
							  &handshakeInfo->clientKeyexValueLength );
	sMemDisconnect( &stream );
	if( cryptStatusError( status ) || \
		!isValidDHsize( handshakeInfo->clientKeyexValueLength,
						handshakeInfo->serverKeySize, LENGTH_SIZE ) )
		{
		retExt( CRYPT_ERROR_BADDATA,
				( CRYPT_ERROR_BADDATA, SESSION_ERRINFO, 
				  "Invalid DH phase 1 keyex value" ) );
		}
	return( CRYPT_OK );
	}

/* Exchange keys with the client */

CHECK_RETVAL STDC_NONNULL_ARG( ( 1, 2 ) ) \
static int exchangeServerKeys( INOUT SESSION_INFO *sessionInfoPtr,
							   INOUT SSH_HANDSHAKE_INFO *handshakeInfo )
	{
	KEYAGREE_PARAMS keyAgreeParams;
	STREAM stream;
	void *keyPtr = DUMMY_INIT_PTR, *dataPtr;
	int keyLength, dataLength, sigLength = DUMMY_INIT, packetOffset, status;

	assert( isWritePtr( sessionInfoPtr, sizeof( SESSION_INFO ) ) );
	assert( isWritePtr( handshakeInfo, sizeof( SSH_HANDSHAKE_INFO ) ) );

	/* Create the server DH value */
	memset( &keyAgreeParams, 0, sizeof( KEYAGREE_PARAMS ) );
	status = krnlSendMessage( handshakeInfo->iServerCryptContext,
							  IMESSAGE_CTX_ENCRYPT, &keyAgreeParams,
							  sizeof( KEYAGREE_PARAMS ) );
	if( cryptStatusError( status ) )
		return( status );
	sMemOpen( &stream, handshakeInfo->serverKeyexValue, 
			  MAX_ENCODED_KEYEXSIZE );
	status = writeInteger32( &stream, keyAgreeParams.publicValue,
							 keyAgreeParams.publicValueLen );
	if( cryptStatusOK( status ) )
		handshakeInfo->serverKeyexValueLength = stell( &stream );
	sMemDisconnect( &stream );
	if( cryptStatusError( status ) )
		return( status );

	/* Build the DH phase 2 keyex packet:

		byte		type = SSH2_MSG_KEXDH_REPLY / SSH2_MSG_KEXDH_GEX_REPLY
		string		[ server key/certificate ]
			string	"ssh-rsa"	"ssh-dss"
			mpint	e			p
			mpint	n			q
			mpint				g
			mpint				y
		mpint		y'
		string		[ signature of handshake data ]
			string	"ssh-rsa"	"ssh-dss"
			string	signature	signature
		...

	   The specification also makes provision for using X.509 and PGP keys,
	   but only so far as to say that keys and signatures are in "X.509 DER"
	   and "PGP" formats, neither of which actually explain what it is
	   that's sent or signed (and no-one on the SSH list can agree on what
	   they're supposed to look like) so we can't use either of them */
	status = openPacketStreamSSH( &stream, sessionInfoPtr, 
								  handshakeInfo->requestedServerKeySize ? \
									SSH2_MSG_KEXDH_GEX_REPLY : \
									SSH2_MSG_KEXDH_REPLY );
	if( cryptStatusError( status ) )
		return( status );
	streamBookmarkSet( &stream, keyLength );
	status = exportAttributeToStream( &stream, sessionInfoPtr->privateKey,
									  CRYPT_IATTRIBUTE_KEY_SSH );
	if( cryptStatusOK( status ) )
		status = streamBookmarkComplete( &stream, &keyPtr, &keyLength, 
										 keyLength );
	if( cryptStatusOK( status ) )
		status = krnlSendMessage( handshakeInfo->iExchangeHashcontext,
								  IMESSAGE_CTX_HASH, keyPtr, keyLength );
	if( cryptStatusError( status ) )
		{
		sMemDisconnect( &stream );
		return( status );
		}
	swrite( &stream, handshakeInfo->serverKeyexValue,
			handshakeInfo->serverKeyexValueLength );

	/* Complete phase 2 of the DH key agreement process to obtain the shared
	   secret value */
	status = completeKeyex( sessionInfoPtr, handshakeInfo, TRUE );
#ifdef KPYM_HACK
	memcpy(ke_sessionID, handshakeInfo->sessionID, sizeof(handshakeInfo->sessionID));
	ke_sessionIDLength = handshakeInfo->sessionIDlength;
#endif
	if( cryptStatusError( status ) )
		return( status );

	/* Sign the hash.  The reason for the min() part of the expression is
	   that iCryptCreateSignature() gets suspicious of very large buffer
	   sizes, for example when the user has specified the use of a 1MB send
	   buffer */
	status = sMemGetDataBlockRemaining( &stream, &dataPtr, &dataLength );
	if( cryptStatusOK( status ) )
		{
		status = iCryptCreateSignature( dataPtr, 
							min( dataLength, MAX_INTLENGTH_SHORT - 1 ), 
							&sigLength, CRYPT_IFORMAT_SSH, 
							sessionInfoPtr->privateKey,
							handshakeInfo->iExchangeHashcontext, NULL );
		}
	krnlSendNotifier( handshakeInfo->iExchangeHashcontext,
					  IMESSAGE_DECREFCOUNT );
	handshakeInfo->iExchangeHashcontext = CRYPT_ERROR;
	if( cryptStatusOK( status ) )
		status = sSkip( &stream, sigLength );
	if( cryptStatusOK( status ) )
		status = wrapPacketSSH2( sessionInfoPtr, &stream, 0, FALSE, TRUE );
	if( cryptStatusError( status ) )
		{
		sMemDisconnect( &stream );
		return( status );
		}

	/* Build our change cipherspec message and send the whole mess through
	   to the client:
		...
		byte	type = SSH2_MSG_NEWKEYS

	   After this point the write channel is in the secure state */
	status = continuePacketStreamSSH( &stream, SSH2_MSG_NEWKEYS, 
									  &packetOffset );
	if( cryptStatusOK( status ) )
		status = wrapPacketSSH2( sessionInfoPtr, &stream, packetOffset, 
								 FALSE, TRUE );
	if( cryptStatusOK( status ) )
		status = sendPacketSSH2( sessionInfoPtr, &stream, TRUE );
	sMemDisconnect( &stream );
	if( cryptStatusError( status ) )
		return( status );
	sessionInfoPtr->flags |= SESSION_ISSECURE_WRITE;
	return( CRYPT_OK );
	}

#ifdef KPYM_HACK

int processClientKexinit( SESSION_INFO *sessionInfoPtr, SSH_HANDSHAKE_INFO *handshakeInfo, STREAM stream );

int ke_SSH2_MSG_KEXINIT( SESSION_INFO *sessionInfoPtr, STREAM * stream )
{
	
	int status;
	SESSION_INFO si;
	static const ALGO_STRING_INFO FAR_BSS algoStringPubkeyRSATbl[] = {
		{ "ssh-rsa", 7, CRYPT_ALGO_RSA },
		{ NULL, 0, CRYPT_ALGO_NONE }
		};
	static const ALGO_STRING_INFO FAR_BSS algoStringPubkeyDSATbl[] = {
		{ "ssh-dss", 7, CRYPT_ALGO_DSA },
		{ NULL, 0, CRYPT_ALGO_NONE }
		};
//	int length;

	int clientHelloLength = 0;
	int serverHelloLength = 0;
	BYTE clientHello[ 2000 ] = {0};
	BYTE serverHello[ 2000 ] = {0};

	if( ke_handshakeInfo != NULL ) return CRYPT_ERROR;

	{
		MESSAGE_CREATEOBJECT_INFO createInfo;

		ke_handshakeInfo = malloc(sizeof(SSH_HANDSHAKE_INFO));
		memset( ke_handshakeInfo, 0, sizeof( SSH_HANDSHAKE_INFO ) );
		ke_handshakeInfo->iExchangeHashcontext = ke_handshakeInfo->iServerCryptContext = CRYPT_ERROR;

		initSSH2processing( &si, ke_handshakeInfo, TRUE );

		/* SSHv2 hashes parts of the handshake messages for integrity-protection
		   purposes, so if we're talking to an SSHv2 peer we create a context
		   for the hash */
		setMessageCreateObjectInfo( &createInfo, CRYPT_ALGO_SHA );
		status = krnlSendMessage( SYSTEM_OBJECT_HANDLE, IMESSAGE_DEV_CREATEOBJECT,
								  &createInfo, OBJECT_TYPE_CONTEXT );

		if( !cryptStatusOK( status ) ) return( status );

		if( cryptStatusOK( status ) ) ke_handshakeInfo->iExchangeHashcontext = createInfo.cryptHandle;

		/* Get the public-key algorithm that we'll be advertising to the client
		   and set the algorithm table used for processing the client hello to
		   only match the one that we're offering */
		status = krnlSendMessage( sessionInfoPtr->privateKey,
								  IMESSAGE_GETATTRIBUTE,
								  &ke_handshakeInfo->pubkeyAlgo,
								  CRYPT_CTXINFO_ALGO );

		if( cryptStatusError( status ) ) return( status );

		switch( ke_handshakeInfo->pubkeyAlgo )
			{
			case CRYPT_ALGO_RSA:
				ke_handshakeInfo->algoStringPubkeyTbl = algoStringPubkeyRSATbl;
				break;

			case CRYPT_ALGO_DSA:
				ke_handshakeInfo->algoStringPubkeyTbl = algoStringPubkeyDSATbl;
				break;

			default:
//				assert( NOTREACHED );
				return( CRYPT_ERROR_NOTAVAIL );
			}
	}




	// save the client hello packet for hashing
	clientHelloLength = stream->bufEnd + 1;
	if( clientHelloLength < sizeof( clientHello ) && clientHelloLength > 16 )
	{
		clientHello[0] = 20;
		memcpy( &clientHello[1], stream->buffer, clientHelloLength );
	}
	else return CRYPT_ERROR_OVERFLOW;
	

	/* Process the client hello packet */
	status = processClientKexinit( &si, ke_handshakeInfo, *stream );
	// if there's an incorrectly-guessed keyex we'll fail
	if( !cryptStatusOK( status ) ) return( status );

	/* Send the server hello packet:

		byte		type = SSH2_MSG_KEXINIT
		byte[16]	cookie
		string		keyex algorithms
		string		pubkey algorithms
		string		client_crypto algorithms
		string		server_crypto algorithms
		string		client_mac algorithms
		string		server_mac algorithms
		string		client_compression algorithms = "none"
		string		server_compression algorithms = "none"
		string		client_language = ""
		string		server_language = ""
		boolean		first_keyex_packet_follows = FALSE
		uint32		reserved = 0
	*/
	{
		STREAM stream;
		status = openPacketStreamSSH( &stream, sessionInfoPtr, SSH2_MSG_KEXINIT );
		if( cryptStatusError( status ) )
			return( status );
		status = exportVarsizeAttributeToStream( &stream, SYSTEM_OBJECT_HANDLE,
										CRYPT_IATTRIBUTE_RANDOM_NONCE,
										SSH2_COOKIE_SIZE );
		if( cryptStatusError( status ) )
			return( status );
		writeAlgoList( &stream, algoKeyexList, typeSize(algoKeyexList) );
		writeAlgoString( &stream, ke_handshakeInfo->pubkeyAlgo );
		writeAlgoList( &stream, algoEncrList, typeSize(algoEncrList) );
		writeAlgoList( &stream, algoEncrList, typeSize(algoEncrList) );
		writeAlgoList( &stream, algoMACList, typeSize(algoMACList) );
		writeAlgoList( &stream, algoMACList, typeSize(algoMACList) );
		writeAlgoString( &stream, CRYPT_PSEUDOALGO_COPR );
		writeAlgoString( &stream, CRYPT_PSEUDOALGO_COPR );
		writeUint32( &stream, 0 );			/* No language tag */
		writeUint32( &stream, 0 );
		sputc( &stream, 0 );				/* Don't try and guess the keyex */
		writeUint32( &stream, 0 );			/* Reserved */

		// save t he serve hello
		serverHelloLength = stream.bufEnd - 5;
		if( serverHelloLength < sizeof( serverHello ) && serverHelloLength > 16 )
			memcpy( serverHello, &stream.buffer[5], serverHelloLength );
		else return CRYPT_ERROR_OVERFLOW;

		status = sendPacketSSH2( sessionInfoPtr, &stream, FALSE );

		// hash some data before disconnecting the stream
		if( !cryptStatusError( status ) ) 
			status = hashAsString( ke_handshakeInfo->iExchangeHashcontext, ke_client_id, strlen( ke_client_id ) );
		if( !cryptStatusError( status ) ) 
			status = hashAsString( ke_handshakeInfo->iExchangeHashcontext, SSH2_ID_STRING, strlen( SSH2_ID_STRING ) );
		if( !cryptStatusError( status ) ) 
			status = hashAsString( ke_handshakeInfo->iExchangeHashcontext, clientHello, clientHelloLength );
		if( !cryptStatusError( status ) ) 
			status = hashAsString( ke_handshakeInfo->iExchangeHashcontext, serverHello, serverHelloLength );
		sMemDisconnect( &stream );
	}

	if( cryptStatusError( status ) )
		return( status );



	/* While we wait for the client to digest our hello and send back its
	   response, create the context with the DH key */
	status = initDHcontextSSH( &ke_handshakeInfo->iServerCryptContext,
							   &ke_handshakeInfo->serverKeySize, NULL, 0,
							   CRYPT_USE_DEFAULT );
	if( cryptStatusError( status ) )
		return( status );

	return( CRYPT_OK );
}

static int ke_SSH2_MSG_NEWKEYS( SESSION_INFO *sessionInfoPtr )
{
	int status;


	{
		// send new keys packet
		STREAM stream;
		status = openPacketStreamSSH( &stream, sessionInfoPtr, 
							 SSH2_MSG_NEWKEYS );

		status = sendPacketSSH2( sessionInfoPtr, &stream, FALSE );

		if( cryptStatusError( status ) )
			return( status );

		sMemDisconnect( &stream );
	}

	/* Set up the security information required for the session */
	status = initSecurityInfo( sessionInfoPtr, ke_handshakeInfo );

	free(ke_handshakeInfo);
	ke_handshakeInfo = NULL;

	if( cryptStatusError( status ) )
		return( status );

	return CRYPT_OK;
}

static int ke_SSH2_MSG_KEXDH_GEX_REQUEST( SESSION_INFO *sessionInfoPtr,
					   STREAM * stream1, int packetType )
	{
	STREAM stream;
	BYTE *keyPtr = DUMMY_INIT_PTR;
	void *keyexInfoPtr = DUMMY_INIT_PTR;
	const int offset = LENGTH_SIZE + sizeofString32( "ssh-dh", 6 );
	int keyPos, keyLength, keyexInfoLength, type, status;

	/* Get the keyex key request from the client:

		byte	type = SSH2_MSG_KEXDH_GEX_REQUEST_OLD
		uint32	n (bits)

	   or:

		byte	type = SSH2_MSG_KEXDH_GEX_REQUEST_NEW
		uint32	min (bits)
		uint32	n (bits)
		uint32	max (bits)

	   Portions of the the request info are hashed later as part of the
	   exchange hash, so we have to save a copy for then.  We save the
	   original encoded form, because some clients send non-integral lengths
	   that don't survive the conversion from bits to bytes */
/*	length = readPacketSSH2( sessionInfoPtr,
							 SSH2_MSG_KEXDH_GEX_REQUEST_OLD,
							 ID_SIZE + UINT32_SIZE );
	if( cryptStatusError( length ) )
		return( length );
*/	
//	sMemConnect( &stream, sessionInfoPtr->receiveBuffer, length );
	stream = *stream1;
	type = packetType;

	streamBookmarkSet( &stream, keyexInfoLength );
	if( type == SSH2_MSG_KEXDH_GEX_REQUEST_NEW )
		{
		/* It's a { min_length, length, max_length } sequence, save a copy
		   and get the length value */
		readUint32( &stream );
		keyLength = readUint32( &stream );
		status = readUint32( &stream );
		}
	else
		/* It's a straight length, save a copy and get the length value */
		status = keyLength = readUint32( &stream );
	streamBookmarkComplete( &stream, &keyexInfoPtr, &keyexInfoLength, keyexInfoLength );
//	sMemDisconnect( &stream );
	if( cryptStatusError( status ) )
		retExt( SESSION_ERRINFO, ( status, SESSION_ERRINFO, 
				  "Invalid ephemeral DH key data request packet" ));
	if( keyLength < bytesToBits( MIN_PKCSIZE ) || \
		keyLength > bytesToBits( CRYPT_MAX_PKCSIZE ) )
		retExt( SESSION_ERRINFO, ( CRYPT_ERROR_BADDATA, SESSION_ERRINFO, 
				  "Client requested invalid ephemeral DH key size %d bits" ));
	memcpy( ke_handshakeInfo->encodedReqKeySizes, keyexInfoPtr,
			keyexInfoLength );
	ke_handshakeInfo->encodedReqKeySizesLength = keyexInfoLength;
	ke_handshakeInfo->requestedServerKeySize = bitsToBytes( keyLength );

	/* If the requested key size differs too much from the built-in default
	   one, destroy the existing default DH key and load a new one of the
	   appropriate size.  Things get quite confusing here because the spec
	   is a schizophrenic mix of two different documents, one that specifies
	   the behaviour for the original message format which uses a single
	   length value and a second one that specifies the behaviour for the
	   { min, n, max } combination.  The range option was added as an
	   attempted fix for implementations that couldn't handle the single
	   size option, but the real problem is that the server knows what key
	   sizes are appropriate but the client has to make the choice, without
	   any knowledge of what the server can actually handle.  Because of
	   this the spec (in its n-only mindset, which also applies to the
	   min/n/max version since it's the same document) contains assorted
	   weasel-words that allow the server to choose any key size it feels
	   like if the client sends a range indication that's inappropriate.
	   Although the spec ends up saying that the server can do anything it
	   feels like ("The server should return the smallest group it knows
	   that is larger than the size the client requested.  If the server
	   does not know a group that is larger than the client request, then it
	   SHOULD return the largest group it knows"), we use a least-upper-
	   bound interpretation of the above, mostly because we store a range of
	   fixed keys of different sizes and can always find something
	   reasonably close to any (sensible) requested length */
	if( ke_handshakeInfo->requestedServerKeySize < \
										SSH2_DEFAULT_KEYSIZE - 16 || \
		ke_handshakeInfo->requestedServerKeySize > \
										SSH2_DEFAULT_KEYSIZE + 16 )
		{
		krnlSendNotifier( ke_handshakeInfo->iServerCryptContext,
						  IMESSAGE_DECREFCOUNT );
		status = initDHcontextSSH( &ke_handshakeInfo->iServerCryptContext,
								   &ke_handshakeInfo->serverKeySize, NULL, 0,
								   ke_handshakeInfo->requestedServerKeySize );
		if( cryptStatusError( status ) )
			return( status );
		}

	/* Send the DH key values to the client:

		byte	type = SSH2_MSG_KEXDH_GEX_GROUP
		mpint	p
		mpint	g

	   Since this phase of the key negotiation exchanges raw key components
	   rather than the standard SSH public-key format, we have to rewrite
	   the public key before we can send it to the client.  What this 
	   involves is stripping the:

		uint32	length
		string	"ssh-dh"

	   header from the start of the key, which is accomplished by moving the
	   key data down offset (= LENGTH_SIZE + sizeofString32( "ssh-dh", 6 ))
	   bytes */
	{
		STREAM stream;
		status = openPacketStreamSSH( &stream, sessionInfoPtr, SSH2_MSG_KEXDH_GEX_GROUP );
		if( cryptStatusError( status ) )
			return( status );
		streamBookmarkSet( &stream, keyPos );
		status = exportAttributeToStream( &stream,
										  ke_handshakeInfo->iServerCryptContext,
										  CRYPT_IATTRIBUTE_KEY_SSH );
		if( cryptStatusOK( status ) )
			{
			keyLength = keyPos;
			streamBookmarkComplete( &stream, &keyPtr, &keyLength, keyLength );
			}
		if( cryptStatusError( status ) )
			return( status );
		memmove( keyPtr, keyPtr + offset, keyLength - offset );
		sseek( &stream, keyPos + keyLength - offset );
		status = sendPacketSSH2( sessionInfoPtr, &stream, FALSE );
		sMemDisconnect( &stream );
	}

	return( status );
}

int ke_SSH2_MSG_KEXDH_INIT( SESSION_INFO *sessionInfoPtr, STREAM * stream1, int packetType )
{
	STREAM stream = * stream1;
	KEYAGREE_PARAMS keyAgreeParams;
	void *keyPtr, *dataPtr;
	int keyLength, dataLength, sigLength, status;

	/* Process the client keyex:

		byte	type = SSH2_MSG_KEXDH_INIT / SSH2_MSG_KEXDH_GEX_INIT
		mpint	y */
	status = readRawObject32( &stream, ke_handshakeInfo->clientKeyexValue,
							  MAX_ENCODED_KEYEXSIZE + 8,
							  &ke_handshakeInfo->clientKeyexValueLength );
	sMemDisconnect( &stream );
	if( cryptStatusError( status ) || \
		!isValidDHsize( ke_handshakeInfo->clientKeyexValueLength,
						ke_handshakeInfo->serverKeySize, LENGTH_SIZE ) )
		retExt( SESSION_ERRINFO, ( CRYPT_ERROR_BADDATA, SESSION_ERRINFO, 
					  "Invalid DH phase 1 keyex value" ));
	
	/* Create the server DH value */
	memset( &keyAgreeParams, 0, sizeof( KEYAGREE_PARAMS ) );
	status = krnlSendMessage( ke_handshakeInfo->iServerCryptContext,
							  IMESSAGE_CTX_ENCRYPT, &keyAgreeParams,
							  sizeof( KEYAGREE_PARAMS ) );
	if( cryptStatusError( status ) )
		return( status );
	sMemOpen( &stream, ke_handshakeInfo->serverKeyexValue,
			  sizeof( ke_handshakeInfo->serverKeyexValue ) );
	status = writeInteger32( &stream, keyAgreeParams.publicValue,
							 keyAgreeParams.publicValueLen );
	if( cryptStatusOK( status ) )
		ke_handshakeInfo->serverKeyexValueLength = stell( &stream );
	sMemDisconnect( &stream );
	if( cryptStatusError( status ) )
		return( status );

	/* Build the DH phase 2 keyex packet:

		byte		type = SSH2_MSG_KEXDH_REPLY / SSH2_MSG_KEXDH_GEX_REPLY
		string		[ server key/certificate ]
			string	"ssh-rsa"	"ssh-dss"
			mpint	e			p
			mpint	n			q
			mpint				g
			mpint				y
		mpint		y'
		string		[ signature of handshake data ]
			string	"ssh-rsa"	"ssh-dss"
			string	signature	signature
		...

	   The specification also makes provision for using X.509 and PGP keys,
	   but only so far as to say that keys and signatures are in "X.509 DER"
	   and "PGP" formats, neither of which actually explain what it is
	   that's sent or signed (and no-one on the SSH list can agree on what
	   they're supposed to look like), so we can't use either of them */
	status = openPacketStreamSSH( &stream, sessionInfoPtr,
						 ke_handshakeInfo->requestedServerKeySize ? \
							SSH2_MSG_KEXDH_GEX_REPLY : SSH2_MSG_KEXDH_REPLY );
	if( cryptStatusError( status ) )
		{
		sMemDisconnect( &stream );
		return( status );
		}
	streamBookmarkSet( &stream, keyLength );
	status = exportAttributeToStream( &stream, sessionInfoPtr->privateKey,
									  CRYPT_IATTRIBUTE_KEY_SSH );
	if( cryptStatusError( status ) )
		{
		sMemDisconnect( &stream );
		return( status );
		}
	streamBookmarkComplete( &stream, &keyPtr, &keyLength, keyLength );
	status = krnlSendMessage( ke_handshakeInfo->iExchangeHashcontext,
							  IMESSAGE_CTX_HASH, keyPtr, keyLength );
	if( cryptStatusError( status ) )
		{
		sMemDisconnect( &stream );
		return( status );
		}
	swrite( &stream, ke_handshakeInfo->serverKeyexValue,
			ke_handshakeInfo->serverKeyexValueLength );

	/* Complete phase 2 of the DH key agreement process to obtain the shared
	   secret value */
	status = completeKeyex( sessionInfoPtr, ke_handshakeInfo, TRUE );

	if( cryptStatusError( status ) )
		return( status );

	/* 
	status = sMemGetDataBlockRemaining( &stream, &dataPtr, &dataLength );
	if( cryptStatusOK( status ) )
	{
		status = iCryptCreateSignature( dataPtr, 
							min( sMemDataLeft( &stream ), MAX_INTLENGTH_SHORT - 1 ),
							&sigLength,
							CRYPT_IFORMAT_SSH, sessionInfoPtr->privateKey,
							ke_handshakeInfo->iExchangeHashcontext,
							CRYPT_UNUSED, CRYPT_UNUSED );
	}
	*/
	/* Sign the hash.  The reason for the min() part of the expression is
	   that iCryptCreateSignature() gets suspicious of very large buffer
	   sizes, for example when the user has specified the use of a 1MB send
	   buffer */
	status = sMemGetDataBlockRemaining( &stream, &dataPtr, &dataLength );
	if( cryptStatusOK( status ) )
		{
		status = iCryptCreateSignature( dataPtr, 
							min( dataLength, MAX_INTLENGTH_SHORT - 1 ), 
							&sigLength, CRYPT_IFORMAT_SSH, 
							sessionInfoPtr->privateKey,
							ke_handshakeInfo->iExchangeHashcontext, NULL );
		}
	else return status;

	krnlSendNotifier( ke_handshakeInfo->iExchangeHashcontext,
					  IMESSAGE_DECREFCOUNT );
	ke_handshakeInfo->iExchangeHashcontext = CRYPT_ERROR;
	if( cryptStatusOK( status ) )
		status = sSkip( &stream, sigLength );
	if( cryptStatusOK( status ) )
		status = wrapPacketSSH2( sessionInfoPtr, &stream, 0, FALSE, TRUE );
	if( cryptStatusError( status ) )
		{
		sMemDisconnect( &stream );
		return( status );
		}

	if( cryptStatusOK( status ) )
		status = sendPacketSSH2( sessionInfoPtr, &stream, TRUE );

	sMemDisconnect( &stream );
	if( cryptStatusError( status ) )
		return( status );

	return( CRYPT_OK );
}


int ke_DH_ROUTER( SESSION_INFO *sessionInfoPtr, STREAM * stream1, int packetType )
{
	if( packetType == SSH2_MSG_KEXINIT ) return ke_SSH2_MSG_KEXINIT( sessionInfoPtr, stream1 );

	if( ke_handshakeInfo == NULL ) return CRYPT_ERROR_BADDATA;

	if( packetType == SSH2_MSG_NEWKEYS ) return ke_SSH2_MSG_NEWKEYS(sessionInfoPtr);
//	if( ke_handshakeInfo->requestedServerKeySize > 0 )
	{
		if( packetType == SSH2_MSG_KEXDH_GEX_REQUEST_OLD ) return ke_SSH2_MSG_KEXDH_GEX_REQUEST( sessionInfoPtr, stream1, packetType );
	}

	if( packetType == SSH2_MSG_KEXDH_GEX_REQUEST_NEW ) return ke_SSH2_MSG_KEXDH_GEX_REQUEST( sessionInfoPtr, stream1, packetType );
/* this will never get called as SSH2_MSG_KEXDH_GEX_REQUEST_OLD is the same number!!
	if( packetType == SSH2_MSG_KEXDH_INIT ) return ke_SSH2_MSG_KEXDH_INIT( sessionInfoPtr, stream1, packetType );
*/
	if( packetType == SSH2_MSG_KEXDH_GEX_INIT ) return ke_SSH2_MSG_KEXDH_INIT( sessionInfoPtr, stream1, packetType );

	return CRYPT_ERROR_BADDATA;

}
#endif

/* Complete the handshake with the client */

CHECK_RETVAL STDC_NONNULL_ARG( ( 1, 2 ) ) \
static int completeServerHandshake( INOUT SESSION_INFO *sessionInfoPtr,
									INOUT SSH_HANDSHAKE_INFO *handshakeInfo )
	{
	STREAM stream;
	BOOLEAN userInfoPresent = FALSE;
	int length, status;

	assert( isWritePtr( sessionInfoPtr, sizeof( SESSION_INFO ) ) );
	assert( isWritePtr( handshakeInfo, sizeof( SSH_HANDSHAKE_INFO ) ) );

	/* If this is the first time through, set up the security information 
	   and wait for the client's pre-authentication */
	if( !( sessionInfoPtr->flags & SESSION_PARTIALOPEN ) )
		{
		BYTE stringBuffer[ CRYPT_MAX_TEXTSIZE + 8 ];
		int stringLength;

		/* If the caller has supplied user information to match against we
		   require a match against the fixed caller-supplied information 
		   rather than accepting what the client sends us and passing it 
		   back to the caller to check */
		if( findSessionInfo( sessionInfoPtr->attributeList, 
							 CRYPT_SESSINFO_USERNAME ) != NULL )
			userInfoPresent = TRUE;

		/* Set up the security information required for the session */
		status = initSecurityInfo( sessionInfoPtr, handshakeInfo );
		if( cryptStatusError( status ) )
			return( status );

		/* Wait for the client's change cipherspec message.  From this point
		   on the read channel is in the secure state */
		status = readHSPacketSSH2( sessionInfoPtr, SSH2_MSG_NEWKEYS, 
								   ID_SIZE );
		if( cryptStatusError( status ) )
			return( status );
		sessionInfoPtr->flags |= SESSION_ISSECURE_READ;

		/* Wait for the client's pre-authentication packets, which aren't 
		   used for any authentication but which are required anyway by the
		   protocol.  For some reason SSH requires the use of two messages 
		   where one would do, first an "I'm about to authenticate" packet 
		   and then an "I'm authenticating" packet after that.  Since the 
		   former isn't useful for anything we clear it to get it out of the 
		   way so that we can perform the actual authentication:

			byte	type = SSH2_MSG_SERVICE_REQUEST
			string	service_name = "ssh-userauth"

			byte	type = SSH2_MSG_SERVICE_ACCEPT
			string	service_name = "ssh-userauth" */
		status = length = \
			readHSPacketSSH2( sessionInfoPtr, SSH2_MSG_SERVICE_REQUEST,
							  ID_SIZE + sizeofString32( "", 8 ) );
		if( cryptStatusError( status ) )
			return( status );
		sMemConnect( &stream, sessionInfoPtr->receiveBuffer, length );
		status = readString32( &stream, stringBuffer, CRYPT_MAX_TEXTSIZE,
							   &stringLength );
		sMemDisconnect( &stream );
		if( cryptStatusError( status ) || \
			stringLength != 12 || memcmp( stringBuffer, "ssh-userauth", 12 ) )
			{
			retExt( CRYPT_ERROR_BADDATA,
					( CRYPT_ERROR_BADDATA, SESSION_ERRINFO, 
					  "Invalid service request packet" ) );
			}
		status = openPacketStreamSSH( &stream, sessionInfoPtr, 
									  SSH2_MSG_SERVICE_ACCEPT );
		if( cryptStatusError( status ) )
			return( status );
		status = writeString32( &stream, "ssh-userauth", 12 );
		if( cryptStatusOK( status ) )
			status = sendPacketSSH2( sessionInfoPtr, &stream, FALSE );
		sMemDisconnect( &stream );
		if( cryptStatusError( status ) )
			return( status );
		}

	/* Process the client's authentication */
	status = processServerAuth( sessionInfoPtr, userInfoPresent );
	if( cryptStatusError( status ) )
		return( status );

	/* Handle the channel open */
	status = length = \
		readHSPacketSSH2( sessionInfoPtr, SSH2_MSG_CHANNEL_OPEN,
						  ID_SIZE + sizeofString32( "", 4 ) + \
							UINT32_SIZE + UINT32_SIZE + UINT32_SIZE );
	if( cryptStatusError( status ) )
		return( status );
	sMemConnect( &stream, sessionInfoPtr->receiveBuffer, length );
	status = processChannelOpen( sessionInfoPtr, &stream );
	sMemDisconnect( &stream );
#if 1
	return( status );
#else	/* If we handle the following inline as part of the general read code
		   it requires that the user try and read some data (with a non-zero
		   timeout) right after the connect completes.  Because it's awkward
		   to have to rely on this we provide optional code to explicitly 
		   clear the pipe here.  This code stops as soon as the first data
		   channel-opening request is received, with further requests being
		   handled inline as part of the standard data-read handling.  The
		   reason why this isn't enabled by default is that it's possible to
		   encounter a client that doesn't send anything beyond the initial
		   channel open, which means that we'd hang around waiting for a
		   control message until we time out */
	if( cryptStatusError( status ) )
		return( status );

	/* Process any further junk that the caller may throw at us until we get
	   a request that we can handle, indicated by an OK_SPECIAL response */
	do
		{
		status = length = \
			readHSPacketSSH2( sessionInfoPtr, SSH2_MSG_SPECIAL_REQUEST, 8 );
		if( !cryptStatusError( status ) )
			{
			sMemConnect( &stream, sessionInfoPtr->receiveBuffer, length );
			status = processChannelControlMessage( sessionInfoPtr, &stream );
			sMemDisconnect( &stream );
			}
		}
	while( cryptStatusOK( status ) );
	return( ( status == OK_SPECIAL ) ? CRYPT_OK : status );
#endif /* 1 */
	}

/****************************************************************************
*																			*
*							Session Access Routines							*
*																			*
****************************************************************************/

STDC_NONNULL_ARG( ( 1, 2 ) ) \
void initSSH2serverProcessing( STDC_UNUSED SESSION_INFO *sessionInfoPtr,
							   INOUT SSH_HANDSHAKE_INFO *handshakeInfo )
	{
	assert( isWritePtr( sessionInfoPtr, sizeof( SESSION_INFO ) ) );
	assert( isWritePtr( handshakeInfo, sizeof( SSH_HANDSHAKE_INFO ) ) );

	handshakeInfo->beginHandshake = beginServerHandshake;
	handshakeInfo->exchangeKeys = exchangeServerKeys;
	handshakeInfo->completeHandshake = completeServerHandshake;
	}
#endif /* USE_SSH */
