/*
===========================================================================
Copyright (C) 2015 - 2020, Ane-Jouke Schat (Boe!Man)
Copyright (C) 2015 - 2020, Jordi Epema (Henkie)

This file is part of the 1fx. Client Additions source code.

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

typedef UINT (CALLBACK* LPFNDLLFUNC1)(UINT,UINT);

int checkEntryPoint(HINSTANCE hDLL, char *name)
{
    LPFNDLLFUNC1 lpfnDllFunc1;    // Function pointer

    lpfnDllFunc1 = (LPFNDLLFUNC1)GetProcAddress(hDLL, name);
    if (!lpfnDllFunc1)
    {
        printf("%s couldn't be found!\n", name);
        return 1;
    }
    else
    {
        // call the function
        printf("%s is VALID!\n", name);
    }

    return 0;
}
int main()
{
    DWORD err;
	HINSTANCE hDLL = LoadLibrary("CoreUI_initial.dll");
	int e;

	if(hDLL != NULL){
		printf("Library has been loaded\n");
    }else{
        err = GetLastError();
		printf("Couldn't load dll CoreUI_initial.dll\n");
		printf("Trying the other one..\n");

		hDLL = LoadLibrary("sof2mp_uix86.dll");
		if(hDLL != NULL){
            printf("Library has been loaded\n");
        }else{
            err = GetLastError();
            printf("Couldn't load dll sof2mp_uix86.dll\n");
        }
	}

	if(hDLL != NULL){
        e = 0;
        e += checkEntryPoint(hDLL, "vmMain");
        e += checkEntryPoint(hDLL, "dllEntry");
        if(e != 0){
            printf("\n\nTHIS DLL IS NOT VALID FOR SOF2!\n");
        }else{
            printf("\n\nThis DLL is all good to be loaded up in SoF2!\n");
        }
        FreeLibrary(hDLL);
	}else{
	    printf("\n\nTHIS DLL IS NOT VALID FOR SOF!\n");
	}

    system("pause");
    return EXIT_SUCCESS;
}
