#include "common.h"

using namespace std;

int main()
{
    socklen_t length;
    struct sockaddr_in name;
    char buf[4096];
    int download = socket(AF_INET, SOCK_DGRAM, 0);
    int upload = socket(AF_INET, SOCK_DGRAM, 0);
    if (download == -1 || upload == -1) {
        perror("opening datagram socket");
        exit(1);
    }
    
    /* Create name with wildcards. */
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(8000);
    if (bind(download,(struct sockaddr *)&name, sizeof name) == -1) {
        perror("binding datagram socket");
        exit(1);
    }
    
    /* Wydrukuj na konsoli numer portu */
    length = sizeof(name);
    if (getsockname(download,(struct sockaddr *) &name, &length) == -1) {
        perror("getting socket name");
        exit(1);
    }
    printf("Socket port #%d\n", ntohs(name.sin_port));
    struct sockaddr_in source_address;
    socklen_t source_len = sizeof(source_address);
    int port = 8001;

    while(true){
        /* Read from the socket. */
        if ( recvfrom(download, buf, 4096, MSG_WAITALL, (sockaddr *) &source_address, &source_len) == -1 ) {
            perror("receiving start packet");
            exit(2); 
        }
        int test_type;
        int packet_size;
        time_t start_timestamp;
        source_address.sin_family = AF_INET;
        source_address.sin_port = htons(port);
        memcpy(&test_type, buf, sizeof(int));
        memcpy(&packet_size, buf + sizeof(int), sizeof(int));
        memcpy(&start_timestamp, buf + 2 * sizeof(int), sizeof(time_t));

        auto timestamp = chrono::time_point_cast<std::chrono::microseconds>(chrono::system_clock::now());
        time_t first_packet_timestamp = timestamp.time_since_epoch().count();
        packet_start response_first;
        response_first.packet_size = packet_size;
        response_first.timestamp_ =  first_packet_timestamp;
        if(test_type == 1)
            response_first.type = DOWNLOAD;
        else
            response_first.type = UPLOAD;
        if (sendto(upload, (const void *) &response_first, sizeof response_first, 0, (struct sockaddr *) &source_address, source_len) == -1)
            perror("sending datagram message");
        
        int packet_count = 0;
        char *data;
        int id = 0;
        bool terminated = false;
        time_t start = clock();
        packet_response_start server_response;
        while(true){
            int bytes_received = read(download, buf, 4096);
            ++packet_count;
            if (bytes_received == -1)
            {
                perror("receiving stream packet");
                exit(2);
            }
            if (bytes_received == 0){
                break;
            }
            if (bytes_received == 8)
                break;
            memcpy(&id, buf, sizeof(int));
            if (!terminated && (clock() - start) / (double)CLOCKS_PER_SEC > 0.2 )
            {
                auto timestamp = chrono::time_point_cast<std::chrono::microseconds>(chrono::system_clock::now());
                time_t last_packet_timestamp = timestamp.time_since_epoch().count();
                server_response = {
                    .number_of_packets=packet_count,
                    .first_packet=first_packet_timestamp,
                    .last_packet=last_packet_timestamp
                };
                terminated = true;
            }
            if (id == -1){
                if(!terminated)
                {
                    auto timestamp = chrono::time_point_cast<std::chrono::microseconds>(chrono::system_clock::now());
                    time_t last_packet_timestamp = timestamp.time_since_epoch().count();
                    server_response = {
                        .number_of_packets=packet_count,
                        .first_packet=first_packet_timestamp,
                        .last_packet=last_packet_timestamp
                };
                }
                if (sendto(upload, (const void *) &server_response, sizeof server_response, 0, (struct sockaddr *) &source_address, source_len) == -1)
                    perror("sending datagram message");
                break;
            }
        }
    }
    // close(download);
    // close(upload);
    exit(0);
}