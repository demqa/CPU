#ifndef CPU_CONFIG_H

#define CPU_CONFIG_H

#define COMPILE_ERROR(msg){                                             \
    printf("[%s:%d] %s\n", __PRETTY_FUNCTION__, __LINE__, #msg);         \
    DestructText(&text);                                                  \
    StackDtor(&stack);                                                     \
    return 0;                                                               \
}

#endif