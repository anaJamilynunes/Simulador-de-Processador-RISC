#ifndef CPU_H
#define CPU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

#define MEMORY_SIZE 8192 
#define MAX_BREAKPOINTS 100

extern uint16_t breakpoints[MAX_BREAKPOINTS];
extern int qtd_breakpoints;

typedef struct cpu {
    uint16_t memory[MEMORY_SIZE];
    uint16_t registers[16]; // R0–R15
    uint16_t IR;

    bool flagZ;
    bool flagC;
    bool mem_acessed[MEMORY_SIZE];

    bool hatl;
} CPU;

void cpu_init(CPU* cpu);

void cpu_instruction_execute(CPU* cpu);

void cpu_run(CPU* cpu);

bool breakpoint(uint16_t pc);

void print_cpu_state(CPU* cpu);

#endif