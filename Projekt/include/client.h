#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"

int sock;
struct sockaddr_in name;
struct hostent *hp;
struct sockaddr_in read_name;
char buf[4096];
bool PACKET_LOSS_ACHIEVED = false;
ofstream Plik("./client_logs.txt");

void setup_sockets(const char * address, const char * port);
void send_first(int sock, int type, int packet_size_);
void send_packet_upload(int sock, int packet_size_, int how_many_bytes);
void receive_packet_download(int sock, int packet_size_, int how_many_bytes);
void start_test(int type, int sock, int packet_size_, int how_many_bytes);

#endif
