#ifndef CPU_H
#define CPU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

#define MEMORY_SIZE 8192 

typedef struct cpu {
    uint16_t memory[MEMORY_SIZE];
    uint16_t registers[16];
    uint16_t PC; // Faz referencia a proxima instrucao
    uint16_t SP; // Aponta para o topo da pilha
    uint16_t IR;  // Instrucao atual
       
    bool hatl;
    
    bool flagZ;
    bool flagC;
    bool mem_acessed[MEMORY_SIZE];
} CPU;

void cpu_init(CPU* cpu);

void cpu_instruction_execute(CPU* cpu);

void cpu_run(CPU* cpu);

#endif