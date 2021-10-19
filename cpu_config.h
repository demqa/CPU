#ifndef CPU_CONFIG_H

#define CPU_CONFIG_H

#include "debug_lib.h"

#include <math.h>

u_int32_t signature = 'SSD';
u_int32_t version = 1;

typedef double    Elem_t;
typedef u_int64_t Command;

struct Header
{
    u_int32_t signature;
    u_int32_t version;
    size_t buffsize;
};

enum Commands
{
    HLT  = 0,
    PUSH = 1,
    POP  = 2,
    ADD  = 3,
    MUL  = 4,
    SUB  = 5,
    DIV  = 6,
    OUT  = 7,
    DUMP = 8,

};

u_int32_t CommandCode(char command[20])
{
    u_int32_t code = 65535U;

    if (strcmp(command, "push") == 0)
    {
        code = PUSH;
    }
    else
    if (strcmp(command, "pop")  == 0)
    {
        code = POP;
    }
    else
    if (strcmp(command, "add")  == 0)
    {
        code = ADD;
    }
    else
    if (strcmp(command, "mul")  == 0)
    {
        code = MUL;
    }
    else
    if (strcmp(command, "sub")  == 0)
    {
        code = SUB;
    }
    else
    if (strcmp(command, "div")  == 0)
    {
        code = DIV;
    }
    else
    if (strcmp(command, "out")  == 0)
    {
        code = OUT;
    }
    else
    if (strcmp(command, "hlt")  == 0)
    {
        code = HLT;
    }
    else
    if (strcmp(command, "dump") == 0)
    {
        code = DUMP;
    }
    else
    {
        code = 65535U;
    }

    return code;
}



#endif
