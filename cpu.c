#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include "cpu.h"

void cpu_init(CPU* cpu){
    memset(cpu, 0, sizeof(CPU));

    cpu->PC = 0x0000;
    cpu->SP = MEMORY_SIZE;
    cpu->hatl = false;
}

void cpu_instruction_execute(CPU* cpu){
    uint16_t opcode = cpu->IR & 0x000F; // Pegar apenas o opcode (4 bits menos significativos)
    // OBS: opcode vem do IR, que ja foi carregado com a instrucao atual

    switch (opcode) {

        // Dentro deste switch, implementar as instrucoes que a CPU deve executar e foi solicitado la no documento
        // ADD, ADD
        // SUB, SUBI
        // AND, OR
        // SHL, SHR
        // CMP 
        // LDR, STR
        // JMP, JEQ, JNE, JLT, JGE
        // PUSH, POP
        // HALT
        // Qualquer coisa olha no documento, ta no ponto 3 eu acho

        case 0x0: 
            cpu->hatl = true;
            break;
        case 0x1: // ADD
            // Implementar da instrucao ADD
            break;
        case 0x2: // SUB
            // implementar da instrucao SUB
            break;
        default:
            printf("Opcode invalido: %X\n", opcode);
            cpu->hatl = true;
            break;
    }
}

void cpu_run(CPU* cpu){
    while(!cpu->hatl){
        cpu->IR = cpu->memory[cpu->PC];
        cpu->PC++;
        cpu_instruction_execute(cpu);
    }
}