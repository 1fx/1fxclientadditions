// Copyright (C) 2015 - Boe!Man, Henkie.
//
// 1fx_local.h - All the Core UI (QVM and DLL) definitions go here.

//==================================================================

#ifndef _1FX_LOCAL
#define _1FX_LOCAL

//
// 1fx_httpdownload.c
//

#define HTTPDL_BASEURL "http://1fxmod.org/clientadditions"

#ifndef Q3_VM
typedef enum {
	HTTPDL_CANCEL 		= -1,
	HTTPDL_IDLE 		= 0,
	HTTPDL_DOWNLOADING 	= 1,
	HTTPDL_FINISHED 	= 2
} httpDownloadState_t;

extern int httpDLStatus;

void	_1fx_httpDL_initialize	( void );
void 	_1fx_joinHTTPThread		( void );
#endif // not Q3_VM

//
// 1fx_qvmfuncs.c
//

#ifdef Q3_VM
#define CHUNK_SIZE 131072 // Write in chunks of 128K to avoid exceeding the temporary memory buffer limit.

void 	_1fx_coreUI_checkDLL 	( void );
#endif // Q3_VM

#endif // _1FX_LOCAL
