/* toy-io.c
 * Limited versions of printf and scanf
 *
 * Programmer: Mayer Goldberg, 2018
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* the states in the printf state-machine */
enum printf_state {
  st_printf_init,
  st_printf_percent,
  st_printf_octal2,
  st_printf_octal3,
  st_printf_array
};

#define MAX_NUMBER_LENGTH 64
#define is_octal_char(ch) ('0' <= (ch) && (ch) <= '7')

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

int array_helper(char* arr,int arrSize, int base){

  int toReturn;
  for (int i=0; i<arrSize*sizeof(int); i++){

    if (base == 162)
      toReturn += print_int(arr[i],16,DIGIT);
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

/* SUPPORTED:
 *   %b, %d, %o, %x, %X -- 
 *     integers in binary, decimal, octal, hex, and HEX
 *   %s -- strings
 */

int toy_printf(char *fs, ...) {
  int chars_printed = 0;
  int int_value = 0;

  char* arr;
  char** arrOfStrings;
  int arrSize;

  char *string_value;
  char char_value;
  char octal_char;
  va_list args;
  enum printf_state state;

  va_start(args, fs);

  state = st_printf_init; 

  for (; *fs != '\0'; ++fs) {
    switch (state) {
    case st_printf_init:
      switch (*fs) {
      case '%':
	state = st_printf_percent;
	break;

      default:
	putchar(*fs);
	++chars_printed;
      }
      break;

 

    case st_printf_array:
      
      switch (*fs) {

            case 'd':
      arr = va_arg(args, char*);
      arrSize = va_arg(args,int);
      chars_printed += array_helper(arr,arrSize,10);
      state = st_printf_init;
      break;


            case 'b':
      arr = va_arg(args, char*);
      arrSize = va_arg(args,int);
      chars_printed += array_helper(arr,arrSize,2);
      state = st_printf_init;
      break;

            case 'o':
      arr = va_arg(args, char*);
      arrSize = va_arg(args,int);
      chars_printed += array_helper(arr,arrSize,8);
      state = st_printf_init;
      break;
  
            case 'x':
      arr = va_arg(args, char*);
      arrSize = va_arg(args,int);
      chars_printed += array_helper(arr,arrSize,16);
      state = st_printf_init;
      break;

            case 'X':
        arr = va_arg(args, char*);
      arrSize = va_arg(args,int);
      chars_printed += array_helper(arr,arrSize,162);
      state = st_printf_init;
      break;

            case 'u':
      arr = va_arg(args, char*);
      arrSize = va_arg(args,int);
      chars_printed += array_helper(arr,arrSize,10);
      state = st_printf_init;
      break;

            case 's':
      arrOfStrings =(char**) va_arg(args,char*);
      arrSize = va_arg(args,int);

      for(int i=0;i<arrSize;i++){
        string_value = arrOfStrings[i];
        while(*string_value){
          chars_printed++;
          putchar(*string_value);
          string_value++;
        }
        if(i+1<arrSize){
          putchar(',');
          putchar(' ');
          chars_printed += 2;
        }
      }
      putchar('}');
      chars_printed++;
      state = st_printf_init;
      break;

          case 'c':
      arrOfStrings = (char**) va_arg(args,char*);
      arrSize = va_arg(args,int);
      for(int i=0;i<arrSize;i++){
        char_value = arrOfStrings[i];
        putchar(char_value);
        ++chars_printed;

        if(i+1<arrSize){
          putchar(',');
          putchar(' ');
          chars_printed += 2;
        }
      }
      putchar('}');
      chars_printed++;
      state = st_printf_init;
      break;

        default:
    toy_printf("Unhandled format %%%c...\n", *fs);
    exit(-1);
        }
        break;
      

    case st_printf_percent:
      switch (*fs) {
      case '%':
	putchar('%');
	++chars_printed;
	state = st_printf_init;
	break;

      case 'A':
  putchar('{');
  ++chars_printed;
  state = st_printf_array;
  break;

      case 'd':
	int_value = va_arg(args, int);
	chars_printed += print_int(int_value, 10, digit);
	state = st_printf_init;
	break;

      case 'b':
	int_value = va_arg(args, int);
	chars_printed += print_int(int_value, 2, digit);
	state = st_printf_init;
	break;

      case 'o':
	int_value = va_arg(args, int);
	chars_printed += print_int(int_value, 8, digit);
	state = st_printf_init;
	break;
	
      case 'x':
	int_value = va_arg(args, int);
	chars_printed += print_int(int_value, 16, digit);
	state = st_printf_init;
	break;

      case 'X':
	int_value = va_arg(args, int);
	chars_printed += print_int(int_value, 16, DIGIT);
	state = st_printf_init;
	break;

      case 's':
	string_value = va_arg(args, char *);
	while(*string_value){
		chars_printed++;
		putchar(*string_value);
		string_value++;
	}
	state = st_printf_init;
	break;

      case 'c':
	char_value = (char)va_arg(args, int);
	putchar(char_value);
	++chars_printed;
	state = st_printf_init;
	break;

      case 'u':
	int_value = va_arg(args, int);
	chars_printed += print_int(int_value,10,digit);
	state = st_printf_init;
	break;

      default:
	toy_printf("Unhandled format %%%c...\n", *fs);
	exit(-1);
      }
      break;

    default:
      toy_printf("toy_printf: Unknown state -- %d\n", (int)state);
      exit(-1);
    }
  }

  va_end(args);

  return chars_printed;
}


