#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>



#define __BUFFER_SIZE 2048

int socketKill;
pid_t readerPid;
int semId;

struct cmdStruct {                                                             /*struktura do przechowywania komend przeslanych przez uzytkownika w postaci poszczegolnych slow*/
  char** words;
  int    wordCount;
};

void reader_killer (int);
void put_file(struct cmdStruct*, int);
void get_file(struct cmdStruct*, int);
void connect_creator(struct sockaddr_in);