#ifndef COMMON_H
#define COMMON_H

#define MAX_CLIENTS 2
#define MAX_BUF_SIZE 1024

#define PORT 8080

#define ADD 1
#define SUB 2
#define MUL 3
#define DIV 4
#define TRY 5
#define REPLY 6
#define EXIT 7

#define OK 0
#define NAME_TAKEN 1
#define LIST_FULL 2

struct Message
{
    int number;
    int action;
    int arg1;
    int arg2;
};

struct Reply
{
    int number;
    char name[64];
    int result;
};

#endif
