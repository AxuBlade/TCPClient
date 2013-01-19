#include "semaphores.h"

#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>




union semun semData;

int semaphore_create(char* filename, int semQuantity)  {

  int semId;
  int i;
  int token = ftok(filename, 0);

  if ((semId = semget(token, semQuantity, 0664)) < 0)  {
      if (errno == ENOENT)  {
        if ((semId = semget(token, semQuantity, 0664 | IPC_CREAT | IPC_EXCL)) < 0)  {
          perror("Blad tworzenia semaforow");
          exit(2);
        }
        semData.val = 1;
        for (i = 0; i < semQuantity; ++i) semctl(semId, i, SETVAL, semData);
      } else  {
          perror("Blad tworzenia semaforow");
          exit(3);
        }
    }

  return semId;

}


void P(int semId, int semIdElement)  {

  struct sembuf commands = {semIdElement, -1, 0};

  if (semop(semId, &commands, 1) == -1)  {
    perror("Blad podnoszenia semafora");
    exit(6);
  }

}


void V(int semId , int semIdElement)  {

  struct sembuf commands = {semIdElement, 1, 0};

  if (semop(semId, &commands, 1) == -1)  {
    perror("Blad opuszczania semafora");
    exit(7);
  }

}

int semaphore_value_lookup(int semId, int semIdElement)  {

  return semctl(semId, semIdElement, GETVAL, 0);

}


void semaphore_remove(int semId)  {

  if (semctl(semId, 0, IPC_RMID, 0) < 0)  {
    perror("Nie udalo sie usuwanie semaforow:");
    exit(4);
  }

}
