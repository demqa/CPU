#ifndef STACK_CONFIG_H

#define STACK_CONFIG_H

#define DEBUG_MODE   01

#include <stdio.h>
#include <math.h> // for NAN

typedef double Elem_t;

const Elem_t POISONED_ELEM = NAN;

void PrintDouble(void *memory, size_t size, FILE *stream);

#endif