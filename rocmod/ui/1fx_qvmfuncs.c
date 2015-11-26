// Copyright (C) 2015 - Boe!Man, Henkie.
//
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
    fileHandle_t    input, output;
    char            *data;
    int             len, len2 = 0, lenChunk = 0;
    #ifdef _DEBUG
    int             chunkNum;
    #endif // _DEBUG

    // The base Core UI DLL wasn't detected,
    // so we're going to extract it now from the .pk3.

    // Open the input file, and verify it can be opened properly first before opening the (perhaps existing) DLL for writing.
    len = trap_FS_FOpenFile("install/CoreUI_initial.dll", &input, FS_READ);
    if(!input || !len){
        // Fatal error on initial install.
        Com_Error(ERR_FATAL, "\n\n=====ERROR=====\nThe 1fx. Core UI pk3 file is corrupt. Remove the 1fx_coreUI_x.x.pk3 file from your mod directories and restart the game (x.x represents a number).");
    }

    // Open the output DLL for writing.
    trap_FS_FOpenFile("sof2mp_uix86.dll", &output, FS_WRITE);
    if(!output){
        Com_Error(ERR_DROP, "Couldn't write to your SoF2 folder. Try to remove your mod directories and restart the game.");
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

        #ifdef _DEBUG
        Com_Printf("[CoreUI_QVM]: Wrote chunk #%d with a length of %d B.\n", chunkNum, lenChunk);

        // Advance to the next chunk.
        chunkNum++;
        #endif // _DEBUG
    }while(len != len2);

    // Safe to close the input and output file now.
    trap_FS_FCloseFile(input);
    trap_FS_FCloseFile(output);

    #ifdef _DEBUG
    Com_Printf("[CoreUI_QVM]: Initial Core UI DLL successfully installed.\n");
    #endif // _DEBUG
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

    // Try to open the file.
    len = trap_FS_FOpenFile("sof2mp_uix86.dll", &f, FS_READ);
    if(!f || !len){
        #ifdef _DEBUG
        Com_Printf("[CoreUI_QVM]: DLL not found (or empty), installing...\n");
        #endif // _DEBUG

        // Start installation routine.
        trap_FS_FCloseFile(f);
        _1fx_coreUI_installDLL();
        return;
    }

    // File is present and contains data.
    trap_FS_FCloseFile(f);

    #ifdef _DEBUG
    Com_Printf("[CoreUI_QVM]: No initial Core UI DLL installation required.\n");
    #endif // _DEBUG
}

#endif // Q3_VM
