#include "statistics.h"

float calculate_packet_loss(int packets_received, int packets_sent){
    float packet_loss = (1 - packets_received/ (float)packets_sent) * 100;
    cout << "Utrata pakietów: " << packet_loss <<"%\n";
    return packet_loss;
}

void print_internet_speed(float packet_loss, int how_many_bytes){
    cout << (1 - (packet_loss / 100))* how_many_bytes * 5 * 8 / 1e6 <<"Mbps\n";
    return;
}
