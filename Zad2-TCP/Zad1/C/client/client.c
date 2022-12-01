#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA "Test Message no:  "

int main(int argc, char *argv[])
{
    int sock, client;
    struct sockaddr_in name;
    struct hostent *hp;
    
    /* Create socket on which to send. */ 
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("opening datagram socket");
        exit(1);
    }
    hp = gethostbyname(argv[1]);
    if (hp == (struct hostent *) 0) {
        fprintf(stderr, "%s: unknown host\n", argv[1]);
        exit(2);
    }
    memcpy((char *) &name.sin_addr, (char *) hp->h_addr,
    hp->h_length);
    name.sin_family = AF_INET;
    name.sin_port = htons( atoi( argv[2] ));

    /* Send message. */
    int i = 1;
    while(i <= 5){
        char msg[] = DATA;
        msg[strlen(msg) - 1] = '0' + i;
        //client = connect(sock, (struct sockaddr *) &name, sizeof(name));
        if(client < 0){
            perror("connecting with server");
        }
        //send(sock, msg, strlen(msg), 0);
        i++;
        //close(client);
    }
    exit(0);
}
