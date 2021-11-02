#include <string.h>
#include <time.h>

#include "cpu_config.h"
#include "stack/stack.h"
#include "stack/print_func.cpp"

#include "debug_lib.h"

#define PROCESSING_ERROR(msg){                                                                          \
    printf("cur_cmd #%lu [%s:%d] message = %s\n", cpu.cur_cmd, __PRETTY_FUNCTION__, __LINE__, #msg);  \
    StackDtor(&cpu.stack);                                                                                    \
    return 0;                                                                                              \
}

#define ASSIGN_AND_GO_NEXT(name, type)   \
    type name = *(type *)ip;              \
    ip += sizeof(type);

void sleep(int milliseconds)
{
    clock_t time_end = clock() + milliseconds * CLOCKS_PER_SEC / 1000;
    while (clock() < time_end) ;
}

struct CPU
{
    stack_t stack;
    stack_t stack_adr;

    char *code;
    size_t code_size;

    size_t cur_cmd;

    Elem_t regs[5];
    Elem_t *RAM;
};


int main(int argc, char *argv[])
{
    Header file_info = {};
    char *header     = nullptr;
    CPU cpu = {};

    if (argc == 2)
    {
        FILE *stream = fopen(argv[1], "rb");
        if (stream == nullptr)
        {
            perror("FATAL ERROR, CANT OPEN FILE");
            return 0;
        }

        header = (char *) calloc(sizeof(Header), sizeof(char));

        size_t n_bytes = fread(header, sizeof(char), sizeof(Header), stream);
        if (ferror(stream))
        {
            perror("THERE IS SOME ERROR IN FILE READING");
        }
        if (feof(stream))
        {
            perror("EOF REACHED");
        }
        if (n_bytes != sizeof(Header))
        {
            perror("THIS BINARY HAS AN ERROR. TOO SHORT TO BE MY FILE");
            fclose(stream);
            free(header);
            return 0;
        }

        if (*((u_int32_t *)header) == signature)
        {
            file_info = *(Header *)header;
        }
        else
        {
            perror("THIS BINARY ISNT MINE\n");
            fclose(stream);
            return 0;
        }

        if (file_info.version != version)
        {
            perror("THIS BINARY IS OLDER THAN PROGRAM, RECOMPILE ASM\n");
            fclose(stream);
            return 0;
        }


        cpu.code_size = file_info.buffsize;
        cpu.code = (char *) calloc(file_info.buffsize + 1, sizeof(char));
                                                // because read want to do smth
                                                // with End Of File
                                                // I think

        n_bytes = fread(cpu.code, sizeof(char), file_info.buffsize, stream);
        if (n_bytes != file_info.buffsize)
        {
            perror("WRONG BUFFSIZE or smth, i dont really know how to call this error (file hasnt reached eof)");
        }

        fclose(stream);
    }
    else
    {
        perror("give me nice arguments, pls\n");
        return 0;
    }

    StackCtor(&cpu.stack, 0, PrintDouble);

    char *ip = nullptr;

    cpu.RAM = (Elem_t *) calloc(RamSize, sizeof(Elem_t));
    if (cpu.RAM == nullptr)
    {
        StackDtor(&cpu.stack);
        perror("NO RAM IN PROGRAM");
        return -1;
    }

    for (ip = cpu.code; ip < cpu.code + file_info.buffsize; ++cpu.cur_cmd)
    {
        ASSIGN_AND_GO_NEXT(command, Command);

        switch (command & ~(IMM | REG | OSU))
        {
            #define DEF_CMD(cmd_name, cmd_num, cmd_n_args, cmd_code) \
            {                                                         \
                case CMD_ ## cmd_name:                                 \
                    cmd_code                                            \
                    break;                                               \
            }

            #define DEF_JMP(jmp_name, jmp_num, jmp_sign) \
            {                                             \
                case JMP_ ## jmp_name:                     \
                    if (jmp_num == 0x10) /* case JMP */     \
                    {                                        \
                        ASSIGN_AND_GO_NEXT(index, size_t);    \
                        ip = cpu.code + index;                 \
                    }                                           \
                                                                 \
                    Elem_t x = POP;                               \
                    Elem_t y = POP;                                \
                    if (y jmp_sign x)                               \
                    {                                                \
                        ASSIGN_AND_GO_NEXT(index, size_t);            \
                                                                       \
                        ip = cpu.code + index;                          \
                    }                                                    \
                    else                                                  \
                    {                                                      \
                        ip += sizeof(size_t);                               \
                    }                                                        \
                                                                              \
                    PUSH(y);                                                   \
                    PUSH(x);                                                    \
                    break;                                                       \
            }

            #include "commands"
            
            #undef DEF_CMD
            #undef DEF_JMP
            #undef ASSIGN_AND_GO_NEXT

            default:
                PROCESSING_ERROR(UNKNOWN_CMD);
        }
    }

    perror("WRONG_ASSEMBLER_CODE, NO HLT");
    StackDtor(&cpu.stack);

    free(cpu.RAM);
    free(header);
    free(cpu.code);

    return 0;
}