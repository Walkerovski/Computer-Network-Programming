#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>



void main(void)
{
	int sock, length, msg, valread, socket_fd;
	struct sockaddr_in name;
	char buf[9];
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		perror("opening datagram socket");
		exit(1);
	}

	/* Create name with wildcards. */
	name.sin_family = AF_INET;
	name.sin_addr.s_addr = INADDR_ANY;
	name.sin_port = htons(8000);
	if (bind(sock, (struct sockaddr*) & name, sizeof name) == -1)
	{
		perror("binding stream socket");
		exit(1);
	}

	/* Wydrukuj na konsoli numer portu */
	length = sizeof(name);
	if (getsockname(sock, (struct sockaddr*) & name, &length) == -1)
	{
		perror("getting socket name");
		exit(1);
	}
	printf("Socket port #%d\n", ntohs(name.sin_port));

	if (listen(sock, 16) != 0)
	{
		perror("Listen failed.");
		exit(1);
	}

	const int MAX_SERVICED_CLIENTS = 10; /* We quit after accepting requests from 10 clients, to verify the wait() for children works. */
	int totalServicedClients = 0;

	const int STARTING_BUFFER_SIZE = 0xFF;

	while (true) {
		socket_fd = accept(sock, (struct sockaddr*) & name, (socklen_t*)& length);
		if (socket_fd < 0)
		{
			perror("accept");
			exit(1);
		}
		else
		{
			pid_t pid = fork();
			if (pid == 0)
			{
				close(sock);

				int bytes_recieved;
				int bufSize = STARTING_BUFFER_SIZE;
				char* buffer = (char*)malloc(bufSize * sizeof(char));
				char* bufferHead = buffer; 
				char* bufferTail = buffer + bufSize; 
				
				while (true)
				{
					bytes_recieved = read(socket_fd, bufferHead, bufferTail - bufferHead);
					if (bytes_recieved > 0)
					{
						bufferHead += bytes_recieved; 
						if (bufferHead - bufferTail < 0) {
							continue; 
						}
						else {
							bufSize *= 2; 
							char* newBuffer = (char*)malloc(bufSize * sizeof(char)); 
							size_t headOffset = bufferHead - buffer; 
							memcpy(newBuffer, buffer, headOffset); 
							free(buffer); 
							buffer = newBuffer; 
							bufferHead = buffer + headOffset; 
							bufferTail = buffer + bufSize; 
						}
					}
					else if (bytes_recieved == 0) {
						*bufferHead = '\0';
						break;
					}
					else /* bytes_recieved < 0 */
					{
						perror("receiving stream packet");
						exit(2);
					}
				}
				printf("-->%s\n", buffer);
				fflush(stdout);
				free(buffer); 
				close(socket_fd);
				exit(0); 
			}
			else
			{
				++totalServicedClients;
				if (totalServicedClients >= MAX_SERVICED_CLIENTS)
					break; // test the wait() functionality 
			}
		}
	}

	while (wait() != -1) {};
	if (errno != ECHILD)
	{
		perror("Unsuccessful waiting for children");
		exit(1);
	}
	shutdown(sock, SHUT_RDWR);
	exit(0);
}
