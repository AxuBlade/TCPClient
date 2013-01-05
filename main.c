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

  parsedArgs.sin_addr.s_addr = inet_addr("127.0.0.1");                             /*domyslnie: localhost*/
  parsedArgs.sin_port = htons((short) atoi("13333"));                                 /*domyslnie: port 13333*/
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
    perror("nie udalo sie utworzyc gniazda");
    exit(1);
    } else  {
    return socketDescriptor;
    }


}


void connection_creator(struct sockaddr_in serverAddress)  {

  int socketDescriptor = socket_descriptor_create();

  if(connect(socketDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0 )  {
      perror( "nie mozna nawiazac polaczania z serwerem" );
      exit(1);
  } else {
   connectionHandler(socketDescriptor);
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

  temp[vTabSize] = NULL;                                                           /*ostatni element tablicy **words musi miec wartosc NULL dla zgodnosci z execvp*/

  tempStruct->wordCount = vTabSize;
  tempStruct->words = temp;

  return tempStruct;


}


pid_t connectionHandler(int socket)  {

  char buffer[__BUFFER_SIZE];
  int readSize;
  struct cmdStruct* commands;
  int i;
  short int statTemp;                                                          /* zmienne tylko po to, aby write i fgets mial gdzie zwracac*/
  char* strTemp;

  readerPid = fork();
  if(readerPid == 0)  {
    socketKill = socket;
    signal( SIGINT, reader_killer );
    memset(buffer,0,__BUFFER_SIZE);
    while( (readSize = recv( socket, buffer, __BUFFER_SIZE, 0 )) > 0 )  {
      for( i=0; i<readSize; i++ ) {
           printf("%c", buffer[i]);
        }
        fflush( stdout );
      memset(buffer,0,__BUFFER_SIZE);
      }
    } else  {
    sleep(1);
    while(1)  {
      strTemp = fgets( buffer, __BUFFER_SIZE, stdin );
      commands = splitter(buffer);
      statTemp = write(socket, buffer, strlen(buffer));
      if(!strncmp(commands->words[0],"quit",6))  {
        kill(readerPid, SIGINT);
        exit(0);
      }
      if(!strncmp(commands->words[0],"put",3))  {
        put_file(commands, socket);
        kill(readerPid, SIGINT);
        break;
        }
      if(!strncmp(commands->words[0],"get",3))  {
        get_file(commands, socket);
        kill(readerPid, SIGINT);
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
  int recvSize = 0;
  int writeSize = 0;
  short int statTemp;                                                          /* zmienna tylko po to, aby write i fgets mial gdzie zwracac*/
  char* errorMsg1 = "Nie udalo sie utworzyc pliku\n";
  char* errorMsg2 = "Blad transmisji\n";
  char* completeMsg = "Transfer pomyslnie zakonczono\n";
  char* writeMode;
  char* writeModeText = "wt";
  char* writeModeBinary = "wr";

  if(!strncmp(commands->words[2],"-b",2) || !strncmp(commands->words[2],"--binary",8)) writeMode = writeModeBinary;
  else writeMode = writeModeText;                                                /*domyslny tryb transferu :tekstowy*/
  if((fileStream = fopen(commands->words[1],writeMode)) == NULL)  {
    statTemp = write(socket,errorMsg1,strlen(errorMsg1));
    }
  else  {
    printf("start processing...\n");
    while((recvSize = recv(socket, writeBuffer, __BUFFER_SIZE, 0)) > 0) {
      writeSize = fwrite(writeBuffer, sizeof(char), recvSize, fileStream);
      if(recvSize != writeSize) statTemp = write(socket, errorMsg2, strlen(errorMsg2));
      remove(commands->words[2]);
      printf("blad transferu\n");
      break;
      }
    printf("kjkjslk\n");
    fclose(fileStream);
    }


}


void put_file(struct cmdStruct* commands, int socket)  {

  FILE* fileStream;
  char readBuffer[__BUFFER_SIZE];
  int readSize = 0;
  char* errorMsg = "Nie udalo sie otworzyc pliku\n";
  char* readModeBinary = "rb";
  char* readModeText = "rt";
  char* readMode;
  int a = 0;

  if(commands->wordCount < 3 || !strncmp(commands->words[2],"-t",2) || !strncmp(commands->words[2],"--text",8)) readMode = readModeText;
  else if(!strncmp(commands->words[2],"-b",2) || !strncmp(commands->words[2],"--binary",8)) readMode = readModeBinary;
  fflush(stdout);
  if((fileStream = fopen(commands->words[1],readMode)) == NULL)  {
    printf("%s",errorMsg);
    } else  {
    while((readSize = fread(readBuffer, sizeof(char), __BUFFER_SIZE, fileStream)) > 0)  {
      send(socket, readBuffer, readSize, 0);
      }
      fclose(fileStream);
      shutdown(socket, 1);
      printf("Zakonczono wysylanie pliku. ponowne laczenie...\n");
    }


}


void reader_killer(int signo)  {

  close(socketKill);
  exit(0);


}


int main (int argc, char** argv)  {

  int socketDescriptor;
  struct sockaddr_in serverAddress;

  signal(SIGINT,reader_killer);
  serverAddress = pars_input(argc,argv);
  printf("łączenie...\n");
  while(1)  {
    connection_creator(serverAddress);
    }
  kill(readerPid, SIGINT);
  close(socketDescriptor);
  while(wait(0) != -1) printf("smiec\n");


}