#ifndef STATISTICS_H
#define STATISTICS_H

#include "common.h"

float calculate_packet_loss(int packets_received, int packets_sent);
void print_internet_speed(float packet_loss, int how_many_bytes);

#endif
