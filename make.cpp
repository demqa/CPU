#include <stdlib.h>

int main()
{
    system("./asm code/code.asm code/code.binary");
    system("./cpu code/code.binary");
    return 0;
}