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

RegNum RegNumber(char c)
{
    switch (c)
    {
        case 'a':
            return 1;
            break;
        case 'b':
            return 2;
            break;
        case 'c':
            return 3;
            break;
        case 'd':
            return 4;
            break;
        
        default:
            perror("You shouldn't be there");
            break;
    }
}

#define ADD_CMD_FLAGS(flags)                        \
{                                                    \
    *(Command *)(ptr - sizeof(Command)) |= (flags);   \
}
 
#define ASSIGN_CMD_ARG(arg, type)                    \
{                                                     \
    *(type *)ptr = (arg);                              \
    ptr += sizeof(type);                                \
}

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
        char command[20] = {};

        int count = sscanf(text.lines[i].ptr, "%s", command);

        if (count == 0)
        {
            COMPILE_ERROR(REWRITE_ASSEMBLER_CANT_READ_CMD);
        }


    #define GetArg()                                \
    {                                                \
        char reg_letter[20] = {};                     \
        int  first          = 0;                       \
        int  last           = 0;                        \
        int  count          = 0;                         \
        int  check          = 0;                          \
        Elem_t x            = NAN;                         \
        size_t index          = 0;                          \
                                                                   \
        if (sscanf(text.lines[i].ptr, "%s %lf", command, &x) == 2)  \
        {                                                            \
            ADD_CMD_FLAGS(IMM);                                 \
                                                                 \
            ASSIGN_CMD_ARG(x, Elem_t);                            \
        }                                                          \
        else                                                                                       \
        if (sscanf(text.lines[i].ptr, "%s %n%1[abcd]x%n", command, &first, reg_letter, &last) == 2  \
            && last - first == 2)                                                                    \
        {                                                              \
            ADD_CMD_FLAGS(REG);                                         \
                                                                         \
            ASSIGN_CMD_ARG(RegNumber(*reg_letter), RegNum);               \
        }                                                                  \
        else                                                                    \
        if (sscanf(text.lines[i].ptr, "%s [%lu]%n", command, &index, &check) == 2  \
            && check != 0)                                                        \
        {                                                                      \
            ADD_CMD_FLAGS(OSU);                                                 \
                                                                                 \
            ASSIGN_CMD_ARG(index, size_t);                                          \
        }                                                                          \
        else                                                                                         \
        if (sscanf(text.lines[i].ptr, "%s %n[%1[abcd]x]%n", command, &first, reg_letter, &last) == 2  \
            && last - first == 4)                                                                      \
        {                                                                              \
            ADD_CMD_FLAGS(REG | OSU);                                                   \
                                                                                         \
            ASSIGN_CMD_ARG(RegNumber(*reg_letter), RegNum);                               \
        }                                                                                  \
        else                                                                                                             \
        if (sscanf(text.lines[i].ptr, "%s %n[%1[abcd]x+%n%lu]%n", command, &first, reg_letter, &last, &index, &check) == 3  \
            && last - first == 4 && check != 0)                                                                            \
        {                                                                                      \
            ADD_CMD_FLAGS(IMM | OSU | REG);                                                     \
                                                                                                 \
            ASSIGN_CMD_ARG(RegNumber(*reg_letter), RegNum);                                       \
            ASSIGN_CMD_ARG(index, size_t);                                                           \
        }                                                                                           \
        else                                                                                         \
        {                                                                                             \
            COMPILE_ERROR(UNRECOGNISABLE_CMD_FORMAT_ASM);                                              \
        }                                                                                               \
    }
     
    #define DEF_CMD(cmd_name, cmd_num, cmd_n_args, cmd_code)                            \
        if (strcmp(command, #cmd_name) == 0)                                             \
        {                                                                                 \
            *(Command *)ptr = cmd_num;                                                     \
            ptr += sizeof(Command);                                                         \
                                                                                             \
            if (cmd_n_args == 1)                                                              \
            {                                                                                  \
                GetArg();                                                                       \
            }                                                                                    \
        }                                                                                         \
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


#undef ADD_CMD_FLAGS
#undef ASSIGN_CMD_ARG