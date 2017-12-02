#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <limits.h>
#include <thread>
#include <chrono>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include <sys/un.h>
#include <vector>
#include <pthread.h>
#include <thread>
#include <chrono>

#include "libmigrateclient.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "Usage: client <ip address of server>" << std::endl;
    exit(1);
  }
  char *ip_addr = argv[1];
  int sock;
  struct sockaddr_in addr;

  std::cout << "Connecting to " << ip_addr << std::endl;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("sock");
    exit(1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip_addr);
  addr.sin_port = htons(8000);

  if (connect(sock, (sockaddr *) &addr, sizeof(addr)) < 0) {
    perror("connect");
    exit(1);
  }

  char buf[255];
  int in_bytes;

  MigrateClient *migrate_client = InitMigrateClient(8000);
  migrate_client->remote_sock = sock;
  SendDescriptor(migrate_client, sock);

  int prev_sock = sock;

  while (1) {
    if (migrate_client->remote_sock != prev_sock) {
      std::this_thread::sleep_for(std::chrono::seconds(10));
      prev_sock = migrate_client->remote_sock;
    }
    in_bytes = recv(migrate_client->remote_sock, buf, 255, 0);
    if (in_bytes < 0) {
      perror("recv");
      std::cout << "Error connecting to socket" << std::endl;
    } else if (in_bytes == 0) {
      std::cout << "Connection closed" << std::endl;
    } else {
      std::string msg = std::string(buf, in_bytes);
      std::cout << msg << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
