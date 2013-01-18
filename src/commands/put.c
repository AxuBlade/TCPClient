#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include "../defines.h"
#include "commands.h"

void put_file(struct cmdStruct* commands, int socket)  {

  FILE* fileStream;
  char readBuffer[__BUFFER_SIZE];
  int readSize = 0;
  char* readModeBinary = "rb";
  char* readModeText = "rt";
  char* readMode;

  if (commands->wordCount == 4) {

    int textModeCheck = !strncmp(commands->words[3], "-t", 2) || !strncmp(commands->words[3], "--text", 6);
    int binaryModeCheck = !strncmp(commands->words[3], "-b", 2) || !strncmp(commands->words[3], "--binary", 8);

      if (binaryModeCheck || textModeCheck) {
        fflush(stdout);
        if (textModeCheck) readMode = readModeText;
        else if (binaryModeCheck) readMode = readModeBinary;
        if ((fileStream = fopen(commands->words[1],readMode)) == NULL)  {
          printf("Klient: Nie udalo sie otworzyc pliku\n");
        } else  {
            while ((readSize = fread(readBuffer, sizeof(char), __BUFFER_SIZE, fileStream)) > 0)  {
              send(socket, readBuffer, readSize, 0);
            }
            fclose(fileStream);
            shutdown(socket, 1);
            printf("Klient: Zakonczono wysylanie pliku. ponowne laczenie...\n");
        }
      } else  {
        printf("Nie wybrano trybu przesylania\n");
        shutdown(socket, 1);
      }
  } else {
      printf("Klient: Za malo argumentow\n");
      shutdown(socket, 1);
  }

}