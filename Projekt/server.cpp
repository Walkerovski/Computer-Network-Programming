#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <bits/stdc++.h>
#include <unistd.h>

using namespace std;

struct packet_start{ 
    int32_t number_of_packets;
    time_t first_packet;
    time_t last_packet;
    int32_t packet_size;
};

int main()
{
    int sock;
    socklen_t length;
    struct sockaddr_in name;
    char buf[1024];
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("opening datagram socket");
        exit(1);
    }
    
    /* Create name with wildcards. */
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(8000);
    if (bind(sock,(struct sockaddr *)&name, sizeof name) == -1) {
        perror("binding datagram socket");
        exit(1);
    }
    
    /* Wydrukuj na konsoli numer portu */
    length = sizeof(name);
    if (getsockname(sock,(struct sockaddr *) &name, &length) == -1) {
        perror("getting socket name");
        exit(1);
    }
    printf("Socket port #%d\n", ntohs(name.sin_port));

    while(true){
        /* Read from the socket. */
         if ( read(sock, buf, 1024) == -1 ) {
            perror("receiving start packet");
            exit(2); 
        }
        while(true){

            int bytes_recieved = read(sock, buf, 1024);
            if (bytes_recieved == -1)
            {
                perror("receiving stream packet");
                exit(2);
            }
            if (bytes_recieved == 0){
                break;
            }
            buf[bytes_recieved] = '\0';
            printf("-->%s", buf);
            fflush(stdout);
                //send(socket_acc, buf, strlen(buf), 0);
        }
        printf("-->%s\n", buf);
    }
    close(sock);
    exit(0);
} 