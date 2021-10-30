#include <string.h>
#include "cpu_config.h"
#include "stack/stack.h"
#include "stack/print_func.cpp"

#include "debug_lib.h"

#define PROCESSING_ERROR(msg){                                                                          \
    printf("instruction #%d [%s:%d] message = %s\n", instruction, __PRETTY_FUNCTION__, __LINE__, #msg);  \
    StackDtor(&stack);                                                                                    \
    return 0;                                                                                              \
}

#define ASSIGN_AND_GO_NEXT(name, type)   \
    type name = *(type *)ip;              \
    ip += sizeof(type);

int main(int argc, char *argv[])
{
    Header file_info = {};
    char *ptr    = nullptr;
    char *buffer = nullptr;

    if (argc == 2)
    {
        FILE *stream = fopen(argv[1], "rb");
        if (stream == nullptr)
        {
            perror("FATAL ERROR, STREAM IS NULL");
            return 0;
        }

        ptr = (char *) calloc(sizeof(Header), sizeof(char));

        size_t n_bytes = fread(ptr, sizeof(char), sizeof(Header), stream);
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
            free(ptr);
            return 0;
        }

        if (*((u_int32_t *)ptr) == signature)
        {
            file_info = *(Header *)ptr;
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

        buffer = (char *) calloc(file_info.buffsize + 1, sizeof(char));
                                                // because read want to do smth
                                                // with End Of File
                                                // I think

        n_bytes = fread(buffer, sizeof(char), file_info.buffsize, stream);
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

    stack_t stack = {};

    StackCtor(&stack, 0, PrintDouble);

    char *ip = nullptr;
    int instruction = 1;

    Elem_t regs[5] = {};
    Elem_t *RAM = (Elem_t *) calloc(RamSize, sizeof(Elem_t));
    if (RAM == nullptr)
    {
        StackDtor(&stack);
        perror("NO RAM IN PROGRAM");
        return -1;
    }

    for (ip = buffer; ip < buffer + file_info.buffsize; ++instruction)
    {
        Command command = *(Command *)ip;
        ip += sizeof(Command);

        switch (command & ~(IMM | REG | OSU))
        {
            #define DEF_CMD(cmd_name, cmd_num, cmd_n_args, cmd_code) \
            {                                                         \
                case CMD_ ## cmd_name:                                 \
                    cmd_code                                            \
                    break;                                               \
            }

            #define DEF_JMP(jmp_name, jmp_num, jmp_sign)  \
            {                                              \
                case JMP_ ## jmp_name:                      \
                    Elem_t x = POP();                        \
                    Elem_t y = POP();                         \
                    if (y jmp_sign x)                          \
                    {                                           \
                        ASSIGN_AND_GO_NEXT(index, size_t);       \
                                                                  \
                        ip = buffer + index;                       \
                    }                                               \
                    else                                             \
                    {                                                 \
                        ip += sizeof(size_t);                          \
                    }                                                   \
                                                                         \
                    PUSH(y);                                              \
                    PUSH(x);                                               \
                    break;                                                  \
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
    StackDtor(&stack);

    free(ptr);
    free(buffer);

    return 0;
}