#include "cpu.h"

#include <stdio.h>

int main () {
    printf("%04x\n", cpu.pc++);
    printf("%04x\n", cpu.pc++);
    printf("%04x\n", cpu.pc++);

    return 0;
}
