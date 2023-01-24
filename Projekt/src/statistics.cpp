#include "statistics.h"

double calculate_packet_loss(double packets_received, double packets_sent){
    double packet_loss = (1 - packets_received/ packets_sent) * 100;
    cout << "Utrata pakietów: " << packet_loss <<"%\n";
    return packet_loss;
}

void print_internet_speed(double packet_loss, int how_many_bytes){
    cout << (1 - (packet_loss / 100))* how_many_bytes * 8 / 1e6 <<"Mbps\n";
    return;
}
