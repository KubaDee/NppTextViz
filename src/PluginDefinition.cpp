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
#include "menuCmdID.h"
#include <atlstr.h>
//#include "DockingFeature/searchBoxDlg.h"

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

//
// JD: Show/Hide lines sequence
//
CString cstrSequence;
long stStepPosition = 0; // for single stepping

//
// JD: config values
//
BOOL bVizWholeWords = FALSE;
BOOL bVizCaseSensitive = FALSE;
BOOL bVizRegExp = FALSE;
// set flags according search conf
#define VIZMENUFLAGS ((bVizCaseSensitive?SCFIND_MATCHCASE:0)|(bVizWholeWords?SCFIND_WHOLEWORD:0)|(bVizRegExp?SCFIND_REGEXP:0))

// inicializace fci pro zmenu checku v menu konfig (funkce prehazuje 1/0)
unsigned miVizWholeWords; void doUpdateConfWholeWords() { bVizWholeWords = AlterMenuCheck(miVizWholeWords, '!'); }
unsigned miVizRegExp; void doUpdateConfRegExp() { bVizRegExp = AlterMenuCheck(miVizRegExp, '!'); }
unsigned miVizCaseSens; void doUpdateConfCaseSens() { bVizCaseSensitive = AlterMenuCheck(miVizCaseSens, '!'); }


//
// DEFINE part
//

// JD: Basic kod prevezmut z NFX
#define INT_CURRENTEDIT int currentEdit
#define GET_CURRENTEDIT SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit)
//#define GET_CURRENTSCINTILLA HWND curScint = (currentEdit == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle
#define SENDMSGTOCED(whichEdit,mesg,wpm,lpm) SendMessage(((whichEdit)?nppData._scintillaSecondHandle:nppData._scintillaMainHandle),mesg,(WPARAM)(wpm),(LPARAM)(lpm))

CString tmpMsg; // for MessageBoxExtra format

const char eoltypes[3][4] = { ("\r\n"), ("\r"), ("\n") };
#ifndef NELEM
#define NELEM(xyzzy) (sizeof(xyzzy)/sizeof(xyzzy[0]))
#endif

#define SCDS_COPY 0x1   /* To cut, specify SCDS_COPY|SCDS_DELETE */
#define SCDS_DELETE 0x2
#define SCDS_COPYRECTANGULAR 0x4 /* Mark the text as from a rectangular selection according to the Scintilla standard */
#define SCDS_COPYAPPEND 0x8 /* Reads the old clipboard (depends on flags) and appends the new text to it */
//#define SCDS_COPYEOLALWAYSCRLF 0x10 /* EOL are converted CRLF before placing on the clipboard so the text will paste properly in apps that can't handle CR or LF termination. */
//#define SCDS_COPYALSOUTF8 0x20 /* UNICODE is always copied in UNICODEMODE, this also copies UTF-8 into CF_TEXT in UNICODE */
//#define SCDS_COPYCONVERTNULLSTOSPACE 0x40 /* Convert NUL to space for non binary safe editors */
//#define SCDS_COPYNOTUNICODE 0x80 /* Do not convert UTF-8 to UNICODE for CF_UNICODETEXT. System will automatically produce wchar_t UTF-8 which will paste into any application as UTF-8. */
//#define SCDS_UNICODEMODE 0x100 /* Paste: Paste CF_TEXT in ANSI mode, CF_UNICODETEXT in UTF-8 OR UCS-2 modes; Copy: produce CF_UNICODETEXT in UTF-8/UCS-2 modes, produce CF_TEXT in ANSI mode */
//#define SCDS_COPYPASTEBINARY 0x200 /* Retrieve entire Clipboard with last nul character removed; otherwise paste strlen() */
//#define SCDS_PASTEANSIUTF8 0x400 /* Always Paste from CF_TEXT */
//#define SCDS_PASTETOEDITOREOL 0x800 /* Convert all EOL's to SCI_GETEOLMODE */

UINT cfColumnSelect;

//
// SearchBox DLG
//
//SearchBoxDemoDlg _SearchBox;


//
// JD own functions
//

/// helping msg box procedure
void MessageBoxExtra(size_t intToMsg)
{
	CString csMsg;
	csMsg.Format(_T("%d"), intToMsg);
	MessageBox(nppData._nppHandle, csMsg, _T(PLUGIN_NAME), MB_ICONINFORMATION);
}

/// helping msg box procedure
void MessageBoxExtra(const char * strToMsg)
{
	unsigned long kolik = MultiByteToWideChar(CP_UTF8, 0, &strToMsg[0], -1, NULL, 0);

	// if need to convert to Widechar
	if (strlen(strToMsg) != kolik - 1) { // - "\0" at end of string
		wchar_t* bufWchar;
		CString csMsg;
		bufWchar = new wchar_t[kolik];
		MultiByteToWideChar(CP_UTF8, 0, &strToMsg[0], -1, &bufWchar[0], kolik);
		csMsg = bufWchar;
		MessageBox(nppData._nppHandle, csMsg, _T(PLUGIN_NAME), MB_ICONINFORMATION);
		delete[] bufWchar;
	}
	else {
		MessageBox(nppData._nppHandle, (wchar_t*)strToMsg, _T(PLUGIN_NAME), MB_ICONINFORMATION);
	}
}

/// helping msg box procedure
void MessageBoxExtra(wchar_t * strToMsg)
{
	CString csMsg;
	csMsg = strToMsg;
	MessageBox(nppData._nppHandle, csMsg, _T(PLUGIN_NAME), MB_ICONINFORMATION);
}

/// realloc safe procedure - msg box when error
void * reallocsafe(void *bf, size_t ct)
{
	void * bf2;
	if (bf == NULL) {
		bf2 = (void *)malloc(ct);
	}
	else {
		bf2 = (void *)realloc(bf, ct);
	}
	if (bf2 == NULL) {
		MessageBox(nppData._nppHandle, _T("Err reallocsafe !!!"), _T(PLUGIN_NAME), MB_OK | MB_ICONERROR);
		return (NULL);
	}
	else {
		return(bf2);
	}
}

#define mallocsafe(ct) reallocsafe(NULL,ct)

/// Concatenate new strings with resize
bool strcatX(char**(orig), const char* concatenateText)
{
	size_t newLen = strlen(concatenateText);
	char * newArr = (char *)reallocsafe(*orig, strlen(*orig) + newLen + 1);
	if (newArr == NULL) {
		MessageBox(nppData._nppHandle, _T("Err strcatX !!!"), _T(PLUGIN_NAME), MB_OK | MB_ICONERROR);
		return false;
	}
	else {
		strcat(newArr, concatenateText);
		*orig = newArr;
	}
	return true;
}


/* =========================================== INIT BEGIN =========================================== */
//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE /*hModule*/)
{
	cfColumnSelect = RegisterClipboardFormat(_T("MSDEVColumnSelect"));
	// tady se spousti search box
	//_SearchBox.init((HINSTANCE)hModule, NULL);
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{
	//--------------------------------------------//
	//-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
	//--------------------------------------------//
	// with function :
	// setCommand(int index,                      // zero based number to indicate the order of command
	//            TCHAR *commandName,             // the command name that you want to see in plugin menu
	//            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
	//            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
	//            bool check0nInit                // optional. Make this menu item be checked visually
	//            );
	setCommand(0,  TEXT("Hide selected or all text"), doHideSelectedOrAllLines, NULL, false);
	setCommand(1,  TEXT("Show selected or all text"), doShowSelectedOrAllLines, NULL, false);
	setCommand(2,  TEXT("Invert visibility"), doInvertSelectedOrAllLines, NULL, false);
	setCommand(3,  TEXT("----"), NULL, NULL, false);
	setCommand(4,  TEXT("Hide linec with clipboard text"), doLinesHideWith, NULL, false);
	setCommand(5,  TEXT("Hide lines without clipboard text"), doLinesHideWithout, NULL, false);
	setCommand(6,  TEXT("Show lines with clipboard text"), doLinesShowWith, NULL, false);
	setCommand(7,  TEXT("Show lines without clipboard text"), doLinesShowWithout, NULL, false);
	setCommand(8,  TEXT("----"), NULL, NULL, false);
	setCommand(9,  TEXT("Copy hidden selection"), doCopyHidden, NULL, false);
	setCommand(10, TEXT("Copy visible selection"), doCopyVisible, NULL, false);
	setCommand(11, TEXT("Copy all selection"), doCopyAll, NULL, false);
	setCommand(12, TEXT("Cut hidden selection"), doCutHidden, NULL, false);
	setCommand(13, TEXT("Cut visible selection"), doCutVisible, NULL, false);
	setCommand(14, TEXT("Cut all selection"), doCutAll, NULL, false);
	setCommand(15, TEXT("Delete hidden selection"), doDelHidden, NULL, false);
	setCommand(16, TEXT("Delete visible selection"), doDelVisible, NULL, false);
	setCommand(17, TEXT("Delete all selection"), doDelAll, NULL, false);
	setCommand(18, TEXT("Append hidden selection"), doAppendHidden, NULL, false);
	setCommand(19, TEXT("Append visible selectiont"), doAppendVisible, NULL, false);
	setCommand(20, TEXT("Append all selection"), doAppendAll, NULL, false);
	setCommand(21, TEXT("(----"), NULL, NULL, false);
	setCommand(22, TEXT("Hide/Show sequence run all steps"), doSequenceAll, NULL, false);
	setCommand(23, TEXT("Hide/Show sequence run first step"), doSequenceStart, NULL, false);
	setCommand(24, TEXT("Hide/Show sequence run next step"), doSequenceNext, NULL, false);
	setCommand(25, TEXT("Hide/Show sequence run rest steps"), doSequenceRest, NULL, false);
	setCommand(26, TEXT("Copy selected as sequence"), doSelectedAsSequence, NULL, false);
	setCommand(27, TEXT("Paste Hide/Show sequence"), doInsertSequence, NULL, false);
	setCommand(28, TEXT("(----"), NULL, NULL, false);
	setCommand(29, TEXT("Search Whole Words"), doUpdateConfWholeWords, NULL, bVizWholeWords!=0); // VS2015 140_xp
	setCommand(30, TEXT("Search Case sensitive"), doUpdateConfCaseSens, NULL, bVizCaseSensitive!=0); // VS2015 140_xp
	setCommand(31, TEXT("Search RegEx"), doUpdateConfRegExp, NULL, bVizRegExp != 0); // VS2015 140_xp
	setCommand(32, TEXT("(----"), NULL, NULL, false);
	setCommand(33, TEXT("About"), doAboutDlg, NULL, false);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
	IniSaveSettings(true);
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit)
{
	if (index >= nbFunc)
		return false;

	if (!pFunc)
		return false;

	lstrcpy(funcItem[index]._itemName, cmdName);
	funcItem[index]._pFunc = pFunc;
	funcItem[index]._init2Check = check0nInit;
	funcItem[index]._pShKey = sk;

	return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//

/// ulozi text do schranky
void InsertTextToClipboard(const char * strInput, unsigned flags)
{
	INT_CURRENTEDIT;
	GET_CURRENTEDIT;
	CString TextToClip = strInput;

	if (strInput != NULL) {
		// When not Codepage ANSI (codepage != CP_ACP), convert to widechar
		int intCodePage = (int)SENDMSGTOCED(currentEdit, SCI_GETCODEPAGE, 0, 0);
		if (intCodePage != CP_ACP) {
			wchar_t* bufWchar;
			int kolik = MultiByteToWideChar(CP_UTF8, 0, &strInput[0], -1, NULL, 0);
			bufWchar = new wchar_t[kolik];
			MultiByteToWideChar(CP_UTF8, 0, &strInput[0], -1, &bufWchar[0], kolik);
			TextToClip = bufWchar;
			delete[] bufWchar;
		}

		if (flags&SCDS_COPYAPPEND) TextToClip = GetClipboard() + TextToClip;

		if (OpenClipboard(NULL)) {
			EmptyClipboard();

			HGLOBAL hClipboardData;
			size_t size = (TextToClip.GetLength() + 1) * sizeof(TCHAR);
			hClipboardData = GlobalAlloc(NULL, size);
			TCHAR* pchData = (TCHAR*)GlobalLock(hClipboardData);
			memcpy(pchData, LPCTSTR(TextToClip.GetString()), size);
			SetClipboardData(CF_UNICODETEXT, hClipboardData);
			if (flags&SCDS_COPYRECTANGULAR) {
				SetClipboardData(cfColumnSelect, 0);
			}
			GlobalUnlock(hClipboardData);
			CloseClipboard();
		}
	}
}

/// Hide range
void HideLineRange(INT_CURRENTEDIT, int curline, int L1, int L2)
{
	if (L1 == 0) L1++; // Scintilla does not allow us to hide the first line AKA Line#0.
	if (L1 == curline) L1++;
	if (L2 == curline) L2--;
	if (L2 >= L1) {
		if (L1 < curline && curline < L2) {
			SENDMSGTOCED(currentEdit, SCI_HIDELINES, L1, curline - 1); // don't hide the current line
			SENDMSGTOCED(currentEdit, SCI_HIDELINES, curline + 1, L2);
		}
		else {
			SENDMSGTOCED(currentEdit, SCI_HIDELINES, L1, L2);
		}
	}
}

/// based on TextFX v0.25 version
bool ShowHideLinesRoutine(INT_CURRENTEDIT, const char *str, char reveal, BOOL complementary, unsigned searchflags)
{
#define FLAG_CONTINUE 1
#define FLAG_FIRST 2
	BOOL rv = TRUE;

	if (str) {
		struct TextToFind tr;
		tr.chrg.cpMin = 0;
		tr.chrg.cpMax = (long)SENDMSGTOCED(currentEdit, SCI_GETLENGTH, 0, 0);
		tr.lpstrText = str;
		long curpos = (long)SENDMSGTOCED(currentEdit, SCI_GETCURRENTPOS, 0, 0);
		long curline = (long)SENDMSGTOCED(currentEdit, SCI_LINEFROMPOSITION, curpos, 0);
		unsigned flags = FLAG_FIRST; 
		do {
			long L1 = (long)SENDMSGTOCED(currentEdit, SCI_LINEFROMPOSITION, tr.chrg.cpMin, 0);
			if (SENDMSGTOCED(currentEdit, SCI_FINDTEXT, searchflags, &tr) >= 0) flags |= FLAG_CONTINUE; else flags &= ~FLAG_CONTINUE;
			if ((flags&(FLAG_FIRST | FLAG_CONTINUE)) == FLAG_FIRST /* halt when first search fails */) {
				MessageBox(nppData._nppHandle, _T("I'm not going to hide all the lines! Copy something to the Clipboard that can be found!"), _T(PLUGIN_NAME), MB_OK || MB_ICONSTOP);
				rv = FALSE;
				break;
			}
			long L2 = (long)SENDMSGTOCED(currentEdit, SCI_LINEFROMPOSITION, ((flags&FLAG_CONTINUE) ? tr.chrgText.cpMin : tr.chrg.cpMax), 0);
			if (reveal == '+') {
				if (!complementary) L2 = L1;
				SENDMSGTOCED(currentEdit, SCI_SHOWLINES, L1, L2);
			}
			else {
				if (complementary) HideLineRange(currentEdit, curline, L1 + 1 /* This shouldn't always be +1 but thanks to Scintilla not allowing us to hide line 0, it is! */, L2 - ((flags&FLAG_CONTINUE) ? 1 : 0));
				else if (L1 != curline) SENDMSGTOCED(currentEdit, SCI_HIDELINES, L1, L1);
			}
			tr.chrg.cpMin = tr.chrgText.cpMax;
			flags &= ~FLAG_FIRST;
		} while (flags&FLAG_CONTINUE);
		SENDMSGTOCED(currentEdit, SCI_GOTOPOS, curpos, 0);
	}
	return(rv!=0); // VS2015 140_xp
#undef FLAG_CONTINUE
#undef FLAG_FIRST
}

void doLinesHideWith()
{
	ClipLinesRoutine('-', FALSE, VIZMENUFLAGS);
}

void doLinesHideWithout()
{
	ClipLinesRoutine('-', TRUE, VIZMENUFLAGS);
}

void doLinesShowWith()
{
	ClipLinesRoutine('+', FALSE, VIZMENUFLAGS);
}

void doLinesShowWithout()
{
	ClipLinesRoutine('+', TRUE, VIZMENUFLAGS);
}

void doCopyVisible()
{
	CopyCutDelRoutine(SCDS_COPY, '+');
}

void doCopyHidden()
{
	CopyCutDelRoutine(SCDS_COPY, '-');
}

void doCopyAll()
{
	CopyCutDelRoutine(SCDS_COPY, '*');
}

void doCutVisible()
{
	CopyCutDelRoutine(SCDS_COPY | SCDS_DELETE, '+');
}

void doCutHidden()
{
	CopyCutDelRoutine(SCDS_COPY | SCDS_DELETE, '-');
}

void doCutAll()
{
	CopyCutDelRoutine(SCDS_COPY | SCDS_DELETE, '*');
}

void doDelVisible()
{
	CopyCutDelRoutine(SCDS_DELETE, '+');
}

void doDelHidden()
{
	CopyCutDelRoutine(SCDS_DELETE, '-');
}

void doDelAll()
{
	CopyCutDelRoutine(SCDS_DELETE, '*');
}

void doAppendVisible()
{
	CopyCutDelRoutine(SCDS_COPYAPPEND | SCDS_COPY, '+');
}

void doAppendHidden()
{
	CopyCutDelRoutine(SCDS_COPYAPPEND | SCDS_COPY, '-');
}

void doAppendAll()
{
	CopyCutDelRoutine(SCDS_COPYAPPEND | SCDS_COPY, '*');
}


/// Read TCHAR from Clipboard (UNICODE)
CString GetClipboard()
{
	CString strData;
	const TCHAR* returnChar;

	if (OpenClipboard(NULL)) {
		HANDLE hClipboardData = GetClipboardData(CF_UNICODETEXT);
		if (hClipboardData) {
			WCHAR *pchData = (WCHAR*)GlobalLock(hClipboardData);
			if (pchData) {
				strData = pchData;
				GlobalUnlock(hClipboardData);
			}
		}
		CloseClipboard();
	}
	returnChar = strData.GetString();
	strData.ReleaseBuffer();
	return returnChar;
}

/// Set CString to Clipboard
bool SetClipboard(CString textToclipboard)
{
	bool success = true;

	if (OpenClipboard(NULL)) {
		EmptyClipboard();
		HGLOBAL hClipboardData;
		size_t size = (textToclipboard.GetLength() + 1) * sizeof(TCHAR);
		hClipboardData = GlobalAlloc(NULL, size);
		TCHAR* pchData = (TCHAR*)GlobalLock(hClipboardData);
		memcpy(pchData, LPCTSTR(textToclipboard.GetString()), size);
		SetClipboardData(CF_UNICODETEXT, hClipboardData);
		//SetClipboardData(cfColumnSelect, 0); // pri vkladani RECT
		GlobalUnlock(hClipboardData);
		CloseClipboard();
	}
	return success;
}

/// Set CString to Clipboard
bool SetClipboard(char* textToclipboard)
{
	CString txtToClp;
	txtToClp.Format(_T("%s"), textToclipboard);
	return SetClipboard(txtToClp);
}

/// Append CString to Clipboard
void AppendClipboard(CString cstrText)
{
	SetClipboard(GetClipboard() + cstrText);
}

/// Provede operaci se schrankou
void ClipLinesRoutine(char reveal, BOOL complementary, unsigned searchflags)
{
	CString va = GetClipboard();

	if (va.Find(_T("\r\n"), 0) + va.Find(_T("\n"), 0) + va.Find(_T("\r"), 0) <= -3) {
		INT_CURRENTEDIT;
		GET_CURRENTEDIT;

		// convert to multibyte according current Codepage
		int intCodePage = (int)SENDMSGTOCED(currentEdit, SCI_GETCODEPAGE, 0, 0);
		int intWcharLen = WideCharToMultiByte(intCodePage, 0, va.GetString(), -1, NULL, 0, NULL, NULL); // with "\0" at end of string
		char *szTo = new char[intWcharLen + 1];

		WideCharToMultiByte(intCodePage, 0, va.GetString(), -1, szTo, intWcharLen, NULL, NULL);

		if (ShowHideLinesRoutine(currentEdit, szTo, reveal, complementary, searchflags)) {
			cstrSequence.AppendFormat(_T("%c:%s%s%s%s:%s\r\n"), reveal, complementary ? _T("!") : _T(""),
				(searchflags&SCFIND_MATCHCASE) ? _T("") : _T("^"),
				(searchflags&SCFIND_WHOLEWORD) ? _T("w") : _T(""),
				(searchflags&SCFIND_REGEXP) ? _T("r") : _T(""),
				va.GetBuffer());
			va.ReleaseBuffer();
		}
	}
	else {
		MessageBox(nppData._nppHandle, _T("Search text cannot contain EOL characters"), _T(PLUGIN_NAME), MB_OK | MB_ICONINFORMATION);
	}
}

/// Make Cupy/Cut with text
void CopyCutDelRoutine(unsigned flags, char which)
{
	INT_CURRENTEDIT;
	GET_CURRENTEDIT;

	unsigned *lps = NULL, *lpe = NULL;
	char * charBuf = NULL;
	size_t buflen;
	bool isError = false; // to check whether an error appears

	unsigned lplen;
	long p1 = (long)SENDMSGTOCED(currentEdit, SCI_GETSELECTIONSTART, 0, 0);
	long p2 = (long)SENDMSGTOCED(currentEdit, SCI_GETSELECTIONEND, 0, 0);
	if (p1 < p2) {
		unsigned eoltype = (unsigned)SENDMSGTOCED(currentEdit, SCI_GETEOLMODE, 0, 0);
		unsigned eoltypelen = eoltype ? 1 : 2; // EOL length

		if (eoltype >= NELEM(eoltypes))
			eoltype = NELEM(eoltypes) - 1;

		if (SENDMSGTOCED(currentEdit, SCI_SELECTIONISRECTANGLE, 0, 0)) {
			flags |= SCDS_COPYRECTANGULAR;
			eoltypelen = 0; // In case of RECT not adding extra EOL
		}
		else {
			flags &= ~SCDS_COPYRECTANGULAR;
		}
		unsigned blockstart = (unsigned)SENDMSGTOCED(currentEdit, SCI_LINEFROMPOSITION, p1, 0);
		unsigned blocklines = (unsigned)SENDMSGTOCED(currentEdit, SCI_LINEFROMPOSITION, p2, 0) - blockstart + 1;
		unsigned ln; for (lplen = ln = 0; ln < blocklines; ln++) {
			int vis = (int)SENDMSGTOCED(currentEdit, SCI_GETLINEVISIBLE, (blockstart + ln), 0);
			if (which == '*' || (vis && which == '+') || (!vis && which == '-')) {
				unsigned ls; if ((unsigned)INVALID_POSITION == (ls = (long)SENDMSGTOCED(currentEdit, SCI_GETLINESELSTARTPOSITION, (blockstart + ln), 0)))
					continue;
				unsigned le; if ((unsigned)INVALID_POSITION == (le = (long)SENDMSGTOCED(currentEdit, SCI_GETLINESELENDPOSITION, (blockstart + ln), 0) + (ln + 1 >= blocklines ? 0 : eoltypelen)))
					continue;

				if (!lplen || lpe[lplen - 1] != ls) {
					lps = (unsigned *)reallocsafe(lps, sizeof(*lps) * (lplen + 1));
					if (!lps) {
						//MessageBox(nppData._nppHandle, _T("Prusvih lps !!!"), _T(PLUGIN_NAME), MB_OK | MB_ICONERROR);
						isError = true;
						break;
					}

					lpe = (unsigned *)reallocsafe(lpe, sizeof(*lpe) * (lplen + 1));
					if (!lpe) {
						//MessageBox(nppData._nppHandle, _T("Prusvih lpe !!!"), _T(PLUGIN_NAME), MB_OK | MB_ICONERROR);
						isError = true;
						break;
					}

					lps[lplen] = ls;
					lpe[lplen] = le;
					lplen++;
				}
				else {
					lpe[lplen - 1] = le;
				}
			}
		}

		if (flags & SCDS_COPY && !isError) {
			for (buflen = 0, ln = 0; ln < lplen; ln++) {
				struct TextRange tr;
				tr.chrg.cpMin = lps[ln];
				tr.chrg.cpMax = lpe[ln];

				charBuf = (char *)reallocsafe(charBuf, buflen + tr.chrg.cpMax - tr.chrg.cpMin + 1);
				if (!charBuf) {
					//MessageBox(nppData._nppHandle, _T("Prusvih buf !!!"), _T(PLUGIN_NAME), MB_OK | MB_ICONERROR);
					isError = true;
					break;
				}

				tr.lpstrText = charBuf + buflen;
				buflen += (long)SENDMSGTOCED(currentEdit, SCI_GETTEXTRANGE, 0, &tr);

				if (flags&SCDS_COPYRECTANGULAR && ln + 1 < lplen) { // for RECT adding EOL
					if (strcatX(&charBuf, eoltypes[eoltype])) {
						buflen += strlen(eoltypes[eoltype]);
					}
					else {
						//MessageBox(nppData._nppHandle, _T("Prusvih RECT EOL !!!"), _T(PLUGIN_NAME), MB_OK | MB_ICONERROR);
						isError = true;
						break;
					}
				}
			}

			InsertTextToClipboard(charBuf, flags | (SENDMSGTOCED(currentEdit, SCI_SELECTIONISRECTANGLE, 0, 0) ? SCDS_COPYRECTANGULAR : 0));
		}

		if (flags & SCDS_DELETE && !isError) {
			int removeeoltype = 0;
			unsigned curpos = (unsigned)SENDMSGTOCED(currentEdit, SCI_GETCURRENTPOS, 0, 0);
			SENDMSGTOCED(currentEdit, SCI_BEGINUNDOACTION, 0, 0);
			for (ln = lplen; ln > 0; ln--) {
				SENDMSGTOCED(currentEdit, SCI_SETTARGETSTART, lps[ln - 1], 0);
				SENDMSGTOCED(currentEdit, SCI_SETTARGETEND, lpe[ln - 1] + removeeoltype, 0);
				SENDMSGTOCED(currentEdit, SCI_REPLACETARGET, 0, "");
				if (curpos >= lpe[ln - 1]) {						//MODIF Harry: first check if out of entire selection range
					curpos -= lpe[ln - 1] - lps[ln - 1];
				}
				else if (curpos > lps[ln - 1]) {					//MODIF Harry: >= to >, position not affected if selection starts at that position (this is the original check)
					curpos -= (curpos - lps[ln - 1]);				//Only subtract difference
				}
				// in last line of text i'm not removing EOL, current cycle is from last line to first, so adding EOL at end of first pass through
				if (!(flags&SCDS_COPYRECTANGULAR)) removeeoltype = eoltypelen;
			}
			SENDMSGTOCED(currentEdit, SCI_GOTOPOS, curpos, 0);
			SENDMSGTOCED(currentEdit, SCI_ENDUNDOACTION, 0, 0);
		}

		delete[] charBuf;
		delete[] lps;
		delete[] lpe;

	}
	else {
		//MessageBox(nppData._nppHandle, _T("Nothing is selected"), _T(PLUGIN_NAME), MB_OK | MB_ICONINFORMATION);
	}
}


/* ============= INI config part ============= */

unsigned FindMenuItem(PFUNCPLUGINCMD _pFunc)
{
	unsigned itemno, items = NELEM(funcItem);
	for (itemno = 0; itemno < items; itemno++) {
		if (_pFunc == funcItem[itemno]._pFunc) {
			return(itemno);
		}
	}
	return(0);
}

void IniSaveSettings(bool save)
{
	const TCHAR sectionSearch[] = TEXT("NppTextViz");
	const TCHAR keyWholeWords[] = TEXT("WholeWordSearch");
	const TCHAR keyRegpExpSearch[] = TEXT("RegExpSearch");
	const TCHAR keyCaseSensitiveSearch[] = TEXT("CaseSensitiveSearch");
	const TCHAR configFileName[] = TEXT("NppTextViz.ini");
	TCHAR iniFilePath[MAX_PATH];

	// initialize Mmenu Item holder
	miVizWholeWords = FindMenuItem(doUpdateConfWholeWords);
	miVizCaseSens = FindMenuItem(doUpdateConfCaseSens);
	miVizRegExp = FindMenuItem(doUpdateConfRegExp);

	// get path of plugin configuration
	SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)iniFilePath);

	// if config path doesn't exist, we create it
	if (PathFileExists(iniFilePath) == FALSE) {
		CreateDirectory(iniFilePath, NULL);
	}

	PathAppend(iniFilePath, configFileName);

	if (!save) {
		bVizWholeWords = AlterMenuCheck(miVizWholeWords, (GetPrivateProfileInt(sectionSearch, keyWholeWords, 0, iniFilePath) != 0) ? '1' : '0');
		bVizRegExp = AlterMenuCheck(miVizRegExp, (GetPrivateProfileInt(sectionSearch, keyRegpExpSearch, 0, iniFilePath) != 0) ? '1' : '0');
		bVizCaseSensitive = AlterMenuCheck(miVizCaseSens, (GetPrivateProfileInt(sectionSearch, keyCaseSensitiveSearch, 0, iniFilePath) != 0) ? '1' : '0');

		VIZMENUFLAGS;
	}
	else {
		WritePrivateProfileString(sectionSearch, keyWholeWords, bVizWholeWords ? TEXT("1") : TEXT("0"), iniFilePath);
		WritePrivateProfileString(sectionSearch, keyCaseSensitiveSearch, bVizCaseSensitive ? TEXT("1") : TEXT("0"), iniFilePath);
		WritePrivateProfileString(sectionSearch, keyRegpExpSearch, bVizRegExp ? TEXT("1") : TEXT("0"), iniFilePath);
	}
}


/// Check settings menu item
bool AlterMenuCheck(int itemno, char action)
{
	switch (action)
	{
	case '1': // set
		funcItem[itemno]._init2Check = TRUE;
		break;
	case '0': // clear
		funcItem[itemno]._init2Check = FALSE;
		break;
	case '!': // invert
		funcItem[itemno]._init2Check = !funcItem[itemno]._init2Check;
		break;
	case '-': // change nothing but apply the current check value
		break;
	}
	CheckMenuItem(GetMenu(nppData._nppHandle), funcItem[itemno]._cmdID, MF_BYCOMMAND | ((funcItem[itemno]._init2Check) ? MF_CHECKED : MF_UNCHECKED));

	return(funcItem[itemno]._init2Check);
}


/* ============= Invert selection ============= */

// This robot routine expects the fake line numbers 1..SCI_GETLINECOUNT, not 0..SCI_GETLINECOUNT-1
// call with L1=0,L2=0 to do all lines
// reveal: '-'=hide; '+'=show, '!'=invert
void InvertSelectionRoutine(INT_CURRENTEDIT, char reveal, int curline, int L1, int L2)
{
	if (L1 > L2) {
		int temp = L1;
		L1 = L2;
		L2 = temp;
	}
	int L2x = (int)SENDMSGTOCED(currentEdit, SCI_GETLINECOUNT, 0, 0);

	if (L1<1 || L1>L2x || L2<1 || L2>L2x) {
		L1 = 1;
		L2 = L2x;
	}
	else {
		L2--; L1++;
		if (L1 > L2) { // was >=  => reset view even when 3 lines was selected
			L1 = 1;
			L2 = L2x;
		}
	}

	if (reveal == '!') {
		for (; L2 >= L1; L2--)
			if (L2 != curline) {
				if (SENDMSGTOCED(currentEdit, SCI_GETLINEVISIBLE, L2 - 1, 0))
					SENDMSGTOCED(currentEdit, SCI_HIDELINES, L2 - 1, L2 - 1);
				else
					SENDMSGTOCED(currentEdit, SCI_SHOWLINES, L2 - 1, L2 - 1);
			}
	}
	else {
		if (reveal == '+')
			SENDMSGTOCED(currentEdit, SCI_SHOWLINES, L1 - 1, L2 - 1);
		else
			HideLineRange(currentEdit, curline - 1, L1 - 1, L2 - 1);
	}
}

void InvertSelectedOrAllLines(char reveal)
{
	INT_CURRENTEDIT;
	GET_CURRENTEDIT;
	long curpos = (long)SENDMSGTOCED(currentEdit, SCI_GETCURRENTPOS, 0, 0);
	long curline = (long)SENDMSGTOCED(currentEdit, SCI_LINEFROMPOSITION, curpos, 0);
	int L2 = curline + 1;
	int L1 = (int)SENDMSGTOCED(currentEdit, SCI_LINEFROMPOSITION, SENDMSGTOCED(currentEdit, SCI_GETANCHOR, 0, 0), 0) + 1;
	if (L1 > L2) { int temp = L1; L1 = L2; L2 = temp; }
	if (L1 + 1 != L2) { // when selected two consecutive lines, not doing anything, reset works with only one line
		if (L1 >= L2) {
			L2 = L1 = 0;
			if ((reveal == '+' || reveal == '-') && cstrSequence.GetLength()) {
				cstrSequence.Format(_T(""));
			}
			if (reveal != '+')
				cstrSequence.AppendFormat(_T("%c*\r\n"), reveal);
		}
		else {
			cstrSequence.AppendFormat(_T("%c[%d-%d]\r\n"), reveal, L1, L2);
		}
		InvertSelectionRoutine(currentEdit, reveal, curline + 1, L1, L2);
		SENDMSGTOCED(currentEdit, SCI_GOTOPOS, curpos, 0);
	}
}

void doHideSelectedOrAllLines()
{
	InvertSelectedOrAllLines('-');
}

void doShowSelectedOrAllLines()
{
	InvertSelectedOrAllLines('+');
}

void doInvertSelectedOrAllLines()
{
	InvertSelectedOrAllLines('!');
}

void doInsertSequence()
{
	INT_CURRENTEDIT;
	GET_CURRENTEDIT;
	if (cstrSequence.GetLength() > 0) {
		// convert to multibyte according Codepage
		int intCodePage = (int)SENDMSGTOCED(currentEdit, SCI_GETCODEPAGE, 0, 0);
		long intWcharLen = WideCharToMultiByte(intCodePage, 0, cstrSequence.GetString(), -1, NULL, 0, NULL, NULL); // with "\0" at end of string
		char *szTo = new char[intWcharLen + 1];

		WideCharToMultiByte(intCodePage, 0, cstrSequence.GetString(), -1, szTo, intWcharLen, NULL, NULL);
		SENDMSGTOCED(currentEdit, SCI_REPLACESEL, 0, szTo);
	}
	else
		MessageBox(nppData._nppHandle, _T("You have not performed any show hide operations since your last Show All-Reset Lines"), _T(PLUGIN_NAME), MB_OK | MB_ICONINFORMATION);
}


/* ============= Sequence part ============= */

/* Commands: \r\n, \r, or \n terminate each command, all commands sequences start with an implicit SHOW ALL LINES
   +:r:searchtext show lines with found reg-exp text
   +:!:searchtext show lines without found text
   -:w:searchtext hide lines with found text whole words only
   -:!^:searchtext hide lines without found text case insensitive
   +[L1-L2]\r\n      Show lines between L1-L2
   -[L1-L2]\r\n      Hide lines between L1-L2
   ![L1-L2]\r\n      Invert lines from L1-L2
   +* Show all lines
   -* Hide all lines
   !* Invert all lines
*/
void doSequenceNext()
{
	INT_CURRENTEDIT;
	GET_CURRENTEDIT;
	long curpos = (long)SENDMSGTOCED(currentEdit, SCI_GETCURRENTPOS, 0, 0);
	long curline = (long)SENDMSGTOCED(currentEdit, SCI_LINEFROMPOSITION, curpos, 0);
	TCHAR * tchSequence = cstrSequence.GetBuffer();

	long eol = stStepPosition + (long)wcscspn(tchSequence + stStepPosition, _T("\r\n"));
	char chtemp = (char)tchSequence[eol];

	tchSequence[eol] = '\0';
	int L1 = 0, L2 = 0;
	TCHAR *p, *end;
	char reveal;
	BOOL complementary = FALSE;
	unsigned searchflags;

	switch (reveal = (char)tchSequence[stStepPosition]) {
	case '\0': break;
	case '+':
	case '-':
	case '!':
		switch (tchSequence[stStepPosition + 1]) {
		case ':':
			p = tchSequence + stStepPosition + 2;
			searchflags = SCFIND_MATCHCASE;
			end = wcschr(p, ':');
			if (!end) {
				MessageBoxExtra(_T("Can't find : terminator for search text, be sure to use :: if there are no flags"));
				break;
			}
			for (; p < end; p++) switch (*p) {
			case 'w': searchflags |= SCFIND_WHOLEWORD; break;
			case 'r': searchflags |= SCFIND_REGEXP; break;
			case '^': searchflags &= ~SCFIND_MATCHCASE; break;
			case '!': complementary = TRUE; break;
			}
			if (wcslen(end + 1)) {
				// convert to multibyte according Codepage
				int intCodePage = (int)SENDMSGTOCED(currentEdit, SCI_GETCODEPAGE, 0, 0);
				int intWcharLen = WideCharToMultiByte(intCodePage, 0, end + 1, -1, NULL, 0, NULL, NULL); // with "\0" at end of string
				char *szTo = new char[intWcharLen + 1];

				WideCharToMultiByte(intCodePage, 0, end + 1, -1, szTo, intWcharLen, NULL, NULL);
				ShowHideLinesRoutine(currentEdit, szTo, reveal, complementary, searchflags);
			}
			break;
		case '[':
		{
			CString tmpCs = tchSequence + stStepPosition + 1;
			int uIndex = 0;
			L1 = _wtol(tmpCs.Tokenize(_T("[-]"), uIndex));
			L2 = _wtol(tmpCs.Tokenize(_T("[-]"), uIndex));
		}
		case '*':
		{
			InvertSelectionRoutine(currentEdit, reveal, curline + 1, L1, L2);
			break;
		}
		default:
			tmpMsg.Format(_T("Invalid target %c, try :[*"), (wchar_t*)(tchSequence + stStepPosition + 1));
			MessageBoxExtra((TCHAR*)tmpMsg.GetBuffer()); tmpMsg.ReleaseBuffer();
			break;
		}
		break;
	default:
		tmpMsg.Format(_T("Invalid reveal %c, try +-!"), (wchar_t *)reveal);
		MessageBoxExtra((TCHAR*)tmpMsg.GetBuffer()); tmpMsg.ReleaseBuffer();
		break;
	}
	tchSequence[stStepPosition = eol] = chtemp;
	while (cstrSequence[stStepPosition] == '\r' || cstrSequence[stStepPosition] == '\n') {
		stStepPosition++;
	}
	SENDMSGTOCED(currentEdit, SCI_GOTOPOS, curpos, 0);
}

void doSequenceRest()
{
	if (!cstrSequence) MessageBox(nppData._nppHandle, _T("You have not performed any show hide operations since your last Show All Lines"),
		_T(PLUGIN_NAME), MB_OK | MB_ICONINFORMATION);
	else
		while (cstrSequence[stStepPosition]) doSequenceNext();
}

void doSequenceStart()
{
	INT_CURRENTEDIT;
	GET_CURRENTEDIT;
	long curpos = (long)SENDMSGTOCED(currentEdit, SCI_GETCURRENTPOS, 0, 0);
	stStepPosition = 0; // we can do this even if the sequence is invalid
	InvertSelectionRoutine(currentEdit, '+', 0, 0, 0);
	SENDMSGTOCED(currentEdit, SCI_GOTOPOS, curpos, 0);
	doSequenceNext();
}

void doSequenceAll()
{
	doSequenceStart();
	doSequenceRest();
}

void doSelectedAsSequence()
{
	INT_CURRENTEDIT;
	GET_CURRENTEDIT;
	TCHAR *rv;
	unsigned st;

	if ((st = (long)SENDMSGTOCED(currentEdit, SCI_GETSELTEXT, 0, NULL)) > 1 && ((rv = (TCHAR *) mallocsafe((st + 1) * sizeof(TCHAR))) != NULL)) {
		SENDMSGTOCED(currentEdit, SCI_GETSELTEXT, 0, rv);
		cstrSequence = rv;

		// When not Codepage ANSI (codepage != CP_ACP), convert to widechar
		int intCodePage = (int)SENDMSGTOCED(currentEdit, SCI_GETCODEPAGE, 0, 0);
		if (intCodePage != CP_ACP) {
			wchar_t* bufWchar;
			char * strSelected;

			strSelected = (char *)mallocsafe((st + 1) * sizeof(char));

			SENDMSGTOCED(currentEdit, SCI_GETSELTEXT, 0, strSelected);
			int kolik = MultiByteToWideChar(CP_UTF8, 0, &strSelected[0], -1, NULL, 0);
			bufWchar = new wchar_t[kolik];
			MultiByteToWideChar(CP_UTF8, 0, &strSelected[0], -1, &bufWchar[0], kolik);
			cstrSequence = bufWchar;
			delete[] bufWchar;
		}

		cstrSequence.Replace(_T("\n"), _T("\r"));
		cstrSequence.Replace(_T("\r\r"), _T("\r"));
		cstrSequence.Replace(_T("\r"), _T("\r\n"));
		if (cstrSequence.Right(1) != _T("\n")) cstrSequence.Append(_T("\r\n")); // always add EOL to last inserted line
	}
	else MessageBox(nppData._nppHandle, _T("No text selected"), _T(PLUGIN_NAME), MB_ICONINFORMATION);
}


/* ============= About dialog ============= */

void doAboutDlg()
{
	CString aboutMsg = _T("Version: ");
	aboutMsg += _T(PLUGIN_VERSION);
	aboutMsg += _T("\n\nLicense: GPL\n\n");
	aboutMsg += _T("Author: Jakub Dvorak <dvorak.jakub@outlook.com>\n");
	aboutMsg += _T("Based on TextFX plugin v0.25 by Chris Severance");
	MessageBox(nppData._nppHandle, aboutMsg, _T(PLUGIN_NAME), MB_OK);
}

/* ============= Dockable search box part ============= */

/*
#define DOCKABLE_DEMO_INDEX findmenuitem(DockableDlgDemo)

void DockableDlgDemo()
{
	_SearchBox.setParent(nppData._nppHandle);
	tTbData	data = { 0 };

	if (!_SearchBox.isCreated()) {
		_SearchBox.create(&data);

		// define the default docking behaviour
		data.uMask = DWS_DF_CONT_RIGHT;

		data.pszModuleName = _SearchBox.getPluginFileName();

		// the dlgDlg should be the index of funcItem where the current function pointer is
		// in this case is DOCKABLE_DEMO_INDEX
		data.dlgID = 1;
		SendMessage(nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&data);
	}
	_SearchBox.display();
}
*/
