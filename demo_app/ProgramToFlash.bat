REM @echo off

REM IF "%1"=="" GOTO SKIP
REM IF "%2"=="" GOTO SKIP

REM pushd %~dp0

set TargetName=demo_app
set Configuration=Debug
set TargetExt=.ldr
set CCESDir=c:\Analog Devices\CrossCore Embedded Studio 2.8.1
set DriverPath=D:\Projects\Wagner\flash_programmer.dxe

ECHO Programming %TargetName% to SPI Flash

"%CCESDir%\cldp.exe" -proc ADSP-BF707 -emu 1000 -driver "%DriverPath%" -cmd prog -erase affected -format hex -file %Configuration%\%TargetName%%TargetExt% -verbose -log 
REM "%CCESDir%\cldp.exe" -verbose -log -proc ADSP-BF707 -emu 1000 -driver "%DriverPath%" -cmd save -format hex -numbyte 1024 -file d:\savemem5.ldr
REM "%CCESDir%\cldp.exe" -verbose -log -proc ADSP-BF707 -emu 1000 -driver "%DriverPath%" -cmd compare -file %Configuration%\%TargetName%%TargetExt% -format hex 
REM popd

:DONE
