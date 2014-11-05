#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

#include <windows.h>

#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <share.h>

#include "..\shared\kts.h"
#include "..\shared\KTrace.hxx"

#define byte unsigned char
#define uint32 unsigned int
#define uint64 unsigned __int64

class KSftp
{
	/*==============================================================================
	 * user token
	 *=============================================================================*/
public:
	static void SetUserToken( const HANDLE token )
	{
		ktrace_in();
		ktrace("KSftp::SetUserToken( " << token<< " )");
		GetUserToken( ) = token;
	}
	static HANDLE & GetUserToken( )
	{
		ktrace_in();
		ktrace("KSftp::GetUserToken( )");

		static HANDLE token;

		return token;
	}

private:
	/*==============================================================================
	 * packet types
	 *=============================================================================*/
	const static byte SSH_FXP_INIT =			1;
	const static byte SSH_FXP_VERSION =			2;
	const static byte SSH_FXP_OPEN =			3;
	const static byte SSH_FXP_CLOSE =			4;
	const static byte SSH_FXP_READ =			5;
	const static byte SSH_FXP_WRITE =			6;
	const static byte SSH_FXP_LSTAT =			7;
	const static byte SSH_FXP_FSTAT =			8;
	const static byte SSH_FXP_SETSTAT =			9;
	const static byte SSH_FXP_FSETSTAT =		10;
	const static byte SSH_FXP_OPENDIR =			11;
	const static byte SSH_FXP_READDIR =			12;
	const static byte SSH_FXP_REMOVE =			13;
	const static byte SSH_FXP_MKDIR =			14;
	const static byte SSH_FXP_RMDIR =			15;
	const static byte SSH_FXP_REALPATH =		16;
	const static byte SSH_FXP_STAT =			17;
	const static byte SSH_FXP_RENAME =			18;
	const static byte SSH_FXP_READLINK =		19;
	const static byte SSH_FXP_LINK =			21;
	const static byte SSH_FXP_BLOCK =			22;
	const static byte SSH_FXP_UNBLOCK =			23;

	const static byte SSH_FXP_STATUS =			101;
	const static byte SSH_FXP_HANDLE =			102;
	const static byte SSH_FXP_DATA =			103;
	const static byte SSH_FXP_NAME =			104;
	const static byte SSH_FXP_ATTRS =			105;

	const static byte SSH_FXP_EXTENDED =		200;
	const static byte SSH_FXP_EXTENDED_REPLY =	201;

	/*==============================================================================
	 * SSH_FXP_STATUS
	 *=============================================================================*/
	const static byte SSH_FX_OK=0;
	const static byte SSH_FX_EOF=1;
	const static byte SSH_FX_NO_SUCH_FILE=2;
	const static byte SSH_FX_PERMISSION_DENIED=3;
	const static byte SSH_FX_FAILURE=4;
	const static byte SSH_FX_BAD_MESSAGE=5;
	const static byte SSH_FX_NO_CONNECTION=6;
	const static byte SSH_FX_CONNECTION_LOST=7;
	const static byte SSH_FX_OP_UNSUPPORTED=8;

	/*==============================================================================
	 * ATTRS valid-attribute-flags
	 *=============================================================================*/
	const static uint32 SSH_FILEXFER_ATTR_SIZE=0x00000001;
	const static uint32 SSH_FILEXFER_ATTR_UIDGID=0x00000002;
	const static uint32 SSH_FILEXFER_ATTR_PERMISSIONS=0x00000004;
	const static uint32 SSH_FILEXFER_ATTR_ACMODTIME=0x00000008;
	const static uint32 SSH_FILEXFER_ATTR_EXTENDED=0x80000000;

	/*==============================================================================
	 * open file handle
	 *=============================================================================*/
   	const static uint32 SSH_FXF_READ=0x00000001;
   	const static uint32 SSH_FXF_WRITE=0x00000002;
   	const static uint32 SSH_FXF_APPEND=0x00000004;
   	const static uint32 SSH_FXF_CREAT=0x00000008;
   	const static uint32 SSH_FXF_TRUNC=0x00000010;
   	const static uint32 SSH_FXF_EXCL=0x00000020;

	/*==============================================================================
	 * session variables
	 *=============================================================================*/
	static uint32 client_version;

	/*==============================================================================
	 * buff manipulation routines
	 *=============================================================================*/
	static void byte2b( const byte v, std::string & buff )
	{
		ktrace_in();
		ktrace("KSftp::byte2b( )");

		buff.push_back( v );
	}

	static byte b2byte( const std::string & buff, unsigned & l, bool & ret )
	{
		ktrace_in();
		ktrace("KSftp::b2byte( )");

		if( l >= buff.length( ) )
		{
			ret = false;
			return 0;
		}

		byte v = buff[ l ];
		l++;
		ret = true;
		return v;
	}

	static void uint322b( const uint32 v, std::string & buff )
	{
		ktrace_in();
		ktrace("KSftp::uint322b( )");

		buff.push_back( ( unsigned char )( 0xFF & ( v >> 24 ) ) );
		buff.push_back( ( unsigned char )( 0xFF & ( v >> 16 ) ) );
		buff.push_back( ( unsigned char )( 0xFF & ( v >>  8 ) ) );
		buff.push_back( ( unsigned char )( 0xFF & ( v >>  0 ) ) );
	}

	static uint32 b2uint32( const std::string & buff, unsigned & l, bool & ret )
	{
		ktrace_in();
		ktrace("KSftp::b2uint32( )");

		if( l + 3 >= buff.length( ) )
		{
			ret = false;
			return 0;
		}

		uint32 v = 
				   ( (unsigned char)buff[ l + 0 ] << 24 )
				 | ( (unsigned char)buff[ l + 1 ] << 16 )
				 | ( (unsigned char)buff[ l + 2 ] <<  8 )
				 | ( (unsigned char)buff[ l + 3 ] <<  0 );

		l += 4;
		ret = true;
		return v;
	}

	static void uint642b( const uint64 v, std::string & buff )
	{
		ktrace_in();
		ktrace("KSftp::uint642b( )");

		buff.push_back( ( unsigned char )( 0xFF & ( v >> 56 ) ) );
		buff.push_back( ( unsigned char )( 0xFF & ( v >> 48 ) ) );
		buff.push_back( ( unsigned char )( 0xFF & ( v >> 40 ) ) );
		buff.push_back( ( unsigned char )( 0xFF & ( v >> 32 ) ) );


		buff.push_back( ( unsigned char )( 0xFF & ( v >> 24 ) ) );
		buff.push_back( ( unsigned char )( 0xFF & ( v >> 16 ) ) );
		buff.push_back( ( unsigned char )( 0xFF & ( v >>  8 ) ) );
		buff.push_back( ( unsigned char )( 0xFF & ( v >>  0 ) ) );

	}

	static uint64 b2uint64( const std::string & buff, unsigned & l, bool & ret )
	{
		ktrace_in();
		ktrace("KSftp::b2uint64( )");

		if( l + 7 >= buff.length( ) )
		{
			ret = false;
			return 0;
		}

		uint64 v = 
				   ( (unsigned char)buff[ l + 0 ] << 56 )
				 | ( (unsigned char)buff[ l + 1 ] << 48 )
				 | ( (unsigned char)buff[ l + 2 ] << 40 )
				 | ( (unsigned char)buff[ l + 3 ] << 32 )

				 | ( (unsigned char)buff[ l + 4 ] << 24 )
				 | ( (unsigned char)buff[ l + 5 ] << 16 )
				 | ( (unsigned char)buff[ l + 6 ] <<  8 )
				 | ( (unsigned char)buff[ l + 7 ] <<  0 );

		l += 8;
		ret = true;
		return v;
	}

	static void string2b(const std::string & v, std::string & buff )
	{
		ktrace_in();
		ktrace("KSftp::string2b( )");

		uint322b( ( unsigned int )v.length(), buff );
		buff.append( v );
	}

	static std::string b2string( const std::string & buff, unsigned & l, bool & ret )
	{
		ktrace_in();
		ktrace("KSftp::b2string( )");

		if( l >= buff.length( ) )
		{
			ret = false;
			return "";
		}

		uint32 len = b2uint32( buff, l, ret );
		if( !ret ) 
		{
			return "";
		}

		std::string v = "";
		for( unsigned int i = 0; i < len; i++ )
		{
			if( l >= buff.length( ) ) 
			{
				ret = false;
				return "";
			}
			v.push_back( buff[ l ] );
			l++;
		}

		return v;
	}

	static void raw2b(const std::string & v, std::string & buff )
	{
		ktrace_in();
		ktrace("KSftp::raw2b( )");

		buff.append( v );
	}



	/*==============================================================================
	 * file utils
	 *=============================================================================*/
	struct ATTRS
	{
		uint32 flags;
		uint64 size;           // present only if flag SSH_FILEXFER_ATTR_SIZE
		uint32 uid;            // present only if flag SSH_FILEXFER_ATTR_UIDGID
		uint32 gid;            // present only if flag SSH_FILEXFER_ATTR_UIDGID
		uint32 permissions;    // present only if flag SSH_FILEXFER_ATTR_PERMISSIONS
		uint32 atime;          // present only if flag SSH_FILEXFER_ACMODTIME
		uint32 mtime;          // present only if flag SSH_FILEXFER_ACMODTIME
		//uint32   extended_count present only if flag SSH_FILEXFER_ATTR_EXTENDED
		//string   extended_type
		//string   extended_data

		bool Flag( uint32 flag )
		{
			ktrace_in();
			ktrace("KSftp::ATTRS::Flag( )");

			return ( ( this->flags & flag ) != 0 );
		}

		int Length( )
		{
			ktrace_in();
			ktrace("KSftp::ATTRS::Length( )");

			int l = 0;

			//uint32 flags;
			l += 4;

			//uint64 size;
			if( this->Flag( SSH_FILEXFER_ATTR_SIZE ) ) l += 8;

			//uint32 uid;
			if( this->Flag( SSH_FILEXFER_ATTR_UIDGID ) ) l += 4;

			//uint32 gid;
			if( this->Flag( SSH_FILEXFER_ATTR_UIDGID ) ) l += 4;

			//uint32 permissions;
			if( this->Flag( SSH_FILEXFER_ATTR_PERMISSIONS ) ) l += 4;

			//uint32 atime;
			if( this->Flag( SSH_FILEXFER_ATTR_ACMODTIME ) ) l += 4;

			//uint32 mtime;
			if( this->Flag( SSH_FILEXFER_ATTR_ACMODTIME ) ) l += 4;

			return l;

		}

		void ToBuffer(  std::string & buff )
		{
			ktrace_in();
			ktrace("KSftp::ATTRS::ToBuffer( )");

			uint322b( this->flags, buff );

			//uint64 size;
			if( this->Flag( SSH_FILEXFER_ATTR_SIZE ) ) uint642b( this->size, buff );

			//uint32 uid;
			if( this->Flag( SSH_FILEXFER_ATTR_UIDGID ) ) uint322b( this->uid, buff );

			//uint32 gid;
			if( this->Flag( SSH_FILEXFER_ATTR_UIDGID ) ) uint322b( this->gid, buff );

			//uint32 permissions;
			if( this->Flag( SSH_FILEXFER_ATTR_PERMISSIONS ) ) uint322b( this->permissions, buff );

			//uint32 atime;
			if( this->Flag( SSH_FILEXFER_ATTR_ACMODTIME ) ) uint322b( this->atime, buff );

			//uint32 mtime;
			if( this->Flag( SSH_FILEXFER_ATTR_ACMODTIME ) ) uint322b( this->mtime, buff );
		}
	};

	struct FILE_ATTRS
	{
		std::string filename;
		std::string longname;

		ATTRS attrs;
	};


	static bool md( const std::string & dir )
	{
		ktrace_in();
		ktrace("KSftp::md( )");

		std::string path = normalize_path( dir );
		if( dir == "." ) return false;

		if( !CreateDirectoryA( path.c_str(), NULL ) )return false;
		return true;
	}

	static std::vector<FILE_ATTRS> list_drives( )
	{
		std::vector<FILE_ATTRS> res;

		DWORD drives = GetLogicalDrives();
		// we scip A, and B drive cause these block us 
		for(int i = 2; i < 32; i++ )
		{
			if( drives & ( 1 << i ) ) 
			{
				std::string D = std::string("") + (char)('A' + i);

				FILE_ATTRS fa;
				fa.filename = D + "$";
				fa.longname = "drwxrwxrwx    1 user     user            0 Jan  1 2000 " + D + "$";
				attrs_safe( D + ":\\", fa.attrs );

				res.push_back( fa );
			}
		}

		return res;
	}

	static bool rd( std::string dir )
	{
		ktrace_in();
		ktrace("KSftp::rd(" << dir << ")");

		std::string path = normalize_path( dir );
		if( dir == "." ) return false;

		// traverse one folder up so wa wont lock the rm target
		SetCurrentDirectoryA("..");

		if( !RemoveDirectoryA( path.c_str() ) ) 
		{
			return false;
		}
		return true;
	}


	static bool cd( std::string dir, bool & isroot )
	{
		ktrace_in();
		ktrace("KSftp::cd(" << dir << ")");

		isroot = false;

		std::string path = normalize_path( dir );
		if( dir == "" ) //bitvise tunellier
		{
			char buff[1002];
			int len = GetCurrentDirectoryA( 1000, buff );

			if( len == 0 ) return false;
			return true;
		}
		if( dir == "." )
		{
			char buff[1002];
			int len = GetCurrentDirectoryA( 1000, buff );

			if( len == 0 ) return false;
			return true;
		}
		if( dir == "/" || dir == "/." )
		{
			// list drives
			isroot = true;
			return true;
		}
		if( path.length() == 5 && path.substr( 1, 4 ) == ":\\.." )
		{
				isroot = true;
				return true;
		}

		if( path != "" ) if( path[path.length() - 1] != '\\' ) path += '\\';

		if( !SetCurrentDirectoryA( path.c_str() ) ) return false;

		char buff[1002];
		int len = GetCurrentDirectoryA( 1000, buff );

		if( len == 0 ) return false;
		return true;
	}

	static bool del( std::string file )
	{
		ktrace_in();
		ktrace("KSftp::del( )");

		std::string path = normalize_path( file );

		if( DeleteFileA( path.c_str() ) )
		{
			return true;
		}
		return false;
	}
	static bool ren( std::string oldpath, std::string newpath )
	{
		ktrace_in();
		ktrace("KSftp::ren( )");

		std::string path1 = normalize_path( oldpath );
		std::string path2 = normalize_path( newpath );

		if( MoveFileA( path1.c_str(), path2.c_str() ) )
		{
			return true;
		}
		return false;
	}

	static bool create( const std::string & file )
	{
		ktrace_in();
		ktrace("KSftp::create( " << file << " )");

		std::string path = normalize_path( file );

		FILE *f = _fsopen( path.c_str(), "wb", _SH_DENYNO );

		if( f == NULL ) return false;

		fclose( f );

		return true;
	}

	static bool write( const std::string & file, uint64 offset, const std::string & data )
	{
		ktrace_in();
		ktrace("KSftp::write( )");

		std::string path = normalize_path( file );

		FILE *f = NULL;
		
		if( offset == 0 ) f = _fsopen( path.c_str(), "wb", _SH_DENYNO );
		else f = _fsopen( path.c_str(), "ab", _SH_DENYNO );
//		if( offset == 0 ) f = fopen( path.c_str(), "wb" );
//		else f = fopen( path.c_str(), "ab" );

		if( f == NULL ) return false;

		fseek( f, ( long )offset, SEEK_SET );
		fwrite( data.c_str(), sizeof( char ), data.length(), f );
		fclose( f );

		return true;
	}

	static bool read( const std::string & file, uint64 offset, uint32 len, std::string & data )
	{
		ktrace_in();
		ktrace("KSftp::read( )");

		std::string path = normalize_path( file );

		FILE *f = NULL;
		
		f = _fsopen( path.c_str(), "rb", _SH_DENYNO );
//		f = fopen( path.c_str(), "rb" );

		if( f == NULL ) return false;

		fseek( f, ( long )offset, SEEK_SET );

		char * buff = ( char * )malloc( len );

		size_t res = fread( buff, sizeof( char ), len, f );

		fclose( f );

		data.assign( buff, res );

		free( buff );

		return true;
	}
	static uint32 toUTCseconds(FILETIME ft)
	{
		//They are represented as seconds from Jan 1, 1970 in UTC.

		SYSTEMTIME sys1970 = {0};
		sys1970.wYear = 1970;
		sys1970.wMonth = 1;
		sys1970.wDay=1;

		FILETIME utc1970;
		SystemTimeToFileTime(&sys1970, &utc1970);

		ULARGE_INTEGER t1; 
		t1.HighPart = ft.dwHighDateTime; 
		t1.LowPart = ft.dwLowDateTime; 

		ULARGE_INTEGER t2; 
		t2.HighPart = utc1970.dwHighDateTime; 
		t2.LowPart = utc1970.dwLowDateTime; 

		return (uint32)(( t1.QuadPart - t2.QuadPart ) / 10000000);
	}

	static bool attrs( const std::string & file, ATTRS & a )
	{
		ktrace_in();
		ktrace("KSftp::attrs( )");

		std::string path = normalize_path( file );

		WIN32_FILE_ATTRIBUTE_DATA fd;

		if( !GetFileAttributesExA( path.c_str(), GetFileExInfoStandard, &fd) ) return false;

		FILETIME atime = fd.ftLastAccessTime;
		FILETIME mtime = fd.ftLastWriteTime;

/*
		The `size' field specifies the size of the file in bytes.

		The `atime' and `mtime' contain the access and modification times of
		   the files, respectively.  They are represented as seconds from Jan 1,
		   1970 in UTC.
*/
		a.flags = SSH_FILEXFER_ATTR_SIZE | SSH_FILEXFER_ATTR_ACMODTIME | SSH_FILEXFER_ATTR_UIDGID | SSH_FILEXFER_ATTR_PERMISSIONS;
		a.size = ((fd.nFileSizeHigh * (MAXDWORD+1)) + (uint64)fd.nFileSizeLow);
		a.atime = toUTCseconds(atime);
		a.mtime = toUTCseconds(mtime);
		a.gid = 0;
		a.uid = 0;
		if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			// this is folder: drwxrwxrwx = 16895
			a.permissions = 16895;
		}
		else
		{
			// this is file: -rwxrwxrwx = 33279
			a.permissions = 33279;
		}
		
		return true;
	}

	static bool exist( const std::string & file )
	{
		ktrace_in();
		ktrace("KSftp::exist( )");

		std::string path = normalize_path( file );

		WIN32_FILE_ATTRIBUTE_DATA fd;

		if( !GetFileAttributesExA( path.c_str(), GetFileExInfoStandard, &fd) ) return false;

		return true;
	}

	static std::vector<FILE_ATTRS> ls( std::string path )
	{
		ktrace_in();
		ktrace("KSftp::ls( )");

		std::vector<FILE_ATTRS> res;

		std::string path1 = path + "\\*";

		WIN32_FIND_DATAA FindFileData;
		HANDLE hFind = INVALID_HANDLE_VALUE;

		hFind = FindFirstFileA(path1.c_str(), &FindFileData);

		if (hFind == INVALID_HANDLE_VALUE) return res;

		FILE_ATTRS fa;
		fa.filename = FindFileData.cFileName;
		fa.longname = ffd2string( FindFileData );
		attrs_safe( path + "\\" + fa.filename, fa.attrs );

		res.push_back( fa );

		while (FindNextFileA(hFind, &FindFileData) != 0)
		{
			FILE_ATTRS fa;
			fa.filename = FindFileData.cFileName;
			fa.longname = ffd2string( FindFileData );
//			fa.attrs.flags = 0;
			attrs_safe( path + "\\" + fa.filename, fa.attrs );
			res.push_back( fa );
		}

		FindClose(hFind);

		return res;
	}



	static std::string ffd2string( std::string file )
	{
		ktrace_in();
		ktrace("KSftp::ffd2string( " << file << " )");

		WIN32_FIND_DATAA FindFileData;
		HANDLE hFind = INVALID_HANDLE_VALUE;

		hFind = FindFirstFileA(file.c_str(), &FindFileData);

		if (hFind == INVALID_HANDLE_VALUE) return "";

		std::string longname = ffd2string( FindFileData );

		FindClose(hFind);

		return longname;
	}

	static std::string ToMonth(int m)
	{
		switch(m)
		{
		case 1: return "Jan";
		case 2: return "Feb";
		case 3: return "Mar";
		case 4: return "Apr";
		case 5: return "May";
		case 6: return "Jun";
		case 7: return "Jul";
		case 8: return "Aug";
		case 9: return "Sep";
		case 10: return "Oct";
		case 11: return "Nov";
		case 12: return "Dec";
		}

		return "Err";
	}

	static std::string ffd2string( WIN32_FIND_DATAA fd )
	{
		ktrace_in();
		ktrace("KSftp::ffd2string( )");

		SYSTEMTIME time;

		FileTimeToSystemTime( &fd.ftCreationTime, &time );

		std::stringstream s;

		if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			s << "drwxrwxrwx    1 user     user   "
				// size
				<< " " << std::setw( 9 ) <<  0
				// date
				<< " " << ToMonth( time.wMonth ) << " " << std::setw( 2 ) <<  time.wDay
 				// hour:minute
				<< " " << std::setw( 2 ) << std::setfill( '0' ) <<  time.wHour << ":" << std::setw( 2 ) << std::setfill( '0' ) <<  time.wMinute 
				// filename
				<< " " << fd.cFileName;
		}
		else
		{
 			s << "-rwxrwxrwx    1 user     user   "
				// size
				<< " " << std::setw( 9 ) <<  ((fd.nFileSizeHigh * (MAXDWORD+1)) + (uint64)fd.nFileSizeLow)
				// date
				<< " " << ToMonth( time.wMonth ) << " " << std::setw( 2 ) <<  time.wDay
 				// hour:minute
				<< " " << std::setw( 2 ) << std::setfill( '0' ) <<  time.wHour << ":" << std::setw( 2 ) << std::setfill( '0' ) <<  time.wMinute 
				// filename
				<< " " << fd.cFileName;
		}

		return s.str();
	}

	static std::string fix_slashes( const std::string & path1 )
	{
		ktrace_in();
		ktrace("KSftp::fix_slashes( )");

		std::string path = path1;
		for( unsigned i = 0; i < path.length(); i++ ) if( path[i] == '/' ) path[i] = '\\';
		return path;
	}

	static std::string fix_backslashes( const std::string & path1 )
	{
		ktrace_in();
		ktrace("KSftp::fix_backslashes( )");

		std::string path = path1;
		for( unsigned i = 0; i < path.length(); i++ ) if( path[i] == '\\' ) path[i] = '/';
		return path;
	}


	static std::string translate_to_nix(const std::string & pathi )
	{
		// we will translate the win path to *nix like path
		// as most of the sftp clients speak *nix language only
		// also we will change the : to $ in the drive letter or expandrive will brak at us
		// c:\documents and settings\kpym -> /c$/documents and settings/kpym
		std::string path = pathi;
		path = "/" + path;
		if( path.length() > 1 && path[0] == '/' && path[2] == ':' ) path[2] = '$';

		return path;
	}

	static std::string translate_from_nix(const std::string & pathi )
	{
		std::string path = pathi;
		// we will reverse translate  $ to : in the drive letter
		// /X$/X$/X$ => /X:/X$/X$ 
		if( path.length() > 1 && path[0] == '/' && path[2] == '$' ) path[2] = ':';

		return path;
	}

	static std::string normalize_path( const std::string & pathi )
	{
		ktrace_in();
		ktrace("KSftp::normalize_path( " << pathi << " )");

		std::string path = translate_from_nix(pathi);

		// Filezilla suxx at paths
		// it sends sheats like this:
		// cd C:/Documents and Settings/kpym/My Documents/C:/Documents and Settings/kpym\.gimp-2.4
		// check if this is filezilla like bug
		if( path.length() > 1 )
		{	
			int i;
			for( i = (int)path.length() - 1; i >= 0; i-- ) if( path[ i ] == ':' ) break;
			if( i > 0 ) path = path.substr( i - 1 );
		}


		unsigned sep = 0;
		//split dir
		for( sep = 0; sep < path.length(); sep++ ) if( path[ sep ] == '/' ) break;

 		// no sep just return dir
		if( sep >= path.length() ) return path;

		std::string path1 = fix_slashes( path.substr( 0, sep ) );
		std::string path2 = fix_slashes( path.substr( sep + 1 ) );

 		// first is sep
		if( path1.length() == 0 ) return path2;

		// no sep just return dir
		if( path2.length() == 0 ) return path;


 		// drive letter
		if( path2.length() > 2 && path2[1] == ':' && path2[2] == '\\' ) return path2;


		// root directory
		if( path2.length() > 0 && path2[0] == '\\' ) 
		{
			// we have a drive letter add it to the root path
			if(path1.length() >= 2 && path[1] == ':')
			{
				path1 = path1.substr(0, 2);
				return path1 + path2;
			}
			else
			{
				return path2;
			}
		}

 		return path1 + "\\" + path2;
	}

	static void trace_packet( std::string mode, std::string buff )
	{
		return;
/*
		printf( "%s:\r\n", mode.c_str() );
		for( unsigned i = 0; i < buff.length(); i++ )
		{
			printf( "%2.2X ", (unsigned char)buff[i]);
		}
		printf( "\r\n" );
*/
	}




	/*==============================================================================
	 * safe functions
	 *=============================================================================*/

	static std::vector<FILE_ATTRS> ls_safe( const std::string & dir )
	{
		ktrace_in();
		ktrace( "KSftp::ls_safe(" << dir << ")" );

		std::vector<FILE_ATTRS> ret;

		if( !ImpersonateLoggedOnUser( GetUserToken( ) ) ) 
		{
			kerror( "ImpersonateLoggedOnUser" );
			return ret;
		}
		ret = ls( dir );
		if( !RevertToSelf( ) )
		{
			kerror( "RevertToSelf" );
			return ret;
		}

		return ret;
	}

	static bool md_safe( const std::string & dir )
	{
		ktrace_in();
		ktrace( "KSftp::md_safe(" << dir << ")" );

		if( !ImpersonateLoggedOnUser( GetUserToken( ) ) ) 
		{
			kerror( "ImpersonateLoggedOnUser" );
			return false;
		}
		bool ret = md( dir );
		if( !RevertToSelf( ) )
		{
			kerror( "RevertToSelf" );
			return false;
		}

		return ret;
	}

	static bool rd_safe( const std::string & dir )
	{
		ktrace_in();
		ktrace( "KSftp::rd_safe(" << dir << ")" );

		if( !ImpersonateLoggedOnUser( GetUserToken( ) ) )
		{
			kerror( "ImpersonateLoggedOnUser" );
			return false;
		}
		bool ret = rd( dir );
		if( !RevertToSelf( ) )
		{
			kerror( "RevertToSelf" );
			return false;
		}

		return ret;
	}

	static bool cd_safe( const std::string & dir, bool & isroot )
	{
		ktrace_in();
		ktrace( "KSftp::cd_safe(" << dir << ")" );

		if( !ImpersonateLoggedOnUser( GetUserToken( ) ) ) 
		{
			kerror( "ImpersonateLoggedOnUser" );
			return false;
		}

		bool ret = cd( dir, isroot );
		if( !RevertToSelf( ) )
		{
			kerror( "RevertToSelf" );
			return false;
		}

		return ret;
	}

	static bool del_safe( const std::string & dir )
	{
		ktrace_in();
		ktrace( "KSftp::del_safe(" << dir << ")" );

		if( !ImpersonateLoggedOnUser( GetUserToken( ) ) ) 
		{
			kerror( "ImpersonateLoggedOnUser" );
			return false;
		}

		bool ret = del( dir );
		if( !RevertToSelf( ) )
		{
			kerror( "RevertToSelf" );
			return false;
		}

		return ret;
	}

	static bool ren_safe( const std::string & oldpath, const std::string & newpath )
	{
		ktrace_in();
		ktrace( "KSftp::ren_safe(" << oldpath << ", " << newpath << ")" );

		if( !ImpersonateLoggedOnUser( GetUserToken( ) ) ) 
		{
			kerror( "ImpersonateLoggedOnUser" );
			return false;
		}
		bool ret = ren( oldpath, newpath );
		if( !RevertToSelf( ) )
		{
			kerror( "RevertToSelf" );
			return false;
		}

		return ret;
	}
	static bool write_safe( const std::string & file, uint64 offset, const std::string & data )
	{
		ktrace_in();
		ktrace( "KSftp::write_safe(" << file << ")" );

		if( !ImpersonateLoggedOnUser( GetUserToken( ) ) ) 
		{
			kerror( "ImpersonateLoggedOnUser" );
			return false;
		}
		bool ret = write( file, offset, data );
		if( !RevertToSelf( ) )
		{
			kerror( "RevertToSelf" );
			return false;
		}

		return ret;
	}
	static bool read_safe( const std::string & file, uint64 offset, uint32 len, std::string & data )
	{
		ktrace_in();
		ktrace( "KSftp::read_safe(" << file << ")" );

		if( !ImpersonateLoggedOnUser( GetUserToken( ) ) ) 
		{
			kerror( "ImpersonateLoggedOnUser" );
			return false;
		}
		bool ret = read( file, offset, len, data );
		if( !RevertToSelf( ) )
		{
			kerror( "RevertToSelf" );
			return false;
		}

		return ret;
	}
	static bool attrs_safe( const std::string & file, ATTRS & a )
	{
		ktrace_in();
		ktrace( "KSftp::len_safe(" << file << ")" );

		if( !ImpersonateLoggedOnUser( GetUserToken( ) ) ) 
		{
			kerror( "ImpersonateLoggedOnUser" );
			return false;
		}
		bool ret = attrs( file, a );
		if( !RevertToSelf( ) )
		{
			kerror( "RevertToSelf" );
			return false;
		}

		return ret;
	}

	static bool exist_safe( const std::string & file )
	{
		ktrace_in();
		ktrace( "KSftp::exist_safe(" << file << ")" );

		if( !ImpersonateLoggedOnUser( GetUserToken( ) ) ) 
		{
			kerror( "ImpersonateLoggedOnUser" );
			return false;
		}
		bool ret = exist( file );
		if( !RevertToSelf( ) )
		{
			kerror( "RevertToSelf" );
			return false;
		}

		return ret;
	}
	
	static bool create_safe( const std::string & file )
	{
		ktrace_in();
		ktrace( "KSftp::create_safe(" << file << ")" );

		if( !ImpersonateLoggedOnUser( GetUserToken( ) ) ) 
		{
			kerror( "ImpersonateLoggedOnUser" );
			return false;
		}
		bool ret = create( file );
		if( !RevertToSelf( ) )
		{
			kerror( "RevertToSelf" );
			return false;
		}

		return ret;
	}
	/*==============================================================================
	 * handle routines
	 *=============================================================================*/
	static const int MAX_OPEN_HANDLES = 200;	
	struct SftpHandle
	{
		const static int KSFTP_HANDLE_CLOSED = 0;
		const static int KSFTP_HANDLE_OPENED = 1;
		const static int KSFTP_HANDLE_EOF = 2;

		std::string file;
		int state;

		SftpHandle()
		{
			this->file = "";
			this->state = KSFTP_HANDLE_CLOSED;
		}
	};

	static SftpHandle & SftpHandles( int i )
	{
		static SftpHandle _handles[200];

		return _handles[i];
	}
	static std::string OpenSftpHandle(std::string file)
	{
		ktrace_in();
		ktrace("KSftp::OpenSftpHandle( )");

		int i = 1;
		for( i = 1; i < MAX_OPEN_HANDLES; i++ )
		{
			if( SftpHandles(i).state == SftpHandle::KSFTP_HANDLE_CLOSED ) break;
		}

		if( i >= MAX_OPEN_HANDLES ) return "";

		SftpHandles(i).file = file;
		SftpHandles(i).state = SftpHandle::KSFTP_HANDLE_OPENED;

		std::stringstream s;

		s << i << " " << file;

		return s.str();
	}

	static int ParseSftpHandle( std::string handle )
	{
		ktrace_in();
		ktrace("KSftp::ParseSftpHandle( )");

		unsigned i = 0;
		for( i = 0; i < handle.length(); i++ )
		{
			if( handle[i] == ' ' ) break;
		}

		if( i >= handle.length() ) return 0;

		std::string tmp = handle.substr( 0, i );

		i = ( unsigned )atoi(tmp.c_str());

		if( i == 0 ) return 0;
		if( i >= MAX_OPEN_HANDLES ) return 0;

		std::stringstream s;

		s << i << " " << SftpHandles(i).file;

		if(  s.str() != handle ) return 0;

		return i;
	}

	static void CloseSftpHandle( std::string handle)
	{
		ktrace_in();
		ktrace("KSftp::CloseSftpHandle( )");

		int i = ParseSftpHandle(handle);
		if( i == 0 ) return;
		SftpHandles(i).state = SftpHandle::KSFTP_HANDLE_CLOSED;
	}

	static int GetSftpHandleState( std::string handle )
	{
		ktrace_in();
		ktrace("KSftp::GetSftpHandleState( )");

		int i = ParseSftpHandle(handle);
		if( i == 0 ) return SftpHandle::KSFTP_HANDLE_CLOSED;

		return( SftpHandles(i).state );
	}
	static std::string GetSftpHandleFile( std::string handle )
	{
		ktrace_in();
		ktrace("KSftp::GetSftpHandleFile( )");

		int i = ParseSftpHandle(handle);
		if( i == 0 ) return SftpHandle::KSFTP_HANDLE_CLOSED;

		return( SftpHandles(i).file );
	}
	static void SetSftpHandleState( std::string handle, int state )
	{
		ktrace_in();
		ktrace("KSftp::SetSftpHandleState( )");

		int i = ParseSftpHandle(handle);
		if( i == 0 ) return;

		SftpHandles(i).state = state;
	}

		
	/*==============================================================================
	 * sftp packets
	 *=============================================================================*/
	class c_SSH_FXP_UNSUPPORTED
	{
	private:
		uint32 length;
		const static byte type = 0;
		uint32 request_id;

		bool FromBuffer( const std::string & buff )
		{
			ktrace_in( );
			ktrace( "KSftp::c_SSH_FXP_UNSUPPORTED( )" );

			unsigned l = 0;
			bool ret = false;

			this->length = b2uint32( buff, l, ret );
			if( !ret ) return false;

			byte type = b2byte( buff, l, ret );
			USE(type);
			if( !ret ) return false;

			this->request_id = b2uint32( buff, l, ret );
			if( !ret ) return false;

			return true;
		}


	public:
		std::string Process( const std::string & packet )
		{
			bool ret = false;

			ret = this->FromBuffer( packet );
			if( !ret ) return false;

			s_SSH_FXP_STATUS s;
			s.request_id = this->request_id;
			s.error_code = SSH_FX_OP_UNSUPPORTED;

			std::string buff_out;
			s.ToBuffer( buff_out );

			trace_packet( "server", buff_out );
			return buff_out;
		}
	};

private:
	class c_SSH_FXP_FSETSTAT
	{
	private:
		uint32 length;
		uint32 request_id;

		bool FromBuffer( const std::string & buff )
		{
			unsigned int l = 0;
			bool ret = false;

			this->length = b2uint32( buff, l, ret );
			if( !ret ) return false;

			byte type = b2byte( buff, l, ret );
			USE(type);
			if( !ret ) return false;

			this->request_id = b2uint32( buff, l, ret );
			if( !ret ) return false;

			return true;
		}
	public:
		std::string Process( const std::string & packet )
		{
			bool ret = false;

			ret = this->FromBuffer( packet );
			if( !ret ) return "";

			std::string buff_out;
			// TODO: 
			s_SSH_FXP_STATUS s1;
			s1.request_id = this->request_id;
			s1.error_code = SSH_FX_OK;

			s1.ToBuffer( buff_out );

			trace_packet( "server", buff_out );
			return buff_out;
		}
	};
private:
	// SSH_FXP_LSTAT
	// SSH_FXP_FSTAT
	class c_SSH_FXP_STAT
	{
	private:
		uint32 length;
		const static byte type1 = SSH_FXP_STAT;
		const static byte type2 = SSH_FXP_LSTAT;
		const static byte type3 = SSH_FXP_FSTAT;
		uint32 request_id;
		std::string path;
		std::string handle;
		byte type;

		bool FromBuffer( const std::string & buff )
		{
			unsigned int l = 0;
			bool ret = false;

			this->length = b2uint32( buff, l, ret );
			if( !ret ) return false;

			this->type = b2byte( buff, l, ret );
			if( this->type != this->type1 
			 && this->type != this->type2 
			 && this->type != this->type3 ) return false;
			if( !ret ) return false;

			this->request_id = b2uint32( buff, l, ret );
			if( !ret ) return false;

			this->path = b2string( buff, l, ret );
			this->handle = this->path;
			if( !ret ) return false;

			return true;
		}
	public:
		std::string Process( const std::string & packet )
		{
			bool ret = false;

			ret = this->FromBuffer( packet );
			if( !ret ) return "";

			if( this->type == SSH_FXP_FSTAT )
			{
				/*
				SSH_FXP_FSTAT differs from the others in that it returns status
				information for an open file (identified by the file handle).  Its
				format is as follows:

				  uint32     id
				  string     handle
				*/

				this->path = GetSftpHandleFile(this->handle);
			}

			std::string buff_out;

			s_SSH_FXP_ATTRS s;
			s.request_id = this->request_id;
			if( !exist_safe(this->path) )
			{
				// file does not exists
				s_SSH_FXP_STATUS s1;
				s1.request_id = this->request_id;
				s1.error_code = SSH_FX_NO_SUCH_FILE;

				s1.ToBuffer( buff_out );
			}
			else
			{
				if( !attrs_safe(this->path, s.attrs) )
				{
					// we have an error retreiving attr
					s_SSH_FXP_STATUS s1;
					s1.request_id = this->request_id;
					s1.error_code = SSH_FX_FAILURE;

					s1.ToBuffer( buff_out );
				}
				else
				{
					// we are ok
					s.ToBuffer( buff_out );
				}
			}
			trace_packet( "server", buff_out );
			return buff_out;
		}
	};

	class c_SSH_FXP_REMOVE
	{
	private:
		uint32 length;
		const static byte type = SSH_FXP_REMOVE;
		uint32 request_id;
		std::string filename;

		bool FromBuffer( const std::string & buff )
		{
			ktrace_in( );
			ktrace( "KSftp::c_SSH_FXP_REMOVE( )" );

			unsigned l = 0;
			bool ret = false;

			this->length = b2uint32( buff, l, ret );
			if( !ret ) return false;

			byte type = b2byte( buff, l, ret );
			if( type != this->type ) return false;
			if( !ret ) return false;

			this->request_id = b2uint32( buff, l, ret );
			if( !ret ) return false;

			this->filename = b2string( buff, l, ret );
			if( !ret ) return false;

			return true;
		}
	public:
		std::string Process( const std::string & packet )
		{
			ktrace_in( );
			ktrace( "c_SSH_FXP_REMOVE::Process" );

			bool ret = false;

			ret = this->FromBuffer( packet );
			if( !ret ) return "";

			klog( "sftp::del " << this->filename );

			std::string buff_out;

			if( !del_safe( this->filename ) )
			{
				s_SSH_FXP_STATUS s;
				s.request_id = this->request_id;
				s.error_code = SSH_FX_FAILURE;

				s.ToBuffer( buff_out );
			}
			else
			{
				s_SSH_FXP_STATUS s;
				s.request_id = this->request_id;
				s.error_code = SSH_FX_OK;

				s.ToBuffer( buff_out );
			}
			trace_packet( "server", buff_out );
			return buff_out;
		}
	};


    class c_SSH_FXP_RENAME
	{
	private:
		uint32 length;
		const static byte type = SSH_FXP_RENAME;
		uint32 request_id;
		std::string oldpath;
		std::string newpath;

		bool FromBuffer( const std::string & buff )
		{
			ktrace_in( );
			ktrace( "KSftp::c_SSH_FXP_RENAME( )" );

			unsigned l = 0;
			bool ret = false;

			this->length = b2uint32( buff, l, ret );
			if( !ret ) return false;

			byte type = b2byte( buff, l, ret );
			if( type != this->type ) return false;
			if( !ret ) return false;

			this->request_id = b2uint32( buff, l, ret );
			if( !ret ) return false;

			this->oldpath = b2string( buff, l, ret );
			if( !ret ) return false;

			this->newpath = b2string( buff, l, ret );
			if( !ret ) return false;

			return true;
		}
	public:
		std::string Process( const std::string & packet )
		{
			ktrace_in( );
			ktrace( "c_SSH_FXP_RENAME::Process( )" );

			bool ret = false;

			ret = this->FromBuffer( packet );
			if( !ret ) return "";

			klog( "sftp::ren " << this->oldpath << " " << this->newpath );

			std::string buff_out;

			if( !ren_safe( this->oldpath, this->newpath ) )
			{
				s_SSH_FXP_STATUS s;
				s.request_id = this->request_id;
				s.error_code = SSH_FX_FAILURE;

				s.ToBuffer( buff_out );
			}
			else
			{
				s_SSH_FXP_STATUS s;
				s.request_id = this->request_id;
				s.error_code = SSH_FX_OK;

				s.ToBuffer( buff_out );
			}
			trace_packet( "server", buff_out );
			return buff_out;
		}
	};

	class c_SSH_FXP_INIT
	{
	private:
		uint32 length;
		const static byte type = SSH_FXP_INIT;
		uint32 version;
		bool FromBuffer( const std::string & buff )
		{
			ktrace_in( );
			ktrace( "KSftp::c_SSH_FXP_INIT( )" );

			unsigned l = 0;
			bool ret = false;

			this->length = b2uint32( buff, l, ret );
			if( !ret ) return false;

			byte type = b2byte( buff, l, ret );
			if( type != this->type ) return false;
			if( !ret ) return false;

			this->version = b2uint32( buff, l, ret );
			if( !ret ) return false;

			KSftp::client_version = this->version;

			return true;
		}

	public:
		std::string Process( const std::string & packet )
		{
			bool ret = false;
			
			ret = this->FromBuffer( packet );
			if( !ret ) return "";

			s_SSH_FXP_VERSION s;
			s.version = 3;

			std::string buff_out;

			s.ToBuffer( buff_out );

			trace_packet( "server", buff_out );
			return buff_out;
		}
	};

	class c_SSH_FXP_READ
	{
	private:
		uint32 length;
		const static byte type = SSH_FXP_READ;
		uint32 request_id;
		std::string handle;
		uint64 offset;
		uint32 len;

		bool FromBuffer( const std::string & buff )
		{
			ktrace_in( );
			ktrace( "KSftp::c_SSH_FXP_READ( )" );

			unsigned l = 0;
			bool ret = false;

			this->length = b2uint32( buff, l, ret );
			if( !ret ) return false;

			byte type = b2byte( buff, l, ret );
			if( type != this->type ) return false;
			if( !ret ) return false;

			this->request_id = b2uint32( buff, l, ret );
			if( !ret ) return false;

			this->handle = b2string( buff, l, ret );
			if( !ret ) return false;

			this->offset = b2uint64( buff, l, ret );
			if( !ret ) return false;
			
			this->len = b2uint32( buff, l, ret );
			if( !ret ) return false;

			return true;
		}
	public:
		std::string Process( const std::string packet )
		{
			ktrace_in( );
			ktrace( "c_SSH_FXP_READ::Process" );

			bool ret = false;

			ret = this->FromBuffer( packet );
			if( !ret ) return "";

			std::string buff_out;

			if( GetSftpHandleState( this->handle ) != SftpHandle::KSFTP_HANDLE_OPENED )
			{
				s_SSH_FXP_STATUS s;
				s.request_id = this->request_id;
				s.error_code = SSH_FX_EOF;

				s.ToBuffer( buff_out );
			}
			else
			{
				std::string data;

				if( this->offset == 0 ) klog( "sftp::read " << GetSftpHandleFile(this->handle) );

				if( !read_safe( GetSftpHandleFile(this->handle), this->offset, this->len, data ) )
				{
					s_SSH_FXP_STATUS s;
					s.request_id = this->request_id;
					s.error_code = SSH_FX_FAILURE;

					s.ToBuffer( buff_out );
				}
				else
				{
					s_SSH_FXP_DATA s;
					s.request_id = this->request_id;
					s.data = data;

					s.ToBuffer( buff_out );
					if( data.length() != this->len ) SetSftpHandleState( this->handle, SftpHandle::KSFTP_HANDLE_CLOSED );
				}
			}
			trace_packet( "server", buff_out );

			return buff_out;
		}
	};

	class c_SSH_FXP_OPEN
	{
	private:
		uint32 length;
		const static byte type = SSH_FXP_OPEN;
		uint32 request_id;
		std::string filename;
   		uint32 pflags;
   		ATTRS attrs;

		bool FromBuffer( const std::string & buff )
		{
			ktrace_in( );
			ktrace( "KSftp::c_SSH_FXP_OPEN( )" );

			unsigned l = 0;
			bool ret = false;

			this->length = b2uint32( buff, l, ret );
			if( !ret ) return false;

			byte type = b2byte( buff, l, ret );
			if( type != this->type ) return false;
			if( !ret ) return false;

			this->request_id = b2uint32( buff, l, ret );
			if( !ret ) return false;

			this->filename = b2string( buff, l, ret );
			if( !ret ) return false;

			this->pflags = b2uint32( buff, l, ret );
			if( !ret ) return false;
			
			// skip
			//this->attrs

			return true;
		}
	public:
		std::string Process( const std::string & packet )
		{
			bool ret = false;

			ret = this->FromBuffer( packet );
			if( !ret ) return "";

			s_SSH_FXP_HANDLE s;
			s.request_id = this->request_id;
			s.handle = OpenSftpHandle( this->filename );

			std::string buff_out;

			if( !exist_safe( this->filename ) )
			{
				// try to create the file
				if( !create_safe( this->filename ) )
				{
					s_SSH_FXP_STATUS s;
					s.request_id = this->request_id;
					s.error_code = SSH_FX_FAILURE;

					s.ToBuffer( buff_out );
				}
			}
			if( buff_out.length() == 0 ) s.ToBuffer( buff_out );

			trace_packet( "server", buff_out );
			return buff_out;
		}
	};

	class c_SSH_FXP_WRITE
	{
	private:
		uint32 length;
		const static byte type = SSH_FXP_WRITE;
		uint32 request_id;
		std::string handle;
   		uint64 offset;
   		std::string data;

		bool FromBuffer( const std::string & buff )
		{
			ktrace_in( );
			ktrace( "KSftp::c_SSH_FXP_WRITE( )" );

			unsigned l = 0;
			bool ret = false;

			this->length = b2uint32( buff, l, ret );
			if( !ret ) return false;

			byte type = b2byte( buff, l, ret );
			if( type != this->type ) return false;
			if( !ret ) return false;

			this->request_id = b2uint32( buff, l, ret );
			if( !ret ) return false;

			this->handle = b2string( buff, l, ret );
			if( !ret ) return false;

			this->offset = b2uint64( buff, l, ret );
			if( !ret ) return false;

			this->data = b2string( buff, l, ret );
			if( !ret ) return false;

			return true;
		}
	public:
		std::string Process( const std::string & packet )
		{
			ktrace_in( );
			ktrace( "c_SSH_FXP_WRITE::Process" );

			bool ret = false;

			ret = this->FromBuffer( packet );
			if( !ret ) return "";

			if( this->offset == 0 ) klog( "sftp::write " << GetSftpHandleFile(this->handle) );

			std::string buff_out;

			if( write_safe( GetSftpHandleFile(this->handle), this->offset, this->data ) )
			{
				s_SSH_FXP_STATUS s;
				s.request_id = this->request_id;
				s.error_code = SSH_FX_OK;

				s.ToBuffer( buff_out );
			}
			else
			{
				s_SSH_FXP_STATUS s;
				s.request_id = this->request_id;
				s.error_code = SSH_FX_FAILURE;

				s.ToBuffer( buff_out );
			}
			trace_packet( "server", buff_out );

			return buff_out;
		}
	};

	class c_SSH_FXP_REALPATH
	{
	private:
		uint32 length;
		const static byte type = SSH_FXP_REALPATH;
		uint32 request_id;
		std::string path;

		bool FromBuffer( const std::string & buff )
		{
			ktrace_in( );
			ktrace( "KSftp::c_SSH_FXP_REALPATH( )" );

			unsigned l = 0;
			bool ret = false;

			this->length = b2uint32( buff, l, ret );
			if( !ret ) return false;

			byte type = b2byte( buff, l, ret );
			if( type != this->type ) return false;
			if( !ret ) return false;

			this->request_id = b2uint32( buff, l, ret );
			if( !ret ) return false;

			this->path = b2string( buff, l, ret );
			if( !ret ) return false;

			return true;
		}

	public:
		std::string Process( const std::string & pack )
		{
			bool ret = false;

			ret = this->FromBuffer( pack );
			if( !ret ) return "";

			std::string buff_out;
			bool isroot = false;

			if( !cd_safe(this->path, isroot) )
			{
				s_SSH_FXP_STATUS s;
				s.request_id = this->request_id;
				s.error_code = SSH_FX_FAILURE;

				s.ToBuffer( buff_out );
			}
			else
			{

				char buff1[1002] = {0};
				int len = GetCurrentDirectoryA( 1000, buff1 );
				buff1[len] = 0;

				std::string buff = translate_to_nix(buff1);

				s_SSH_FXP_NAME s;
				s.request_id = this->request_id;

				FILE_ATTRS fa;

				if( isroot )
				{
					fa.filename = fix_backslashes( "/" );
					fa.longname = ffd2string( "/" );
					attrs_safe( buff1, fa.attrs );
				}
				else
				{
					fa.filename = fix_backslashes( buff );
					fa.longname = ffd2string( buff1 );
					attrs_safe( buff1, fa.attrs );
				}

				s.names.push_back( fa );

				s.ToBuffer( buff_out );
			}

			trace_packet( "server", buff_out );
			return buff_out;
		}
	};

	class c_SSH_FXP_MKDIR
	{
	private:
		uint32 length;
		const static byte type = SSH_FXP_MKDIR;
		uint32 request_id;
		std::string path;
		ATTRS attrs;

		bool FromBuffer( const std::string & buff )
		{
			ktrace_in( );
			ktrace( "KSftp::c_SSH_FXP_MKDIR( )" );

			unsigned l = 0;
			bool ret = false;

			this->length = b2uint32( buff, l, ret );
			if( !ret ) return false;

			byte type = b2byte( buff, l, ret );
			if( type != this->type ) return false;
			if( !ret ) return false;

			this->request_id = b2uint32( buff, l, ret );
			if( !ret ) return false;

			this->path = b2string( buff, l, ret );
			if( !ret ) return false;

			// not used
			// this->attrs =
			return true;
		}
	public:
		std::string Process( const std::string & packet )
		{
			ktrace_in( );
			ktrace( "c_SSH_FXP_MKDIR::Process" );

			bool ret = false;

			ret = this->FromBuffer( packet );
			if( !ret ) return "";

			klog( "sftp::md " << this->path );

			std::string buff_out;

			if( !md_safe( this->path ) )
			{
				s_SSH_FXP_STATUS s;
				s.request_id = this->request_id;
				s.error_code = SSH_FX_FAILURE;

				s.ToBuffer( buff_out );
			}
			else
			{
				s_SSH_FXP_STATUS s;
				s.request_id = this->request_id;
				s.error_code = SSH_FX_OK;

				s.ToBuffer( buff_out );
			}

			trace_packet( "server", buff_out );
			return buff_out;
		}

	};

	class c_SSH_FXP_RMDIR
	{
	private:
		uint32 length;
		const static byte type = SSH_FXP_RMDIR;
		uint32 request_id;
		std::string path;

		bool FromBuffer( const std::string & buff )
		{
			ktrace_in( );
			ktrace( "KSftp::c_SSH_FXP_RMDIR( )" );

			unsigned l = 0;
			bool ret = false;

			this->length = b2uint32( buff, l, ret );
			if( !ret ) return false;

			byte type = b2byte( buff, l, ret );
			if( type != this->type ) return false;
			if( !ret ) return false;

			this->request_id = b2uint32( buff, l, ret );
			if( !ret ) return false;

			this->path = b2string( buff, l, ret );
			if( !ret ) return false;

			// not used
			// this->attrs =
			return true;
		}
	public:
		std::string Process( const std::string & packet )
		{
			ktrace_in( );
			ktrace( "c_SSH_FXP_RMDIR::Process" );

			bool ret = false;

			ret = this->FromBuffer( packet );
			if( !ret ) return "";

			klog( "sftp::rd " << this->path );

			std::string buff_out;

			if( !rd_safe( this->path ) )
			{
				s_SSH_FXP_STATUS s;
				s.request_id = this->request_id;
				s.error_code = SSH_FX_FAILURE;

				s.ToBuffer( buff_out );
			}
			else
			{
				s_SSH_FXP_STATUS s;
				s.request_id = this->request_id;
				s.error_code = SSH_FX_OK;

				s.ToBuffer( buff_out );
			}

			trace_packet( "server", buff_out );
			return buff_out;
		}
	};

	class c_SSH_FXP_CLOSE
	{
	private:
		uint32 length;
		const static byte type = SSH_FXP_CLOSE;
		uint32 request_id;
		std::string handle;

		bool FromBuffer( const std::string & buff )
		{
			ktrace_in( );
			ktrace( "KSftp::c_SSH_FXP_CLOSE( )" );

			unsigned l = 0;
			bool ret = false;

			this->length = b2uint32( buff, l, ret );
			if( !ret ) return false;

			byte type = b2byte( buff, l, ret );
			if( type != this->type ) return false;
			if( !ret ) return false;

			this->request_id = b2uint32( buff, l, ret );
			if( !ret ) return false;

			this->handle = b2string( buff, l, ret );
			if( !ret ) return false;

			return true;
		}

	public:
		std::string Process( const std::string packet )
		{
			bool ret = false;

			ret = this->FromBuffer( packet );
			if( !ret ) return "";

			SetSftpHandleState( this->handle, SftpHandle::KSFTP_HANDLE_CLOSED );

			s_SSH_FXP_STATUS s;
			s.request_id = this->request_id;
			s.error_code = SSH_FX_OK;

			std::string buff_out;
			s.ToBuffer( buff_out );

			trace_packet( "server", buff_out );
			return buff_out;

		}
	};

	class c_SSH_FXP_OPENDIR
	{
	private:
		uint32 length;
		const static byte type = SSH_FXP_OPENDIR;
   		uint32 request_id;
		std::string path;

		bool FromBuffer( const std::string & buff )
		{
			ktrace_in( );
			ktrace( "KSftp::c_SSH_FXP_OPENDIR( )" );

			unsigned l = 0;
			bool ret = false;

			this->length = b2uint32( buff, l, ret );
			if( !ret ) return false;

			byte type = b2byte( buff, l, ret );
			if( type != this->type ) return false;
			if( !ret ) return false;

			this->request_id = b2uint32( buff, l, ret );
			if( !ret ) return false;

			this->path = b2string( buff, l, ret );
			if( !ret ) return false;

			return true;
		}

	public:
		std::string Process( const std::string & packet )
		{
			bool ret = false;

			ret = this->FromBuffer( packet );
			if( !ret ) return "";

			std::string buff_out;
			bool isroot = false;

			if( !cd_safe( this->path, isroot ) )
			{
				s_SSH_FXP_STATUS s;
				s.request_id = this->request_id;
				s.error_code = SSH_FX_FAILURE;

				s.ToBuffer( buff_out );
			}
			else
			{
				s_SSH_FXP_HANDLE s;
				s.request_id = this->request_id;
				s.handle = OpenSftpHandle( this->path );

				s.ToBuffer( buff_out );
			}
			trace_packet( "server", buff_out );
			return buff_out;
		}
	};

	class c_SSH_FXP_READDIR
	{
	private:
		uint32 length;
		const static byte type = SSH_FXP_READDIR;
   		uint32 request_id;
		std::string handle;

		bool FromBuffer( const std::string & buff )
		{
			ktrace_in( );
			ktrace( "KSftp::c_SSH_FXP_READDIR( )" );

			unsigned l = 0;
			bool ret = false;

			this->length = b2uint32( buff, l, ret );
			if( !ret ) return false;

			byte type = b2byte( buff, l, ret );
			if( type != this->type ) return false;
			if( !ret ) return false;

			this->request_id = b2uint32( buff, l, ret );
			if( !ret ) return false;

			this->handle = b2string( buff, l, ret );
			if( !ret ) return false;

			return true;
		}
	public:
		std::string Process( const std::string packet )
		{
			bool ret = false;

			ret = this->FromBuffer( packet );
			if( !ret ) return "";

			
			std::string buff_out;
			bool isroot = false;

			if( !cd_safe( GetSftpHandleFile( this->handle ), isroot ) )
			{
				s_SSH_FXP_STATUS s;
				s.request_id = this->request_id;
				s.error_code = SSH_FX_FAILURE;

				s.ToBuffer( buff_out );
			}
			else
			{
				if( GetSftpHandleState( this->handle ) == SftpHandle::KSFTP_HANDLE_OPENED )
				{
					s_SSH_FXP_NAME s;
					s.request_id = this->request_id;
					
					if( isroot )
					{
						s.names = list_drives( );
					}
					else
					{
						char path[1002];
						int len = GetCurrentDirectoryA( 1000, path );
						if( len == 0 ) return "";

						s.names = ls_safe( path );
					}
					s.ToBuffer( buff_out );

					SetSftpHandleState( this->handle, SftpHandle::KSFTP_HANDLE_EOF );
				}
				else
				{
					s_SSH_FXP_STATUS s;
					s.request_id = this->request_id;
					s.error_code = SSH_FX_EOF;

					s.ToBuffer( buff_out );
				}
			}
			trace_packet( "server", buff_out );
			return buff_out;
		}
	};



	struct s_SSH_FXP_VERSION
	{
		const static uint32 length = 5;
		const static byte type = SSH_FXP_VERSION;
		uint32 version;
		// no extension pairs
		//extension-pair extensions[0..n]

		void ToBuffer( std::string & buff )
		{
			ktrace_in( );
			ktrace( "KSftp::s_SSH_FXP_VERSION( )" );

			uint322b( this->length, buff );

			byte2b( this->type, buff );

			uint322b( this->version, buff );
		}
	};

	struct s_SSH_FXP_HANDLE
	{
		uint32 length;
		const static byte type = SSH_FXP_HANDLE;
   		uint32 request_id;
		std::string handle;

		void ToBuffer(  std::string & buff )
		{
			ktrace_in( );
			ktrace( "KSftp::s_SSH_FXP_HANDLE( )" );

			this->length = 
				  1 //type
				+ 4 //request_id
				+ 4 + ( unsigned int )handle.length();

			uint322b( this->length, buff );

			byte2b( this->type, buff );

			uint322b( this->request_id, buff );

			string2b( this->handle, buff );
		}
	};


	struct s_SSH_FXP_NAME
	{
		uint32 length;
		const static byte type = SSH_FXP_NAME;
		uint32 request_id;
		uint32 count;
		std::vector<FILE_ATTRS> names;

		void ToBuffer(  std::string & buff )
		{
			ktrace_in( );
			ktrace( "KSftp::s_SSH_FXP_NAME( )" );

			this->count = ( unsigned int )this->names.size();
			
			int names_size = 0;
			for( unsigned i = 0; i < this->count; i++ )
			{
				names_size = names_size
					+ 4 // string len
					+ ( unsigned int )this->names[ i ].filename.length()
					+ 4 // string len
					+ ( unsigned int )this->names[ i ].longname.length()
					+ this->names[ i ].attrs.Length();
					
			}

			this->length = 
				  1	//type
				+ 4 //request_id
				+ 4 //count
				+ names_size;


			uint322b( this->length, buff );

			byte2b( this->type, buff );

			uint322b( this->request_id, buff );

			uint322b( this->count, buff );

			// names
			for( unsigned i = 0; i < this->count; i++ )
			{
				std::string attrs_buff;
				this->names[ i ].attrs.ToBuffer( attrs_buff );

				string2b( this->names[ i ].filename, buff );
				string2b( this->names[ i ].longname, buff );
				raw2b( attrs_buff, buff );
			}
		}
	};

	struct s_SSH_FXP_STATUS
	{
		uint32 length;
		const static byte type = SSH_FXP_STATUS;
		uint32 request_id;
		uint32 error_code;
		std::string error_message;
		std::string language;

		void ToBuffer(  std::string & buff )
		{
			ktrace_in( );
			ktrace( "KSftp::s_SSH_FXP_STATUS( )" );

			this->length = 
				  1 //type
				+ 4 //request_id
				+ 4;//error_code
			if( KSftp::client_version >= 3 )
			{
				this->length +=
				+ 4 + ( unsigned int )error_message.length()
				+ 4 + ( unsigned int )language.length();
			}

			uint322b( this->length, buff );

			byte2b( this->type, buff );

			uint322b( this->request_id, buff );

			uint322b( this->error_code, buff );

			if( KSftp::client_version >= 3 )
			{
				string2b( this->error_message, buff );

				string2b( this->language, buff );
			}
		}
	};

	struct s_SSH_FXP_DATA
	{
		uint32 length;
		const static byte type = SSH_FXP_DATA;
		uint32 request_id;
		std::string data;

		void ToBuffer(  std::string & buff )
		{
			ktrace_in( );
			ktrace( "KSftp::s_SSH_FXP_DATA( )" );

			this->length = 
				  1 //type
				+ 4 //request_id
				+ 4 + ( unsigned int )data.length();

			uint322b( this->length, buff );

			byte2b( this->type, buff );

			uint322b( this->request_id, buff );

			string2b( this->data, buff );
		}
	};

	struct s_SSH_FXP_ATTRS
	{
		uint32 length;
		const static byte type = SSH_FXP_ATTRS;
		uint32 request_id;
		ATTRS attrs;
		void ToBuffer(  std::string & buff )
		{
			this->length = 
				  1 //type
				+ 4 //request_id
				+ attrs.Length();

			uint322b( this->length, buff );

			byte2b( this->type, buff );

			uint322b( this->request_id, buff );

			std::string attrs_data;
			attrs.ToBuffer(attrs_data);

			raw2b(attrs_data, buff);
		}
	};

private:
	/*==============================================================================
	 * process packet
	 *=============================================================================*/
	static std::string ProcessPacket( byte type, uint32 request_id, const std::string & buff_in )
	{
		ktrace_in( );
		ktrace( "KSftp::ProcessPacket( " << type << ", " << request_id << " )" );
//klog( "KSftp::ProcessPacket( " << (int)type << ", " << request_id << " )" );
		std::string buff_out;

		trace_packet( "client", buff_in );

		switch( type )
		{
		case SSH_FXP_INIT:
			{
			c_SSH_FXP_INIT c;
			return c.Process( buff_in );
			}

		case SSH_FXP_REALPATH:
			{
			c_SSH_FXP_REALPATH c;
			return c.Process( buff_in );
			}

		case SSH_FXP_OPENDIR:
			{
			c_SSH_FXP_OPENDIR c;
			return c.Process( buff_in );
			}

		case SSH_FXP_READDIR:
			{
			c_SSH_FXP_READDIR c;
			return c.Process( buff_in );
			}

		case SSH_FXP_MKDIR:
			{
			c_SSH_FXP_MKDIR c;
			return c.Process( buff_in );
			}

		case SSH_FXP_RMDIR:
			{
			c_SSH_FXP_RMDIR c;
			return c.Process( buff_in );
			}

		case SSH_FXP_CLOSE:
			{
			c_SSH_FXP_CLOSE c;
			return c.Process( buff_in );
			}

		case SSH_FXP_OPEN:
			{
			c_SSH_FXP_OPEN c;
			return c.Process( buff_in );
			}

		case SSH_FXP_WRITE:
			{
			c_SSH_FXP_WRITE c;
			return c.Process( buff_in );
			}

		case SSH_FXP_READ:
			{
			c_SSH_FXP_READ c;
			return c.Process( buff_in );
			}

		case SSH_FXP_REMOVE:
			{
			c_SSH_FXP_REMOVE c;
			return c.Process( buff_in );
			}

		case SSH_FXP_RENAME:
			{
			c_SSH_FXP_RENAME c;
			return c.Process( buff_in );
			}

		case SSH_FXP_LSTAT:
		case SSH_FXP_FSTAT:
		case SSH_FXP_STAT:
			{
			c_SSH_FXP_STAT c;
			return c.Process( buff_in );
			}
		case SSH_FXP_FSETSTAT:
			{
			c_SSH_FXP_FSETSTAT c;
			return c.Process( buff_in );
			}
		default:
			{
			c_SSH_FXP_UNSUPPORTED c;
			return c.Process( buff_in );
			}
		}
	}

public:
	/*==============================================================================
	 * process
	 *=============================================================================*/
	static int Process( const std::string & buff, std::string & buff_out )
	{
		ktrace_in( );
		ktrace( "KSftp::Process( )" );

		unsigned offset = 0;
		bool ret = false;

		buff_out = "";

		while( true )
		{
			unsigned l = offset;

			uint32 length = b2uint32( buff, l, ret ) + 4;
			if( !ret ) break; 

			if( length + offset > buff.length() ) 
			{
				break;
			}

			byte type = b2byte( buff, l, ret );
			if( !ret )
			{
				kerror( "can't read sftp packet type" );
				return -1;
			}

			uint32 request_id = 0;
			if( type != SSH_FXP_INIT ) request_id = b2uint32( buff, l, ret );
			if( !ret )
			{
				kerror( "can't read sftp packet [ " << (int)type << " ] id" );
				return -1;
			}

			std::string pack = buff.substr( offset, length );

			std::string tmp = ProcessPacket( type, request_id, pack );
			if( tmp == "" ) 
			{
				kerror( "can't process sftp packet [ " << (int)type << " ]" );
				return -1;
			}

			buff_out += tmp;

			offset += length;

			// we want to chunk the data
			if( buff_out.length() > 2000 ) break;

			if( offset >= buff.length() ) break;
		}

		return offset;
	}
};

uint32 KSftp::client_version = 0;

#undef byte
#undef uint32
#undef uint64
