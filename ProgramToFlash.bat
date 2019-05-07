REM @echo off

REM IF "%1"=="" GOTO SKIP
REM IF "%2"=="" GOTO SKIP

REM pushd %~dp0

set TargetName=demo_app
set Configuration=Debug
set TargetExt=.ldr
set CCESDir=c:\Analog Devices\CrossCore Embedded Studio 2.8.3
set DriverPath=D:\Projects\Wagner\flash_programmer\flash_programmer.dxe
set configFile=wagner_custom_board_support.xml

ECHO Programming %TargetName% to SPI Flash

"%CCESDir%\cldp.exe" -proc ADSP-BF707 -emu 1000 -driver "%DriverPath%" -cmd prog -erase affected -format hex -file %TargetName%\%Configuration%\%TargetName%%TargetExt% -verbose -log 
REM "%CCESDir%\cldp.exe" -proc ADSP-BF707 -emu 1000 -driver "%DriverPath%" -cmd info -verbose -log 
REM popd

:DONE
