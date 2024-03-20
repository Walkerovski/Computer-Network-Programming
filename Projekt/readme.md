# Network performance tester

## Members
- Szymon Dyszewski
- Karol Kasperek
- Piotr Kosmala
- Mikołaj Gutowski

## Task
Design of a network performance tester using the UDP protocol. The tool would consist of a server and a client (or one role-taking program depending on the call parameters). The call parameter would be the size of the data packet and the server address in the case of the client. At the end of the test, statistics on upload and download speeds and packet loss (plus possibly delay) would be displayed. It would be good (but not necessarily) if the tests could be carried out in real conditions of wide-area networks, which requires server access directly from the Internet (server on the Internet or a free instance, e.g. e2-micro in GCP or port forwarding in some home Internet service).