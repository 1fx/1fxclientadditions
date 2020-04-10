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
// 1fx_qvmfuncs.c - All the QVM Core UI functions go here, e.g. installation and DLL updating.

//==================================================================

#ifdef Q3_VM
#include "ui_local.h"
#include "1fx_local.h"

/*
==========================
_1fx_coreUI_installDLL
9/27/15 - 3:29 PM
Installs the initial Core UI DLL from the .pk3 package,
or updates the Core UI DLL with the one found in the Mod directory.
==========================
*/

static void _1fx_coreUI_installDLL()
{
    fileHandle_t    input, output, testPk3;
    char            *data;
    int             len, len2 = 0, lenChunk = 0;
    int             chunkNum;

    // The base Core UI DLL wasn't detected,
    // so we're going to extract it now from the .pk3.

    // Open the input file, and verify it can be opened properly first before opening the (perhaps existing) DLL for writing.
    len = trap_FS_FOpenFile("install/CoreUI_initial.dll", &input, FS_READ);
    if(!input || !len){
        // Fatal error on initial install.
        Com_Error(ERR_FATAL, "\n\n=====ERROR=====\n" \
                             "The 1fx. Core UI pk3 file is corrupt!\n" \
                             "Remove the 1fx_coreUI_" _1FX_CLADD_VER ".pk3 file from your mod directories and restart the game.");
    }

    // Open the output DLL for writing.
    trap_FS_FOpenFile("sof2mp_uix86.dll", &output, FS_WRITE);
    if(!output){
        char username[MAX_CVAR_VALUE_STRING];
        char cdPath[MAX_CVAR_VALUE_STRING];
        char oldHomePath[MAX_CVAR_VALUE_STRING];

        // We assume this is a problem with the VirtualStore where we may not write DLLs.
        // If the user has *no* write permission on the directory, this initial pk3 could have never been written.
        // the file and re-open the output DLL for writing. We now target our backup location.

        // Copy all relevant engine CVARs into appropriate buffers.
        trap_Cvar_VariableStringBuffer("username", username, sizeof(username));
        trap_Cvar_VariableStringBuffer("fs_cdpath", cdPath, sizeof(cdPath));
        trap_Cvar_VariableStringBuffer("fs_homepath", oldHomePath, sizeof(oldHomePath));

        // Point the homepath to the CD path, so we write the DLL to the CD path instead.
        trap_Cvar_Set("fs_homepath", cdPath);
        trap_FS_FOpenFile("sof2mp_uix86.dll", &output, FS_WRITE);

        // Also write a .pk3 to this location, so we can figure out later on that we've installed a DLL here.
        trap_FS_FOpenFile("secondaryLocation.pk3", &testPk3, FS_WRITE);
        trap_FS_FCloseFile(testPk3);

        // Revert the fs_homepath directory change.
        trap_Cvar_Set("fs_homepath", oldHomePath);

        if(!output){
            // Also cannot write to the backup location. This should, in theory, never happen.
            // First close all open files.
            trap_FS_FCloseFile(input);

            // Bail out with an error.
            Com_Error(ERR_FATAL, va("\n\n=====ERROR=====\n" \
                                    "Couldn't write to your SoF2 OR your AppData directory.\n" \
                                    "Please re-install the game to, for example, your \"%s\" or \"Desktop\" directory.", username));
        }
    }

    // Now loop and write a chunk of the file to avoid the local VM alloc functions being exhausted.
    do{
        // Determine the length of the chunk to write.
        if(len - len2 > CHUNK_SIZE){
            lenChunk = CHUNK_SIZE;
        }else{
            lenChunk = len - len2;
        }
        len2 += lenChunk;

        // Allocate the memory needed to read the entire chunk.
        data = trap_VM_LocalTempAlloc(lenChunk);

        // Read the contents of the input chunk to the buffer.
        trap_FS_Read(data, lenChunk, input);
        data[lenChunk] = '\0';

        // Write the chunk data to the DLL.
        trap_FS_Write(data, lenChunk, output);

        // Free memory allocated.
        trap_VM_LocalTempFree(lenChunk);

        if(ui_1fxAdditionsVerbose.integer){
            Com_Printf("[CoreUI_QVM]: Wrote chunk #%d with a length of %d B.\n", chunkNum, lenChunk);
        }

        // Advance to the next chunk.
        chunkNum++;
    }while(len != len2);

    // Safe to close the input and output file now.
    trap_FS_FCloseFile(input);
    trap_FS_FCloseFile(output);

    if(ui_1fxAdditionsVerbose.integer){
        Com_Printf("[CoreUI_QVM]: Initial Core UI DLL successfully installed.\n");
    }
}

/*
==========================
_1fx_coreUI_checkSecondaryLocation
3/20/16 - 10:38 PM
Checks the secondary location
where the *.DLLs could be installed to.
A regular trap_FS_Read doesn't work
on this directory, so we will list
all available Mods, with a nasty
hack applied, instead.
==========================
*/

static qboolean _1fx_coreUI_checkSecondaryLocation()
{
    char        fs_homepath[MAX_CVAR_VALUE_STRING];
    char        fs_basepath[MAX_CVAR_VALUE_STRING];
    char        fs_cdpath[MAX_CVAR_VALUE_STRING];
    char        fs_game[MAX_CVAR_VALUE_STRING];
    char        modDirectories[4096];
    char*		modDirectoryPtr;
    int         fileLen;
    int         i;
    int         len;

    // First off, we copy all relevant engine CVARs into appropriate buffers.
    trap_Cvar_VariableStringBuffer("fs_homepath", fs_homepath, sizeof(fs_homepath));
    trap_Cvar_VariableStringBuffer("fs_basepath", fs_basepath, sizeof(fs_basepath));
    trap_Cvar_VariableStringBuffer("fs_cdpath", fs_cdpath, sizeof(fs_cdpath));
    trap_Cvar_VariableStringBuffer("fs_game", fs_game, sizeof(fs_game));

    // Set the home- and basepath to the CD path,
    // so other real Mod directories are not detected.
    trap_Cvar_Set("fs_homepath", fs_cdpath);
    trap_Cvar_Set("fs_basepath", fs_cdpath);

    // Build the Mod list.
    len = trap_FS_GetFileList("$modlist", NULL, modDirectories, sizeof(modDirectories));

    // Reset the home- and basepath to their original values.
    trap_Cvar_Set("fs_homepath", fs_homepath);
    trap_Cvar_Set("fs_basepath", fs_basepath);

    // Iterate through the list. If a directory matching our current
    // fs_game is found, no initial installation is needed.
    modDirectoryPtr = modDirectories;
    for ( i = 0; i < len; i++, modDirectoryPtr += fileLen+1){
        // We need to skip this file the next iteration, so remember its length.
        fileLen = strlen(modDirectoryPtr);

        if(ui_1fxAdditionsVerbose.integer){
            Com_Printf("[CoreUI_QVM]: Found mod directory \"%s\" in secondary location...\n", modDirectoryPtr);
        }

        if(!Q_stricmp(fs_game, modDirectoryPtr)){
            if(ui_1fxAdditionsVerbose.integer){
                Com_Printf("[CoreUI_QVM]: MATCHED mod directory \"%s\" in secondary location!\n", modDirectoryPtr);
            }
            return qfalse;
        }
    }

    return qtrue;
}

/*
==========================
_1fx_coreUI_checkDLL
9/27/15 - 3:17 PM
Checks if the Core UI DLL is installed or needs updating.
==========================
*/

void _1fx_coreUI_checkDLL()
{
    int             len;
    fileHandle_t    f;
    qboolean        needInstall;

    // Try to open the file.
    len = trap_FS_FOpenFile("sof2mp_uix86.dll", &f, FS_READ);
    if(!f || !len){
        // Check if we need to install the Mod in the secondary location.
        needInstall = _1fx_coreUI_checkSecondaryLocation();

        if(needInstall){
            if(ui_1fxAdditionsVerbose.integer){
                Com_Printf("[CoreUI_QVM]: DLL not found (or empty), installing...\n");
            }

            // Start installation routine.
            trap_FS_FCloseFile(f);
            _1fx_coreUI_installDLL();
            return;
        }
    }

    // File is present and contains data.
    trap_FS_FCloseFile(f);

    if(ui_1fxAdditionsVerbose.integer){
        Com_Printf("[CoreUI_QVM]: No initial Core UI DLL installation required.\n");
    }
}

#endif // Q3_VM
