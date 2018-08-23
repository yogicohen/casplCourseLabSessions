#include "toy_stdio.h"


int main(int argc, char *argv[]) {

toy_printf("Hex unsigned: %x\n", -1);
toy_printf("Octal unsigned: %o\n", -1);

toy_printf("Hex unsigned: %x\n", 10);
toy_printf("Dec unsigned: %d\n", -1);

toy_printf("Unsigned value: %u\n", 15);
toy_printf("Unsigned value: %u\n", -15);

int integers_array[] = {1,2,3,4,5};
int hex_array[] = {-1,-2,-3,-4,-5};
int octal_array[] = {-1,-2,-3,-4,-5};

char * strings_array[] = {"This", "is", "array", "of", "strings"};
int array_size = 5;
toy_printf("Print array of integers: %Ad\n", integers_array, array_size);
toy_printf("Print array of strings: %As\n", strings_array, array_size);
toy_printf("Print array of Hex unsigned: %Ax\n", hex_array, array_size);
toy_printf("Print array of Octal unsigned: %Ax\n", octal_array, array_size);
}
