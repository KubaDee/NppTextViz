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

#ifndef PLUGINDEFINITION_H
#define PLUGINDEFINITION_H

//
// All difinitions of plugin interface
//
#include "PluginInterface.h"
#include <atlstr.h>

//-------------------------------------//
//-- STEP 1. DEFINE YOUR PLUGIN NAME --//
//-------------------------------------//
// Here define your plugin name
//
#define PLUGIN_NAME "TextViz"
const TCHAR NPP_PLUGIN_NAME[] = TEXT(PLUGIN_NAME);
#define PLUGIN_VERSION "0.2"


//-----------------------------------------------//
//-- STEP 2. DEFINE YOUR PLUGIN COMMAND NUMBER --//
//-----------------------------------------------//
//
// Here define the number of your plugin commands
//
const int nbFunc = 35;


//
// Initialization of your plugin data
// It will be called while plugin loading
//
void pluginInit(HANDLE hModule);

//
// Cleaning of your plugin
// It will be called while plugin unloading
//
void pluginCleanUp();

//
//Initialization of your plugin commands
//
void commandMenuInit();

//
//Clean up your plugin commands allocation (if any)
//
void commandMenuCleanUp();

//
// Function which sets your command 
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk = NULL, bool check0nInit = false);


//
// Your plugin command functions
//
void doLinesHideWith();
void doLinesHideWithout();
void doLinesShowWith();
void doLinesShowWithout();
void ClipLinesRoutine(char reveal, BOOL complementary, unsigned searchflags);
void doCopyVisible();
void doCopyHidden();
void doCopyAll();
CString GetClipboard();
bool SetClipboard(CString textToclipboard);
void AppendClipboard(CString cstrText);
void AppendSequence(CString cstrText);
void CopyCutDelRoutine(unsigned flags, char which);
void doCutVisible();
void doCutHidden();
void doCutAll();
void doDelVisible();
void doDelHidden();
void doDelAll();
void doAppendVisible();
void doAppendHidden();
void doAppendAll();
bool AlterMenuCheck(int itemno, char action);
void IniSaveSettings(bool save);
unsigned FindMenuItem(PFUNCPLUGINCMD _pFunc);
void doUpdateCapsSeq();
void DockableDlgDemo();
void doAboutDlg();
void doHideSelectedOrAllLines();
void doShowSelectedOrAllLines();
void doInvertSelectedOrAllLines();
void doInsertSequence();
void doSequenceStart();
void doSequenceNext();
void doSequenceRest();
void doSequenceAll();
void doSelectedAsSequence();
#endif //PLUGINDEFINITION_H