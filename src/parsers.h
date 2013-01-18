#ifndef PARSERS_H
#define PARSERS_H

struct sockaddr_in pars_input(int, char**);
int splitter_countWords(char*);
struct cmdStruct* splitter(char*);

#endif