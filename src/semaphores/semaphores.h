#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#define _XOPEN_SOURCE 700

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
#else
union semun  {
        int val;
        struct semid_ds* buf;
        unsigned short int* array;
};
#endif

int semaphore_create(char*, int);                                    /* tworzy zbior semaforow i nadaje im wartosc 1 (podniesione) */
void P(int, int);                                                    /* opuszcza semafor */
void V(int, int);                                                    /* podnosi semafor */
void semaphore_remove(int);                                          /* usuwa zbior semaforow o wskazanym ID */
int semaphore_value_lookup(int, int);                                /* sprawdza wartosc wybranego semafora */

#endif