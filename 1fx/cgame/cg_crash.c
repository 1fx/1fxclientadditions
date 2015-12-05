// Copyright (C) 2015 - Boe!Man, Henkie.
// Copyright (C) 2011 - ETpub team.
//
// cg_crash.c - Crash log handler for when we need it.

//==================================================================

#include "cg_local.h"
#include "../ui/1fx_local.h"

#ifdef _CRASHLOGGER

#include <windows.h>
#include <process.h>
#include <imagehlp.h>

// Assign types.
typedef BOOL (WINAPI *PFNSYMINITIALIZE)(HANDLE, LPSTR, BOOL);
typedef BOOL (WINAPI *PFNSYMCLEANUP)(HANDLE);
typedef PGET_MODULE_BASE_ROUTINE PFNSYMGETMODULEBASE;
typedef BOOL (WINAPI *PFNSTACKWALK)(DWORD, HANDLE, HANDLE, LPSTACKFRAME, LPVOID, PREAD_PROCESS_MEMORY_ROUTINE, PFUNCTION_TABLE_ACCESS_ROUTINE, PGET_MODULE_BASE_ROUTINE, PTRANSLATE_ADDRESS_ROUTINE);
typedef BOOL (WINAPI *PFNSYMGETSYMFROMADDR)(HANDLE, DWORD, LPDWORD, PIMAGEHLP_SYMBOL);
typedef BOOL (WINAPI *PFNSYMENUMERATEMODULES)(HANDLE, PSYM_ENUMMODULES_CALLBACK, PVOID);
typedef PFUNCTION_TABLE_ACCESS_ROUTINE PFNSYMFUNCTIONTABLEACCESS;

// Function declarations.
LONG CALLBACK   crashHandler    (LPEXCEPTION_POINTERS e);

// Local variables.
fileHandle_t                f                           = 0;
int                         numStackFrames              = 0;
char                        *filename                   = NULL;
char                        *gamepath                   = NULL;
HMODULE                     imagehlp                    = NULL;
PFNSYMINITIALIZE            pfnSymInitialize            = NULL;
PFNSYMCLEANUP               pfnSymCleanup               = NULL;
PFNSYMGETMODULEBASE         pfnSymGetModuleBase         = NULL;
PFNSTACKWALK                pfnStackWalk                = NULL;
PFNSYMGETSYMFROMADDR        pfnSymGetSymFromAddr        = NULL;
PFNSYMENUMERATEMODULES      pfnSymEnumerateModules      = NULL;
PFNSYMFUNCTIONTABLEACCESS   pfnSymFunctionTableAccess   = NULL;

/*
==========================
crashLogger
10/11/15 - 6:27 PM
Logs a string to the crash log,
or log prints it if no file is
currently open.
==========================
*/

void crashLogger(char *s)
{
    if(f){
        trap_FS_Write(s, strlen(s), f);
    }else{
       Com_Printf(s);
    }
}

/*
==========================
getWindowsOSVersion
12/05/15 - 10:38 AM
Gets the Windows OS version
and returns the full name.
==========================
*/

char *getWindowsOSVersion()
{
    OSVERSIONINFOEX osvi;

    // Determine windows OS version.
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    // Boe!Man 12/05/15: Get some basic info.
    if(!GetVersionEx((OSVERSIONINFO *) &osvi)){
        // If the extended info failed, fall back to the smaller structure.
        osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        if(!GetVersionEx((OSVERSIONINFO *) &osvi)){
            return "Unknown";
        }
    }

    // Get the associated name.
    switch(osvi.dwMajorVersion){
        case 5:
            if(osvi.dwMinorVersion == 0){
                return "Win 2K";
            }else if(osvi.dwMinorVersion == 1){
                return "Win XP";
            }else if(osvi.dwMinorVersion == 2){
                if(osvi.wProductType == VER_NT_WORKSTATION){
                    return "Win XP 64";
                }else{
                    return "Server 2003";
                }
            }

            break;
        case 6:
            if(osvi.dwMinorVersion == 0){
                if(osvi.wProductType == VER_NT_WORKSTATION){
                    return "Win Vista";
                }else{
                    return "Server 2008";
                }
            }else if(osvi.dwMinorVersion == 1){
                if(osvi.wProductType == VER_NT_WORKSTATION){
                    return "Win 7";
                }else{
                    return "Server 2008 R2";
                }
            }else if(osvi.dwMinorVersion == 2){
                if(osvi.wProductType == VER_NT_WORKSTATION){
                    return "Win 8 or higher";
                }else{
                    return "Server 2012 or higher";
                }
            }

            break;
        default:
            break;
    }

    return va("Unknown version %d.%d", osvi.dwMajorVersion, osvi.dwMinorVersion);
}

/*
==========================
openCrashLog
10/11/15 - 6:19 PM
Opens a new crash log for writing.
==========================
*/

qboolean openCrashLog()
{
    // Determine current time.
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    static char fileName[FILENAME_MAX];

    // Determine filename.
    Q_strncpyz(fileName, va("crashdumps/%d%02d%02d%02d%02d%02d.txt", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec), sizeof(fileName));
    filename = fileName;

    // Open the file.
    trap_FS_FOpenFile(filename, &f, FS_WRITE);
    Com_Printf("Writing crash log file: %s\n", filename);

    // Return success.
    return f;
}

/*
==========================
closeCrashLog
10/11/15 - 6:20 PM
Closes a previously opened crash log
if one was opened to begin with.
==========================
*/

void closeCrashLog()
{
    if(f){
        trap_FS_FCloseFile(f);
        Com_Printf("Closed crash log file.\n");
        Com_Printf("Please submit the created crash log to the 1fx. Mod developers\n");
        Com_Printf("by e-mailing the generated file to boe@1fxmod.org.\n\n");
    }
}

/*
==========================
logStart
10/13/15 - 6:42 PM
Inform the user the client
has crashed, log the header
to the crash log and write
the generics to file.
==========================
*/

void logStart()
{
    // Inform the user of what just happened.
    Com_Printf("\n-------------- FATAL ERROR ------------\n");
    Com_Printf("--         Game has crashed!         --\n");
    Com_Printf("---------------------------------------\n");
    Com_Printf("Now gathering information regarding the crash.\n");

    // Open the crash log.
    if(!openCrashLog()){
        Com_Printf("Unable to create crashdump (couldn't open file for writing).\n");
        Com_Printf("Writing to logfile or console instead.\n");
    }

    // Write header.
    crashLogger("------------- Crash Report ------------\n");
    crashLogger("--  Please e-mail to boe@1fxmod.org  --\n");
    crashLogger("---------------------------------------\n");
    crashLogger("Client Additions version: " _1FX_CLADD_VER "\n");

    // Log SoF2 version.
    crashLogger(va("SoF2 version: %s\n", GAME_VERSION));

    // Log Operating System.
    crashLogger(va("OS: %s\n", getWindowsOSVersion()));

    // See if we can log the map played.
    // During init this may be empty.
    if(cgs.mapname[0]){
        crashLogger(va("Map: %s\n", cgs.mapname));
    }
}

/*
==========================
logStop
10/13/15 - 6:44 PM
Write footer and try to
throw a fatal error.
==========================
*/

void logStop()
{
    // Write footer and close the crash log.
    crashLogger("---------------------------------------\n");
    closeCrashLog();

    Com_Error(ERR_FATAL, "Game crashed! Please forward the crashdump to boe@1fxmod.org\n\nCrash dump file: %s/%s", gamepath, filename);
}

/*
==========================
enableCrashHandler
-
Main entry for this file,
enables the crash handler.

Original code from ETpub.
==========================
*/

void enableCrashHandler()
{
    // Load the imagehlp library which we need for crash logging.
    imagehlp = LoadLibrary("IMAGEHLP.DLL");
    if(!imagehlp){
        Com_Printf("WARNING: imagehlp.dll unavailable for crash logging.\n");
        return;
    }

    // Check symbols.
    pfnSymInitialize = (PFNSYMINITIALIZE) GetProcAddress(imagehlp, "SymInitialize");
    pfnSymCleanup = (PFNSYMCLEANUP) GetProcAddress(imagehlp, "SymCleanup");
    pfnSymGetModuleBase = (PFNSYMGETMODULEBASE) GetProcAddress(imagehlp, "SymGetModuleBase");
    pfnStackWalk = (PFNSTACKWALK) GetProcAddress(imagehlp, "StackWalk");
    pfnSymGetSymFromAddr = (PFNSYMGETSYMFROMADDR) GetProcAddress(imagehlp, "SymGetSymFromAddr");
    pfnSymEnumerateModules = (PFNSYMENUMERATEMODULES) GetProcAddress(imagehlp, "SymEnumerateModules");
    pfnSymFunctionTableAccess = (PFNSYMFUNCTIONTABLEACCESS) GetProcAddress(imagehlp, "SymFunctionTableAccess");

    // Verify if the DLL is correctly loaded.
    if(
        !pfnSymInitialize ||
        !pfnSymCleanup ||
        !pfnSymGetModuleBase ||
        !pfnStackWalk ||
        !pfnSymGetSymFromAddr ||
        !pfnSymEnumerateModules ||
        !pfnSymFunctionTableAccess
    ){
        FreeLibrary(imagehlp);
        Com_Printf("WARNING: imagehlp.dll is missing exports, and is thus unavailble for crash logging.\n");
        imagehlp = NULL;
        return;
    }

    // It is now safe to install the exception handler.
    SetUnhandledExceptionFilter(crashHandler);
}

/*
==========================
disableCrashHandler
-
Disables the crash handler.

Original code from ETpub.
==========================
*/

void disableCrashHandler()
{
    SetUnhandledExceptionFilter(NULL);
    pfnSymInitialize = NULL;
    pfnSymCleanup = NULL;
    pfnSymGetModuleBase = NULL;
    pfnStackWalk = NULL;
    pfnSymGetSymFromAddr = NULL;
    pfnSymEnumerateModules = NULL;
    pfnSymFunctionTableAccess = NULL;
    FreeLibrary(imagehlp);
}

/*
==========================
ExceptionName
-
Returns exception name.

Original code from ETpub.
==========================
*/

char *ExceptionName(DWORD exceptioncode)
{
    switch (exceptioncode){
        case EXCEPTION_ACCESS_VIOLATION: return "Access violation"; break;
        case EXCEPTION_DATATYPE_MISALIGNMENT: return "Datatype misalignment"; break;
        case EXCEPTION_BREAKPOINT: return "Breakpoint"; break;
        case EXCEPTION_SINGLE_STEP: return "Single step"; break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "Array bounds exceeded"; break;
        case EXCEPTION_FLT_DENORMAL_OPERAND: return "Float denormal operand"; break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "Float divide by zero"; break;
        case EXCEPTION_FLT_INEXACT_RESULT: return "Float inexact result"; break;
        case EXCEPTION_FLT_INVALID_OPERATION: return "Float invalid operation"; break;
        case EXCEPTION_FLT_OVERFLOW: return "Float overflow"; break;
        case EXCEPTION_FLT_STACK_CHECK: return "Float stack check"; break;
        case EXCEPTION_FLT_UNDERFLOW: return "Float underflow"; break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO: return "Integer divide by zero"; break;
        case EXCEPTION_INT_OVERFLOW: return "Integer overflow"; break;
        case EXCEPTION_PRIV_INSTRUCTION: return "Privileged instruction"; break;
        case EXCEPTION_IN_PAGE_ERROR: return "In page error"; break;
        case EXCEPTION_ILLEGAL_INSTRUCTION: return "Illegal instruction"; break;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "Noncontinuable exception"; break;
        case EXCEPTION_STACK_OVERFLOW: return "Stack overflow"; break;
        case EXCEPTION_INVALID_DISPOSITION: return "Invalid disposition"; break;
        case EXCEPTION_GUARD_PAGE: return "Guard page"; break;
        case EXCEPTION_INVALID_HANDLE: return "Invalid handle"; break;
        default: break;
    }

    return "Unknown exception";
}

/*
==========================
retrieveExceptionInfo
-
Retrieve and write exception and
additional exception address to file.

Original code from ETpub.
==========================
*/

void retrieveExceptionInfo(LPEXCEPTION_POINTERS e)
{
    // Log the exception thrown.
    crashLogger(va("Exception: %s (0x%08x)\n", ExceptionName(e->ExceptionRecord->ExceptionCode), e->ExceptionRecord->ExceptionCode));
    crashLogger(va("Exception address: 0x%08x\n", e->ExceptionRecord->ExceptionAddress));
}

/*
==========================
doBacktrace
-
Do a backtrace and retrieve as
much information as possible.

Original code from ETpub.
==========================
*/

void doBacktrace(LPEXCEPTION_POINTERS e)
{
    PIMAGEHLP_SYMBOL    pSym;
    STACKFRAME          sf;
    HANDLE              process, thread;
    DWORD               dwModBase, Disp;
    BOOL                more = FALSE;
    char                moduleName[MAX_PATH] = "";

    // Allocate some memory for the symbols.
    pSym = (PIMAGEHLP_SYMBOL)GlobalAlloc(GMEM_FIXED, 16384);

    // Initialize memory and relevant variables.
    ZeroMemory(&sf, sizeof(sf));
    sf.AddrPC.Offset = e->ContextRecord->Eip;
    sf.AddrStack.Offset = e->ContextRecord->Esp;
    sf.AddrFrame.Offset = e->ContextRecord->Ebp;
    sf.AddrPC.Mode = AddrModeFlat;
    sf.AddrStack.Mode = AddrModeFlat;
    sf.AddrFrame.Mode = AddrModeFlat;

    process = GetCurrentProcess();
    thread = GetCurrentThread();

    crashLogger("Backtrace:\n");

    // Loop through the stack trace.
    while(1){
        more = pfnStackWalk(
            IMAGE_FILE_MACHINE_I386,
            process,
            thread,
            &sf,
            e->ContextRecord,
            NULL,
            pfnSymFunctionTableAccess,
            pfnSymGetModuleBase,
            NULL
        );

        // No more frames available.
        if(!more || sf.AddrFrame.Offset == 0){
            break;
        }

        dwModBase = pfnSymGetModuleBase(process, sf.AddrPC.Offset);

        if(dwModBase){
            GetModuleFileName((HINSTANCE)dwModBase, moduleName, MAX_PATH);
        }else{
            wsprintf(moduleName, "Unknown");
        }

        pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        pSym->MaxNameLength = MAX_PATH;

        // Log as much information as possible.
        if(pfnSymGetSymFromAddr(process, sf.AddrPC.Offset, &Disp, pSym)){
            crashLogger(va("(%d) %s(%s+%#0x) [0x%08x]\n", ++numStackFrames, moduleName, pSym->Name, Disp, sf.AddrPC.Offset));
        }else{
            crashLogger(va("(%d) %s [0x%08x]\n", ++numStackFrames, moduleName, sf.AddrPC.Offset));
        }
    }

    // Finish, also log the number of stack frames processed.
    crashLogger(va("Stack frames: %d entries\n", numStackFrames));

    // Free allocated memory.
    GlobalFree(pSym);
}

/*
==========================
crashHandler
-
This function handles the
exception thrown.

Original code from ETpub.
==========================
*/

LONG CALLBACK crashHandler(LPEXCEPTION_POINTERS e)
{
    char        basePath[MAX_PATH];
    static char gamePath[MAX_PATH];

    // Try to load the symbols.
    trap_Cvar_VariableStringBuffer("fs_basepath", basePath, sizeof(basePath));
    trap_Cvar_VariableStringBuffer("fs_game", gamePath, sizeof(gamePath));
    pfnSymInitialize(GetCurrentProcess(), va("%s\\%s", basePath, gamePath), TRUE);
    gamepath = gamePath;

    // Open the crash log and log the generic information
    // such as header, operating system and such.
    logStart();

    // Log if imagehlp is unavailable on the host system.
    if(imagehlp == NULL){
        crashLogger("imagehlp.dll is missing exports or is unavailable on system.\n");
    }

    // Retrieve what exception we got.
    retrieveExceptionInfo(e);

    // Attempt a backtrace.
    doBacktrace(e);

    // Cleanup symbols.
    pfnSymCleanup(GetCurrentProcess());

    // Log the footer and exit.
    logStop();

    return 1;
}

#endif // _CRASHLOGGER
