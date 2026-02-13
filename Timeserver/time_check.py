#!/usr/bin/env python3
import socket, subprocess, sys, os, time, struct, datetime, threading

class Color:
    Blue = '\033[94m'
    Green = '\033[92m'
    Red = '\033[91m'
    Bold = '\033[1m'
    End = '\033[0m'

def check(expr, message):
	s = Color.Green + 'OK' if expr else Color.Red + 'FAIL'
	message += ':'
	print(f'  {message:<40} {Color.Bold}{s}{Color.End}')
	return expr

def test_server(command):
	print(Color.Bold+"Testing server"+Color.End)
	proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	time.sleep(0.1)
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	sock.sendto(bytes(), ("127.0.0.1", 3737))
	b = sock.recvfrom(1024)
	try:
		values = struct.unpack('>I', b[0])
	except:
		values = None
	
	if not check(b is not None, 'Decode data'): 
		return False

	now = datetime.datetime.now()
	rawdate = datetime.datetime.fromtimestamp(values[0])
	date = datetime.datetime.fromtimestamp(values[0]-2208988800)
	print("  Received date is:"+ Color.Bold, date, Color.End)

	if not check(rawdate.year != now.year, 'Start year is 1900'):
		return False

	if not check(date.year == now.year, 'Correct year'):
		return False

	if not check(date.month == now.month, 'Correct month'):
		return False

	if not check(date.day == now.day, 'Correct day'):
		return False

	if not check(date.hour == now.hour, 'Correct hour'):
		return False

	if not check(date.minute == now.minute, 'Correct minute'):
		return False

	proc.kill()

def udp_server():
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	server_address = ('127.0.0.1', 3737)
	sock.bind(server_address)
	print("  Server waiting for reply")
	b, addr = sock.recvfrom(1024)

	epoch = datetime.datetime(1900, 1, 1, tzinfo=datetime.timezone.utc)
	now = datetime.datetime.now(datetime.timezone.utc)
	time = int((now - epoch).total_seconds())
	try:
		send = struct.pack('>I', time)
	except:
		send = None
	print("  Server sending reply")
	sock.sendto(send, addr)

def test_client(command):
	thread = threading.Thread(target=udp_server)

	thread.start()
	time.sleep(0.5)
	output = subprocess.check_output(command).decode('utf8')
	print("  Reply:"+ Color.Bold, output.strip(), Color.End, "[please verify]")

if len(sys.argv) < 3:
	print('''
./time_check.py ./[server] ./[client] [client_args]

Note that the time server and client must use port 3737
to work with the test script. ''')
	exit(1)

test_server(sys.argv[1])
test_client(sys.argv[2:])

