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
    fprintf(stream, "\n");
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

    FILE *stream = fopen(argv[2], "wb");

    if (stream == nullptr)
    {
        printf("OOPS, FATAL ERROR\n");
        return -1;
    }

    char *binary_code = (char *) calloc(text.nlines * (sizeof(Elem_t) + sizeof(Command)) + sizeof(Header) + 1, sizeof(char));

    char *ptr = binary_code;

    ptr += sizeof(Header);

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

            // fprintf(stream, "%d\n", code);

            // fprint(listing, format, something);

            ptr += sizeof(Command);
        }
        else
        if (count == 2)
        {
            // fprintf(stream, "%d %lf\n", code, num);
            *((Command *)ptr) = code;
            // fprint(listing, format, something);
            ptr += sizeof(Command);

            *((Elem_t *)ptr) = num;
            // fprint(listing, format, something);
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

    size_t buffsize = ptr - binary_code - sizeof(Header);
    // printf("buffsize = %d\n", buffsize);

    Header header = {};

    header.signature    = signature;
    header.version      = version;
    header.buffsize     = buffsize;

    *((Header *)binary_code) = header;

    size_t nbytes = fwrite(binary_code, sizeof(char), sizeof(Header) + buffsize, stream);
    if (nbytes != sizeof(Header) + buffsize)
    {
        COMPILE_ERROR(CANT_WRITE_IN_BINARY);
    }

    // for (size_t i = 0; i < sizeof(Header) + buffsize; ++i){
    //     printf("%02x ", *((u_int8_t *)binary_code + i));
    // }
    // printf("\n");

    // PrintHex(binary_code, sizeof(Header) + buffsize, stdout);

    free(binary_code);

    if (error_code != -1)
    {
        COMPILE_ERROR(REWRITE_ASSEMBLER_THERE_IS_NO_HLT);
    }

    fclose(stream);

    return 0;
}

