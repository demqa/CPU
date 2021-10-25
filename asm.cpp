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
        printf("OOPS, FATAL ERROR\n");
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

        int count = sscanf(text.lines[i].ptr, "%s %lf", command, &num);

        u_int32_t code = CommandCode(command);

        if (code == 65535U)
        {
            COMPILE_ERROR(REWRITE_ASSEMBLER_UNKNOWN_COMMAND);
        }

        if (count == 0)
        {
            COMPILE_ERROR(REWRITE_ASSEMBLER_CANT_READ);
        }
        else
        if (count == 1)
        {
            *((Command *)ptr) = code;

            fprintf(listing, "%04x %4s        %02x\n", ptr - binary, command, code);

            ptr += sizeof(Command);
        }
        else
        if (count == 2)
        {
            fprintf(listing, "%04x %4s %06.2lf %02x %08x\n", ptr - binary, command, num, code, num);

            *((Command *)ptr) = code;
            
            ptr += sizeof(Command);

            *((Elem_t *)ptr)  = num;

            ptr += sizeof(Elem_t);
        }
        else
        {
            COMPILE_ERROR(REWRITE_ASSEMBLER_EMPTY_LINE);
            return 0;
        }
            

        if (code == HLT)
        {
            error_code = -1;
            if (i != text.nlines - 1)
                COMPILE_ERROR(REWRITE_ASSEMBLER_THERE_IS_SOMETHING_AFTER_HLT);
            break;
        }
    }

    size_t buffsize     = ptr - binary;

    Header header = {};

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

