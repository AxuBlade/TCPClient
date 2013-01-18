#include <arpa/inet.h>
#include <getopt.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "defines.h"
#include "parsers.h"

struct sockaddr_in pars_input(int argc_s, char** argv_s) {

  int opt;
  struct sockaddr_in parsedArgs;
  static const char *optString = "i:p:";                                         /*rozpoznawalne krotkie opcje dla parsera*/
  static const struct option longOpts[] = {
    {"interface", required_argument, NULL, 'i'},                                 /*tablica opcji z pelnymi nazwami opcji*/
    {"port", required_argument, NULL, 'p'}                                       /*oraz ich nazwami skrotowymi*/
  };

  memset(&parsedArgs, 0, sizeof(struct sockaddr_in) );

  parsedArgs.sin_addr.s_addr = inet_addr("127.0.0.1");                           /*domyslnie: localhost*/
  parsedArgs.sin_port = htons((short) atoi("13333"));                            /*domyslnie: port 13333*/
  while ((opt = getopt_long(argc_s, argv_s, optString, longOpts, NULL )) != -1)  {
    switch (opt)  {
      case 'p':
        parsedArgs.sin_port = htons((short) atoi(optarg));
        break;

      case 'i':
        parsedArgs.sin_addr.s_addr = inet_addr(optarg);
        break;

      default:                                                                   /*opcja tylko dla celow zgodnosci ze standardem. Nie ma prawa wystąpic*/
        break;
    }
  }

  parsedArgs.sin_family = AF_INET;

  return parsedArgs;

}




int splitter_countWords(char* buffer)  {

  int i;
  int wordCount = 1;

  for (i = 0; i < (strlen(buffer)); ++i)  {
    if ((int)buffer[i] == 32) wordCount++;
  }
  return wordCount;

}


struct cmdStruct* splitter(char* buffer)  {

  int i;
  int wordSize = 0;
  int vTabSize = splitter_countWords(buffer);
  char spaces[] = {' ', '\n', '\0'};                                             /*tablica bialych znaków, mozliwych do wystapienia (spacja i nowa linia)*/
  char** temp = (char**) malloc((vTabSize + 1) * sizeof(char*));
  struct cmdStruct* tempStruct;

  tempStruct = (struct cmdStruct*) malloc(sizeof(struct cmdStruct));             /*inicjalizacja struktury*/
  for (i = 0; i < vTabSize; ++i)  {
    wordSize = strcspn(buffer, spaces);
    temp[i] = (char*) malloc((wordSize + 1) * sizeof(char));
    memset(temp[i], 0, sizeof(temp[i]));
    strncpy(temp[i], buffer, wordSize);
    buffer = buffer + wordSize + 1;
  }

  temp[vTabSize] = NULL;                                                         /*ostatni element tablicy **words musi miec wartosc NULL dla zgodnosci z execvp*/

  tempStruct->wordCount = vTabSize;
  tempStruct->words = temp;

  return tempStruct;

}