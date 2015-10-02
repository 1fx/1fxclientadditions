// Copyright (C) 2015 - Boe!Man, Henkie.
//
// 1fx_local.h - All the Core UI (QVM and DLL) definitions go here.

//==================================================================

#ifndef _1FX_LOCAL
#define _1FX_LOCAL

//
// 1fx_qvmfuncs.c
//

#ifdef Q3_VM
#define CHUNK_SIZE 131072 // Write in chunks of 128K to avoid exceeding the temporary memory buffer limit.

void _1fx_coreUI_checkDLL ( void );
#endif // Q3_VM

#endif // _1FX_LOCAL
