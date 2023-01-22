#ifndef STATISTICS_H
#define STATISTICS_H

#include "common.h"

double calculate_packet_loss(double packets_received, double packets_sent);
void print_internet_speed(double packet_loss, int how_many_bytes);

#endif
