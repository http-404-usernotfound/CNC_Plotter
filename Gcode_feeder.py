import serial

ser = serial.Serial(port="/dev/ttyACM0", baudrate=115200, timeout=0)

def Automatic_execution():
	gcode = []
	i = 1
	while True:
		try:
			path = input("Enter the path of Gcode file: ")
			with open(path, 'r') as gfile:
				for line in gfile:
					if line[0] != 'G' and line[0] != 'M':
						continue
					gcode.append(line.encode())

		except FileNotFoundError:
			print("File Not Found!\n")
			continue
		break


	ser.write(('?').encode())
	for line in gcode:
		ser.write(('?').encode())
		while True:
			try:
				while ser.read().decode() != 'R':
					continue

			except UnicodeDecodeError:
				continue
			break

		ser.write(line)
		print(i, line.decode())
		i += 1


def Manual_execution():
	rSent = False
	print("Enter Command:\n")
	command = ""
	while command != "EOF":
		command = input(">> ")
		ser.write(('?').encode())
		while True:
			ser.write(('?').encode())
			try:
				while ser.read().decode() != "R":
					continue
			except UnicodeDecodeError:
				continue
			break

		ser.write((command + '\n').encode())


mode = ''
while mode!='E':
	mode = str(input(
		 '''Select Mode:

	(F) - Execute Gcode from file
	(M) - Manual Execution
	(E) - Exit

	>> ''')).upper()

	if mode == 'F':
		Automatic_execution()

	elif mode == 'M':
		Manual_execution()

	elif mode == 'E':
		print("Exiting...\n")
		continue

	else:
		print("Enter a valid input")
		continue

	while True:
		ser.write(('?').encode())
		try:
			while ser.readline().decode() != "R":
				continue
		except UnicodeDecodeError:
			continue
		break
	ser.write(("exit\n").encode())
	print("done")
