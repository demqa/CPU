#include "string_lib/string_lib.h"
#include "cpu_config.h"
#include <math.h>

#define COMPILE_ERROR(msg)                                      \
{                                                                \
    printf("[%s:%d] %s\n", __PRETTY_FUNCTION__, __LINE__, #msg);  \
    DestructText(&text);                                           \
}

void PrintHex(void *ptr, size_t size, FILE *stream)
{
    char *mem = (char *) ptr;
    if (mem != nullptr){
        for (char *c = mem + size - 1; c >= mem; c--){
            fprintf(stream, "%02x ", *((u_int8_t *)c));
        }
    }
}

#define GetArgs(n_args){}

int main(int argc, char *argv[])
{
    Text text = {};
    int error_code = 0;

    if (argc == 3)
    {
        error_code = ConstructText(argv[1], &text);
        CheckError(&error_code);
    }
    else
    {
        printf("give me nice arguments, pls\n");
        return 0;
    }

    FILE *stream  = fopen(argv[2],  "wb");
    FILE *listing = fopen("listing", "w");

    if (stream == nullptr || listing == nullptr)
    {
        printf("OOPS, FATAL ERROR(CANT OPEN BINARY OR LISTING)\n");
        return -1;
    }

    char *header_ptr = (char *) calloc(text.nlines * (sizeof(Elem_t) + sizeof(Command)) + sizeof(Header) + 1, sizeof(char));

    char *ptr = header_ptr;
    ptr += sizeof(Header);

    char *binary = ptr;

    for (int i = 0; i < text.nlines; ++i)
    {
        char command[20];
        double num = NAN;

        int count = sscanf(text.lines[i].ptr, "%s", command);

        if (count == 0)
        {
            COMPILE_ERROR(REWRITE_ASSEMBLER_CANT_READ_CMD);
        }


    #define GetArg()                                              \
    {                                                              \
        double arg = NAN;                                           \
        count = sscanf(text.lines[i].ptr, "%s %lf", command, &arg);  \
                                                                      \
        if (count == 1)                                                \
        {                                                               \
            COMPILE_ERROR(THERE_IS_ONE_ARG_LOL);                         \
        }                                                                 \
                                                                           \
        *(Elem_t *)ptr = arg;                                               \
        ptr += sizeof(Elem_t);                                               \
    }

    #define DEF_CMD(cmd_name, cmd_num, cmd_n_args, cmd_code)                            \
        if (strcmp(command, #cmd_name) == 0)                                                \
        {                                                                                  \
            *(Command *)ptr = cmd_num;                                                      \
            ptr += sizeof(Command);                                                            \
                                                                                                \
            if (cmd_n_args == 1) GetArg();                                                                  \
        }                                                                                   \
        else

    // вставить в дефайн листинг потом...

    // if (cmd_n_args = 0)
    //     fprintf(listing, "%04x %4s        %02x\n", ptr - binary, cmd_name, cmd_num);

        #include "commands"
        /* else */
        {
            COMPILE_ERROR(REWRITE_ASSEMBLER_UNKNOWN_COMMAND);
        }

        #undef DEF_CMD

        if (strcmp(command, "HLT") == 0)
        {
            error_code = -1;
            if (i != text.nlines - 1)
                COMPILE_ERROR(REWRITE_ASSEMBLER_THERE_IS_SOMETHING_AFTER_HLT);
            break;
        }
    }

    size_t buffsize     = ptr - binary;

    Header header       = {};

    header.signature    = signature;
    header.version      = version;
    header.buffsize     = buffsize;

    *((Header *)header_ptr) = header;

    size_t nbytes = fwrite(header_ptr, sizeof(char), sizeof(Header) + buffsize, stream);
    if (nbytes != sizeof(Header) + buffsize)
    {
        COMPILE_ERROR(CANT_WRITE_IN_BINARY);
    }

    free(header_ptr);

    if (error_code != -1)
    {
        COMPILE_ERROR(REWRITE_ASSEMBLER_THERE_IS_NO_HLT);
    }

    fclose(stream);

    return 0;
}

