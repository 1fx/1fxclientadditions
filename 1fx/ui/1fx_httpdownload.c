// Copyright (C) 2015 - Boe!Man, Henkie.
//
// 1fx_httpdownload.c - All the DLL HTTP download functions go here.

//==================================================================

#ifndef Q3_VM
// Include cURL for clients (in conjunction with pthreads), and the MD5 header for hashing.
#include <curl/curl.h>
#include <pthread.h>
#include <openssl/md5.h>

#include "ui_local.h"
#include "1fx_local.h"

static CURL         *curl;
static pthread_t    downloadThread;
static char         fs_game[MAX_CVAR_VALUE_STRING];
static qboolean     baseChecksComplete  = qfalse;
static qboolean     restartWhenFinished = qfalse;
int                 httpDLStatus        = HTTPDL_IDLE;

/*
==========================
_1fx_httpDL_checkPaks
9/27/15 - 8:50 PM
Checks if there are .pk3 files missing locally
but are available on the remote server.
==========================
*/

static void _1fx_httpDL_checkPaks()
{
    // Check if the server owner made files available to download.
    if(!strlen(ui_httpRefPaks.string) || strlen(ui_httpBaseURL.string)){
        #ifdef _DEBUG
        Com_Printf("[CoreUI_DLL]: No files available to be downloaded (CVARs empty).\n");
        #endif // _DEBUG
        return;
    }
}

/*
==========================
_1fx_httpDL_cURL_write
10/26/15 - 10:17 PM
cURL write callback.
Also updates UI variables.
==========================
*/

size_t _1fx_httpDL_cURL_write(void *ptr, size_t size, size_t nmemb, FILE *f)
{
    size_t written = fwrite(ptr, size, nmemb, f);
	//downloaded += written;

	//curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &bytesDone);
	//curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &speedAvgInBytes);

	if(httpDLStatus == HTTPDL_CANCEL){
        return -1;
	}

	return written;
}

/*
==========================
_1fx_httpDL_getFileChecksum
9/27/15 - 10:16 PM
Gets MD5 of any file.
==========================
*/

static char *_1fx_httpDL_getFileChecksum(char *filename)
{
    FILE            *f;
    MD5_CTX         mdContext;
    int             i;
    unsigned char   data[1024], digest[MD5_DIGEST_LENGTH];
    char            *hash;

    // Open the file.
    f = fopen(filename, "rb");

    // Verify success.
    if(f == NULL){
        return NULL;
    }

    // Go ahead and calculate the checksum.
    // Initialize the MD5 hash structure.
    MD5_Init(&mdContext);

    // Loop while there is data available to hash.
    while ((i = fread(data, 1, 1024, f)) != 0){
        MD5_Update(&mdContext, data, i);
    }

    // Finalize the hash.
    MD5_Final (digest, &mdContext);

    // Close the input file.
    fclose(f);

    // Allocate memory for the output.
    hash = calloc(MD5_DIGEST_LENGTH * 2 + 1, sizeof(char));
    if(hash == NULL){
        return NULL;
    }

    for(i = 0; i < MD5_DIGEST_LENGTH; ++i){
        snprintf(&(hash[i*2]), MD5_DIGEST_LENGTH * 2, "%02x", (unsigned int)digest[i]);
    }

    // Success, return the hash.
    return hash;
}

/*
==========================
_1fx_httpDL_getRemoteChecksum
10/26/15 - 9:37 PM
Gets MD5 of any remote MD5SUM file.
==========================
*/

static char *_1fx_httpDL_getRemoteChecksum(char *url)
{
    size_t  fsize;
    FILE    *f;
    char    *hash;
    int     res;

    // Open the file, both read and write.
    f = fopen(va("%s\\1fx_MD5SUM", fs_game), "wb+");

    // Verify success.
    if(f == NULL){
        return NULL;
    }

    // Go ahead and fetch the checksum.
    // Start cURL routine.
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _1fx_httpDL_cURL_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);

    // Get the remote file.
    res = curl_easy_perform(curl);

    // Get the file size.
    fsize = ftell(f);

    // Verify if the download went correctly,
    // and if this could be a MD5 hash.
    if(res != CURLE_OK || fsize != MD5_DIGEST_LENGTH * 2 + 1){
        fclose(f);
        return NULL;
    }

    // Now let's re-wind the file to read it.
    fseek(f, 0, SEEK_SET);

    // Allocate memory for the hash.
    hash = calloc(MD5_DIGEST_LENGTH * 2 + 1, sizeof(char));

    // Read the contents to the buffer.
    fread(hash, sizeof(char), MD5_DIGEST_LENGTH * 2, f);

    // Close the file, and return the hash.
    fclose(f);

    return hash;
}

/*
==========================
_1fx_httpDL_getRemoteFile
10/28/15 - 1:46 PM
Gets any remote file.
==========================
*/

static qboolean _1fx_httpDL_getRemoteFile(char *url, char *destination)
{
    FILE    *f;
    int     res, fsize;
    char    fileBuffer[4096];

    // Open the file in write mode.
    f = fopen(destination, "wb+");

    // Verify success.
    if(f == NULL){
        return qfalse;
    }

    // Go ahead and fetch the checksum.
    // Start cURL routine.
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _1fx_httpDL_cURL_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);

    // Get the remote file.
    res = curl_easy_perform(curl);

    // Check success.
    if(res != CURLE_OK){
        // Try to remove the incorrectly fetched file.
        fclose(f);
        DeleteFile(TEXT(destination));

        return qfalse;
    }

    // Have some sort of verification here.
    // If the file was not found on the server, this file will probably contain a 404 error page.
    // So check the file for some HTML contents, and if found, this file is still invalid.

    // Get the file size.
    fsize = ftell(f);

    // Re-wind the file to read it.
    fseek(f, 0, SEEK_SET);

    // Read as a maximum 4KB of the output file.
    // This should be enough to determine the validness of the file.
    res = fread(fileBuffer, sizeof(char), (fsize >= 4096) ? 4095 : fsize, f);
    fileBuffer[res] = '\0';

    // Close the file.
    fclose(f);

    // Determine if this file was valid.
    // Check for some contents.
    if(strstr(fileBuffer, "<html>")){
        DeleteFile(TEXT(destination));
        #ifdef _DEBUG
        Com_Printf("[CoreUI_DLL]: Incorrectly downloaded file (contained HTML, so possibly a 404 or 500 web page).\n");
        #endif // _DEBUG

        return qfalse;
    }

    return qtrue;
}

/*
==========================
_1fx_httpDL_checkModFile
10/26/15 - 6:24 PM
Checks a local Mod file,
and if out of sync from the
remote one, updates it.
==========================
*/

static void _1fx_httpDL_checkModFile(char *filename, qboolean restartRequired)
{
    char *localChecksum;
    char *remoteChecksum;

    // Check if the 1fx. Mod website has an update available for us.
	// Calculate local checksum.
	if(!strcmp(filename, "CoreUI_update.dll")){
        localChecksum = _1fx_httpDL_getFileChecksum(va("%s\\sof2mp_uix86.dll", fs_game));
    }else{
        localChecksum = _1fx_httpDL_getFileChecksum(va("%s\\%s", fs_game, filename));
    }

    #ifdef _DEBUG
    if(localChecksum != NULL){
        Com_Printf("[CoreUI_DLL]: Calculated local hash (%s): %s\n", filename, localChecksum);
    }else{
        Com_Printf("[CoreUI_DLL]: No such local file: %s\n", filename);
    }
    #endif // _DEBUG

    remoteChecksum = _1fx_httpDL_getRemoteChecksum(va("%s/%s/%s.MD5SUM", HTTPDL_BASEURL, fs_game, filename));

    #ifdef _DEBUG
    if(remoteChecksum != NULL){
        Com_Printf("[CoreUI_DLL]: Received remote hash (%s): %s\n", filename, remoteChecksum);
    }else{
        Com_Printf("[CoreUI_DLL]: No such remote file: %s\n", filename);
    }
    #endif // _DEBUG

    // Check if there's an update required.
    if(remoteChecksum && (!localChecksum || (localChecksum && strcmp(localChecksum, remoteChecksum)))){
        #ifdef _DEBUG
        Com_Printf("[CoreUI_DLL]: Update required for file: %s\n", filename);
        #endif // _DEBUG

        // Fetch file and verify success,
        // and check if this was a critical file that needs a restart into QVM to install.
        if(_1fx_httpDL_getRemoteFile(va("%s/%s/%s", HTTPDL_BASEURL, fs_game, filename), va("%s\\%s", fs_game, filename)) && restartRequired){
            restartWhenFinished = qtrue;
        }
    }

    // Free allocated memory.
    if(localChecksum != NULL){
        free(localChecksum);
    }
    if(remoteChecksum != NULL){
        free(remoteChecksum);
    }
}

/*
==========================
_1fx_httpDL_mainDownloader
9/27/15 - 9:08 PM
Main HTTP download routine to execute when
initializing the downloader.
==========================
*/

static void *_1fx_httpDL_mainDownloader()
{
    #ifdef _DEBUG
    Com_Printf("[CoreUI_DLL]: Main downloader thread initialized.\n");
    #endif // _DEBUG

    // Start initializing cURL.
    curl = curl_easy_init();
	if(!curl){
        Com_Printf("ERROR: Couldn't initialize cURL in the 1fx. HTTP downloader.\n");
        Com_Printf("Please e-mail the given error to boe@1fxmod.org\n");
        Com_Printf("Aborting.\n");

        httpDLStatus = HTTPDL_FINISHED;
        return NULL;
	}

	_1fx_httpDL_checkModFile("CoreUI_update.dll", qtrue);
	_1fx_httpDL_checkModFile("sof2mp_cgamex86.dll", qtrue);

	// Cleanup cURL.
    curl_easy_cleanup(curl);

    // Delete temporary MD5SUM file.
    DeleteFile(TEXT(va("%s\\1fx_MD5SUM", fs_game)));

    #ifdef _DEBUG
    Com_Printf("[CoreUI_DLL]: Main downloader thread finished.\n");
    #endif // _DEBUG
}

/*
==========================
_1fx_httpDL_initialize
9/27/15 - 8:01 PM
Initializes the HTTP download thread.
==========================
*/

void _1fx_httpDL_initialize()
{
    int errnum;

    // Fetch the fs_game CVAR value, and store it locally.
    trap_Cvar_VariableStringBuffer("fs_game", fs_game, sizeof(fs_game));

    // Start the thread and start iterating through the files.
    errnum = pthread_create(&downloadThread, NULL, _1fx_httpDL_mainDownloader, NULL);
    if(errnum){
        Com_Printf("ERROR: Failed to initialize the 1fx. HTTP downloader even though there are files available.\n");
        Com_Printf("Please e-mail the following error code to boe@1fxmod.org -> %d\n", errnum);
        Com_Printf("Aborting.\n");

        httpDLStatus = HTTPDL_FINISHED;
        return;
    }

    httpDLStatus = HTTPDL_DOWNLOADING;
    #ifdef _DEBUG
    Com_Printf("[CoreUI_DLL]: Initialized HTTP download thread.\n");
    #endif // _DEBUG
}

/*
==========================
_1fx_joinHTTPThread
10/26/15 - 5:15 PM
Joins the HTTP download thread
prior to shutting down.
==========================
*/

void _1fx_joinHTTPThread()
{
    // Give thread cancel signal if still running.
    if(httpDLStatus == HTTPDL_DOWNLOADING){
        httpDLStatus = HTTPDL_CANCEL;
    }

	pthread_join(downloadThread, NULL);
}
#endif // not Q3_VM
