#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void main(void)
{
    int sock, length, msg, valread, socket_acc;
    struct sockaddr_in name;
    char buf[1024];
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("opening datagram socket");
        exit(1);
    }
    
    /* Create name with wildcards. */
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(8000);
    if (bind(sock,(struct sockaddr *)&name, sizeof name) == -1) {
        perror("binding stream socket");
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
        listen(sock, 5);
        socket_acc = accept(sock, (struct sockaddr *) &name, (socklen_t*)&length);
        if( socket_acc < 0){
            perror("accept");
            exit(1);
        }
        else{
            /* Read from the socket. */
            if ( read(sock, buf, 1024) == -1 ) {
                perror("receiving strea packet");
                exit(2); 
            }
        }
        printf("-->%s\n", buf);
        close(socket_acc);
    }
    shutdown(sock, SHUT_RDWR);
    exit(0);
} 
