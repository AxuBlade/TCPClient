#ifndef CONNECTION_HANDLING_H
#define CONNECTION_HANDLING_H

pid_t connectionHandler(int);
int socket_descriptor_create(void);
int connection_creator(struct sockaddr_in);
void sigint_handler(int);
#endif