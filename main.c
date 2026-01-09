#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include "cpu.h"

int main(int argc, char* argv[]) {
    CPU cpu;
    cpu_init(&cpu);
    
    if(argc < 2) {
        printf("Uso: %s <arquivo_binario>\n", argv[0]);
        return 1;
    }

    //teste EUDES
    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    
    // Carregar um programa de exemplo na memoria (opcodes ficticios)
    cpu.memory[0x0000] = 0x01;
    cpu.memory[0x0001] = 0x02;
    cpu.memory[0x0002] = 0x00;
    
    // Ler os breakpoints dos argumentos da linha de comando
    qtd_breakpoints = 0;
    for (int i = 2; i < argc && qtd_breakpoints < MAX_BREAKPOINTS; i++) {
        breakpoints[qtd_breakpoints] = (uint16_t) strtol(argv[i], NULL, 16);
        qtd_breakpoints++;
    }

    cpu_run(&cpu);

    printf("Programa finalizado. PC: %04X\n", cpu.registers[15]);
    return 0;
}