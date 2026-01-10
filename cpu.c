#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include "cpu.h"

uint16_t breakpoints[MAX_BREAKPOINTS];
int qtd_breakpoints = 0;

void cpu_init(CPU *cpu)
{
    memset(cpu, 0, sizeof(CPU));

    cpu->registers[15] = 0x0000;      // pc
    cpu->registers[14] = MEMORY_SIZE; // SP
    cpu->halt = false;
}

void cpu_instruction_execute(CPU *cpu)
{
    uint16_t opcode = cpu->IR & 0x000F; // Pegar apenas o opcode (4 bits menos significativos)
    // OBS: opcode vem do IR, que ja foi carregado com a instrucao atual

    switch (opcode)
    {

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
    case 0x0:
    { // JMP
        int16_t imm = (int16_t)(cpu->IR & 0xFFF0);
        imm = imm >> 4; // extensão de sinal

        cpu->registers[15] = cpu->registers[15] + imm;
        break;
    }
    case 0x1:
    { // JCOND
        uint16_t cond = (cpu->IR >> 14) & 0x3;

        int16_t imm = (int16_t)(cpu->IR & 0x3FF0);
        imm = imm >> 4; // extensão de sinal

        if (imm & 0x0200)
        {
            imm |= 0xFC00; // Preenche os bits superiores com 1
        }

        bool jump = false;

        switch (cond)
        {
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

        if (jump)
        {
            cpu->registers[15] = cpu->registers[15] + imm;
        }
        break;
    }

    case 0x2:
    {
        uint8_t Rd = GET_RD(cpu->IR);
        uint8_t Rm = GET_RM(cpu->IR);
        uint8_t imm = GET_IMM4(cpu->IR);

        uint16_t addr = cpu->registers[Rm] + imm;

        /* IO – ENTRADA */
        if (addr == 0xF000)
        { // CHAR IN
            char c;
            scanf(" %c", &c);
            cpu->registers[Rd] = (uint16_t)c;
        }
        else if (addr == 0xF002)
        { // INT IN
            int x;
            scanf("%d", &x);
            cpu->registers[Rd] = (uint16_t)x;
        }
        /* MEMÓRIA NORMAL */
        else if (addr < MEMORY_SIZE)
        {
            cpu->registers[Rd] = cpu->memory[addr];
            cpu->mem_acessed[addr] = true;
        }
        break;
    }

    /* STR – MEM[Rm + imm] = Rn */
    case 0x3:
    {
        uint8_t Rn = GET_RN(cpu->IR);
        uint8_t Rm = GET_RM(cpu->IR);
        uint8_t imm = (cpu->IR >> 12) & 0x000F;

        uint16_t addr = cpu->registers[Rm] + imm;

        /* IO – SAÍDA */
        if (addr == 0xF001 || addr == 0xF000)
        { // CHAR OUT
            printf("OUT <= %c\n", (char)cpu->registers[Rn]);
        }
        else if (addr == 0xF003)
        { // INT OUT
            printf("OUT <= %d\n", (int16_t)cpu->registers[Rn]);
        }
        /* MEMÓRIA NORMAL */
        else if (addr < MEMORY_SIZE)
        {
            cpu->memory[addr] = cpu->registers[Rn];
            cpu->mem_acessed[addr] = true;
        }
        break;
    }

    /* MOV */
    case 0x4:
    {
        uint8_t Rd = GET_RD(cpu->IR);

        // Imediato de 8 bits (bits 4 a 11) com extensão de sinal
        int8_t imm8 = (int8_t)((cpu->IR >> 4) & 0xFF);
        cpu->registers[Rd] = (uint16_t)imm8;
        break;
    }

    case 0x5:
    { // ADD Rd = Rm + Rn
        uint8_t Rd = GET_RD(cpu->IR);
        uint8_t Rm = GET_RM(cpu->IR);
        uint8_t Rn = GET_RN(cpu->IR);

        uint32_t result = cpu->registers[Rm] + cpu->registers[Rn];
        cpu->registers[Rd] = (uint16_t)result;

        cpu->flagZ = (cpu->registers[Rd] == 0);
        cpu->flagC = (result > 0xFFFF);
        break;
    }

    case 0x6:
    { // ADDI Rd = Rm + #Im
        uint8_t Rd = GET_RD(cpu->IR);
        uint8_t Rm = GET_RM(cpu->IR);
        uint8_t imm = GET_IMM4(cpu->IR);

        uint32_t result = cpu->registers[Rm] + imm;
        cpu->registers[Rd] = (uint16_t)result;

        cpu->flagZ = (cpu->registers[Rd] == 0);
        cpu->flagC = (result > 0xFFFF);
        break;
    }

    case 0x7:
    { // SUB Rd = Rm - Rn
        uint8_t Rd = GET_RD(cpu->IR);
        uint8_t Rm = GET_RM(cpu->IR);
        uint8_t Rn = GET_RN(cpu->IR);

        int32_t result = (int32_t)cpu->registers[Rm] -
                         (int32_t)cpu->registers[Rn];

        cpu->registers[Rd] = (uint16_t)result;

        cpu->flagZ = (cpu->registers[Rd] == 0);
        cpu->flagC = (result < 0);
        break;
    }

    case 0x8:
    { // SUBI Rd = Rm - #Im
        uint8_t Rd = GET_RD(cpu->IR);
        uint8_t Rm = GET_RM(cpu->IR);
        uint8_t imm = GET_IMM4(cpu->IR);

        int32_t result = (int32_t)cpu->registers[Rm] - imm;
        cpu->registers[Rd] = (uint16_t)result;

        cpu->flagZ = (cpu->registers[Rd] == 0);
        cpu->flagC = (result < 0);
        break;
    }

    case 0x9:
    { // AND Rd = Rm and Rn
        uint8_t Rd = GET_RD(cpu->IR);
        uint8_t Rm = GET_RM(cpu->IR);
        uint8_t Rn = GET_RN(cpu->IR);

        cpu->registers[Rd] = cpu->registers[Rm] & cpu->registers[Rn];

        cpu->flagZ = (cpu->registers[Rd] == 0);
        cpu->flagC = 0;
        break;
    }

    case 0xA:
    { // OR Rd = Rm OR Rn
        uint8_t Rd = GET_RD(cpu->IR);
        uint8_t Rm = GET_RM(cpu->IR);
        uint8_t Rn = GET_RN(cpu->IR);

        cpu->registers[Rd] = cpu->registers[Rm] | cpu->registers[Rn];

        cpu->flagZ = (cpu->registers[Rd] == 0);
        cpu->flagC = 0;
        break;
    }

    case 0xB:
    { // SHR Rd = Rm >> #Im
        uint8_t Rd = GET_RD(cpu->IR);
        uint8_t Rm = GET_RM(cpu->IR);
        uint8_t imm = GET_IMM4(cpu->IR);

        cpu->registers[Rd] = cpu->registers[Rm] >> imm;

        cpu->flagZ = (cpu->registers[Rd] == 0);
        cpu->flagC = 0;
        break;
    }

    case 0xC:
    { // SHL Rd = Rm << #Im
        uint8_t Rd = GET_RD(cpu->IR);
        uint8_t Rm = GET_RM(cpu->IR);
        uint8_t imm = GET_IMM4(cpu->IR);

        cpu->registers[Rd] = cpu->registers[Rm] << imm;

        cpu->flagZ = (cpu->registers[Rd] == 0);
        cpu->flagC = 0;
        break;
    }

    case 0xD:
    { // CMP Rm, Rn =>
        uint8_t Rm = GET_RM(cpu->IR);
        uint8_t Rn = GET_RN(cpu->IR);

        cpu->flagZ = (cpu->registers[Rm] == cpu->registers[Rn]); // z = Rm = Rn
        cpu->flagC = (cpu->registers[Rm] < cpu->registers[Rn]);  // c = Rm < Rn
        break;
    }

    /* PUSH */
    case 0xE:
    {
        uint8_t Rn = GET_RN(cpu->IR);

        cpu->registers[14]--; // SP--
        cpu->memory[cpu->registers[14]] = cpu->registers[Rn];
        cpu->mem_acessed[cpu->registers[14]] = true;
        break;
    }

    case 0xF:
    { // HALT && POP
        if (cpu->IR == 0xFFFF)
        {
            // --- HALT ---
            print_cpu_state(cpu);
            cpu->halt = true;
            // Decrementa PC para mostrar o PC apontando para o HALT na saída final
            cpu->registers[15]--;
        }
        else
        {
            // --- POP ---
            uint8_t Rd = GET_RD(cpu->IR);
            if (cpu->registers[14] < MEMORY_SIZE)
            {
                cpu->registers[Rd] = cpu->memory[cpu->registers[14]];
                cpu->mem_acessed[cpu->registers[14]] = true; // Opcional marcar leitura
            }
            cpu->registers[14]++; // SP++
        }
        break;
    }

    default:
        // Instrução inválida
        printf("Instrução inválida: 0x%04hX\n", cpu->IR);
        cpu->halt = true;
        break;
    }
}

void cpu_run(CPU *cpu)
{
    while (!cpu->halt)
    {
        uint16_t pc_anterior = cpu->registers[15];

        cpu->IR = cpu->memory[cpu->registers[15]];
        cpu->registers[15]++;

        if (breakpoint(pc_anterior))
        {
            print_cpu_state(cpu);
        }

        cpu_instruction_execute(cpu);
    }
}

bool breakpoint(uint16_t pc)
{
    for (int i = 0; i < qtd_breakpoints; i++)
    {
        if (breakpoints[i] == pc)
            return true;
    }
    return false;
}

void print_cpu_state(CPU *cpu)
{
    // Registradores
    for (int i = 0; i < 16; i++)
    {
        printf("R%d = 0x%04hX\n", i, cpu->registers[i]);
    }

    // Flags
    printf("Z = %d\n", cpu->flagZ);
    printf("C = %d\n", cpu->flagC);

    // Memória acessada (dados do programa, não pilha e não instruções)
    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        // --- FILTRO DE LIMPEZA ---
        // Ignoramos a região da pilha (acima de 0x1000) aqui
        // A pilha será impressa separadamente abaixo
        if (i > 0x1000)
            break;

        if (cpu->mem_acessed[i])
        {
            printf("[0x%04X] = 0x%04hX\n", i, cpu->memory[i]);
        }
    }

    // A pilha é descendente: cresce de 0x2000 para baixo
    if (cpu->registers[14] < MEMORY_SIZE)
    {
        // Imprime da posição atual do SP até 0x1FFF (último elemento empilhado)
        for (int i = MEMORY_SIZE - 1; i >= cpu->registers[14]; i--)
        {
            if (cpu->mem_acessed[i])
            {
                printf("[0x%04X] = 0x%04hX\n", i, cpu->memory[i]);
            }
        }
    }
}