#include "BroadCast.h"


BroadcastMessage2 bcm;

BroadcastMessage2 :: BroadcastMessage2()
{
	Init();
	cs.Init();
	cs.SetDebugName("CS_BCM");
	cs.disabled = true;
}

BroadcastMessage2 :: ~BroadcastMessage2()
{
	Free();
}

void BroadcastMessage2 :: EnterCS(const char *request)
{
	//EnterCriticalSection(&cs);
	cs.Enter(request);
}

void BroadcastMessage2 :: LeaveCS(void)
{
	//LeaveCriticalSection(&cs);
	cs.Leave();
}

void BroadcastMessage2 :: Init(void)
{
	//InitializeCriticalSection(&cs);
}

void BroadcastMessage2 :: Free(void)
{
	mlog.clear();
	//DeleteCriticalSection(&cs);
}

int BroadcastMessage2 :: AddEventCopy(MessageComponent &msg)
{
	int r = -1;
	EnterCS("BroadcastMessage2::AddEventCopy");
	mlog.push_back(msg);
	r = mlog.size() - 1;
	LeaveCS();
	return r;
}

int BroadcastMessage2 :: AddEvent(int newSimID, long newParam1, int newMessage, ActiveInstance * newActInst)
{
	int r = -1;
	EnterCS("BroadcastMessage2::AddEvent");
	MessageComponent msg = {0};
	msg.SimulatorID = newSimID;
	msg.param1 = newParam1;
	msg.message = newMessage;
	msg.actInst = newActInst;
	msg.x = 0;
	msg.z = 0;
	mlog.push_back(msg);
	r = mlog.size() - 1;
	LeaveCS();
	return r;
}

int BroadcastMessage2 :: AddEvent2(int newSimID, long newParam1, long newParam2, int newMessage, ActiveInstance* newActInst)
{
	int r = -1;
	EnterCS("BroadcastMessage2::AddEvent2");
	MessageComponent msg = {0};
	msg.SimulatorID = newSimID;
	msg.param1 = newParam1;
	msg.param2 = newParam2;
	msg.message = newMessage;
	msg.actInst = newActInst;
	msg.x = 0;
	msg.z = 0;
	mlog.push_back(msg);
	r = mlog.size() - 1;
	LeaveCS();
	return r;
}

int BroadcastMessage2 :: RemoveSimulator(int SimulatorID)
{
	if(mlog.size() == 0)
		return 0;

	EnterCS("BroadcastMessage2::RemoveSimulator");
	int count = 0;
	size_t pos = 0;
	while(pos < mlog.size())
	{
		if(mlog[pos].SimulatorID == SimulatorID)
		{
			mlog.erase(mlog.begin() + pos);
			count++;
		}
		else
			pos++;
	}
	LeaveCS();

	return count;
}