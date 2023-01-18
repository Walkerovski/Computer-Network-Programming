#include "common.h"
using namespace std;

int sock, sock2;
struct sockaddr_in name;
struct hostent *hp;

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
    cout<<"Prosimy o podanie wielkości pakietu. Wielkośc pakiety musi być z przedziału 50 - 1000 (wielkość podana w bajtach)\n";
    cin>>input;
    int size_input = atoi(input.c_str());
    while(size_input > 1000 || size_input < 50){
        cout<<"Podane dane na wejściu są złe! Proszę wpisać wartość z przedziału 50 - 1000 (wielkość podana w bajtach)\n";
        cin>>input;
        size_input = atoi(input.c_str());
    }
    return {type_input, size_input};
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
    char buf[1024];
    int cos;
    int cos2;
    time_t cos3;
    if ( read(sock2, buf, 1024) == -1 ) {
        perror("receiving start packet");
        exit(2); 
    }
    timestamp = chrono::time_point_cast<std::chrono::microseconds>(chrono::system_clock::now());
    time_t last_packet_time = timestamp.time_since_epoch().count();
    memcpy(&cos, buf, sizeof(int));
    memcpy(&cos2, buf + sizeof(int), sizeof(int));
    memcpy(&cos3, buf + 2 * sizeof(int), sizeof(time_t));
    cout << cos << " " << cos2 << " " << cos3 <<"\n";
    cout << "Ping: " << (last_packet_time - first_packet_time) / 1000.0 << "ms" << endl; 
    close(sock2);
}

void send_packet(int sock, int packet_size_, int how_many_bytes){
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
        bytes_send += packet_size_;
        ++loop;
    }
    char stop[] = "STOP";
    packet_test last_packet = {.id = -1, .data = stop};
    if (sendto(sock, (const void *) &last_packet, sizeof(last_packet), 0, (struct sockaddr *) &name,sizeof name) == -1)
            perror("sending datagram message");
    
    struct sockaddr_in read_name;
    read_name.sin_family = AF_INET;
    read_name.sin_addr.s_addr = INADDR_ANY;
    read_name.sin_port = htons(8001);
    sock2 = socket(AF_INET, SOCK_DGRAM, 0);
    if (bind(sock2,(struct sockaddr *)&read_name, sizeof read_name) == -1) {
        perror("binding datagram socket");
        exit(1);
    }

    char buf[1024];
    int number_of_packets;
    time_t first_packet;
    time_t last_packet_time;
    if ( read(sock2, buf, 1024) == -1 ) {
        perror("receiving start packet");
        exit(2); 
    }
    memcpy(&number_of_packets, buf, sizeof(int));
    // memcpy(&first_packet, buf + 2 * sizeof(int), sizeof(time_t));
    // memcpy(&last_packet, buf + 2 * sizeof(int) + sizeof(time_t), sizeof(time_t));
    cout << number_of_packets << " " << " " << first_packet << " " << last_packet_time << endl;
    close(sock2);
}

int main(int argc, char *argv[])
{
    /* Create socket on which to send. */ 
    packet_test cos;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("opening datagram socket");
        exit(1);
    }
    hp = gethostbyname(argv[1]);
    if (hp == (struct hostent *) 0) {
        fprintf(stderr, "%s: unknown host\n", argv[1]);
        exit(2);
    }
    memcpy((char *) &name.sin_addr, (char *) hp->h_addr, hp->h_length);
    name.sin_family = AF_INET;
    name.sin_port = htons( atoi( argv[2] ));
    pair <int, int> user_input = interface();
    int summary_bytes_to_send = 200000 / 8; // 0.2Mb / 0.2s = 1Mb/s
    while(true){
        send_first(sock, user_input.first, user_input.second);
        send_packet(sock, user_input.second, summary_bytes_to_send);
        summary_bytes_to_send += 2000000 / 8; // 2Mb / 0.2s = 10Mb/s
    }
    exit(0);
}
