#include "client.h"
#include "interface.h"
#include "statistics.h"

void setup_sockets(const char * address, const char * port){
    read_name.sin_family = AF_INET;
    read_name.sin_addr.s_addr = INADDR_ANY;
    read_name.sin_port = htons(8001);
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    Plik << "---Lin9: Otworzono gniazdo do wysyłania\n";
    if (sock == -1) {
        perror("opening datagram socket");
        exit(1);
    }
    Plik << "---Lin15: Otworzono gniazdo do odbierania\n";
    hp = gethostbyname(address);
    if (hp == (struct hostent *) 0) {
        fprintf(stderr, "%s: unknown host\n", address);
        exit(2);
    }

    memcpy((char *) &name.sin_addr, (char *) hp->h_addr, hp->h_length);
    name.sin_family = AF_INET;
    name.sin_port = htons(atoi(port));
}

void send_first(int sock, int type, int packet_size_){
    packet_start first;
    auto timestamp = chrono::time_point_cast<std::chrono::microseconds>(chrono::system_clock::now());
    time_t first_packet_time = timestamp.time_since_epoch().count();
    first.timestamp_ = first_packet_time;
    first.packet_size = packet_size_;
    if(type == 1){
        first.type = DOWNLOAD;
    }
    else if(type == 2){
        first.type = UPLOAD;
    }
    else if(type == 3){
        first.type = BREAK;
    }
    if (sendto(sock, (const void *) &first, sizeof first, 0, (struct sockaddr *) &name, sizeof name) == -1)
        perror("sending datagram message");
    Plik << "---Lin39: Wysłano pakiet startowy\n";
    int type_of_test, packet_size;
    time_t server_time_stamp;
    if ( read(sock, buf, 4096) == -1 ) {
        perror("receiving start packet");
        exit(2); 
    }
    Plik << "---Lin44: Odebrano odpowiedź na pakiet startowy\n";
    timestamp = chrono::time_point_cast<std::chrono::microseconds>(chrono::system_clock::now());
    time_t last_packet_time = timestamp.time_since_epoch().count();
    memcpy(&type_of_test, buf, sizeof(int));
    memcpy(&packet_size, buf + sizeof(int), sizeof(int));
    memcpy(&server_time_stamp, buf + 2 * sizeof(int), sizeof(time_t));
    cout << "Ping: " << (last_packet_time - first_packet_time) / 1000.0 << "ms" << endl; 
    Plik << "---Lin54: Obliczono ping na podstawie komunikacji i wyświetlono użytkownikowi\n";
}

void send_packet_upload(int sock, int packet_size_, int how_many_bytes){
    Plik << "---Lin58: Rozpoczęto test wysyłania danych\n";
    int bytes_send = 0;
    int loop = 1;
    char data_[packet_size_];
    for (int i = 0; i < packet_size_; ++i)
        data_[i] = 'x';
    data_[packet_size_] = '\0';
    Plik << "---Lin67: Wysyłanie danych\n";
    while(bytes_send < how_many_bytes){
        packet_test test = {.id = loop, .data = data_};
        if (sendto(sock, (const void *) &test, strlen(data_) + sizeof(int), 0, (struct sockaddr *) &name, sizeof name) == -1)
            perror("sending datagram message");
        usleep(1);
        bytes_send += packet_size_;
        ++loop;
    }

    Plik << "---Lin74: Wszystkie pakiety testowe wysłane\n";

    char stop[] = "STOP";
    packet_test last_packet = {.id = -1, .data = stop};

    sleep(1);
    if (sendto(sock, (const void *) &last_packet, 
            sizeof(last_packet), 0, (struct sockaddr *) &name,
                sizeof name) == -1)
            perror("sending datagram message");

    Plik << "---Lin82: Wysłanie pakietu kończącego etap testu\n";
    int number_of_packets;
    time_t first_packet_time;
    time_t last_packet_time;
    if ( read(sock, buf, 4096) == -1 ) {
        perror("receiving start packet");
        exit(2); 
    }
    Plik << "---Lin91: Odebranie infomacji o ilości odebranych pakietów\n";
    memcpy(&number_of_packets, buf, sizeof(int));
    float packet_loss = calculate_packet_loss(number_of_packets, loop);
    Plik << "---Lin97: Obliczono ilość utraconych pakietów\n";
    if(packet_loss > 20)
    {
        PACKET_LOSS_ACHIEVED = 1;
    }
    print_internet_speed(packet_loss, how_many_bytes);
    Plik << "---Lin103: Wyświetlono użytkownikowi prędkość po aktualnym etapie testu\n";
}

void receive_packet_download(int sock, int packet_size_, int how_many_bytes){
    Plik << "---Lin107: Rozpoczęto test pobierania danych\n";
    int packet_count = 0;
    char *data;
    int id = 0;
    bool terminated = false;
    time_t start = clock();
    packet_response_start server_response;
    while(true){
        int number_of_packets;
        time_t first_packet_time;
        time_t last_packet_time;
        int bytes_received = read(sock, buf, 4096);
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
            Plik << "---Lin142: Przekroczono limit czasu odczytawania, liczba odczytanych pakietów: *** "<<packet_count<<" ***\n";                                                                                                                                                                                                                                                                                                                      
            float packet_loss = calculate_packet_loss(packet_count, how_many_bytes / (float)packet_size_);
            if(packet_loss > 20)
            {
                PACKET_LOSS_ACHIEVED = 1;
            }
            print_internet_speed(packet_loss, how_many_bytes);
            terminated = true;
        }
        if (id == -1){
            Plik << "---Lin153: Zakończono odbieranie danych\n";
            if(!terminated)
                {
                    Plik << "---Lin155: Udało się odebrać wszystkie pakiety! Etap zakończony\n";
                    if ( read(sock, buf, 4096) == -1 ) {
                        perror("receiving stats packet");
                        exit(2); 
                    }
                    memcpy(&number_of_packets, buf, sizeof(int));
                    double packet_loss = calculate_packet_loss(packet_count, how_many_bytes / (float)packet_size_);
                    if(packet_loss > 20)
                    {
                        PACKET_LOSS_ACHIEVED = 1;
                    }
                    print_internet_speed(packet_loss, how_many_bytes);
                    Plik << "---Lin168: Wypisano użytkownikowi aktualną prędkość\n";
                }
            break;
        }
        ++packet_count;
    }
}

void start_test(int type, int sock, int packet_size_, int how_many_bytes){
    if (type == 1){
        Plik << "---Lin177: Wybrano test pobierania\n";
        receive_packet_download(sock, packet_size_, how_many_bytes);
    }
    if (type == 2){
        Plik << "---Lin181: Wybrano test wysyłania\n";
        send_packet_upload(sock, packet_size_, how_many_bytes);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3){
        cout << "Prosimy o podanie adresu serwera i portu" << endl;
        exit(0);
    }
    Plik << "***ROZPOCZĘCIE DZIAŁANIA KLIENTA***\n\n\n\n";
    setup_sockets(argv[1], argv[2]);
    Plik << "***ZAKOŃCZONO USTAWIANIE GNIAZD***\n\n\n\n";
    pair <int, int> user_input = interface_read();
    Plik << "***ZAKOŃCZONO INTERAKCJĘ Z UŻYTKOWNIKIEM***\n\n\n\n";
    int test_id = 1;
    int summary_bytes_to_send = 2e5 / 8;
    while(true){
        send_first(sock, user_input.first, user_input.second);
        Plik << "***WYSŁANO INFORMACJĘ DO SERWERA O TYPIE TESTU***\n\n\n\n";
        start_test(user_input.first, sock, user_input.second, summary_bytes_to_send);
        Plik << "***ZAKOŃCZONO " << test_id << " ETAP TESTU***\n\n\n\n";
        if(PACKET_LOSS_ACHIEVED)
            break;
        summary_bytes_to_send += 2e6 / 8;
        ++test_id;
    }
    send_first(sock, 3, 0);
    Plik << "***ZAKOŃCZENIE DZIAŁANIA KLIENTA***\n\n\n\n";
    Plik.close();
    close(sock);
    exit(0);
}
