

ADAL_6110 Demo Application
--------------------

This project runs on a BlackFin processor connected to a ADAL_6110 chip via
SPI

The "USB TO UART" connector is used as a debugging console.


Note:
--------------------

At this time, only the "Debug" configuration is fonctionnal.

The script "ProgramToFlash.bat" can be used to program the binary in the serial Flash.

Project Description:
--------------------

BF707_init: this is the bootloader of the application. It's base on the reference design with soma small change to support the Wagner board
demo_app: This is the Lidar test application
Flash_programmer: This application is used to generate the flash programmer and support the Flash device on the Wagner board
configdev.py: configuration script that read from a database file to configure a device via the serial port. run without argument for help

Instructions to flash:
--------------------

1) Edit the ProgramToFlash.bat file
    Replace all variable with your own specification
    Edit the command to use the programmer you own(i.e. ICE-1000, ICE-2000, ...)
2) Save
3) edit the xml configuration script for 50MHz clock --- to be validated...
4) In a command shell execute the batch file.
    You should see in the screen some dot(.......) while it program
	
Instructions to configure:
--------------------

1) Make sure that the Pyhton environment is installed (requires 3.7 or above) (https://www.python.org/)
2) Have the serial PySerial package installed in the python environment (https://pypi.org/project/pyserial/)
3) power down the device
4) run the script with correct options (python configdev.py -f {database_filename} [-p {portname}][-n {desired_serial_number}]
5) power up the device when prompted
