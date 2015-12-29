@set include=

del /q vm
mkdir vm
cd vm

set cc=sof2lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui %1

%cc% ../ui_main.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_misc.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_player.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_weapons.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_lib.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_gametype.c
@if errorlevel 1 goto quit
%cc% ../../game/q_math.c
@if errorlevel 1 goto quit
%cc% ../../game/q_shared.c
@if errorlevel 1 goto quit
%cc% ../ui_atoms.c
@if errorlevel 1 goto quit
%cc% ../ui_players.c
@if errorlevel 1 goto quit
%cc% ../ui_shared.c
@if errorlevel 1 goto quit
%cc% ../ui_gameinfo.c
@if errorlevel 1 goto quit

%cc% ../1fx_misc.c
@if errorlevel 1 goto quit
%cc% ../1fx_qvmfuncs.c
@if errorlevel 1 goto quit

sof2asm -f ../ui
@if errorlevel 1 goto quit

mkdir "..\..\..\base\1fx\vm"
copy *.map "..\..\..\base\1fx\vm"
copy *.qvm "..\..\..\base\1fx\vm"

:quit
pause
cd ..
