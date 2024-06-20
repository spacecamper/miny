#include "stdio.h"

int main(int argc, char** argv) {
    if (argc != 2) return 1;
    printf("char %s[] = {", argv[1]);
    int i;
    while ((i = getchar()) != EOF) printf("%d,", i);
    puts("0};");
    return 0;
}
