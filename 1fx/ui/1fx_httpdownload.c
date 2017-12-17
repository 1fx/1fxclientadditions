// Copyright (C) 2015 - Boe!Man, Henkie.
//
// 1fx_httpdownload.c - All the DLL HTTP download functions go here.

//==================================================================

#ifndef Q3_VM
// Include cURL for clients (in conjunction with pthreads), and the MD5 header for hashing.
#include <curl/curl.h>
#include <pthread.h>
#include <openssl/md5.h>
#include "Shlwapi.h"

#include "ui_local.h"
#include "1fx_local.h"

static CURL             *curl;
static pthread_t        downloadThread;
static char             fs_game[MAX_CVAR_VALUE_STRING];
static char             fs_cdpath[MAX_CVAR_VALUE_STRING];
static qboolean         coreSecondaryLocation;

static double           timeSinceNoProgress             = -1;
static curl_off_t       downloadedBytes                 = 0;

httpDownloadLocals_t    httpDL                          = {0};

/*
==========================
_1fx_httpDL_cURL_write
10/26/15 - 10:17 PM
cURL write callback.
Also updates UI variables.
==========================
*/

static size_t _1fx_httpDL_cURL_write(void *ptr, size_t size, size_t nmemb, FILE *f)
{
    size_t written = fwrite(ptr, size, nmemb, f);

	curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &httpDL.bytesReceived);
	curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &httpDL.speedAvg);

	if(httpDL.httpDLStatus == HTTPDL_CANCEL){
        return -1;
	}

	return written;
}

/*
==========================
_1fx_httpDL_cURL_writeEmpty
10/26/15 - 10:17 PM
cURL write callback that
doesn't actually perform
any write.
==========================
*/

static size_t _1fx_httpDL_cURL_writeEmpty(void *ptr, size_t size, size_t nmemb, void *data)
{
  (void)ptr;
  (void)data;

  // Only return the size we would have saved.
  return (size_t)(size * nmemb);
}

/*
==========================
_1fx_httpDL_cURL_checkProgress
11/4/15 - 4:20 PM
cURL progress callback.
We use this to cancel a
download that is unresponsive.
==========================
*/

static size_t _1fx_httpDL_cURL_checkProgress(void *ptr, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    double currentTime;

    // Get the current time.
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &currentTime);

    // Ideally every time we call this function we've progressed a little bit.
    if(dlnow != downloadedBytes || timeSinceNoProgress == -1){
        timeSinceNoProgress = currentTime;
        downloadedBytes = dlnow;
    }

    // Cancel the download after 5 seconds with no progress.
    if(currentTime - timeSinceNoProgress > 4 || httpDL.httpDLStatus == HTTPDL_CANCEL){
        return 1;
    }

    return 0;
}

/*
==========================
_1fx_httpDL_getFileChecksum
3/20/16 - 4:45 PM
Determines the location
of core DLL files.
==========================
*/

static qboolean _1fx_httpDL_useSecondaryLocation()
{
    return !PathFileExists(va("%s\\sof2mp_uix86.dll", fs_game));
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
    double  remoteFileSize;

    // Open the file for writing.
    f = fopen(va("%s\\1fx_MD5SUM", fs_game), "wb");

    // Verify success.
    if(f == NULL){
        return NULL;
    }

    // Go ahead and fetch the checksum.
    // Start cURL routine.
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);

    // Grab the file size from the header first.
    curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    timeSinceNoProgress = -1;

    // Get the remote header.
    res = curl_easy_perform(curl);

    // Fetch the download size from the retrieved info.
    curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &remoteFileSize);

    // Verify retrieved header info.
    if(res != CURLE_OK || remoteFileSize != MD5_DIGEST_LENGTH * 2 + 1){
        // This probably means the server returned a >= 400 HTTP error.
        // Most likely the file is not present on the remote server.
        // Or the remote file isn't a valid MD5SUM file.
        fclose(f);
        return NULL;
    }

    // Premature checks seem fine, go ahead and download the real file.
    // Reset some cURL options.
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
    timeSinceNoProgress = -1;

    // Reopen the file in write + read mode.
    freopen(va("%s\\1fx_MD5SUM", fs_game), "wb+", f);
    if(f == NULL){
        return NULL;
    }

    // Perform actual download.
    res = curl_easy_perform(curl);

    // Get the size of the retrieved data.
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

static qboolean _1fx_httpDL_getRemoteFile(char *url, char *destination, char *pakName)
{
    FILE    *f;
    int     res;
    char    destBuf[MAX_PATH];
    int     fsize;

    // Make a copy of the destination filename.
    Q_strncpyz(destBuf, destination, sizeof(destBuf));

    // Open the file in write mode.
    f = fopen(destBuf, "wb");

    // Verify success.
    if(f == NULL){
        return qfalse;
    }

    if(ui_1fxAdditionsVerbose.integer){
        Com_Printf("[CoreUI_DLL]: Trying to downloaded file: %s\n", url);
    }

    // Start cURL routine.
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);

    // Grab the file size from the header first.
    curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    httpDL.initializingDl = qtrue;
    timeSinceNoProgress = -1;

    // Let the UI know we're downloading a file.
    httpDL.pakName = pakName;

    // Get the remote header.
    res = curl_easy_perform(curl);

    // Fetch the download size from the retrieved info.
    curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &httpDL.pakSize);

    // Verify retrieved header info.
    if(res != CURLE_OK){
        // This probably means the server returned a >= 400 HTTP error.
        // Most likely the file is not present on the remote server.
        // Or the remote file isn't a valid MD5SUM file.
        if(ui_1fxAdditionsVerbose.integer){
            Com_Printf("[CoreUI_DLL]: Getting file headers failed: %s.\n", curl_easy_strerror(res));
        }
        fclose(f);
        DeleteFile(destBuf);
        return qfalse;
    }

    // Premature checks seem fine, go ahead and download the real file.
    // Reset some cURL options.
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
    httpDL.initializingDl = qfalse;
    timeSinceNoProgress = -1;

    // Reopen the file to ensure no bogus data is left.
    freopen(destBuf, "wb", f);
    if(f == NULL){
        DeleteFile(destBuf);
        return qfalse;
    }

    // Get the remote file.
    res = curl_easy_perform(curl);

    // Get the size of the retrieved data.
    fsize = ftell(f);

    // Close the file.
    fclose(f);

    // Cleanup download info.
    httpDL.pakName = NULL;
    httpDL.pakSize = -1;

    // Check success.
    if(res != CURLE_OK || fsize == 0){
        // Try to remove the incorrectly fetched file.
        DeleteFile(destBuf);
        return qfalse;
    }

    if(ui_1fxAdditionsVerbose.integer){
        Com_Printf("[CoreUI_DLL]: Successfully downloaded file: %s\n", pakName);
    }

    return qtrue;
}

/*
==========================
_1fx_httpDL_getRemoteFileList
1/2/16 - 10:37 AM
Gets a remote file list,
if present and specified.
==========================
*/

static char *_1fx_httpDL_getRemoteFileList(char *baseURL, char *paks)
{
    FILE    *f;
    char    *iPaks = NULL;
    char    fileExtension[5];
    size_t  fsize;

    // If the referenced paks contain a space, this can't be a file list.
    if(strstr(paks, " ")){
        return NULL;
    }

    // The server owner should redirect us to a .txt file with the containing paks,
    // or this is not a file list.
    Q_strncpyz(fileExtension, paks + strlen(paks) - 4, sizeof(fileExtension));
    if(Q_stricmp(fileExtension, ".txt") != 0){
        return NULL;
    }

    // Try to download the file list.
    if(!_1fx_httpDL_getRemoteFile(va("%s%s", baseURL, paks), va("%s\\%s.tmp", fs_game, paks), paks)){
        return NULL;
    }

    // Move to the new location and remove the old file.
    MoveFile(va("%s\\%s.tmp", fs_game, paks), va("%s\\%s", fs_game, paks));
    DeleteFile(va("%s\\%s.tmp", fs_game, paks));

    // Open the file in read mode.
    f = fopen(va("%s\\%s", fs_game, paks), "r");

    // Verify success.
    if(f == NULL){
        DeleteFile(va("%s\\%s", fs_game, paks));
        return NULL;
    }

    // Get the size of the retrieved data.
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Not valid if the file is smaller than 5 bytes.
    // For the sake of safety we maintain a max of 16KB.
    if(fsize < 5 || fsize > 16384){
        fclose(f);
        DeleteFile(va("%s\\%s", fs_game, paks));
        return NULL;
    }

    // Allocate memory.
    iPaks = calloc(fsize, sizeof(char));
    if(iPaks == NULL){
        fclose(f);
        DeleteFile(va("%s\\%s", fs_game, paks));
        return NULL;
    }

    // Read the first line from the file.
    if(fgets(iPaks, fsize, f) == NULL){
        fclose(f);
        DeleteFile(va("%s\\%s", fs_game, paks));
        free(iPaks);
        return NULL;
    }

    // Remove newline characters from the fetched line.
    if(iPaks[strlen(iPaks) - 2] == '\r'){
        iPaks[strlen(iPaks) - 2] = '\0';
    }else if(iPaks[strlen(iPaks) - 1] == '\n'){
        iPaks[strlen(iPaks) - 1] = '\0';
    }

    // Close the file, and delete it.
    fclose(f);
    DeleteFile(va("%s\\%s", fs_game, paks));

    return iPaks;
}

/*
==========================
_1fx_httpDL_replaceModFile
10/30/15 - 3:30 PM
Updates Mod file with
downloaded *.tmp file.
==========================
*/

static void _1fx_httpDL_replaceModFile(char *filename, char *remoteChecksum, qboolean isDLL)
{
    char fullFilename[MAX_PATH];
    char *newChecksum;

    // Make a copy of the file name with the fs_game CVAR path embedded,
    // or with also the fs_cdpath CVAR path embedded.
    if(!isDLL || !coreSecondaryLocation){
        Q_strncpyz(fullFilename, va("%s\\%s", fs_game, filename), sizeof(fullFilename));
    }else{
        Q_strncpyz(fullFilename, va("%s\\%s\\%s", fs_cdpath, fs_game, filename), sizeof(fullFilename));
    }

    // Don't just overwrite the original file in case it's still in use. Rename it first.
    if(!MoveFile(fullFilename, va("%s.bak", fullFilename))){
        if(ui_1fxAdditionsVerbose.integer){
            Com_Printf("[CoreUI_DLL]: Couldn't move mod file to .bak.\n");
        }
        return;
    }

    // Made a backup of the original file (and possibly file handle).
    // Now move the .tmp file to the original file location.
    if(!MoveFile(va("%s.tmp", fullFilename), fullFilename)){
        if(ui_1fxAdditionsVerbose.integer){
            Com_Printf("[CoreUI_DLL]: Couldn't move update file to mod file\n.");
        }

        // Restore old file.
        // Don't check it here, worst case is that this fails and CoreUI QVM will restore the original one.
        MoveFile(va("%s.bak", fullFilename), fullFilename);
        return;
    }

    // Success moving file to .bak and moving .tmp to file,
    // now verify checksum.
    newChecksum = _1fx_httpDL_getFileChecksum(fullFilename);

    // Verify the downloaded package.
    if(newChecksum == NULL || strcmp(newChecksum, remoteChecksum)){
        if(ui_1fxAdditionsVerbose.integer){
            Com_Printf("[CoreUI_DLL]: Checksum didn't match of new file.\n");
        }

        // Restore old file.
        DeleteFile(fullFilename);
        MoveFile(va("%s.bak", fullFilename), fullFilename);

        // Free possibly allocated memory and return.
        if(newChecksum != NULL){
            free(newChecksum);
        }
        return;
    }

    if(ui_1fxAdditionsVerbose.integer){
        Com_Printf("[CoreUI_DLL]: Checksum matched of new file, success!\n");
    }
    free(newChecksum);
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

static void _1fx_httpDL_checkModFile(char *filename, qboolean isDLL)
{
    char    *localChecksum;
    char    *remoteChecksum;
    char    fullFilename[MAX_PATH];

    // Determine the full filename.
    if(!isDLL || !coreSecondaryLocation){
        Q_strncpyz(fullFilename, va("%s\\%s", fs_game, filename), sizeof(fullFilename));
    }else{
        // Only for DLLs in the secondary core location.
        Q_strncpyz(fullFilename, va("%s\\%s\\%s", fs_cdpath, fs_game, filename), sizeof(fullFilename));
    }

    // First off, check if there's an old backup left. Remove that now.
    if(PathFileExists(va("%s.bak", fullFilename))){
        DeleteFile(va("%s.bak", fullFilename));
    }

    // Check if the 1fx. Mod website has an update available for us.
    // Calculate local checksum.
    localChecksum = _1fx_httpDL_getFileChecksum(fullFilename);

    if(ui_1fxAdditionsVerbose.integer){
        if(localChecksum != NULL){
            Com_Printf("[CoreUI_DLL]: Calculated local hash (%s): %s\n", filename, localChecksum);
        }else{
            Com_Printf("[CoreUI_DLL]: No such local file: %s\n", filename);
        }
    }

    remoteChecksum = _1fx_httpDL_getRemoteChecksum(va("%s/%s/%s/%s.MD5SUM", HTTPDL_BASEURL, SOF2_VERSION_ID, fs_game, filename));

    if(ui_1fxAdditionsVerbose.integer){
        if(remoteChecksum != NULL){
            Com_Printf("[CoreUI_DLL]: Received remote hash (%s): %s\n", filename, remoteChecksum);
        }else{
            Com_Printf("[CoreUI_DLL]: No such remote file: %s\n", filename);
        }
    }

    // Check if there's an update required.
    if(remoteChecksum && (!localChecksum || (localChecksum && strcmp(localChecksum, remoteChecksum)))){
        if(ui_1fxAdditionsVerbose.integer){
            Com_Printf("[CoreUI_DLL]: Update required for file: %s\n", filename);
        }

        // Fetch file and verify success.
        if(PathFileExists(fullFilename)){
            if(_1fx_httpDL_getRemoteFile(va("%s/%s/%s/%s", HTTPDL_BASEURL, SOF2_VERSION_ID, fs_game, filename), va("%s.tmp", fullFilename), filename)){
                // We can replace the original file now with the downloaded file.
                _1fx_httpDL_replaceModFile(filename, remoteChecksum, isDLL);
            }
        }else{
            // First time we download this file. Save it to the proper location immediately.
            // If this happens to fail, no worries, next update the checksum won't match,
            // forcing a re-download.
            _1fx_httpDL_getRemoteFile(va("%s/%s/%s/%s", HTTPDL_BASEURL, SOF2_VERSION_ID, fs_game, filename), fullFilename, filename);

            if(ui_1fxAdditionsVerbose.integer){
                Com_Printf("[CoreUI_DLL]: Initial download complete of file: %s\n", filename);
            }
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
_1fx_httpDL_checkExtraPaks
9/27/15 - 8:50 PM
Checks if there are extra .pk3 files missing
locally but are available on the remote server.
==========================
*/

static void _1fx_httpDL_checkExtraPaks()
{
	char        *s, *iPaks, *fileExtension;
	char        baseURL[MAX_CVAR_VALUE_STRING], paks[MAX_CVAR_VALUE_STRING], currentPak[MAX_CVAR_VALUE_STRING];
    int         pakLength;

    // Check if the server owner made files available to download.
    if(strlen(ui_httpRefPaks.string) < 5 || strlen(ui_httpBaseURL.string) < 5){
        if(ui_1fxAdditionsVerbose.integer){
            Com_Printf("[CoreUI_DLL]: No extra files available to be downloaded (CVARs invalid/empty).\n");
        }
        return;
    }

    // Make a copy of the base URL, and check if there's a trailing space.
    // If not, append one.
    Q_strncpyz(baseURL, ui_httpBaseURL.string, sizeof(baseURL) - 1);
    if(baseURL[strlen(baseURL) - 1] != '/'){
        strcat(baseURL, "/");
    }

    if(ui_1fxAdditionsVerbose.integer){
        Com_Printf("[CoreUI_DLL]: Trying to download extra .pk3 files from: %s\n", baseURL);
    }

	// Loop through the paks available on the server.
    Q_strncpyz(paks, ui_httpRefPaks.string, sizeof(paks));

    // Check if the specified string should be parsed as file list.
    iPaks = _1fx_httpDL_getRemoteFileList(baseURL, paks);
    if(iPaks != NULL){
        s = iPaks;
    }else{
        s = paks;
    }

    memset(currentPak, 0, sizeof(currentPak));
    do{
        // Determine if there's another package available for download.
        char *nextPak = strstr(s, " ");
        if(nextPak != NULL){
            pakLength = nextPak - s;
        }else{
            pakLength = strlen(s);
            pakLength++;
        }

        // Is our destination buffer full? If so, there have been numerous faulty files.
        if(strlen(currentPak) + pakLength >= sizeof(currentPak)){
            Com_Printf("1fx. HTTP Downloader: invalid file list (did you append .pk3 after the filenames?).\n");
            break;
        }
        // Is the next pak too large to handle?
        if(pakLength >= sizeof(currentPak)){
            // Advance to the next pak.
            s = nextPak;
            continue;
        }

        // We can set the current pak now.
        strncat(currentPak, s, pakLength);

        // We need a valid .pk3 extension to continue.
        if(strlen(currentPak) > 4){
            fileExtension = currentPak + pakLength - 4;

            if(Q_stricmp(fileExtension, ".pk3") == 0){
                if(!PathFileExists(va("%s\\%s", fs_game, currentPak))){
                    // We can download this .pk3 file now.
                    if(_1fx_httpDL_getRemoteFile(va("%s%s", baseURL, currentPak), va("%s\\%s.tmp", fs_game, currentPak), currentPak)){
                        // Move to the new location and remove the old file.
                        // Don't bother too much with this, if it fails we'll download it again the next time.
                        MoveFile(va("%s\\%s.tmp", fs_game, currentPak), va("%s\\%s", fs_game, currentPak));
                        DeleteFile(va("%s\\%s.tmp", fs_game, currentPak));
                    }
                }

                // Don't continue if the user wants to cancel.
                if(httpDL.httpDLStatus == HTTPDL_CANCEL){
                    curl_easy_cleanup(curl);
                    curl_global_cleanup();

                    // Reset last connected server to ensure the downloader starts upon reconnect.
                    trap_Cvar_Set("ui_lastConnectedServer", "");
                    trap_Cvar_Update(&ui_lastConnectedServer);

                    pthread_exit(0);
                }

                // Reset currentPak variable.
                memset(currentPak, 0, sizeof(currentPak));
            }else{
                // No .pk3 file, this file may have spaces in it.
                // Add a space to the end of the buffer - if there's room for it.
                if(strlen(currentPak) + 1 < sizeof(currentPak)){
                    currentPak[strlen(currentPak)] = ' ';
                }
            }
        }

        // Increase to the next package,
        // or break if there's nothing available anymore.
        s = nextPak;
    }while(s != NULL && strlen(s) > 2 && s++);

    if(iPaks != NULL){
        free(iPaks);
    }

    if(ui_1fxAdditionsVerbose.integer){
        Com_Printf("[CoreUI_DLL]: Finished downloading extra .pk3 files.\n");
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
    if(ui_1fxAdditionsVerbose.integer){
        Com_Printf("[CoreUI_DLL]: Main downloader thread initialized.\n");
    }

    // Start initializing cURL.
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

	if(!curl){
        Com_Printf("ERROR: Couldn't initialize cURL in the 1fx. HTTP downloader.\n");
        Com_Printf("Please e-mail the given error to boe@1fxmod.org\n");
        Com_Printf("Aborting.\n");

        httpDL.httpDLStatus = HTTPDL_FINISHED;
        return NULL;
	}

	// Set some global cURL settings here.
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _1fx_httpDL_cURL_write);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, _1fx_httpDL_cURL_writeEmpty);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, _1fx_httpDL_cURL_checkProgress);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "1fx-httpdownloader/" _1FX_CLADD_VER " (Windows)");
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);

    // Is there a speed limit set by the server?
    if(ui_httpMaxSpeed.integer > 0){
        curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, (curl_off_t) 1024 * ui_httpMaxSpeed.integer);
    }

    // Initial pak size is -1 (so we don't get weird time left in seconds).
    httpDL.pakSize = -1;

    // Determine the location of core files.
    coreSecondaryLocation = _1fx_httpDL_useSecondaryLocation();

    Com_Printf("Checking 1fx. Client Additions..\n");

    // Start checking the core Mod files.

    // We never really use this core file,
    // but it's good practice to keep this in
    // sync with the server.
    // Also reference the old paks here, we most definitely
    // don't want to join an old server with the older core pk3 referenced.
    _1fx_httpDL_checkModFile("1fx_coreUI_1.00.pk3", qfalse);
    _1fx_httpDL_checkModFile("1fx_coreUI_1.02.pk3", qfalse);
    _1fx_httpDL_checkModFile("1fx_coreUI_1.10.pk3", qfalse);
    _1fx_httpDL_checkModFile("1fx_coreUI_1.20.pk3", qfalse);
    _1fx_httpDL_checkModFile("1fx_coreUI_1.21.pk3", qfalse);

	_1fx_httpDL_checkModFile("sof2mp_uix86.dll", qtrue);
	_1fx_httpDL_checkModFile("sof2mp_cgamex86.dll", qtrue);

	// Delete temporary MD5SUM file.
    DeleteFile(va("%s\\1fx_MD5SUM", fs_game));

    // Safe to cancel from this point forward, should we?
    if(httpDL.httpDLStatus == HTTPDL_CANCEL){
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        // Reset last connected server to ensure the downloader starts upon reconnect.
        trap_Cvar_Set("ui_lastConnectedServer", "");
        trap_Cvar_Update(&ui_lastConnectedServer);

        pthread_exit(0);
    }

	// Continue by fetching the extra pk3 files exposed to the server.
	httpDL.baseChecksComplete = qtrue;
	Com_Printf("Downloading extra files from the server..\n");
    _1fx_httpDL_checkExtraPaks();

	// Cleanup cURL.
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    if(ui_1fxAdditionsVerbose.integer){
        Com_Printf("[CoreUI_DLL]: Main downloader thread finished.\n");
    }else{
        Com_Printf("1fx. HTTP Downloader finished!\n");
    }

    // Ensure the engine wants to update the FS list.
    trap_Cvar_Set("fs_game", "none");
    trap_Cvar_Set("fs_game", fs_game);

    // Give the signal to reconnect.
    httpDL.httpDLStatus = HTTPDL_FINISHED;
    trap_Cmd_ExecuteText(EXEC_APPEND, va("connect %s\n", ui_lastConnectedServer.string));
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

    // Fetch the fs_game and fs_cdpath CVAR values, and store it locally.
    trap_Cvar_VariableStringBuffer("fs_game", fs_game, sizeof(fs_game));
    trap_Cvar_VariableStringBuffer("fs_cdpath", fs_cdpath, sizeof(fs_cdpath));

    // Start the thread and start iterating through the files.
    errnum = pthread_create(&downloadThread, NULL, _1fx_httpDL_mainDownloader, NULL);
    if(errnum){
        Com_Printf("ERROR: Failed to initialize the 1fx. HTTP downloader even though there are files available.\n");
        Com_Printf("Please e-mail the following error code to boe@1fxmod.org -> %d\n", errnum);
        Com_Printf("Aborting.\n");

        httpDL.httpDLStatus = HTTPDL_FINISHED;
        return;
    }

    httpDL.httpDLStatus = HTTPDL_DOWNLOADING;

	// Boe!Man 10/30/15: Also set last connected server here.
	trap_Cvar_Set("ui_lastConnectedServer", ui_connectedServer.string);
	trap_Cvar_Update(&ui_lastConnectedServer);

    if(ui_1fxAdditionsVerbose.integer){
        Com_Printf("[CoreUI_DLL]: Initialized HTTP download thread.\n");
    }else{
        Com_Printf("Initialized 1fx. HTTP Downloader.\n");
    }
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
    if(httpDL.httpDLStatus == HTTPDL_DOWNLOADING){
        httpDL.httpDLStatus = HTTPDL_CANCEL;
    }

	pthread_join(downloadThread, NULL);
}
#endif // not Q3_VM
