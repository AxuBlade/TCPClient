#ifndef CONNECTION_HANDLING_H
#define CONNECTION_HANDLING_H

void connection_handler(int);
int socket_descriptor_create(void);
int connection_creator(struct sockaddr_in);
void sigint_handler(int);
void reader_kill(int);
#endif