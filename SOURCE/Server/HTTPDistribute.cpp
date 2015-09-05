#include "HTTPDistribute.h"

#include "Config.h"
#include "Globals.h"
#include "StringList.h"
#include "Simulator.h"
#include "Components.h"
#include "RemoteAction.h"
#include "Account.h"
#include "Util.h"
#include "ZoneDef.h"
#include <stdlib.h>
#include "FileReader.h"

PLATFORM_THREADRETURN HTTPDistributeThreadProc(PLATFORM_THREADARGS lpParam);

HTTPDistributeManager g_HTTPDistributeManager;
//std::list<HTTPDistribute> HTTPDistributeServer;

Platform_CriticalSection http_cs("HTTP_CS");
FileChecksum g_FileChecksum;

void FileChecksum :: LoadFromFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("[WARNING] Could not open file: %s", filename);
		return;
	}
	lfr.CommentStyle = Comment_Semi;
	std::string readfilename;
	std::string readchecksum;
	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.BreakUntil("=", '=');
		if(r >= 2)
		{
			readfilename = lfr.BlockToStringC(0, 0);
			readchecksum = lfr.BlockToStringC(1, 0);
			mChecksumData[readfilename] = readchecksum;
			//g_Log.AddMessageFormat("SET: %s=%s (%s)", readfilename.c_str(), mChecksumData[readfilename].c_str(), readchecksum.c_str());
		}
	}
	lfr.CloseCurrent();
}

bool FileChecksum :: MatchChecksum(const std::string &filename, const std::string &checksum)
{
	CHECKSUM_MAP::iterator it = mChecksumData.find(filename);

	//If it doesn't appear in the list, assume it's valid so the client doesn't redownload
	if(it == mChecksumData.end()) {
		g_Log.AddMessageFormat("[WARNING] File %s is not in the index, so it's checksum is unknown. Assuming no download required.");
		return true;
	}

	if(it->second.compare(checksum) == 0)
		return true;

	return false;
}


HTTPDistribute :: HTTPDistribute()
{
	//The reset function attempts to close a file if it's non-null, so
	//set this here.
	DataFile = NULL;
	ResetValues(true);
}

HTTPDistribute :: ~HTTPDistribute()
{
	sc.ShutdownServer();
	CloseFileHandle();
}

void HTTPDistribute :: ResetValues(bool fullRestart)
{
	//Erases the receiving buffer and resets the byte counters.
	//Optionally if <fullRestart> is set to true, it performs a hard reset
	//of the thread.
	memset(RecBuf, 0, sizeof(RecBuf));
	memset(LogBuffer, 0, sizeof(LogBuffer));

	FileNameRequest.clear();
	FileNameLocal.clear();
	FileNameRequestBare.clear();

	MessageCountRec = 0;

	InternalIndex = 0;

	RecBytes = 0;
	TotalRecBytes = 0;

	SendBytes = 0;
	TotalSendBytes = 0;

	SendFile = false;
	DataPos = 0;
	FileSize = 0;
	CloseFileHandle();

	RecCurrentPos = 0;

	DataRemain = 0;
	ChunkNum = 0;
	Finished = true;  //Set to true to prevent some debug output errors for now
	Disconnect = false;

	inUse = false;
	Status = Status_Wait;

	LastAction = 0;
	ExpireDelay = 0;

	if(fullRestart == true)
	{
		ThreadID = 0;
		isExist = false;
		isActive = false;
	}
}

int HTTPDistribute :: InitThread(int instanceindex)
{
	//LogMessageL("[HTTP:%d] Address: %08X", instanceindex, this);
	//ResetValues(true);
	InternalIndex = instanceindex;

	//Set to active now because the thread might launch and quit
	//before it gets set to active later.
	//isActive = true;

	/*
	HANDLE h = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)HTTPDistributeThreadProc, this, 0, &ThreadID);
	if(h == NULL)
	{
		isActive = false;
		LogMessageL("Could not create thread.");
		return 1;
	}
	*/
	int r = Platform_CreateThread(0, (void*)HTTPDistributeThreadProc, this, &ThreadID);
	if(r == 0)
	{
		isActive = false;
		LogMessageL(LOG_CRITICAL, "Could not create thread.");
		return -1;
	}

	isExist = true;
	//Status = Status_Wait;

	//LogMessageL("[HTTP:%d] InitThread completed (ID: %d).", InternalIndex, ThreadID);

	//AdjustComponentCount(1);
	return 0;
}

void HTTPDistribute :: OnConnect(void)
{

}

void HTTPDistribute :: CloseFileHandle(void)
{
	if(DataFile != NULL)
	{
		fclose(DataFile);
		DataFile = NULL;
	}
}

void HTTPDistribute :: DisconnectClient(const char *debugReason)
{
	if(Finished == false || SendFile == true)
		LogMessageL(LOG_NORMAL, "[HTTP:%d] Disconnecting [%s] in progress for reason [%s]", InternalIndex, FileNameRequest.c_str(), debugReason);
	else
		LogMessageL(LOG_NORMAL, "[HTTP:%d] Disconnecting [%s] for reason [%s]", InternalIndex, FileNameRequest.c_str(), debugReason);

	Finished = true;
	SendFile = false;
	Status = Status_Wait;
	RecCurrentPos = 0;

	CloseFileHandle();

	sc.DisconnectClient();
	inUse = false;
}

bool HTTPDistribute :: QualifyDelete(void)
{
	//Only qualify if the thread has ended.
	if(isExist == false)
	{
		if(g_ServerTime - LastAction >= 20000)
			return true;
		return false;
	}

	//If not, we can trigger a thread to close by setting its loop to inactive.
	if(LastAction == 0)
	{
		g_Log.AddMessageFormat("[DEBUGHTTP] Status_Ready interrupt");
		LastAction = g_ServerTime;
	}
	if(ExpireDelay == 0)
	{
		g_Log.AddMessageFormat("[DEBUGHTTP] Status_Wait expire");
		ExpireDelay = g_Config.HTTPDeleteConnectedTime;
	}

	if(Status == Status_Wait)
	{
		//if(g_ServerTime - LastAction >= ExpireDelay)
		if(g_ServerTime - LastAction > (unsigned long)g_Config.HTTPDeleteDisconnectedTime)
		{
			isActive = false;
			DisconnectClient("QualifyDelete (wait mode)");
			LogMessageL(LOG_VERBOSE, "Flagging inactive thread (wait mode).");
			//return true;
		}
	}

	//Drop a hanging thread (marked as ready but has no recent activity)
	//Keep the thread alive until it properly expires.
	if(Status == Status_Ready)
	{
		if(g_ServerTime - LastAction > (unsigned long)g_Config.HTTPDeleteConnectedTime)
		{
			isActive = false;
			DisconnectClient("QualifyDelete (ready mode)");
			LogMessageL(LOG_VERBOSE, "Flagging active thread (ready mode).");
			//return true;
		}
	}
	return false;
}

//DWORD WINAPI HTTPDistributeThreadProc(LPVOID lpParam)
PLATFORM_THREADRETURN HTTPDistributeThreadProc(PLATFORM_THREADARGS lpParam)
{
	HTTPDistribute *controller = (HTTPDistribute*)lpParam;
	controller->isActive = true;
	controller->isExist = true;
	controller->inUse = true;
	AdjustComponentCount(1);

	//controller->LogMessageL("[HTTP:%d] Thread launched (ID: %d).", controller->InternalIndex, controller->ThreadID);

	controller->RunMainLoop();

	// Thread has been deactivated, shut it down
	controller->sc.ShutdownServer();

	controller->LogMessageL(LOG_ALWAYS, "Thread shut down.");

	controller->isExist = false;
	controller->LastAction = g_ServerTime;
	AdjustComponentCount(-1);

	PLATFORM_CLOSETHREAD(0);
	return 0;
}

void HTTPDistribute :: RunMainLoop(void)
{
	LastAction = g_ServerTime;
	while(isActive == true)
	{
		//The server will almost always be in the ready state, so it will be more
		//efficient to check for and process this status first.
		BEGINTRY
		{
		if(Status == Status_Ready)
		{
			int res = recv(sc.ClientSocket, &RecBuf[RecCurrentPos], sizeof(RecBuf) - RecCurrentPos - 1, 0);
			LastAction = g_ServerTime;
			if(res > 0)
			{
				uint newSize = RecCurrentPos + res;
				if(newSize >= sizeof(RecBuf) - 2)
				{
					//Buffer is full
					g_Log.AddMessageFormatW(MSG_ERROR, "[ERROR] HTTP:%d received data is too large for buffer", InternalIndex);
					DisconnectClient("HTTPDistribute::RunMainLoop too much data");
				}
				else
				{
					RecCurrentPos += res;
					RecBuf[RecCurrentPos] = 0;

					RecBytes = res;
					TotalRecBytes += res;

					CheckAutoResponse();
					MessageCountRec++;

					if(Finished == true)
					{
						RecCurrentPos = 0;
						CloseFileHandle();
					}
					if(Disconnect == true)
						DisconnectClient("HTTPDistribute::RunMainLoop disconnect flag set");
				}
			}
			else if(res == 0)
			{
				LogMessageL(LOG_VERBOSE, "Socket was closed normally.");
				DisconnectClient("HTTPDistribute::RunMainLoop socket closed normally");
			}
			else
			{
				LogMessageL(LOG_WARNING, "Socket was closed abnormally.");
				DisconnectClient("HTTPDistribute::RunMainLoop socket closed abnormally");
			}
		}
		else if(Status == Status_Wait)
		{
			if(Finished == false)
			{
				LogMessageL(LOG_CRITICAL, "REPORTING ERROR");
				PLATFORM_SLEEP(15);
			}
			PLATFORM_SLEEP(15);
		}
		else
		{
			LogMessageL(LOG_CRITICAL, "Unknown status: %d", Status);
			PLATFORM_SLEEP(5);
		}
		//Keep it from burning up unnecessary CPU cycles.
		PLATFORM_SLEEP(5);
		} //end try
		BEGINCATCH
		{
			g_Log.AddMessageFormat("[CRITICAL] HTTPDistributeThreadProc Exception caught");
			DisconnectClient("HTTPDistribute::RunMainLoop exception");
		}
	}
}


char * HTTPDistribute :: LogMessageL(int logLevel, const char *format, ...)
{
	if(logLevel > g_Config.LogLevelHTTPDistribute)
		return NULL;

	sprintf(LogBuffer, "[HTTP:%d] ", InternalIndex);
	int Pos = strlen(LogBuffer);

	va_list args;
	va_start (args, format);
	//vsnprintf(&LogBuffer[Pos], sizeof(LogBuffer) - 1 - Pos, format, args);
	Util::SafeFormatArg(&LogBuffer[Pos], sizeof(LogBuffer) - Pos, format, args);
	va_end (args);

	g_Log.AddMessage(LogBuffer);
	return LogBuffer;
}

char * ExtractHeader(char *incdata, MULTISTRING &extract)
{
	//Takes a string containing a number of lines (newline-terminated)
	//then breaks each line as delimited by spaces.
	//Places the results into an array of strings.
	//Each row will contain a name and value:
	//  extract[0][0] = first item
	//  extract[0][1] = second item, etc.
	//The scan ends when two newlines in a row are found.
	//Returns the position in the string array, where a binary data
	//section might begin.

	int len = strlen(incdata);
	int row = 0;
	char *curpos = incdata;
	char *end = NULL;
	vector<string> newItem;
	extract.push_back(newItem);
	int last = 0;
	do
	{
		end = strpbrk(curpos, " \r");
		if(end != NULL)
		{
			char temp = *end;
			*end = 0;
			extract[row].push_back(curpos);
			//printf("1Added: [%s]\n", curpos);
			*end = temp;

			if(temp == ' ')
			{
				curpos = end + 1;
				last = 1;
			}
			else if(temp == '\r')
			{
				curpos = end + 2;

				if(last == 2)
					break;

				extract.push_back(newItem);
				row++;
				last = 2;
			}
 		}
		else
		{
			extract[row].push_back(curpos);
			break;
		}
	} while(curpos - incdata < len);
	return curpos;
}

int ExtractPairs(char *incdata, MULTISTRING &extract)
{
	//Takes a null terminated string of key/value pairs in the format of:
	//  key=value&key=value&key=value...
	//and converts the results to an array of strings.
	//Each row will contain a name and value:
	//  extract[0][0] = name
	//  extract[0][1] = value
	//Returns the number of rows.
//	int len = strlen(incdata);
	int row = 0;
	char *curpos = incdata;
	char *end = NULL;
	vector<string> newItem;
	extract.push_back(newItem);
//	int last = 0;
	do
	{
		end = strpbrk(curpos, "=&");
		if(end != NULL)
		{
			char temp = *end;
			*end = 0;
			extract[row].push_back(curpos);
			*end = temp;

			if(temp == '=')
			{
				curpos = end + 1;
//				last = 1;
			}
			else if(temp == '&')
			{
				curpos = end + 1;
				extract.push_back(newItem);
				row++;
//				last = 2;
			}
		}
		else
		{
			extract[row].push_back(curpos);
			break;
		}
	} while(end != NULL);

	int rows = extract.size();


	/*
	g_Log.AddMessageFormat("ROWS: [%d]", rows);
	int a;
	for(a = 0; a < rows; a++)
	{
		g_Log.AddMessageFormat("PAIR: [%s, %s]", extract[a][0].c_str(), extract[a][1].c_str());
	}
	*/

	return rows;
}

char *SafeGet(MULTISTRING &extract, int index1, int index2)
{
	int rows = extract.size();
	if(index1 < 0 || index1 >= rows)
		return NULL;

	int strings = extract[index1].size();
	if(index2 < 0 || index2 >= strings)
		return NULL;

	return (char*)extract[index1][index2].c_str();
}

const char *GetValueOfKey(MULTISTRING &extract, const char *key)
{
	//Search the given list of rows for an entry with a certain key value
	//Then return a string pointer to its value.
	for(size_t i = 0; i < extract.size(); i++)
	{
		size_t vals = extract[i].size();
		if(vals >= 2)
		{
			if(extract[i][0].compare(key) == 0)
				return extract[i][1].c_str();
		}
	}
	//g_Log.AddMessageFormat("Not found: %s", key);
	return NULL;
}

void HTTPDistribute :: CheckAutoResponse(void)
{
	if(SendFile == false)
		ProcessHTTPRequest();

	if(SendFile == true)
		SendNextFileChunk();
}

void HTTPDistribute :: SendNextFileChunk(void)
{
	if(Finished == true)
	{
		LogMessageL(LOG_WARNING, "[WARNING] SendNextFileChunk() on Finish (Chunk: %d)", ChunkNum);
		return;
	}

	long ToRead = 0;
	int rs = 0;

	while(DataRemain > 0)
	{
		LastAction = g_ServerTime;
		ToRead = sizeof(SendBuf);
		if(ToRead > DataRemain)
			ToRead = DataRemain;

		rs = fread(SendBuf, ToRead, 1, DataFile);
		if(rs == 0)
		{
			LogMessageL(LOG_ERROR, "Failed to read data from file [%s]", FileNameLocal.c_str());
			DataRemain = 0;
		}
		else
		{
			int res = sc.AttemptSend(SendBuf, ToRead);
			if(res == -1)
			{
				LogMessageL(LOG_ERROR, "Failed to send data from file [%s] on chunk %d.", FileNameLocal.c_str(), ChunkNum);
				DataRemain = 0;
			}
			else
			{
				TotalSendBytes += res;
			}
		}
		ChunkNum++;
		DataRemain -= ToRead;
	}

	CloseFileHandle();
	SendFile = false;
	Finished = true;
}

void HTTPDistribute :: LogInvalidRequest(void)
{
	LogMessageL(LOG_ERROR, "[ERROR] Invalid request of %d bytes from address: %d.%d.%d.%d",
		RecBytes,
		(unsigned char)remotename.sa_data[2],
		(unsigned char)remotename.sa_data[3],
		(unsigned char)remotename.sa_data[4],
		(unsigned char)remotename.sa_data[5]
	);
}

void HTTPDistribute :: ProcessHTTPRequest(void)
{
	//g_Log.AddMessageFormat("[%s]", RecBuf);

	MULTISTRING extract;
	//g_Log.AddMessageFormat("[HTTP:%d] ExtractHeader [%s]", InternalIndex, RecBuf);
	char *pos = ExtractHeader(RecBuf, extract);
	//g_Log.AddMessageFormat("[HTTP:%d] ExtractHeader done", InternalIndex);

	if(extract.size() == 0)
	{
		LogMessageL(LOG_WARNING, "Invalid Header (no rows)");
		LogInvalidRequest();
		Finished = true;
		return;
	}
	if(extract[0].size() == 0)
	{
		LogMessageL(LOG_WARNING, "Invalid Header (no command");
		LogInvalidRequest();
		Finished = true;
		return;
	}

	const char *sizeStr = GetValueOfKey(extract, "Content-Length:");
	if(sizeStr != NULL)
	{
		int expectedDataSize = atoi(sizeStr);
		int hasSize = (RecCurrentPos - (pos - RecBuf));
		if(expectedDataSize != hasSize)
		{
			LogMessageL(LOG_WARNING, "Data size mismatch: expected: %d, has: %d\n", expectedDataSize, hasSize);
			Finished = false;
			return;
		}
		else
			LogMessageL(LOG_VERBOSE, "Content length PASSED (required: %d)\n", expectedDataSize);
	}

	if(extract[0][0].compare("GET") == 0)
	{
		HandleHTTP_GET(pos, extract);
	}
	else if(extract[0][0].compare("POST") == 0)
	{
		HandleHTTP_POST(pos, extract);
	}
	else
	{
		LogInvalidRequest();
		/*
		sockaddr_in* sa = (sockaddr_in*) &sc.acceptData;
		LogMessageL("Connection Port: %d, Address: %d.%d.%d.%d", sa->sin_port,
			sa->sin_addr.S_un.S_un_b.s_b1,
			sa->sin_addr.S_un.S_un_b.s_b2,
			sa->sin_addr.S_un.S_un_b.s_b3,
			sa->sin_addr.S_un.S_un_b.s_b4
			);
		*/
		Finished = true;
		return;
	}
}

bool HTTPDistribute :: NeedFile(bool verifyExist, std::string &checksum)
{
	if(verifyExist == false)
		return false;

	if(checksum.size() == 0)
		return false;

	if(g_FileChecksum.MatchChecksum(FileNameAsset, checksum) == true)
		return false;

	return true;
}


void HTTPDistribute :: HandleHTTP_GET(char *dataStart, MULTISTRING &header)
{
//	Sleep(3500);   //Was debugging some stuff with simulated lag

	//An HTTP request by the client looks like this:
	/*  ---- In 0.8.6 ----
		GET /Release/Current/EarthEternal.car HTTP/1.1
		Host: localhost
		User-Agent: ire-3dply(VERSION)
		If-None-Match: "48a754eb7395dcf3603e2d0d9221a7c5"
	*/
	//LogMessageL("GET MESSAGE [%s]", RecBuf);
	FileNameRequest.clear();
	FileNameAsset.clear();
	bool verifyExist = false;
	int customError = 0;
	std::string checksum;
	for(size_t i = 0; i < header.size(); i++)
	{
		//All the rows we need to examine have at least 2 elements in them.
		if(header[i].size() < 2)
			continue;
		if(header[i][0].compare("GET") == 0)
		{
			FileNameRequest = header[i][1];
			Util::RemoveStringsFrom("~", FileNameRequest);
			Util::RemoveStringsFrom("*", FileNameRequest);
			Util::RemoveStringsFrom("..", FileNameRequest);
	
			//Remove the last token " HTTP/1.1"
			size_t pos = FileNameRequest.rfind(" ");
			if(pos != std::string::npos)
				FileNameRequest.erase(pos, FileNameRequest.size());

			PrepareFileNames();
			LogMessageL(LOG_VERBOSE, "Requested file [%s]", FileNameRequest.c_str());
		}
		else if(header[i][0].compare("If-None-Match:") == 0)
		{
			verifyExist = true;
			checksum = header[i][1];
		}
		else if(header[i][0].compare("If-Modified-Since:") == 0)
		{
			verifyExist = true;
			checksum = header[i][1];
		}
	}

	customError = 0;
	bool need = false;
	bool api = false;
	if(IsEmptyFileRequest() == false)
	{
		// An API?
		if(Util::HasBeginning(FileNameRequest, "/api/"))
		{
			api = true;
		}
		else
		{

			//We don't have the file, so prepare it.
			if(verifyExist == false)
			{
				customError = OpenLocalFileName();
				need = true;
			}
			else
			{
				//If the checksum does not match, we need to update.
				if(g_FileChecksum.MatchChecksum(FileNameAsset, checksum) == false)
				{
					customError = OpenLocalFileName();
					need = true;
					//LogMessageL("NEED FILE");
				}
			}
		}
	}
	else
	{
		customError = ERROR_CUSTOM404;
		//Disconnect = true;
	}

	int wpos = 0;

	//No error, has a file prepared to upload.
	if(customError == 0)
	{
		if(api)
		{
			wpos = FillAPI();
		}
		else
		{
			if(need == true)
			{
				wpos = FillClientNeeds();
				SendFile = true;
			}
			else
				wpos = FillClientHas();
		}
	}
	else
	{
		wpos = FillErrorMessage(customError);
	}

	//LogMessageL("Sending: [%s]", SendBuf);

	int res = sc.AttemptSend(SendBuf, wpos);
	if(res >= 0)
	{
		TotalSendBytes += res;
	}
	else
	{
		LogMessageL(LOG_WARNING, "Failed to send response for file [%s]", FileNameLocal.c_str());
		SendFile = false;
		Finished = true;
	}

	if(FileSize == 0 || SendFile == false)
		Finished = true;
	else
		Finished = false;
}

void HTTPDistribute :: HandleHTTP_POST(char *dataStart, MULTISTRING &header)
{
	ThreadRequest threadReq;
	threadReq.status = ThreadRequest::STATUS_WAITMAIN;
	g_SimulatorManager.RegisterAction(&threadReq);
	bool res = threadReq.WaitForStatus(ThreadRequest::STATUS_WAITWORK, 1, ThreadRequest::DEFAULT_WAIT_TIME);
	if(res == false)
	{
		threadReq.status = ThreadRequest::STATUS_COMPLETE;
		g_SimulatorManager.UnregisterAction(&threadReq);
		Finished = true;
		Disconnect = true;
		return;
	}

	//LogMessageL("POST MESSAGE [%s]", RecBuf);
	for(size_t a = 0; a < header.size(); a++)
	{
		//All the rows we need to examine have at least 2 elements in them.
		if(header[a].size() < 2)
			continue;
		if(header[a][0].compare("POST") == 0)
		{
			if(header[a][1].compare("/remoteaction") == 0)
			{
				MULTISTRING params;
				ExtractPairs(dataStart, params);

				ReportBuffer report(65535, ReportBuffer::NEWLINE_WEB);
				int r = RunRemoteAction(report, header, params);

				int wpos = 0;
				wpos += sprintf(&SendBuf[wpos], "HTTP/1.1 200 OK\r\n");
				wpos += sprintf(&SendBuf[wpos], "Content-Type: text/html\r\n");
				wpos += sprintf(&SendBuf[wpos], "\r\n");

				switch(r)
				{
				case REMOTE_COMPLETE:
					wpos += sprintf(&SendBuf[wpos], "<b>Operation successful.</b>");
					break;
				case REMOTE_FAILED:
					wpos += sprintf(&SendBuf[wpos], "<b>Operation failed.</b>");
					break;
				case REMOTE_AUTHFAILED:
					wpos += sprintf(&SendBuf[wpos], "<b>Permission denied.</b>");
					break;
				case REMOTE_INVALIDPOST:
					wpos += sprintf(&SendBuf[wpos], "<b>Invalid or malformed request.</b>");
					break;
				case REMOTE_HANDLER:
					wpos += sprintf(&SendBuf[wpos], "<b>No handler is capable of processing that action.</b>");
					break;
				}

				TotalSendBytes += sc.AttemptSend(SendBuf, wpos);

				if(r == REMOTE_REPORT)
				{
					TotalSendBytes += sc.AttemptSend(report.getData(), report.getLength());
				}
			}
			else if(header[a][1].compare("/newaccount") == 0)
			{
				MULTISTRING params;
				ExtractPairs(dataStart, params);

				int r = RunAccountCreation(params);

				int wpos = 0;
				wpos += sprintf(&SendBuf[wpos], "HTTP/1.1 200 OK\r\n");
				wpos += sprintf(&SendBuf[wpos], "Content-Type: text/html\r\n");
				wpos += sprintf(&SendBuf[wpos], "\r\n");
				wpos += sprintf(&SendBuf[wpos], "%s", g_AccountManager.GetErrorMessage(r));
				TotalSendBytes += sc.AttemptSend(SendBuf, wpos);
			}
			else if(header[a][1].compare("/resetpassword") == 0)
			{
				MULTISTRING params;
				ExtractPairs(dataStart, params);

				int r = RunPasswordReset(params);

				int wpos = 0;
				wpos += sprintf(&SendBuf[wpos], "HTTP/1.1 200 OK\r\n");
				wpos += sprintf(&SendBuf[wpos], "Content-Type: text/html\r\n");
				wpos += sprintf(&SendBuf[wpos], "\r\n");
				wpos += sprintf(&SendBuf[wpos], "%s", g_AccountManager.GetErrorMessage(r));
				TotalSendBytes += sc.AttemptSend(SendBuf, wpos);
			}
			else if(header[a][1].compare("/accountrecover") == 0)
			{
				MULTISTRING params;
				ExtractPairs(dataStart, params);

				int r = RunAccountRecover(params);

				int wpos = 0;
				wpos += sprintf(&SendBuf[wpos], "HTTP/1.1 200 OK\r\n");
				wpos += sprintf(&SendBuf[wpos], "Content-Type: text/html\r\n");
				wpos += sprintf(&SendBuf[wpos], "\r\n");
				wpos += sprintf(&SendBuf[wpos], "%s", g_AccountManager.GetErrorMessage(r));
				TotalSendBytes += sc.AttemptSend(SendBuf, wpos);
			}
			else if(header[a][1].compare("/bugreport") == 0)
			{
				FILE *output = fopen("BugReport.log", "a");
				if(output != NULL)
				{
					fprintf(output, "%s\r\n\r\n\r\n", dataStart);
					fclose(output);
				}
				int wpos = 0;
				wpos += sprintf(&SendBuf[wpos], "HTTP/1.1 200 OK\r\n");
				wpos += sprintf(&SendBuf[wpos], "Content-Type: text/html\r\n");
				wpos += sprintf(&SendBuf[wpos], "\r\n");
				wpos += sprintf(&SendBuf[wpos], "Completed");
				TotalSendBytes += sc.AttemptSend(SendBuf, wpos);
			}
			else
			{
				g_Log.AddMessageFormat("[WARNING] POST file [%s] is not accepted.", header[a][1].c_str());
			}
		}
	}

	threadReq.status = ThreadRequest::STATUS_COMPLETE;
	g_SimulatorManager.UnregisterAction(&threadReq);

	Finished = true;
	Disconnect = true;
}

bool HTTPDistribute :: IsEmptyFileRequest(void)
{
	if(FileNameRequest.compare("/") == 0)
		return true;

	return false;
}

void HTTPDistribute :: PrepareFileNames(void)
{
	//Resolve various file name substrings extracted from the full HTTP GET resource

	//Find the first slash following the domain
	const char *header = "http://";
	size_t startPos = 0;

	startPos = FileNameRequest.find(header);
	if(startPos != std::string::npos)
	{
		startPos += strlen(header);
	}
	else
	{
		startPos = 0;
	}
	
	//Advance to the beginning of the file name part of the string
	size_t pos = FileNameRequest.find("/", startPos);
	if(pos != std::string::npos)
	{
		startPos = pos;
	}

	//The file name should begin with a slash, or else the generated paths may not work.
	//Refer to HTTPBaseFolder in ServerConfigs.txt
	FileNameAsset = FileNameRequest.substr(startPos, FileNameRequest.size());

	FileNameLocal = g_HTTPBaseFolder;
	FileNameLocal.append(FileNameAsset);
	Platform::FixPaths(FileNameLocal);

	//A separate extract for just the file name, excluding the path
	pos = FileNameRequest.rfind("/");
	if(pos == std::string::npos)
		pos = 0;
	else
		pos++;
	FileNameRequestBare = FileNameRequest.substr(pos, FileNameRequest.size());
}

int HTTPDistribute :: OpenLocalFileName(void)
{
	DataFile = fopen(FileNameLocal.c_str(), "rb");
	if(DataFile == NULL)
	{
		LogMessageL(LOG_ERROR, "Cannot find file [%s] accessed by [%d.%d.%d.%d].", FileNameLocal.c_str(),
			(unsigned char)remotename.sa_data[2],
			(unsigned char)remotename.sa_data[3],
			(unsigned char)remotename.sa_data[4],
			(unsigned char)remotename.sa_data[5]);

		SendFile = false;
		if(FileNameLocal.find(".car") != std::string::npos)
			return ERROR_STRICT404;
		return ERROR_CUSTOM404;
	}
	else
	{
		fseek(DataFile, 0, SEEK_END);
		FileSize = ftell(DataFile);
		fseek(DataFile, 0, SEEK_SET);
		if(FileSize > 0)
			SendFile = true;

		DataRemain = FileSize;
		ChunkNum = 0;
	}
	return 0;
}

int HTTPDistribute :: FillErrorMessage(int errCode)
{
	//Fall back to strict 404 if a custom message isn't set.
	if(errCode == ERROR_CUSTOM404 && g_HTTP404Header.size() == 0)
		errCode = ERROR_STRICT404;

	int wpos = 0;
	if(errCode == ERROR_CUSTOM404)
	{
		wpos += sprintf(&SendBuf[wpos], "%s\r\n", g_HTTP404Header.c_str());

		if(g_HTTP404Redirect != 0)  //Redirect will skip the payload.  Probably not necessary, but just in case.
		{
			wpos += sprintf(&SendBuf[wpos], "Content-Length: %d\r\n", (int)g_HTTP404Message.size());
			wpos += sprintf(&SendBuf[wpos], "Content-Type: text/html\r\n\r\n");
			wpos += sprintf(&SendBuf[wpos], "%s", g_HTTP404Message.c_str());
		}
		else
		{
			wpos += sprintf(&SendBuf[wpos], "Content-Length: 0\r\n");
			wpos += sprintf(&SendBuf[wpos], "Content-Type: text/html\r\n\r\n");
			wpos += sprintf(&SendBuf[wpos], "\r\n");
		}
	}
	else if(errCode == ERROR_STRICT404)
	{
		wpos += sprintf(&SendBuf[wpos], "HTTP/1.1 404 Not Found\r\n");
		wpos += sprintf(&SendBuf[wpos], "Content-Length: %d\r\n", (int)g_HTTP404Message.size());
		wpos += sprintf(&SendBuf[wpos], "Content-Type: text/html\r\n\r\n");
		wpos += sprintf(&SendBuf[wpos], "%s", g_HTTP404Message.c_str());
	}
	return wpos;
}

int HTTPDistribute :: FillClientHas(void)
{
	//Prepares a response header when the client already has an up-to-date file.
	int wpos = 0;
	wpos += sprintf(&SendBuf[wpos], "HTTP/1.1 304 Not Modified\r\n");
	wpos += sprintf(&SendBuf[wpos], "Date: Tue, 1 Nov 2011 00:00:00 GMT\r\n");
	wpos += sprintf(&SendBuf[wpos], "Expires: Tue, 1 Nov 2011 00:00:00 GMT\r\n");
	wpos += sprintf(&SendBuf[wpos], "Last-Modified: Tue, 1 Nov 2011 00:00:00 GMT\r\n");
	wpos += sprintf(&SendBuf[wpos], "Cache-Control: max-age=0\r\n");
	wpos += sprintf(&SendBuf[wpos], "\r\n");
	return wpos;
}

int HTTPDistribute :: FillAPI(void)
{
	int wpos = 0;
	string response;
	char buf[256];
	int no = 0;
	if(Util::HasEnding(FileNameRequest, "/who")) {
		response += "{ ";
		SIMULATOR_IT it;
		for(it = Simulator.begin(); it != Simulator.end(); ++it)
		{
			if(it->isConnected == true && it->ProtocolState == 1 && it->LoadStage == SimulatorThread::LOADSTAGE_GAMEPLAY)
			{
				if(it->IsGMInvisible() == true)  //Hide GM Invisibility from the list.
					continue;
				CharacterData *cd = it->pld.charPtr;
				ZoneDefInfo *zd = g_ZoneDefManager.GetPointerByID(it->pld.CurrentZoneID);
				if(cd != NULL && zd != NULL)
				{
					no++;
					Util::SafeFormat(buf, sizeof(buf), "\"%s\" : { \"zone\": \"%s\", \"shard\": \"%s\" }%s", cd->cdef.css.display_name, zd->mName.c_str(), zd->mShardName.c_str(), no > 1 ? "," : "");
					response += buf;
				}
			}
		}
		response += " }";
	}

	wpos += sprintf(&SendBuf[wpos], "HTTP/1.1 200 OK\r\n");
	wpos += sprintf(&SendBuf[wpos], "Content-Type: application/json\r\n");
	wpos += sprintf(&SendBuf[wpos], "Content-Length: %d\r\n", (int)response.size());
	wpos += sprintf(&SendBuf[wpos], "\r\n");
	wpos += sprintf(&SendBuf[wpos], "%s", response.c_str());

	return wpos;
}

int HTTPDistribute :: FillClientNeeds(void)
{
	//Prepares a response header when the client needs to download a file.

	int wpos = 0;
	size_t findCar = FileNameRequest.find(".car");
	wpos += sprintf(&SendBuf[wpos], "HTTP/1.1 200 OK\r\n");
	if(findCar != std::string::npos)
		wpos += sprintf(&SendBuf[wpos], "Content-Type: application/octet-stream\r\n");
	else
		wpos += sprintf(&SendBuf[wpos], "Content-Type: text/html\r\n");

	wpos += sprintf(&SendBuf[wpos], "Content-Length: %d\r\n", (int)FileSize);
	wpos += sprintf(&SendBuf[wpos], "Accept-Ranges: bytes\r\n");

	if(findCar != std::string::npos)
		wpos += sprintf(&SendBuf[wpos], "Content-Disposition: attachment; filename=\"%s\";\r\n", FileNameRequestBare.c_str());

	wpos += sprintf(&SendBuf[wpos], "Last-Modified: Wed, 31 Dec 2010 10:00:00 GMT\r\n\r\n");

	return wpos;
}

HTTPDistributeManager :: HTTPDistributeManager()
{
	mDebugThreadCollision = false;
	mNextInactiveScan = 0;
	mDroppedConnections = 0;
	mTotalSendBytes = 0;
	mTotalRecBytes = 0;
}

HTTPDistributeManager :: ~HTTPDistributeManager()
{
}

HTTPDistribute* HTTPDistributeManager :: GetNewDistributeSlot(void)
{
	http_cs.Enter("HTTPDistributeManager::GetNewDistributeSlot");
	mDebugThreadCollision = true;

	size_t origSize = mDistributeList.size();

	HTTPDistribute *ptr = NULL;

	HTTPDistribute newSlot;
	mDistributeList.push_back(newSlot);
	size_t newSize = mDistributeList.size();
	if(newSize == 0)
	{
		g_Log.AddMessageFormat("[CRITICAL] GetNewDistributeSlot added slot but zero size.");
	}
	else
	{
		if(mDistributeList.size() != origSize)
			ptr = &mDistributeList.back();
	}

	mDebugThreadCollision = false;
	http_cs.Leave();

	return ptr;
}

void HTTPDistributeManager :: ShutDown(void)
{
	ITERATOR it;
	for(it = mDistributeList.begin(); it != mDistributeList.end(); ++it)
	{
		if(it->isExist == true)
		{
			it->isActive = false;
			it->DisconnectClient("HTTPDistributeManager::ShutDown");
		}
	}
}

int HTTPDistributeManager :: GetSlotCount(void)
{
	return static_cast<int>(mDistributeList.size());
}

void HTTPDistributeManager :: CheckInactiveDistribute(void)
{
	if(mNextInactiveScan > g_ServerTime)
		return;

	mNextInactiveScan = g_ServerTime + g_Config.HTTPDeleteRecheckDelay;

	http_cs.Enter("HTTPDistributeManager::CheckInactiveThreads");
	mDebugThreadCollision = true;

	ITERATOR hdit = mDistributeList.begin();
	while(hdit != mDistributeList.end())
	{
		if(hdit->QualifyDelete() == true)
		{
			/*
			sockaddr *remotename = &hdit->remotename;
			g_Log.AddMessageFormat("[ERROR] HTTP:%d was forced to shut down for inactivity.", hdit->InternalIndex);
			g_Log.AddMessageFormat("Stuck on IP address: %d.%d.%d.%d",
				(unsigned char)remotename->sa_data[2],
				(unsigned char)remotename->sa_data[3],
				(unsigned char)remotename->sa_data[4],
				(unsigned char)remotename->sa_data[5]);
			hdit->isActive = false;
			hdit->Status = Status_Wait;
			hdit->Shutdown();
			
			Debug_DroppedConnections++;
			*/

			mTotalRecBytes += hdit->TotalRecBytes;
			mTotalSendBytes += hdit->TotalSendBytes;
			
			hdit->DisconnectClient("HTTPDistributeManager::CheckInactiveThreads");

			hdit->LogMessageL(LOG_VERBOSE, "Erasing HTTP:%d", hdit->InternalIndex);
			mDistributeList.erase(hdit++);
			mDroppedConnections++;
		}
		else
			++hdit;
	}

	mDebugThreadCollision = false;
	http_cs.Leave();
}
