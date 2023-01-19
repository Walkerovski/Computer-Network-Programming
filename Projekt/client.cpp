#include "common.h"
using namespace std;

int sock, sock2;
struct sockaddr_in name;
struct hostent *hp;
char buf[4096];
bool PACKET_LOSS_ACHIEVED = false;

pair <int, int> interface(){
    string input;
    cout<<"Witamy w speedteście! Prosimy o podanie typu operacji: \n1 - DOWNLOAD \n2 - UPLOAD \n";
    cin>>input;
    int type_input = atoi(input.c_str());
    while(type_input > 2 || type_input <= 0){
        cout<<"Podane dane na wejściu są złe! Proszę wybrać jedną z poniższych operacji: \n1 - DOWNLOAD \n2 - UPLOAD \n";
        cin>>input;
        type_input = atoi(input.c_str());
    }
    cout<<"\n";
    cout<<"Prosimy o podanie wielkości pakietu. Wielkośc pakiety musi być z przedziału 512 - 4096 (wielkość podana w bajtach)\n";
    cin>>input;
    int size_input = atoi(input.c_str());
    while(size_input > 4096 || size_input < 512){
        cout<<"Podane dane na wejściu są złe! Proszę wpisać wartość z przedziału 512 - 4096 (wielkość podana w bajtach)\n";
        cin>>input;
        size_input = atoi(input.c_str());
    }
    return {type_input, size_input};
}

void setup_sockets(const char * address, const char * port){
    /* Create socket on which to send. */ 
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("opening datagram socket");
        exit(1);
    }
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
    struct sockaddr_in read_name;
    read_name.sin_family = AF_INET;
    read_name.sin_addr.s_addr = INADDR_ANY;
    read_name.sin_port = htons(8001);
    sock2 = socket(AF_INET, SOCK_DGRAM, 0);
    if (bind(sock2,(struct sockaddr *)&read_name, sizeof read_name) == -1) {
        perror("binding datagram socket");
        exit(1);
    }
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
    if (sendto(sock, (const void *) &first, sizeof first, 0, (struct sockaddr *) &name,sizeof name) == -1)
        perror("sending datagram message");
    int type_of_test, packet_size;
    time_t server_time_stamp;
    if ( read(sock2, buf, 4096) == -1 ) {
        perror("receiving start packet");
        exit(2); 
    }
    timestamp = chrono::time_point_cast<std::chrono::microseconds>(chrono::system_clock::now());
    time_t last_packet_time = timestamp.time_since_epoch().count();
    memcpy(&type_of_test, buf, sizeof(int));
    memcpy(&packet_size, buf + sizeof(int), sizeof(int));
    memcpy(&server_time_stamp, buf + 2 * sizeof(int), sizeof(time_t));
    cout << "Ping: " << (last_packet_time - first_packet_time) / 1000.0 << "ms" << endl; 
    close(sock2);
}

void send_packet_upload(int sock, int packet_size_, int how_many_bytes){
    int bytes_send = 0;
    int loop = 1;
    char data_[packet_size_];
    for (int i = 0; i < packet_size_; ++i)
        data_[i] = 'x';
    data_[packet_size_] = '\0';
    while(bytes_send < how_many_bytes){
        packet_test test = {.id = loop, .data = data_};
        if (sendto(sock, (const void *) &test, strlen(data_) + sizeof(int), 0, (struct sockaddr *) &name,sizeof name) == -1)
            perror("sending datagram message");
        usleep(1);
        bytes_send += packet_size_;
        ++loop;
    }

    struct sockaddr_in read_name;
    read_name.sin_family = AF_INET;
    read_name.sin_addr.s_addr = INADDR_ANY;
    read_name.sin_port = htons(8001);
    sock2 = socket(AF_INET, SOCK_DGRAM, 0);
    if (bind(sock2,(struct sockaddr *)&read_name, sizeof read_name) == -1) {
        perror("binding datagram socket");
        exit(1);
    }

    char stop[] = "STOP";
    packet_test last_packet = {.id = -1, .data = stop};

    sleep(1);
    if (sendto(sock, (const void *) &last_packet, 
            sizeof(last_packet), 0, (struct sockaddr *) &name,
                sizeof name) == -1)
            perror("sending datagram message");


    int number_of_packets;
    time_t first_packet_time;
    time_t last_packet_time;

    if ( read(sock2, buf, 4096) == -1 ) {
        perror("receiving start packet");
        exit(2); 
    }
    memcpy(&number_of_packets, buf, sizeof(int));
    float packet_loss = (1 - number_of_packets / (float)loop ) * 100;
    if(packet_loss > 20)
    {
        PACKET_LOSS_ACHIEVED = 1;
    }
    cout << "Utrata pakietów: " << packet_loss <<"%\n";
    cout << (1 - (packet_loss / 100))* how_many_bytes * 5 * 8 / 1e6 <<"Mbps\n";
    close(sock2);
}

void receive_packet_download(int sock, int packet_size_, int how_many_bytes){
    int packet_count = 0;
    char *data;
    int id = 0;
    bool terminated = false;
    time_t start = clock();
    packet_response_start server_response;
    struct sockaddr_in read_name;
    read_name.sin_family = AF_INET;
    read_name.sin_addr.s_addr = INADDR_ANY;
    read_name.sin_port = htons(8001);
    sock2 = socket(AF_INET, SOCK_DGRAM, 0);
    if (bind(sock2,(struct sockaddr *)&read_name, sizeof read_name) == -1) {
        perror("binding datagram socket");
        exit(1);
    }
    while(true){
        int number_of_packets;
        time_t first_packet_time;
        time_t last_packet_time;
        int bytes_received = read(sock2, buf, 4096);
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
            if ( read(sock2, buf, 4096) == -1 ) {
                perror("receiving stats packet");
                exit(2); 
            }
            memcpy(&number_of_packets, buf, sizeof(int));
            cout << "num_of_packets: " << number_of_packets << "packet_count: " << packet_count << endl;
            float packet_loss = (1 - packet_count / (float)number_of_packets) * 100;
            if(packet_loss > 20)
            {
                PACKET_LOSS_ACHIEVED = 1;
            }
            cout << "Utrata pakietów: " << packet_loss <<"%\n";
            cout << (1 - (packet_loss / 100))* how_many_bytes * 5 * 8 / 1e6 <<"Mbps\n";
            terminated = true;
        }
        if (id == -1){
            if(!terminated)
                {
                    if ( read(sock2, buf, 4096) == -1 ) {
                        perror("receiving stats packet");
                        exit(2); 
                    }
                    memcpy(&number_of_packets, buf, sizeof(int));
                    cout << "num_of_packets: " << number_of_packets << "packet_count: " << packet_count << endl;
                    float packet_loss = (1 - packet_count/ (float)number_of_packets) * 100;
                    if(packet_loss > 20)
                    {
                        PACKET_LOSS_ACHIEVED = 1;
                    }
                    cout << "Utrata pakietów: " << packet_loss <<"%\n";
                    cout << (1 - (packet_loss / 100))* how_many_bytes * 5 * 8 / 1e6 <<"Mbps\n";
                }
            close(sock2);
            break;
        }
    }
}

void start_test(int type, int sock, int packet_size_, int how_many_bytes){
    if (type == 1)
        receive_packet_download(sock, packet_size_, how_many_bytes);
    if (type == 2)
        send_packet_upload(sock, packet_size_, how_many_bytes);
}

int main(int argc, char *argv[])
{
    if (argc != 3){
        cout << "Prosimy o podanie adresu serwera i portu" << endl;
        exit(0);
    }
    setup_sockets(argv[1], argv[2]);
    pair <int, int> user_input = interface();
    int summary_bytes_to_send = 2e5 / 8; // 0.2Mb / 0.2s = 1Mb/s
    while(true){
        send_first(sock, user_input.first, user_input.second);
        start_test(user_input.first, sock, user_input.second, summary_bytes_to_send);
        if(PACKET_LOSS_ACHIEVED)
            break;
        summary_bytes_to_send += 2e6 / 8; // 2Mb / 0.2s = 10Mb/s
    }
    exit(0);
}
