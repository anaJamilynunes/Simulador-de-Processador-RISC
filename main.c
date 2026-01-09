#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include "cpu.h"

int main(){
    CPU cpu;
    cpu_init(&cpu);

    // Carregar um programa de exemplo na memoria (opcodes ficticios)
    cpu.memory[0x0000] = 0x01;
    cpu.memory[0x0001] = 0x02;
    cpu.memory[0x0002] = 0x00;

    cpu_run(&cpu);

    printf("Programa finalizado. PC: %04X\n", cpu.PC);
    return 0;
}