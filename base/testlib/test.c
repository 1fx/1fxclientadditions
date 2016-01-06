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
