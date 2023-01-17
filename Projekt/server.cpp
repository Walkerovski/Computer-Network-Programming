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

enum type{UPLOAD = 2, DOWNLOAD = 1};
struct packet_response_start{ 
    int32_t number_of_packets;
    time_t first_packet;
    time_t last_packet;
    int32_t packet_size;
};

struct packet_start{ 
    enum type type;
    int packet_size;
    time_t timestamp_;
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
    int sock2 = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr source_address;
    socklen_t source_len;
    while(true){
        /* Read from the socket. */
         if ( recvfrom(sock, buf, 1024, MSG_WAITALL, &source_address, &source_len) == -1 ) {
            perror("receiving start packet");
            exit(2); 
        }
        int cos;
        int cos2;
        time_t cos3;

        memcpy(&cos, buf, sizeof(int));
        memcpy(&cos2, buf + sizeof(int), sizeof(int));
        memcpy(&cos3, buf + 2 * sizeof(int), sizeof(time_t));
        packet_start response_first;
        response_first.packet_size = cos2;
        response_first.timestamp_ = clock();
        if(cos == 1)
            response_first.type = DOWNLOAD;
        else
            response_first.type = UPLOAD;
        if (sendto(sock2, (const void *) &response_first, sizeof response_first, 0, (struct sockaddr *) &source_address, source_len) == -1)
            perror("sending datagram message");
        cout << cos << " " << cos2 << " " << cos3 <<"\n";
        
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
            //printf("-->%s", buf);
            fflush(stdout);
                //send(socket_acc, buf, strlen(buf), 0);
        }
        //printf("-->%s\n", buf);
    }
    close(sock);
    exit(0);
} 