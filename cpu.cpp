#include <string.h>
#include "cpu_config.h"
#include "stack/stack.h"
#include "stack/print_func.cpp"

#define PROCESSING_ERROR(msg){                                                                          \
    printf("instruction #%d [%s:%d] message = %s\n", instruction, __PRETTY_FUNCTION__, __LINE__, #msg);  \
    StackDtor(&stack);                                                                                    \
    return 0;                                                                                              \
}

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

        // for (size_t i = 0; i < sizeof(Header) + file_info.buffsize; ++i){
        //     printf("%02x ", *((u_int8_t *)buffer + i));
        // }
        // printf("\n");

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

    for (ip = buffer; ip < buffer + file_info.buffsize; ++instruction)
    {
        Command command = *(Command *)ip;
        ip += sizeof(Command);

        if (command == PUSH)
        {
            Elem_t num = *(Elem_t *)ip;
            ip += sizeof(Elem_t);

            StackPush(&stack, num);
        }
        else
        {
            if (command == POP)
            {
                StackPop(&stack);
            }
            else
            if (command == ADD)
            {
                double x = 0;
                x += StackPop(&stack);
                x += StackPop(&stack);

                if (isnan(x))
                {
                    PROCESSING_ERROR(WRONG_SEQ_OF_COMMANDS_STACK_POPPING_WHEN_EMPTY);
                }
                
                StackPush(&stack, x);
            }
            else
            if (command == MUL)
            {
                double x = 1;
                x *= StackPop(&stack);
                x *= StackPop(&stack);

                if (isnan(x))
                {
                    PROCESSING_ERROR(WRONG_SEQ_OF_COMMANDS_STACK_POPPING_WHEN_EMPTY);
                }

                StackPush(&stack, x);
            }
            else
            if (command == SUB)
            {
                double x = 0;
                x -= StackPop(&stack);
                x += StackPop(&stack);

                if (isnan(x))
                {
                    PROCESSING_ERROR(WRONG_SEQ_OF_COMMANDS_STACK_POPPING_WHEN_EMPTY);
                }
                
                StackPush(&stack, x);
            }
            else
            if (command == DIV)
            {
                double x = 1;
                x /= StackPop(&stack);
                x *= StackPop(&stack);

                if (isnan(x))
                {
                    PROCESSING_ERROR(WRONG_SEQ_OF_COMMANDS_STACK_POPPING_WHEN_EMPTY);
                }

                StackPush(&stack, x);
            }
            else
            if (command == OUT)
            {
                double x = StackTop(&stack);

                if (isnan(x))
                {
                    PROCESSING_ERROR(WRONG_SEQ_OF_COMMANDS_STACK_TOP_WHEN_EMPTY);
                }

                printf("top = %lf\n", x);
                
            }
            else
            if (command == HLT)
            {
                StackDtor(&stack);
                printf("end\n");
                return 0;
            }
            else
            if (command == DUMP)
            {
                // TODO
                printf("dump\n");
            }
            else
            {
                PROCESSING_ERROR(UNKNOWN_COMMAND);
            }
        }
    }

    perror("WRONG_ASSEMBLER_CODE, NO HLT");
    StackDtor(&stack);

    free(ptr);
    free(buffer);

    return 0;
}