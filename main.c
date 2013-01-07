#include "header.h"


struct sockaddr_in pars_input(int argc_s, char **argv_s) {

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
  while((opt = getopt_long(argc_s, argv_s, optString, longOpts, NULL )) != -1)  {
    switch(opt)  {
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


int socket_descriptor_create(void)  {

  int socketDescriptor = 0;

  if((socketDescriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)  {
    perror("Klient: Nie udalo sie utworzyc gniazda");
    exit(1);
  } else return socketDescriptor;


}


int connection_creator(struct sockaddr_in serverAddress)  {

  int socketDescriptor = socket_descriptor_create();

  if(connect(socketDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0 )  {
      perror( "Klient: Nie mozna nawiazac polaczania z serwerem" );
      exit(1);
  } else  {
      connectionHandler(socketDescriptor);
      return socketDescriptor;
  }


}


int splitter_countWords(char* buffer)  {

  int i;
  int wordCount = 0;

  for(i = 0; i < (strlen(buffer)); ++i)  {
    if((int)buffer[i] == 32) wordCount++;
  }
  return wordCount + 1;


}


struct cmdStruct* splitter(char* buffer)  {

  int i;
  int wordSize = 0;
  int vTabSize = splitter_countWords(buffer);
  char spaces[] = {' ', '\n', '\0'};                                             /*tablica bialych znaków, mozliwych do wystapienia (spacja i nowa linia)*/
  char** temp = (char**) malloc((vTabSize + 1) * sizeof(char*));
  struct cmdStruct* tempStruct;

  tempStruct = (struct cmdStruct*) malloc(sizeof(struct cmdStruct));             /*inicjalizacja struktury*/
  for(i = 0; i < vTabSize; ++i)  {
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


pid_t connectionHandler(int socket)  {

  char buffer[__BUFFER_SIZE];
  int readSize;
  struct cmdStruct* commands;
  int i;
  short int statTemp;                                                            /* zmienne tylko po to, aby write i fgets mial gdzie zwracac*/
  char* strTemp;
  char* temp;
  readerPid = fork();
  if(readerPid == 0)  {
    socketKill = socket;
    /*signal( SIGINT, reader_killer );*/
    memset(buffer,0,__BUFFER_SIZE);
    while( (readSize = recv( socket, buffer, __BUFFER_SIZE, 0 )) > 0 )  {
      printf("%s", buffer);
      fflush( stdout );
      memset(buffer,0,__BUFFER_SIZE);
    }
    exit(0);
  } else  {
      sleep(1);
      while(1)  {
        strTemp = fgets( buffer, __BUFFER_SIZE, stdin );
        commands = splitter(buffer);
        statTemp = write(socket, buffer, strlen(buffer));
        if(!strncmp(commands->words[0],"quit",6))  {
          close(socket);
          exit(0);
        }
        if(!strncmp(commands->words[0],"put",3))  {
          put_file(commands, socket);
          close(socket);
          break;
        }
        if(!strncmp(commands->words[0],"replace",7))  {
          temp =  commands->words[1];
          commands->words[1]=commands->words[2];
          commands->words[2]=temp;
          put_file(commands, socket);
          close(socket);
          break;
        }
        if(!strncmp(commands->words[0],"get",3))  {
          get_file(commands, socket);
          break;
        }
      }
    }
    close(socket);
    return readerPid;


}


void get_file(struct cmdStruct* commands, int socket)  {

  FILE* fileStream;
  char writeBuffer[__BUFFER_SIZE];
  int textModeCheck = !strncmp(commands->words[3],"-t",2) || !strncmp(commands->words[3],"--text",8);
  int binaryModeCheck = !strncmp(commands->words[3],"-b",2) || !strncmp(commands->words[3],"--binary",8);
  int recvSize = 0;
  int writeSize = 0;
  short int statTemp;                                                            /* zmienna tylko po to, aby write i fgets mial gdzie zwracac*/
  char* errorMsg1 = "Klient: Nie udalo sie utworzyc pliku\n";
  char* errorMsg2 = "Klient: Blad transmisji\n";
  char* completeMsg = "Klient: Transfer pomyslnie zakonczono\n";
  char* writeMode;
  char* writeModeText = "wt";
  char* writeModeBinary = "wr";
  struct stat fileStats;

  if(commands->wordCount == 4 && (binaryModeCheck || textModeCheck))  {
    if((fileStream = fopen(commands->words[1], writeMode)) == NULL)  {
      statTemp = write(socket,errorMsg1,strlen(errorMsg1));
    } else  {
        while((recvSize = recv(socket, writeBuffer, __BUFFER_SIZE, 0)) > 0)  {
          writeSize = fwrite(writeBuffer, sizeof(char), recvSize, fileStream);
          if(recvSize != writeSize)  {
            remove(commands->words[1]);
            printf("blad transferu\n");
            break;
          }
        }
        fclose(fileStream);
        close(socket);
        stat(commands->words[1], &fileStats);
        if(!fileStats.st_size)  {
          printf("Klient: Rozmiar pobranego pliku: %d -> usuwanie...\n", (signed int)fileStats.st_size);
          remove(commands->words[1]);
        } else  {
          printf("Klient: Rozmiar pobranego pliku: %d\n", (signed int)fileStats.st_size);
        }
      }
    } else  printf("Klient: Za malo argumentow\n");
    close(socket);


}




void put_file(struct cmdStruct* commands, int socket)  {

  FILE* fileStream;
  char readBuffer[__BUFFER_SIZE];
  int readSize = 0;
  int textModeCheck = !strncmp(commands->words[3],"-t",2) || !strncmp(commands->words[3],"--text",8);
  int binaryModeCheck = !strncmp(commands->words[3],"-b",2) || !strncmp(commands->words[3],"--binary",8);
  char* errorMsg = "Klient: Nie udalo sie otworzyc pliku\n";
  char* readModeBinary = "rb";
  char* readModeText = "rt";
  char* readMode;

  if(commands->wordCount == 4 && (binaryModeCheck || textModeCheck))  {
    if(textModeCheck) readMode = readModeText;
    else if(binaryModeCheck) readMode = readModeBinary;
    if((fileStream = fopen(commands->words[1],readMode)) == NULL)  {
      printf("%s",errorMsg);
    } else  {
        while((readSize = fread(readBuffer, sizeof(char), __BUFFER_SIZE, fileStream)) > 0)  {
          send(socket, readBuffer, readSize, 0);
        }
        fclose(fileStream);
        shutdown(socket, 1);
        printf("Klient: Zakonczono wysylanie pliku. ponowne laczenie...\n");
    }
  }
  else printf("Klient: Za malo argumentow\n");
  shutdown(socket, 1);

}


void reader_killer(int signo)  {

  close(socketKill);
  exit(0);


}


int main (int argc, char** argv)  {

  int socketDescriptor;
  struct sockaddr_in serverAddress;

  sigset(SIGINT,reader_killer);
  serverAddress = pars_input(argc,argv);
  printf("Klient: Łączenie...\n");
  while(1)  {
    socketDescriptor = connection_creator(serverAddress);
  }
  kill(readerPid, SIGINT);
  close(socketDescriptor);
  while(wait(0) != -1) continue;


}