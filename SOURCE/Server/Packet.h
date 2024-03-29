#ifndef PACKET_H
#define PACKET_H

#include "Components.h"

#include <string>
#include <list>
#include <vector>
#include <thread>

#include <map>
#include "Report.h"  //Only needed for debug report.

using namespace std;

class Packet
{
public:
	Packet();
	~Packet();
	string mData;
	void Assign(const char *buffer, int length);
	void AssignFrom(const Packet &source, size_t begin, size_t end);
	void Append(const char *buffer, int length);
	void Append(const Packet &data);
	void Clear(void);
	void TrimFront(size_t byteCount);
};

class MultiSendPacket : public Packet
{
	vector<int> mSocketList;
};

class PendingSocket
{
public:
	PendingSocket();
	PendingSocket(int socket);
	~PendingSocket();
	int mSocket;
	list<Packet> mPacketList;
	unsigned long mForcedResumeTime;        //Experimental fix for some socket errors by delaying immediate retries for a very short time.
	
	int AddData(const Packet &data);
	int DebugGetPendingSize(void);
	
	static const int DATA_ELEMENT_LIMIT = 1000;   //Do not append data to an existing element if the size would exceed this.
	static const int DATA_ELEMENT_NEW    = 0;     //A new element was pushed onto the list.
	static const int DATA_ELEMENT_APPEND = 1;     //Data was added to an existing buffer.
};

struct DebugPacketStatistics
{
	unsigned long mPendingBytesTotal;   //Total number of bytes left pending from a failed send()
	unsigned long mPartialBytesTotal;   //Total number of bytes successfully sent from a failed send()
	unsigned long mPartialEventCount;   //Total number of times the send() command was unable to send all bytes in the queue at once.
	unsigned long mLastErrorTime;
	string mErrorTimestamp;
	DebugPacketStatistics();
	void Clear(void);
	void UpdateReport(ReportBuffer &report);
};

struct DebugPacketManager
{
	DebugPacketStatistics mCurrent;
	vector<DebugPacketStatistics> mHistory;
	void UpdateHistory(void);
	void UpdateReport(ReportBuffer &report);
};

class PacketManager
{
public:
	PacketManager();
	~PacketManager();

	typedef pair<int, Packet> PACKET_PAIR;   //int socket, Packet data
	typedef list<PACKET_PAIR> PACKET_CONT;   //int socket, Packet data
	typedef map<int, DebugPacketManager> DEBUG_MAP;  //int socket, struct of error information.
	/*
	list<PACKET_PAIR> mListQueue;   //Packet data that has yet to be handled by the thread.
	list<PACKET_PAIR> mListSending; //Data that has been acquired by the thread and is pending sending.
	void AddOutgoingPacket(int socket, Packet &data);
	bool AppendExistingPacket(int socket, Packet &data);
	int AttemptSend(int socket, const char *buffer, int length);
	int GetPackets(void);
	void SendPackets(void);
	*/

	static const int SEND_SUCCESS = 0;
	static const int SEND_FAILED = -1;
	static const int SEND_DELAY = -2;
	static const int SEND_DATA = -3;  //Invalid data

	void GetThread(const char *request);
	void ReleaseThread(void);

	void RunMain(void);
	void InitThread(void);
	void Shutdown(void);
	bool bThreadActive;

	unsigned long mTotalBytesSent;         //Total number of bytes successfully sent on the socket.
	unsigned long mTotalBytesDropped;      //Total number of bytes that failed to send (socket error)
	unsigned long mTotalBytesIncomplete;   //Total number of bytes left pending in buffers if a send() operation could not transmit the entire buffer.
	unsigned long mTotalPacketsSent;       //Total packets successfully sent.
	unsigned long mTotalPacketsDropped;    //Total packets that failed ot send.
	unsigned long mTotalPacketsIncomplete; //Total count of incomplete packets, where the send() operation was not able to transmit the entire buffer in one go.
	unsigned long mClusterPackets;         //Number of packets appended to the end of an existing queued packet before transmit.
	unsigned long mClusterPacketBytes;     //Number of bytes appended to existing packets.
	unsigned long mTotalSendZero;          //Total number of packets where send() returned zero.
	unsigned long mCountForceDelay;        //If a delay between retrying packets has been forced, this is number of times this has occurred. 
	unsigned long mCountForceDelayAck;     //Total count of times hwere a forced delay was acknowledged by passing over a pending socket.
	unsigned long mTotalWait;
	
	void AddOutgoingPacket2(int socket, const Packet &data);
	void ExternalAddPacket(int socket, const char *data, int length);
	void SendPacketsFor(int socket);
	void GenerateDebugReport(ReportBuffer &report);

private:
	PendingSocket* GetPendingSocket(int socket);
	int AttemptSend2(int socket, const char *buffer, int length);
	int SendSocket(int socket, PendingSocket &data);
	void SendPackets2(void);
	void GetPackets2(void);
	PendingSocket* GetSendSocket(int socket);

	list<PendingSocket> mQueueData;   //Data queued for sending.  The thread hasn't acquired this data yet.
	list<PendingSocket> mTransition;  //Transitional buffer.  The data has been taken from the Queue, but needs to be processed into the Send list.
	list<PendingSocket> mSendData;    //Data acquired by the thread, and undergoing send processing.
	map<int, DebugPacketManager> mDebugData;
	thread *mThread;
	int mLastError;
	Platform_CriticalSection cs;
};

extern PacketManager g_PacketManager;

#endif //#ifndef PACKET_H
