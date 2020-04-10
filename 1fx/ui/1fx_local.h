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
// 1fx_local.h - All the Core UI (QVM and DLL) definitions go here.

//==================================================================

#ifndef _1FX_LOCAL
#define _1FX_LOCAL

// 1fx. Client Additions version number.
// They are the same for the UI and CGAME module,
// so rebuild both when pushing a new build.
#define _1FX_CLADD_VER "1.21"

//
// 1fx_httpdownload.c
//

#ifndef Q3_VM
#ifndef CGAME
#define HTTPDL_BASEURL	"http://1fxmod.org/clientadditions"
#define MAX_PAK_NAME 	256

typedef enum {
	HTTPDL_CANCEL 		= -1,
	HTTPDL_IDLE 		= 0,
	HTTPDL_DOWNLOADING 	= 1,
	HTTPDL_FINISHED 	= 2
} httpDownloadState_t;

typedef struct {
    int			httpDLStatus;		// Status of the downloader.
    qboolean	baseChecksComplete; // True if base Core UI file checks are complete.

    char		*pakName;			// Name of the file currently downloaded.
    qboolean    initializingDl;     // True when the download is initializing.
    double		pakSize;			// Size of the file currently downloaded.
    double		bytesReceived;		// Bytes already received.
    double		speedAvg;			// Average speed in bytes.
} httpDownloadLocals_t;
extern httpDownloadLocals_t httpDL;

void	    _1fx_httpDL_initialize	        ( void );
void 	    _1fx_joinHTTPThread		        ( void );
#endif // not CGAME
#endif // not Q3_VM

//
// 1fx_misc.c
//

qboolean    _1fx_misc_shouldInitDownload    ( void );

//
// 1fx_qvmfuncs.c
//

#ifdef Q3_VM
#define CHUNK_SIZE 131072 // Write in chunks of 128K to avoid exceeding the temporary memory buffer limit.

void 	    _1fx_coreUI_checkDLL 	        ( void );
#endif // Q3_VM

#endif // _1FX_LOCAL
