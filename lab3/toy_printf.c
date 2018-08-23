/* toy-io.c
 * Limited versions of printf and scanf
 *
 * Programmer: Mayer Goldberg, 2018
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* the states in the printf state-machine */
enum printf_state {
    st_printf_init,
    st_printf_percent,
    st_printf_array,
    st_struct_size
};

#define MAX_NUMBER_LENGTH 64
#define is_octal_char(ch) ('0' <= (ch) && (ch) <= '7')

typedef struct state_args {
    char* fs;
    int int_value;

    char* arr;
    char** arrOfStrings;
    int arrSize;

    int length;
    char pad_char;
    int pad_left;
    int pad_right;
    int padAmount;

    char *string_value;
    char char_value;
    enum printf_state state;
    va_list args;
    int chars_printed;
} state_args;

typedef struct state_result {
    int printed_chars;
    enum printf_state new_state;
}state_result;


struct state_result(*handlersArray[128])(va_list args, state_args* state,struct state_result toReturn);
struct state_result etcHandler(va_list args, state_args* state, struct state_result toReturn);
struct state_result uHandler(va_list args, state_args* state, struct state_result toReturn);
struct state_result cHandler(va_list args, state_args* state, struct state_result toReturn);
struct state_result sHandler(va_list args, state_args* state, struct state_result toReturn);
struct state_result xHandler(va_list args, state_args* state, struct state_result toReturn);
struct state_result XHandler(va_list args, state_args* state, struct state_result toReturn);
struct state_result oHandler(va_list args, state_args* state, struct state_result toReturn);
struct state_result bHandler(va_list args, state_args* state, struct state_result toReturn);
struct state_result dHandler(va_list args, state_args* state, struct state_result toReturn);
struct state_result AHandler(va_list args, state_args* state, struct state_result toReturn);
struct state_result defaultHandler(va_list args, state_args* state, struct state_result toReturn);
struct state_result percentHandler(va_list args, state_args* state, struct state_result toReturn);

struct state_result init_state_handler(va_list args, state_args* state);
struct state_result percent_state_handler(va_list args, state_args* state);
struct state_result array_state_handler(va_list args, state_args* state);

int toy_printf(char *fs, ...);

const char *digit = "0123456789abcdef";
const char *DIGIT = "0123456789ABCDEF";


int print_int_helper(unsigned int n, int radix, const char *digit) {
    int result;

    if (n < radix) {
        putchar(digit[n]);
        return 1;
    }
    else {
        result = print_int_helper(n / radix, radix, digit);
        putchar(digit[n % radix]);
        return 1 + result;
    }
}

int print_int(unsigned int n, int radix, const char * digit) {

    if (radix < 2 || radix > 16) {
        toy_printf("Radix must be in [2..16]: Not %d\n", radix);
        exit(-1);
    }

    if (n > 0) {
        return print_int_helper(n, radix, digit);
    }
    if (n == 0) {
        putchar('0');
        return 1;
    }
    else {
        putchar('-');
        return 1 + print_int_helper(-n, radix, digit);
    }
}

int print_signed_int_helper(int n, int radix, const char *digit) {
    int result;

    if (n < radix) {
        putchar(digit[n]);
        return 1;
    }
    else {
        result = print_int_helper(n / radix, radix, digit);
        putchar(digit[n % radix]);
        return 1 + result;
    }
}

int print_signed_int(int n, int radix, const char * digit) {

    if (radix < 2 || radix > 16) {
        toy_printf("Radix must be in [2..16]: Not %d\n", radix);
        exit(-1);
    }

    if (n > 0) {
        return print_int_helper(n, radix, digit);
    }
    if (n == 0) {
        putchar('0');
        return 1;
    }
    else {
        putchar('-');
        return 1 + print_int_helper(-n, radix, digit);
    }
}

int array_helper(char* arr,int arrSize, int base){

    int toReturn;
    for (int i=0; i<arrSize*sizeof(int); i++){

        if (base == 162)
            toReturn += print_int(arr[i],16,DIGIT);
        else if (base == 102)
            toReturn += print_signed_int(arr[i],10,digit);
        else
            toReturn += print_int(arr[i],base,digit);

        i+=3;
        if(i+1<arrSize*4){
            putchar(',');
            putchar(' ');
            toReturn += 2;
        }
    }
    putchar('}');
    toReturn += 1;

    return toReturn;
}

int get_length (int value){
    int l=!value;
    while(value){
        l++;
        value/=10;
    }
    return l;
}

struct state_result init_state_handler(va_list args, state_args* state){
    struct state_result toReturn;
    toReturn.printed_chars=0;
    switch (*state->fs) {
        case '%':
        	toReturn.new_state=st_printf_percent;
            return toReturn;

        default:
            putchar(*state->fs);
            toReturn.printed_chars++;
            toReturn.new_state=st_printf_init;
            return toReturn;
    }
}

struct state_result percent_state_handler(va_list args, state_args* state){
    struct state_result toReturn;
    toReturn.printed_chars=0;
    return handlersArray[(int)*state->fs](args,state,toReturn);
}

struct state_result array_state_handler(va_list args, state_args* state) {
    struct state_result toReturn;
    toReturn.printed_chars=0;
    return handlersArray[(int)*state->fs](args,state,toReturn);
}

/******* task2c handlers ********/

struct state_result etcHandler(va_list args, state_args* state, struct state_result toReturn){
    if(*state->fs == '0'){
        state->pad_char = '0';
        state->fs++;
    }
    else state->pad_char = ' ';

    if(*state->fs == '-'){
        state->pad_left=1;
        state->fs++;
    }
    else state->pad_right=1;

    while(*state->fs >= '0' && *state->fs <= '9'){
        state->padAmount = state->padAmount*10 +(*state->fs -'0');
        state->fs++;
    }
    if(*state->fs=='d' || *state->fs=='s')
        state->fs--;
    toReturn.new_state = st_printf_percent;
    return toReturn;
}

struct state_result uHandler(va_list args, state_args* state,struct state_result toReturn) {
    if (state->state == st_printf_percent){
        state->int_value = va_arg(args, int);
        toReturn.printed_chars += print_int(state->int_value,10,digit);
        toReturn.new_state = st_printf_init;
    }
    else if (state->state == st_printf_array){
        state->arr = va_arg(args, char*);
        state->arrSize = va_arg(args, int);
        toReturn.printed_chars += array_helper(state->arr, state->arrSize, 10);
        toReturn.new_state = st_printf_init;
    }
    return toReturn;
}

struct state_result cHandler(va_list args, state_args* state,struct state_result toReturn) {
    if (state->state == st_printf_percent){
        state->char_value = (char)va_arg(args, int);
        putchar(state->char_value);
        toReturn.printed_chars++;
        toReturn.new_state = st_printf_init;
    }
    else if (state->state == st_printf_array){
        state->arrOfStrings = (char **) va_arg(args, char*);
        state->arrSize = va_arg(args, int);

        for (int i = 0; i < state->arrSize; i++) {
            state->string_value = state->arrOfStrings[i];
            while (*state->string_value) {
                toReturn.printed_chars++;
                putchar(*state->string_value);
                state->string_value++;
            }
            if (i + 1 < state->arrSize) {
                putchar(',');
                putchar(' ');
                toReturn.printed_chars += 2;
            }
        }
        putchar('}');
        toReturn.printed_chars++;
        toReturn.new_state = st_printf_init;
    }
    return toReturn;
}

struct state_result sHandler(va_list args, state_args* state,struct state_result toReturn) {
    if (state->state == st_printf_percent){
        state->string_value = va_arg(args, char *);
        state->length = strlen(state->string_value);
        state->padAmount=state->padAmount-state->length;

        if (state->pad_left) {
            while(state->padAmount>0){
                putchar(state->pad_char);
                state->padAmount--;
                toReturn.printed_chars++;
            }
        }

        while(*state->string_value){
            toReturn.printed_chars++;
            putchar(*state->string_value);
            state->string_value++;
        }

        if (state->pad_right){
            while(state->padAmount>0){
                putchar(state->pad_char);
                state->padAmount--;
                toReturn.printed_chars++;
            }
            putchar('#');
            toReturn.printed_chars++;
        }

        toReturn.new_state = st_printf_init;
    }
    else if (state->state == st_printf_array){
        state->arrOfStrings = (char **) va_arg(args, char*);
        state->arrSize = va_arg(args, int);

        for (int i = 0; i < state->arrSize; i++) {
            state->string_value = state->arrOfStrings[i];
            while (*state->string_value) {
                toReturn.printed_chars++;
                putchar(*state->string_value);
                state->string_value++;
            }
            if (i + 1 < state->arrSize) {
                putchar(',');
                putchar(' ');
                toReturn.printed_chars += 2;
            }
        }
        putchar('}');
        toReturn.printed_chars++;
        toReturn.new_state = st_printf_init;
    }
    return toReturn;
}

struct state_result XHandler(va_list args, state_args* state,struct state_result toReturn) {
    if (state->state == st_printf_percent){
        state->int_value = va_arg(args, int);
        toReturn.printed_chars += print_int(state->int_value, 16, DIGIT);
        toReturn.new_state = st_printf_init;
    }
    else if (state->state ==st_printf_array){
        state->arr = va_arg(args, char*);
        state->arrSize = va_arg(args, int);
        toReturn.printed_chars += array_helper(state->arr, state->arrSize, 162);
        toReturn.new_state = st_printf_init;
    }
    return toReturn;
}

struct state_result xHandler(va_list args, state_args* state,struct state_result toReturn) {
    if(state->state == st_printf_percent){
        state->int_value = va_arg(args, int);
        toReturn.printed_chars += print_int(state->int_value, 16, digit);
        toReturn.new_state = st_printf_init;
    }
    else if (state->state ==st_printf_array){
        state->arr = va_arg(args, char*);
        state->arrSize = va_arg(args, int);
        toReturn.printed_chars += array_helper(state->arr, state->arrSize, 16);
        toReturn.new_state = st_printf_init;
    }
    return toReturn;
}

struct state_result oHandler(va_list args, state_args* state,struct state_result toReturn) {
    if (state->state == st_printf_percent){
        state->int_value = va_arg(args, int);
        toReturn.printed_chars += print_int(state->int_value, 8, digit);
        toReturn.new_state = st_printf_init;
    }
    else if (state->state == st_printf_array){
        state->arr = va_arg(args, char*);
        state->arrSize = va_arg(args, int);
        toReturn.printed_chars += array_helper(state->arr, state->arrSize, 8);
        toReturn.new_state = st_printf_init;
    }
    return toReturn;
}

struct state_result bHandler(va_list args, state_args* state,struct state_result toReturn) {
    if(state->state == st_printf_percent){
        state->int_value = va_arg(args, int);
        toReturn.printed_chars += print_int(state->int_value, 2, digit);
        toReturn.new_state = st_printf_init;
    }
    else if (state->state == st_printf_array){
        state->arr = va_arg(args, char*);
        state->arrSize = va_arg(args, int);
        toReturn.printed_chars += array_helper(state->arr, state->arrSize, 2);
        toReturn.new_state = st_printf_init;
    }
    return toReturn;
}

struct state_result dHandler(va_list args, state_args* state,struct state_result toReturn) {

    if(state->state == st_printf_percent){
        state->int_value = va_arg(args, int);

        state->length = get_length(state->int_value);
        state->padAmount = state->padAmount-state->length;

        if(state->int_value<0){
            putchar('-');
            toReturn.printed_chars++;
            state->int_value = (state->int_value)*(-1);
            state->padAmount--;
        }

        if(state->pad_left){
            while(state->padAmount>0){
                putchar(state->pad_char);
                state->padAmount--;
                toReturn.printed_chars++;
            }
        }
        if(state->pad_char=='0'&& state->pad_right){
            while(state->padAmount>0){
                putchar(state->pad_char);
                state->padAmount--;
                toReturn.printed_chars++;
            }
        }

        toReturn.printed_chars+= print_signed_int(state->int_value,10,digit);

        if(state->pad_char!='0' && state->pad_right){
            while(state->padAmount>0){
                putchar(state->pad_char);
                state->padAmount--;
                toReturn.printed_chars++;
            }
            putchar('#');
            toReturn.printed_chars++;

        }

        toReturn.new_state = st_printf_init;
    }
    
    else if (state->state == st_printf_array){
        state->arr = va_arg(args, char*);
        state->arrSize = va_arg(args, int);
        toReturn.printed_chars += array_helper(state->arr, state->arrSize, 102);
        toReturn.new_state = st_printf_init;
    }
    return toReturn;
}

struct state_result AHandler(va_list args, state_args* state,struct state_result toReturn) {
    putchar('{');
    toReturn.printed_chars++;
    toReturn.new_state = st_printf_array;
    return toReturn;
}

struct state_result percentHandler(va_list args, state_args* state,struct state_result toReturn) {
    if (state->state == st_printf_init){
        toReturn.new_state=st_printf_percent;
    }
    else{
        putchar('%');
        toReturn.printed_chars++;
        toReturn.new_state = st_printf_init;
    }
    return toReturn;
}

struct state_result defaultHandler(va_list args, state_args* state,struct state_result toReturn) {
    putchar(*state->fs);
    toReturn.printed_chars++;
    toReturn.new_state=st_printf_init;
    return toReturn;
}

/********* task2c handlers ***********/

/* SUPPORTED:
 *   %b, %d, %o, %x, %X --
 *     integers in binary, decimal, octal, hex, and HEX
 *   %s -- strings
 */

int toy_printf(char *fs, ...) {
    struct state_args myStruct;
    struct state_result res;
    myStruct.state = st_printf_init;
    myStruct.fs=fs;
    va_start(myStruct.args, myStruct.fs);

    for (int i=0;i<128;i++)
        handlersArray[i] = &defaultHandler;

    handlersArray['%'] = &percentHandler;
    handlersArray['A'] = &AHandler;
    handlersArray['d'] = &dHandler;
    handlersArray['b'] = &bHandler;
    handlersArray['o'] = &oHandler;
    handlersArray['x'] = &xHandler;
    handlersArray['X'] = &XHandler;
    handlersArray['s'] = &sHandler;
    handlersArray['c'] = &cHandler;
    handlersArray['u'] = &uHandler;
    
    handlersArray['0'] = &etcHandler;
    handlersArray['1'] = &etcHandler;
    handlersArray['2'] = &etcHandler;
    handlersArray['3'] = &etcHandler;
    handlersArray['4'] = &etcHandler;
    handlersArray['5'] = &etcHandler;
    handlersArray['6'] = &etcHandler;
    handlersArray['7'] = &etcHandler;
    handlersArray['8'] = &etcHandler;
    handlersArray['9'] = &etcHandler;
    handlersArray['-'] = &etcHandler;


    struct state_result(*stateHandlersArray[st_struct_size])(va_list args, state_args* state);

    stateHandlersArray[st_printf_init] = &init_state_handler;
    stateHandlersArray[st_printf_percent] = &percent_state_handler;
    stateHandlersArray[st_printf_array] = &array_state_handler;

    for (; *myStruct.fs != '\0'; ++myStruct.fs) {
        res = stateHandlersArray[myStruct.state](myStruct.args,&myStruct);
        myStruct.state = res.new_state;
        myStruct.chars_printed += res.printed_chars;
    }

    va_end(myStruct.args);
    return myStruct.chars_printed;
}