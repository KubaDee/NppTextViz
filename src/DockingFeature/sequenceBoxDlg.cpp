//this file is part of notepad++
//Copyright (C)2003 Don HO ( donho@altern.org )
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

#include "sequenceBoxDlg.h"
#include "..\PluginDefinition.h"

extern NppData nppData;

#define rcwidth(rc) (rc.right - rc.left)
#define rcheight(rc) (rc.bottom - rc.top)
void move_resize(HWND child, int dx, int dy, int dw, int dh)
{
	if (!child) return;
	if (!IsWindow(child)) return;
	if (!GetParent(child)) return;

	//find child window's coordinates relative to top-left of parent:

	RECT rc;
	GetWindowRect(child, &rc);
	//rc is now child control's rectangle in screen coordinates

	POINT pt = { 0 };
	ScreenToClient(GetParent(child), &pt);
	OffsetRect(&rc, pt.x, pt.y);
	//rc is now child control's rectangle relative to parent window

	//prevent negative size
	if ((rcwidth(rc) + dw) < 0) dw = -rcwidth(rc);
	if ((rcheight(rc) + dh) < 0) dh = -rcheight(rc);

	MoveWindow(child, rc.left + dx, rc.top + dy, rcwidth(rc) + dw, rcheight(rc) + dh, TRUE);
}

INT_PTR CALLBACK SequenceBoxDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwDlgItem = GetDlgItem(_hSelf, ID_SEQUENCE_EDIT);
	static RECT save_rect;

	switch (message)
	{
		case WM_COMMAND : 
		{
			switch (wParam)
			{
				case IDSET : // set text box sequence as global sequence
				{
					TCHAR * tcsSequence = NULL;
					tcsSequence = getSequence();
					if (tcsSequence != NULL) {
						SetSequence(tcsSequence);
						return true;
					}
					else {
						return false;
					}
					//break;
				}

				case IDGET: // read sequence and put it into text box
				{
					setSequence(GetSequence());
					return true;
				}

				case IDRUNALL: // run all steps from first to last
				{
					doSequenceAll();
					return true;
				}

				case IDRUNFIRST: // run first stop and then pause
				{
					doSequenceStart();
					return true;
				}

				case IDRUNNEXT: // run next step
				{
					doSequenceNext();
					return true;
				}

				case IDRUNREST: // run rest of sequence
				{
					doSequenceRest();
					return true;
				}

				case ID_SETSELECTED: // copy selected as sequence
				{
					doSelectedAsSequence();
					return true;
				}
				
				case ID_CLEAR: // clear sequence
				{
					doSequenceClear();
					return true;
				}

				case ID_SHOWALL: // show all lines
				{
					doShowAllLines();
					return true;
				}

				case ID_CHECKEDIT: // enable/diasble button + text box
				{
					setInvertEditButton();
					return true;
				}

				case ID_UPDATE_SEQUENCE_SELECTED: // update selected linex in sequence
				{

					SendMessage(GetDlgItem(_hSelf, ID_SEQUENCE_EDIT), EM_SETSEL, lParam, GetLineBegin() - 1);
					return true;
				}
			}
				return FALSE;
		}

		case WM_SIZE:
			if (lParam && save_rect.right) // hack - waiting for window initialize to get proper window size
			{
				int cx = LOWORD(lParam);
				int cy = HIWORD(lParam);
				int dx = cx - save_rect.right;
				int dy = cy - save_rect.bottom;
				if (cy > 130)	// fix for inner textbox overflow out of main window
				{
					move_resize(hwDlgItem, 0, 0, dx, dy);
					GetClientRect(_hSelf, &save_rect);
				}
				return true;
			}
			GetClientRect(_hSelf, &save_rect);
			return false;

		///
		/*case WM_KEYDOWN:
		{
			if (wParam==1) SendMessage(hwDlgItem, EM_SETSEL, 0, -1);
			switch (wParam)
			{
			case VK_CONTROL:
				//Do your stuff
				SendMessage(hwDlgItem, EM_SETSEL, 0, -1);

				if (GetKeyState(VK_CONTROL) & 0x8000 && wParam == 'A') {
					SendMessage(hwDlgItem, EM_SETSEL, 0, -1);
				}
			}
		}*/
		///

		default :
			return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
	}
}

