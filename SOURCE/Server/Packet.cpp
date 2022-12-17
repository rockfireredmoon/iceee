#include "SocketClass3.h"
#include "Packet.h"


PacketManager g_PacketManager;

//Just some debugging stuff
#include <time.h>
#include "Util.h"
#include "Report.h"
#include "Config.h"
#include "Debug.h"
#include "util/Log.h"

DebugPacketStatistics::DebugPacketStatistics() {
	Clear();
}
void DebugPacketStatistics::Clear(void) {
	mPendingBytesTotal = 0;
	mPartialBytesTotal = 0;
	mPartialEventCount = 0;
	mLastErrorTime = 0;
	mErrorTimestamp.clear();
}
void DebugPacketStatistics::UpdateReport(ReportBuffer &report) {
	report.AddLine("%s|%lu|%lu|%lu", mErrorTimestamp.c_str(),
			mPartialEventCount, mPartialBytesTotal, mPendingBytesTotal);
}

void DebugPacketManager::UpdateHistory(void) {
	if (mCurrent.mLastErrorTime == 0)
		return;

	unsigned long elapsed = g_ServerTime - mCurrent.mLastErrorTime;
	if (elapsed > 10000) {
		char timeBuf[64];
		time_t curtime;
		time(&curtime);
		strftime(timeBuf, sizeof(timeBuf), "%x %X", localtime(&curtime));
		mCurrent.mErrorTimestamp = timeBuf;

		mHistory.push_back(mCurrent);
		mCurrent.Clear();

		if (mHistory.size() > 512) {
			g_Logs.server->debug("mHistory size too large");
			mHistory.erase(mHistory.begin(), mHistory.begin() + 256);
		}
	}
}
void DebugPacketManager::UpdateReport(ReportBuffer &report) {
	report.AddLine("History");
	for (size_t i = 0; i < mHistory.size(); i++)
		mHistory[i].UpdateReport(report);
	mCurrent.UpdateReport(report);
}
//end debugging

Packet::Packet() {
}

Packet::~Packet() {
}

void Packet::Assign(const char *buffer, int length) {
	//Sanity checks, old debugging stuff...
	if (length > 100000)
		g_Logs.server->error("Assign() length is %v", length);
	//mData.reserve(length);
	mData.assign(buffer, length);
}

void Packet::AssignFrom(const Packet &source, size_t begin, size_t end) {
	if (end > source.mData.size())
		end = source.mData.size();
	mData.assign(source.mData.begin() + begin, source.mData.begin() + end);
}

void Packet::Append(const char *buffer, int length) {
	if (length > 100000)
		g_Logs.server->error("Append() length is %v", length);

	if (mData.size() == 0)
		mData.assign(buffer, length);
	else
		mData.append(buffer, length);
}

void Packet::Append(const Packet &data) {
	if (data.mData.size() > 100000)
		g_Logs.server->error("Append(&data) bytecount: %v",	data.mData.size());

	if (mData.size() == 0)
		mData.assign(data.mData);
	else
		mData.append(data.mData);
}

void Packet::Clear(void) {
	mData.clear();
}

void Packet::TrimFront(size_t byteCount) {
	if (byteCount > 100000)
		g_Logs.server->error("TrimFront bytecount: %v", byteCount);
	if (mData.size() > 100000)
		g_Logs.server->error("TrimFront mData.size: %v",
				mData.size());
	mData.erase(0, byteCount);
}

PacketManager::PacketManager() {
	mLastError = 0;
	//cs.SetDebugName("CS_PACKMAN");
	bThreadActive = false;

	mTotalBytesSent = 0;
	mTotalBytesDropped = 0;

	mTotalPacketsSent = 0;
	mTotalPacketsDropped = 0;

	mTotalBytesIncomplete = 0;
	mTotalPacketsIncomplete = 0;

	mTotalSendZero = 0;
	mCountForceDelay = 0;
	mCountForceDelayAck = 0;

	mTotalWait = 0;
	mClusterPackets = 0;
	mClusterPacketBytes = 0;
	cs.Init();
}

PacketManager::~PacketManager() {
	bThreadActive = false;
}

void PacketManager::GetThread(const char *request) {
	cs.Enter(request);
}

void PacketManager::ReleaseThread() {
	cs.Leave();
}

void PacketManager::RunMain(void) {
	bThreadActive = true;

	while (bThreadActive == true) {
		GetPackets2();
		//Debug_RunDiagnostics();
		SendPackets2();
		PLATFORM_SLEEP(1);
	}
}

void PacketManager::InitThread()
{
	mThread = new boost::thread( { &PacketManager::RunMain, this });
}

void PacketManager::Shutdown(void) {
	bThreadActive = false;
	mThread->join();
	delete mThread;
}

void PacketManager::AddOutgoingPacket2(int socket, const Packet &data) {
	if (data.mData.size() == 0) {
		g_Logs.server->error("Cannot add packet of zero size.");
		return;
	}

	PendingSocket *pSock = GetPendingSocket(socket);
	if (pSock != NULL) {
		pSock->mPacketList.push_back(data);
	} else
		g_Logs.server->error("Could not retrieve a PendingSocket object.");
}

void PacketManager::ExternalAddPacket(int socket, const char *data,
		int length) {
	if (length == 0) {
		g_Logs.server->error("Cannot add packet of zero size.");
		return;
	}

	Packet packet;
	packet.Assign(data, length);

	GetThread("PacketManager::ExternalAddPacket");
	PendingSocket *pSock = GetPendingSocket(socket);
	if (pSock != NULL)
		pSock->mPacketList.push_back(packet); //Clustering will be performed when the thread grabs and sorts this pending data.
	else
		g_Logs.server->error("Could not retrieve a PendingSocket object.");
	ReleaseThread();
}

void PacketManager::GetPackets2(void) {
	if (mQueueData.size() == 0)
		return;

#ifdef DEBUG_TIME
	Debug::TimeTrack("GetPackets2", 50);
#endif

	GetThread("PacketManager::GetPackets2");
	mTransition.assign(mQueueData.begin(), mQueueData.end());
	mQueueData.clear();
	ReleaseThread();

	//Sort the transition list into the outgoing Send list according to socket.
	//Try to cluster the packets if possible.
	std::list<PendingSocket>::iterator it;
	std::list<Packet>::iterator pit;
	for (it = mTransition.begin(); it != mTransition.end(); ++it) {
		PendingSocket *pSock = GetSendSocket(it->mSocket);
		if (pSock != NULL) {
			for (pit = it->mPacketList.begin(); pit != it->mPacketList.end();
					++pit) {
				int r = pSock->AddData(*pit);
				if (r == PendingSocket::DATA_ELEMENT_APPEND) {
					mClusterPackets++;
					mClusterPacketBytes += pit->mData.size();
				}
			}
		}
	}

	//All sorted, wipe the transition buffer.
	mTransition.clear();

	/*
	 if(mSendData.size() == 0)
	 mSendData.assign(mQueueData.begin(), mQueueData.end());
	 else
	 mSendData.insert(mSendData.end(), mQueueData.begin(), mQueueData.end());
	 mQueueData.clear();
	 ReleaseThread();
	 */
}

void PacketManager::SendPacketsFor(int socket) {
	if (mSendData.size() == 0)
		return;

#ifdef DEBUG_TIME
	Debug::TimeTrack("SendPacketsFor", 50);
#endif

	std::list<PendingSocket>::iterator it;
	it = mSendData.begin();
	while (it != mSendData.end()) {
		if(it->mSocket == socket) {
			int res = SendSocket(it->mSocket, *it);
			if (res == SEND_DELAY) {
				mTotalWait++;
				++it;
			} else {
				//If we get here, it had a serious failure or it succeeded.
				//Either way, we're done with the data, so it can be erased.
				if (res == SEND_FAILED) {
					g_Logs.server->error("Disconnecting Socket:%v", it->mSocket);
					SocketClass::DisconnectClient(it->mSocket);
				}
				mSendData.erase(it++);
			}
		}
		else
			++it;
	}
}

void PacketManager::SendPackets2(void) {
	if (mSendData.size() == 0)
		return;

#ifdef DEBUG_TIME
	Debug::TimeTrack("SendPackets2", 50);
#endif

	std::list<PendingSocket>::iterator it;
	it = mSendData.begin();
	while (it != mSendData.end()) {
		int res = SendSocket(it->mSocket, *it);
		if (res == SEND_DELAY) {
			mTotalWait++;
			++it;
		} else {
			//If we get here, it had a serious failure or it succeeded.
			//Either way, we're done with the data, so it can be erased.
			if (res == SEND_FAILED) {
				g_Logs.server->error("Disconnecting Socket:%v", it->mSocket);
				SocketClass::DisconnectClient(it->mSocket);
			}
			mSendData.erase(it++);
		}
	}
}

int PacketManager::SendSocket(int socket, PendingSocket &socketData) {
	if (socketData.mPacketList.size() == 0) {
		g_Logs.server->error("No socket data");
		return SEND_DATA;
	}

	//Hack, experimental fix to see if easing up on send attempts will reduce lag cascades
	//when the server is experiencing packet loss.
	if (g_ServerTime < socketData.mForcedResumeTime) {
		mCountForceDelayAck++;
		return SEND_DELAY;
	}

	std::list<Packet>::iterator pit;
	pit = socketData.mPacketList.begin();
	int sendRes = 0;

	//Iterate over the pending data.  Return on the first chance of failure so that we can
	//quickly skip ahead to any other pending sockets.
	while (pit != socketData.mPacketList.end()) {
		int size = pit->mData.size();
		//g_Log.AddMessageFormat("Sending data: %d bytes to socket: %d", size, socket);
		sendRes = AttemptSend2(socket, pit->mData.data(), size);
		if (sendRes == size) {
			mTotalPacketsSent++;
			mTotalBytesSent += size;
			socketData.mPacketList.erase(pit++);

			//g_Log.AddMessageFormat("Full packet sent: %d", size);
		} else if (sendRes == -1) {
			mTotalPacketsDropped++;
			mTotalBytesDropped += size;
			//g_Log.AddMessageFormat("Failed to send packet: %d", size);
			return SEND_FAILED;
		} else {
			//g_Log.AddMessageFormat("Partial send: %d / %d", sendRes, size);
			if (sendRes > 0) {
				pit->TrimFront(sendRes);

				mTotalBytesIncomplete += pit->mData.size();
				mTotalPacketsIncomplete++;
			} else if (sendRes == 0) {
				mTotalSendZero++;
			}

			//For debugging only.
			DebugPacketManager &derror = mDebugData[socketData.mSocket];
			derror.UpdateHistory();
			derror.mCurrent.mPendingBytesTotal += pit->mData.size();
			derror.mCurrent.mPartialBytesTotal += sendRes;
			derror.mCurrent.mPartialEventCount++;
			derror.mCurrent.mLastErrorTime = g_ServerTime;
			if (derror.mCurrent.mPartialEventCount
					% g_Config.DebugPacketSendTrigger == 0) {
				if (g_Config.DebugPacketSendMessage == true)
					g_Logs.server->error("Socket:%v failing to send (%v/%v), %v bytes remaining in %v packets",
							socketData.mSocket, sendRes, size,
							socketData.DebugGetPendingSize(),
							socketData.mPacketList.size());

				if (g_Config.DebugPacketSendDelay != 0) {
					mCountForceDelay++;
					socketData.mForcedResumeTime = g_ServerTime
							+ g_Config.DebugPacketSendDelay;
				}
			}
			//End debugging.

			return SEND_DELAY;
		}
	}
	return SEND_SUCCESS;
}

int PacketManager::AttemptSend2(int socket, const char *buffer, int length) {
#ifdef WINDOWS_PLATFORM
	const int FLAGS = SocketClass::SEND_FLAGS;
#else
	const int FLAGS = MSG_DONTWAIT | SocketClass::SEND_FLAGS;
#endif
	int written = 0;
	int r = 0;
	while (written < length) {
		r = send(socket, &buffer[written], length - written, FLAGS);
		if (r == -1) {
#ifdef WINDOWS_PLATFORM
			mLastError = WSAGetLastError();
			if(mLastError == WSAEWOULDBLOCK)
			return written;
#else
			mLastError = errno;
			if (mLastError == EAGAIN || mLastError == EWOULDBLOCK)
				return written;
#endif
			//Not a wait/block error, could be an invalid socket or something else
			//important.
			g_Logs.server->error("Unspecified socket error: %v",
					mLastError);
			return -1;
		}
		written += r;

		if (r == 0) //Unable to send, but the connection is still valid, hence no error.
			break;
	}
	return written;
}

PendingSocket* PacketManager::GetPendingSocket(int socket) {
	std::list<PendingSocket>::iterator it;
	if (mQueueData.size() > 0) {
		for (it = mQueueData.begin(); it != mQueueData.end(); ++it)
			if (it->mSocket == socket)
				return &*it;
	}
	mQueueData.push_back(PendingSocket(socket));
	if (mQueueData.size() > 0)
		return &mQueueData.back();

	return NULL;
}

PendingSocket* PacketManager::GetSendSocket(int socket) {
	//Same as for the Queue return.
	std::list<PendingSocket>::iterator it;
	if (mSendData.size() > 0) {
		for (it = mSendData.begin(); it != mSendData.end(); ++it)
			if (it->mSocket == socket)
				return &*it;
	}
	mSendData.push_back(PendingSocket(socket));
	if (mSendData.size() > 0)
		return &mSendData.back();

	return NULL;
}

void PacketManager::GenerateDebugReport(ReportBuffer &report) {
	DEBUG_MAP::iterator it;
	report.AddLine("--Packet Debug Report--");
	for (it = mDebugData.begin(); it != mDebugData.end(); ++it) {
		report.AddLine("Socket:%d", it->first);
		it->second.UpdateReport(report);
		report.AddLine(NULL);
	}
}

PendingSocket::PendingSocket() {
	mSocket = -1;
	mForcedResumeTime = 0;
}

PendingSocket::PendingSocket(int socket) {
	mSocket = socket;
	mForcedResumeTime = 0;
}

PendingSocket::~PendingSocket() {
	mPacketList.clear();
}

int PendingSocket::AddData(const Packet &data) {
//Hack for some run-time configurable chunk size debugging.
	size_t CHUNK_SIZE = DATA_ELEMENT_LIMIT;
	if (g_Config.ForceMaxPacketSize > 0)
		CHUNK_SIZE = g_Config.ForceMaxPacketSize;

	if (mPacketList.size() > 0) {
		if (mPacketList.back().mData.size() + data.mData.size() <= CHUNK_SIZE) //DATA_ELEMENT_LIMIT
				{
			mPacketList.back().Append(data);
			return DATA_ELEMENT_APPEND;
		}
	}

	if (g_Config.ForceMaxPacketSize > 0) {
		size_t pos = 0;
		for (pos = 0; pos < data.mData.size(); pos += CHUNK_SIZE) {
			Packet piece;
			piece.AssignFrom(data, pos, pos + CHUNK_SIZE);
			mPacketList.push_back(piece);
		}
	} else {
		mPacketList.push_back(data);
	}
	return DATA_ELEMENT_NEW;
}

int PendingSocket::DebugGetPendingSize(void) {
	int remain = 0;
	std::list<Packet>::iterator it;
	for (it = mPacketList.begin(); it != mPacketList.end(); ++it)
		remain += it->mData.size();

	return remain;
}

