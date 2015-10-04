/*	NOTE: enable the following define in both main.cpp and MainWindow.cpp
	to enable the Windows GUI.

	#define USE_WINDOWS_GUI
*/

#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// The components file is a central place where platform-specific defines 
// and alternative routines are specified.
#include "Components.h"

#ifdef USE_WINDOWS_GUI

/////////////////////////////////////////////////
// Main file section begin here.

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

struct WindowControl
{
	int ControlIndex;
	int X;
	int Y;
	int Width;
	int Height;
	const char *ClassName;
	const char *WindowName;
	DWORD Style;
	DWORD ExStyle;
	DWORD Page;
};

struct ToolControl
{
	const char *ToolName;
	DWORD CallID;
	const char *ItemName[5];
	const char *DefaultString[5];
	DWORD ItemType[5];
};

enum ToolControlID
{
	TCID_Chat,
	TCID_CueEffect,
	TCID_CueEffectTarget,
};

enum ToolItemTypeEnum
{
	TI_None = 0,
	TI_String,
	TI_Byte,
	TI_Short,
	TI_Integer,
	TI_Float
};

struct ToolFunctionCallData
{
	const char *ListName;
	void (*FunctionPointer)(void);
};
extern ToolFunctionCallData toolFunction[];


enum MainWindowControlSetEnum
{
	MWCS_Static_Status,
	MWCS_Edit_Status,
	MWCS_Button_StatusClear,

	//MWCS_Static_PortLabel,
	//MWCS_Edit_IPEdit,
	//MWCS_Edit_PortEdit,
	//MWCS_Button_ClientStart,

	MWCS_Button_SaveItems,
	MWCS_Button_DisassembleScripts,
	MWCS_Button_TestCrash,
	MWCS_Button_VariableDump,
	MWCS_Button_FlushLog,
	MWCS_Button_ReloadQuests,
	MWCS_Button_ReloadChecksum,
	MWCS_Button_ReloadAbilities,
	MWCS_Button_DamageTest,
	MWCS_Static_Version,


	MWCS_Static_ToolText0,
	MWCS_Static_ToolText1,
	MWCS_Static_ToolText2,
	MWCS_Static_ToolText3,
	MWCS_Static_ToolText4,

	MWCS_Edit_ToolEdit0,
	MWCS_Edit_ToolEdit1,
	MWCS_Edit_ToolEdit2,
	MWCS_Edit_ToolEdit3,
	MWCS_Edit_ToolEdit4,

	MWCS_Button_UseTool,

	MWCS_Static_ToolLabel,
	MWCS_ComboBox_ToolList,

	MWCS_Tab_Main,

	MWCS_Edit_SimulatorID,

	//MWCS_Edit_Convert,
	//MWCS_Button_ToANSI,
	//MWCS_Button_ToUTF,

	MWCS_Button_RefreshStats,
	MWCS_Button_RefreshProps,
	MWCS_Button_RefreshPlayers,
	MWCS_Button_RefreshPref,
	MWCS_Button_RefreshInventory,
	MWCS_Button_RefreshPointer,
	MWCS_Button_RefreshInstance,
	MWCS_Button_RefreshSpawnPackages,
	MWCS_Button_RefreshSpawn,
	MWCS_Button_RefreshTime,
	MWCS_Button_RefreshParty,
	MWCS_Button_RefreshAIScripts,
	MWCS_Button_RefreshMods,
	MWCS_Button_RefreshHate,
	MWCS_Button_RefreshProfiler,
	MWCS_Button_RefreshQuests,
	MWCS_Button_RefreshChar,
	MWCS_Edit_Stats,

	MWCS_Button_INITIATEFUN,


	MWCS_Button_ClearChat,
	MWCS_Edit_ChatBox,
	MWCS_Edit_ChatName,
	MWCS_Edit_ChatChannel,
	MWCS_Edit_ChatMessage,
	MWCS_ComboBox_ChatChannel,
	MWCS_Static_ChatName,
	MWCS_Static_ChatChannel,
	MWCS_Static_ChatMessage,
	MWCS_Button_ChatSend,

	MWCS_MAX
};

extern char szMainWindow[];
extern HWND MainWindowControlSet[];
extern WindowControl MainWindowControlDef[];
extern ToolControl ToolControlDef[];
extern const char *ToolItemType[6];

extern int MainWindowSizeX;
extern int MainWindowSizeY;
extern int WindowPosX;
extern int WindowPosY;

extern int VisPage;

extern HWND hwndMainWindow;
extern bool bMainWindowStatus;

extern int ActiveComponents;
extern HFONT StandardFont;
extern HINSTANCE g_hInst;
extern char WindowTextBuffer[32767];

//FAR PASCAL
bool __stdcall InitMainWindow(HINSTANCE hInstance, int nCmdShow);
long __stdcall MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void FreeWindow(void);
void ProcessControls(WPARAM wParam);
void ClearStatusWindowText(void);
void SetWindowTextToValue(HWND hwnd, int value);

void ChangePage(int NewPage);
void ChangeToolIndex(void);      //Changes the control sets (box labels, edit values) when a preset tool item is selected from the dropdown box on the Tools page
void ChangeChatChannelListIndex(void);

void Event_Click_Button_ServerStart(void);
void Event_Click_Button_StatusClear(void);
void Event_Click_Button_SaveHistory(void);
void Event_Click_Button_ClearHistory(void);
void Event_Click_Button_UseTool(void);
void Event_Click_Button_RefreshStats(void);
void Event_Click_Button_RefreshProps(void);
void Event_Click_Button_RefreshPlayers(void);
void Event_Click_Button_RefreshPrefs(void);
void Event_Click_Button_RefreshInventory(void);
void Event_Click_Button_RefreshInstance(void);
void Event_Click_Button_RefreshPointer(void);
void Event_Click_Button_RefreshSpawnPackages(void);
void Event_Click_Button_RefreshSpawn(void);
void Event_Click_Button_RefreshTime(void);
void Event_Click_Button_RefreshParty(void);
void Event_Click_Button_RefreshAIScripts(void);
void Event_Click_Button_RefreshMods(void);
void Event_Click_Button_RefreshHate(void);
void Event_Click_Button_RefreshProfiler(void);
void Event_Click_Button_RefreshQuests(void);
void Event_Click_Button_RefreshChar(void);
void Event_Click_Button_ClearChat(void);
void Event_Click_Button_ToolClearProps(void);
void Event_Click_Button_ChatSend(void);
void Event_Click_Button_SplitArray(void);
void Event_Click_Button_SaveItems(void);
void Event_Click_Button_DisassembleScripts(void);
void Event_Click_Button_TestCrash(void);
void Event_Click_Button_VariableDump(void);
void Event_Click_Button_FlushLog(void);
void Event_Click_Button_ReloadQuests(void);
void Event_Click_Button_ReloadChecksum(void);
void Event_Click_Button_ReloadAbilities(void);
void Event_Click_Button_DamageTest(void);
void RefreshChatBox(void);

class SimulatorThread;
SimulatorThread* GetDebugSimulator(void);


//void Event_Click_Button_ToANSI(void);
//void Event_Click_Button_ToUTF(void);

//extern char LogBuffer[4096];
extern char LogBuffer[4096];

extern int StatusWindowPending;
char *LogMessage(char *format, ...);

enum RefreshEnum
{
	Refresh_None           = 0,
	Refresh_RefreshStats   = 1,
	Refresh_RefreshProps   = 2,
	Refresh_RefreshPlayers = 3,
};

extern int LastRefresh;

#define PAGEMAIN   0x00000001
#define PAGETOOL   0x00000002
#define PAGESTAT   0x00000004
#define PAGECHAT   0x00000008
#define PAGEALL    0xFFFFFFFF

struct ChatChannelListDef
{
	const char *channelName;
	const char *channelStr;
};

extern const int NumChatChannelListItems;
extern ChatChannelListDef ChatChannelList[];

#endif //MAINWINDOW_H

#endif //USE_WINDOWS_GUI