#include "server.h"

void setup_sockets(){
    upload = socket(AF_INET, SOCK_DGRAM, 0);
    download = socket(AF_INET, SOCK_DGRAM, 0);
    if (download == -1 || upload == -1) {
        perror("opening datagram socket");
        exit(1);
    }
    
    /* Create name with wildcards. */
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(8002);
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
}

pair<packet_start, int> receive_first(){
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
    return {response_first, test_type};
}

void receive_packet_upload(time_t first_packet_timestamp){
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
        if (bytes_received == 0)
            break;
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

void send_packet_download(int packet_size_, int how_many_bytes){
    int bytes_send = 0;
    int loop = 1;
    char data_[packet_size_];
    for (int i = 0; i < packet_size_; ++i)
        data_[i] = 'x';
    data_[packet_size_] = '\0';
    auto timestamp = chrono::time_point_cast<std::chrono::microseconds>(chrono::system_clock::now());
    time_t first_packet_time = timestamp.time_since_epoch().count();
    while(bytes_send < how_many_bytes){
        packet_test test = {.id = loop, .data = data_};
        if (sendto(upload, (const void *) &test, strlen(data_) + sizeof(int), 0, (struct sockaddr *) &source_address, source_len) == -1)
            perror("sending datagram message");
        usleep(1);
        bytes_send += packet_size_;
        ++loop;
    }

    char stop[] = "STOP";
    packet_test last_packet = {.id = -1, .data = stop};

    sleep(1);
    if (sendto(upload, (const void *) &last_packet, sizeof(last_packet), 0, (struct sockaddr *) &source_address, source_len) == -1)
            perror("sending last packet");

    timestamp = chrono::time_point_cast<std::chrono::microseconds>(chrono::system_clock::now());
    time_t last_packet_timestamp = timestamp.time_since_epoch().count();
    packet_response_start server_response = {
        .number_of_packets=loop,
        .first_packet=first_packet_time,
        .last_packet=last_packet_timestamp
    };
    if (sendto(upload, (const void *) &server_response, sizeof server_response, 0, (struct sockaddr *) &source_address, source_len) == -1)
        perror("sending network stats");
    
    
}

void start_test(time_t first_packet_timestamp, int type, int packet_size_, int how_many_bytes){
    if (type == 1){
        send_packet_download(packet_size_, how_many_bytes);
    }
    if (type == 2)
        receive_packet_upload(first_packet_timestamp);
}

int main()
{
    int summary_bytes_to_send = 2e5 / 8; // 0.2Mb / 0.2s = 1Mb/s
    setup_sockets();
    while(true){
        /* Read from the socket. */
        pair<packet_start, int> test_data = receive_first();
        start_test(test_data.first.timestamp_, test_data.second, test_data.first.packet_size, summary_bytes_to_send);
        summary_bytes_to_send += 2e6 / 8; // 2Mb / 0.2s = 10Mb/s
    }
    close(download);
    close(upload);
    exit(0);
}
