#ifndef DEBUG_LIB_H

#define DEBUG_LIB_H

#define PRINT_LINE printf("I'm at %s at line %d\n", __PRETTY_FUNCTION__, __LINE__);

#define PRINT_PTR(ptr) printf("pointer %s at %p at line %d\n", #ptr, ptr, __LINE__);

#define PRINT(str) printf("%s\n", #str);

#endif
