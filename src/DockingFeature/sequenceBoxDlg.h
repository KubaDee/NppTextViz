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

#ifndef SEQUENCEBOX_DLG_H
#define SEQUENCEBOX_DLG_H

#include "DockingDlgInterface.h"
#include "resource.h"

class SequenceBoxDlg : public DockingDlgInterface
{
public :
	SequenceBoxDlg() : DockingDlgInterface(IDD_SEQUENCEBOX){};

    virtual void display(bool toShow = true) const {
        DockingDlgInterface::display(toShow);
        if (toShow)
            ::SetFocus(::GetDlgItem(_hSelf, ID_SEQUENCE_EDIT));
    };

	void setParent(HWND parent2set){
		_hParent = parent2set;
	};

protected :
	virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

private :

    int getLine() const {
        BOOL isSuccessful;
		int line = GetDlgItemInt(_hSelf, ID_SEQUENCE_EDIT, &isSuccessful, FALSE);
        return (isSuccessful?line:-1);
    };

	TCHAR * getSequence() const {
		TCHAR * tcsText;
		HWND hwDlgItem = GetDlgItem(_hSelf, ID_SEQUENCE_EDIT);
		int bufSize = GetWindowTextLength(hwDlgItem);
		tcsText = (TCHAR *) malloc(bufSize * (sizeof(TCHAR) + 1));
		GetDlgItemText(_hSelf, ID_SEQUENCE_EDIT, tcsText, bufSize);
		return tcsText;
	};

	void setSequence( TCHAR * tcsText) {
		SetDlgItemText(_hSelf, ID_SEQUENCE_EDIT, tcsText);
	};

	void setInvertEditButton() {
		BOOL bStatus = (int)SendDlgItemMessage(_hSelf, ID_CHECKEDIT, BM_GETCHECK, 0, 0);
		EnableWindow(GetDlgItem(_hSelf, IDSET), bStatus); // change Button
		SendMessage(GetDlgItem(_hSelf, ID_SEQUENCE_EDIT), EM_SETREADONLY, !bStatus, 0); // change edit text
	};
};

#endif //SEQUENCEBOX_DLG_H
