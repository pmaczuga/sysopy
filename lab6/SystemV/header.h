#ifndef HEADER_H
#define HEADER_H

#define PROJECT_ID 'P'
#define CLIENT_PROJECT_ID rand()%255 + 1
#define MAX_SIZE 1024
#define KEY_PATH getenv("HOME")
#define MAX_CLIENTS 10

#define START 1
#define END 2
#define STOP 3

#define MIRROR 4
#define TIME 5

#define ADD 6
#define SUB 7
#define MUL 8
#define DIV 9

#endif