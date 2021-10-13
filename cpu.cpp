#include "string_lib/string_lib.h"
#include "stack/stack.h"
#include "stack/print_func.cpp"

#include "cpu_config.h"

int main(int argc, char *argv[]){

    Text text = {};
    int error_code = 0;

    if (argc == 2)
    {
        error_code = ConstructText(argv[1], &text);
        CheckError(&error_code);
    }
    else
    {
        printf("give me nice arguments, pls\n");
        return 0;
    }

    stack_t stack = {};

    StackCtor(&stack, 0, PrintDouble);

    for (int i = 0; i < text.nlines; ++i){
        char command[20];
        double num = NAN;

        int count = sscanf(text.lines[i].ptr, "%s %lf", &command, &num);

        if (count == 1)
        {
            if (strcmp(command, "pop") == 0)
            {
                StackPop(&stack);
            }
            else
            if (strcmp(command, "add") == 0)
            {
                double x = 0;
                x += StackPop(&stack);
                x += StackPop(&stack);

                if (isnan(x))
                {
                    COMPILE_ERROR(REWRITE_ASSEMBLER_STACK_POPPING_WHEN_EMPTY);
                }
                
                StackPush(&stack, x);
            }
            else
            if (strcmp(command, "sub") == 0)
            {
                double x = 0;
                x -= StackPop(&stack);
                x += StackPop(&stack);

                if (isnan(x))
                {
                    COMPILE_ERROR(REWRITE_ASSEMBLER_STACK_POPPING_WHEN_EMPTY);
                }
                
                StackPush(&stack, x);
            }
            else
            if (strcmp(command, "mul") == 0)
            {
                double x = 1;
                x *= StackPop(&stack);
                x *= StackPop(&stack);

                if (isnan(x))
                {
                    COMPILE_ERROR(REWRITE_ASSEMBLER_STACK_POPPING_WHEN_EMPTY);
                }

                StackPush(&stack, x);
            }
            else
            if (strcmp(command, "div") == 0)
            {
                double x = 1;
                x /= StackPop(&stack);
                x *= StackPop(&stack);

                if (isnan(x))
                {
                    COMPILE_ERROR(REWRITE_ASSEMBLER_STACK_POPPING_WHEN_EMPTY);
                }

                StackPush(&stack, x);
                
            }
            else
            if (strcmp(command, "out") == 0)
            {
                double x = StackTop(&stack);

                if (isnan(x))
                {
                    COMPILE_ERROR(REWRITE_ASSEMBLER_STACK_TOP_WHEN_EMPTY);
                }

                printf("top = %lf\n", x);
                
            }
            else
            if (strcmp(command, "hlt") == 0)
            {
                DestructText(&text);
                StackDtor(&stack);
                printf("end\n");
                return 0;
            }
            else
            {
                COMPILE_ERROR(REWRITE_ASSEMBLER_UNKNOWN_COMMAND);
            }

        }
        else
        if (count == 2)
        {
            if (strcmp(command, "push") == 0)
            {   
                if (isnan(num))
                {
                    COMPILE_ERROR(REWRITE_ASSEMBLER_PUSH_NUM_IS_NAN);
                }

                StackPush(&stack, num);
            }
            else
            {
                printf("i = %d\n", i);
                COMPILE_ERROR(REWRITE_ASSEMBLER_UNKNOWN_COMMAND);
            }
        }
        else
        {
            COMPILE_ERROR(REWRITE_ASSEMBLER_EMPTY_LINE);
        }
    }

    COMPILE_ERROR(REWRITE_ASSEMBLER_THERE_IS_NO_HLT);

    return 0;
}