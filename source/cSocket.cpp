
#include "Globals.h"  // NOTE: MSVC stupidness requires this to be the same across all modules

#include "cSocket.h"

#ifndef _WIN32
	#include <netdb.h>
	#include <unistd.h>
	// #include <sys/socket.h>
	// #include <netinet/in.h>
	#include <arpa/inet.h>		//inet_ntoa()
#else
	#define socklen_t int
#endif





cSocket::cSocket(xSocket a_Socket)
	: m_Socket(a_Socket)
{
}





cSocket::~cSocket()
{
}





cSocket::operator const cSocket::xSocket() const
{
	return m_Socket;
}





cSocket::xSocket cSocket::GetSocket() const
{
	return m_Socket;
}





bool cSocket::IsValid(void) const
{
	#ifdef _WIN32
	return (m_Socket != INVALID_SOCKET);
	#else  // _WIN32
	return (m_Socket >= 0);
	#endif  // else _WIN32
}





void cSocket::CloseSocket()
{
	#ifdef _WIN32
	
	closesocket(m_Socket);
	
	#else  // _WIN32
	
	if (shutdown(m_Socket, SHUT_RDWR) != 0)//SD_BOTH);
	{
		LOGWARN("Error on shutting down socket (%s)", m_IPString.c_str());
	}
	if (close(m_Socket) != 0)
	{
		LOGWARN("Error closing socket (%s)", m_IPString.c_str());
	}
	
	#endif  // else _WIN32

	// Invalidate the socket so that this object can be re-used for another connection
	m_Socket = INVALID_SOCKET;
}





AString cSocket::GetErrorString( int a_ErrNo )
{
	char buffer[ 1024 ];
	AString Out;

	#ifdef _WIN32

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, a_ErrNo, 0, buffer, ARRAYCOUNT(buffer), NULL);
	Printf(Out, "%d: %s", a_ErrNo, buffer);
	return Out;
	
	#else  // _WIN32
	
	// According to http://linux.die.net/man/3/strerror_r there are two versions of strerror_r():
	
	#if (((_POSIX_C_SOURCE >= 200112L) || (_XOPEN_SOURCE >= 600)) && ! _GNU_SOURCE)  // XSI version of strerror_r():
	
	int res = strerror_r( errno, buffer, ARRAYCOUNT(buffer) );
	if( res == 0 )
	{
		Printf(Out, "%d: %s", a_ErrNo, buffer);
		return Out;
	}
	
	#else  // GNU version of strerror_r()
	
	char * res = strerror_r( errno, buffer, ARRAYCOUNT(buffer) );
	if( res != NULL )
	{
		Printf(Out, "%d: %s", a_ErrNo, res);
		return Out;
	}
	
	#endif  // strerror_r() version
	
	else
	{
		Printf(Out, "Error %d while getting error string for error #%d!", errno, a_ErrNo);
		return Out;
	}
	
	#endif  // else _WIN32
}




int cSocket::GetLastError()
{
#ifdef _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}





int cSocket::SetReuseAddress()
{
#ifdef _WIN32
	char yes = 1;
#else
	int yes = 1;
#endif
	return setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
}





int cSocket::WSAStartup()
{
#ifdef _WIN32
	WSADATA wsaData;
	memset(&wsaData, 0, sizeof(wsaData));
	return ::WSAStartup(MAKEWORD(2, 2),&wsaData);
#else
	return 0;
#endif
}





cSocket cSocket::CreateSocket()
{
	return socket(AF_INET,SOCK_STREAM,0);
}





int cSocket::Bind(SockAddr_In& a_Address)
{
	sockaddr_in local;

	if (a_Address.Family == ADDRESS_FAMILY_INTERNET)
		local.sin_family = AF_INET;

	if (a_Address.Address == INTERNET_ADDRESS_ANY)
		local.sin_addr.s_addr = INADDR_ANY;

	local.sin_port=htons((u_short)a_Address.Port);

	return bind(m_Socket, (sockaddr*)&local, sizeof(local));
}





int cSocket::Listen(int a_Backlog)
{
	return listen(m_Socket, a_Backlog);
}





cSocket cSocket::Accept()
{
	sockaddr_in from;
	socklen_t fromlen=sizeof(from);

	cSocket SClient = accept(m_Socket, (sockaddr*)&from, &fromlen);

	if (from.sin_addr.s_addr && SClient.IsValid())	// Get IP in string form
	{
		SClient.m_IPString = inet_ntoa(from.sin_addr);
		//LOG("cSocket::Accept() %s", SClient.m_IPString);
	}

	return SClient;
}





int cSocket::Receive(char* a_Buffer, unsigned int a_Length, unsigned int a_Flags)
{
	return recv(m_Socket, a_Buffer, a_Length, a_Flags);
}




