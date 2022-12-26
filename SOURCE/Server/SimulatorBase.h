// Thread for the Simulator listening server
//
//

#ifndef SIMULATORBASE_H
#define SIMULATORBASE_H

#include "Components.h"
#include "SocketClass3.h"

#include <thread>

using namespace std;

class SimulatorBaseThread {
public:
	SimulatorBaseThread();
	~SimulatorBaseThread();

	unsigned long ThreadID;      //ID of the created thread

	char RecBuf[64]; //Holds the receiving data, most likely the HTTP GET request for a file.
	char SendBuf[64];    //Holds the data that will be sent back to the client

	long TotalRecBytes; //The total number of bytes received since the server was launched

	long TotalSendBytes; //The total number of bytes sent since the server was launched

	bool isExist; //If true, the thread exists.  If it doesn't, the main program should attempt to reactivate it.
	int Status; //This maintains the activity Status, determining whether it needs to Init, Restart, etc.

	int HomePort;
	string BindAddress;

	void SetBindAddress(const string &address);
	void SetHomePort(int port);
	void InitThread(int instanceindex, int globalThreadID);
	void OnConnect(void);  //Called once a connection has been made
	void Shutdown(void);   //Force a complete shutdown, thread too
	void CheckAutoResponse(void);

	int LaunchSimulatorThread(void);
private:
	void RunMain(void);
	void RunMainLoop(void);

	int MessageCountRec;
	thread *mThread;
	bool isActive;        //If true, the is active and running.
	SocketClass sc;       //Controls the socket connection for this thread.
	long RecBytes;       //The number of bytes received from the last message
	long SendBytes;       //The number of bytes sent on the last message
	int InternalIndex; //A user defined value to identify this instance index in order to assist in diagnostics
	int GlobalThreadID; //A unique application-defined thread ID for debugging purposes.
};

extern SimulatorBaseThread SimulatorBase;

#endif //SIMULATORBASE_H
