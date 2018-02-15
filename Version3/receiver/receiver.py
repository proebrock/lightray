import serial
import RS_FEC
import binascii
import sys

START  = 0xfc
ESCAPE = 0xfd



def check_test_message(message):
	if len(message) != 16:
		print('Message length error')
		return
	if 'cnt' in check_test_message.__dict__:
		if ((check_test_message.cnt + 16) & 0xff) != message[0]:
			print('Outer sequence error')
			check_test_message.cnt = message[0]
			return
	check_test_message.cnt = message[0]
	for i in range(16):
		if message[i] != (message[0] + i):
			print('Inner sequence error')
			return



def handle_buffer(receive_buffer):
	# De-escaping_and removing header
	decoded_buffer = bytearray()
	receive_buffer_it = iter(receive_buffer)
	_ = next(receive_buffer_it) # skip START
	if len(receive_buffer) != next(receive_buffer_it):
		print('Message length error')
		return
	for value in receive_buffer_it:
		if value == ESCAPE:
			decoded_buffer.append(next(receive_buffer_it) ^ 0x20)
		else:
			decoded_buffer.append(value)
	assert(len(decoded_buffer) == 32)
	# Decode forward error correction (FEC)
	try:
		message = RS_FEC.decode(decoded_buffer)
	except RS_FEC.DecodeError:
		print('FEC decoding failed')
		return
	# Decode byte array to unicode string
	try:
		message = message.decode('utf-8').strip('\0')
	except UnicodeDecodeError:
		print('Unicode decoding failed')
		return
	# Parse message
	splitmsg = message.split(';')
	if len(splitmsg) != 3:
		print('Parsing message failed '+ message)
		return
	try:
		tempCelsis = float(splitmsg[0]) / 10.0
		pressureMilliBar = float(splitmsg[1]) / 10.0
		humidityPercent = 1.0 * float(splitmsg[2])
	except ValueError:
		print('Converting to number failed ' + message)
		return
	print((tempCelsis, pressureMilliBar, humidityPercent))


	#check_test_message(message)



# Receiving
def receiver_main():
	while True:
		data = port.read()
		try:
			message = data.decode('utf-8')
		except UnicodeDecodeError:
			message = binascii.hexlify(data)
		print(message, end='')


# Receiving
def receiver_main(mode=None):
	# Set up serial port
	port = serial.Serial()
	port.port = '/dev/ttyAMA0'
	port.baudrate = 4800
	port.bytesize = 8
	port.parity = 'N'
	port.stopbits = 1
	port.timeout = 3.0
	port.xonxoff = 0
	port.rtscts = 0
	port.open()
	# Receiver main loop
	if mode == 'raw':
		while True:
			data = port.read()
			try:
				message = data.decode('utf-8')
			except UnicodeDecodeError:
				message = binascii.hexlify(data)
			print(message, end='')
	else:
		receive_buffer = bytearray()
		bytes_expected = 0
		while True:
			data = port.read()
				
			for value in data:
				if value == START:
					receive_buffer = bytearray()
					receive_buffer.append(START)
				elif len(receive_buffer) == 1:
					receive_buffer.append(value)
					bytes_expected = value
				elif len(receive_buffer) > 1:
					receive_buffer.append(value)
					if len(receive_buffer) == bytes_expected:
						handle_buffer(receive_buffer)
						receive_buffer = []
					elif len(receive_buffer) > bytes_expected:
						receive_buffer = []
	#port.close()




if __name__ == "__main__":
	if len(sys.argv) == 2:
		receiver_main(sys.argv[1])
	else:
		receiver_main()

