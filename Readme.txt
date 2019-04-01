BF707_Wagner:
-------------

This project runs on a BF707 evaluation board connected to a Gordon chip via
SPI on the "RF WIRELESS" connector.

The "USB TO UART" connector is used as a debugging console.


Note:
-----

At this time, only the "Debug" configuration is fonctionnal.

The script "ProgramToFlash.bat" can be used to program the binary in the serial Flash.


Instructions to flash:
----------------------

1) Unzip the bf707_w25q32bv_dpia.7z file somewhere out of the Wagner repository.
2) In the Wagner repository, edit the files ProgramToFlash.bat and ReadFromFlash.bat:
	2.1) Change the CCESDir PATH to match your CrossCore installation.
	2.2) Change the DriverPath PATH to where the bf707_w25q32bv_dpia.dxe is unzipped.
3) Build the Wagner project using CrossCore Embedded Studio
4) Execute the ProgramToFlash.bat file to flash the Blackfin.