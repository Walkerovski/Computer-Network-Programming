import socket
import time
import sys


if len(sys.argv) < 3:
   print("No server adrress or port was given")
   print("Setting default values: a: 172.18.0.2, p: 8000")
   localIP = '172.18.0.2'
   localPort = 8000
else:
   localIP = sys.argv[1]
   localPort = int(sys.argv[2])

serverAddrPort = (localIP, localPort)
bufferSize = 1024


# define message
message = 'Test Message no:  '

# connecting to hosts
TCPClientSocket = socket.socket(family = socket.AF_INET, type = socket.SOCK_STREAM)

TCPClientSocket.connect(serverAddrPort)
i = 1
while i <= 5:
   # sending user message by encoding it
   bytesToSend1 = str.encode(f'{message}{i}\0')
   TCPClientSocket.sendall(bytesToSend1)
   ret_msg = TCPClientSocket.recv(bufferSize)
   time.sleep(2)
   print(f'"<--{message}" {i} | -->"{ret_msg.decode()}"')
   i += 1
TCPClientSocket.close()


