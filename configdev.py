# configuration script for EVAL-ADAL6110-16

"""
  This module is used to program the flash information of the EVAL-ADAL6110-16
  requires pyserial add-on.
"""
import os
import sys
import datetime
from datetime import date
import serial

filename = ""
serialnumber = 0
devicetype = 0x0A03
month = date.today().month
year = date.today().year -2000


#check arguments
if (2 > len(sys.argv) or (len(sys.argv)%2==0)):
	print("missing arguments")
	print("usage:")
	print("configdev.py -f filemane [-n current_serial_number]")
	print("\n\rwhere:\r\n\t-f filename \t\t\t-> indicate the database file")
	print("\n\rwhere:\r\n\t-t devicetype \t\t\t-> indicate what device serie \n\r\t\t\t(hex format... 0xccss where cc is client code and ss is configuration")
	print("\t-n current_serial_number \t-> indicate the current serial number to use. \n\r\t\t\tMust be higher than the last serial number in the database file")
else: 
	for i in range(len(sys.argv)):
		if sys.argv[i] == '-f':
			filename = str(sys.argv[i+1])
		elif sys.argv[i] == "-n":
			serialnumber = int(sys.argv[i+1])
		elif sys.argv[i] == "-t":
			devicetype = int(sys.argv[i+1],16)


#open file and validate content
if filename == "":
	print("I pity the fool with no files!!!")
	exit()
elif os.path.isfile(filename):
	print("Opening file: {:s}".format(filename))
	with open(filename,'r') as f:
		#todo: get last serial number and update the current serial number.
		datainfile = f.readlines()
else:
	print("Creating file: {:s}".format(filename))
	with open(filename,'w') as f:
		f.write("date and time\t\tSerial\tConfig\n")


#configuration validation
for i in range(len(datainfile)-1,1,-1) :
	print('.',end='')
	s = int(datainfile[i][20:24]) 
	#t = int(datainfile[i][35:31],16)
	if serialnumber <= s: serialnumber = s+1
print()

#Programing the flash information



#save record in file, and we're done
with open(filename,'a') as f:
	f.write(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S") + "\t{0:04d}\t0x{1:04X}\n".format(serialnumber,devicetype))

print("configuration of unit #{:04d} of type 0x{:04X} done at ".format(serialnumber,devicetype) + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
