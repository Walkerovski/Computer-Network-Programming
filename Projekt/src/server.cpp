#include "server.h"

void setup_sockets(){
    upload = socket(AF_INET, SOCK_DGRAM, 0);
    Plik << "---Lin4: Otworzono gniazdo do wysyłania\n";
    download = socket(AF_INET, SOCK_DGRAM, 0);
    Plik << "---Lin6: Otworzono gniazdo do pobierania\n";
    if (download == -1 || upload == -1) {
        perror("opening datagram socket");
        exit(1);
    }
    
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(8002);
    Plik << "---Lin15: Ustawiono port na 8002\n";
    if (bind(download,(struct sockaddr *)&name, sizeof name) == -1) {
        perror("binding datagram socket");
        exit(1);
    }
    
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
    Plik << "---Lin31: Odebrano pakiet startowy\n";
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
    if(test_type == 1){
        response_first.type = DOWNLOAD;
        Plik << "---Lin41: Wybrano test pobierania\n";
    }
    else{
        response_first.type = UPLOAD;
        Plik << "---Lin41: Wybrano test wysyłania\n";
    }
    if (sendto(upload, (const void *) &response_first, sizeof response_first, 0, (struct sockaddr *) &source_address, source_len) == -1)
        perror("sending datagram message");
    Plik << "---Lin58: Odesłano odpowiedź na pakiet startowy\n";
    
    return {response_first, test_type};
}

void receive_packet_upload(time_t first_packet_timestamp){
    Plik << "---Lin65: Rozpoczęto test wysyłania danych\n";
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
            Plik << "---Lin87: Przekroczono limit czasu odczytawania, liczba odczytanych pakietów: *** "<<packet_count<<" ***\n";
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
            Plik << "---Lin99: Zakończono odbieranie danych\n";
            if(!terminated)
            {
                Plik << "---Lin101: Udało się odebrać wszystkie pakiety! Etap zakończony\n";
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
            Plik << "---Lin112: Odesłano informację do klienta o ilości odebranych pakietów\n";
            break;
        }
    }
}

void send_packet_download(int packet_size_, int how_many_bytes){
    Plik << "---Lin120: Rozpoczęto test pobierania danych\n";
    int bytes_send = 0;
    int loop = 1;
    char data_[packet_size_];
    for (int i = 0; i < packet_size_; ++i)
        data_[i] = 'x';
    data_[packet_size_] = '\0';
    Plik << "---Lin125: Utworzono pakiet o podanym przez kliencia rozmiarze\n";
    auto timestamp = chrono::time_point_cast<std::chrono::microseconds>(chrono::system_clock::now());
    time_t first_packet_time = timestamp.time_since_epoch().count();
    Plik << "---Lin132: Wysyłanie danych\n";
    while(bytes_send < how_many_bytes){
        packet_test test = {.id = loop, .data = data_};
        if (sendto(upload, (const void *) &test, strlen(data_) + sizeof(int), 0, (struct sockaddr *) &source_address, source_len) == -1)
            perror("sending datagram message");
        usleep(1);
        bytes_send += packet_size_;
        ++loop;
    }

    Plik << "---Lin139: Wszystkie pakiety testowe wysłane\n";
    char stop[] = "STOP";
    packet_test last_packet = {.id = -1, .data = stop};

    sleep(1);
    if (sendto(upload, (const void *) &last_packet, sizeof(last_packet), 0, (struct sockaddr *) &source_address, source_len) == -1)
            perror("sending last packet");
    Plik << "---Lin146: Wysłanie pakietu kończącego etap testu\n";

    timestamp = chrono::time_point_cast<std::chrono::microseconds>(chrono::system_clock::now());
    time_t last_packet_timestamp = timestamp.time_since_epoch().count();
    packet_response_start server_response = {
        .number_of_packets=loop,
        .first_packet=first_packet_time,
        .last_packet=last_packet_timestamp
    };
    if (sendto(upload, (const void *) &server_response, sizeof server_response, 0, (struct sockaddr *) &source_address, source_len) == -1)
        perror("sending network stats");
    Plik << "---Lin157: Odesłano informację o ilości wysłanych pakietów\n";
}

void start_test(time_t first_packet_timestamp, int type, int packet_size_, int how_many_bytes){
    if (type == 1){
        Plik << "---Lin163: Wybrano test pobierania\n";
        send_packet_download(packet_size_, how_many_bytes);
    }
    if (type == 2){
        receive_packet_upload(first_packet_timestamp);
        Plik << "---Lin167: Wybrano test wysyłania\n";
    }
}

int main()
{
    Plik << "***ROZPOCZĘCIE DZIAŁANIA SERWERA***\n\n\n\n";
    int summary_bytes_to_send = 2e5 / 8;
    setup_sockets();
    Plik << "***ZAKOŃCZONO USTAWIANIE GNIAZD***\n\n\n\n";
    int test_id = 1;
    while(true){
        pair<packet_start, int> test_data = receive_first();
        Plik << "***ODEBRANO INFORMACJE OD KLIENTA O TYPE TESTU***\n\n\n\n";
        start_test(test_data.first.timestamp_, test_data.second, test_data.first.packet_size, summary_bytes_to_send);
        Plik << "***ZAKOŃCZONO " << test_id << " ETAP TESTU***\n\n\n\n";
        summary_bytes_to_send += 2e6 / 8;
        ++test_id;
    }
    Plik << "***ZAKOŃCZENIE DZIAŁANIA SERWERA***\n\n\n\n";
    Plik.close();
    close(download);
    close(upload);
    exit(0);
}
