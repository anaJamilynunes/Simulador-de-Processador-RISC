#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include "cpu.h"

uint16_t breakpoints[MAX_BREAKPOINTS];
int qtd_breakpoints = 0;

void cpu_init(CPU* cpu){
    memset(cpu, 0, sizeof(CPU));

    cpu->registers[15] = 0x0000; // pc
    cpu->registers[14] = MEMORY_SIZE; // SP
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
        // JMP, JEQ, JNE, JLT, JGE (FEITO POR EUDES)
        // PUSH, POP
        // HALT
        // Qualquer coisa olha no documento, ta no ponto 3 eu acho

        // Alterações Eudes (JMP, JEQ, JNE, JLT, JGE)
        case 0x0: { // JMP
            // imediato ocupa os 12 bits mais significativos
            int16_t imm = (int16_t)(cpu->IR & 0xFFF0);
            imm = imm >> 4;   // extensão de sinal

            cpu->registers[15] = cpu->registers[15] + imm;
            break;
        }
        case 0x1: { // JCOND
            uint16_t cond = (cpu->IR >> 14) & 0x3;

            int16_t imm = (int16_t)(cpu->IR & 0x0FF0);
            imm = imm >> 4; // extensão de sinal

            bool jump = false;

            switch (cond) {
                case 0x0: // JEQ
                    jump = cpu->flagZ;
                    break;
                case 0x1: // JNE
                    jump = !cpu->flagZ;
                    break;
                case 0x2: // JLT
                    jump = (!cpu->flagZ && cpu->flagC);
                    break;
                case 0x3: // JGE
                    jump = (cpu->flagZ || !cpu->flagC);
                    break;
            }

            if (jump) {
                cpu->registers[15] = cpu->registers[15] + imm;
            }
            break;
        }

        case 0xF: { // HALT
            print_cpu_state(cpu);
            cpu->hatl = true;
            break;
        }

        // case 0x0: 
        //     cpu->hatl = true;
        //     break;
        // case 0x1: // ADD
        //     // Implementar da instrucao ADD
        //     break;
        // case 0x2: // SUB
        //     // implementar da instrucao SUB
        //     break;

        default:
            printf("Opcode invalido: %X\n", opcode);
            cpu->hatl = true;
            break;
    }
}

void cpu_run(CPU* cpu){
    while(!cpu->hatl){
        uint16_t pc_anterior = cpu->registers[15];

        cpu->IR = cpu->memory[cpu->registers[15]];
        cpu->registers[15]++;

        if (breakpoint(pc_anterior)) {
            print_cpu_state(cpu);
        }

        cpu_instruction_execute(cpu);
    }
}

bool breakpoint(uint16_t pc) {
    for (int i = 0; i < qtd_breakpoints; i++) {
        if (breakpoints[i] == pc)
            return true;
    }
    return false;
}

void print_cpu_state(CPU* cpu) {
    // Registradores
    for (int i = 0; i < 16; i++) {
        printf("R%d = 0x%04hX\n", i, cpu->registers[i]);
    }

    // Flags
    printf("Z = %d\n", cpu->flagZ);
    printf("C = %d\n", cpu->flagC);

    // Memória acessada
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (cpu->mem_acessed[i]) {
            printf("[0x%04X] = 0x%04hX\n", i, cpu->memory[i]);
        }
    }

    // Pilha
    if (cpu->registers[14] != MEMORY_SIZE) {
        for (int i = MEMORY_SIZE - 1; i >= cpu->registers[14]; i--) {
            printf("[0x%04X] = 0x%04hX\n", i, cpu->memory[i]);
        }
    }
}
