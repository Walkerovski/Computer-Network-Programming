import socket
import sys
import random
from time import sleep


if len(sys.argv) < 3:
    print("No server adrress or port was given")
    print("Setting default values: a: 172.18.0.2, p: 8000")
    localIP = '172.18.0.2'
    localPort = 8000
else:
    localIP = sys.argv[1]
    localPort = int(sys.argv[2])

serverAddrPort = (localIP, localPort)

# define message
message = 'Test Message no: '

# connecting to hosts
TCPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM)

TCPClientSocket.connect(serverAddrPort)
i = 1
while i <= 5:
    # sending user message by encoding it
    bytesToSend1 = str.encode(f'{message}{i}')
    print(TCPClientSocket.send(bytesToSend1))
    # ensuring that the messages reach the server in a random order
    sleep(random.uniform(0.01, 0.1))
    i += 1
TCPClientSocket.close()
