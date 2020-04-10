# 1fx. Client Additions
The 1fx. Client Additions are additional client features that can be used in
conjunction with 1fx. Mod servers for the v1.03 version of the game. The
additions mainly add support for HTTP downloads and features to bring v1.00
functionality to the v1.03 version of the game, such as third person support
and support for the Hide&Seek and Humans&Zombies game types.

### Overview
The client additions span across both the cgame module (client functionality)
and the ui module (HTTP downloads).

The cgame module can be compiled as both QVM and DLL, but only the compiled DLL
was ever distributed in favor of a slight performance boost. The ui module is
expected to be built as both QVM and DLL and distributed to the user as the
"1fx. Core UI" (as a .pk3 archive). This archive does not contain the cgame
module.

#### Core UI
When a client connects to a 1fx. Mod server and client downloads are enabled,
the client is offered to download the Core UI archive, which consists of both
the QVM and the DLL ui module. The Core UI QVM module will load and will attempt
to install the Core UI DLL module to the file system of the user. If the
installation succeeds, the QVM module will attempt to load the DLL.

This initiates the 1fx. HTTP Downloader, which will in turn download the cgame
module and any updates available. The downloader will then look at the server
and start downloading the .pk3s as specified by the server owner and finish
connecting.

### Additional client features
Next to the HTTP Downloader, the following additional features are added to the
cgame/ui modules:

* Re-enabled the removed v1.00 grenades (M67, F1, L2A2 and MDN11) and their
associated ammo types.
* Client-side recoil control through the *cg_recoilRatio* and
*cg_inaccuracyRatio* CVARs.
* Additional Means of Death methods, such as by pop or burn.
* The forcing of game type skins in certain teams of the additional game types
Hide&Seek and Humans&Zombies.
* Third person support. Whether this is enabled is enforced through the server
using the *g_allowThirdPerson* CVAR.
* Other fixes for compiling properly as DLL module.
* Optional crash logger (only when *_CRASHLOGGER* is defined).

### Client support
The client additions are meant to be included on existing client (cgame/ui)
Mods, and therefore add little to the client experience other than the
aforementioned functionality. This repository contains the following client Mods
(with the 1fx. Client Additions on top of them):

* *Vanilla* - The default SoF2MPSDK v1.03 client.
* *ROCmod* - A slightly modified version of ROCmod 2.1c (to support MinGW-w64).

### Getting started
#### Compiling the cgame module
The cgame module can be built as QVM or DLL. QVMs are usually preferred on
Q3-based games due to their high portability cross-platform, and because they
are easier to build and deploy. With DLLs however, you gain an extra speed boost
in addition to debugging the loaded DLL modules more easily.

Building the cgame module as DLL sacrifices any Mac OS X compatibility, but keep
in mind that the ui module *needs* to be ported to Mac OS X first before that
could work. Since the Mac OS X version was only released for PowerPC-based Macs,
and thus requires Mac OS X 10.1 - 10.5, one could argue this as an acceptable
loss. Because of this, there is currently no support to build the cgame module
as QVM, it is however very possible to build the cgame module as QVM by
restoring the deleted files required to compile the module.

To build the cgame module as **DLL**, you can use the included *cgame.sh* file
in one of the *cgame* source directories to build with MinGW or MinGW-w64, in
conjunction with MSYS(2). You can also use this file to build the cgame module
cross platform, from e.g. Linux, FreeBSD or macOS. Having MinGW or MinGW-w64
installed is also prerequisite for compiling cross-platform. Using Visual Studio
on Windows is also possible, but no project files are present. It is also not
tested or supported.

The following DLL compilation method assumes you're building using MSYS/MinGW32.
This is how you compile the game module as DLL in interactive mode, the script
will ask you what you want to build:

```bash
$ ./cgame.sh
Enter the type of build:
1: Release build
2: Debug build
3: Debug build with crash logger
Enter your choice and press [ENTER]:
```

The script will detect if being run from a Windows host or something else,
and adjust the compiler and linker accordingly.

For more control, you may also pass arguments to the script. The script
supports the following parameters:

* -cc: Specifies compiler name (default: "gcc" on Windows,
"i686-w64-mingw32-gcc" everywhere else).
* -l: Specifies linker name (default: "gcc" on Windows, "i686-w64-mingw32-gcc"
everywhere else).
* -c: Compile choice, either 1, 2, or 3.
* -o: Output filename.

The following *optional* parameters may also be specified:

* -v: Verbose mode, will print e.g. full compile lines.
* -noclear: Don't clear upon selecting choice.

Here's an example of compiling the game module using parameters:

```bash
$ ./cgame.sh -cc gcc -l gcc -c 2 -o /c/SoF2Gold/1fx/sof2mp_cgamex86.dll
```

If compilation fails and you didn't alter anything in the code yet, this is
usually something in the run time not correctly installed or set. Verbose output
is always logged to the *compile_log* file.

#### Compiling the ui module
The ui module must be built as both QVM and DLL.

To build the ui module as **QVM**, and you're familiar with building Mods for
SoF2MP using the SDK, the process is pretty much the same. Make sure the
binutils are in your PATH environment variable and you're ready to compile.
There is a *ui.bat* batch file in one of the *ui* source directories that can be
used to build the ui module as QVM.

Building the ui module as **DLL** is a little more complicated. The DLL depends
on the following libraries (development files of these libraries must be
installed):

* libcurl
* libcrypto
* pthreads-w32

To build the ui module, you can use the included *ui.sh* file in one of the
*ui* source directories to build with MinGW or MinGW-w64, in conjunction with
MSYS(2). The same instructions and restrictions that apply to the cgame module
also apply to the ui module.

In our builds the cURL library doesn't include SSL support. If you want support
for SSL and your cURL supports it, the ui.sh file must be modified to also link
against libssl. At many occasions the cURL library won't allow to be statically
linked. In that case, cURL must be recompiled and properly configured to allow
static linking.

The following DLL compilation method assumes you're building using MSYS/MinGW32.
This is how you compile the game module as DLL in interactive mode, the script
will ask you what you want to build:

```bash
$ ./ui.sh
Enter the type of build:
1: Release build
2: Debug build
Enter your choice and press [ENTER]:
```

The script is a copy of the cgame DLL compilation script and therefore it can
also be used cross-platform and be called with the same arguments.

If compilation fails and you didn't alter anything in the code yet, this is
usually something in the run time not correctly installed or set. Verbose output
is always logged to the *compile_log* file.

### License
The 1fx. Client Additions are licensed under the GNU General Public License
v2.0. You can find a copy of the license in the LICENSE.md file in the
repository root.
