// Now with slightly more portability.

#pragma once
#ifndef SOCKETCLASS3_H
#define SOCKETCLASS3_H

#include "Components.h"

#ifdef WINDOWS_PLATFORM
#include <ws2tcpip.h>   //For struct sockaddr_in
#else
#include <netinet/in.h> //For struct sockaddr_in
#include <errno.h>
#endif

extern char *LogMessage(const char *format, ...);

class SocketClass
{
public:
	SocketClass();
	~SocketClass();
	void Clear(void);

	int CreateSocket(char *port, const char *address);
	int Accept(void);
	void DisconnectClient(void);
	void ShutdownServer(void);

	int port;
	int ListenSocket;
	int ClientSocket;
	char destAddr[INET6_ADDRSTRLEN];
	sockaddr_in acceptData;
	char debugName[32];  //Used to identify the owner of this socket, for debug purposes.

	int AttemptSend(const char *buffer, int buflen);
	int AttemptSendNoBlock(const char *buffer, int buflen);
	int tryrecv(char *buffer, int bufsize);
	void TransferClientSocketFrom(SocketClass& source);
	void SetClientNoDelay(void);
	void SetTimeOut(int seconds);
	void SetDebugName(const char *name);

	const char * GetErrorMessage(void);

	static const int Invalid_Socket = -1;

#ifdef WINDOWS_PLATFORM
	static const int SEND_FLAGS = 0;
	static const int SHUTDOWN_PARAM = SD_BOTH;
#else
	static const int SEND_FLAGS = MSG_NOSIGNAL;
	static const int SHUTDOWN_PARAM = SHUT_RDWR;
#endif

	static void DisconnectClient(int socket);

private:
	fd_set readset;
	timeval seltime;
	bool shuttingDown;
};

#endif //#ifndef SOCKETCLASS3_H
