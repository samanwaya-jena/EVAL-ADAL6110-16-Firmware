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
import re
import time


filename = ""
serialnumber = 51
devicetype = 0x0A03
month = date.today().month
year = date.today().year -2000
portname = 'COM4'

'''
Validate the command line arguments and give usage if incorrect
'''
def check_arguments():
	global filename, serialnumber, devicetype, portname
	try:
		if (2 > len(sys.argv) or (len(sys.argv)%2==0)):
			print("missing arguments")
			print("usage:")
			print("\tconfigdev.py -f filemane [-n|t|p param]")
			print("\n\rwhere:")
			print("\t -f filename   \t\t\t-> indicate the database file")
			print("\t -t devicetype \t\t\t-> indicate what device serie (default = 0x0A03) \n\r\t\t\t(hex format... 0xccss where cc is client code and ss is configuration")
			print("\t -n current_serial_number \t-> indicate the current serial number to use. \n\r\t\t\tMust be higher than the last serial number in the database file")
			print("\t -p portname   \t\t\t -> indicate the serial port name (default = 'COM4')")
		else: 
			for i in range(len(sys.argv)):
				if sys.argv[i] == '-f':
					filename = str(sys.argv[i+1])
				elif sys.argv[i] == "-n":
					serialnumber = int(sys.argv[i+1])
				elif sys.argv[i] == "-t":
					devicetype = int(sys.argv[i+1],16)
				elif sys.argv[i] == "-p":
					portname = str(sys.argv[i+1])
	except:
		print("Error in check_arguments()")
		raise

'''
Validate the presence of database file
if file not present, create it
otherwise validate that the desired serial number is highest in file
'''
def check_file():
	global filename,serialnumber
	if filename == "":
		print("I pity the fool with no files!!!")
		exit()
	elif os.path.isfile(filename):
		print("Opening file: {:s} continuing with serial number ".format(filename),end='')
		with open(filename,'r') as f:
			datainfile = f.readlines()
			if len(datainfile) > 2:
				for i in range(len(datainfile)-1,1,-1) :
					val = re.search(r'\t(\w+)\t(\w+)',datainfile[i]) 
					s= int(val.group(1))
					#t = int(val.group(2),16)
					if serialnumber <= s: serialnumber = s+1
			else:
				val = re.search(r'\t(\w+)\t(\w+)',datainfile[1])
				s= int(val.group(1))
				if serialnumber <= s: serialnumber = s+1
			print(serialnumber)
	else:
		print("Creating file: {:s} starting with serial number {:04d}".format(filename,serialnumber))
		with open(filename,'w') as f:
			f.write("date and time\t\tSerial\tConfig\n")

'''
read the device info on the serial port
'''
def read_info(sp):
	infocmd = b'i\r'
	datain = b'read'
	global serialnumber,month,year,devicetype
	ser = 0
	try:
		sp.timeout = 0.5
		sp.write(infocmd)
		while b'' != datain: 
			datain = sp.readline()
			data = re.search(r'(\w+): ',datain.decode('ascii'))
			if data:
				if data.group(1) == 'date':
					data = re.search(r': (\w+)/(\w+)',datain.decode('ascii'))
					yy = int(data.group(1)) - 2000
					mm = int(data.group(2))
				elif data.group(1) == 'ID':
					data = re.search(r': (\w+)',datain.decode('ascii'))
					ID = int(data.group(1),16)
				elif data.group(1) == 'Number':
					data = re.search(r': (\w+)',datain.decode('ascii'))
					ser = int(data.group(1))
				elif data.group(1) == 'version':
					data = re.search(r': (\w+).(\w+)',datain.decode('ascii'))
					majv = data.group(1)
					minv = data.group(2)
			elif re.search(r'Lidar',datain.decode('ascii')):
				datain = b''
				
	except:
		print("Exception occured in read_info()")
		raise
	else:
		print("{} {} {} {} {} {}".format(ID,ser,yy,mm,majv,minv))
		# quelque chose chie dans ces lignes...
		print("Device found! Type 0x{0:04X} with firmware version {1:02d}.{2:03d}".format(ID, majv, minv),end="")
		if ser == 0:
			print(" without serial information")
		else:
			print(" that has serial number {0:04d} programmed on 20{1:02d}/{2:02d}".format(ser, yy, mm))
	finally:
		if ser: return False
		else: return True
'''
send one command to the device
wait for a correct answer. will retry if necessary
return if command sent with success
'''
def send_cmd(sp,cmd,val,OK_val):
	wait_for_answer = 1
	progcmd = "{:s} {:d}\r\n".format(cmd,val)
	sp.write(progcmd.encode('ascii'))
	while wait_for_answer:
		datain = sp.readline()
		print(datain.decode('ascii'))	
		if re.search(OK_val,datain.decode('ascii')):
			print("message answered")
			return True
		elif re.search(r'Say again',datain.decode('ascii')):
			print("whooops!")
			return False
	return

'''
send the info to the device
'''
def program_device(sp):
	global serialnumber, year, month
	try:
		while not send_cmd(sp,"N",serialnumber,r'Serial'): 
			time.sleep(1)
		while not send_cmd(sp,"D",256*year+month,r'Date'):
			time.sleep(1)	
	except:
		print("exception occured in program_device()")
		raise


'''
Programing the flash information
'''
def configure_device():
	global portname
	try:
		sp = serial.Serial(portname, 1000000,xonxoff=True,timeout=1)
		datain = b''
		while b'' == datain:
			datain = sp.readline() #wait for the boot message
		while b'' != datain:
			datain = sp.readline() #skip the boot message
			if re.search(r'Enumerated',datain.decode('ascii')):
				datain = b''
		if True: #read_info(sp):
			program_device(sp)
			read_info(sp)
			if ser != serialnumber or mm != month:
				raise
		else: 
			print("Previous configuration detected")
			raise
				
	except OSError:
		print("Failed to open serial port {}".format(portname))
		raise
	except:
		print('Error in configuration')
		raise
	else:
		print("config OK, writing to flash")
		sp.write(b'S\r\n')
		sp.close()
	finally:
		pass
'''
main()
'''
def main():
	try:
		check_arguments()
		check_file()
		configure_device()
	except:
		print("An error occured!")
	else:
		with open(filename,'a') as f:
			f.write(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S") + "\t{0:04d}\t0x{1:04X}\n".format(serialnumber,devicetype))
		print("configuration of unit #{:04d} of type 0x{:04X} done at ".format(serialnumber,devicetype) + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
	finally: 
		print("Exiting configuration software")
	
'''
Command line script
'''
if __name__== "__main__":
	main()