#include "Components.h"

// COMPLETELY IGNORE THE CONTENTS OF THIS FILE IF USING A CONSOLE
#ifdef USE_WINDOWS_GUI

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#pragma comment(lib, "Comctl32.lib")
#include "MainWindow.h"
#include "StringList.h"
#include "Chat.h"  //For the chat tab

int StatusWindowPending = 0;

int MessagePending = 0;
//bool ChatSendFocus = false;

extern char szMainWindow[] = "Server Emulator";

int LastRefresh = Refresh_None;

WNDPROC wpMainProc = NULL;

HWND MainWindowControlSet[MWCS_MAX];
WindowControl MainWindowControlDef[MWCS_MAX] = {
	{MWCS_Static_Status, 20,  25, 50, 20, "STATIC", "Status:",  WS_CHILD | WS_VISIBLE, NULL, PAGEMAIN },
	{MWCS_Edit_Status,   20,  50, 460, 310, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, WS_EX_CLIENTEDGE, PAGEMAIN },
	{MWCS_Button_StatusClear, 500, 260, 80, 18, "BUTTON", "Clear Status", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGEMAIN },

	//MWCS_Static_PortLabel, 500, 25, 50, 20, "STATIC", "Port:",  WS_CHILD | WS_VISIBLE, NULL, PAGEMAIN,
	//MWCS_Edit_IPEdit,     500, 50, 80, 20, "EDIT", "192.168.1.46", WS_CHILD | WS_VISIBLE | WS_HSCROLL | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE, PAGEMAIN,
	//MWCS_Edit_PortEdit,   500, 80, 50, 20, "EDIT", "4242", WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE, PAGEMAIN,
	//MWCS_Button_ClientStart, 500, 140, 80, 18, "BUTTON", "Start Client", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGEMAIN,

	{MWCS_Button_SaveItems, 480, 120, 100, 18, "BUTTON", "Save Items", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGETOOL },
	{MWCS_Button_DisassembleScripts, 480, 140, 100, 18, "BUTTON", "Disassemble Scripts", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGETOOL },
	{MWCS_Button_TestCrash, 480, 160, 100, 18, "BUTTON", "Test Crash", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGETOOL },
	{MWCS_Button_VariableDump, 480, 180, 100, 18, "BUTTON", "Variable Dump", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGETOOL },
	{MWCS_Button_FlushLog, 480, 200, 100, 18, "BUTTON", "Flush Log File", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGETOOL },
	{MWCS_Button_ReloadQuests, 480, 240, 100, 18, "BUTTON", "Reload Quests", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGETOOL },
	{MWCS_Button_ReloadChecksum, 480, 260, 100, 18, "BUTTON", "Reload HTTP Checksums", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGETOOL },
	{MWCS_Button_ReloadAbilities, 480, 280, 100, 18, "BUTTON", "Reload Abilities", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGETOOL },
	{MWCS_Button_DamageTest, 480, 300, 100, 18, "BUTTON", "Damage Test", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGETOOL },

	{MWCS_Static_Version, 450, 350, 140, 20, "STATIC", VersionString,  WS_CHILD | WS_VISIBLE, NULL, PAGETOOL },
	
	//Continuing page 2

	{MWCS_Static_ToolText0, 250,  80, 200, 20, "STATIC", "Arbitrary Value 0:",  WS_CHILD | WS_VISIBLE, NULL, PAGETOOL },
	{MWCS_Static_ToolText1, 250, 100, 200, 20, "STATIC", "Arbitrary Value 1:",  WS_CHILD | WS_VISIBLE, NULL, PAGETOOL },
	{MWCS_Static_ToolText2, 250, 120, 200, 20, "STATIC", "Arbitrary Value 2:",  WS_CHILD | WS_VISIBLE, NULL, PAGETOOL },
	{MWCS_Static_ToolText3, 250, 140, 200, 20, "STATIC", "Arbitrary Value 3:",  WS_CHILD | WS_VISIBLE, NULL, PAGETOOL },
	{MWCS_Static_ToolText4, 250, 160, 200, 20, "STATIC", "Arbitrary Value 4:",  WS_CHILD | WS_VISIBLE, NULL, PAGETOOL },

	{MWCS_Edit_ToolEdit0,  40,  80, 200, 20, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE, PAGETOOL },
	{MWCS_Edit_ToolEdit1,  40, 100, 200, 20, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE, PAGETOOL },
	{MWCS_Edit_ToolEdit2,  40, 120, 200, 20, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE, PAGETOOL },
	{MWCS_Edit_ToolEdit3,  40, 140, 200, 20, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE, PAGETOOL },
	{MWCS_Edit_ToolEdit4,  40, 160, 200, 20, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE, PAGETOOL },

	{MWCS_Button_UseTool,   260, 200, 80, 18, "BUTTON", "Use Tool", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGETOOL },

	{MWCS_Static_ToolLabel,  40, 185,  100,  20, "STATIC", "Preset Tools:",  WS_CHILD | WS_VISIBLE, NULL, PAGETOOL },
	{MWCS_ComboBox_ToolList, 40, 200,  180, 100, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_DISABLENOSCROLL, NULL, PAGETOOL },

	{MWCS_Tab_Main,  0, 0, 590, 370, WC_TABCONTROL, "", WS_CHILD | WS_VISIBLE, NULL, PAGEALL },


	{MWCS_Edit_SimulatorID, 500, 25,  70, 20, "EDIT", "0", WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE, PAGEMAIN },

	{MWCS_Button_RefreshStats, 25, 22, 100, 18, "BUTTON", "Refresh Threads", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGESTAT },
	{MWCS_Button_RefreshProps, 135, 22, 100, 18, "BUTTON", "Refresh Props", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGESTAT },
	{MWCS_Button_RefreshPlayers, 245, 22, 100, 16, "BUTTON", "Refresh Players", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGESTAT },
	{MWCS_Button_RefreshPref, 355, 22, 50, 16, "BUTTON", "Prefs", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGESTAT },
	{MWCS_Button_RefreshInventory, 410, 22, 50, 16, "BUTTON", "Inv.", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGESTAT },
	{MWCS_Button_RefreshPointer, 355, 39, 50, 16, "BUTTON", "Ptr", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGESTAT },
	{MWCS_Button_RefreshInstance, 410, 39, 50, 16, "BUTTON", "Inst.", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGESTAT },
	{MWCS_Button_RefreshSpawnPackages, 465, 22, 50, 16, "BUTTON", "Sp Pkg.", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGESTAT },
	{MWCS_Button_RefreshSpawn, 465, 39, 50, 16, "BUTTON", "Spawn", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGESTAT },
	{MWCS_Button_RefreshTime, 245, 39, 40, 16, "BUTTON", "Time", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGESTAT },
	{MWCS_Button_RefreshParty, 290, 39, 40, 16, "BUTTON", "Party", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGESTAT },
	{MWCS_Button_RefreshAIScripts, 135, 39, 100, 18, "BUTTON", "Refresh AI Scripts", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGESTAT },
	{MWCS_Button_RefreshMods, 25, 39, 40, 18, "BUTTON", "Mods", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGESTAT },
	{MWCS_Button_RefreshHate, 70, 39, 40, 18, "BUTTON", "Hate", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGESTAT },
	{MWCS_Button_RefreshProfiler, 110, 39, 20, 18, "BUTTON", "Pr", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGESTAT },
	{MWCS_Button_RefreshQuests, 520, 22, 50, 18, "BUTTON", "Quests", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGESTAT },
	{MWCS_Button_RefreshChar, 520, 39, 50, 18, "BUTTON", "Char", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGESTAT },

	{MWCS_Edit_Stats, 20,  55, 550, 300, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, WS_EX_CLIENTEDGE, PAGESTAT },
	
	{MWCS_Button_INITIATEFUN,   100, 300, 220, 30, "BUTTON", "INITIATE SUPER FUN MODE", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGETOOL },


	//Chat Tab
	{MWCS_Button_ClearChat, 25, 30, 80, 18, "BUTTON", "Clear Chat", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGECHAT },
	{MWCS_Edit_ChatBox, 20,  75, 550, 275, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, WS_EX_CLIENTEDGE, PAGECHAT },
	{MWCS_Edit_ChatName, 200, 30, 100, 18, "EDIT", "Admin", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE, PAGECHAT },
	{MWCS_Edit_ChatChannel, 360, 30, 100, 18, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE, PAGECHAT },
	{MWCS_Edit_ChatMessage, 75, 55, 425, 18, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE, PAGECHAT },
	{MWCS_ComboBox_ChatChannel, 470, 30, 100, 160, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_DISABLENOSCROLL, NULL, PAGECHAT },

	{MWCS_Static_ChatName, 150,  30, 40, 20, "STATIC", "Name:",  WS_CHILD | WS_VISIBLE, NULL, PAGECHAT },
	{MWCS_Static_ChatChannel, 310,  30, 50, 20, "STATIC", "Channel:",  WS_CHILD | WS_VISIBLE, NULL, PAGECHAT },
	{MWCS_Static_ChatMessage, 25,  55, 50, 18, "STATIC", "Message:",  WS_CHILD | WS_VISIBLE, NULL, PAGECHAT },
	{MWCS_Button_ChatSend,  510, 55, 60, 18, "BUTTON", "Send", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, NULL, PAGECHAT },

	//MWCS_Edit_Convert,   20,  420, 460, 40, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, WS_EX_CLIENTEDGE,
	//MWCS_Button_ToANSI,  20, 400, 80, 18, "BUTTON", "To ANSI", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, NULL,
	//MWCS_Button_ToUTF,  100, 400, 80, 18, "BUTTON", "To UTF", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, NULL,
};

const int NumTools = 3;
ToolControl ToolControlDef[3] = {
	{
		"Chat",  TCID_Chat,
		{ "Name", "Channel", "Message", NULL, NULL },
		{ "Admin", "*SysChat", "Message", NULL, NULL },
		{ TI_String, TI_String, TI_String, TI_None, TI_None }
	},

	{
		"Cue Effect", TCID_CueEffect,
		{ "ID", "Effect", NULL, NULL, NULL },
		{ "10000000", "", NULL, NULL, NULL },
		{ TI_Integer, TI_String, TI_None, TI_None, TI_None }
	},

	{
		"Cue Effect (Target)", TCID_CueEffectTarget,
		{"ID", "Effect", "Target", NULL, NULL},
		{"10000000", "", "", NULL, NULL},
		{TI_Integer, TI_String, TI_Integer, TI_None, TI_None}
	}
};

const int NumChatChannelListItems = 4;
ChatChannelListDef ChatChannelList[4] = {
	{"Say",       "s" },
	{"Region",    "rc/" },
	{"GM",        "gm/earthsages" },
	{"SysChat",   "*SysChat" },
	//"Tell/\"Name\"", "tell/\"First Last\"",
	//"Clan",      "clan",
	//"Friends",   "friends"
};

const int MAX_TOOL_FUNCTION = 12;
ToolFunctionCallData toolFunction[MAX_TOOL_FUNCTION] =
{
	{"Stats", Event_Click_Button_RefreshStats },
	{"Props", Event_Click_Button_RefreshProps },
	{"Players", Event_Click_Button_RefreshPlayers },
	{"Preferences", Event_Click_Button_RefreshPrefs },
	{"Inventory", Event_Click_Button_RefreshInventory },
	{"Instance", Event_Click_Button_RefreshInstance },
	{"Pointer", Event_Click_Button_RefreshPointer },
	{"Spawn Packages", Event_Click_Button_RefreshSpawnPackages },
	{"Spawn Pages", Event_Click_Button_RefreshSpawn },
	{"Session Times", Event_Click_Button_RefreshTime },
	{"AI Scripts", Event_Click_Button_RefreshAIScripts },
	{"Stat Mods/Buffs", Event_Click_Button_RefreshMods }
};

const char *ToolItemType[6] = { "none", "string", "byte", "short", "integer", "float" };

int MainWindowSizeX = 600;
int MainWindowSizeY = 400;
int WindowPosX = -1;
int WindowPosY = -1;

int VisPage = 1;           //Currently visible page.  Controls use a bit mask, allowing for up to 32 unique pages

HWND hwndMainWindow = NULL;
bool bMainWindowStatus = false;

HFONT StandardFont = NULL;
HINSTANCE g_hInst = NULL;

char WindowTextBuffer[32767];

bool __stdcall InitMainWindow(HINSTANCE hInstance, int nCmdShow)
{
	ZeroMemory(MainWindowControlSet, sizeof(MainWindowControlSet));
	HDC dc;
	dc = GetDC(NULL);
	long lfHeight = -MulDiv(8, GetDeviceCaps(dc, LOGPIXELSY), 72);
	StandardFont = CreateFont(lfHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Tahoma");
	ReleaseDC(NULL, dc);

	WNDCLASSEX	wcx;
	ZeroMemory(&wcx, sizeof(WNDCLASSEX));
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.lpfnWndProc = MainWindowProc;
	wcx.hInstance = hInstance;
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.hIcon = NULL;
	wcx.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcx.lpszClassName = szMainWindow;
	wcx.lpszMenuName = szMainWindow;

	if(!RegisterClassEx(&wcx))
		return false;

	int xloc = WindowPosX;
	int yloc = WindowPosY;
	if(xloc == -1)
		xloc = (GetSystemMetrics(SM_CXSCREEN) / 2) - (MainWindowSizeX / 2);
	if(yloc == -1)
		yloc = (GetSystemMetrics(SM_CYSCREEN) / 2) - (MainWindowSizeY / 2);

	hwndMainWindow = CreateWindowEx(0, szMainWindow, szMainWindow, WS_VISIBLE | WS_OVERLAPPEDWINDOW, xloc, yloc, MainWindowSizeX, MainWindowSizeY, NULL, NULL, hInstance, NULL);
	if(hwndMainWindow == NULL)
		return false;

	ShowWindow(hwndMainWindow, SW_NORMAL);
	UpdateWindow(hwndMainWindow);
	return true;
}

LRESULT APIENTRY StandardEditBoxProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool Ctrl = false;
	int tSize = 0;

	switch(message)
	{
	case WM_KILLFOCUS:
		Ctrl = false;
		break;
	case WM_KEYUP:
		switch(wParam)
		{
		case VK_CONTROL:
			Ctrl = false;
			break;
		}
		break;
	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_CONTROL:
			Ctrl = true;
			break;
		case 'A':
			if(Ctrl == true)
			{
				tSize = SendMessage(hWnd, EM_GETLIMITTEXT, 0, 0);
				SendMessage(hWnd, EM_SETSEL, 0, tSize);
			}
			break;
		case VK_RETURN:
			//Special case for the chat message box, pressing Enter will send a message
			if(hWnd == MainWindowControlSet[MWCS_Edit_ChatMessage])
				Event_Click_Button_ChatSend();
			break;
		}
		break;
	}
	return CallWindowProc(wpMainProc, hWnd, message, wParam, lParam);
}

long __stdcall MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
    {
		case WM_CREATE:
			int a;

			InitCommonControls();

			for(a = 0; a < MWCS_MAX; a++)
			{
				MainWindowControlSet[a] = CreateWindowEx(MainWindowControlDef[a].ExStyle, MainWindowControlDef[a].ClassName, MainWindowControlDef[a].WindowName, MainWindowControlDef[a].Style, MainWindowControlDef[a].X, MainWindowControlDef[a].Y, MainWindowControlDef[a].Width, MainWindowControlDef[a].Height, hWnd, (HMENU)MainWindowControlDef[a].ControlIndex, g_hInst, 0);
			}
			for(a = 0; a < MWCS_MAX; a++)
			{
				if(MainWindowControlSet[a] != NULL)
				{
					SendDlgItemMessage(hWnd, a, WM_SETFONT, (WPARAM)StandardFont, TRUE);
					if((MainWindowControlDef[a].Page & VisPage) == 0)
						ShowWindow(MainWindowControlSet[a], SW_HIDE);
				}
			}

//			MainWindowControlSet[MWCS_Tab_Main] = CreateWindow(WC_TABCONTROL, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 400, 300, hWnd, (HMENU)MWCS_Tab_Main, g_hInst, 0);

			TCITEM tci;
			memset(&tci, 0, sizeof(tci));
			tci.mask = TCIF_TEXT;
			tci.pszText = TEXT("Main");
			SendDlgItemMessage(hWnd, MWCS_Tab_Main, TCM_INSERTITEM, 0, (LPARAM)&tci);
			tci.pszText = TEXT("Tools");
			SendDlgItemMessage(hWnd, MWCS_Tab_Main, TCM_INSERTITEM, 1, (LPARAM)&tci);
			tci.pszText = TEXT("Stats");
			SendDlgItemMessage(hWnd, MWCS_Tab_Main, TCM_INSERTITEM, 2, (LPARAM)&tci);
			tci.pszText = TEXT("Chat");
			SendDlgItemMessage(hWnd, MWCS_Tab_Main, TCM_INSERTITEM, 3, (LPARAM)&tci);


			for(a = 0; a < NumTools; a++)
				SendDlgItemMessage(hWnd, MWCS_ComboBox_ToolList, CB_ADDSTRING, 0, (LPARAM)ToolControlDef[a].ToolName);
			SendDlgItemMessage(hWnd, MWCS_ComboBox_ToolList, CB_SETCURSEL, 0, 0);
			ChangeToolIndex();

			/*
			for(a = 0; a < NumChatChannelListItems; a++)
				SendDlgItemMessage(hWnd, MWCS_ComboBox_ChatChannel, CB_ADDSTRING, 0, (LPARAM)ChatChannelList[a].channelName);
			SendDlgItemMessage(hWnd, MWCS_ComboBox_ChatChannel, CB_SETCURSEL, 0, 0);
			*/

			//Index zero is a null entry so skip it
			for(a = 1; a < NumValidChatChannel; a++)
			{
				const char *name = ValidChatChannel[a].channel;
				if(ValidChatChannel[a].friendly != NULL)
					name = ValidChatChannel[a].friendly;
				SendDlgItemMessage(hWnd, MWCS_ComboBox_ChatChannel, CB_ADDSTRING, 0, (LPARAM)name);
			}
			SendDlgItemMessage(hWnd, MWCS_ComboBox_ChatChannel, CB_SETCURSEL, 0, 0);
			ChangeChatChannelListIndex();


			wpMainProc = (WNDPROC) GetWindowLongPtr(MainWindowControlSet[MWCS_Edit_ChatMessage], GWL_WNDPROC);
			for(a = 0; a < MWCS_MAX; a++)
				if(strcmp(MainWindowControlDef[a].ClassName, "EDIT") == 0)
					SetWindowLongPtr(MainWindowControlSet[a], GWL_WNDPROC, (LONG)StandardEditBoxProc);


			//wpMainProc = (WNDPROC) SetWindowLongPtr(MainWindowControlSet[MWCS_Edit_ChatMessage], GWL_WNDPROC, (LONG)StandardEditBoxProc);
			//SetWindowLongPtr(MainWindowControlSet[MWCS_Edit_ChatMessage], GWL_WNDPROC, (LONG)StandardEditBoxProc);

			/*
			SetWindowLongPtr(MainWindowControlSet[MWCS_Edit_Send], GWL_WNDPROC, (LONG)StandardEditBoxProc);
			SetWindowLongPtr(MainWindowControlSet[MWCS_Edit_Receive], GWL_WNDPROC, (LONG)StandardEditBoxProc);
			SetWindowLongPtr(MainWindowControlSet[MWCS_Edit_Status], GWL_WNDPROC, (LONG)StandardEditBoxProc);
			SetWindowLongPtr(MainWindowControlSet[MWCS_Edit_Stats], GWL_WNDPROC, (LONG)StandardEditBoxProc);
			SetWindowLongPtr(MainWindowControlSet[MWCS_Edit_ChatBox], GWL_WNDPROC, (LONG)StandardEditBoxProc);
			SetWindowLongPtr(MainWindowControlSet[MWCS_Edit_ChatName], GWL_WNDPROC, (LONG)StandardEditBoxProc);
			SetWindowLongPtr(MainWindowControlSet[MWCS_Edit_ChatChannel], GWL_WNDPROC, (LONG)StandardEditBoxProc);

			SetWindowLongPtr(MainWindowControlSet[MWCS_Edit_ToolEdit0], GWL_WNDPROC, (LONG)StandardEditBoxProc);
			SetWindowLongPtr(MainWindowControlSet[MWCS_Edit_ToolEdit1], GWL_WNDPROC, (LONG)StandardEditBoxProc);
			SetWindowLongPtr(MainWindowControlSet[MWCS_Edit_ToolEdit2], GWL_WNDPROC, (LONG)StandardEditBoxProc);
			SetWindowLongPtr(MainWindowControlSet[MWCS_Edit_ToolEdit3], GWL_WNDPROC, (LONG)StandardEditBoxProc);
			SetWindowLongPtr(MainWindowControlSet[MWCS_Edit_ToolEdit4], GWL_WNDPROC, (LONG)StandardEditBoxProc);
			*/

			//SetWindowSubclass(hWnd, *ChatSendProc, MWCS_Edit_ChatMessage, NULL);

			/*
			for(a = 0; a < NUMACTIONTYPES; a++)
				SendDlgItemMessage(hWnd, EWCS_ComboBox_Type, CB_ADDSTRING, 0, (LPARAM)ActionTypes[a]);

			for(a = 0; a < NUMCMDTYPES; a++)
				SendDlgItemMessage(hWnd, EWCS_ComboBox_ShowCmd, CB_ADDSTRING, 0, (LPARAM)CmdTypes[a]);
				*/
	
			break;
		case WM_CTLCOLORSTATIC:
			break;
     	case WM_ACTIVATE:
			bMainWindowStatus = (LOWORD(wParam) != WA_INACTIVE) && !((BOOL)HIWORD(wParam)); 
			break;
        case WM_DESTROY:
			DestroyWindow(hwndMainWindow);
			break;
		case WM_CLOSE:
			g_ServerStatus = 0;
            return 0L;
		case WM_COMMAND:
			ProcessControls(wParam);
            break;
		case WM_NOTIFY:
			LPNMHDR lpnmhdr;
			lpnmhdr = (LPNMHDR)lParam;
			if(lpnmhdr->code == TCN_SELCHANGE)
			{
				LRESULT res;
				res = SendMessage(MainWindowControlSet[MWCS_Tab_Main], TCM_GETCURSEL, 0, 0);
				ChangePage(1 << res);
			}
			break;
	}
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void FreeWindow(void)
{
	if(StandardFont != NULL)
	{
		DeleteObject(StandardFont);
		StandardFont = NULL;
	}
	if(hwndMainWindow != NULL)
	{
		DestroyWindow(hwndMainWindow);
		hwndMainWindow = NULL;
	}
}

void ProcessControls(WPARAM wParam)
{
	switch(HIWORD(wParam))
	{
	case BN_CLICKED:
		switch(LOWORD(wParam))
		{
		case MWCS_Button_StatusClear:
			Event_Click_Button_StatusClear();
			break;
		case MWCS_Button_SaveItems:
			Event_Click_Button_SaveItems();
			break;
		case MWCS_Button_DisassembleScripts:
			Event_Click_Button_DisassembleScripts();
			break;
		case MWCS_Button_TestCrash:
			Event_Click_Button_TestCrash();
			break;
		case MWCS_Button_VariableDump:
			Event_Click_Button_VariableDump();
			break;
		case MWCS_Button_FlushLog:
			Event_Click_Button_FlushLog();
			break;
		case MWCS_Button_ReloadQuests:
			Event_Click_Button_ReloadQuests();
			break;
		case MWCS_Button_ReloadChecksum:
			Event_Click_Button_ReloadChecksum();
			break;
		case MWCS_Button_ReloadAbilities:
			Event_Click_Button_ReloadAbilities();
			break;
		case MWCS_Button_DamageTest:
			Event_Click_Button_DamageTest();
			break;
		case MWCS_Button_UseTool:
			Event_Click_Button_UseTool();
			break;
		case MWCS_Button_RefreshStats:
			Event_Click_Button_RefreshStats();
			break;
		case MWCS_Button_RefreshProps:
			Event_Click_Button_RefreshProps();
			break;
		case MWCS_Button_RefreshPlayers:
			Event_Click_Button_RefreshPlayers();
			break;
		case MWCS_Button_RefreshPref:
			Event_Click_Button_RefreshPrefs();
			break;
		case MWCS_Button_RefreshInventory:
			Event_Click_Button_RefreshInventory();
			break;
		case MWCS_Button_RefreshInstance:
			Event_Click_Button_RefreshInstance();
			break;
		case MWCS_Button_RefreshPointer:
			Event_Click_Button_RefreshPointer();
			break;
		case MWCS_Button_RefreshSpawnPackages:
			Event_Click_Button_RefreshSpawnPackages();
			break;
		case MWCS_Button_RefreshSpawn:
			Event_Click_Button_RefreshSpawn();
			break;
		case MWCS_Button_RefreshTime:
			Event_Click_Button_RefreshTime();
			break;
		case MWCS_Button_RefreshParty:
			Event_Click_Button_RefreshParty();
			break;
		case MWCS_Button_RefreshAIScripts:
			Event_Click_Button_RefreshAIScripts();
			break;
		case MWCS_Button_RefreshMods:
			Event_Click_Button_RefreshMods();
			break;
		case MWCS_Button_RefreshHate:
			Event_Click_Button_RefreshHate();
			break;
		case MWCS_Button_RefreshProfiler:
			Event_Click_Button_RefreshProfiler();
			break;
		case MWCS_Button_RefreshQuests:
			Event_Click_Button_RefreshQuests();
			break;
		case MWCS_Button_RefreshChar:
			Event_Click_Button_RefreshChar();
			break;
		case MWCS_Button_ClearChat:
			Event_Click_Button_ClearChat();
			break;
		case MWCS_Button_ChatSend:
			Event_Click_Button_ChatSend();
			break;
		case MWCS_Button_INITIATEFUN:
			static int t = 0;

			if(t == 0)
				MessageBox(0, "Current versions of the client protocol do not support this feature.", "Critical Error", MB_ICONERROR);
			else if(t == 1)
				MessageBox(0, "No, seriously.  It can't be done.", "Impossible", MB_ICONERROR);
			else if(t == 2)
				MessageBox(0, "It's just not possible.", "Really", MB_ICONERROR);
			else if(t == 3)
				MessageBox(0, "Clicking more will not help.", "Listen", MB_ICONERROR);
			else if(t >= 4 && t <= 7)
				MessageBox(0, "Stop clicking.", "Seriously", MB_ICONERROR);
			else if(t == 8)
				MessageBox(0, "Enough of this.  One more click will terminate this program.", "Last Warning", MB_ICONERROR);
			else if(t == 9)
				g_ServerStatus = 0;
			t++;
			break;
			/*
		case MWCS_Button_ToANSI:
			Event_Click_Button_ToANSI();
			break;
		case MWCS_Button_ToUTF:
			Event_Click_Button_ToUTF();
			break;*/
		}
		break;
	case CBN_SELCHANGE:
		switch(LOWORD(wParam))
		{
			case MWCS_ComboBox_ToolList:
				ChangeToolIndex();
				break;
			case MWCS_ComboBox_ChatChannel:
				ChangeChatChannelListIndex();
				break;
		}
		break;
		/*
	case EN_SETFOCUS:
		switch(LOWORD(wParam))
		{
		case MWCS_Edit_ChatMessage:
			ChatSendFocus = true;
			break;
		}
		break;
	case EN_KILLFOCUS:
		switch(LOWORD(wParam))
		{
		case MWCS_Edit_ChatMessage:
			ChatSendFocus = false;
			break;
		}
		break;*/
	}
}

void ChangePage(int NewPage)
{
	if(VisPage == NewPage)
		return;
	VisPage = NewPage;

	int a;
	for(a = 0; a < MWCS_MAX; a++)
	{
		if(MainWindowControlSet[a] != NULL)
		{
			if(MainWindowControlDef[a].Page & VisPage)
				ShowWindow(MainWindowControlSet[a], SW_SHOW);
			else
				ShowWindow(MainWindowControlSet[a], SW_HIDE);
		}
	}
}

void ChangeToolIndex(void)
{
	LRESULT res = SendMessage(MainWindowControlSet[MWCS_ComboBox_ToolList], CB_GETCURSEL, 0, 0);

	//Update the controls
	static char buffer[256];
	
	int a;
	for(a = 0; a < 5; a++)
	{
		if(ToolControlDef[res].ItemType[a] != TI_None)
		{
			sprintf(buffer, "%s (%s)", ToolControlDef[res].ItemName[a], ToolItemType[ToolControlDef[res].ItemType[a]]);
			SetWindowText(MainWindowControlSet[MWCS_Static_ToolText0 + a], buffer);
			SetWindowText(MainWindowControlSet[MWCS_Edit_ToolEdit0 + a], ToolControlDef[res].DefaultString[a]);
		}
		else
		{
			SetWindowText(MainWindowControlSet[MWCS_Static_ToolText0 + a], "Unused");
			SetWindowText(MainWindowControlSet[MWCS_Edit_ToolEdit0 + a], "");
		}
	}
}

void ChangeChatChannelListIndex(void)
{
	LRESULT res = SendMessage(MainWindowControlSet[MWCS_ComboBox_ChatChannel], CB_GETCURSEL, 0, 0);
	//SetWindowText(MainWindowControlSet[MWCS_Edit_ChatChannel], ChatChannelList[res].channelStr);

	//The list of options does not show the initial NULL entry so offset to the correct one
	SetWindowText(MainWindowControlSet[MWCS_Edit_ChatChannel], ValidChatChannel[res + 1].channel);
	EnableWindow(MainWindowControlSet[MWCS_Edit_ChatName], ValidChatChannel[res + 1].name);
}

/*
void AddStatusWindowText(char *text)
{
	StatusWindowPending++;
	int Remain = 0;
	int ToCopy = 0;
	try
	{
	GetWindowText(MainWindowControlSet[MWCS_Edit_Status], WindowTextBuffer, sizeof(WindowTextBuffer));
	Remain = sizeof(WindowTextBuffer) - strlen(WindowTextBuffer) - 2;
	ToCopy = strlen(text);
	if(ToCopy >= Remain)
	{
		StatusWindowPending--;
		return;
	}
	strncat_s(WindowTextBuffer, sizeof(WindowTextBuffer) - 1, text, ToCopy);
	if(Remain > 2)
		strncat_s(WindowTextBuffer, sizeof(WindowTextBuffer), "\r\n", 2);
	SetWindowText(MainWindowControlSet[MWCS_Edit_Status], WindowTextBuffer);

	HRESULT hres;
	hres = SendMessage(MainWindowControlSet[MWCS_Edit_Status], EM_GETLINECOUNT, NULL, NULL);
	SendMessage(MainWindowControlSet[MWCS_Edit_Status], EM_LINESCROLL, NULL, hres);
	}
	catch (...)
	{
		MessageBox(NULL, "Exception raised in AddStatusWindowText", "Critical Error", MB_OK);
		SetWindowText(MainWindowControlSet[MWCS_Edit_Status], "CRITICAL ERROR: Exception raised, clearing buffer.");
		AddStatusWindowTextFormat("  Remain: %d, ToCopy: %d", Remain, ToCopy);
	}
	StatusWindowPending--;
}
*/

/*
void AddStatusWindowTextFormat(char *format, ...)
{
	static char PrepBuffer[2048];
	va_list args;
	va_start (args, format);
	vsprintf_s(PrepBuffer, sizeof(PrepBuffer), format, args);
	va_end (args);

	AddStatusWindowText(PrepBuffer);
}
*/

void ClearStatusWindowText(void)
{
	SetWindowText(MainWindowControlSet[MWCS_Edit_Status], "");
}

void SetWindowTextToValue(HWND hwnd, int value)
{
	static char ConvBuf[32];
	sprintf(ConvBuf, "%d", value);
	SetWindowText(hwnd, ConvBuf);
}

#endif // USE_WINDOWS_GUI