#include "string_lib/string_lib.h"
#include "cpu_config.h"
#include <math.h>

#define COMPILE_ERROR(msg)                                      \
{                                                                \
    printf("[%s:%d] %s\n", __PRETTY_FUNCTION__, __LINE__, #msg);  \
    DestructText(&text);                                           \
    return 0;                                                       \
}

void PrintHex(void *ptr, size_t size, FILE *stream)
{
    if (ptr == nullptr || stream == nullptr)
    {
        perror("PrintHex ptr is null or stream is null");
        return;
    }

    char *mem = (char *) ptr;
    for (char *c = mem + size - 1; c >= mem; c--)
        fprintf(stream, "%02x ", *((u_int8_t *)c));
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

#define ADD_CMD_FLAGS(flags)                      \
{                                                  \
    *(Command *)(ip - sizeof(Command)) |= (flags);  \
}
 
#define ASSIGN_CMD_ARG(arg, type)                      \
{                                                       \
    *(type *)ip = (arg);                                 \
    ip += sizeof(type);                                   \
}

struct Label
{
    size_t index;
    char name[20];
};

const size_t labels_capacity = 10;

size_t FindLabel(char *label, Label *labels)
{
    for (size_t i = 0; i < labels_capacity; ++i)
    {
        if (strcmp(label, labels[i].name) == 0)
        {
            return i;
        }
    }
    return labels_capacity;
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

    char *header_ptr = (char *) calloc(text.nlines * (sizeof(Elem_t) + sizeof(Command) + sizeof(u_int64_t)) + sizeof(Header) + 1, sizeof(char));
    
    size_t label_n = 0;
    Label *labels = (Label *) calloc(labels_capacity, sizeof(Label));
    for (size_t j_ = 0; j_ < labels_capacity; ++j_) labels[j_].index = (size_t)(-1);

    char *binary = header_ptr;
    binary += sizeof(Header);

    char *ip = nullptr;

for (int i_ = 0; i_ < 2; ++i_)
{   
    ip = binary;
    label_n = 0;

    for (int i = 0; i < text.nlines; ++i)
    {
        char command[20] = {};

        int count = sscanf(text.lines[i].ptr, "%s", command);

        if (count == 0)
        {
            COMPILE_ERROR(REWRITE_ASSEMBLER_CANT_READ_CMD);
        }

        int check = 0;

        char label[20] = {};
        count = sscanf(text.lines[i].ptr, "%[A-Za-z0-9_]:%n", label, &check);

        if (count == 1 && check != 0)
        {
            if (label_n >= labels_capacity)
            {
                COMPILE_ERROR(TOO_MUCH_LABELS);
            }

            size_t index = ip - binary + sizeof(Command);
            
            for (size_t j = 0; j < label_n; ++j)
            {
                if (strcmp(labels[j].name, label) == 0 &&
                    labels[j].index != index           &&
                    labels[j].index != (size_t)(-1))
                {
                    COMPILE_ERROR(REPEATING_LABEL);
                }
            }

            Label lbl = {};

            lbl.index = index;
            memcpy(lbl.name, label, 20);

            size_t label_number = FindLabel(label, labels);
            if (label_number == labels_capacity)
                labels[label_n++]    = lbl;
            else
                labels[label_number] = lbl;

            continue;
        }

        #define DEF_JMP(jmp_name, jmp_num, jmp_sign)                              \
            if (strcmp(command, #jmp_name) == 0)                                   \
            {                                                                       \
                ASSIGN_CMD_ARG(jmp_num, Command);                                    \
                                                                                                  \
                if (sscanf(text.lines[i].ptr, "%s %[A-Za-z0-9_]:%n", command, label, &check) == 2  \
                    && check != 0)                                                                  \
                {                                                                        \
                    size_t label_number = FindLabel(label, labels);                       \
                    if (label_number == labels_capacity)                                   \
                    {                                                                       \
                        if (label_n == labels_capacity) COMPILE_ERROR(TOO_MUCH_LABELS);      \
                        memcpy(labels[label_n++].name, label, 20);                            \
                    }                                                                          \
                    else                                                                        \
                    {                                                                            \
                        ASSIGN_CMD_ARG(labels[label_number].index, size_t);                       \
                        PRINT(labels[label_number]);          \
                    }                                                                              \
                }                                                                                   \
                else                                                                                 \
                if (sscanf(text.lines[i].ptr, "%s %lu%n", command, &index_n, &check) == 2             \
                    && check != 0)                                                                     \
                {                                                                                       \
                    ASSIGN_CMD_ARG(index_n, size_t);                                                     \
                }                                                                                         \
            }                                                                                              \
            else                                              

        #define GetArg()                     \
        {                                     \
            char reg_letter[20] = {};          \
            int  first          = 0;            \
            int  last           = 0;             \
            int  count          = 0;              \
            int  check          = 0;               \
            Elem_t x            = NAN;              \
            size_t index        = 0;                 \
                                                                       \
            if (sscanf(text.lines[i].ptr, "%s %lf", command, &x) == 2)  \
            {                                                            \
                ADD_CMD_FLAGS(IMM);                      \
                                                          \
                ASSIGN_CMD_ARG(x, Elem_t);                 \
            }                                               \
            else                                                                                       \
            if (sscanf(text.lines[i].ptr, "%s %n%1[abcd]x%n", command, &first, reg_letter, &last) == 2  \
                && last - first == 2)                                                                    \
            {                                                   \
                ADD_CMD_FLAGS(REG);                              \
                                                                  \
                ASSIGN_CMD_ARG(RegNumber(*reg_letter), RegNum);    \
            }                                                       \
            else                                                                      \
            if (sscanf(text.lines[i].ptr, "%s [%lu]%n", command, &index, &check) == 2  \
                && check != 0)                                                          \
            {                                                           \
                ADD_CMD_FLAGS(OSU);                                      \
                                                                          \
                ASSIGN_CMD_ARG(index, size_t);                             \
            }                                                               \
            else                                                                                         \
            if (sscanf(text.lines[i].ptr, "%s %n[%1[abcd]x]%n", command, &first, reg_letter, &last) == 2  \
                && last - first == 4)                                                                      \
            {                                                                   \
                ADD_CMD_FLAGS(REG | OSU);                                        \
                                                                                  \
                ASSIGN_CMD_ARG(RegNumber(*reg_letter), RegNum);                    \
            }                                                                       \
            else                                                                                                               \
            if (sscanf(text.lines[i].ptr, "%s %n[%1[abcd]x+%n%lu]%n", command, &first, reg_letter, &last, &index, &check) == 3  \
                && last - first == 4 && check != 0)                                                                              \
            {                                                                           \
                ADD_CMD_FLAGS(IMM | OSU | REG);                                          \
                                                                                          \
                ASSIGN_CMD_ARG(RegNumber(*reg_letter), RegNum);                            \
                ASSIGN_CMD_ARG(index, size_t);                                              \
            }                                                                                \
            else                                                                              \
            {                                                                                  \
                COMPILE_ERROR(UNRECOGNISABLE_CMD_FORMAT_ASM);                                   \
            }                                                                                    \
        }
        
        #define DEF_CMD(cmd_name, cmd_num, cmd_n_args, cmd_code)   \
            if (strcmp(command, #cmd_name) == 0)                    \
            {                                                        \
                ASSIGN_CMD_ARG(cmd_num, Command);                     \
                                                                       \
                if (cmd_n_args == 1)                                    \
                {                                                        \
                    GetArg();                                             \
                }                                                          \
            }                                                               \
            else

        size_t index_n = 0;

        #include "commands"
        /* else */
        {
            COMPILE_ERROR(ASM_UNKNOWN_COMMAND);
        }

        #undef DEF_CMD
        #undef DEF_JMP

        if (strcmp(command, "HLT") == 0)
        {
            error_code = -1;
        }
    }
}

    for (size_t i = 0; i < labels_capacity; ++i)
    {
        printf("labels[i] = %s\n", labels[i].name);
    }

    for (size_t i = 0; i < labels_capacity; ++i)
    {
        if (labels[i].name[0] != '\0' && labels[i].index == (size_t)(-1))
        {
            COMPILE_ERROR(ASM_LABEL_DOESNT_HAVE_INDEX);
        }
    }

    size_t buffsize    = ip - binary;

    Header header      = {};

    header.signature   = signature;
    header.version     = version;
    header.buffsize    = buffsize;

    *((Header *)header_ptr) = header;

    size_t nbytes = fwrite(header_ptr, sizeof(char), sizeof(Header) + buffsize, stream);
    if (nbytes != sizeof(Header) + buffsize)
    {
        COMPILE_ERROR(CANT_WRITE_IN_BINARY);
    }

    free(header_ptr);
    free(labels);

    if (error_code != -1)
    {
        COMPILE_ERROR(REWRITE_ASSEMBLER_THERE_IS_NO_HLT);
    }

    fclose(stream);

    return 0;
}


#undef ADD_CMD_FLAGS
#undef ASSIGN_CMD_ARG