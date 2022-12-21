#include "Components.h"
#include <time.h>

#include "SocketClass3.h"
#include "util/Log.h"

#ifdef WINDOWS_PLATFORM
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#define CLOSE_SOCKET   closesocket
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h> //for close()
#include <errno.h>
#define CLOSE_SOCKET   close
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

SocketClass :: SocketClass()
{
	Clear();
}

SocketClass :: ~SocketClass()
{
	ShutdownServer();
}

void SocketClass :: Clear(void)
{
	disconnecting = false;
	port = 0;
	ListenSocket = Invalid_Socket;
	ClientSocket = Invalid_Socket;
	memset(&acceptData, 0, sizeof(acceptData));
	FD_ZERO(&readset);
	memset(&seltime, 0, sizeof(seltime));
	memset(debugName, 0, sizeof(debugName));
}

int SocketClass :: CreateSocket(unsigned int port, const std::string &address)
{
	this->port = port;

#ifdef WINDOWS_PLATFORM
	addrinfo hints;
	addrinfo *result;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	if(getaddrinfo(NULL, itoa(port), &hints, &result) != 0)
	{
		LogMessage("getaddrinfo failed for port: %d", port);
		return -1;
	}

	addrinfo *search;
	SOCKET newsocket = -1;
	for(search = result; search != NULL; search = result->ai_next)
	{
		newsocket = socket(search->ai_family, search->ai_socktype, search->ai_protocol);
		if(newsocket != -1)
		{
			int res = bind(newsocket, search->ai_addr, (int)search->ai_addrlen);
			if(res == SOCKET_ERROR)
			{
				closesocket(newsocket);
				newsocket = SOCKET_ERROR;
			}
			else
			{
				break;
			}
		}
	}

	freeaddrinfo(result);
	if(newsocket == -1)
	{
		LogMessage("Unable to create socket for port: %d", port);
		return -1;
	}

	if(listen(newsocket, SOMAXCONN) == SOCKET_ERROR)
	{
		LogMessage("Unable to listen on port: %d", port);
		closesocket(newsocket);
	}

	ListenSocket = newsocket;
	return 0;


#else //#ifdef WINDOWS_PLATFORM

	int socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if(socketfd == Invalid_Socket)
	{
		g_Logs.server->error("Could not create socket.");
		return -1;
	}

	sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);

	// Em - 13/3/2015 - Allow binding to different address (to have multiple servers on same host with no client mod)
	if(address.length() > 0) {
		if(inet_aton(address.c_str(), &serveraddr.sin_addr) < 0) {
			g_Logs.server->info("Binding to all address for port %v because bind address %v is invalid", port, address);
			serveraddr.sin_addr.s_addr = INADDR_ANY;
		}
		else {
			g_Logs.server->info("Bound port %v to %v", port, address);
		}
	}
	else {
		g_Logs.server->info("Binding to all address");
		serveraddr.sin_addr.s_addr = INADDR_ANY;
	}

	int len = sizeof(serveraddr);

	// Em 5/3/2015 - Set SO_REUSEADDR, means server can be restarted and re-use the
	// the ports earlier
	int so_reuseaddr = 1;
	if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof so_reuseaddr) < 0) {
		g_Logs.server->warn("Could not set SO_REUSEADDR on listening socket: %v (%v)", port, strerror(errno));
		CLOSE_SOCKET(socketfd);
		return -1;
	}

	if(bind(socketfd, (sockaddr*)&serveraddr, len) < 0)
	{
		g_Logs.server->error("Could not bind: %v (%v)", port, strerror(errno));
		CLOSE_SOCKET(socketfd);
		return -1;
	}

	if(listen(socketfd, 5) < 0)
	{
		g_Logs.server->error("Could not listen: %v (%v)", port, strerror(errno));
		CLOSE_SOCKET(socketfd);
		return -1;
	}

	ListenSocket = socketfd;
	return 0;

#endif //#ifdef WINDOWS_PLATFORM

	// UNUSED LINUX CODE
	/*
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	addrinfo *result;

	int r = getaddrinfo(NULL, port, &hints, &result);
	if(r != 0)
		return -1;

	if(result == NULL)
		return -1;

	addrinfo *sp;
	int socketfd = -1;
	for(sp = result; sp != NULL; sp = sp->ai_next)
	{
		socketfd = socket(sp->ai_family, sp->ai_socktype, sp->ai_protocol);
		if(socketfd == -1)
			continue;

		if(bind(socketfd, sp->ai_addr, sp->ai_addrlen) == 0)
			break;

		close(socketfd);
	}

	freeaddrinfo(result);
	if(sp == NULL)
	{
		LogMessage("Failed to bind on port: %s", port);
		return -1;
	}

	ListenSocket = socketfd;
	*/
}

int SocketClass :: Accept(void)
{
	//This phase appears to be the same between Linux and Windows

	memset(&acceptData, 0, sizeof(acceptData));
	socklen_t len = sizeof(acceptData);
	int clientfd = accept(ListenSocket, (sockaddr*)&acceptData, &len);
	if(clientfd == -1)
	{
		if(!disconnecting)
			g_Logs.server->error("Failed to accept on port: %v", this->port);
		return -1;
	}
	ClientSocket = clientfd;

	/* Get the address of the route the client connected TO. This is useful
	 * because it allows us to leave SimulatorAddress blank in ServerConfig
	 * for most setups as the router is very likely to be running on the
	 * same host as the simulator
	 */
	socklen_t sa_len;
	struct sockaddr_storage sa;
	sa_len = sizeof(sa);
	if(getsockname(clientfd, (struct sockaddr*)&sa, &sa_len) == -1) {
		g_Logs.server->error("Failed to get destination sockname: %v", this->port);
		return -1;
	}
	memset(destAddr,0,INET6_ADDRSTRLEN);
	int err=getnameinfo((struct sockaddr*)&sa,sa_len,destAddr,sizeof(destAddr),
	    0,0,NI_NUMERICHOST);
	if (err!=0) {
		g_Logs.server->error("Failed to to convert address to string ");
		return -1;
	}

	linger lData = {0};
	lData.l_linger = 5;
	lData.l_onoff = 1;

	int optval = 1;
	int res;
	res = setsockopt(ClientSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
	if(res != 0)
		g_Logs.server->error("setsockopt SO_REUSEADDR error: %v", errno);
	res = setsockopt(ClientSocket, SOL_SOCKET, SO_LINGER, (const char*)&lData, sizeof(lData));
	if(res != 0)
		g_Logs.server->error("setsockopt SO_LINGER error: %v", errno);


	FD_ZERO(&readset);
	FD_SET(ClientSocket, &readset);

	seltime.tv_sec = 5;
	seltime.tv_usec = 0;
	return 0;
}

void SocketClass :: DisconnectClient(void)
{
	if(ClientSocket != Invalid_Socket)
	{
		g_Logs.server->debug("Disconnect socket %v", debugName);
		shutdown(ClientSocket, SHUTDOWN_PARAM);
		CLOSE_SOCKET(ClientSocket);
		ClientSocket = Invalid_Socket;
	}
}

void SocketClass :: ShutdownServer(void)
{
	DisconnectClient();
	if(ListenSocket != Invalid_Socket)
	{
		disconnecting = true;
		shutdown(ListenSocket, SHUTDOWN_PARAM);
		CLOSE_SOCKET(ListenSocket);
		ListenSocket = Invalid_Socket;
	}
}

int SocketClass :: AttemptSend(const std::string &string) {
	return AttemptSend(string.c_str(), string.length());
}

int SocketClass :: AttemptSend(const char *buffer, int buflen)
{
	//Returns the number of bytes sent, or -1 of failure so that the calling function
	//can output its own success or failure message.
#ifdef DEBUG_TIME
	unsigned long startTime = g_PlatformTime.getMilliseconds();
#endif

	int errCount = 0;
	int pos = 0;
	int ssize = 0;
	int res = 0;
	do
	{
		ssize = buflen - pos;

		res = send(ClientSocket, &buffer[pos], ssize, SEND_FLAGS);
		if(res == -1)
		{
			// Debugging for Linux to see if this fixes problems with signals.
#ifndef WINDOWS_PLATFORM
			if(errno == EPIPE)
				fprintf(stderr, "[WARNING] AttemptSend() socket error: EPIPE\n");
			else
				fprintf(stderr, "[WARNING] AttemptSend() socket error: %d\n", errno);
			fflush(stderr);
#endif
			return -1;
		}
		else if(res == 0)
		{
			errCount++;
			if(errCount > 5)
			{
				char timeBuf[64];
				time_t curtime;
				time(&curtime);
				strftime(timeBuf, sizeof(timeBuf), "%x %X", localtime(&curtime));

				fprintf(stderr, "%s [CRITICAL] AttemptSend() repeated send zero size\r\n", timeBuf);
				break;
			}
		}

		pos += res;
	} while(pos < buflen);

#ifdef DEBUG_TIME
	unsigned long passTime = g_PlatformTime.getMilliseconds() - startTime;
	if(passTime > 250)
		g_Logs.server->warn("Socket Send Time: %v ms", passTime);
#endif

	if(pos == buflen)
		return pos;

	return -1;
}

int SocketClass :: AttemptSendNoBlock(const std::string &string) {
	return AttemptSendNoBlock(string.c_str(), string.length());
}

int SocketClass :: AttemptSendNoBlock(const char *buffer, int buflen)
{
	//Returns the number of bytes sent, or -1 of failure so that the calling function
	//can output its own success or failure message.

#ifdef DEBUG_TIME
	unsigned long startTime = g_PlatformTime.getMilliseconds();
#endif

#ifdef WINDOWS_PLATFORM
	const int FLAGS = SocketClass::SEND_FLAGS;
#else
	const int FLAGS = MSG_DONTWAIT | SocketClass::SEND_FLAGS;
#endif

	int errCount = 0;
	int pos = 0;
	int ssize = 0;
	int res = 0;
	do
	{
		ssize = buflen - pos;
		res = send(ClientSocket, &buffer[pos], ssize, FLAGS);
		if(res == -1)
		{
			// Debugging for Linux to see if this fixes problems with signals.
#ifndef WINDOWS_PLATFORM
			if(errno == EPIPE)
			{
				fprintf(stderr, "[WARNING] AttemptSend() socket error: EPIPE\n");
				fflush(stderr);
			}
			else if(errno == EAGAIN || errno == EWOULDBLOCK)
			{
				return -1;
			}
			else
			{
				fprintf(stderr, "[WARNING] AttemptSend() socket error: %d\n", errno);
				fflush(stderr);
			}
#endif
			return -1;
		}
		else if(res == 0)
		{
			errCount++;
			if(errCount > 5)
			{
				char timeBuf[64];
				time_t curtime;
				time(&curtime);
				strftime(timeBuf, sizeof(timeBuf), "%x %X", localtime(&curtime));

				fprintf(stderr, "%s [CRITICAL] AttemptSendNoBlock() repeated failures\r\n", timeBuf);
				break;
			}
		}

		pos += res;
	} while(pos < buflen);

#ifdef DEBUG_TIME
	unsigned long passTime = g_PlatformTime.getMilliseconds() - startTime;
	if(passTime > 50)
		g_Logs.server->warn("Socket AttemptSendNoBlock Send Time: %v ms", passTime);
#endif

	if(pos == buflen)
		return pos;

	return -1;
}


const char * SocketClass :: GetErrorMessage(void)
{
#ifdef WINDOWS_PLATFORM

	// Windows errors.
	switch(WSAGetLastError())
	{
	case WSANOTINITIALISED:
		return "WSANOTINITIALISED";
	case WSAEFAULT:
		return "WSAEFAULT";
	case WSAENETDOWN:
		return "WSAENETDOWN";
	case WSAEINVAL:
		return "WSAEINVAL";
	case WSAEINTR:
		return "WSAEINTR";
	case WSAEINPROGRESS:
		return "WSAEINPROGRESS";
	case WSAENOTSOCK:
		return "WSAENOTSOCK";
	default:
		LogMessage("[SOCKET] Unidentified error: %d", WSAGetLastError());
		return "Unidentified error.";
	}
#else

	// Linux errors.
	switch(errno)
	{
	case EBADF:
		return "sockfd is not a valid descriptor.";
	case EINVAL:
		return "The socket is already bound to an address.";
	case EACCES:
		return "The address is protected, and the user is not the super-user.";
	case ENOTSOCK:
		return "Argument is a descriptor for a file, not a socket.";
	//case EINVAL:
	//	return "The addrlen is wrong, or the socket was not in the AF_UNIX family.";
	case EROFS:
		return "The socket inode would reside on a read-only file system.";
	case EFAULT:
		return "my_addr points outside the user's accessible address space.";
	case ENAMETOOLONG:
		return "my_addr is too long.";
	case ENOENT:
		return "The file does not exist.";
	case ENOMEM:
		return "Insufficient kernel memory was available.";
	case ENOTDIR:
		return "A component of the path prefix is not a directory.";
	//case EACCES:
	//	return "Search permission is denied on a component of the path prefix.";
	case ELOOP:
		return "Too many symbolic links were encountered in resolving my_addr.";
	default:
		g_Logs.server->error("Unidentified socket error: %v", errno);
		return "Unidentified error.";
	}
#endif

	return "Unidentified error.";
}


int SocketClass :: tryrecv(char *buffer, int bufsize)
{
#ifdef WINDOWS_PLATFORM
	return recv(ClientSocket, buffer, bufsize, 0);
#else 
	int res = 0;
	do
	{
		res = recv(ClientSocket, buffer, bufsize, 0);
		if(res != -1)
			break;
		if(errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
			break;
		fprintf(stderr, "tryrecv failed: %d", errno);

	} while(1);
	return res;
#endif //#ifdef WINDOWS_PLATFORM

	/* OBSOLETE, NO LONGER NEEDED
	int res = 0;
	while(res == 0)
	{
		FD_ZERO(&readset);
		FD_SET(ClientSocket, &readset);
		seltime.tv_sec = 2;
		seltime.tv_usec = 0;
		res = select(ClientSocket + 1, &readset, NULL, NULL, &seltime);
		if(res == -1)
		{
			if(errno == EINTR)
			{
				fprintf(stderr, "tryrecv EINTR\r\n");
				res = 0;
			}
		}
	}

	if(res == -1)
	{
		fprintf(stderr, "tryrecv error: %d\r\n", errno);
		return -2;
	}

    if(FD_ISSET(ClientSocket, &readset))
        return recv(ClientSocket, buffer, bufsize, 0);

	fprintf(stderr, "tryrecv failed (res: %d, errno: %d)\r\n", res, errno);
	*/
}

void SocketClass :: SetClientNoDelay(void)
{
	int flag = 1;
	setsockopt(ClientSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, sizeof(flag));
}

void SocketClass :: SetTimeOut(int seconds)
{
	/*
	timeval timedata;
	timedata.tv_sec = seconds;
	timedata.tv_usec = 0;
	int r = setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timedata, sizeof(timedata)); 
	if(r == -1)
		fprintf(stderr, "[SOCKET] SetTimeOut failed: %d", errno);

	r = setsockopt(ClientSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timedata, sizeof(timedata)); 
	if(r == -1)
		fprintf(stderr, "[SOCKET] SetTimeOut failed: %d", errno);
		*/
	//timeval timedata;
	//timedata.tv_sec = seconds;
	//timedata.tv_usec = 0;

	/*
#ifndef WINDOWS_PLATFORM
	int r;
	int option;
	
	option = 1;
	r = setsockopt(ClientSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&option, sizeof(option)); 
	if(r == -1)
		fprintf(stderr, "[SOCKET] setsockopt SO_KEEPALIVE failed: %d", errno);
	r = getsockopt(ClientSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&option, sizeof(option)); 
	if(r == -1)
		fprintf(stderr, "[SOCKET] getsockopt SO_KEEPALIVE failed: %d", errno);

	option = 5;
	r = setsockopt(ClientSocket, SOL_TCP, TCP_KEEPCNT, (char*)&option, sizeof(option)); 
	if(r == -1)
		fprintf(stderr, "[SOCKET] setsockopt TCP_KEEPCNT failed: %d", errno);
	r = getsockopt(ClientSocket, SOL_TCP, TCP_KEEPCNT, (char*)&option, sizeof(option)); 
	if(r == -1)
		fprintf(stderr, "[SOCKET] getsockopt TCP_KEEPCNT failed: %d", errno);


	option = 5;
	r = setsockopt(ClientSocket, SOL_TCP, TCP_KEEPIDLE, (char*)&option, sizeof(option)); 
	if(r == -1)
		fprintf(stderr, "[SOCKET] setsockopt TCP_KEEPIDLE failed: %d", errno);
	r = getsockopt(ClientSocket, SOL_TCP, TCP_KEEPIDLE, (char*)&option, sizeof(option)); 
	if(r == -1)
		fprintf(stderr, "[SOCKET] getsockopt TCP_KEEPIDLE failed: %d", errno);


	option = 5;
	r = setsockopt(ClientSocket, SOL_TCP, TCP_KEEPINTVL, (char*)&option, sizeof(option)); 
	if(r == -1)
		fprintf(stderr, "[SOCKET] setsockopt TCP_KEEPINTVL failed: %d", errno);
	r = getsockopt(ClientSocket, SOL_TCP, TCP_KEEPINTVL, (char*)&option, sizeof(option)); 
	if(r == -1)
		fprintf(stderr, "[SOCKET] getsockopt TCP_KEEPINTVL failed: %d", errno);
#endif
		*/
}


//static
void SocketClass :: DisconnectClient(int socket)
{
	if(socket != Invalid_Socket)
	{
		shutdown(socket, SHUTDOWN_PARAM);
		CLOSE_SOCKET(socket);
	}
}

void SocketClass::SetDebugName(const char *name)
{
	strncpy(debugName, name, sizeof(debugName) - 1);
}

void SocketClass::TransferClientSocketFrom(SocketClass& source)
{
	//Listening sockets may transfer their client to another socket control system (usually in
	//a different thread). If two systems have control of the same ClientSocket simultaneously,
	//and are both shut down (such as a server shut down) then the socket will be closed twice
	//and potentially cause crashes or resource corruption.
	ClientSocket = source.ClientSocket;
	source.ClientSocket = Invalid_Socket;
}
