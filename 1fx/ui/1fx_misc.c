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
// 1fx_mic.c - Misc functions that aren't related to the HTTP
// download code go here. Universal module usage (QVM & DLL).

//==================================================================

#include "ui_local.h"
#include "1fx_local.h"

/*
==========================
_1fx_misc_shouldInitDownload
12/29/15 - 11:26 PM
Checks if the HTTP download
system should be initialized.
==========================
*/

qboolean _1fx_misc_shouldInitDownload()
{
    fileHandle_t    f;

    // Servers need to be different.
    if(Q_stricmp(ui_connectedServer.string, ui_lastConnectedServer.string) == 0){
        return qfalse;
    }
    // Don't initialize when connecting to localhost without a different port, we're probably not dedicated if this happens.
    if(Q_stricmp(ui_connectedServer.string, "localhost") == 0){
        return qfalse;
    }

    // Demos can have any name, so check if this file exists in the demos folder.
    trap_FS_FOpenFile(va("demos/%s.dm_%d", ui_connectedServer.string, (int)trap_Cvar_VariableValue("protocol")), &f, FS_READ);

    // If we could open the file, we can safely assume it is indeed a demo we're loading.
    if(f){
        trap_FS_FCloseFile(f);
        return qfalse;
    }

    // Meets all criteria, this is a new server.
    return qtrue;
}
