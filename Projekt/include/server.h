#ifndef SERVER_H
#define SERVER_H

#include "common.h"

char buf[4096];
socklen_t length;
struct sockaddr_in name;
int upload, download;
struct sockaddr_in source_address;
socklen_t source_len = sizeof(source_address);
int summary_bytes_to_send = 1e6 / 8;
ofstream Plik("./server_logs.txt");

void setup_sockets();
pair<packet_start, int> receive_first();
void receive_packet_upload(time_t first_packet_timestamp);
void send_packet_download(int packet_size_, int how_many_bytes);
void start_test(time_t first_packet_timestamp, int type, int packet_size_, int how_many_bytes);

#endif
