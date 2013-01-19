#include "defines.h"
#include "connection_handling.h"
#include "parsers.h"
#include "semaphores/flags.h"
#include <stdio.h>
#include <signal.h>

int main (int argc, char** argv)  {

  struct sockaddr_in serverAddress;

  sigset(SIGINT,sigint_handler);
  set_flag(0, 1);
  serverAddress = pars_input(argc,argv);
  printf("Klient: Łączenie...\n");
  while (get_flag(0))  {
    connection_creator(serverAddress);
  }
  kill(readerPid, SIGINT);
  close(socketDescriptor);
  while(wait(0) != -1) continue;
  exit(0);

}