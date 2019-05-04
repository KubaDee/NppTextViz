//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
//#include "DockingFeature/searchBoxDlg.h"

extern FuncItem funcItem[nbFunc];
extern NppData nppData;
//extern SearchBoxDemoDlg _SearchBox;

#define INT_CURRENTEDIT int currentEdit
#define GET_CURRENTEDIT ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit)
#define SENDMSGTOCED(whichEdit,mesg,wpm,lpm) SendMessage(((whichEdit)?nppData._scintillaSecondHandle:nppData._scintillaMainHandle),mesg,(WPARAM)(wpm),(LPARAM)(lpm))

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  reasonForCall, LPVOID /*lpReserved*/)
{
    switch (reasonForCall)
    {
      case DLL_PROCESS_ATTACH:
        pluginInit(hModule);
        break;

      case DLL_PROCESS_DETACH:
        pluginCleanUp();
        break;

      case DLL_THREAD_ATTACH:
        break;

      case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}

extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	nppData = notpadPlusData;
	commandMenuInit();
	IniSaveSettings(false);
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return NPP_PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
	*nbF = nbFunc;
	return funcItem;
}


extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
	bool kscapital;
	static unsigned prevline;
	INT_CURRENTEDIT;
	GET_CURRENTEDIT;

	switch (notifyCode->nmhdr.code) 
	{
		unsigned curpos;
		unsigned curline;

		case NPPN_SHUTDOWN:
		{
			commandMenuCleanUp();
		}
		break;

		case SCN_UPDATEUI:
			curpos = (int)SENDMSGTOCED(currentEdit, SCI_GETCURRENTPOS, 0, 0);
			curline = (int)SENDMSGTOCED(currentEdit, SCI_LINEFROMPOSITION, curpos, 0);
			kscapital = (GetAsyncKeyState(VK_CAPITAL) & 0x8000) ? TRUE : FALSE;

			if (kscapital) {
				if (prevline != curline) {
					int dir = curline > prevline ? 1 : -1;
					if (kscapital && prevline + dir != curline && !SENDMSGTOCED(currentEdit, SCI_GETLINEVISIBLE, prevline + dir, 0)) {
						unsigned col = (int)SENDMSGTOCED(currentEdit, SCI_GETCOLUMN, curpos, 0);
						curline = prevline + dir;
						SENDMSGTOCED(currentEdit, SCI_SHOWLINES, curline, curline);
						SENDMSGTOCED(currentEdit, SCI_GOTOPOS, SENDMSGTOCED(currentEdit, SCI_FINDCOLUMN, curline, col), 0);
						if (curline && AlterMenuCheck(FindMenuItem(doUpdateCapsSeq), '-')) {
							CString cstrTemp;
							cstrTemp.Format(_T("+[%d-%d]"), curline, curline + 2);
							AppendSequence(cstrTemp);
						}
					}
				}
			}
			prevline = curline;

		default:
			return;
	}
}


// Here you can process the Npp Messages 
// I will make the messages accessible little by little, according to the need of plugin development.
// Please let me know if you need to access to some messages :
// http://sourceforge.net/forum/forum.php?forum_id=482781
//
extern "C" __declspec(dllexport) LRESULT messageProc(UINT /*Message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{/*
	if (Message == WM_MOVE)
	{
		::MessageBox(NULL, "move", "", MB_OK);
	}
*/
	return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
#endif //UNICODE
