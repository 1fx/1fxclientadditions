// Copyright (C) 2015 - Boe!Man, Henkie.
//
// 1fx_local.h - All the Core UI (QVM and DLL) definitions go here.

//==================================================================

#ifndef _1FX_LOCAL
#define _1FX_LOCAL

// 1fx. Client Additions version number.
// They are the same for the UI and CGAME module,
// so rebuild both when pushing a new build.
#define _1FX_CLADD_VER "0.30"

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

void	_1fx_httpDL_initialize	( void );
void 	_1fx_joinHTTPThread		( void );
#endif // not CGAME
#endif // not Q3_VM

//
// 1fx_qvmfuncs.c
//

#ifdef Q3_VM
#define CHUNK_SIZE 131072 // Write in chunks of 128K to avoid exceeding the temporary memory buffer limit.

void 	_1fx_coreUI_checkDLL 	( void );
#endif // Q3_VM

#endif // _1FX_LOCAL
