import socket
import sys
import threading


def receive_client_data(host: socket, thread_number: int) -> None:
    with host:
        while(True):
            msg = host.recv(bufferSize)
            if not msg:
                print(f'Thread no. {thread_number} exiting...')
                sys.exit()
            msg = msg.decode()
            print(f'Thread no. {thread_number}: {msg}')


if len(sys.argv) < 3:
    print("No server adrress or port was given")
    print("Setting default values: a: 0.0.0.0, p: 8000")
    localIP = '0.0.0.0'
    localPort = 8000
else:
    localIP = sys.argv[1]
    localPort = int(sys.argv[2])
bufferSize = 18

TCPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM)
TCPServerSocket.bind((localIP, localPort))
print("TCP server up and listening")

TCPServerSocket.listen(16)
threads_created = 0
while(True):
    # receiving name from client
    host, addr = TCPServerSocket.accept()
    server_thread = threading.Thread(target=receive_client_data, args=(host, threads_created), daemon=True)
    server_thread.start()
    threads_created += 1

# nigdy nie osiągnięte
TCPServerSocket.close()
