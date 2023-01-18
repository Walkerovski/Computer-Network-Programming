#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <time.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include <chrono>

using namespace std;

enum type{UPLOAD = 2, DOWNLOAD = 1};

struct packet_start{ 
    enum type type;
    int packet_size;
    time_t timestamp_;
};

struct packet_response_start{ 
    int number_of_packets;
    time_t first_packet;
    time_t last_packet;
};

struct packet_test{ 
    int id;
    char *data;
};
