REM @echo off

REM IF "%1"=="" GOTO SKIP
REM IF "%2"=="" GOTO SKIP

REM pushd %~dp0

set TargetName=BF707_Wagner
set Configuration=Debug
set TargetExt=.ldr
set CCESDir=c:\Analog Devices\CrossCore Embedded Studio 2.8.1
set DriverPath=D:\Projects\sandbox\wagner_ws\bf707_w25q32bv_dpia\Debug\bf707_w25q32bv_dpia.dxe

ECHO Programming %TargetName% to SPI Flash

"%CCESDir%\cldp.exe" -verbose -log -proc ADSP-BF707 -emu 1000 -driver "%DriverPath%" -cmd prog -erase affected -format hex -file %Configuration%\%TargetName%%TargetExt%

REM popd

:DONE
