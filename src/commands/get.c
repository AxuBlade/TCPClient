#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../defines.h"
#include "commands.h"

void get_file(struct cmdStruct* commands, int socket)  {

  FILE* fileStream;
  char writeBuffer[__BUFFER_SIZE];

  int recvSize = 0;
  int writeSize = 0;
  short int statTemp;                                                            /* zmienna tylko po to, aby write i fgets mial gdzie zwracac*/
  char* writeMode;
  char* writeModeText = "wt";
  char* writeModeBinary = "wr";
  struct stat fileStats;

  fflush(stdout);
  if (commands->wordCount == 4 )  {

    int textModeCheck = !strncmp(commands->words[3], "-t", 2) || !strncmp(commands->words[3], "--text", 6);
    int binaryModeCheck = !strncmp(commands->words[3], "-b", 2) || !strncmp(commands->words[3], "--binary", 8);

    if (binaryModeCheck || textModeCheck) {
      if (textModeCheck) writeMode = writeModeText;
      else if (binaryModeCheck) writeMode = writeModeBinary;
      if ((fileStream = fopen(commands->words[2], writeMode)) == NULL)  {
        printf("Klient: Nie udalo sie utworzyc pliku\n");
      } else  {
          while ((recvSize = recv(socket, writeBuffer, __BUFFER_SIZE, 0)) > 0)  {
            writeSize = fwrite(writeBuffer, sizeof(char), recvSize, fileStream);
            if (recvSize != writeSize)  {
              remove(commands->words[2]);
              printf("Klient: Blad transmisji\n");
              break;
            }
          }
          fclose(fileStream);
          close(socket);
          stat(commands->words[2], &fileStats);
          if (!fileStats.st_size)  {
            printf("Klient: Rozmiar pobranego pliku: %d -> usuwanie...\n", (signed int)fileStats.st_size);
            remove(commands->words[1]);
          } else  {
              printf("Klient: Rozmiar pobranego pliku: %d\n", (signed int)fileStats.st_size);
          }
        }
    } else  {
        printf("Klient: Za malo argumentow\n");
        close(socket);
    }
  } else  {
      printf("Klient: Za malo argumentow\n");
      close(socket);
  }

}