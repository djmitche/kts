#pragma once

#include "..\shared\KFlags.hxx"
#include "..\shared\KSsh.hxx"
#include ".\KCommUtils.hxx"
#include <vector>

class KPortForward
{
	/*==============================================================================
	 * var
	 *=============================================================================*/
private:
	KSsh * ssh;
	KFlags * flags;
	bool first;

	/*==============================================================================
	 * internal struct
	 *=============================================================================*/
	struct FORWARD_INFO
	{
		int channel;
		SOCKET sock;
	};

	std::vector<FORWARD_INFO> forwards;

private:
	/*==============================================================================
	 * params
	 *=============================================================================*/
	struct Params
	{
		int max_portforward_channels;

		Params( )
		{
			KIni ini;
			ini.File( ".\\kts.ini" );

			ini.GetKey( "KSession", "max_portforward_channels", this->max_portforward_channels );
		}
	} params;
	

public:
	/*==============================================================================
	 * constructor
	 *=============================================================================*/
	KPortForward( KSsh * ssh, KFlags * flags )
	{
		ktrace_in( );
		ktrace( "KPortForward::KPortForward( )" );

		this->flags = flags;
		this->ssh = ssh;
		this->first = true;
	}

public:
	// =============================================================================
	// is resource request
	// =============================================================================
	bool IsResourceRequest()
	{
		ktrace_in( );
		ktrace( "KPortForward::IsResourceRequest( )" );

		return this->ssh->status == CRYPT_ENVELOPE_RESOURCE;
	}
public:
	// =============================================================================
	// get port forward request
	// =============================================================================
	bool GetPortForwardRequest(std::string & request)
	{
		ktrace_in( );
		ktrace( "KPortForward::GetPortForwardRequest( )" );

		return this->ssh->GetPortForwardRequest(request);
	}
public:
	// =============================================================================
	// transfer data on channels ( and close inactive channels/sockets )
	// =============================================================================
	bool TransferChannelData( )
	{
		ktrace_in( );
		ktrace( "KPortForward::TransferChannelData( )" );

		KCommUtils ssh_comm( NULL, this->ssh, this->flags, true );

		// receive from socket and transfer to ssh party
		for( size_t i = 0; i < this->forwards.size(); i++ )
		{
			FORWARD_INFO fi = this->forwards[ i ];
			std::string buff;
			buff = "";

			if( fi.channel == -1 && fi.sock == INVALID_SOCKET ) continue;

			KSocket s(fi.sock);
			KCommUtils sock_comm( &s, NULL, this->flags, false );

			if( !sock_comm.Receive( buff ) )
			{
				this->CloseForwardChannel( (int)i );
				continue;
			}

			// we will push even empty data to detect inactive ssh channels here
			if( !ssh_comm.Send( buff, fi.channel) )
			{
				this->CloseForwardChannel( (int)i );
				continue;
			}
		}

		std::string buff1;
		// receive from ssh party and transfer to socket
		if( !ssh_comm.Receive( buff1 ) )
		{
			if(this->IsResourceRequest()) return true;
			else return false;
		}
		if( this->ssh->status == CRYPT_ERROR_TIMEOUT ) return true;

		int channel = this->ssh->channel;

		for( size_t i = 0; i < this->forwards.size(); i++ )
		{
			FORWARD_INFO fi = this->forwards[ i ];

			if( fi.channel == channel )
			{
				KSocket s(fi.sock);
				KCommUtils sock_comm( &s, NULL, this->flags, false );

				if( !sock_comm.Send( buff1 ) )
				{
					this->CloseForwardChannel( (int)i );
				}
				break;
			}
		}

		return true;
	}

private:
	// =============================================================================
	// close ssh channel and socket
	// =============================================================================
	void CloseForwardChannel( int i )
	{
		ktrace_in( );
		ktrace( "KPortForward::CloseForwardChannel( " << i << " )" );

		FORWARD_INFO fi = this->forwards[ i ];
		ktrace( "sock = " << fi.sock << " channel = " << fi.channel );

		if( fi.sock != INVALID_SOCKET ) 
		{
			KSocket s(fi.sock);
			s.Close( false );
		}

		if( fi.channel != -1 )
		{
			this->ssh->CloseChannel( fi.channel );
			klog( "close port forward channel " << fi.channel );

			std::stringstream str;
			str << "\r\nclose port forward channel " << fi.channel;

			KCommUtils ssh_comm( NULL, this->ssh, this->flags, true );
			ssh_comm.Send( str.str(), 1 );
		}

		this->forwards[ i ].sock = INVALID_SOCKET;
		this->forwards[ i ].channel = -1;
	}

private:
	// =============================================================================
	// create connected socket from the request
	// =============================================================================
	SOCKET CreateForwardSocket( const std::string & request )
	{
		ktrace_in( );
		ktrace( "KPortForward::CreateForwardSocket( " << request << " )" );

		std::string server;
		u_short port;

		size_t sep_pos = request.find(":");
		if( sep_pos == std::string::npos )
		{
			klog("unparsable port forward request [ " << request << " ]" );
			return INVALID_SOCKET;
		}

		server = request.substr(0, sep_pos);
		port = (u_short)atoi(request.substr( sep_pos + 1 ).c_str());

		KSocket sock;
		return sock.Connect( server, port );
	}

public:
	// =============================================================================
	// create port forward channel
	// =============================================================================
	void CreatePortForwardChannel(const std::string & request)
	{
		ktrace_in( );
		ktrace( "KPortForward::CreatePortForwardChannel( " << this->ssh->channel << ", " << request << " )" );

		KCommUtils ssh_comm( NULL, this->ssh, this->flags, true );

		if( this->first )
		{
			ssh_comm.Send( "\r\n=================================", 1 );
			ssh_comm.Send( "\r\n entering port forward mode", 1 );
			ssh_comm.Send( "\r\n=================================", 1 );
			ssh_comm.Send( "\r\n", 1 );
			ssh_comm.Send( "\r\n", 1 );
			this->first = false;
		}

		klog( "create port forward channel " << this->ssh->channel << " for " << request );

		std::stringstream str;
		str << "\r\ncreate port forward channel " << this->ssh->channel << " for " << request;
		ssh_comm.Send( str.str(), 1 );

		size_t i = 0;

		// find free forward info entry
		for( i = 0; i < this->forwards.size(); i++)
		{
			if( this->forwards[i].channel == -1 && this->forwards[i].sock == INVALID_SOCKET ) break;
		}

		FORWARD_INFO fi;

		if( i < this->forwards.size() )
		{
			// found free entry, update this entry

			fi.channel = this->ssh->channel;
			fi.sock = this->CreateForwardSocket( request );
			this->forwards[i] = fi;
		}
		else
		{
			// no free entry, create new entry
			if( this->forwards.size() < (unsigned)this->params.max_portforward_channels )
			{
				fi.channel = this->ssh->channel;
				fi.sock = this->CreateForwardSocket( request );
				this->forwards.push_back( fi );
			}
			else
			{
				klog( "max_portforward_channels reached channel " << this->ssh->channel << "not created" );

				std::stringstream str;
				str << "\r\nmax_portforward_channels reached channel " << this->ssh->channel << "not created";
				ssh_comm.Send( str.str(), 1 );

				fi.channel = this->ssh->channel;
				fi.sock = INVALID_SOCKET;

				// we will create unactive channel socket
				if( this->forwards.size() == (unsigned)this->params.max_portforward_channels )
				{
					this->forwards.push_back( fi );
				}
				else
				{
					i = this->params.max_portforward_channels;
					this->forwards[ i ] = fi;
				}
			}
		}
	}
};