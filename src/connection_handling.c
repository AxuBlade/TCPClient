#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "defines.h"
#include "commands/commands.h"
#include "parsers.h"
#include "connection_handling.h"

int socket_descriptor_create(void)  {

  socketDescriptor = 0;

  if ((socketDescriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)  {
    perror("Klient: Nie udalo sie utworzyc gniazda");
    exit(1);
  } else return socketDescriptor;

}


int connection_creator(struct sockaddr_in serverAddress)  {

  socketDescriptor = socket_descriptor_create();

    if (connect(socketDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0 )  {
        perror( "Klient: Nie mozna nawiazac polaczania z serwerem" );
        exit(1);
    } else  {
        connection_handler(socketDescriptor);
        return socketDescriptor;
    }

}



void connection_handler(int socket)  {

  char buffer[__BUFFER_SIZE];
  int readSize;
  struct cmdStruct* commands;
  int i;
  short int statTemp;                                                            /* zmienne tylko po to, aby write i fgets mial gdzie zwracac*/
  char* strTemp;
  int childPid;

  childPid = fork();

  if (childPid == 0)  {
    sigset(SIGINT, reader_kill);
    memset(buffer, 0, __BUFFER_SIZE);
    while ((readSize = recv( socket, buffer, __BUFFER_SIZE, 0 )) > 0 )  {
      printf("%s", buffer);
      fflush( stdout );
      memset(buffer, 0, __BUFFER_SIZE);
    }
    close(socket);
    exit(0);
  } else  {
      readerPid = childPid;
      while (1)  {
        strTemp = fgets( buffer, __BUFFER_SIZE, stdin );
        commands = splitter(buffer);
        if (!strncmp(commands->words[0], "put", 3))  {
          kill(readerPid, SIGINT);
          wait(0);
          statTemp = write(socket, buffer, strlen(buffer));
          put_file(commands, socket);
          close(socket);
          break;
        }
        if (!strncmp(commands->words[0], "replace", 7))  {
          kill(readerPid, SIGINT);
          wait(0);
          statTemp = write(socket, buffer, strlen(buffer));
          replace_file(commands, socket);
          close(socket);
          break;
        }
        if (!strncmp(commands->words[0], "get", 3))  {
          kill(readerPid, SIGINT);
          wait(0);
          statTemp = write(socket, buffer, strlen(buffer));
          get_file(commands, socket);
          break;
        }
        if (!strncmp(commands->words[0], "quit", 4))  {
          statTemp = write(socket, buffer, strlen(buffer));
          kill(readerPid, SIGINT);
          close(socket);
          exit(0);
        }
        else statTemp = write(socket, buffer, strlen(buffer));
      }
    }
    close(socket);

}


void sigint_handler(int signo)  {
  
  kill(readerPid, SIGINT);
  while(wait(0) != -1) continue;
  close(socketDescriptor);
  exit(0);

}
void reader_kill(int signo)  {
  close(socketDescriptor);
  exit(0);
}